#!/bin/bash

TSTDIR=`dirname "$0"`
LOGFILE="test.log"
CAP32DIR="$TSTDIR/../../../"

export SDL_VIDEODRIVER=dummy
cd "$TSTDIR"
touch "${LOGFILE}"

$CAP32DIR/cap32 -c cap32.cfg -a "run\"hello" -a "call 0" -a CAP32_WAITBREAK -a CAP32_EXIT hello.zip >> "${LOGFILE}" 2>&1

echo "cap32 in TSTDIR:"
find $TSTDIR | grep cap32

echo "CAP32DIR: $CAP32DIR"
echo "TSTDIR: $TSTDIR"
echo "CAP32DIR content:"
ls $CAP32DIR
echo "TSTDIR content:"
ls $TSTDIR


if $DIFF printer.dat expected.dat >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
