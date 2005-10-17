# Program Makefile for Stiff
# Copyright (C) 2003 Emmanuel Bertin.
SUBDIRS			= fits
bin_PROGRAMS		= stiff
stiff_SOURCES		= field.c image.c main.c makeit.c prefs.c tiff.c \
			  define.h field.h globals.h image.h key.h prefs.h \
			  preflist.h tiff.h types.h
stiff_LDADD		= $(top_builddir)/src/fits/libfits.a
DATE=`date +"%Y-%m-%d"`
