#!/bin/bash
# =========================================================== #
# This shell script calls on the make to build the Compressor
# executable. 
# =========================================================== #


make -s
if [ $? -eq 0 ]; then
 echo "BUILD SUCCESSFULL"
else
 echo "BUILD FAILED"
fi
