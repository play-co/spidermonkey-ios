#!/bin/sh

# Shell script for compiling SpiderMonkey for iPhone
#
# 1) Install autoconf 2.13 if you don't have it. You can install it with
#    port install autoconf213, or manually like so:
#    curl -O http://ftp.gnu.org/gnu/autoconf/autoconf-2.13.tar.gz
#    tar xzf autoconf-2.13.tar.gz
#    cd autoconf-2.13
#    ./configure --program-suffix=213 --prefix=/Users/nathano/.local
#    make install
#
#    Or you can use brew.  Start with "brew search autoconf" and then
#    probably "brew tap homebrew/versions" and
#    "brew install homebrew/versions/autoconf213" unless it has moved.
#
# 2) Get the SpiderMonkey source:
#    hg clone http://hg.mozilla.org/mozilla-central/
#
#    Or you can grab just the release we support:
#    http://hg.mozilla.org/mozilla-central/file/c64d97a1ebd7
#
# 3) Dump the files from this gist into mozilla-central/js/src
#
#    Or you can dump them under mozilla-central and run "patch -p1 < js.src.patch"
#
# 4) Run "./build.sh" from the ./js/src path.



# Set CFlags below:
export CAT_CFLAGS="-O3"

export CAT_SIMULATOR_CFLAGS="$CAT_CFLAGS"
export CAT_NATIVE_CFLAGS="$CAT_CFLAGS"
#export CAT_NATIVE_CFLAGS="$CAT_CFLAGS -ggdb -gdwarf-2"


if [ ! -f ./configure ]; then
#    patch -p0 -i build.patch || exit 1
    autoconf213 || exit 1
fi

# Backup the environment variables
OLD_CC=$CC
OLD_CFLAGS=$CFLAGS
OLD_CXX=$CXX
OLD_CXXFLAGS=$CXXFLAGS
OLD_LD=$LD
OLD_LDFLAGS=$LD
OLD_AR=$AR
OLD_AS=$AS
OLD_RANLIB=$RANLIB
OLD_HOST_CC=$HOST_CC
OLD_HOST_CXX=$HOST_CXX


# Build for iPhone device
make distclean
rm -f libjs_static.a.armv7
export DEVROOT=/Applications/Xcode.app/Contents/Developer//Platforms/iPhoneOS.platform/Developer/
export SDKROOT=$DEVROOT/SDKs/iPhoneOS6.0.sdk
#_local_cflags="-isysroot $SDKROOT -no-cpp-precomp -pipe -I$SDKROOT/usr/include/ -L$SDKROOT/usr/lib/"

echo "------------------------------------------------------------------------"
echo "Building for iPhone with $SDKROOT"
echo "------------------------------------------------------------------------"

export SDKCFLAGS="-isysroot $SDKROOT -no-cpp-precomp -pipe -I$SDKROOT/usr/include/ -Wall -L$SDKROOT/usr/lib -F$SDKROOT/System/Library/Frameworks -march=armv7-a $CAT_NATIVE_CFLAGS -no-integrated-as -DWTF_CPU_ARM_THUMB2=1 -ftree-vectorize -mfloat-abi=hard -ffast-math -fsingle-precision-constant"
export CC="$DEVROOT/usr/bin/arm-apple-darwin10-llvm-gcc-4.2 $SDKCFLAGS"
export CFLAGS=""
export CXX="$DEVROOT/usr/bin/arm-apple-darwin10-llvm-g++-4.2 $SDKCFLAGS -I$SDKROOT/usr/include/c++/4.2.1/"
export CXXFLAGS="$CFLAGS"
export LD="$DEVROOT/usr/bin/ld"
export LDFLAGS="-L$SDKROOT/usr/lib"
export AR="$DEVROOT/usr/bin/ar"
export AS="$DEVROOT/usr/bin/as -arch armv7"
export RANLIB="$DEVROOT/usr/bin/ranlib"
export HOST_CC="/usr/bin/gcc"
export HOST_CXX="/usr/bin/g++"

echo "Testing GCC for workingness...."
echo $CC
$CC test.c || exit 1

echo "Testing G++ for workingness...."
echo $CXX
$CXX test.cpp || exit 1

./configure \
	--disable-tracejit \
	--disable-optimize \
	--host=arm-apple-darwin \
    --disable-shared \
    --enable-static || exit 1
make -j8 || exit 1

mv libjs_static.a libjs_static.a.armv7


# Build for iPhone simulator
make distclean
rm -f libjs_static.a.i386
export DEVROOT=/Applications/Xcode.app/Contents/Developer//Platforms/iPhoneSimulator.platform/Developer/
export SDKROOT=$DEVROOT/SDKs/iPhoneSimulator6.0.sdk

echo "------------------------------------------------------------------------"
echo "Building for iPhone Simulator with $SDKROOT"
echo "------------------------------------------------------------------------"

export SDKCFLAGS="-isysroot $SDKROOT -mmacosx-version-min=10.6 -L$SDKROOT/usr/lib -no-cpp-precomp -pipe $CAT_SIMULATOR_CFLAGS"
export CC="$DEVROOT/usr/bin/i686-apple-darwin11-llvm-gcc-4.2 $SDKCFLAGS"
export CFLAGS=""
export CXX="$DEVROOT/usr/bin/i686-apple-darwin11-llvm-g++-4.2 $SDKCFLAGS"
export CXXFLAGS="$CFLAGS"
export LD="$DEVROOT/usr/bin/ld"
export LDFLAGS="-L$SDKROOT/usr/lib"
export AR="$DEVROOT/usr/bin/ar"
export AS="$DEVROOT/usr/bin/as"
export RANLIB="$DEVROOT/usr/bin/ranlib"
export HOST_CC="/usr/bin/gcc"
export HOST_CXX="/usr/bin/g++"

echo "Testing GCC for workingness...."
echo $CC
$CC test.c || exit 1

echo "Testing G++ for workingness...."
echo $CXX
$CXX test.cpp || exit 1

./configure \
	--enable-debug \
    --enable-debugger-info-modules \
    --disable-tracejit \
    --disable-shared \
	--disable-optimize \
    --enable-static || exit 1
make -j8 || exit 1

mv libjs_static.a libjs_static.a.i386


# Make a fat library
export LIPO="xcrun -sdk iphoneos lipo"

$LIPO \
    -arch armv7 libjs_static.a.armv7 \
    -arch i386 libjs_static.a.i386 \
    -create -output libjs_static.a

# Restore the environment variables
export CC=$OLD_CC
export CFLAGS=$OLD_CFLAGS
export CXX=$OLD_CXX
export CXXFLAGS=$OLD_CXXFLAGS
export LD=$OLD_LD
export LDFLAGS=$OLD_LDFLAGS
export AR=$OLD_AR
export AS=$OLD_AS
export RANLIB=$OLD_RANLIB
export HOST_CC=$OLD_HOST_CC
export HOST_CXX=$OLD_HOST_CXX
