r"""Wrapper for wdissector.h

Generated with:
/usr/local/bin/ctypesgen -DGEN_PY_MODULE /home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/src/wdissector.h -Lbin -lbin/libwdissector.so -o /home/asset/Desktop/work/wireless-deep-fuzzer-zigbee/bindings/python/wdissector.py

Do not modify this file.
"""

__docformat__ = "restructuredtext"

# Begin preamble for Python v(3, 2)

import ctypes, os, sys
from ctypes import *

_int_types = (c_int16, c_int32)
if hasattr(ctypes, "c_int64"):
    # Some builds of ctypes apparently do not have c_int64
    # defined; it's a pretty good bet that these builds do not
    # have 64-bit pointers.
    _int_types += (c_int64,)
for t in _int_types:
    if sizeof(t) == sizeof(c_size_t):
        c_ptrdiff_t = t
del t
del _int_types


class UserString:
    def __init__(self, seq):
        if isinstance(seq, bytes):
            self.data = seq
        elif isinstance(seq, UserString):
            self.data = seq.data[:]
        else:
            self.data = str(seq).encode()

    def __bytes__(self):
        return self.data

    def __str__(self):
        return self.data.decode()

    def __repr__(self):
        return repr(self.data)

    def __int__(self):
        return int(self.data.decode())

    def __long__(self):
        return int(self.data.decode())

    def __float__(self):
        return float(self.data.decode())

    def __complex__(self):
        return complex(self.data.decode())

    def __hash__(self):
        return hash(self.data)

    def __cmp__(self, string):
        if isinstance(string, UserString):
            return cmp(self.data, string.data)
        else:
            return cmp(self.data, string)

    def __le__(self, string):
        if isinstance(string, UserString):
            return self.data <= string.data
        else:
            return self.data <= string

    def __lt__(self, string):
        if isinstance(string, UserString):
            return self.data < string.data
        else:
            return self.data < string

    def __ge__(self, string):
        if isinstance(string, UserString):
            return self.data >= string.data
        else:
            return self.data >= string

    def __gt__(self, string):
        if isinstance(string, UserString):
            return self.data > string.data
        else:
            return self.data > string

    def __eq__(self, string):
        if isinstance(string, UserString):
            return self.data == string.data
        else:
            return self.data == string

    def __ne__(self, string):
        if isinstance(string, UserString):
            return self.data != string.data
        else:
            return self.data != string

    def __contains__(self, char):
        return char in self.data

    def __len__(self):
        return len(self.data)

    def __getitem__(self, index):
        return self.__class__(self.data[index])

    def __getslice__(self, start, end):
        start = max(start, 0)
        end = max(end, 0)
        return self.__class__(self.data[start:end])

    def __add__(self, other):
        if isinstance(other, UserString):
            return self.__class__(self.data + other.data)
        elif isinstance(other, bytes):
            return self.__class__(self.data + other)
        else:
            return self.__class__(self.data + str(other).encode())

    def __radd__(self, other):
        if isinstance(other, bytes):
            return self.__class__(other + self.data)
        else:
            return self.__class__(str(other).encode() + self.data)

    def __mul__(self, n):
        return self.__class__(self.data * n)

    __rmul__ = __mul__

    def __mod__(self, args):
        return self.__class__(self.data % args)

    # the following methods are defined in alphabetical order:
    def capitalize(self):
        return self.__class__(self.data.capitalize())

    def center(self, width, *args):
        return self.__class__(self.data.center(width, *args))

    def count(self, sub, start=0, end=sys.maxsize):
        return self.data.count(sub, start, end)

    def decode(self, encoding=None, errors=None):  # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.decode(encoding, errors))
            else:
                return self.__class__(self.data.decode(encoding))
        else:
            return self.__class__(self.data.decode())

    def encode(self, encoding=None, errors=None):  # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.encode(encoding, errors))
            else:
                return self.__class__(self.data.encode(encoding))
        else:
            return self.__class__(self.data.encode())

    def endswith(self, suffix, start=0, end=sys.maxsize):
        return self.data.endswith(suffix, start, end)

    def expandtabs(self, tabsize=8):
        return self.__class__(self.data.expandtabs(tabsize))

    def find(self, sub, start=0, end=sys.maxsize):
        return self.data.find(sub, start, end)

    def index(self, sub, start=0, end=sys.maxsize):
        return self.data.index(sub, start, end)

    def isalpha(self):
        return self.data.isalpha()

    def isalnum(self):
        return self.data.isalnum()

    def isdecimal(self):
        return self.data.isdecimal()

    def isdigit(self):
        return self.data.isdigit()

    def islower(self):
        return self.data.islower()

    def isnumeric(self):
        return self.data.isnumeric()

    def isspace(self):
        return self.data.isspace()

    def istitle(self):
        return self.data.istitle()

    def isupper(self):
        return self.data.isupper()

    def join(self, seq):
        return self.data.join(seq)

    def ljust(self, width, *args):
        return self.__class__(self.data.ljust(width, *args))

    def lower(self):
        return self.__class__(self.data.lower())

    def lstrip(self, chars=None):
        return self.__class__(self.data.lstrip(chars))

    def partition(self, sep):
        return self.data.partition(sep)

    def replace(self, old, new, maxsplit=-1):
        return self.__class__(self.data.replace(old, new, maxsplit))

    def rfind(self, sub, start=0, end=sys.maxsize):
        return self.data.rfind(sub, start, end)

    def rindex(self, sub, start=0, end=sys.maxsize):
        return self.data.rindex(sub, start, end)

    def rjust(self, width, *args):
        return self.__class__(self.data.rjust(width, *args))

    def rpartition(self, sep):
        return self.data.rpartition(sep)

    def rstrip(self, chars=None):
        return self.__class__(self.data.rstrip(chars))

    def split(self, sep=None, maxsplit=-1):
        return self.data.split(sep, maxsplit)

    def rsplit(self, sep=None, maxsplit=-1):
        return self.data.rsplit(sep, maxsplit)

    def splitlines(self, keepends=0):
        return self.data.splitlines(keepends)

    def startswith(self, prefix, start=0, end=sys.maxsize):
        return self.data.startswith(prefix, start, end)

    def strip(self, chars=None):
        return self.__class__(self.data.strip(chars))

    def swapcase(self):
        return self.__class__(self.data.swapcase())

    def title(self):
        return self.__class__(self.data.title())

    def translate(self, *args):
        return self.__class__(self.data.translate(*args))

    def upper(self):
        return self.__class__(self.data.upper())

    def zfill(self, width):
        return self.__class__(self.data.zfill(width))


class MutableString(UserString):
    """mutable string objects

    Python strings are immutable objects.  This has the advantage, that
    strings may be used as dictionary keys.  If this property isn't needed
    and you insist on changing string values in place instead, you may cheat
    and use MutableString.

    But the purpose of this class is an educational one: to prevent
    people from inventing their own mutable string class derived
    from UserString and than forget thereby to remove (override) the
    __hash__ method inherited from UserString.  This would lead to
    errors that would be very hard to track down.

    A faster and better solution is to rewrite your program using lists."""

    def __init__(self, string=""):
        self.data = string

    def __hash__(self):
        raise TypeError("unhashable type (it is mutable)")

    def __setitem__(self, index, sub):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data):
            raise IndexError
        self.data = self.data[:index] + sub + self.data[index + 1 :]

    def __delitem__(self, index):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data):
            raise IndexError
        self.data = self.data[:index] + self.data[index + 1 :]

    def __setslice__(self, start, end, sub):
        start = max(start, 0)
        end = max(end, 0)
        if isinstance(sub, UserString):
            self.data = self.data[:start] + sub.data + self.data[end:]
        elif isinstance(sub, bytes):
            self.data = self.data[:start] + sub + self.data[end:]
        else:
            self.data = self.data[:start] + str(sub).encode() + self.data[end:]

    def __delslice__(self, start, end):
        start = max(start, 0)
        end = max(end, 0)
        self.data = self.data[:start] + self.data[end:]

    def immutable(self):
        return UserString(self.data)

    def __iadd__(self, other):
        if isinstance(other, UserString):
            self.data += other.data
        elif isinstance(other, bytes):
            self.data += other
        else:
            self.data += str(other).encode()
        return self

    def __imul__(self, n):
        self.data *= n
        return self


class String(MutableString, Union):

    _fields_ = [("raw", POINTER(c_char)), ("data", c_char_p)]

    def __init__(self, obj=""):
        if isinstance(obj, (bytes, UserString)):
            self.data = bytes(obj)
        else:
            self.raw = obj

    def __len__(self):
        return self.data and len(self.data) or 0

    def from_param(cls, obj):
        # Convert None or 0
        if obj is None or obj == 0:
            return cls(POINTER(c_char)())

        # Convert from String
        elif isinstance(obj, String):
            return obj

        # Convert from bytes
        elif isinstance(obj, bytes):
            return cls(obj)

        # Convert from str
        elif isinstance(obj, str):
            return cls(obj.encode())

        # Convert from c_char_p
        elif isinstance(obj, c_char_p):
            return obj

        # Convert from POINTER(c_char)
        elif isinstance(obj, POINTER(c_char)):
            return obj

        # Convert from raw pointer
        elif isinstance(obj, int):
            return cls(cast(obj, POINTER(c_char)))

        # Convert from c_char array
        elif isinstance(obj, c_char * len(obj)):
            return obj

        # Convert from object
        else:
            return String.from_param(obj._as_parameter_)

    from_param = classmethod(from_param)


def ReturnString(obj, func=None, arguments=None):
    return String.from_param(obj)


# As of ctypes 1.0, ctypes does not support custom error-checking
# functions on callbacks, nor does it support custom datatypes on
# callbacks, so we must ensure that all callbacks return
# primitive datatypes.
#
# Non-primitive return values wrapped with UNCHECKED won't be
# typechecked, and will be converted to c_void_p.
def UNCHECKED(type):
    if hasattr(type, "_type_") and isinstance(type._type_, str) and type._type_ != "P":
        return type
    else:
        return c_void_p


# ctypes doesn't have direct support for variadic functions, so we have to write
# our own wrapper class
class _variadic_function(object):
    def __init__(self, func, restype, argtypes, errcheck):
        self.func = func
        self.func.restype = restype
        self.argtypes = argtypes
        if errcheck:
            self.func.errcheck = errcheck

    def _as_parameter_(self):
        # So we can pass this variadic function as a function pointer
        return self.func

    def __call__(self, *args):
        fixed_args = []
        i = 0
        for argtype in self.argtypes:
            # Typecheck what we can
            fixed_args.append(argtype.from_param(args[i]))
            i += 1
        return self.func(*fixed_args + list(args[i:]))


def ord_if_char(value):
    """
    Simple helper used for casts to simple builtin types:  if the argument is a
    string type, it will be converted to it's ordinal value.

    This function will raise an exception if the argument is string with more
    than one characters.
    """
    return ord(value) if (isinstance(value, bytes) or isinstance(value, str)) else value

# End preamble

_libs = {}
_libdirs = ['bin']

# Begin loader

# ----------------------------------------------------------------------------
# Copyright (c) 2008 David James
# Copyright (c) 2006-2008 Alex Holkner
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of pyglet nor the names of its
#    contributors may be used to endorse or promote products
#    derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

import os.path, re, sys, glob
import platform
import ctypes
import ctypes.util


def _environ_path(name):
    if name in os.environ:
        return os.environ[name].split(":")
    else:
        return []


class LibraryLoader(object):
    # library names formatted specifically for platforms
    name_formats = ["%s"]

    class Lookup(object):
        mode = ctypes.DEFAULT_MODE

        def __init__(self, path):
            super(LibraryLoader.Lookup, self).__init__()
            self.access = dict(cdecl=ctypes.CDLL(path, self.mode))

        def get(self, name, calling_convention="cdecl"):
            if calling_convention not in self.access:
                raise LookupError(
                    "Unknown calling convention '{}' for function '{}'".format(
                        calling_convention, name
                    )
                )
            return getattr(self.access[calling_convention], name)

        def has(self, name, calling_convention="cdecl"):
            if calling_convention not in self.access:
                return False
            return hasattr(self.access[calling_convention], name)

        def __getattr__(self, name):
            return getattr(self.access["cdecl"], name)

    def __init__(self):
        self.other_dirs = []

    def __call__(self, libname):
        """Given the name of a library, load it."""
        paths = self.getpaths(libname)

        for path in paths:
            try:
                return self.Lookup(path)
            except:
                pass

        raise ImportError("Could not load %s." % libname)

    def getpaths(self, libname):
        """Return a list of paths where the library might be found."""
        if os.path.isabs(libname):
            yield libname
        else:
            # search through a prioritized series of locations for the library

            # we first search any specific directories identified by user
            for dir_i in self.other_dirs:
                for fmt in self.name_formats:
                    # dir_i should be absolute already
                    yield os.path.join(dir_i, fmt % libname)

            # then we search the directory where the generated python interface is stored
            for fmt in self.name_formats:
                yield os.path.abspath(os.path.join(os.path.dirname(__file__), fmt % libname))

            # now, use the ctypes tools to try to find the library
            for fmt in self.name_formats:
                path = ctypes.util.find_library(fmt % libname)
                if path:
                    yield path

            # then we search all paths identified as platform-specific lib paths
            for path in self.getplatformpaths(libname):
                yield path

            # Finally, we'll try the users current working directory
            for fmt in self.name_formats:
                yield os.path.abspath(os.path.join(os.path.curdir, fmt % libname))

    def getplatformpaths(self, libname):
        return []


# Darwin (Mac OS X)


class DarwinLibraryLoader(LibraryLoader):
    name_formats = [
        "lib%s.dylib",
        "lib%s.so",
        "lib%s.bundle",
        "%s.dylib",
        "%s.so",
        "%s.bundle",
        "%s",
    ]

    class Lookup(LibraryLoader.Lookup):
        # Darwin requires dlopen to be called with mode RTLD_GLOBAL instead
        # of the default RTLD_LOCAL.  Without this, you end up with
        # libraries not being loadable, resulting in "Symbol not found"
        # errors
        mode = ctypes.RTLD_GLOBAL

    def getplatformpaths(self, libname):
        if os.path.pathsep in libname:
            names = [libname]
        else:
            names = [format % libname for format in self.name_formats]

        for dir in self.getdirs(libname):
            for name in names:
                yield os.path.join(dir, name)

    def getdirs(self, libname):
        """Implements the dylib search as specified in Apple documentation:

        http://developer.apple.com/documentation/DeveloperTools/Conceptual/
            DynamicLibraries/Articles/DynamicLibraryUsageGuidelines.html

        Before commencing the standard search, the method first checks
        the bundle's ``Frameworks`` directory if the application is running
        within a bundle (OS X .app).
        """

        dyld_fallback_library_path = _environ_path("DYLD_FALLBACK_LIBRARY_PATH")
        if not dyld_fallback_library_path:
            dyld_fallback_library_path = [os.path.expanduser("~/lib"), "/usr/local/lib", "/usr/lib"]

        dirs = []

        if "/" in libname:
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
        else:
            dirs.extend(_environ_path("LD_LIBRARY_PATH"))
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))

        if hasattr(sys, "frozen") and sys.frozen == "macosx_app":
            dirs.append(os.path.join(os.environ["RESOURCEPATH"], "..", "Frameworks"))

        dirs.extend(dyld_fallback_library_path)

        return dirs


# Posix


class PosixLibraryLoader(LibraryLoader):
    _ld_so_cache = None

    _include = re.compile(r"^\s*include\s+(?P<pattern>.*)")

    class _Directories(dict):
        def __init__(self):
            self.order = 0

        def add(self, directory):
            if len(directory) > 1:
                directory = directory.rstrip(os.path.sep)
            # only adds and updates order if exists and not already in set
            if not os.path.exists(directory):
                return
            o = self.setdefault(directory, self.order)
            if o == self.order:
                self.order += 1

        def extend(self, directories):
            for d in directories:
                self.add(d)

        def ordered(self):
            return (i[0] for i in sorted(self.items(), key=lambda D: D[1]))

    def _get_ld_so_conf_dirs(self, conf, dirs):
        """
        Recursive funtion to help parse all ld.so.conf files, including proper
        handling of the `include` directive.
        """

        try:
            with open(conf) as f:
                for D in f:
                    D = D.strip()
                    if not D:
                        continue

                    m = self._include.match(D)
                    if not m:
                        dirs.add(D)
                    else:
                        for D2 in glob.glob(m.group("pattern")):
                            self._get_ld_so_conf_dirs(D2, dirs)
        except IOError:
            pass

    def _create_ld_so_cache(self):
        # Recreate search path followed by ld.so.  This is going to be
        # slow to build, and incorrect (ld.so uses ld.so.cache, which may
        # not be up-to-date).  Used only as fallback for distros without
        # /sbin/ldconfig.
        #
        # We assume the DT_RPATH and DT_RUNPATH binary sections are omitted.

        directories = self._Directories()
        for name in (
            "LD_LIBRARY_PATH",
            "SHLIB_PATH",  # HPUX
            "LIBPATH",  # OS/2, AIX
            "LIBRARY_PATH",  # BE/OS
        ):
            if name in os.environ:
                directories.extend(os.environ[name].split(os.pathsep))

        self._get_ld_so_conf_dirs("/etc/ld.so.conf", directories)

        bitage = platform.architecture()[0]

        unix_lib_dirs_list = []
        if bitage.startswith("64"):
            # prefer 64 bit if that is our arch
            unix_lib_dirs_list += ["/lib64", "/usr/lib64"]

        # must include standard libs, since those paths are also used by 64 bit
        # installs
        unix_lib_dirs_list += ["/lib", "/usr/lib"]
        if sys.platform.startswith("linux"):
            # Try and support multiarch work in Ubuntu
            # https://wiki.ubuntu.com/MultiarchSpec
            if bitage.startswith("32"):
                # Assume Intel/AMD x86 compat
                unix_lib_dirs_list += ["/lib/i386-linux-gnu", "/usr/lib/i386-linux-gnu"]
            elif bitage.startswith("64"):
                # Assume Intel/AMD x86 compat
                unix_lib_dirs_list += ["/lib/x86_64-linux-gnu", "/usr/lib/x86_64-linux-gnu"]
            else:
                # guess...
                unix_lib_dirs_list += glob.glob("/lib/*linux-gnu")
        directories.extend(unix_lib_dirs_list)

        cache = {}
        lib_re = re.compile(r"lib(.*)\.s[ol]")
        ext_re = re.compile(r"\.s[ol]$")
        for dir in directories.ordered():
            try:
                for path in glob.glob("%s/*.s[ol]*" % dir):
                    file = os.path.basename(path)

                    # Index by filename
                    cache_i = cache.setdefault(file, set())
                    cache_i.add(path)

                    # Index by library name
                    match = lib_re.match(file)
                    if match:
                        library = match.group(1)
                        cache_i = cache.setdefault(library, set())
                        cache_i.add(path)
            except OSError:
                pass

        self._ld_so_cache = cache

    def getplatformpaths(self, libname):
        if self._ld_so_cache is None:
            self._create_ld_so_cache()

        result = self._ld_so_cache.get(libname, set())
        for i in result:
            # we iterate through all found paths for library, since we may have
            # actually found multiple architectures or other library types that
            # may not load
            yield i


# Windows


class WindowsLibraryLoader(LibraryLoader):
    name_formats = ["%s.dll", "lib%s.dll", "%slib.dll", "%s"]

    class Lookup(LibraryLoader.Lookup):
        def __init__(self, path):
            super(WindowsLibraryLoader.Lookup, self).__init__(path)
            self.access["stdcall"] = ctypes.windll.LoadLibrary(path)


# Platform switching

# If your value of sys.platform does not appear in this dict, please contact
# the Ctypesgen maintainers.

loaderclass = {
    "darwin": DarwinLibraryLoader,
    "cygwin": WindowsLibraryLoader,
    "win32": WindowsLibraryLoader,
    "msys": WindowsLibraryLoader,
}

load_library = loaderclass.get(sys.platform, PosixLibraryLoader)()


def add_library_search_dirs(other_dirs):
    """
    Add libraries to search paths.
    If library paths are relative, convert them to absolute with respect to this
    file's directory
    """
    for F in other_dirs:
        if not os.path.isabs(F):
            F = os.path.abspath(F)
        load_library.other_dirs.append(F)


del loaderclass

# End loader

add_library_search_dirs(['bin'])

# Begin libraries
_libs["bin/libwdissector.so"] = load_library("bin/libwdissector.so")

# 1 libraries
# End libraries

# No modules

enum_ftenum = c_int# src/wdissector.h: 51

FT_NONE = 0# src/wdissector.h: 51

FT_PROTOCOL = (FT_NONE + 1)# src/wdissector.h: 51

FT_BOOLEAN = (FT_PROTOCOL + 1)# src/wdissector.h: 51

FT_CHAR = (FT_BOOLEAN + 1)# src/wdissector.h: 51

FT_UINT8 = (FT_CHAR + 1)# src/wdissector.h: 51

FT_UINT16 = (FT_UINT8 + 1)# src/wdissector.h: 51

FT_UINT24 = (FT_UINT16 + 1)# src/wdissector.h: 51

FT_UINT32 = (FT_UINT24 + 1)# src/wdissector.h: 51

FT_UINT40 = (FT_UINT32 + 1)# src/wdissector.h: 51

FT_UINT48 = (FT_UINT40 + 1)# src/wdissector.h: 51

FT_UINT56 = (FT_UINT48 + 1)# src/wdissector.h: 51

FT_UINT64 = (FT_UINT56 + 1)# src/wdissector.h: 51

FT_INT8 = (FT_UINT64 + 1)# src/wdissector.h: 51

FT_INT16 = (FT_INT8 + 1)# src/wdissector.h: 51

FT_INT24 = (FT_INT16 + 1)# src/wdissector.h: 51

FT_INT32 = (FT_INT24 + 1)# src/wdissector.h: 51

FT_INT40 = (FT_INT32 + 1)# src/wdissector.h: 51

FT_INT48 = (FT_INT40 + 1)# src/wdissector.h: 51

FT_INT56 = (FT_INT48 + 1)# src/wdissector.h: 51

FT_INT64 = (FT_INT56 + 1)# src/wdissector.h: 51

FT_IEEE_11073_SFLOAT = (FT_INT64 + 1)# src/wdissector.h: 51

FT_IEEE_11073_FLOAT = (FT_IEEE_11073_SFLOAT + 1)# src/wdissector.h: 51

FT_FLOAT = (FT_IEEE_11073_FLOAT + 1)# src/wdissector.h: 51

FT_DOUBLE = (FT_FLOAT + 1)# src/wdissector.h: 51

FT_ABSOLUTE_TIME = (FT_DOUBLE + 1)# src/wdissector.h: 51

FT_RELATIVE_TIME = (FT_ABSOLUTE_TIME + 1)# src/wdissector.h: 51

FT_STRING = (FT_RELATIVE_TIME + 1)# src/wdissector.h: 51

FT_STRINGZ = (FT_STRING + 1)# src/wdissector.h: 51

FT_UINT_STRING = (FT_STRINGZ + 1)# src/wdissector.h: 51

FT_ETHER = (FT_UINT_STRING + 1)# src/wdissector.h: 51

FT_BYTES = (FT_ETHER + 1)# src/wdissector.h: 51

FT_UINT_BYTES = (FT_BYTES + 1)# src/wdissector.h: 51

FT_IPv4 = (FT_UINT_BYTES + 1)# src/wdissector.h: 51

FT_IPv6 = (FT_IPv4 + 1)# src/wdissector.h: 51

FT_IPXNET = (FT_IPv6 + 1)# src/wdissector.h: 51

FT_FRAMENUM = (FT_IPXNET + 1)# src/wdissector.h: 51

FT_PCRE = (FT_FRAMENUM + 1)# src/wdissector.h: 51

FT_GUID = (FT_PCRE + 1)# src/wdissector.h: 51

FT_OID = (FT_GUID + 1)# src/wdissector.h: 51

FT_EUI64 = (FT_OID + 1)# src/wdissector.h: 51

FT_AX25 = (FT_EUI64 + 1)# src/wdissector.h: 51

FT_VINES = (FT_AX25 + 1)# src/wdissector.h: 51

FT_REL_OID = (FT_VINES + 1)# src/wdissector.h: 51

FT_SYSTEM_ID = (FT_REL_OID + 1)# src/wdissector.h: 51

FT_STRINGZPAD = (FT_SYSTEM_ID + 1)# src/wdissector.h: 51

FT_FCWWN = (FT_STRINGZPAD + 1)# src/wdissector.h: 51

FT_NUM_TYPES = (FT_FCWWN + 1)# src/wdissector.h: 51

# src/wdissector.h: 103
class struct__GPtrArray(Structure):
    pass

GPtrArray = struct__GPtrArray# src/wdissector.h: 101

# src/wdissector.h: 108
class struct__GByteArray(Structure):
    pass

GByteArray = struct__GByteArray# src/wdissector.h: 102

struct__GPtrArray.__slots__ = [
    'pdata',
    'len',
]
struct__GPtrArray._fields_ = [
    ('pdata', POINTER(None)),
    ('len', c_uint),
]

struct__GByteArray.__slots__ = [
    'data',
    'len',
]
struct__GByteArray._fields_ = [
    ('data', POINTER(c_ubyte)),
    ('len', c_uint),
]

# src/wdissector.h: 113
class struct__proto_node(Structure):
    pass

struct__proto_node.__slots__ = [
    'first_child',
    'last_child',
    'next',
    'parent',
    'finfo',
    'tree_data',
]
struct__proto_node._fields_ = [
    ('first_child', POINTER(struct__proto_node)),
    ('last_child', POINTER(struct__proto_node)),
    ('next', POINTER(struct__proto_node)),
    ('parent', POINTER(struct__proto_node)),
    ('finfo', POINTER(None)),
    ('tree_data', POINTER(None)),
]

proto_node = struct__proto_node# src/wdissector.h: 120

proto_tree = proto_node# src/wdissector.h: 123

# src/wdissector.h: 135
for _lib in _libs.values():
    try:
        DEMO_PKT_RRC_CONNECTION_SETUP = (c_ubyte * int(128)).in_dll(_lib, "DEMO_PKT_RRC_CONNECTION_SETUP")
        break
    except:
        pass

# src/wdissector.h: 137
for _lib in _libs.values():
    try:
        DEMO_PKT_RRC_SETUP_COMPLETE = (c_ubyte * int(122)).in_dll(_lib, "DEMO_PKT_RRC_SETUP_COMPLETE")
        break
    except:
        pass

# src/wdissector.h: 139
for _lib in _libs.values():
    try:
        DEMO_PKT_RRC_RECONFIGURATION = (c_ubyte * int(114)).in_dll(_lib, "DEMO_PKT_RRC_RECONFIGURATION")
        break
    except:
        pass

# src/wdissector.h: 141
for _lib in _libs.values():
    try:
        DEMO_PKT_NAS_ATTACH_REQUEST = (c_ubyte * int(118)).in_dll(_lib, "DEMO_PKT_NAS_ATTACH_REQUEST")
        break
    except:
        pass

# src/wdissector.h: 148
for _lib in _libs.values():
    if not _lib.has("wdissector_init", "cdecl"):
        continue
    wdissector_init = _lib.get("wdissector_init", "cdecl")
    wdissector_init.argtypes = [String]
    wdissector_init.restype = c_ubyte
    break

enum_ws_log_level = c_int# src/wdissector.h: 149

# src/wdissector.h: 149
for _lib in _libs.values():
    if not _lib.has("wdissector_set_log_level", "cdecl"):
        continue
    wdissector_set_log_level = _lib.get("wdissector_set_log_level", "cdecl")
    wdissector_set_log_level.argtypes = [enum_ws_log_level]
    wdissector_set_log_level.restype = None
    break

# src/wdissector.h: 150
for _lib in _libs.values():
    if not _lib.has("wdissector_enable_fast_full_dissection", "cdecl"):
        continue
    wdissector_enable_fast_full_dissection = _lib.get("wdissector_enable_fast_full_dissection", "cdecl")
    wdissector_enable_fast_full_dissection.argtypes = [c_ubyte]
    wdissector_enable_fast_full_dissection.restype = None
    break

# src/wdissector.h: 151
for _lib in _libs.values():
    if not _lib.has("wdissector_enable_full_dissection", "cdecl"):
        continue
    wdissector_enable_full_dissection = _lib.get("wdissector_enable_full_dissection", "cdecl")
    wdissector_enable_full_dissection.argtypes = [c_ubyte]
    wdissector_enable_full_dissection.restype = None
    break

# src/wdissector.h: 153
for _lib in _libs.values():
    if not _lib.has("packet_set_protocol", "cdecl"):
        continue
    packet_set_protocol = _lib.get("packet_set_protocol", "cdecl")
    packet_set_protocol.argtypes = [String]
    packet_set_protocol.restype = c_ubyte
    break

# src/wdissector.h: 154
for _lib in _libs.values():
    if not _lib.has("packet_set_protocol_fast", "cdecl"):
        continue
    packet_set_protocol_fast = _lib.get("packet_set_protocol_fast", "cdecl")
    packet_set_protocol_fast.argtypes = [String]
    packet_set_protocol_fast.restype = c_ubyte
    break

# src/wdissector.h: 155
for _lib in _libs.values():
    if not _lib.has("packet_dissect", "cdecl"):
        continue
    packet_dissect = _lib.get("packet_dissect", "cdecl")
    packet_dissect.argtypes = [POINTER(c_ubyte), c_uint]
    packet_dissect.restype = None
    break

# src/wdissector.h: 156
for _lib in _libs.values():
    if not _lib.has("packet_set_direction", "cdecl"):
        continue
    packet_set_direction = _lib.get("packet_set_direction", "cdecl")
    packet_set_direction.argtypes = [c_int]
    packet_set_direction.restype = None
    break

# src/wdissector.h: 157
for _lib in _libs.values():
    if not _lib.has("packet_cleanup", "cdecl"):
        continue
    packet_cleanup = _lib.get("packet_cleanup", "cdecl")
    packet_cleanup.argtypes = []
    packet_cleanup.restype = None
    break

# src/wdissector.h: 158
for _lib in _libs.values():
    if not _lib.has("packet_navigate", "cdecl"):
        continue
    packet_navigate = _lib.get("packet_navigate", "cdecl")
    packet_navigate.argtypes = [c_uint, c_uint, CFUNCTYPE(UNCHECKED(c_ubyte), POINTER(proto_tree), c_ubyte, POINTER(c_ubyte))]
    packet_navigate.restype = None
    break

# src/wdissector.h: 162
for _lib in _libs.values():
    if not _lib.has("wdissector_version_info", "cdecl"):
        continue
    wdissector_version_info = _lib.get("wdissector_version_info", "cdecl")
    wdissector_version_info.argtypes = []
    wdissector_version_info.restype = c_char_p
    break

# src/wdissector.h: 163
for _lib in _libs.values():
    if not _lib.has("wdissector_profile_info", "cdecl"):
        continue
    wdissector_profile_info = _lib.get("wdissector_profile_info", "cdecl")
    wdissector_profile_info.argtypes = []
    wdissector_profile_info.restype = c_char_p
    break

# src/wdissector.h: 167
for _lib in _libs.values():
    if not _lib.has("packet_has_condition", "cdecl"):
        continue
    packet_has_condition = _lib.get("packet_has_condition", "cdecl")
    packet_has_condition.argtypes = [String]
    packet_has_condition.restype = c_ubyte
    break

# src/wdissector.h: 168
for _lib in _libs.values():
    if not _lib.has("packet_register_condition", "cdecl"):
        continue
    packet_register_condition = _lib.get("packet_register_condition", "cdecl")
    packet_register_condition.argtypes = [String, c_ushort]
    packet_register_condition.restype = c_ubyte
    break

# src/wdissector.h: 169
for _lib in _libs.values():
    if not _lib.has("packet_set_condition", "cdecl"):
        continue
    packet_set_condition = _lib.get("packet_set_condition", "cdecl")
    packet_set_condition.argtypes = [c_ushort]
    packet_set_condition.restype = None
    break

# src/wdissector.h: 170
for _lib in _libs.values():
    if not _lib.has("packet_read_condition", "cdecl"):
        continue
    packet_read_condition = _lib.get("packet_read_condition", "cdecl")
    packet_read_condition.argtypes = [c_ushort]
    packet_read_condition.restype = c_ubyte
    break

# src/wdissector.h: 172
for _lib in _libs.values():
    if not _lib.has("packet_register_filter", "cdecl"):
        continue
    packet_register_filter = _lib.get("packet_register_filter", "cdecl")
    packet_register_filter.argtypes = [String]
    packet_register_filter.restype = c_char_p
    break

# src/wdissector.h: 173
for _lib in _libs.values():
    if not _lib.has("packet_set_filter", "cdecl"):
        continue
    packet_set_filter = _lib.get("packet_set_filter", "cdecl")
    packet_set_filter.argtypes = [String]
    packet_set_filter.restype = None
    break

# src/wdissector.h: 174
for _lib in _libs.values():
    if not _lib.has("packet_read_filter", "cdecl"):
        continue
    packet_read_filter = _lib.get("packet_read_filter", "cdecl")
    packet_read_filter.argtypes = [String]
    packet_read_filter.restype = c_ubyte
    break

# src/wdissector.h: 177
for _lib in _libs.values():
    if not _lib.has("packet_get_header_info", "cdecl"):
        continue
    packet_get_header_info = _lib.get("packet_get_header_info", "cdecl")
    packet_get_header_info.argtypes = [String]
    packet_get_header_info.restype = POINTER(c_ubyte)
    packet_get_header_info.errcheck = lambda v,*a : cast(v, c_void_p)
    break

# src/wdissector.h: 178
for _lib in _libs.values():
    if not _lib.has("packet_register_set_field_hfinfo", "cdecl"):
        continue
    packet_register_set_field_hfinfo = _lib.get("packet_register_set_field_hfinfo", "cdecl")
    packet_register_set_field_hfinfo.argtypes = [String]
    packet_register_set_field_hfinfo.restype = POINTER(c_ubyte)
    packet_register_set_field_hfinfo.errcheck = lambda v,*a : cast(v, c_void_p)
    break

# src/wdissector.h: 179
for _lib in _libs.values():
    if not _lib.has("packet_get_field_exists", "cdecl"):
        continue
    packet_get_field_exists = _lib.get("packet_get_field_exists", "cdecl")
    packet_get_field_exists.argtypes = [String]
    packet_get_field_exists.restype = c_int
    break

# src/wdissector.h: 180
for _lib in _libs.values():
    if not _lib.has("packet_get_field", "cdecl"):
        continue
    packet_get_field = _lib.get("packet_get_field", "cdecl")
    packet_get_field.argtypes = [String]
    packet_get_field.restype = POINTER(c_ubyte)
    packet_get_field.errcheck = lambda v,*a : cast(v, c_void_p)
    break

# src/wdissector.h: 181
for _lib in _libs.values():
    if not _lib.has("packet_get_field_name", "cdecl"):
        continue
    packet_get_field_name = _lib.get("packet_get_field_name", "cdecl")
    packet_get_field_name.argtypes = [String]
    packet_get_field_name.restype = c_char_p
    break

# src/wdissector.h: 182
for _lib in _libs.values():
    if not _lib.has("packet_get_field_string", "cdecl"):
        continue
    packet_get_field_string = _lib.get("packet_get_field_string", "cdecl")
    packet_get_field_string.argtypes = [String]
    packet_get_field_string.restype = c_char_p
    break

# src/wdissector.h: 183
for _lib in _libs.values():
    if not _lib.has("packet_get_field_offset", "cdecl"):
        continue
    packet_get_field_offset = _lib.get("packet_get_field_offset", "cdecl")
    packet_get_field_offset.argtypes = [String]
    packet_get_field_offset.restype = c_uint
    break

# src/wdissector.h: 184
for _lib in _libs.values():
    if not _lib.has("packet_get_field_size", "cdecl"):
        continue
    packet_get_field_size = _lib.get("packet_get_field_size", "cdecl")
    packet_get_field_size.argtypes = [String]
    packet_get_field_size.restype = c_uint
    break

# src/wdissector.h: 185
for _lib in _libs.values():
    if not _lib.has("packet_get_field_bitmask", "cdecl"):
        continue
    packet_get_field_bitmask = _lib.get("packet_get_field_bitmask", "cdecl")
    packet_get_field_bitmask.argtypes = [String]
    packet_get_field_bitmask.restype = c_ulong
    break

# src/wdissector.h: 186
for _lib in _libs.values():
    if not _lib.has("packet_get_field_encoding", "cdecl"):
        continue
    packet_get_field_encoding = _lib.get("packet_get_field_encoding", "cdecl")
    packet_get_field_encoding.argtypes = [String]
    packet_get_field_encoding.restype = c_uint
    break

# src/wdissector.h: 187
for _lib in _libs.values():
    if not _lib.has("packet_get_field_type", "cdecl"):
        continue
    packet_get_field_type = _lib.get("packet_get_field_type", "cdecl")
    packet_get_field_type.argtypes = [String]
    packet_get_field_type.restype = c_int
    break

# src/wdissector.h: 188
for _lib in _libs.values():
    if not _lib.has("packet_get_field_type_name", "cdecl"):
        continue
    packet_get_field_type_name = _lib.get("packet_get_field_type_name", "cdecl")
    packet_get_field_type_name.argtypes = [String]
    packet_get_field_type_name.restype = c_char_p
    break

# src/wdissector.h: 189
for _lib in _libs.values():
    if not _lib.has("packet_get_field_encoding_name", "cdecl"):
        continue
    packet_get_field_encoding_name = _lib.get("packet_get_field_encoding_name", "cdecl")
    packet_get_field_encoding_name.argtypes = [String]
    packet_get_field_encoding_name.restype = c_char_p
    break

# src/wdissector.h: 190
for _lib in _libs.values():
    if not _lib.has("packet_get_field_uint32", "cdecl"):
        continue
    packet_get_field_uint32 = _lib.get("packet_get_field_uint32", "cdecl")
    packet_get_field_uint32.argtypes = [String]
    packet_get_field_uint32.restype = c_uint
    break

# src/wdissector.h: 195
for _lib in _libs.values():
    if not _lib.has("packet_set_field_hfinfo", "cdecl"):
        continue
    packet_set_field_hfinfo = _lib.get("packet_set_field_hfinfo", "cdecl")
    packet_set_field_hfinfo.argtypes = [POINTER(None)]
    packet_set_field_hfinfo.restype = None
    break

# src/wdissector.h: 196
for _lib in _libs.values():
    if not _lib.has("packet_set_field_hfinfo_all", "cdecl"):
        continue
    packet_set_field_hfinfo_all = _lib.get("packet_set_field_hfinfo_all", "cdecl")
    packet_set_field_hfinfo_all.argtypes = [POINTER(None)]
    packet_set_field_hfinfo_all.restype = None
    break

# src/wdissector.h: 197
for _lib in _libs.values():
    if not _lib.has("packet_read_field_exists_hfinfo", "cdecl"):
        continue
    packet_read_field_exists_hfinfo = _lib.get("packet_read_field_exists_hfinfo", "cdecl")
    packet_read_field_exists_hfinfo.argtypes = [POINTER(None)]
    packet_read_field_exists_hfinfo.restype = c_int
    break

# src/wdissector.h: 198
for _lib in _libs.values():
    if not _lib.has("packet_read_field_hfinfo", "cdecl"):
        continue
    packet_read_field_hfinfo = _lib.get("packet_read_field_hfinfo", "cdecl")
    packet_read_field_hfinfo.argtypes = [POINTER(None)]
    packet_read_field_hfinfo.restype = POINTER(c_ubyte)
    packet_read_field_hfinfo.errcheck = lambda v,*a : cast(v, c_void_p)
    break

# src/wdissector.h: 199
for _lib in _libs.values():
    if not _lib.has("packet_read_fields_hfinfo", "cdecl"):
        continue
    packet_read_fields_hfinfo = _lib.get("packet_read_fields_hfinfo", "cdecl")
    packet_read_fields_hfinfo.argtypes = [POINTER(None)]
    packet_read_fields_hfinfo.restype = POINTER(GPtrArray)
    break

# src/wdissector.h: 201
for _lib in _libs.values():
    if not _lib.has("packet_register_field", "cdecl"):
        continue
    packet_register_field = _lib.get("packet_register_field", "cdecl")
    packet_register_field.argtypes = [String, c_ushort]
    packet_register_field.restype = c_ubyte
    break

# src/wdissector.h: 202
for _lib in _libs.values():
    if not _lib.has("packet_register_set_field", "cdecl"):
        continue
    packet_register_set_field = _lib.get("packet_register_set_field", "cdecl")
    packet_register_set_field.argtypes = [String, c_ushort]
    packet_register_set_field.restype = None
    break

# src/wdissector.h: 203
for _lib in _libs.values():
    if not _lib.has("packet_set_field", "cdecl"):
        continue
    packet_set_field = _lib.get("packet_set_field", "cdecl")
    packet_set_field.argtypes = [c_ushort]
    packet_set_field.restype = None
    break

# src/wdissector.h: 204
for _lib in _libs.values():
    if not _lib.has("packet_read_field", "cdecl"):
        continue
    packet_read_field = _lib.get("packet_read_field", "cdecl")
    packet_read_field.argtypes = [c_ushort]
    packet_read_field.restype = POINTER(c_ubyte)
    packet_read_field.errcheck = lambda v,*a : cast(v, c_void_p)
    break

# src/wdissector.h: 205
for _lib in _libs.values():
    if not _lib.has("packet_read_fields", "cdecl"):
        continue
    packet_read_fields = _lib.get("packet_read_fields", "cdecl")
    packet_read_fields.argtypes = [c_ushort]
    packet_read_fields.restype = POINTER(GPtrArray)
    break

# src/wdissector.h: 207
for _lib in _libs.values():
    if not _lib.has("packet_read_field_at", "cdecl"):
        continue
    packet_read_field_at = _lib.get("packet_read_field_at", "cdecl")
    packet_read_field_at.argtypes = [POINTER(GPtrArray), c_ushort]
    packet_read_field_at.restype = POINTER(c_ubyte)
    packet_read_field_at.errcheck = lambda v,*a : cast(v, c_void_p)
    break

# src/wdissector.h: 208
for _lib in _libs.values():
    if not _lib.has("packet_read_field_name", "cdecl"):
        continue
    packet_read_field_name = _lib.get("packet_read_field_name", "cdecl")
    packet_read_field_name.argtypes = [POINTER(None)]
    packet_read_field_name.restype = c_char_p
    break

# src/wdissector.h: 209
for _lib in _libs.values():
    if not _lib.has("packet_read_field_abbrev", "cdecl"):
        continue
    packet_read_field_abbrev = _lib.get("packet_read_field_abbrev", "cdecl")
    packet_read_field_abbrev.argtypes = [POINTER(None)]
    packet_read_field_abbrev.restype = c_char_p
    break

# src/wdissector.h: 210
for _lib in _libs.values():
    if not _lib.has("packet_read_field_offset", "cdecl"):
        continue
    packet_read_field_offset = _lib.get("packet_read_field_offset", "cdecl")
    packet_read_field_offset.argtypes = [POINTER(None)]
    packet_read_field_offset.restype = c_ushort
    break

# src/wdissector.h: 211
for _lib in _libs.values():
    if not _lib.has("packet_read_field_size", "cdecl"):
        continue
    packet_read_field_size = _lib.get("packet_read_field_size", "cdecl")
    packet_read_field_size.argtypes = [POINTER(None)]
    packet_read_field_size.restype = c_uint
    break

# src/wdissector.h: 212
for _lib in _libs.values():
    if not _lib.has("packet_read_field_size_bits", "cdecl"):
        continue
    packet_read_field_size_bits = _lib.get("packet_read_field_size_bits", "cdecl")
    packet_read_field_size_bits.argtypes = [c_ulong]
    packet_read_field_size_bits.restype = c_ubyte
    break

# src/wdissector.h: 213
for _lib in _libs.values():
    if not _lib.has("packet_read_field_bitmask", "cdecl"):
        continue
    packet_read_field_bitmask = _lib.get("packet_read_field_bitmask", "cdecl")
    packet_read_field_bitmask.argtypes = [POINTER(None)]
    packet_read_field_bitmask.restype = c_ulong
    break

# src/wdissector.h: 214
for _lib in _libs.values():
    if not _lib.has("packet_read_field_bitmask_offset", "cdecl"):
        continue
    packet_read_field_bitmask_offset = _lib.get("packet_read_field_bitmask_offset", "cdecl")
    packet_read_field_bitmask_offset.argtypes = [c_ulong]
    packet_read_field_bitmask_offset.restype = c_ubyte
    break

# src/wdissector.h: 215
for _lib in _libs.values():
    if not _lib.has("packet_read_field_bitmask_offset_msb", "cdecl"):
        continue
    packet_read_field_bitmask_offset_msb = _lib.get("packet_read_field_bitmask_offset_msb", "cdecl")
    packet_read_field_bitmask_offset_msb.argtypes = [c_ulong]
    packet_read_field_bitmask_offset_msb.restype = c_ubyte
    break

# src/wdissector.h: 216
for _lib in _libs.values():
    if not _lib.has("packet_read_field_encoding", "cdecl"):
        continue
    packet_read_field_encoding = _lib.get("packet_read_field_encoding", "cdecl")
    packet_read_field_encoding.argtypes = [POINTER(None)]
    packet_read_field_encoding.restype = c_uint
    break

# src/wdissector.h: 217
for _lib in _libs.values():
    if not _lib.has("packet_read_field_type", "cdecl"):
        continue
    packet_read_field_type = _lib.get("packet_read_field_type", "cdecl")
    packet_read_field_type.argtypes = [POINTER(None)]
    packet_read_field_type.restype = c_int
    break

# src/wdissector.h: 218
for _lib in _libs.values():
    if not _lib.has("packet_read_field_type_name", "cdecl"):
        continue
    packet_read_field_type_name = _lib.get("packet_read_field_type_name", "cdecl")
    packet_read_field_type_name.argtypes = [POINTER(None)]
    packet_read_field_type_name.restype = c_char_p
    break

# src/wdissector.h: 219
for _lib in _libs.values():
    if not _lib.has("packet_read_field_encoding_name", "cdecl"):
        continue
    packet_read_field_encoding_name = _lib.get("packet_read_field_encoding_name", "cdecl")
    packet_read_field_encoding_name.argtypes = [POINTER(None)]
    packet_read_field_encoding_name.restype = c_char_p
    break

# src/wdissector.h: 220
for _lib in _libs.values():
    if not _lib.has("packet_read_field_string", "cdecl"):
        continue
    packet_read_field_string = _lib.get("packet_read_field_string", "cdecl")
    packet_read_field_string.argtypes = [POINTER(None)]
    packet_read_field_string.restype = c_char_p
    break

# src/wdissector.h: 221
for _lib in _libs.values():
    if not _lib.has("packet_read_field_ustring", "cdecl"):
        continue
    packet_read_field_ustring = _lib.get("packet_read_field_ustring", "cdecl")
    packet_read_field_ustring.argtypes = [POINTER(None)]
    packet_read_field_ustring.restype = POINTER(c_ubyte)
    break

# src/wdissector.h: 222
for _lib in _libs.values():
    if not _lib.has("packet_read_field_bytes", "cdecl"):
        continue
    packet_read_field_bytes = _lib.get("packet_read_field_bytes", "cdecl")
    packet_read_field_bytes.argtypes = [POINTER(None)]
    packet_read_field_bytes.restype = POINTER(GByteArray)
    break

# src/wdissector.h: 223
for _lib in _libs.values():
    if not _lib.has("packet_read_field_uint32", "cdecl"):
        continue
    packet_read_field_uint32 = _lib.get("packet_read_field_uint32", "cdecl")
    packet_read_field_uint32.argtypes = [POINTER(None)]
    packet_read_field_uint32.restype = c_uint
    break

# src/wdissector.h: 224
for _lib in _libs.values():
    if not _lib.has("packet_read_field_int32", "cdecl"):
        continue
    packet_read_field_int32 = _lib.get("packet_read_field_int32", "cdecl")
    packet_read_field_int32.argtypes = [POINTER(None)]
    packet_read_field_int32.restype = c_int
    break

# src/wdissector.h: 225
for _lib in _libs.values():
    if not _lib.has("packet_read_field_uint64", "cdecl"):
        continue
    packet_read_field_uint64 = _lib.get("packet_read_field_uint64", "cdecl")
    packet_read_field_uint64.argtypes = [POINTER(None)]
    packet_read_field_uint64.restype = c_ulong
    break

# src/wdissector.h: 226
for _lib in _libs.values():
    if not _lib.has("packet_read_field_int64", "cdecl"):
        continue
    packet_read_field_int64 = _lib.get("packet_read_field_int64", "cdecl")
    packet_read_field_int64.argtypes = [POINTER(None)]
    packet_read_field_int64.restype = c_long
    break

# src/wdissector.h: 227
for _lib in _libs.values():
    if not _lib.has("packet_read_value_to_string", "cdecl"):
        continue
    packet_read_value_to_string = _lib.get("packet_read_value_to_string", "cdecl")
    packet_read_value_to_string.argtypes = [c_uint, POINTER(None)]
    packet_read_value_to_string.restype = c_char_p
    break

# src/wdissector.h: 230
for _lib in _libs.values():
    if not _lib.has("packet_show", "cdecl"):
        continue
    packet_show = _lib.get("packet_show", "cdecl")
    packet_show.argtypes = []
    packet_show.restype = c_char_p
    break

# src/wdissector.h: 231
for _lib in _libs.values():
    if not _lib.has("packet_summary", "cdecl"):
        continue
    packet_summary = _lib.get("packet_summary", "cdecl")
    packet_summary.argtypes = []
    packet_summary.restype = c_char_p
    break

# src/wdissector.h: 232
for _lib in _libs.values():
    if not _lib.has("packet_layers", "cdecl"):
        continue
    packet_layers = _lib.get("packet_layers", "cdecl")
    packet_layers.argtypes = []
    packet_layers.restype = c_char_p
    break

# src/wdissector.h: 233
for _lib in _libs.values():
    if not _lib.has("packet_layer", "cdecl"):
        continue
    packet_layer = _lib.get("packet_layer", "cdecl")
    packet_layer.argtypes = [c_ubyte]
    packet_layer.restype = c_char_p
    break

# src/wdissector.h: 234
for _lib in _libs.values():
    if not _lib.has("packet_layers_count", "cdecl"):
        continue
    packet_layers_count = _lib.get("packet_layers_count", "cdecl")
    packet_layers_count.argtypes = []
    packet_layers_count.restype = c_uint
    break

# src/wdissector.h: 235
for _lib in _libs.values():
    if not _lib.has("packet_relevant_fields", "cdecl"):
        continue
    packet_relevant_fields = _lib.get("packet_relevant_fields", "cdecl")
    packet_relevant_fields.argtypes = []
    packet_relevant_fields.restype = c_char_p
    break

# src/wdissector.h: 236
for _lib in _libs.values():
    if not _lib.has("packet_description", "cdecl"):
        continue
    packet_description = _lib.get("packet_description", "cdecl")
    packet_description.argtypes = []
    if sizeof(c_int) == sizeof(c_void_p):
        packet_description.restype = ReturnString
    else:
        packet_description.restype = String
        packet_description.errcheck = ReturnString
    break

# src/wdissector.h: 237
for _lib in _libs.values():
    if not _lib.has("packet_direction", "cdecl"):
        continue
    packet_direction = _lib.get("packet_direction", "cdecl")
    packet_direction.argtypes = []
    packet_direction.restype = c_ubyte
    break

# src/wdissector.h: 238
for _lib in _libs.values():
    if not _lib.has("packet_protocol", "cdecl"):
        continue
    packet_protocol = _lib.get("packet_protocol", "cdecl")
    packet_protocol.argtypes = []
    packet_protocol.restype = c_char_p
    break

# src/wdissector.h: 239
for _lib in _libs.values():
    if not _lib.has("packet_show_pdml", "cdecl"):
        continue
    packet_show_pdml = _lib.get("packet_show_pdml", "cdecl")
    packet_show_pdml.argtypes = []
    packet_show_pdml.restype = c_char_p
    break

# src/wdissector.h: 240
for _lib in _libs.values():
    if not _lib.has("packet_field_summary", "cdecl"):
        continue
    packet_field_summary = _lib.get("packet_field_summary", "cdecl")
    packet_field_summary.argtypes = [POINTER(c_ubyte), c_uint, String]
    packet_field_summary.restype = c_ubyte
    break

# src/wdissector.h: 243
for _lib in _libs.values():
    if not _lib.has("wd_log_g", "cdecl"):
        continue
    wd_log_g = _lib.get("wd_log_g", "cdecl")
    wd_log_g.argtypes = [String]
    wd_log_g.restype = None
    break

# src/wdissector.h: 244
for _lib in _libs.values():
    if not _lib.has("wd_log_y", "cdecl"):
        continue
    wd_log_y = _lib.get("wd_log_y", "cdecl")
    wd_log_y.argtypes = [String]
    wd_log_y.restype = None
    break

# src/wdissector.h: 245
for _lib in _libs.values():
    if not _lib.has("wd_log_r", "cdecl"):
        continue
    wd_log_r = _lib.get("wd_log_r", "cdecl")
    wd_log_r.argtypes = [String]
    wd_log_r.restype = None
    break

# src/wdissector.h: 246
for _lib in _libs.values():
    if not _lib.has("set_wd_log_g", "cdecl"):
        continue
    set_wd_log_g = _lib.get("set_wd_log_g", "cdecl")
    set_wd_log_g.argtypes = [CFUNCTYPE(UNCHECKED(None), String)]
    set_wd_log_g.restype = None
    break

# src/wdissector.h: 247
for _lib in _libs.values():
    if not _lib.has("set_wd_log_y", "cdecl"):
        continue
    set_wd_log_y = _lib.get("set_wd_log_y", "cdecl")
    set_wd_log_y.argtypes = [CFUNCTYPE(UNCHECKED(None), String)]
    set_wd_log_y.restype = None
    break

# src/wdissector.h: 248
for _lib in _libs.values():
    if not _lib.has("set_wd_log_r", "cdecl"):
        continue
    set_wd_log_r = _lib.get("set_wd_log_r", "cdecl")
    set_wd_log_r.argtypes = [CFUNCTYPE(UNCHECKED(None), String)]
    set_wd_log_r.restype = None
    break

# src/wdissector.h: 12
try:
    WD_TYPE_FIELD = 0
except:
    pass

# src/wdissector.h: 13
try:
    WD_TYPE_GROUP = 1
except:
    pass

# src/wdissector.h: 14
try:
    WD_TYPE_LAYER = 2
except:
    pass

uint8_t = c_ubyte# src/wdissector.h: 33

uint16_t = c_ushort# src/wdissector.h: 34

uint32_t = c_uint# src/wdissector.h: 35

uint64_t = c_ulong# src/wdissector.h: 36

int64_t = c_long# src/wdissector.h: 37

int32_t = c_int# src/wdissector.h: 38

gboolean = c_ubyte# src/wdissector.h: 39

gchar = c_char# src/wdissector.h: 40

guint8 = c_ubyte# src/wdissector.h: 41

guint32 = c_uint# src/wdissector.h: 42

guchar = c_ubyte# src/wdissector.h: 43

guint = c_uint# src/wdissector.h: 44

guint64 = c_ulong# src/wdissector.h: 45

field_info = None# src/wdissector.h: 46

header_field_info = None# src/wdissector.h: 47

gpointer = None# src/wdissector.h: 48

_GPtrArray = struct__GPtrArray# src/wdissector.h: 103

_GByteArray = struct__GByteArray# src/wdissector.h: 108

_proto_node = struct__proto_node# src/wdissector.h: 113

# No inserted files

# No prefix-stripping

