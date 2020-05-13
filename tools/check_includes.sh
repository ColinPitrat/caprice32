#!/bin/bash

uses="$(mktemp)"
includes="$(mktemp)"

token="std::string"
header="string"
grep -rlI "${token}" src/ test/ | sort -u > ${uses}
grep -rlI "include <${header}>" src/ test/ | sort -u > ${includes}

errors=$(join -v 1 "${uses}" "${includes}" | wc -l)
join -v 1 "${uses}" "${includes}" | while read line
do
  echo "${line} uses ${token} but doesn't include ${include}"
done

exit ${errors}
