#!/bin/bash

if [ ! -e tools/generate_cmakelists.py ]
then
    echo "This script must be run from the HikoGUI root"
    exit 2
fi

# Check common locations for python3
if [ -x /usr/local/bin/python3 ]; then
    PYTHON3=/usr/local/bin/python3
elif [ -x /usr/bin/python3 ]; then
    PYTHON3=/usr/bin/python3
elif [ -x /bin/python3 ]; then
    PYTHON3=/bin/python3
else
    # Use 'which' to find python3 in PATH
    PYTHON3=$(which python3 2>/dev/null)
    if [ -z "$PYTHON3" ]; then
        echo "python3 not found"
        exit 1
    fi
fi

#echo "Using Python interpreter: $PYTHON3"

$PYTHON3 tools/generate_cmakelists.py
