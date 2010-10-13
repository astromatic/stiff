/*
*				field.c
*
* Handle fields (images).
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2003-2010 IAP/CNRS/UPMC
*
*	Author:			Emmanuel Bertin (IAP)
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "define.h"
#include "globals.h"
#include "fits/fitscat.h"
#include "field.h"
#include "prefs.h"

/****** load_field ***********************************************************
PROTO   fieldstruct *load_field(char *filename)
PURPOSE Load field infos.
INPUT   Character string that contains the file name.
OUTPUT  A pointer to the created field structure.
NOTES   Global preferences are used. The function is not reentrant because
	of static variables (prefs structure members are updated).
AUTHOR  E. Bertin (IAP)
VERSION 05/10/2010
*/
fieldstruct	*load_field(char *filename)
  {
   tabstruct	*tab;
   fieldstruct	*field;
   char		*rfilename, *str, *str2;
   int		ext;
   
   if ((str = strrchr(filename, '[')))
    {
    *str = '\0';
    if ((str2 = strrchr(str, ']')))
      *str2 = '\0';
    }

  QCALLOC(field, fieldstruct, 1);

/* A short, "relative" version of the filename */
  if (!(rfilename = strrchr(filename, '/')))
    rfilename = filename;
  else
    rfilename++;
  sprintf(gstr,"Examining File %s", rfilename);
  NFPRINTF(OUTPUT, gstr);

  if (! (field->cat = read_cat(filename)))
    error(EXIT_FAILURE, "*Error*: no FITS data in ", filename);
  if (str)
    {
    if (!(tab = name_to_tab(field->cat, str+1, 0)))
      {
      ext = atoi(str+1);
      tab = pos_to_tab(field->cat, ext, 0);
      }
    }
  else
    tab = field->cat->tab;

  field->tab = tab;
/*-- Force the data to be at least 2D */
  if (tab->naxis<2)
    error(EXIT_FAILURE, "*Error*: no 2D FITS data in ", filename);

/*-- Force unsigned int */
  if (tab->bitsgn && prefs.fitsunsigned_flag)
    tab->bitsgn = 0;

/*-- Get image dimensions */
  field->size[0] = tab->naxisn[0];
  field->size[1] = tab->naxisn[1];

/* A short, "relative" version of the filename (bis) */
  if (!(rfilename = strrchr(field->cat->filename, '/')))
    rfilename = field->cat->filename;
  else
    rfilename++;

  field->rfilename = rfilename;

/* Identifier */
  if (!tab->headbuf || fitsread(tab->headbuf, "OBJECT  ", field->ident,
	H_STRING,T_STRING)!= RETURN_OK)
    strcpy(field->ident, "no ident");

  return field;
  }


/****** end_field *************************************************************
PROTO   void end_field(fieldstruct *field)
PURPOSE Terminate a field structure.
INPUT   Pointer to the field.
OUTPUT	-.
NOTES   -.
AUTHOR  E. Bertin (IAP)
VERSION 25/01/2003
*/
void	end_field(fieldstruct *field)
  {
  free_cat(&field->cat, 1);
  free(field);

  return;
  }


/****** print_fieldinfo ******************************************************
PROTO	void print_fieldinfo(fieldstruct *field)
PURPOSE	Print info about a FITS file
INPUT	Pointer to the field.
OUTPUT	-.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	04/01/2010
 ***/
void	print_fieldinfo(fieldstruct *field)

  {
   tabstruct		*tab, *tabo;
   static char		str[82];
   int			t;

  if (!(tab=field->tab))
    return;
  if (field->cat->ntab>1)
    {
    if (tab->extname)
      sprintf(str, "[%s]", tab->extname);
    else
      {
      tabo = field->cat->tab;
      for (t=0; tabo!=tab; t++)
        tabo = tabo->nexttab;
      sprintf(str, "[%d/%d]", t, field->cat->ntab-1);
      }
    }
  else
    *str = '\0';
  QPRINTF(OUTPUT, "%s: \"%.20s\" %s %dx%d   %d bits (%s)\n"
	"Background level: %-10g  Min level: %-10g  Max level: %-10g\n",
        field->rfilename, field->ident,
        str,
        tab->naxisn[0], tab->naxisn[1], tab->bytepix*8,
        tab->bitpix>0? (tab->compress_type!=COMPRESS_NONE ?
                        "compressed":"integers") : "floats",
	field->back, field->min, field->max);

  return;
  }

