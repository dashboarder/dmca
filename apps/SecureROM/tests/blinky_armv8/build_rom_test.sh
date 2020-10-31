#!/bin/bash

set -e

function usage()
{
	echo "build_rom_test.sh -p <platform> [-a <gpio address>] [-v <gpio value>]"
	exit 1
}

args=$(getopt "a:p:v" $*)
if [[ $? != 0 ]]; then
	usage
fi

# GPIO[0] == GPIO id 20 == 0x20f100050
GPIO_BASE=0x20f100050
GPIO_VALUE=0x70203
PLATFORM="t8010"

set -- $args
for arg; do
	case $arg
	in
		-a)
			GPIO_BASE=$2; shift; shift;;
		-v)
			GPIO_VALUE=$2; shift; shift;;
		-p)
			PLATFORM=$2; shift; shift;;
	esac
done

if [[ "$GPIO_BASE" == "" || "$GPIO_VALUE" == "" || "$PLATFORM" == "" ]]; then
	usage
fi

echo "Cleaning"
rm -f rom_test.o rom_test rom_test.bin rom_test_*.im4p

echo "Running clang"
xcrun -sdk iphoneos.internal clang -arch arm64 \
    -static \
    -std=gnu99 -Werror -W -Wall \
    -DGPIO_BASE=${GPIO_BASE} -DGPIO_VALUE=${GPIO_VALUE} \
    -c rom_test.c -o rom_test.o

echo "Standalone link"
xcrun -sdk iphoneos.internal clang -arch arm64 \
    -static \
    -Wl,-new_linker \
    -Wl,-preload \
    -Wl,-merge_zero_fill_sections \
    -nodefaultlibs \
    -nostartfiles \
    -dead_strip \
    -e _start \
    -o rom_test rom_test.o

echo "Stripping"
xcrun -sdk iphoneos.internal strip -no_uuid rom_test

echo "Finding __text section"
text_offset=$(xcrun -sdk iphoneos.internal otool -l rom_test | grep -A4 'sectname __text' | grep 'offset' | awk '{print $2}')
text_size=$(xcrun -sdk iphoneos.internal otool -l rom_test | grep -A4 'sectname __text' | grep 'size' | awk '{print $2}')
echo "Found at $text_offset size $text_size"

echo "Generating bin"
dd if=rom_test ibs=1 skip=${text_offset} obs=1 count=${text_size} \
    >rom_test.bin 2>/dev/null

echo "Mach-O dump"
xcrun -sdk iphoneos.internal otool -tVj rom_test

echo "Bin Dump"
hexdump -C rom_test.bin

for type in ibss illb; do
    for encrypt in encrypted unencrypted; do
	echo "Generating ${encrypt} ${type} Image4 payload"
	xcrun -sdk iphoneos.internal img4payload \
	    -i rom_test.bin -o rom_test_${type}_${encrypt}.im4p \
	    -t ${type} -v rom_test_${type}_${encrypt}

	if [ "${encrypt}" = "encrypted" ]; then
	    xcrun -sdk iphoneos.internal img4encrypt \
		-i rom_test_${type}_${encrypt}.im4p \
		-o rom_test_${type}_${encrypt}.im4p \
		-p $PLATFORM
	fi
    done
done

# Personalization, for reference:

#xcrun -sdk iphoneos.internal personalize_img4 -i rom_test_ibss.im4p -o /tmp -c 0x<chip id> -b 0xff -d 1 -g 384 -e 0x12345678ABCD -n 0xa4f84e2a89c2ef04a07b9f948fec7c0767c8931bd67ede924040aa4f30afe4118a2d524d687fc0db2f4a27e36c7e75a0 -r 0x12345678AABBCCDD
#xcrun -sdk iphoneos.internal personalize_img4 -i rom_test_llb.im4p  -o /tmp -c 0x<chip id> -b 0xff -d 1 -g 384 -e 0x12345678ABCD -n 0xa4f84e2a89c2ef04a07b9f948fec7c0767c8931bd67ede924040aa4f30afe4118a2d524d687fc0db2f4a27e36c7e75a0 -r 0x12345678AABBCCDD

# Production signing, for reference:

#xcrun -sdk iphoneos.internal personalize_img4 -m -p -i rom_test_illb_encrypted.im4p -o /tmp -c 0x<chip id> -b 0xff -d 1 -g 384 -e 0x12345678ABCD -n 0xa4f84e2a89c2ef04a07b9f948fec7c0767c8931bd67ede924040aa4f30afe4118a2d524d687fc0db2f4a27e36c7e75a0 -s 0x1234567823456789345678904567890156789012 -r 0x12345678AABBCCDD -w -W

# Production signing with demotion, for reference:

#xcrun -sdk iphoneos.internal personalize_img4 -m -p -i rom_test_illb_encrypted.im4p -o /tmp -c 0x<chip id> -b 0xff -d 1 -g 384 -e 0x12345678ABCD -n 0xa4f84e2a89c2ef04a07b9f948fec7c0767c8931bd67ede924040aa4f30afe4118a2d524d687fc0db2f4a27e36c7e75a0 -s 0x1234567823456789345678904567890156789012 -r 0x12345678AABBCCDD -W -D 1

echo "Done"
