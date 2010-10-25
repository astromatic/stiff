/*
*				field.h
*
* Include file for field.c.
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
*	Last modified:		13/10/2010
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

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
  int		size[2];		/* Image size */
  PIXTYPE	back;			/* Median */
  PIXTYPE	min;			/* Low cut */
  PIXTYPE	max;			/* High cut */
  }	fieldstruct;

/*------------------------------- functions ---------------------------------*/

extern fieldstruct	*load_field(char *filename);

extern void		end_field(fieldstruct *field),
			print_fieldinfo(fieldstruct *field);
#endif
