#!/bin/bash

TSTDIR=`dirname "$0"`
OUTPUT_DIR="output"
LOGFILE="test.log"
CAP32DIR="${TSTDIR}/../../../"

if [ -z "$DIFF" ]
then
  DIFF=diff
fi

if [ -z "$SED" ]
then
  SED=sed
fi

rm -rvf ${OUTPUT_DIR}
mkdir -p ${OUTPUT_DIR}
echo "" > "${LOGFILE}"

cd "$TSTDIR"
export SDL_VIDEODRIVER=dummy
export SDL_AUDIODRIVER=disk

FREQS=("11kHz" "22kHz" "44kHz" "48kHz" "96kHz")
BITS=("8bits" "16bits")
CHANNELS=("mono" "stereo")
VOLUMES=(0 10 50 80 100)

FFMPEG_FREQS=(11025 22050 44100 48000 96000)
FFMPEG_BITS=("s8" "s16le")
FFMPEG_CHANS=(1 2)

# The cardinality is too big to test everything.
# Limit ourselves to a representative subset:
TEST_CASES=(
  # 11kHz 8bits mono at 50%
  "0.0.0.2"
  # 22kHz 8bits mono at 80%
  "1.0.0.3"
  # 44kHz 16bits mono at 10%
  "2.1.0.1"
  # 44kHz 16bits stereo at 100%
  "2.1.1.4"
  # 48kHz 16bits stereo at 80%
  "3.1.1.3"
  # 96kHz 16bits stereo at 80%
  "4.1.1.3"
  # 44kHz 16bits mono at 0%
  "4.1.0.0"
)

rc=0
# If we were testing all combination, this would look like this, and would run 100 tests:
#for freq in $(seq 0 $((${#FREQS[@]}-1)))
#for bits in $(seq 0 $((${#BITS[@]}-1)))
#for channels in $(seq 0 $((${#CHANNELS[@]}-1)))
#for volume in $(seq 0 $((${#VOLUMES[@]}-1)))

for tc in ${TEST_CASES[@]}
do
  freq=$(echo $tc | cut -d . -f 1)
  bits=$(echo $tc | cut -d . -f 2)
  channels=$(echo $tc | cut -d . -f 3)
  volume=$(echo $tc | cut -d . -f 4)

  ${SED} -i "s/^playback_rate=.*/playback_rate=$freq/" cap32.cfg
  ${SED} -i "s/^bits=.*/bits=$freq/" cap32.cfg
  ${SED} -i "s/^stereo=.*/stereo=$channels/" cap32.cfg
  ${SED} -i "s/^volume=.*/volume=$volume/" cap32.cfg

  test_case=${FREQS[$freq]}_${BITS[$bits]}_${CHANNELS[$channels]}_${VOLUMES[$volume]}
  filename_base=sdlaudio_${test_case}
  output_raw=output/${filename_base}.raw
  output_wav=output/${filename_base}.wav
  expected_raw=model/${filename_base}.raw
  expected_wav=model/${filename_base}.wav
  export SDL_DISKAUDIOFILE=${PWD}/${output_raw}

  $CAP32DIR/cap32 -c cap32.cfg -a "run\"scale" -a CAP32_WAITBREAK -a CAP32_EXIT scale.zip >> ${LOGFILE} 2>&1

  ffmpeg -f ${FFMPEG_BITS[$bits]} -ar ${FFMPEG_FREQS[$freq]} -ac ${FFMPEG_CHANS[$channels]} -i ${output_raw} \
    -af "silenceremove=start_periods=1:start_threshold=-60dB:stop_periods=-1:stop_threshold=-60dB" \
    -c:a pcm_s16le -y ${output_wav} >> ${LOGFILE} 2>&1

  # Hexdump, which will ignore repeated lines, in particular the start of 0s which
  # can have variable length. Then remove the address as they will vary too.
  hexdump -C ${expected_raw} | ${SED} 's/^[0-9a-fA-F]* *//' > ${expected_raw}.hex
  hexdump -C ${output_raw} | ${SED} 's/^[0-9a-fA-F]* *//' > ${output_raw}.hex

  if $DIFF ${output_raw}.hex ${expected_raw}.hex >> "${LOGFILE}"
  then
    echo " $test_case (hex): PASS"
  else
    echo " $test_case (hex): FAIL"
  fi

  if $DIFF ${output_wav} ${expected_wav} >> "${LOGFILE}"
  then
    echo " $test_case (wav): PASS"
  else
    echo " $test_case (wav): FAIL"
    rc=1
  fi
done

if [ $rc -ne 0 ]
then
  cat "${LOGFILE}"
fi

exit $rc
