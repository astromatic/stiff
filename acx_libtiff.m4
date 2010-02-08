dnl @synopsis ACX_LIBTIFF([LIBTIFF_LIBDIR, LIBTIFF_INCDIR, LIBJPEG_LIBDIR, LIBZ_LIBDIR,[ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]]])
dnl This macro figures out if the libTIFF libraries and header
dnl files are installed.
dnl You may wish to use these variables in your default LIBS and CFLAGS:
dnl
dnl        LIBS="$LIBTIFF_LIBS $LIBS"
dnl
dnl ACTION-IF-FOUND is a list of shell commands to run if libTIFF
dnl is found (HAVE_LIBTIFF is defined first), and ACTION-IF-NOT-FOUND
dnl is a list of commands to run it if it is not found.
dnl
dnl @version $Id: acx_libtiff.m4,v 1.0 2010/02/08 13:30:17 bertin Exp $
dnl @author Emmanuel Bertin <bertin@iap.fr>

AC_DEFUN([ACX_LIBTIFF], [
AC_REQUIRE([AC_CANONICAL_HOST])

dnl --------------------
dnl Search include files
dnl --------------------

acx_libtiff_ok=no
if test x$2 = x; then
  AC_CHECK_HEADER(tiffio.h,[acx_libtiff_ok=yes])
  if test x$acx_libtiff_ok = xyes; then
    AC_DEFINE(LIBTIFF_H, "tiffio.h", [libTIFF header filename.])
  else
    LIBTIFF_ERROR="libTIFF include files not found in default location!"
  fi
else
  AC_CHECK_HEADER($2/tiffio.h,[acx_libtiff_ok=yes])
  if test x$acx_libtiff_ok = xyes; then
    AC_DEFINE_UNQUOTED(LIBTIFF_H, "$2/tiffio.h", [libTIFF header filename.])
  else
    LIBTIFF_ERROR="libTIFF include files not found in $2!"
  fi
fi

dnl --------------------
dnl Search library files
dnl --------------------

LIBTIFF_LIBS=""
OLIBS="$LIBS"
LIBS=""
if test x$acx_libtiff_ok = xyes; then
  if test x$1 = x; then
    LIBTIFF_PATH=""
  else
    LIBTIFF_PATH="-L$1"
  fi
  if test x$3 = x; then
    LIBJPEG_PATH=""
  else
    LIBJPEG_PATH="-L$3"
  fi
  if test x$4 = x; then
    LIBZ_PATH=""
  else
    LIBZ_PATH="-L$4"
  fi
dnl Check jpeg and deflate library files (necessary for static libtiff)
  AC_CHECK_LIB(jpeg, jpeg_start_decompress, [acx_libtiff_ok=yes], [acx_libtiff_ok=no], [$LIBJPEG_PATH])
  if test x$acx_libtiff_ok = xyes; then
    AC_CHECK_LIB(z, deflate, [acx_libtiff_ok=yes], [acx_libtiff_ok=no], [$LIBZ_PATH])
    if test x$acx_libtiff_ok = xyes; then
      AC_CHECK_LIB(tiff, TIFFOpen, [acx_libtiff_ok=yes], [acx_libtiff_ok=no], [$LIBTIFF_PATH $LIBJPEG_PATH -ljpeg $LIBZ_PATH -lz -lm])
      if test x$acx_libtiff_ok = xyes; then
        AC_DEFINE(HAVE_LIBTIFF,1, [Define if you have the libTIFF libraries and header files.])
        LIBTIFF_LIBS="$LIBTIFF_PATH -ltiff $LIBJPEG_PATH -ljpeg $LIBZ_PATH -lz"
      else
        if test x$1 = x; then
          LIBTIFF_ERROR="libTIFF library files not found in $1!"
        else
          LIBTIFF_ERROR="libTIFF library files not found at the usual locations!"
        fi
      fi
    fi
  fi
fi

LIBS="$OLIBS"
if test x$acx_libtiff_ok = xyes; then
  AC_SUBST(LIBTIFF_LIBS)
  $5
else
  AC_SUBST(LIBTIFF_ERROR)
  $6
fi

])dnl ACX_LIBTIFF
