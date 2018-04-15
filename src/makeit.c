/*
*				makeit.c
*
* Main program.
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2003-2016 IAP/CNRS/UPMC
*
*	License:		GNU General Public License
*
*	STIFF is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*	STIFF is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*	You should have received a copy of the GNU General Public License
*	along with STIFF. If not, see <http://www.gnu.org/licenses/>.
*
*	Last modified:		07/06/2016
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include LIBTIFF_H

#include "define.h"
#include "globals.h"
#include "fits/fitscat.h"
#include "field.h"
#include "image.h"
#include "key.h"
#include "prefs.h"
#include "xml.h"


extern pkeystruct	key[];
extern char		keylist[][32];
time_t			thetime, thetime2;
double			dtime;

/********************************** makeit ***********************************/
void	makeit(void)
  {
   struct tm		*tm;
   fieldstruct		**fields;
   PIXTYPE		grey;
   float		ver;
   char			verstr[MAXCHAR], imtype[MAXCHAR],
			*rfilename;
   int			f,i, w,h, nfield, nbit, level;

/* Install error logging */
//  error_installfunc(write_error);

/* Processing start date and time */
  dtime = counter_seconds();
  thetime = time(NULL);
  tm = localtime(&thetime);
  sprintf(prefs.sdate_start,"%04d-%02d-%02d",
	tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
  sprintf(prefs.stime_start,"%02d:%02d:%02d",
	tm->tm_hour, tm->tm_min, tm->tm_sec);

  NFPRINTF(OUTPUT, "");
  QPRINTF(OUTPUT,
        "----- %s %s started on %s at %s with %d thread%s\n\n",
                BANNER,
                MYVERSION,
                prefs.sdate_start,
                prefs.stime_start,
                prefs.nthreads,
                prefs.nthreads>1? "s":"");

  strcpy(verstr, TIFFGetVersion());
  if ((ver=atof(verstr + 16)) >= 4.0)
    NPRINTF(OUTPUT, "> BigTIFF support is: ON (libTIFF V%3.1f)\n\n", ver);
  else
    NPRINTF(OUTPUT, "> BigTIFF support is: OFF (libTIFF V%3.1f)\n\n", ver);

/* Load input images */
  nfield = prefs.nfile;
  QMALLOC(fields, fieldstruct *, nfield);
/* Go argument by argument */
  NFPRINTF(OUTPUT, "Examining input data...");
  NFPRINTF(OUTPUT, "");

/* Initialize the XML stack */
  if (prefs.xml_flag)
    init_xml(nfield);

/* Read the FITS files */
  QPRINTF(OUTPUT, "----- Inputs:\n");
  for (f=0; f<nfield; f++)
    fields[f] = load_field(prefs.file_name[f]);

/* Tag fields */
  tag_fields(fields, nfield);

/* Compute/set flux rescaling */
  for (f=0; f<nfield; f++)
    {
    i = fields[f]->index;
    fields[f]->back = prefs.back_val[i];
    fields[f]->max = prefs.max_val[i];
    fields[f]->min = prefs.min_val[i];
    if (prefs.back_type[i]!=BACK_MANUAL || prefs.max_type[i]==MAX_QUANTILE
	 ||  prefs.min_type[i]==MIN_QUANTILE)
      make_imastats(fields[f],
	prefs.back_type[i]!=BACK_MANUAL, prefs.min_type[i]==MIN_QUANTILE,
	prefs.max_type[i]==MAX_QUANTILE);
    if (fields[f]->max > prefs.sat_val[i])
      warning("MAX_LEVEL exceeds SATUR_LEVEL in ", fields[f]->rfilename);

    if (prefs.min_type[i] == MIN_GREYLEVEL)
      {
      grey = pow(prefs.min_val[i], prefs.gamma_fac);
      fields[f]->min = (fields[f]->max*grey - fields[f]->back) / (grey - 1.0);
      }
    print_fieldinfo(fields[f]);
    }

/* Display Output image characteristics */
/* A short, "relative" version of the filename */
  if (!(rfilename = strrchr(prefs.tiff_name, '/')))
    rfilename = prefs.tiff_name;
  else
    rfilename++;

  w = prefs.bin_size[0]>1?
	  (fields[0]->tab->naxisn[0]+prefs.bin_size[0]-1)/prefs.bin_size[0]
	: fields[0]->tab->naxisn[0];
  h = prefs.bin_size[1]>1?
	  (fields[0]->tab->naxisn[1]+prefs.bin_size[1]-1)/prefs.bin_size[1]
	: fields[0]->tab->naxisn[1];

  prefs.nlines = (double)h;
  prefs.npix = (double)w*(double)h;
  
  nbit = abs(prefs.bpp);
  if (prefs.bpp>0)
    sprintf(imtype,"integers");
  else
    sprintf(imtype,"floats");
  QPRINTF(OUTPUT, "\n----- Output:\n");
  for (level = 1;
	((prefs.format_type2 == FORMAT_TIFF_PYRAMID)
		&& (w>=prefs.min_size[0] || h>=prefs.min_size[1]))
	|| level<2;
	level++, w/=2,h/=2)
    QPRINTF(OUTPUT, "%s: %7dx%-7d  %4dx%-2d bits (%s) gamma: x%4.2f  compression: %s \n",
        rfilename,
	w,
	h,
	nfield,
	nbit,
        imtype,
	prefs.gamma_fac,
	key[findkeys("COMPRESSION_TYPE", keylist,
		FIND_STRICT)].keylist[prefs.compress_type]);

  QPRINTF(OUTPUT, "\n");

/* Do the conversion */
  if (prefs.format_type2 == FORMAT_TIFF_PYRAMID)
    image_convert_pyramid(prefs.tiff_name, fields, nfield);
  else
    image_convert_single(prefs.tiff_name, fields, nfield);

/* Update the output field meta-data */
  for (f=0; f<nfield; f++)
    if (prefs.xml_flag)
      update_xml(fields[f]);

/* Processing end date and time */
  thetime2 = time(NULL);
  tm = localtime(&thetime2);
  sprintf(prefs.sdate_end,"%04d-%02d-%02d",
	tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
  sprintf(prefs.stime_end,"%02d:%02d:%02d",
	tm->tm_hour, tm->tm_min, tm->tm_sec);
  prefs.time_diff = counter_seconds() - dtime;

/* Write XML */
  if (prefs.xml_flag)
    {
    write_xml(prefs.xml_name);
    end_xml();
    }

/* Free memory */
 for (f=0; f<nfield; f++)
    end_field(fields[f]);
  free(fields);

  return;
  }


/****** write_error ********************************************************
PROTO	void	write_error(char *msg1, char *msg2)
PURPOSE	Manage files in case of a catched error
INPUT	a character string,
	another character string
OUTPUT	RETURN_OK if everything went fine, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	26/07/2006
 ***/
void	write_error(char *msg1, char *msg2)
  {
   char	error[MAXCHAR];

  sprintf(error, "%s%s", msg1,msg2);
  if (prefs.xml_flag)
    write_xmlerror(prefs.xml_name, error);
  end_xml();

  return;
  }


/****** counter_seconds *******************************************************
PROTO	static	double counter_seconds(void)
PURPOSE	Count the number of seconds (with an arbitrary offset).
INPUT	-.
OUTPUT	Returns a number of seconds.
NOTES	Results are meaningful only for tasks that take one microsec or more.
AUTHOR	E. Bertin (IAP)
VERSION	24/09/2009
 ***/
double	counter_seconds(void)
  {
   struct timeval	tp;
   struct timezone	tzp;
   int	dummy;

  dummy = gettimeofday(&tp,&tzp);
  return (double) tp.tv_sec + (double) tp.tv_usec * 1.0e-6;
  }


