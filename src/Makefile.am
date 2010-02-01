# Program Makefile for Stiff
# Copyright (C) 2003-2010 Emmanuel Bertin.
SUBDIRS			= fits
bin_PROGRAMS		= stiff
stiff_SOURCES		= datamem.c field.c image.c main.c makeit.c prefs.c \
			  threads.c tiff.c xml.c \
			  datamem.h define.h field.h globals.h image.h \
			  key.h prefs.h preflist.h threads.h tiff.h types.h \
			  xml.h
stiff_LDADD		= $(top_builddir)/src/fits/libfits.a
DATE=`date +"%Y-%m-%d"`

