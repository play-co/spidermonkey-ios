# Mozilla SpiderMonkey

Mozilla SpiderMonkey is an interpreter and JIT (Just-in-Time) Compiler for JavaScript.  Apple does not allow self-modifying app code onto the App Store, so the Game Closure SDK is based on the interpreter mode of SpiderMonkey, rather than the much faster IonMonkey engine.  We compile out the IonMonkey code during our builds.  The [Mozilla Public License](http://en.wikipedia.org/wiki/Mozilla_Public_License) is permissive enough to allow us to use it in a commercial or open-source setting as long as the modified SpiderMonkey source code is provided.  You may download the source code for our branch of SpiderMonkey [here](https://github.com/gameclosure/native-spidermonkey).

Another mature interpreter project is JavaScript Core.  Since it is licensed under the LGPL, however, it cannot be used on the iTunes App Store.  LGPL projects must be dynamically linked, which is not permitted for apps.  And several games have been regrettably pulled off the App Store as a result of using JavaScript Core.

We are branched off Mozilla FIREFOX_AURORA_18_BASE from Nov 8 at node bb4d68b03164eb7480c1a2b5a652d75c50084f18.

We removed most of the code unrelated to the JavaScript engine itself.


# Major improvements from Game Closure:

## Added a build script for generating optimized fat binaries for ARMv7 and i386

+ Added ios target to configure script:

`--with-ios-target=iPhoneOS --with-ios-version=$IOS_SDK --with-ios-min-version=$MIN_IOS_VERSION --with-ios-arch=armv7`

+ configure changes:

`-mvectorize-with-neon-quad -fprefetch-loop-arrays -mfloat-abi=hard -ftree-vectorize -march=armv7-a -mfpu=vfpv3-d16`

+ Remove JIT compiler and other unused features:

~~~
--disable-shared-js --disable-tests
--disable-ion --disable-jm --disable-tm
--enable-llvm-hacks --disable-methodjit
--disable-monoic --disable-polyic
~~~

+ GC Optimizations:

`--disable-root-analysis --disable-exact-rooting --enable-gcincremental`

+ Optimization flags:

`--enable-optimize=-O3 --with-thumb=yes --enable-strip --enable-install-strip`


## Built in NSPR for multithreaded garbage collection

+ Built and linked to NSPR for multithreading
+ `JS_THREADSAFE` = 1
+ Removed `PM_SetCurrentThreadName`, unsupported on our platform
+ Removed PRLink, same problem


## Optimized Garbage Collection for small intervals

From our code we call `MaybeGC()` in `js_tick()` every fourth frame.

+ `gcNextSmallGCTime` added
+ `GC_IDLE_SMALL_SPAN` = 1 second
+ `GC_IDLE_FULL_SPAN` = 10 seconds
+ `MaybeGC()` modified to run incremental GC periodically


## Changed the way ScriptDebugEpilogue works to allow for step-out JS debugger

+ Now will call an executeHook with null hookData instead of not at all


## Optimizations

+ `#define inline __inline__ __attribute__((always_inline))`
+ Removed assertions from critical paths
+ Replaced assembly code double-to-int conversion with integer-only C code

+ Disabled a lot of unused features in jsversion.h:

~~~
#define JS_HAS_STR_HTML_HELPERS 0       /* has str.anchor, str.bold, etc. */
#define JS_HAS_OBJ_PROTO_PROP   1       /* has o.__proto__ etc. */
#define JS_HAS_OBJ_WATCHPOINT   0       /* has o.watch and o.unwatch */
#define JS_HAS_TOSOURCE         0       /* has Object/Array toSource method */
#define JS_HAS_CATCH_GUARD      1       /* has exception handling catch guard */
#define JS_HAS_UNEVAL           0       /* has uneval() top-level function */
#define JS_HAS_CONST            1       /* has JS2 const as alternative var */
#define JS_HAS_FUN_EXPR_STMT    1       /* has function expression statement */
#define JS_HAS_NO_SUCH_METHOD   0       /* has o.__noSuchMethod__ handler */
#define JS_HAS_GENERATORS       0       /* has yield in generator function */
#define JS_HAS_BLOCK_SCOPE      0       /* has block scope via let/arraycomp */
#define JS_HAS_DESTRUCTURING    0       /* has [a,b] = ... or {p:a,q:b} = ... */
#define JS_HAS_GENERATOR_EXPRS  0       /* has (expr for (lhs in iterable)) */
#define JS_HAS_EXPR_CLOSURES    0       /* has function (formals) listexpr */
~~~

## Bugfixes

+ Fixed a null exception in MarkValueRootRange
+ `JS_EncodeStringToBuffer()` now does UTF8 conversion instead of truncation,
	which fixes Chinese/Korean text strings
+ Fixed `SlowArrayClass` so that RegEx results do not act strangely
