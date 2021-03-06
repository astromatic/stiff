/*
*				tag.h
*
* Include file for tag.c.
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

#ifndef _TAG_H_
#define _TAG_H_

#ifndef _FIELD_H_
#include	"field.h"
#endif

/*--------------------------------- Constants -------------------------------*/
#define		MAXTAG		1024	/* Maximum number of tags */

/*--------------------------------- typedefs --------------------------------*/
/*------------------------------- functions ---------------------------------*/
int	tag_match(char *tag, char **taglist, int ntaglist);

void	tag_fields(fieldstruct **fields, int nfield);
#endif
