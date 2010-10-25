/*
*				datamem.c
*
* Handle memory allocation for large image data.
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2009-2010 Emmanuel Bertin -- IAP/CNRS/UPMC
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

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#ifdef	HAVE_SYS_MMAN_H
#include	<sys/mman.h>
#endif
#include	"fits/fitscat_defs.h"
#include	"fits/fitscat.h"
#include	"datamem.h"

size_t	data_maxram = BODY_DEFRAM,
	data_maxvram = BODY_DEFVRAM,
	data_ramleft, data_vramleft, data_ramflag;

int	data_vmnumber;

char	data_swapdirname[MAXCHARS] = BODY_DEFSWAPDIR;

/******* alloc_data ***********************************************************
PROTO	float *alloc_data(size_t ndata, char **swapname)
PURPOSE	Allocate memory for image data. If not enough RAM is available, a swap
	file is created.
INPUT	Number of pixels.
OUTPUT	Pointer to the mapped data if OK, or NULL otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	19/12/2009
 ***/
float	*alloc_data(size_t ndata, char **swapname)
  {
   float	*data;
   size_t	size;
   int		fd;

  *swapname = NULL;
  if (!data_ramflag)
    {
    data_ramleft = data_maxram;
    data_vramleft = data_maxvram;
    data_ramflag = 1;
    }

/* Return a NULL pointer if size is zero */
  if (!ndata)
    return (float *)NULL;

/* Decide if the data will go in physical memory or on swap-space */
  size = ndata*sizeof(float);
  if (size < data_ramleft)
    {
/*-- There should be enough RAM left: try to do a malloc() */
    if ((data = (float *)malloc(size)))
      {
      data_ramleft -= size;

      return data;
      }
    else
      data = (float *)NULL;
    }

  if (size < data_vramleft)
    {
/*-- Convert and copy the data to a swap file, and mmap() it */
    QMALLOC(*swapname, char, MAXCHARS);
    sprintf(*swapname, "%s/vm%05ld_%05x.tmp",
		data_swapdirname, (long)getpid(),
		(unsigned int)++data_vmnumber) ;
    if ((fd=open(*swapname, O_RDWR|O_CREAT|O_TRUNC, 0666)) == -1)
      error(EXIT_FAILURE, "*Error*: cannot create swap-file ", *swapname);
    add_cleanupfilename(*swapname);
    lseek(fd, size - 1, SEEK_SET);
    write(fd, "\0", 1);
    data = mmap(NULL,size,PROT_WRITE|PROT_READ,MAP_SHARED, fd, (off_t)0);
    close(fd);
    data_vramleft -= size;

/*-- Memory mapping problem */
    if (data == (void *)-1)
      return (float *)NULL;
    return data;
    }

/* If no memory left at all: forget it! */
  return NULL;
  }


/******* free_data ************************************************************
PROTO	void free_data(float *data, int ndata, char *swapname)
PURPOSE	Free image data.
INPUT	Pointer to the data,
	number of pixels,
	swap filename.
OUTPUT	-.
NOTES	.
AUTHOR	E. Bertin (IAP)
VERSION	19/12/2009
 ***/
void	free_data(float *data, size_t ndata, char *swapname)

  {
   size_t	size;

/* Free the body! (if allocated) */
  if (data)
    {
    size = ndata*sizeof(float);
    if (swapname)
      {
      if (munmap(data, size))
        warning("Can't unmap virtual memory file ", swapname);
      data_vramleft += size;
      if (unlink(swapname))
        warning("Can't delete ", swapname);
      remove_cleanupfilename(swapname);
      free(swapname);
      }
    else
      {
      QFREE(data);
      data_ramleft += size;
      }
    }

  return;
  }


/******* set_maxdataram *******************************************************
PROTO	int set_maxdataram(size_t maxram)
PURPOSE	Set the maximum amount of silicon memory that can be allocated for
	storing image data.
INPUT	The maximum amount of RAM (in bytes).
OUTPUT	RETURN_OK if within limits, RETURN_ERROR otherwise.
NOTES	.
AUTHOR	E. Bertin (IAP)
VERSION	18/12/2009
 ***/
int set_maxdataram(size_t maxram)
  {

  if (maxram<1)
    return RETURN_ERROR;

  data_maxram = maxram*(size_t)(1024*1024);

  return RETURN_OK;
  }


/******* set_maxdatavram ******************************************************
PROTO	int set_maxdatavram(size_t maxram)
PURPOSE	Set the maximum amount of total virtual memory that can be used for
	storing pixel data.
INPUT	The maximum amount of VRAM (in bytes).
OUTPUT	RETURN_OK if within limits, RETURN_ERROR otherwise.
NOTES	.
AUTHOR	E. Bertin (IAP)
VERSION	18/12/2009
 ***/
int set_maxdatavram(size_t maxvram)
  {

  if (maxvram<1)
    return RETURN_ERROR;

  data_maxvram = maxvram*(size_t)(1024*1024);

  return RETURN_OK;
  }


/******* set_dataswapdir ******************************************************
PROTO	int set_dataswapdir(char *dirname)
PURPOSE	Set the path name of the directory that will be used for storing
	memory swap files.
INPUT	The pointer to the path string.
OUTPUT	RETURN_OK if path appropriate, RETURN_ERROR otherwise.
NOTES	.
AUTHOR	E. Bertin (IAP)
VERSION	05/12/2009
 ***/
int set_dataswapdir(char *dirname)
  {

  if (!dirname)
    return RETURN_ERROR;

  strcpy(data_swapdirname, dirname);

  return RETURN_OK;
  }


