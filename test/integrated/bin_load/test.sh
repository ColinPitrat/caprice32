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

if [ -z "$DIFF" ]
then
  DIFF=diff
fi

rm -rvf ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}
echo "" > "${LOGFILE}"

cd "$TSTDIR"

rc=0
for i in 1 2 3 4 5 6 7 8 9 a b
do
  address=0x${i}000
  $CAP32DIR/cap32 -c cap32.cfg -a CAP32_DELAY -a CAP32_DELAY -a CAP32_SCRNSHOT -a CAP32_EXIT -i cap32_bin_load_test.bin -o ${address} >> "${LOGFILE}" 2>&1

  mv -v screenshot_*png ${OUTPUT_DIR}/screenshot-${address}.png

  if ! $DIFF ${OUTPUT_DIR}/screenshot-${address}.png model/screenshot-${address}.png >> "${LOGFILE}"
  then
    echo "!! Test failed for load at address=${address}"
    rc=1
  fi
done

if [ $rc -ne 0 ]
then
  cat "${LOGFILE}"
fi
exit $rc
