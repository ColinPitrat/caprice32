#!/bin/bash

TSTDIR=`dirname "$0"`
OUTPUT_DIR="output"
LOGFILE="test.log"
CAP32DIR="${TSTDIR}/../../../"

rm -rv ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}
echo "" > "${LOGFILE}"

cd "$TSTDIR"
export SDL_VIDEODRIVER=dummy
export SDL_AUDIODRIVER=disk
export SDL_DISKAUDIOFILE=output/sdlaudio.raw

$CAP32DIR/cap32 -c cap32.cfg -a "run\"scale" -a CAP32_WAITBREAK -a CAP32_EXIT scale.zip

# Hexdump, which will ignore repeated lines, in particular the start of 0s which
# can have variable length. Then remove the address as they will vary too.
hexdump -C expected.raw | sed 's/^[0-9a-fA-F]* *//' > expected.raw.hex
hexdump -C output/sdlaudio.raw | sed 's/^[0-9a-fA-F]* *//' > output/sdlaudio.raw.hex

if $DIFF output/sdlaudio.raw.hex expected.raw.hex >> "${LOGFILE}"
then
  exit 0
else
  cat "${LOGFILE}"
  exit 1
fi
