#!/bin/bash

TSTDIR=`dirname "$0"`
OUTPUT_DIR="output"
LOGFILE="test.log"
CAP32DIR="${TSTDIR}/../../../"

rm -rv ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}
touch "${LOGFILE}"
find . -iname "*~" -exec rm -fv {} \;

cd "$TSTDIR"


$CAP32DIR/cap32 -c cap32.cfg -a 'border 13:ink 0,13:ink 1,0:mode 1:for a=1 to 24:print"Hello World",a:next:call &bd19:call 0' -a 'CAP32_WAITBREAKCAP32_SCRNSHOT CAP32_EXIT'
# Intended test when ready (doesn't work for now because \n are added automatically at the end of -a):
# $CAP32DIR/cap32 -c cap32.cfg -a 'border 13:ink 0,13:ink 1,0:mode 1:for a=1 to 24:print"Hello World",a:next:call &bd19:call 0' -a CAP32_WAITBREAK -a CAP32_SCRNSHOT -a CAP32_EXIT

# Screenshot file name is not predictible (a feature, not a bug)
mv -v ${OUTPUT_DIR}/screenshot_*png ${OUTPUT_DIR}/screenshot.png

if $DIFF -ur model ${OUTPUT_DIR} >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
