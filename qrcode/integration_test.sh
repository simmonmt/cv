#!/bin/bash

SAMPLEDIR="qrcode/samples"

EXTRACTOR=qrcode/util/extractor

passed=1
for file in ${SAMPLEDIR}/* ; do
    echo "=== ${file} ==="
    ${EXTRACTOR} --input "${file}" >/dev/null
    [[ $? -ne 0 ]] && passed=0
done

if [[ ${passed} -eq 1 ]] ; then
    echo PASS
else
    echo FAIL
    exit 1
fi
