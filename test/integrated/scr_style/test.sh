#!/bin/bash

# This test just verifies that no video plugin is broken to the point of
# segfaulting systematically.
# In particular, we're not verifying the output. We cannot use screenshots to
# verify the output because screenshots are taken before the application of the
# video plugin filter.

TSTDIR=`dirname "$0"`
OUTPUT_DIR="output"
LOGFILE="test.log"
CAP32DIR="${TSTDIR}/../../../"

rm -rv ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}
echo "" > "${LOGFILE}"

cd "$TSTDIR"

# TODO: Find a way to test the GL mode if built with
for style in `seq 0 10`
do
  sed -i "s/^scr_style=.*$/scr_style=${style}/" cap32.cfg
  $CAP32DIR/cap32 -c cap32.cfg -a "print #8,\"Hello, scr_style=${style}!\"" -a CAP32_EXIT >> "${LOGFILE}" 2>&1
  mv output/printer.dat output/printer.dat.${style}
done

if $DIFF -ur model ${OUTPUT_DIR} >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
