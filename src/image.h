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
*	Last modify:	13/11/2004
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifndef _IMAGE_H_
#define	_IMAGE_H_

#ifndef _FITSCAT_H_
#include "fits/fitscat.h"
#endif

#ifndef _FIELD_H_
#include "field.h"
#endif

/*----------------------------- Internal constants --------------------------*/
#define KBYTE           1024            /* 1 kbyte! */
#define MBYTE           (1024*KBYTE)    /* 1 Mbyte! */
#define	IMAGE_BUFSIZE	(4*MBYTE)	/* Image buffer for back. computation*/
#define	VIDEO_GAMMA	2.2		/* Standard Video gamma correction */

/*--------------------------------- typedefs --------------------------------*/
typedef struct structimage
  {
   char		filename[MAXCHAR];
   FILE		*file;
   int		width;
   int		height;
   int		nbytes;
  }	imagestruct;

/*------------------------------- functions ---------------------------------*/
extern void	image_convert(char *filename, fieldstruct **field, int ncat),
		make_imastats(fieldstruct *field,
			int backflag, int minflag, int maxflag);

float		fast_median(float *arr, int n),
		fast_quantile(float *arr, int n, float frac);

#endif
