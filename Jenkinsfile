pipeline {
	agent any
	stages {
		stage('Build i386 tools') {
			steps {
				sh '''
cleanup()
{
	rm -rf ${WORKSPACE}/cross-proto ${WORKSPACE}/proto
	cd ${WORKSPACE}/usr/src && git clean -fdx
}

tar_exit_success()
{
	mkdir -p ${WORKSPACE}/output
	cd ${WORKSPACE}/usr/src/tools/proto/root_i386-nd
	gtar -zcvpf ${WORKSPACE}/output/onbld.tar.gz ./opt
	cd ${WORKSPACE}
	gtar -zcvpf ${WORKSPACE}/output/proto.tar.gz ./proto
	exit 0
}

tar_exit_fail()
{
	mkdir -p ${WORKSPACE}/output
	cd ${WORKSPACE}/usr/src/tools
	gtar -zcvpf ${WORKSPACE}/output/proto-failed.tar.gz ./proto
	exit 1
}

trap cleanup INT TERM EXIT

# Ensure we start clean regardless
cleanup
rm -rf ${WORKSPACE}/output

cd ${WORKSPACE}

curl -kOL https://jenkins.perkin.org.uk/job/cross-extra/job/master/lastSuccessfulBuild/artifact/output/proto.tar.gz
tar -zxpf proto.tar.gz
mv proto cross-proto
rm -f proto.tar.gz

#
# Create override environment settings for SmartOS.
#
cat >${WORKSPACE}/override.env <<EOF
export i386_CLOSEDBINS=
export PYTHON=/opt/local/bin/python2.7
export PYTHON3=/opt/local/bin/python3.7
export PYTHON3_VERSION=3.7
export PYTHON3_PKGVERS=-37
export LEX=/opt/local/bin/lex
export YACC=/opt/local/bin/yacc
export MCS=/usr/bin/mcs
export STRIP=/usr/bin/strip
export RPCGEN=/opt/local/bin/rpcgen
export ELFDUMP=/usr/bin/elfdump
export PERL=/opt/local/bin/perl
export PATH=$PATH:/opt/local/bin
EOF

cd usr/src

# CROSS_PROTO is used to determine some illumos.sh variables
export CROSS_PROTO=${WORKSPACE}/cross-proto
export CROSS_ADJUNCT=${CROSS_PROTO}/i86pc/usr
export NATIVE_MACH=i386
export i386_CC=${WORKSPACE}/cross-proto/i86pc/usr/gcc/7/bin/gcc

ksh ./tools/scripts/bldenv \
    ./tools/env/illumos.sh \
    ". ${WORKSPACE}/override.env; /opt/local/bin/dmake setup" \
    || tar_exit_fail

for dir in lib/libc lib/libdisasm lib/libuutil lib/libdemangle lib/libcustr cmd/dis test/util-tests/tests/dis; do
	ksh ./tools/scripts/bldenv \
	    ./tools/env/illumos.sh \
	    ". ${WORKSPACE}/override.env; cd $dir && /opt/local/bin/dmake install" \
	    || tar_exit_fail
done

cd ${WORKSPACE}/proto/root_i386/opt/util-tests/tests/dis
./distest -p arm=${WORKSPACE}/cross-proto/armv8/usr/gnu/bin/gas -n

tar_exit_success
				'''
				archiveArtifacts artifacts: 'output/*.gz', fingerprint: true
			}
		}
	}
}
