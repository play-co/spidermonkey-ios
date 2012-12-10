#!/bin/sh

MIN_IOS_VERSION=4.3
IOS_SDK=6.0


make clean

echo "------------------------------------------------------------------------"
echo "Creating i386 version..."
echo "------------------------------------------------------------------------"

#autoconf

# create i386 version (simulator)
./configure --with-ios-target=iPhoneSimulator --with-ios-version=$IOS_SDK --with-ios-min-version=$MIN_IOS_VERSION --enable-pthreads --with-macos-sdk="/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator6.0.sdk"

make clean

make

