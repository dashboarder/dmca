#!/bin/sh
#
# After pickfiles, scrub the iBoot tree of anything not relevant to the current secure ROM
#

supported_platforms="t8002 t8010"
unwanted_dirs_files_list="target docs build BuildiBoot.sh"

if [ $# -ne "1" ] ; then
        echo "USAGE: scrub_SecureROM <platform>"
        exit
fi

found=0
for word in $supported_platforms; do
        if [ $1 == $word ] ; then
                found=1
        fi
done

if [ $found != 1 ]; then 
        echo "Unsupported platform $1"
        exit
fi

# init variables
securerom_platform=$1
new_working_dir="/tmp/iBoot.$securerom_platform.scrubbed"

# setup new working dir
rm -rf $new_working_dir
cp -R $PWD $new_working_dir

# switch to new working dir
pushd $new_working_dir > /dev/null

# other apps
for dir in `ls -d apps/*`; do
    case $dir in
        apps/SecureROM )
            ;;
        * )
            echo removing directory $dir
            rm -rf $dir
            ;;
        esac
done

# other platforms
for dir in `ls -d platform/*`; do
    case $dir in
        platform/generic )
            ;;
        platform/defaults )
            ;;
        platform/$securerom_platform )
            ;;
        * )
            echo removing directory $dir
            rm -rf $dir
            ;;
        esac
done

# all targets
rm -rf targets

# whitelist Makefiles
for mk in `find makefiles -type f`; do
    case $mk in
        makefiles/build.mk )
            ;;
        makefiles/config.mk )
            ;;
        makefiles/device_map.mk )
            ;;
        makefiles/headers.mk )
            ;;
        makefiles/lib.mk )
            ;;
        makefiles/libraries.mk )
            ;;
        makefiles/macros.mk )
            ;;
        makefiles/main.mk )
            ;;
        makefiles/main-nested-module.mk )
            ;;
        makefiles/macros.mk )
            ;;
        makefiles/tools.mk )
            ;;
        makefiles/SecureROM.mk )
            ;;
        * )
            echo removing file $mk
            rm -f $mk
            ;;
    esac
done

# whitelist tools
for tool in `find tools -type f`; do
    case $tool in
        tools/check_liblist.py )
            ;;
        tools/lldb_init_iboot.py )
            ;;
        tools/lldb_os_iboot.py )
            ;;
        tools/generate_debug_hashes.py )
            ;;
        tools/macho_post_process.py )
            ;;
        tools/macho.py )
            ;;
        * )
            echo removing file $tool
            rm -f $tool
            ;;
    esac
done

# anything referring to other architectures
for word in $supported_platforms; do
    if [ $word != $securerom_platform ] ; then
        find . -name "*$word*" -delete
    fi
done

# version control directories
rm -rf `find . -name ".svn" -type dir`

# strip bad words
for word in $supported_platforms; do
    if [ $word != $securerom_platform ] ; then
        files=`find . -type f | xargs grep -l $word`
        for file in $files; do
            tfile=`mktemp scrub.XXXX`
            sed -e "s/$word//g" < $file > $tfile
            cat $tfile > $file
            rm $tfile
        done
    fi

    # strip irrelevant test files
    dir="apps/SecureROM/tests/images_$word"
    if [ -d "$dir" ] ; then
        echo removing directory $dir
        rm -r "$dir"
    fi
done


# rules.mk with no adjacent sources
for mk in `find . -name rules.mk`; do
    dir=`dirname $mk`
    files=`find $dir -name "*.[chS]"`
    if [ -z "$files" ]; then
        echo removing file $mk
        rm -f $mk
    fi
done

# empty directories
for dir in `find -d . -type dir`; do
    files=`ls -1 $dir`
    if [ -z "$files" ]; then
        echo removing empty directory $dir
        rmdir $dir
    fi
done

# remove all unwanted directories
for word in $unwanted_dirs_files_list; do
        echo removing directory $word
        rm -rf $word
done

# back to original working dir
popd > /dev/null

echo "Scrubbed iBoot -> $new_working_dir"
