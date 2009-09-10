# configure.in for STIFF
# (C) E.Bertin 2003-2009
# Process this file with autoconf to produce a configure script.
# First, disable the annoying config.cache
define([AC_CACHE_LOAD],)
define([AC_CACHE_SAVE],)

# This is your standard Bertin source code...
AC_INIT(stiff, 1.13, [bertin@iap.fr])
AC_CONFIG_SRCDIR(src/makeit.c)
AC_CONFIG_AUX_DIR(autoconf)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE
date=`date +%Y-%m-%d`
date2=`date +"%a %b %d %Y"`
date3=`date +"%B %Y"`
AC_DEFINE_UNQUOTED(DATE, "$date", [Archive creation date])
AC_SUBST(PACKAGER, "Emmanuel Bertin")
AC_SUBST(DATE2, "$date2")
AC_SUBST(DATE3, "$date3")

# Include macros
#sinclude(acx_pthread.m4)
sinclude(acx_prog_cc_optim.m4)

# Display pakage and version number
AC_MSG_RESULT([*********** Configuring:  $PACKAGE $VERSION ($date) **********])

# Initialize the list of compilers to consider
cclist="cc gcc"

AC_MSG_CHECKING([if compilation flags are set automatically])
AC_ARG_ENABLE(auto-flags,
	[AC_HELP_STRING([--enable-auto-flags],
	[Let the configure script choose the compilation flags \
(off by default)])],
	use_autoflag="yes"
	AC_MSG_RESULT([yes]),
	use_autoflag="no"
	AC_MSG_RESULT([no]))

# Provide special option for the Linux Intel C compiler
AC_MSG_CHECKING([for Linux Intel C compiler mode])
AC_ARG_ENABLE(icc,
	[AC_HELP_STRING([--enable-icc],
	[Enable special mode for compilation with the Intel compiler \
(off by default)])],
        use_icc="yes"
        cclist="icc $cclist"
	AC_MSG_RESULT([yes]),
        use_icc="no"
	AC_MSG_RESULT([no]))

# Checks for programs.
# GCC is chosen last because it is likely to yield less optimized code
AC_PROG_CC([$cclist])
# C Compiler: Check that it is ANSI C
AM_PROG_CC_STDC
# C Compiler: Check that it is POSIX-compliant
AC_ISC_POSIX
if test "$use_autoflag" = "yes"; then
  CFLAGS=""
  LDFLAGS =""
  ACX_PROG_CC_OPTIM
fi
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB(m, sin)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h unistd.h sys/mman.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_TYPE_UID_T
AC_CHECK_TYPES([unsigned long long, long long])

# Checks for library functions.
AC_FUNC_MMAP
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([atexit memmove memset munmap strchr strpbrk strrchr strspn strcspn strtol])

# Check support for large files
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

# Provide special option for gprof profiling
AC_MSG_CHECKING([for gprof profiler mode])
AC_ARG_ENABLE(gprof,
        [AC_HELP_STRING([--enable-gprof],
        [Enable special mode for compilation with the gprof profiler \
(off by default)])],
        use_gprof="yes"
        AC_MSG_RESULT([yes]),
        use_gprof="no"
        AC_MSG_RESULT([no]))

# Enable linking options for making the executable as portable as possible.
AC_MSG_CHECKING([best linking option])
AC_ARG_ENABLE(best-link,
	[AC_HELP_STRING([--enable-best-link],
	[Choose the right combination of static and dynamic linking to make \
the executable as portable as possible (off by default)])],
        use_best="yes"
	AC_MSG_RESULT([yes]),
        use_best="no"
	AC_MSG_RESULT([no]))

# Link with gprof option
if test "$use_gprof" = "yes"; then
  if test "$use_icc" = "yes"; then
    AM_CFLAGS="$AM_CFLAGS -pq"
  else
    AM_CFLAGS="$AM_CFLAGS -pg"
  fi
  use_best="no"
fi

# "Best" linking option
if test "$use_best" = "yes"; then
  AM_LDFLAGS="-shared-libgcc -static-libtool-libs $AM_LDFLAGS"
fi

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_LDFLAGS)

# Display compiler and linker flags
AC_MSG_RESULT([***************************************************************])
AC_MSG_RESULT([Compile cmdline:  $CC $AM_CPPFLAGS $CPPFLAGS $AM_CFLAGS $CFLAGS])
AC_MSG_RESULT([Link    cmdline:  $CC $AM_LDFLAGS $LDFLAGS $LIBS])
AC_MSG_RESULT([Default XSLT URL: $xsl_url])
AC_MSG_RESULT([***************************************************************])

AC_CONFIG_FILES([
Makefile
man/Makefile
src/Makefile
src/fits/Makefile
stiff.spec
man/stiff.1])
AC_OUTPUT

