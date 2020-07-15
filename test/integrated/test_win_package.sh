ARCH=$1
TSTDIR=$(mktemp -d)
LOGFILE=$(mktemp)
EXPECTED=$(mktemp)

echo -e 'Hello, world!\r' > ${EXPECTED}

cp -r release/cap32-${ARCH} ${TSTDIR}/
cd ${TSTDIR}/cap32-${ARCH}
sed -i 's/printer=0/printer=1/' cap32.cfg
echo "$WINEPATH"
wine ./cap32.exe -a "print #8,\"Hello, world!\"" -a CAP32_EXIT >> "${LOGFILE}" 2>&1

if ! diff ${EXPECTED} printer.dat
then
  echo "!! package test for ${ARCH} failed"
  cat ${LOGFILE}
  exit 1
else
  echo "The test was successful!"
  cat printer.dat
  cat ${LOGFILE}
fi
