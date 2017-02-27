
AC_INIT([libconcurrent], [1.0.0])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# options
AC_ARG_WITH([unixthreads],
            [AS_HELP_STRING([--with-unixthreads], [use Unix threads (default=no)])],
            [],
            [with_unixthreads=no])
AC_ARG_WITH([windowsthreads],
            [AS_HELP_STRING([--with-windowsthreads], [use Windows threads (default=no)])],
            [],
            [with_windowsthreads=no])

AC_ARG_ENABLE([oscheck], AS_HELP_STRING([--disable-oscheck][ for non Posix threads (enabled by default)]),
              [with_oscheck=no], [with_oscheck=yes])

if test "x$with_unixthreads" = "xyes" ; then
    with_windowsthreads=no;
    with_posixthreads=no;
    if test "x$with_oscheck" = "xno" ; then
        AC_DEFINE([__USE_UNIX_THREAD_LIB], [], [force use of Unix threads])
    fi
elif test "x$with_windowsthreads" = "xyes" ; then
    with_unixthreads=no;
    with_posixthreads=no;
    if test "x$with_oscheck" = "xno" ; then
        AC_DEFINE([__USE_WINAPI_THREAD_LIB], [], [force use of Windows threads])
    fi
else
    with_posixthreads=yes;
fi

# Checks for libraries.
AS_IF([test "x$with_posixthreads" == "xyes"],
    AC_CHECK_LIB(pthread, pthread_create, [], AC_MSG_ERROR(You need to have the Posix thread library., [-1]))
)

AS_IF([test "x$with_unixthreads" == "xyes"],
    AC_CHECK_LIB(thread, thr_create, [AC_DEFINE([__NO_POSIX_THREAD_LIB], [], [Do not use Posix threads])],
                                     [AC_MSG_ERROR(You need to have the unix thread library., [-1])])
)

if test "x$with_windowsthreads" = "xyes" ; then
    AC_CHECK_HEADERS([windows.h],
                     [AC_DEFINE([__NO_POSIX_THREAD_LIB], [], [Do not use Posix threads])],
                     [AC_MSG_ERROR(You need to have the windows thread library., [-1])])
fi

AC_CHECK_LIB(rt, clock_gettime, [], [])

AC_CONFIG_HEADERS([config.h])

# Checks for header files.
AC_CHECK_HEADERS([sys/timeb.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([ftime gettimeofday clock_gettime])

AC_CONFIG_FILES(Makefile src/Makefile examples/Makefile)
AC_OUTPUT
