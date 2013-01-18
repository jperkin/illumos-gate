/*
 * This file and its contents are supplied under the terms of the
 * Common Development and Distribution License ("CDDL"), version 1.0.
 * You may only use this file in accordance with the terms of version
 * 1.0 of the CDDL.
 *
 * A full copy of the text of the CDDL should have accompanied this
 * source.  A copy of the CDDL is also available via the Internet at
 * http://www.illumos.org/license/CDDL.
 */

#include <_libelf.h>
#include <stdio.h>
#include <strings.h>
#include <conv.h>
#include <msg.h>
#include <_elfdump.h>
#include <dwarf.h>

#include <sys/elf_ARM.h>

#define	ARM_TAG_TYPE_STRING	0x1
#define	ARM_TAG_TYPE_ULEB	0x2

#define	ARM_TAG_GET_TYPE(tag)						\
	(((tag == ARM_TAG_CPU_RAW_NAME) || (tag == ARM_TAG_CPU_NAME) ||	\
	(tag == ARM_TAG_COMPATIBILITY) ||				\
	(tag == ARM_TAG_ALSO_COMPATIBLE_WITH) ||			\
	(tag == ARM_TAG_CONFORMANCE)) ?					\
	ARM_TAG_TYPE_STRING : ARM_TAG_TYPE_ULEB)

static uint_t
extract_uint32(const uchar_t *data, uint64_t *off, int do_swap)
{
	uint_t	r;
	uchar_t *p = (uchar_t *)&r;

	if (do_swap)
		UL_ASSIGN_BSWAP_WORD(p, data + *off);
	else
		UL_ASSIGN_WORD(p, data + *off);

	*off += 4;

	return (r);
}

static Boolean
dump_arm_attr_tag(const char *file, const char *sh_name, uchar_t *data,
    uint64_t *off, size_t len)
{
	uint64_t	 stag;
	Conv_inv_buf_t	inv_buf;

	if (uleb_extract(data, off, len, &stag) != DW_SUCCESS) {
		(void) fprintf(stderr, MSG_INTL(MSG_ERR_DWOVRFLW), file,
		    sh_name);
		return (FALSE);
	}


	if (ARM_TAG_GET_TYPE(stag) == ARM_TAG_TYPE_ULEB) {
		uint64_t val;

		if (uleb_extract(data, off, len, &val) != DW_SUCCESS) {
			(void) fprintf(stderr, MSG_INTL(MSG_ERR_DWOVRFLW), file,
			    sh_name);
			return (FALSE);
		}

		/*
		 * the tag length is a 32-bit value and the printf expression
		 * below expects that. Verify this fits before truncating.
		 */
		if (val >= UINT32_MAX) {
			(void) fprintf(stderr,
			    MSG_INTL(MSG_ARM_ATTR_ERR_TOO_LARGE));
			return (FALSE);
		}

		dbg_print(0, MSG_INTL(MSG_ARM_ATTR_VALUE),
		    conv_arm_tag(stag, 0, &inv_buf), (uint32_t)val);
	} else {
		size_t slen;
		char *val = (char *)data + *off;

		slen = strnlen(val, len - 1);
		if (slen == len - 1 && val[slen] != '\0') {
			(void) fprintf(stderr,
			    MSG_INTL(MSG_ARM_ATTR_ERR_UNTERM_STR));
			return (FALSE);
		}
		*off += strlen(val) + 1;
		dbg_print(0, MSG_INTL(MSG_ARM_ATTR_STRVALUE),
		    conv_arm_tag(stag, 0, &inv_buf), val);
	}

	return (TRUE);
}

static Boolean
dump_arm_attr_section(const char *file, const char *sh_name, uchar_t *data,
    uint64_t *off, int do_swap)
{
	size_t  	init_len = *off;
	uint32_t	sec_size = extract_uint32(data, off, do_swap);
	char		*vendor = NULL;

	vendor = (char *)(data + *off);
	dbg_print(0, MSG_INTL(MSG_ARM_ATTR_VENDOR), vendor, sec_size);
	*off += strlen(vendor) + 1;

	if (strcmp(vendor, MSG_ORIG(MSG_ARM_ATTR_AEABI)) != 0) {
		dbg_print(0, MSG_ORIG(MSG_STR_EMPTY));
		*off = (init_len + sec_size);
		return (FALSE);
	}

	if (sec_size < 4) {
		dbg_print(0, MSG_INTL(MSG_ARM_ATTR_INVAL_SEC));
		*off = (init_len + sec_size);
		return (FALSE);
	}

	while ((*off - init_len) < sec_size) {
		uint64_t	tag;
		uint32_t	tsize = 0;
		uchar_t		*end = NULL;
		size_t		data_len;
		Conv_inv_buf_t	inv_buf;
		uint64_t	orig_off = *off;

		if (uleb_extract(data, off, sec_size, &tag) !=
		    DW_SUCCESS) {
			(void) fprintf(stderr, MSG_INTL(MSG_ERR_DWOVRFLW),
			    file, sh_name);
			return (FALSE);
		}

		tsize = extract_uint32(data, off, do_swap);

		data_len = orig_off + tsize;
		end = data + orig_off + tsize;

		dbg_print(0, MSG_INTL(MSG_ARM_ATTR_TAG),
		    conv_arm_tag(tag, 0, &inv_buf), tsize);

		if (tag != ARM_TAG_FILE) {
			uint32_t specifier;

			while ((specifier = extract_uint32(data, off,
			    do_swap)) != 0) {
				if (tag == ARM_TAG_SYMBOL)
					dbg_print(0,
					    MSG_INTL(MSG_ARM_ATTR_SYMBOL_SPEC),
					    specifier);
				else
					dbg_print(0,
					    MSG_INTL(MSG_ARM_ATTR_SECT_SPEC),
					    specifier);
			}
		}

		while ((data + *off) < end)
			dump_arm_attr_tag(file, sh_name, data, off, data_len);
	}

	return (TRUE);
}

/*
 * The .ARM.attributes section is of the form
 *
 * [byte:VERSION   # 'A'
 *   [ uint32:LENGTH string:VENDOR bytes:DATA ]
 *   [ uint32:LENGTH string:VENDOR bytes:DATA ]
 *   [ uint32:LENGTH string:VENDOR bytes:DATA ]...]
 *
 * Where only sub-sections of name "aeabi" are specified by the ABI and of
 * known content.
 *
 * Within an "aeabi" vendored section, are blocks, of one of the forms:
 *   [ ARM_TAG_FILE uint32:SIZE bytes:DATA ]
 *   [ ARM_TAG_SECTION uint32:SIZE uint32:SECNDX1,
 *     uint32:SECNDX2..., uint32:0, bytes:DATA ]
 *   [ ARM_TAG_SYMBOL uint32:SIZE uint32:SYMNDX1,
 *     uint32:SYMNBX2..., uint32:0, bytes:DATA ]
 *
 * The payload of each of this is a tag/value pair, where the value is either
 * a ULEB128 encoded integer, or a null terminated string (dependent on the
 * tag)
 */
void
dump_arm_attributes(Cache *cache, Word shnum, const char *file, int do_swap)
{
	int cnt;

	for (cnt = 1; cnt < shnum; cnt++) {
		Cache	*_cache = &cache[cnt];
		Shdr	*shdr = _cache->c_shdr;
		uchar_t	*data = (uchar_t *)_cache->c_data->d_buf;
		size_t	dsize = _cache->c_data->d_size;
		uint64_t off = 0;
		uchar_t	attrver = 0;

		if (shdr->sh_type != SHT_ARM_ATTRIBUTES)
			continue;

		dbg_print(0, MSG_ORIG(MSG_STR_EMPTY));
		dbg_print(0, MSG_INTL(MSG_ELF_SCN_ARMATTRS), _cache->c_name);

		attrver = *data++;
		dsize--;

		dbg_print(0, MSG_INTL(MSG_ARM_ATTR_VERSION), attrver,
		    (attrver != ARM_ATTR_VERSION) ?
		    MSG_INTL(MSG_ARM_ATTR_UNSUPPORTED) : "");

		if (attrver != ARM_ATTR_VERSION)
			return;

		while (dsize - off > 0) {
			if (dump_arm_attr_section(file, _cache->c_name, data,
			    &off, do_swap) == FALSE) {
				return;
			}
		}
	}
}
