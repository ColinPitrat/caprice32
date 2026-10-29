#!/bin/bash

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
for spec in "lock:0x8000" "rom:0x9100" "test:0x8000"
do
  tst="${spec%%:*}"
  address="${spec#*:}"
  $CAP32DIR/cap32 -c cap32.cfg -O system.limit_speed=0 -a CAP32_WAITBREAK -a CAP32_EXIT -i ${tst}.raw -o ${address} >> "${LOGFILE}" 2>&1

  # Strip 0x02 bytes generated when ga_io_decode scans all 65536 I/O ports with 0x82
  tr -d '\002' < ${OUTPUT_DIR}/printer.dat > ${OUTPUT_DIR}/printer-${tst}.dat
  rm -f ${OUTPUT_DIR}/printer.dat

  if ! $DIFF -au model/printer-${tst}.dat ${OUTPUT_DIR}/printer-${tst}.dat >> "${LOGFILE}"
  then
    echo "!! Test failed for ${tst} (loaded at address=${address})"
    rc=1
  else
    echo "  $tst: PASS"
  fi
done

if [ $rc -ne 0 ]
then
  cat "${LOGFILE}"
fi
exit $rc
