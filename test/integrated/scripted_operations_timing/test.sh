#!/bin/bash

TSTDIR=`dirname "$0"`
LOGFILE="test.log"
CAP32DIR="$TSTDIR/../../../"

#export SDL_VIDEODRIVER=dummy
cd "$TSTDIR"
touch "${LOGFILE}"

find . -iname "*~" -exec rm -fv {} \;

rm -rv output
mkdir -p output

$CAP32DIR/cap32 -c cap32.cfg -a 'border 13:ink 0,13:ink 1,0:mode 1:for a=1 to 24:print"Hello World",a:next:call &bd19:call 0' -a 'CAP32_WAITBREAKCAP32_SCRNSHOTCAP32_EXIT'

# Intended test when ready:
# $CAP32DIR/cap32 -c cap32.cfg -a 'border 13:ink 0,13:ink 1,0:mode 1:for a=1 to 24:print"Hello World",a:next:call &bd19:call 0' -a CAP32_WAITBREAK -a CAP32_SCRNSHOT -a CAP32_EXIT
# Artifcially pass test until code is correct:
# $CAP32DIR/cap32 -c cap32.cfg -a 'border 13:ink 0,13:ink 1,0:mode 1:for a=1 to 24:print"Hello World",a:next:call &bd19:poke 0,&c9:call 0:cat' -a CAP32_WAITBREAK -a CAP32_SCRNSHOT -a CAP32_EXIT

# Screenshot file name is not reproducible (a features, not a bug), TODO have upstream code offer an option?
mv -v output/screenshot_*png output/screenshot.png

if diff -ur model output >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
