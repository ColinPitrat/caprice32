#!/bin/bash

TSTDIR=`dirname "$0"`
LOGFILE="test.log"
CAP32DIR="$TSTDIR/../../../"

export SDL_VIDEODRIVER=dummy
cd "$TSTDIR"
touch "${LOGFILE}"

$CAP32DIR/cap32 -c cap32.cfg -a "run\"hello" -a "call 0" -a CAP32_WAITBREAK -a CAP32_EXIT hello.zip >> "${LOGFILE}" 2>&1

DIFF=diff
if [ -f "/c/projects/msys64/usr/bin/diff" ]
then
	DIFF=/c/projects/msys64/usr/bin/diff
fi
echo "pwd:"
pwd
echo ""
echo "ls /c:"
ls /c
echo ""
echo "ls /c/projects/msys64/usr/bin"
ls /c/projects/msys64/usr/bin
echo ""
echo "ls /c/*:"
ls /c/*

if $DIFF printer.dat expected.dat >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
