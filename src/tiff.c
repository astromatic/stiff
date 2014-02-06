/*
*				tiff.c
*
* Handle TIFF format
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
**	Copyright:		(C) 2003-2014 Emmanuel Bertin -- IAP/CNRS/UPMC
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
*	Last modified:		06/02/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifdef HAVE_CONFIG_H
#include	"config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include LIBTIFF_H
#include <unistd.h>

#include "define.h"
#include "globals.h"
#include "fits/fitscat.h"
#include "image.h"
#include "tiff.h"

int	tiff_compflag[] = {COMPRESSION_NONE, COMPRESSION_LZW, COMPRESSION_JPEG,
			COMPRESSION_DEFLATE, COMPRESSION_ADOBE_DEFLATE};

/****** create_tiff ***********************************************************
PROTO	imagestruct *create_tiff(char *filename, int width, int height,
			int nchan, int bpp, int tilesize,
			double *minvalue, double *maxvalue,
			int big_type, int compress_type, float compress_quality,
			char *copyright, char *description)
PURPOSE	Create a TIFF image (write a TIFF header and return an imagestruct).
INPUT	Output filename,
	image width in pixels,
	image height in pixels,
	number of channels (bytes),
	tilesize (pixels, 0 for stripped),
	TIFF compression type,
	JPEG compression quality,
	copyright string,
	description string.
OUTPUT	Pointer to an imagestruct.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	06/02/2014
 ***/
imagestruct *create_tiff(char *filename, int width, int height,
			int nchan, int bpp, int tilesize,
			double *minvalue, double *maxvalue,
			int big_type, int compress_type, int compress_quality,
			char *copyright, char *description)
  {
   TIFF		*tiff;
   imagestruct	*image;
   char		flagstr[8];

/* Handle BigTIFF format */
  strcpy(flagstr, "w");
  if (atof(TIFFGetVersion() + 16) >= 4.0)
    {
    if (big_type==BIGTIFF_ALWAYS)
      strcpy(flagstr, "w8");
    else if ((big_type==BIGTIFF_AUTO)
	&& ((size_t)width*(size_t)height*(size_t)(nchan*abs(bpp)/8))/(size_t)16
	>=134217728)
      {
      warning("Very large TIFF file: ", "switching to BigTIFF format");
      strcpy(flagstr, "w8");
      }
    }
  else
    {
    if (big_type==BIGTIFF_ALWAYS)
      warning("This version of libtiff does not support ", "BigTIFF format");
    else if ((big_type==BIGTIFF_AUTO)
	&& ((size_t)width*(size_t)height*(size_t)(nchan*abs(bpp)/8))/(size_t)16
	>=134217728)
      warning("Very large TIFF file, ",
	"but no BigTIFF support in this version of libtiff");
    }

  if ((tiff = TIFFOpen(filename, flagstr)) == NULL)
    error(EXIT_FAILURE, "*Error*: cannot open for writing ", filename);
  QCALLOC(image, imagestruct, 1);
  strcpy(image->filename, filename);
  image->tiff = tiff;

  create_tiffdir(image, width, height, nchan, bpp, tilesize, minvalue,
	maxvalue, compress_type, compress_quality, copyright, description);

  return image;
  }


/****** create_tiffdir ***********************************************************
PROTO	void create_tiffdir(imagestruct *image, int width, int height,
			int nchan, int bpp, int tilesize,
			double *minvalue, double *maxvalue,
			int compress_type, int compress_quality,
			char *copyright, char *description)
PURPOSE	Create a TIFF "directory" (a new TIFF subimage).
INPUT	image structure pointer,
	image width in pixels,
	image height in pixels,
	number of channels (bytes),
	tilesize (pixels, 0 for stripped),
	pointer to an array of minimum values (one per channel),
	pointer to an array of maximum values (one per channel),
	TIFF compression type,
	JPEG compression quality,
	copyright string,
	description string.
OUTPUT	-.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	06/02/2014
 ***/
void	create_tiffdir(imagestruct *image, int width, int height,
			int nchan, int bpp, int tilesize,
			double *minvalue, double *maxvalue,
			int compress_type, int compress_quality,
			char *copyright,
			char *description)
  {
   TIFF		*tiff;
   time_t	thetime;
   struct tm	*tm;
   char		datetimeb[32],
		psuserb[MAXCHAR],
		pshostb[MAXCHAR],
		pssoft[MAXCHAR],
		*psuser, *pshost;
   int		nbytes;

  tiff = image->tiff;

  if (image->buf)
    {
    _TIFFfree(image->buf);
/*-- This is not the first sub-image */
    TIFFWriteDirectory(tiff);
    }
  else
    {
    image->width = width;
    image->height = height;
    image->nchan = nchan;
    image->fflag = (bpp<0);
    image->bpp = abs(bpp);
    image->bypp = abs(bpp/8);
    }

  TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, nchan);
  TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, abs(bpp));
  if (bpp<0)
    {
    TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
#ifdef TIFFTAG_PERSAMPLE
    if(nchan>1)
      {
/*---- Hacky TIFF feature: see http://www.asmail.be/msg0055458208.html */
      TIFFSetField(tiff, TIFFTAG_PERSAMPLE, PERSAMPLE_MULTI);
      TIFFSetField(tiff, TIFFTAG_SMINSAMPLEVALUE, minvalue);
      TIFFSetField(tiff, TIFFTAG_SMAXSAMPLEVALUE, maxvalue);
      TIFFSetField(tiff, TIFFTAG_PERSAMPLE, PERSAMPLE_MERGED);
      }
    else
#endif
      {
      TIFFSetField(tiff, TIFFTAG_SMINSAMPLEVALUE, *minvalue);
      TIFFSetField(tiff, TIFFTAG_SMAXSAMPLEVALUE, *maxvalue);
      }
   }
  TIFFSetField(tiff, TIFFTAG_COMPRESSION, tiff_compflag[compress_type]);
  if (tiff_compflag[compress_type] == COMPRESSION_JPEG)
    {
    TIFFSetField(tiff, TIFFTAG_JPEGQUALITY, compress_quality);
/*
    TIFFSetField(tiff, TIFFTAG_YCBCRSUBSAMPLING,
	downsamp_type==DOWNSAMP_444? 1:2,
	downsamp_type==DOWNSAMP_420? 2:1);
*/
    }
  TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC,
		nchan==1? PHOTOMETRIC_MINISBLACK:PHOTOMETRIC_RGB);
  TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
  TIFFSetField(tiff, TIFFTAG_XRESOLUTION, 72.0);
  TIFFSetField(tiff, TIFFTAG_YRESOLUTION, 72.0);

/* Copyright and image description */
  TIFFSetField(tiff, TIFFTAG_COPYRIGHT, copyright);
  TIFFSetField(tiff, TIFFTAG_IMAGEDESCRIPTION, description);


/* Software */
  sprintf(pssoft, "%s version %s (%s)\n", BANNER,MYVERSION,DATE);
  TIFFSetField(tiff, TIFFTAG_SOFTWARE, pssoft);


/* Date and time */
  thetime = time(NULL);
  tm = localtime(&thetime);
  sprintf(datetimeb,"%04d:%02d:%02d %02d:%02d:%02d",
	tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
	tm->tm_hour, tm->tm_min, tm->tm_sec);
  TIFFSetField(tiff, TIFFTAG_DATETIME, datetimeb);

/* Username and host computer */
  psuser = pshost = NULL;
#ifdef HAVE_GETENV
  if (!(psuser = getenv("USERNAME")))	/* Cygwin,... */
    if (!(psuser = getenv("LOGNAME")))	/* Linux,... */
      psuser = getenv("USER");
  if (!(pshost = getenv("HOSTNAME")))
    pshost = getenv("HOST");
#endif

  if (!psuser)
    {
    getlogin_r(psuserb, MAXCHAR);
    psuser = psuserb;
    }
  TIFFSetField(tiff, TIFFTAG_ARTIST, psuser);

  if (!pshost)
    {
    gethostname(pshostb, MAXCHAR);
    pshost = pshostb;
    }
  TIFFSetField(tiff, TIFFTAG_HOSTCOMPUTER, pshost);

  if (tilesize)
    {
    image->tilesize = tilesize;
    image->ntilesx = (width+tilesize-1)/tilesize;
    image->ntilesy = (height+tilesize-1)/tilesize;
    TIFFSetField(tiff, TIFFTAG_TILEWIDTH, tilesize);
    TIFFSetField(tiff, TIFFTAG_TILELENGTH, tilesize);
    image->buf = _TIFFmalloc(image->ntilesx*tilesize*tilesize*nchan
			*image->bypp);
    }
  else
    {
    image->nlines = IMAGE_NLINES;
    nbytes = nchan*width*image->bypp*image->nlines;
    if (TIFFScanlineSize(tiff)*image->nlines < nbytes)
      image->buf = (unsigned char *)_TIFFmalloc(nbytes);
    else
      image->buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tiff)
		*image->nlines);
    TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, IMAGE_ROWS);
    }

  return;
  }


/****** write_tifflines *******************************************************
PROTO	int	write_tifflines(imagestruct *image)
PURPOSE	Write a bunch of pixels in a TIFF image
INPUT	Pointer to the image structure.
OUTPUT	RETURN_OK if OK, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	13/01/2010
 ***/
int	write_tifflines(imagestruct *image)
  {
   int	n,y, step, nstrip;

  y = image->y / IMAGE_ROWS;
  step = image->nchan*image->bypp*image->width*IMAGE_ROWS;
  nstrip = (image->nlines+IMAGE_ROWS-1)/IMAGE_ROWS;
  for (n=0; n<nstrip; n++)
    if (TIFFWriteEncodedStrip(image->tiff, y++,
	(tdata_t *)(image->buf+n*step), (size_t)step) < 0)
      return RETURN_ERROR;

  return RETURN_OK;
  }


/****** write_tifftiles *******************************************************
PROTO	int	write_tiff(imagestruct *image)
PURPOSE	Write a row of tiles
INPUT	Pointer to the image structure,
	current vertical tile index.
OUTPUT	RETURN_OK if OK, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	03/01/2010
 ***/
int	write_tifftiles(imagestruct *image)
  {
   unsigned char	*buft;
   int			x, nx, npix;

  nx = image->ntilesx;
  npix = image->tilesize*image->tilesize*image->nchan*image->bypp;
  buft = image->buf;
  for (x=0; x<nx; x++)
    {
   if (TIFFWriteTile(image->tiff, buft,
	x*image->tilesize, image->tiley*image->tilesize, 0, 0) < 0)
      return RETURN_ERROR;
    buft += npix;
    }

  return RETURN_OK;
  }


/****** end_tiff **************************************************************
PROTO	void	end_tiff(imagestruct *image)
PURPOSE	Terminate everything related to a TIFF file.
INPUT	Pointer to the image structure.
OUTPUT	Computed background.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	01/12/2009
 ***/
void	end_tiff(imagestruct *image)
  {
  if (!image)
    return;
  TIFFClose(image->tiff);
  if (image->buf)
    _TIFFfree(image->buf);
  free(image);

  return;
  }
