#!/bin/bash

TSTDIR=`dirname "$0"`
OUTPUT_DIR="output"
LOGFILE="test.log"
CAP32DIR="${TSTDIR}/../../../"

rm -rv ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}
echo "" > "${LOGFILE}"

cd "$TSTDIR"
export SDL_VIDEODRIVER=dummy

# TODO: Remove CAP32_SCRNSHOT after verifying if it makes the test crash on s390x
$CAP32DIR/cap32 -c cap32.cfg -a "run\"hello" -a "call 0" -a CAP32_WAITBREAK -a CAP32_SCRNSHOT -a CAP32_EXIT hello.zip >> "${LOGFILE}" 2>&1

if $DIFF output/printer.dat expected.dat >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
