# Mozilla SpiderMonkey

Modified for the iOS platform release of the Game Closure engine.

For a complete set of changes see the diff here: ./changes.diff

Major improvements:

+ Added a build script for generating optimized fat binaries for ARM and i386
++ Added ios target to configure script

+ Built in NSPR for multithreaded garbage collection
++ Built and linked to NSPR
++ JS_THREADSAFE = 1
++ Removed PM_SetCurrentThreadName, unsupported on our platform
++ Removed PRLink, same problem

+ Optimized Garbage Collection for small intervals
++ gcNextSmallGCTime added
++ GC_IDLE_SMALL_SPAN = 1 second
++ GC_IDLE_FULL_SPAN = 10 seconds
++ MaybeGC modified to run incremental GC periodically

+ Changed the way ScriptDebugEpilogue works to allow for step-out JS debugger
++ Now will call an executeHook with null hookData instead of not at all

+ Bugfixes
++ Fixed a null exception in MarkValueRootRange
++ JS_EncodeStringToBuffer() now does UTF8 conversion instead of truncation,
	which fixes Chinese/Korean text strings
