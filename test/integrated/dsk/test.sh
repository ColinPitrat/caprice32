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
if $DIFF printer.dat expected.dat >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
