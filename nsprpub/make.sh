#!/bin/sh

MIN_IOS_VERSION=4.3
IOS_SDK=6.0

LIPO="xcrun -sdk iphoneos lipo"
STRIP="xcrun -sdk iphoneos strip"


echo "------------------------------------------------------------------------"
echo "Creating arm version..."
echo "------------------------------------------------------------------------"

make clean

./configure --with-ios-target=iPhoneOS --with-ios-version=$IOS_SDK --with-ios-min-version=$MIN_IOS_VERSION --enable-pthreads --with-macos-sdk="/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator6.0.sdk" --enable-shared=no --disable-shared

make -j8

if (( $? )) ; then
    echo "error when compiling iOS version of the library"
    exit
fi

cp ./pr/src/libnspr4.a libnspr4.armv7.a

echo "------------------------------------------------------------------------"
echo "Creating i386 version..."
echo "------------------------------------------------------------------------"

make clean

./configure --with-ios-target=iPhoneSimulator --with-ios-version=$IOS_SDK --with-ios-min-version=$MIN_IOS_VERSION --enable-pthreads --with-macos-sdk="/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator6.0.sdk" --enable-shared=no --disable-shared

make -j8

if (( $? )) ; then
    echo "error when compiling iOS version of the library"
    exit
fi

cp ./pr/src/libnspr4.a libnspr4.i386.a

if [ -e libnspr4.i386.a ]  && [ -e libnspr4.armv7.a ] ; then
    echo "creating fat version of the library"
    $LIPO -create -output libnspr4.a libnspr4.i386.a libnspr4.armv7.a
    # remove debugging info
    $STRIP -S libnspr4.a
    $LIPO -info libnspr4.a

	cp ./libnspr4.a ./dist/lib/libnspr4.a
fi

