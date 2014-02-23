#
#				configure.ac
#
# Process this file with autoconf to generate a configure script.
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
#	This file part of:	STIFF
#
#	Copyright:		(C) 2003-2014 Emmanuel Bertin -- IAP/CNRS/UPMC
#
#	License:		GNU General Public License
#
#	STIFF is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
# 	(at your option) any later version.
#	STIFF is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#	You should have received a copy of the GNU General Public License
#	along with STIFF.  If not, see <http://www.gnu.org/licenses/>.
#
#	Last modified:		23/02/2014
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

# Process this file with autoconf to produce a configure script.

# First, disable the annoying config.cache
define([AC_CACHE_LOAD],)
define([AC_CACHE_SAVE],)

# This is your standard Bertin source code...
AC_INIT(stiff, 2.4.0, [bertin@iap.fr])
AC_CONFIG_SRCDIR(src/makeit.c)
AC_CONFIG_AUX_DIR(autoconf)
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE
date=`date +%Y-%m-%d`
date2=`date +"%a %b %d %Y"`
date3=`date +"%B %Y"`
AC_DEFINE_UNQUOTED(DATE, "$date", [Archive creation date])
AC_SUBST(PACKAGER, "Emmanuel Bertin")
AC_SUBST(DATE2, "$date2")
AC_SUBST(DATE3, "$date3")

# Include macros
sinclude(acx_pthread.m4)
sinclude(acx_prog_cc_optim.m4)
sinclude(acx_libtiff.m4)
sinclude(acx_urbi_resolve_dir.m4)

# Display pakage and version number
AC_MSG_RESULT([*********** Configuring:  $PACKAGE $VERSION ($date) **********])

AC_MSG_CHECKING([if compilation flags should be set automatically])
AC_ARG_ENABLE(auto-flags,
	[AS_HELP_STRING([--enable-auto-flags],
	[Let the configure script choose the compilation flags \
(off by default)])])

# Provide special option for the Linux Intel C compiler
AC_MSG_CHECKING([whether we should use the INTEL compiler])
AC_ARG_ENABLE(icc,
	[AS_HELP_STRING([--enable-icc],
	[Enable special mode for compilation with the Intel compiler \
(off by default)])],
        CC="icc"
	AC_MSG_RESULT([yes]),
	AC_MSG_RESULT([no]))

# Checks for programs.
AC_LANG(C)

AC_SEARCH_LIBS([strerror],[cposix])
if test "$enable_auto_flags" = "yes"; then
  CFLAGS=""
  LDFLAGS=""
  ACX_PROG_CC_OPTIM
fi
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB(m, sin)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h time.h unistd.h sys/mman.h sys/stat.h sys/time.h sys/types.h)
if test "$enable_icc" = "yes"; then
  AC_CHECK_HEADERS(mathimf.h)
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL
AC_TYPE_LONG_LONG_INT
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MMAP
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([atexit memmove memset munmap strchr strpbrk strrchr strspn strcspn strtol])
AC_CHECK_FUNC([isnan], AC_DEFINE_UNQUOTED([HAVE_ISNAN2], 1,
		[Second isnan check]))

# Check support for large files
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

# Provide special options for libraries
AC_ARG_WITH(z-libdir,
	[AS_HELP_STRING([--with-z-libdir=<libz library path>],
	[Alternative directory for the zlib library])])
AC_ARG_WITH(jpeg-libdir,
	[AS_HELP_STRING([--with-jpeg-libdir=<libjpeg library path>],
	[Alternative directory for the libJPEG library])])
AC_ARG_WITH(tiff-libdir,
	[AS_HELP_STRING([--with-tiff-libdir=<libTIFF library path>],
	[Alternative directory for the libTIFF library])])
AC_ARG_WITH(tiff-incdir,
	[AS_HELP_STRING([--with-tiff-incdir=<libTIFF include dir>],
	[plternative directory for libTIFF include files])])

# Provide a special option for the default XSLT URL
with_xsl_url="file://"$(URBI_RESOLVE_DIR([$datadir]))"/$PACKAGE_NAME/$PACKAGE_NAME.xsl"
AC_ARG_WITH(xsl_url,
	[AS_HELP_STRING([--with-xsl_url=<default URL for XSLT filter>],
	[Provide an alternative default URL of the XSLT filter])])

AC_DEFINE_UNQUOTED([XSL_URL], "$with_xsl_url",[Default URL of the XSLT filter])
# Set flags for multithreading
n_pthreads=64
AC_ARG_ENABLE(threads,
	[AS_HELP_STRING([--enable-threads@<:@=<max_number_of_threads>@:>@],
	[Enable multhreading (on with up to 64 threads by default)])],
    if test "$enableval" = "no"; then
      use_pthreads="no"
    else
      use_pthreads="yes"
      if test "$enableval" != "yes"; then
        n_pthreads=$enableval
      fi
    fi,
    use_pthreads=yes
    )

# Provide special option for profiling
AC_MSG_CHECKING([for profiler mode])
AC_ARG_ENABLE(profiling,
	[AS_HELP_STRING([--enable-profiling],
	[Enable special mode for profiling (off by default)])]
	AC_MSG_RESULT([yes]),
	AC_MSG_RESULT([no]))

# Enable linking options for making the executable as portable as possible.
AC_MSG_CHECKING([best linking option])
AC_ARG_ENABLE(best-link,
	[AS_HELP_STRING([--enable-best-link],
	[Choose the right combination of static and dynamic linking to make \
the executable as portable as possible (off by default)])])


################# Actions to complete in case of multhreading ################
AC_DEFINE_UNQUOTED(THREADS_NMAX, $n_pthreads,[Maximum number of POSIX threads])
if test "$use_pthreads" = "yes"; then
  AC_MSG_CHECKING([for multithreading])
  AC_MSG_RESULT([maximum of $n_pthreads thread(s)])
  AC_DEFINE(USE_THREADS, 1, [Triggers multhreading])
# CC, CFLAGS and LIBS are system and compiler-dependent
  ACX_PTHREAD
  CC="$PTHREAD_CC"
  [AM_CFLAGS="$AM_CFLAGS $PTHREAD_CFLAGS -D_REENTRANT"]
  LIBS="$PTHREAD_LIBS $LIBS"
fi
AM_CONDITIONAL(USE_THREADS, test $use_pthreads = "yes")

######################## handle the libTIFF library ###########################
ACX_LIBTIFF($with_libtiff_libdir,$with_libtiff_incdir, $with_libjpeg_libdir, $with_libz_libdir,
	[use_libtiff=yes],[use_libtiff=no])
if test "$use_libtiff" = "yes"; then
  LIBS="$LIBTIFF_LIBS $LIBS"
  if test "$LIBTIFF_WARN" != ""; then
    AC_MSG_WARN([$LIBTIFF_WARN])
  fi
else
  AC_MSG_ERROR([$LIBTIFF_ERROR Exiting.])
fi

# Compile with profiling option
if test "$enable_profiling" = "yes"; then
  if test "$enable_icc" = "yes"; then
    AM_CFLAGS="$AM_CFLAGS -pq"
  else
    AM_CFLAGS="$AM_CFLAGS -pg"
  fi
  enable_best_link="no"
fi

# "Best" linking option
if test "$enable_best_link" = "yes"; then
  if test "$enable_icc" = "yes"; then
    AM_LDFLAGS="-static-intel -shared-libgcc -no-intel-extensions $AM_LDFLAGS"
  else
    AM_LDFLAGS="-shared-libgcc $AM_LDFLAGS"
  fi
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

