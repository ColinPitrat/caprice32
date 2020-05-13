#!/bin/bash

TMPDIR=`mktemp -d`

CLANG_FORMAT=$1
shift
CLANG_ARGS=$1
shift

error_files=""
format_error=0
for file in $@
do
    mkdir -p `dirname $TMPDIR/$file`
    $CLANG_FORMAT -style=Google $file > $TMPDIR/$file
    if ! diff $file $TMPDIR/$file
    then
        format_error=1
        error_files="$error_files $file"
    fi
done
#rm -rf $TMPDIR
[ $format_error -eq 0 ] || echo "**ERROR**: Files with improper formatting: $error_files"

exit $format_error
