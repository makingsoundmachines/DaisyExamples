#!/bin/bash

START_DIR=$PWD
DAISYSP_DIR=$PWD/DaisySP

echo "building DaisySP . . ."
cd "$DAISYSP_DIR" ; make -s clean ; make -j -s
if [ $? -ne 0 ]
then
    echo "Failed to compile DaisySP"
    exit 1
fi
echo "done."

