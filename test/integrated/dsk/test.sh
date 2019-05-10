#!/bin/bash

TSTDIR=`dirname "$0"`
LOGFILE="test.log"
CAP32DIR="$TSTDIR/../../../"

export SDL_VIDEODRIVER=dummy
cd "$TSTDIR"
touch "${LOGFILE}"

$CAP32DIR/cap32 -c cap32.cfg -a "run\"hello" -a "call 0" -a CAP32_WAITBREAK -a CAP32_EXIT hello.zip >> "${LOGFILE}" 2>&1

echo "$DIFF printer.dat expected.dat:"
$DIFF printer.dat expected.dat
echo "$DIFF --help:"
$DIFF --help

echo "Not expected" > not_expected.dat
for my_diff in /c/msys64/usr/lib/python2.7/site-packages/bzrlib/tests/test_patches_data/diff /c/msys64/usr/share/quilt/diff /c/cygwin/bin/diff.exe /c/cygwin64/bin/diff.exe /c/MinGW/msys/1.0/bin/diff.exe /c/msys64/usr/bin/diff.exe /c/msys64/usr/bin/svn-tools/diff.exe "/c/Program Files/Git/usr/bin/diff.exe" /c/projects/caprice32/msys64/usr/bin/diff.exe /c/Ruby193/DevKit/bin/diff.exe /c/Ruby23/DevKit/bin/diff.exe /c/Ruby23-x64/DevKit/bin/diff.exe
do
	echo " == $my_diff printer.dat not_expected.dat"
	$my_diff printer.dat not_expected.dat
done

if $DIFF printer.dat expected.dat >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
