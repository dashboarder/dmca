#!/bin/bash

# Return an error if any commands below fail
set -e

echo "Creating test filesystem"

HFS_TEMPDIR=$(mktemp -d -t hfstest)

TESTFS_ROOT=${HFS_TEMPDIR}/testfs_root
export TESTFS_COMPARE_ROOT=${HFS_TEMPDIR}/testfs_compare
export TESTFS_IMAGE=${HFS_TEMPDIR}/testfs.dmg

DATA_PATTERN=evYya7KUSI1y5Tc05IQftxDuoIz8SlIRHDQLTA

mkdir $TESTFS_ROOT

for i in a a/a bb bb/bb-a bb/bb-b ccc dddd eeeee ffffff ggggggg hhhhhhhh iiiiiiiii jjjjjjjjjj kkkkkkkkkkk lllllllllllllllllllllllllllllllllllll m n o p q dir; do mkdir -p ${TESTFS_ROOT}/$i; done

dd if=/dev/zero of=${TESTFS_ROOT}/dir/testfile0 bs=1 count=0
dd if=/dev/zero of=${TESTFS_ROOT}/dir/testfile1 bs=1 count=1
dd if=/dev/zero of=${TESTFS_ROOT}/dir/testfile2 bs=1 count=2
dd if=/dev/zero of=${TESTFS_ROOT}/dir/testfile512 bs=1 count=512
dd if=/dev/zero of=${TESTFS_ROOT}/dir/testfile1024 bs=1 count=1024
for i in {1..1024}; do echo ${DATA_PATTERN} >> ${TESTFS_ROOT}/dir/testfile-${DATA_PATTERN}; done

echo "Creating disk image for test filesystem"

hdiutil create -size 4m -srcfolder $TESTFS_ROOT -format UDRW -fs HFSX -layout NONE $TESTFS_IMAGE

./fs
