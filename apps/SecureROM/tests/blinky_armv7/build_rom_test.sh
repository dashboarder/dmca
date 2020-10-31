#!/bin/bash

set -e

function usage()
{
	echo "build_rom_test.sh -f -p <platform> [-a <gpio address>] [-v <gpio value>]"
	echo " -f: Force rebuild rather than using pre-built / audited binaries"
	exit 1
}

args=$(getopt "fa:p:v" $*)
if [[ $? != 0 ]]; then
	usage
fi

FORCE_REBUILD=0
GPIO_BASE=0x47500000
GPIO_VALUE=0x70203
PLATFORM="t8002"

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
		-f)
			FORCE_REBUILD=1; shift;;
	esac
done

if [[ "$GPIO_BASE" == "" || "$GPIO_VALUE" == "" || "$PLATFORM" == "" ]]; then
	usage
fi

if [[ $FORCE_REBUILD != 0 ]]; then
	echo "Cleaning"
	rm -f rom_test.o rom_test rom_test.bin rom_test_*.im4p
fi

if ! [[ -f rom_test.o ]]; then
	echo "Running clang"
	xcrun -sdk iphoneos.internal clang -arch armv7 \
	    -static -marm \
	    -std=gnu99 -Werror -W -Wall \
	    -DGPIO_BASE=${GPIO_BASE} -DGPIO_VALUE=${GPIO_VALUE} \
	    -c rom_test.c -o rom_test.o
fi

if ! [[ -f rom_test ]]; then
	echo "Standalone link"
	xcrun -sdk iphoneos.internal clang -arch armv7 \
	    -static -marm \
	    -Wl,-new_linker \
	    -Wl,-preload \
	    -Wl,-merge_zero_fill_sections \
	    -nodefaultlibs \
	    -nostartfiles \
	    -dead_strip \
	    -e _start \
	    -o rom_test rom_test.o
fi

if ! [[ -f rom_test.bin ]]; then
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
fi


for type in ibss illb; do
	for encrypt in encrypted unencrypted; do
		if ! [[ -f rom_test_${type}_${encrypt}.im4p ]]; then
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
		fi

	done
done

# Personalization, for reference:

#xcrun -sdk iphoneos.internal personalize_img4 -i rom_test_ibss.im4p -o /tmp -c 0x<chip id> -b 0xff -d 1 -e 0x12345678ABCD -n 0x9A7A07F30FE84B6403AF5BB549B405BAE16461D2 -r 0x12345678AABBCCDD
#xcrun -sdk iphoneos.internal personalize_img4 -i rom_test_llb.im4p -o /tmp -c 0x<chip id> -b 0xff -d 1 -e 0x12345678ABCD -n 0x9A7A07F30FE84B6403AF5BB549B405BAE16461D2 -r 0x12345678AABBCCDD

# Production signing, for reference:

#xcrun -sdk iphoneos.internal personalize_img4 -m -p -i rom_test_illb_encrypted.im4p -o /tmp -c 0x<chip id> -b 0xff -d 1 -e 0x12345678ABCD -n 0xcd00fc2e1fa023796d5257b853880e18cd384f7d -s 0x1234567823456789345678904567890156789012 -r 0x12345678AABBCCDD -w -W

# Production signing with demotion, for reference:

#xcrun -sdk iphoneos.internal personalize_img4 -m -p -i rom_test_illb_encrypted.im4p -o /tmp -c 0x<chip id> -b 0xff -d 1 -e 0x12345678ABCD -n 0xcd00fc2e1fa023796d5257b853880e18cd384f7d -s 0x1234567823456789345678904567890156789012 -r 0x12345678AABBCCDD -W -D 1

echo "Done"
