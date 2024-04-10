#!/bin/bash

TMPDIR=tmp_au_units
UNITS="feet inches meters miles" 

if [ ! -e tools/generate_au_units.sh ]
then
    echo "This script must be run from the HikoGUI root"
    exit 2
fi

PYTHON3=python
if [ -e /usr/local/bin/python3 ]
then
    PYTHON3=/usr/local/bin/python3
fi

if [ -e $TMPDIR ]
then
    rm -fr $TMPDIR
fi
git clone --depth 1 https://github.com/aurora-opensource/au.git $TMPDIR

(cd $TMPDIR ; $PYTHON3 tools/bin/make-single-file --units $UNITS) >src/hikothird/au.hh

rm -fr $TMPDIR

