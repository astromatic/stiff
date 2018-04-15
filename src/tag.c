/*
*				tag.c
*
* Handle (channel) tags.
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2003-2016 IAP/CNRS/UPMC
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
*	Last modified:		08/06/2016
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifdef HAVE_CONFIG_H
#include	"config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "define.h"
#include "globals.h"
#include "fits/fitscat.h"
#include "field.h"
#include "key.h"
#include "prefs.h"

static int tag_comp(const void *field1, const void *field2);

/****** tag_fields ***********************************************************
PROTO   void tag_fields(fieldstruct **fields, int nfield)
PURPOSE Tag and sort input fields.
INPUT   Array of pointers to fields,
	number of fields.
OUTPUT  -.
NOTES   Global preferences are used.
AUTHOR  E. Bertin (IAP)
VERSION 08/06/2016
*/
void	tag_fields(fieldstruct **fields, int nfield)
  {
   tabstruct	*tab;
   fieldstruct	*field;
   int		f, fmax;

/* Tag fields */
  fmax = nfield;
  for (f=0; f<nfield; f++) {
    field = fields[f];
    tab = field->tab;
    if (prefs.channeltag_type == CHANNELTAGTYPE_MANUAL)
      strcpy(field->channeltag, prefs.channel_tags[f]);
    else if (!tab->headbuf || fitsread(tab->headbuf, prefs.channeltag_key,
	field->channeltag, H_STRING,T_STRING)!= RETURN_OK)
      strcpy(field->channeltag, "");
    if (prefs.channeltag_type == CHANNELTAGTYPE_MATCH
	&& prefs.channel_tags[0][0]) {
      field->index = tag_match(field->channeltag, prefs.channel_tags,
	prefs.nchannel_tags);
      if (field->index>=0)
/*------ Replace channel tag with the one from the list that matches */
        strcpy(field->channeltag, prefs.channel_tags[field->index]);
      else
/*------ Push to the end if tag unknown */
	field->index = fmax++;
    } else
      field->index = f;
  }

/* Match and sort fields */
  if (prefs.channeltag_type == CHANNELTAGTYPE_MATCH) {
    qsort(fields, nfield, sizeof(fieldstruct *), tag_comp);
  }

  return;
  }


/****** tag_match *************************************************************
PROTO   int tag_match(char *tag, char **taglist, int ntaglist)
PURPOSE Match a tag in a list of tags.
INPUT   Tag string,
	tag list as an array of strings (beginning with 0),
	number of tags in taglist.
OUTPUT  Position in the tag list if the tag was matched, RETURN_ERROR otherwise.
NOTES   The search is case insensitive.
AUTHOR  E. Bertin (IAP)
VERSION 08/06/2016
*/
int	tag_match(char *tag, char **taglist, int ntaglist)

  {
  int i;

  if (*tag)
    for (i=0; i<ntaglist; i++)
      if (!cistrcmp(tag, taglist[i], FIND_NOSTRICT))
        return i;

  return RETURN_ERROR;
  }

/****** tag_comp *************************************************************
PROTO   int tag_comp(const void *field1, const void *field2)
PURPOSE Compare the tag indices of two fields.
INPUT   Pointer to field #1,
	pointer to field #2.
OUTPUT  1 if field #1 tag index > field #2 tag index, 0 if they are equal, 0
	otherwise.
NOTES   -.
AUTHOR  E. Bertin (IAP)
VERSION 07/06/2016
*/
int	tag_comp(const void *field1, const void *field2)

  {
  return ((*(fieldstruct **)field1)->index > (*(fieldstruct **)field2)->index) ? 1
	: ((*(fieldstruct **)field1)->index == (*(fieldstruct **)field2)->index ?
		0 : -1);
  }


