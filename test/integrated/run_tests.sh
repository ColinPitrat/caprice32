#!/bin/sh

OK=0
TOTAL=0

for tst in */test.sh
do
  TSTDIR=`dirname "${tst}"`
  TSTNAME=`basename "$TSTDIR"`
  pushd "${TSTDIR}" >/dev/null

  echo -n "${TSTNAME}: "
  BEGIN=`date +%s`
  if sh test.sh
  then
    let OK=$OK+1
    echo -n "PASS"
  else
    echo -n "FAIL"
  fi
  END=`date +%s`
  let TIMING=$END-$BEGIN
  echo " (${TIMING}s)"

  let TOTAL=$TOTAL+1
  popd >/dev/null
done

echo -n "Integrated tests result: "
if [ $TOTAL -eq $OK ]
then
  echo "PASSED"
  exit 0
else
  echo "FAILED ($((100*$OK/$TOTAL))%)"
  exit 1
fi
