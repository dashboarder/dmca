#!/bin/sh
#
# Find .d files that are produced by compilation and produce a list of all the files they represent
#

# Find source and headers that were built
rm -f built.list built.tmp
for dfile in `find build -name "*.d"`; do
    echo "parsing $dfile"
    cat $dfile | tr : \\n | tail +2 | tr -d \\ | tr ' ' \\n | grep \^\[a-z\] >> built.tmp
done
cat built.tmp | sort | uniq > built.list

# Find all sources
echo "finding base sources"
rm -f all.list
find . -name "*.[chS]" | cut -c 3-1000 > all.list

# Make a list of what's not used
rm -f delete.list
for file in `cat all.list`; do
    /bin/echo -n "checking for $file ... "
    if grep -q $file built.list; then
        echo "found, keeping"
    else
        # handle exceptions
        case $file in
            platform/defaults/template.c ) echo "protected" ;;
            platform/generic/template.c ) echo "protected" ;;
            tools/bhc.c ) echo "protected" ;;

            * )
                echo "not found, deleting"
                echo $file >> delete.list
                ;;
        esac
    fi
done

# Delete unwanted files
for file in `cat delete.list`; do
    rm $file
done

rm -f built.list built.tmp all.list delete.list
