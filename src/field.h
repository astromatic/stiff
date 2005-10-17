 /*
 				field.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	STIFF
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	Handling of field structures.
*
*	Last modify:	28/01/2003
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifdef HAVE_CONFIG_H
#include	"config.h"
#endif

#ifndef _FITSCAT_H_
#include "fits/fitscat.h"
#endif

#ifndef _FIELD_H_
#define _FIELD_H_


/*----------------------------- Internal constants --------------------------*/
/*--------------------------------- typedefs --------------------------------*/

typedef struct field
  {
/* ---- file parameters */
  char		filename[MAXCHAR];	/* Image filename */
  char		*rfilename;		/* Pointer to the reduced image name */
  char		ident[MAXCHAR];		/* Field identifier (read from FITS) */
  catstruct	*cat;			/* Cat structure */
  tabstruct	*tab;			/* Selected structure */
  PIXTYPE	back;			/* Median */
  PIXTYPE	min;			/* Low cut */
  PIXTYPE	max;			/* High cut */
  }	fieldstruct;

/*------------------------------- functions ---------------------------------*/

extern fieldstruct	*load_field(char *filename);

extern void		end_field(fieldstruct *field),
			print_fieldinfo(fieldstruct *field);
#endif
