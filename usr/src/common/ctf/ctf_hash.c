/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright 2020 OmniOS Community Edition (OmniOSce) Association.
 * Copyright 2026 Edgecast Cloud LLC.
 */

#include <ctf_impl.h>
#include <sys/debug.h>

static const ushort_t _CTF_EMPTY[1] = { 0 };

int
ctf_hash_create(ctf_hash_t *hp, ulong_t nelems)
{
	if (nelems > USHRT_MAX)
		return (EOVERFLOW);

	/*
	 * If the hash table is going to be empty, don't bother allocating any
	 * memory and make the only bucket point to a zero so lookups fail.
	 */
	if (nelems == 0) {
		bzero(hp, sizeof (ctf_hash_t));
		hp->h_buckets = (ushort_t *)_CTF_EMPTY;
		hp->h_nbuckets = 1;
		return (0);
	}

	hp->h_nbuckets = 211;		/* use a prime number of hash buckets */
	hp->h_nelems = nelems + 1;	/* we use index zero as a sentinel */
	hp->h_free = 1;			/* first free element is index 1 */

	hp->h_buckets = ctf_alloc(sizeof (ushort_t) * hp->h_nbuckets);
	hp->h_chains = ctf_alloc(sizeof (ctf_helem_t) * hp->h_nelems);

	if (hp->h_buckets == NULL || hp->h_chains == NULL) {
		ctf_hash_destroy(hp);
		return (EAGAIN);
	}

	bzero(hp->h_buckets, sizeof (ushort_t) * hp->h_nbuckets);
	bzero(hp->h_chains, sizeof (ctf_helem_t) * hp->h_nelems);

	return (0);
}

uint_t
ctf_hash_size(const ctf_hash_t *hp)
{
	return (hp->h_nelems ? hp->h_nelems - 1 : 0);
}

static ulong_t
ctf_hash_compute(const char *key, size_t len)
{
	ulong_t g, h = 0;
	const char *p, *q = key + len;

	for (p = key; p < q; p++) {
		h = (h << 4) + *p;

		if ((g = (h & 0xf0000000)) != 0) {
			h ^= (g >> 24);
			h ^= g;
		}
	}

	return (h);
}

int
ctf_hash_insert(ctf_hash_t *hp, ctf_file_t *fp, ushort_t type, uint_t name)
{
	ctf_strs_t *ctsp = &fp->ctf_str[CTF_NAME_STID(name)];
	const char *str = ctsp->cts_strs + CTF_NAME_OFFSET(name);
	ctf_helem_t *hep = &hp->h_chains[hp->h_free];
	ulong_t h;

	if (type == 0)
		return (EINVAL);

	if (hp->h_free >= hp->h_nelems)
		return (EOVERFLOW);

	if (ctsp->cts_strs == NULL)
		return (ECTF_STRTAB);

	if (ctsp->cts_len <= CTF_NAME_OFFSET(name))
		return (ECTF_BADNAME);

	if (str[0] == '\0')
		return (0); /* just ignore empty strings on behalf of caller */

	hep->h_name = name;
	hep->h_type = type;
	h = ctf_hash_compute(str, strlen(str)) % hp->h_nbuckets;
	hep->h_next = hp->h_buckets[h];
	hp->h_buckets[h] = hp->h_free++;

	return (0);
}

/*
 * Wrapper for ctf_hash_lookup/ctf_hash_insert: if the key is already in the
 * hash, override the previous definition with this new official definition.
 * If the key is not present, then call ctf_hash_insert() and hash it in.
 */
int
ctf_hash_define(ctf_hash_t *hp, ctf_file_t *fp, ushort_t type, uint_t name)
{
	const char *str = ctf_strptr(fp, name);
	ctf_helem_t *hep = ctf_hash_lookup(hp, fp, str, strlen(str));

	if (hep == NULL)
		return (ctf_hash_insert(hp, fp, type, name));

	hep->h_type = type;
	return (0);
}

ctf_helem_t *
ctf_hash_lookup(ctf_hash_t *hp, ctf_file_t *fp, const char *key, size_t len)
{
	ctf_helem_t *hep;
	ctf_strs_t *ctsp;
	const char *str;
	ushort_t i;

	ulong_t h = ctf_hash_compute(key, len) % hp->h_nbuckets;

	for (i = hp->h_buckets[h]; i != 0; i = hep->h_next) {
		hep = &hp->h_chains[i];
		ctsp = &fp->ctf_str[CTF_NAME_STID(hep->h_name)];
		str = ctsp->cts_strs + CTF_NAME_OFFSET(hep->h_name);

		if (strncmp(key, str, len) == 0 && str[len] == '\0')
			return (hep);
	}

	return (NULL);
}

void
ctf_hash_destroy(ctf_hash_t *hp)
{
	if (hp->h_buckets != NULL && hp->h_nbuckets != 1) {
		ctf_free(hp->h_buckets, sizeof (ushort_t) * hp->h_nbuckets);
		hp->h_buckets = NULL;
	}

	if (hp->h_chains != NULL) {
		ctf_free(hp->h_chains, sizeof (ctf_helem_t) * hp->h_nelems);
		hp->h_chains = NULL;
	}
}

void
ctf_hash_dump(const char *tag, ctf_hash_t *hp, ctf_file_t *fp)
{
	ctf_dprintf("---------------\nHash dump - %s\n", tag);

	for (ushort_t h = 0; h < hp->h_nbuckets; h++) {
		ctf_helem_t *hep;

		for (ushort_t i = hp->h_buckets[h]; i != 0; i = hep->h_next) {
			ctf_strs_t *ctsp;
			const char *str;

			hep = &hp->h_chains[i];
			ctsp = &fp->ctf_str[CTF_NAME_STID(hep->h_name)];
			str = ctsp->cts_strs + CTF_NAME_OFFSET(hep->h_name);

			ctf_dprintf(" - %3u/%3u  - '%s' type %u\n", h, i, str,
			    hep->h_type);
		}
	}
}

/*
 * General-purpose hash table keyed on C string pointers.
 *
 * ctf_hash_t (above) is designed for use during CTF container
 * deserialisation: its keys are string table offsets requiring a ctf_file_t
 * for resolution, its values are ushort_t type IDs, and it is limited to
 * USHRT_MAX entries.
 *
 * ctf_strhash_t is designed for transient lookups during merge, diff, and
 * DWARF conversion, where the caller has raw C string pointers (e.g. from
 * ctf_strptr or dwarf_formstring) and needs to associate them with arbitrary
 * data (type IDs, struct pointers, etc.) stored as void * values.
 *
 * The hash table uses FNV-1a hashing with separate chaining.  Elements are
 * pre-allocated in a flat array sized to nelems; no dynamic growth is
 * performed.  The caller must provide an upper bound on the number of
 * insertions at creation time.
 *
 * Multiple entries with the same name are permitted, allowing the table to
 * function as a multi-map (e.g. a struct and its forward declaration sharing
 * the same name).  ctf_strhash_lookup() returns the first element whose key
 * matches the given name, and ctf_strhash_next() the subsequent ones; both
 * filter the hash chain by key, so callers see only matching entries.
 */
int
ctf_strhash_create(ctf_strhash_t *hp, ulong_t nelems)
{
	uint_t nbuckets;

	if (nelems == 0) {
		bzero(hp, sizeof (ctf_strhash_t));
		return (0);
	}

	nbuckets = 128;
	while (nbuckets < nelems)
		nbuckets <<= 1;

	hp->h_nbuckets = nbuckets;
	hp->h_nelems = nelems + 1;	/* index 0 is sentinel */
	hp->h_free = 1;

	hp->h_buckets = ctf_alloc(sizeof (uint_t) * hp->h_nbuckets);
	hp->h_chains = ctf_alloc(sizeof (ctf_strhash_elem_t) * hp->h_nelems);

	if (hp->h_buckets == NULL || hp->h_chains == NULL) {
		ctf_strhash_destroy(hp);
		return (EAGAIN);
	}

	bzero(hp->h_buckets, sizeof (uint_t) * hp->h_nbuckets);
	bzero(hp->h_chains, sizeof (ctf_strhash_elem_t) * hp->h_nelems);

	return (0);
}

void
ctf_strhash_destroy(ctf_strhash_t *hp)
{
	if (hp->h_buckets != NULL) {
		ctf_free(hp->h_buckets, sizeof (uint_t) * hp->h_nbuckets);
		hp->h_buckets = NULL;
	}

	if (hp->h_chains != NULL) {
		ctf_free(hp->h_chains,
		    sizeof (ctf_strhash_elem_t) * hp->h_nelems);
		hp->h_chains = NULL;
	}
}

static ulong_t
ctf_strhash_compute(const char *key)
{
	ulong_t h = 2166136261u;

	for (; *key != '\0'; key++) {
		h ^= (ulong_t)(uchar_t)*key;
		h *= 16777619u;
	}

	return (h);
}

int
ctf_strhash_insert(ctf_strhash_t *hp, const char *name, void *value)
{
	ctf_strhash_elem_t *hep;
	ulong_t h;

	if (hp->h_free >= hp->h_nelems)
		return (EOVERFLOW);

	if (name == NULL)
		name = "";

	hep = &hp->h_chains[hp->h_free];
	hep->h_name = name;
	hep->h_value = value;
	hep->h_hash = ctf_strhash_compute(name);

	h = hep->h_hash % hp->h_nbuckets;
	hep->h_next = hp->h_buckets[h];
	hp->h_buckets[h] = hp->h_free++;

	return (0);
}

ctf_strhash_elem_t *
ctf_strhash_lookup(ctf_strhash_t *hp, const char *name)
{
	ulong_t h;
	uint_t i;

	if (hp->h_buckets == NULL)
		return (NULL);

	if (name == NULL)
		name = "";

	h = ctf_strhash_compute(name);

	for (i = hp->h_buckets[h % hp->h_nbuckets]; i != 0;
	    i = hp->h_chains[i].h_next) {
		ctf_strhash_elem_t *hep = &hp->h_chains[i];

		if (hep->h_hash == h && strcmp(hep->h_name, name) == 0)
			return (hep);
	}

	return (NULL);
}

ctf_strhash_elem_t *
ctf_strhash_next(ctf_strhash_t *hp, ctf_strhash_elem_t *elem)
{
	uint_t i;

	for (i = elem->h_next; i != 0; i = hp->h_chains[i].h_next) {
		ctf_strhash_elem_t *hep = &hp->h_chains[i];

		if (hep->h_hash == elem->h_hash &&
		    strcmp(hep->h_name, elem->h_name) == 0)
			return (hep);
	}

	return (NULL);
}
