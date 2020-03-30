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

nb_plugins=`$CAP32DIR/cap32 -V | grep "Number of video plugins available: " | cut -d : -f 2 | xargs`
let last_plugin=${nb_plugins}-1

rc=0
for style in `seq 0 $last_plugin`
do
  sed -i "s/^scr_style=.*$/scr_style=${style}/" cap32.cfg
  $CAP32DIR/cap32 -c cap32.cfg -a "print #8,\"style=${style}\"" -a CAP32_EXIT >> "${LOGFILE}" 2>&1

  mv ${OUTPUT_DIR}/printer.dat ${OUTPUT_DIR}/printer.dat.${style}
  if ! $DIFF ${OUTPUT_DIR}/printer.dat.${style} model/printer.dat.${style} >> "${LOGFILE}"
  then
    echo "!! Test failed for scr_style=${style}"
    rc=1
  fi
done

if [ $rc -ne 0 ]
then
  cat "${LOGFILE}"
fi
exit $rc
