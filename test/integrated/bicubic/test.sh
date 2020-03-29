#!/bin/bash

TSTDIR=`dirname "$0"`
OUTPUT_DIR="output"
LOGFILE="test.log"
CAP32DIR="${TSTDIR}/../../../"

rm -rv ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}
touch "${LOGFILE}"

cd "$TSTDIR"

$CAP32DIR/cap32 -c cap32.cfg -a "print #8,\"Hello, World!\"" -a CAP32_EXIT >> "${LOGFILE}" 2>&1

if $DIFF output/printer.dat expected.dat >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
