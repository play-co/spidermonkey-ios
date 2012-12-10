#!/bin/sh

## this script is supposed to be run one directory below the original configure script
## usually in build-ios

MIN_IOS_VERSION=4.3
IOS_SDK=6.0

LIPO="xcrun -sdk iphoneos lipo"
STRIP="xcrun -sdk iphoneos strip"

make clean

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

NSPR_PREFIX="$CWD/../../nsprpub/dist/"
NSPR_INCL="$NSPR_PREFIX/include/nspr/"
NSPR_LIB="$CWD/../../nsprpub/"

echo "------------------------------------------------------------------------"
echo "Creating arm version..."
echo "------------------------------------------------------------------------"

# create ios version (armv7)
./configure --with-ios-target=iPhoneOS --with-ios-version=$IOS_SDK --with-ios-min-version=$MIN_IOS_VERSION --with-ios-arch=armv7  --disable-shared-js --disable-tests --disable-ion --disable-jm --disable-tm --enable-llvm-hacks --disable-methodjit --with-thumb=yes --enable-strip --enable-install-strip --disable-monoic --disable-polyic --disable-root-analysis --disable-exact-rooting --enable-gcincremental --enable-optimize=-O3 --enable-threadsafe --with-nspr-cflags="-I$NSPR_INCL" --with-nspr-prefix=$NSPR_PREFIX --with-nspr-libs="$NSPR_LIB/libnspr4.a"

make -j8

if (( $? )) ; then
    echo "error when compiling iOS version of the library"
    exit
fi

mv libjs_static.a libjs_static.armv7.a

echo "------------------------------------------------------------------------"
echo "Creating i386 version..."
echo "------------------------------------------------------------------------"

# create i386 version (simulator)
./configure --with-ios-target=iPhoneSimulator --with-ios-version=$IOS_SDK --with-ios-min-version=$MIN_IOS_VERSION --disable-shared-js --disable-tests --disable-ion --disable-jm --disable-tm --enable-llvm-hacks --disable-methodjit --disable-strip --disable-install-strip --disable-monoic --disable-polyic --disable-root-analysis --disable-exact-rooting --enable-gcincremental --enable-optimize=-O3 --enable-threadsafe --with-nspr-cflags="-I$NSPR_INCL" --with-nspr-prefix=$NSPR_PREFIX --with-nspr-libs="$NSPR_LIB/libnspr4.a"

make -j8

if (( $? )) ; then
    echo "error when compiling i386 (iOS Simulator) version of the library"
    exit
fi

mv libjs_static.a libjs_static.i386.a

if [ -e libjs_static.i386.a ]  && [ -e libjs_static.armv7.a ] ; then
    echo "creating fat version of the library"
    $LIPO -create -output libjs_static.a libjs_static.i386.a libjs_static.armv7.a
    # remove debugging info
    $STRIP -S libjs_static.a
    $LIPO -info libjs_static.a
fi

echo "*** DONE ***"
echo "If you want to use spidermonkey, copy the 'dist' directory to some accesible place"
echo "e.g. 'cp -pr dist ~/path/to/your/project'"
echo "and then add the proper search paths for headers and libraries in your Xcode project"

cp libjs_static.a ../..
