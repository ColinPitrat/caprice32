#!/bin/bash

uses="$(mktemp)"
includes="$(mktemp)"

declare -A token_to_header
# TODO: Extend this to vector, list and more ...
token_to_header=( ["std::string"]="string" ["std::map"]="map" )

errors=0
for token in "${!token_to_header[@]}"
do
  header=${token_to_header[${token}]}
  echo "Looking for ${token} and ${header}"
  grep -rlI "${token}" src/ test/ | sort -u > ${uses}
  grep -rlI "include <${header}>" src/ test/ | sort -u > ${includes}

  this_errors=$(join -v 1 "${uses}" "${includes}" | wc -l)
  let errors=${errors}+${this_errors}
  join -v 1 "${uses}" "${includes}" | while read line
  do
    echo "${line} uses ${token} but doesn't include ${header}"
  done
done

exit ${errors}
