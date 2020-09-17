#!/usr/bin/env bash

readonly MF_BASENAME="tool"

function help() {
cat<<EOF
Please execute this shell script from the root directory of "$MF_BASENAME".
EOF
exit 1
}

MF_WORKDIR=$(pwd)
current_basename=`basename $MF_WORKDIR`
test $current_basename != $MF_BASENAME && help

time_stamp=`date`
rm -rf ../../build-MF-*
rm -rf ../out
mkdir -p ../out/lib
mkdir -p ../out/bin
mkdir -p ../out/intermediates
echo $time_stamp > ../out/.mf
echo $time_stamp > ../out/lib/.mf
echo $time_stamp > ../out/bin/.mf
echo $time_stamp > ../out/intermediates/.mf
echo $time_stamp

echo "~~DONE~~"
