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
*	Last modify:	18/01/2003
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/


#ifndef _IMAGE_H_
#include "image.h"
#endif

/*--------------------------------- typedefs --------------------------------*/
typedef struct structtag
  {
  unsigned short	tag;
  unsigned short	type;
  int			count;
  union v      {unsigned char	byte;
		char		character;
		unsigned short	ushort;
		unsigned int	uint;
		int		rational;
	       } value;
  }	tagstruct;

/*------------------------------- functions ---------------------------------*/
extern imagestruct	*create_tiff(char *filename, int width, int height,
				int nbytes);

extern void		end_tiff(imagestruct *image),
			write_tiff(imagestruct *image, unsigned char *pixbuf,
				int npix);
