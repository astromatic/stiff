 /*
 				tiff.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	STIFF
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	include for tiff.c.
*
*	Last modify:	12/01/2010
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/


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
				int nchan, int bpp, int tilesize, int big_type,
				int compress_type, int compress_quality,
				char *copyright, char *description);

extern int		write_tifflines(imagestruct *image),
			write_tifftiles(imagestruct *image);

extern void		create_tiffdir(imagestruct *image, int width,
				int height, int bpp, int nchan, int tilesize,
				int compress_type, int compress_quality,
				char *copyright, char *description),
			end_tiff(imagestruct *image);

