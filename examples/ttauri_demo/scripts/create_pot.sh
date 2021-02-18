#!/bin/sh

if [ -e scripts -a -e src -a -e data -a -e resources ]
then
    echo "Extracting translation strings from project"
else
    echo "Run this script in the project root."
    exit 2
fi

find src -name \*.hpp -or -name \*.cpp >/tmp/source_files_$$.txt

xgettext \
    --files-from=/tmp/source_files_$$.txt \
    --default-domain=demo \
    --force-po \
    --output=data/ttauri_demo.pot \
    --language=C++ \
    --keyword=l10n:1g \
    --keyword=l10p:1g,1g

msginit --no-translator --input=data/ttauri_demo.pot --output-file=resources/locale/en.po
msgmerge --update resources/locale/nl.po data/ttauri_demo.pot

rm /tmp/source_files_$$.txt

