#!/bin/bash

#  XCodeCompileVulkanShaders.sh
#  TTauri
#
#  Created by Tjienta Vara on 2019-02-12.
#  Copyright © 2019 Pokitec. All rights reserved.

GLSLANGVALIDATOR=/usr/local/bin/glslangValidator

if [ ${SCRIPT_INPUT_FILE_COUNT} -ne ${SCRIPT_OUTPUT_FILE_COUNT} ]
then
    echo "Number of input files ${SCRIPT_INPUT_FILE_COUNT} does not equal number of output files ${SCRIPT_OUTPUT_FILE_COUNT}"
    exit 2
fi

for i in `seq 0 $[${SCRIPT_INPUT_FILE_COUNT} - 1]`
do
    INPUT_PATH_VARIABLE="SCRIPT_INPUT_FILE_${i}"
    OUTPUT_PATH_VARIABLE="SCRIPT_OUTPUT_FILE_${i}"
    INPUT_PATH="${!INPUT_PATH_VARIABLE}"
    OUTPUT_PATH="${!OUTPUT_PATH_VARIABLE}"
    INPUT_FILE=`basename "${INPUT_PATH}"`
    OUTPUT_FILE=`basename "${OUTPUT_PATH}" .spv`

    if [ "${INPUT_FILE}" != "${OUTPUT_FILE}" ]
    then
        echo "Input file ${INPUT_PATH} does not match output file ${OUTPUT_PATH}"
        exit 2
    fi

    if [ ! -f "${INPUT_PATH}" ]
    then
        echo "Could not find input file ${INPUT_PATH}"
        exit 2
    fi

    echo "Compiling ${INPUT_FILE}"
    ${GLSLANGVALIDATOR} -V -o "${OUTPUT_PATH}" "${INPUT_PATH}"
    RET=$?
    if [ ${RET} -ne 0 ]
    then
        exit ${RET}
    fi
done
exit 0
