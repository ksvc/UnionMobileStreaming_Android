#!/usr/bin/env bash

echo "====================== copy libs  ========"
rm -rf libunionstreamer/libs
cp -p -r prebuilt/libs libunionstreamer/

echo "====================== build ndk  ========"
MODULES="fdkaacencoder x264encoder librtmppub"
for MODULE in $MODULES
do
cd $MODULE
ndk-build clean
ndk-build -j4
cd ..
done