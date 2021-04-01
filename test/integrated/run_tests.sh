#!/bin/bash

OK=0
TOTAL=0

echo "Starting integrated tests"

DIFF=diff
if [ -f "/c/cygwin/bin/diff.exe" ]
then
	DIFF=/c/cygwin/bin/diff.exe
fi
export DIFF=$DIFF
echo "Using diff: $DIFF"

# TODO(cpitrat): Revert this, stuff in dsk/test.sh and src/z80.cpp after finished investigating
#for tst in */test.sh
for tst in dsk/test.sh
do
  TSTDIR=`dirname "${tst}"`
  TSTNAME=`basename "$TSTDIR"`
  pushd "${TSTDIR}" >/dev/null

  echo ""
  echo " ** Running ${TSTNAME}: "
  BEGIN=`date +%s`
  result="FAIL"
  if sh test.sh
  then
    let OK=$OK+1
    result="PASS"
  fi
  END=`date +%s`
  let TIMING=$END-$BEGIN
  echo " => ${TSTNAME}: ${result} (${TIMING}s)"

  let TOTAL=$TOTAL+1
  popd >/dev/null
done

echo ""
echo -n "Integrated tests result: "
if [ $TOTAL -eq $OK ]
then
  echo "PASSED"
  exit 0
else
  echo "FAILED ($((100*$OK/$TOTAL))%)"
  exit 1
fi
