 /*
 				image.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	STIFF
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	include for image.c.
*
*	Last modify:	13/01/2010
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifndef _IMAGE_H_
#define	_IMAGE_H_

#ifndef _TIFFIO_
#include <tiffio.h>
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
			float *buffer),
		image_convert_single(char *filename, fieldstruct **field,
			int nchan),
		make_imastats(fieldstruct *field,
			int backflag, int minflag, int maxflag);

extern int	image_convert_pyramid(char *filename, fieldstruct **field,
			int nchan),
		raster_to_tiles(unsigned char *inpix, unsigned char *outpix,
			int width, int tilesizey, int tilesize, int nbytes);

float		fast_median(float *arr, int n),
		fast_quantile(float *arr, int n, float frac);

#endif
