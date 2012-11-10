# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

def normalize_suffix(suffix):
    '''Returns a normalized suffix, i.e. ensures it starts with a dot and
    doesn't starts or ends with whitespace characters'''
    value = suffix.strip()
    if len(value) and not value.startswith('.'):
        value = '.' + value
    return value

# Variables from the build system
AR = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar"
AR_EXTRACT = "$(AR) x".replace('$(AR)', AR)
DLL_PREFIX = "lib"
LIB_PREFIX = "lib"
OBJ_SUFFIX = normalize_suffix("o")
LIB_SUFFIX = normalize_suffix("a")
DLL_SUFFIX = normalize_suffix(".dylib")
IMPORT_LIB_SUFFIX = normalize_suffix("")
LIBS_DESC_SUFFIX = normalize_suffix("desc")
EXPAND_LIBS_LIST_STYLE = "list"
EXPAND_LIBS_ORDER_STYLE = ""
LD_PRINT_ICF_SECTIONS = ""
