#!/bin/bash

LIB_NAME="arena"
INSTALL_PATH="/usr/local"

if [[ ! -f "$LIB_NAME.c" || ! -f "$LIB_NAME.h" ]]; then
    echo "Error: $LIB_NAME.c or .h not found in current directory."
    exit 1
fi


gcc -O3 -g -fPIC -c "$LIB_NAME.c" -o "$LIB_NAME.o"

ar rcs "lib$LIB_NAME.a" "$LIB_NAME.o"


sudo install -m 644 "$LIB_NAME.h" "$INSTALL_PATH/include/"
sudo install -m 644 "lib$LIB_NAME.a" "$INSTALL_PATH/lib/"

rm "$LIB_NAME.o" "lib$LIB_NAME.a"

echo "Done! Link with -l$LIB_NAME"
