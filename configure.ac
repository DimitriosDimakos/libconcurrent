
AC_INIT([libconcurrent], [1.0.0])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_ARG_ENABLE([pthreadlib], AS_HELP_STRING([--disable-pthreadlib (enabled by default)]), [pthreadlib=no], [pthreadlib=yes])
AS_IF([test "x$pthreadlib" == "xyes"],
    AC_CHECK_LIB(pthread, pthread_create, [], [echo "You need to have the posix thread library." exit -1])
)
AC_CHECK_LIB(rt, clock_gettime, [], [])

LT_INIT
# AC_PROG_RANLIB

AC_CONFIG_HEADERS([config.h])

# Checks for header files.
AC_CHECK_HEADERS([sys/timeb.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([ftime gettimeofday clock_gettime])

AC_CONFIG_FILES(Makefile src/Makefile examples/Makefile)
AC_OUTPUT
