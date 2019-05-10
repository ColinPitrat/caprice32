#!/bin/bash

TSTDIR=`dirname "$0"`
LOGFILE="test.log"
CAP32DIR="$TSTDIR/../../../"

export SDL_VIDEODRIVER=dummy
cd "$TSTDIR"
touch "${LOGFILE}"

$CAP32DIR/cap32 -c cap32.cfg -a "run\"hello" -a "call 0" -a CAP32_WAITBREAK -a CAP32_EXIT hello.zip >> "${LOGFILE}" 2>&1

DIFF=diff
if [ -f "msys64/usr/bin/diff" ]
then
	DIFF=msys64/usr/bin/diff
fi
echo "msys64:"
ls msys64
echo "msys64/usr/bin:"
ls msys64/usr/bin
echo "msys64/usr/bin/diff:"
ls -l msys64/usr/bin/diff
echo "c:"
ls c
echo "c/Program Files:"
ls "c/Program Files"

if $DIFF printer.dat expected.dat >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
