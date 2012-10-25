spidermonkey-ios
================

This is GECKO20b5pre_20100820_RELBRANCH with some upstream fixes and patches to build for iOS.

We use Spidermonkey because this is the best JavaScript compiler and interpreter without any licensing issues.


1) Install autoconf 2.13 if you don't have it. You can install it with

```
	brew search autoconf
	brew tap homebrew/versions
```

And then probably (unless it has moved):

```
   brew install homebrew/versions/autoconf213
```


2) Run "./build.sh" from the ./js/src path.


This will build a fat binary:

```
 $ file libjs_static.a
  libjs_static.a: Mach-O universal binary with 3 architectures
  libjs_static.a (for architecture cputype (12) cpusubtype (11)):	current ar archive random library
  libjs_static.a (for architecture armv7):	current ar archive random library
  libjs_static.a (for architecture i386):	current ar archive random library
```

By default it uses the -O3 optimization flag but you can change this by editing the build script.

