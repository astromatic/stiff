/*
*				image.c
*
* Convert FITS data.
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2003-2010 Emmanuel Bertin -- IAP/CNRS/UPMC
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
*	Last modified:		16/03/2012
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


#ifndef _IMAGE_H_
#define	_IMAGE_H_

#ifndef _TIFFIO_
#include LIBTIFF_H
#endif

#ifndef _FITSCAT_H_
#include "fits/fitscat.h"
#endif

#ifndef _FIELD_H_
#include "field.h"
#endif

/*----------------------------- Internal constants --------------------------*/
#define KBYTE           1024            /* 1 kbyte! */
#define MBYTE           (1024*KBYTE)    /* 1 Mbyte! */
#define	IMAGE_BUFSIZE	(8*MBYTE)	/* Image buffer for back. computation*/
#define	IMAGE_NLINES	256		/* Number of lines per batch */
#define	IMAGE_ROWS	8		/* Number of rows per strip */
#define	VIDEO_GAMMA	2.2		/* Standard Video gamma correction */

/*--------------------------------- typedefs --------------------------------*/
typedef struct structimage
  {
   char			filename[MAXCHAR];	/* Output filename */
   TIFF			*tiff;			/* TIFF structure pointer */
   int			width;			/* Image width */
   int			height;			/* Image height */
   int			nchan;			/* Number of channels */
   int			bpp;			/* Number of bits per pixel */
   int			bypp;			/* Number of bytes per pixel */
   int			fflag;			/* Float output flag */
   int			tilesize;		/* Tile size (or 0 for strips)*/
   int			ntilesx;		/* Number of tiles along x */
   int			ntilesy;		/* Number of tiles along y */
   int			tiley;			/* Current tile row */
   int			y;			/* Current line index */
   int			nlines;			/* Number of lines in buffer */
   unsigned char	*buf;
  }	imagestruct;

/*------------------------------- functions ---------------------------------*/
extern void	data_to_pix(fieldstruct **field, float **data, size_t offset,
			unsigned char *outpix, int npix, int nchan, int bypp,
			int fflag, float *buffer),
		image_convert_single(char *filename, fieldstruct **field,
			int nchan),
		make_imastats(fieldstruct *field,
			int backflag, int minflag, int maxflag);

extern char	*fitshead_to_desc(char *fitshead, int nheadblock,
			int sizex, int sizey, int binx, int biny,
			int flipxflag, int flipyflag);

extern int	image_convert_pyramid(char *filename, fieldstruct **field,
			int nchan),
		raster_to_tiles(unsigned char *inpix, unsigned char *outpix,
			int width, int tilesizey, int tilesize, int nbytes);

float		fast_median(float *arr, int n),
		fast_quantile(float *arr, int n, float frac);

#endif
