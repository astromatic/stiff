/*
*				tiff.h
*
* Include file for tiff.c.
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2003-2014 Emmanuel Bertin -- IAP/CNRS/UPMC
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


#ifndef _IMAGE_H_
#include "image.h"
#endif

/*---------------------------------- defines --------------------------------*/
#define	BIGTIFF_AUTO	0	/* Switch to BigTIFF for large images */
#define	BIGTIFF_NEVER	1	/* Never use BigTIFF */
#define	BIGTIFF_ALWAYS	2	/* Always use BigTIFF */

#define DOWNSAMP_444	0	/* Chrominance at full resolution */
#define DOWNSAMP_422	1	/* Chrominance at half-resolution in x */
#define DOWNSAMP_420	2	/* Chrominance at half-resolution in x and y */

/*--------------------------------- typedefs --------------------------------*/

/*------------------------------- functions ---------------------------------*/
extern imagestruct	*create_tiff(char *filename, int width, int height,
				int nchan, int bpp, int tilesize,
				double *minvalue,double *maxvalue, int big_type,
				int compress_type, int compress_quality,
				char *copyright, char *description);

extern int		write_tifflines(imagestruct *image),
			write_tifftiles(imagestruct *image);

extern void		create_tiffdir(imagestruct *image, int width,
				int height, int bpp, int nchan, int tilesize,
				double *minvalue, double *maxvalue,
				int compress_type, int compress_quality,
				char *copyright, char *description),
			end_tiff(imagestruct *image);

