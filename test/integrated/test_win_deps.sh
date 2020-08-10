# The idea of this test is to ensure no new DLL dependency is added involuntarly.
# All needed DLLs should be included in the archive.
ARCH=$1
ACTUAL=$(mktemp)
EXPECTED=$(mktemp)

cat << EOF > ${EXPECTED}
I don't know yet
EOF

dumpbin /dependents release/cap32-${ARCH} > ${ACTUAL}

if ! diff ${ACTUAL} ${EXPECTED}
then
  echo "!! package test for ${ARCH} failed"
  cat ${LOGFILE}
  exit 1
else
  echo "The test was successful!"
  cat printer.dat
  cat ${LOGFILE}
fi
