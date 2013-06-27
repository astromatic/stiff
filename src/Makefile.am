#
#				Makefile.am
#
# src Makefile. Process this file with automake to generate a Makefile
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#
#	This file part of:	STIFF
#
#	Copyright:		(C) 2003-2013 Emmanuel Bertin -- IAP/CNRS/UPMC
#
#	License:		GNU General Public License
#
#	SExtractor is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
# 	(at your option) any later version.
#	SExtractor is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#	You should have received a copy of the GNU General Public License
#	along with SExtractor. If not, see <http://www.gnu.org/licenses/>.
#
#	Last modified:		27/06/2013
#
#%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

SUBDIRS			= fits
bin_PROGRAMS		= stiff
stiff_SOURCES		= datamem.c field.c image.c main.c makeit.c prefs.c \
			  threads.c tiff.c xml.c \
			  datamem.h define.h field.h globals.h image.h \
			  key.h prefs.h preflist.h threads.h tiff.h types.h \
			  xml.h
stiff_LDADD		= $(srcdir)/fits/libfits.a
DATE=`date +"%Y-%m-%d"`

