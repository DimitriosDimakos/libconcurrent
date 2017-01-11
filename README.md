# libconcurrent
High level concurrent C library

 _ _ _                                                    _   
| (_) |__   ___ ___  _ __   ___ _   _ _ __ _ __ ___ _ __ | |_ 
| | | '_ \ / __/ _ \| '_ \ / __| | | | '__| '__/ _ \ '_ \| __|
| | | |_) | (_| (_) | | | | (__| |_| | |  | | |  __/ | | | |_ 
|_|_|_.__/ \___\___/|_| |_|\___|\__,_|_|  |_|  \___|_| |_|\__|


-----------------------------------------------------------------------------------------------

Description
-----------
libconcurrent is a C library which provides an implementation of a high level concurrent library,
using the underlying platform thread API. So far the following thread APIs are supported:
- POSIX threads
- win32 API threads

Using libconcurrent
--------------------
You can either copy directly the libconcurrent sources to your project or compile it and install it as
a shared library. See the provided example for more details on how to use libconcurrent.

libconcurrent has been tested on the following platforms.
- Solaris 2.6/2.8 (sparc)
- Linux x86_64 (intel)
- Windows 7 (32 bit/intel)
- Windows 10 (64 bit/intel)
