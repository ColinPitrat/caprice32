#!/bin/bash

# This test just verifies that the CPC starts correctly.
# It proves that it manages to find config and roms.

DESTDIR=$1
LOGFILE=$(mktemp)
EXPECTED=$(mktemp)

sed -i 's/printer=0/printer=1/' ${DESTDIR}/etc/cap32.cfg
echo -e 'Hello, world!\r' > ${EXPECTED}
$DESTDIR/usr/local/bin/cap32 -a "print #8,\"Hello, world!\"" -a CAP32_EXIT >> "${LOGFILE}" 2>&1

if ! diff ${EXPECTED} printer.dat
then
  echo "!! make install test failed"
  cat ${LOGFILE}
  exit 1
fi
