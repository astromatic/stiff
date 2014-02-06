/*
*				image.c
*
* Convert FITS data.
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2003-2014 Emmanuel Bertin -- IAP/CNRS/UPMC
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
*	Last modified:		06/02/2014
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
#include "datamem.h"
#include "field.h"
#include "image.h"
#include "prefs.h"
#include "fits/fitscat.h"
#include "tiff.h"
#ifdef USE_THREADS
#include "threads.h"

   void			pthread_cancel_threads(void);
   static void		*pthread_data_to_pix(void *arg),
			*pthread_write_lines(void *arg),
			*pthread_write_tiles(void *arg);
   pthread_t		*thread, tthread;
   pthread_mutex_t	tiffmutex;
   threads_gate_t	*pthread_startgate, *pthread_stopgate,
			*pthread_startwgate, *pthread_stopwgate;

   fieldstruct		**pthread_field;
   imagestruct		*pthread_image;
   OFF_T		pthread_imoffset;
   float		**pthread_data,
			**pthread_fsbuf;
   unsigned char	*pthread_pix;
   int			*proc,
			pthread_nbuflines, pthread_bufline, pthread_width,
			pthread_nchan, pthread_bypp, pthread_fflag,
			pthread_nproc, pthread_endflag;

#endif

/****** image_convert_single **************************************************
PROTO	void image_convert_single(char *filename, fieldstruct **field,int nchan)
PURPOSE	Read FITS files, rebin, and convert them to TIFF format.
INPUT	Output filename,
	array of pointers to the tabstructs of input FITS extensions,
	number of input FITS files.
OUTPUT	-.
NOTES	Uses the global preferences.
AUTHOR	E. Bertin (IAP)
VERSION	06/02/2014
 ***/
void	image_convert_single(char *filename, fieldstruct **field, int nchan)
  {
   imagestruct		*image;
   catstruct		**cat;
   tabstruct		**tab;
   unsigned char	*extrapix;
   char			*description;
   double		*minvalue, *maxvalue;
   float		*fbuf[3],
			*fbuft0, *fbuft, *fsbuf;
   PIXTYPE		*ibuf,*ibuft,
			fpix;
   long			offset;
   OFF_T		imoffset;
   int			a, x,y, bx,by, width,height, fwidth,fheight,
			binsizex0,binsizey0,binsizexmax,binsizeymax,
			binsizex,binsizey,
			owidth, my, flipxflag, flipyflag, dyflag, dy,
			nlines, ntlines;

/* Start by making a few checks */
  if (nchan != 1 && nchan != 3)
    return;

  QMALLOC(cat, catstruct *, nchan);
  QMALLOC(tab, tabstruct *, nchan);
  fwidth = fheight = 0;
  description = NULL;	/* to avoid gcc -Wall warnings */

  QMALLOC(minvalue, double, nchan);
  QMALLOC(maxvalue, double, nchan);

  for (a=0; a<nchan; a++)
    {
    if (!(cat[a]=field[a]->cat))
      return;
    if (!(tab[a]=field[a]->tab))
      return;
    if (tab[a]->naxis<2)
      error(EXIT_FAILURE, "*Error*: not a 2D image in ", cat[a]->filename);
    if (fwidth)
      {
      if (tab[a]->naxisn[0] != fwidth)
        error(EXIT_FAILURE, "*Error*: Image width doesn't match in ",
		cat[a]->filename);
      }
    else
      fwidth = tab[a]->naxisn[0];
    if (fheight)
      {
      if (tab[a]->naxisn[1] != fheight)
        error(EXIT_FAILURE, "*Error*: Image height doesn't match in ",
		cat[a]->filename);
      }
    else
      fheight = tab[a]->naxisn[1];
    minvalue[a] = field[a]->min;
    maxvalue[a] = field[a]->max;
    }

  flipxflag = (prefs.flip_type == FLIP_X) || (prefs.flip_type == FLIP_XY);
  flipyflag = (prefs.flip_type == FLIP_Y) || (prefs.flip_type == FLIP_XY);
  binsizex0 = prefs.bin_size[0];
  width = binsizex0>1? (fwidth+binsizex0-1)/binsizex0 : fwidth;
  owidth = width*nchan;
  offset = -2*owidth;
  binsizey0 = prefs.bin_size[1];
  height = binsizey0>1? (fheight+binsizey0-1)/binsizey0 : fheight;

/* Create the output header file and prepare things */
  switch(prefs.format_type2)
    {
    case FORMAT_TIFF:
      image = create_tiff(filename, width, height, nchan, prefs.bpp, 0,
		minvalue, maxvalue,
		prefs.bigtiff_type, prefs.compress_type, prefs.compress_quality,
		prefs.copyright,
		prefs.header_flag? (description
			= fitshead_to_desc(tab[0]->headbuf, tab[0]->headnblock,
			width,height, binsizex0, binsizey0,
			flipxflag, flipyflag))
			: prefs.description);
      break;
   default:
     image = NULL; /* To avoid gcc -Wall warnings */
     error(EXIT_FAILURE, "This should not happen!", "");
   }

  if (!(binsizexmax = fwidth%binsizex0))
    binsizexmax = binsizex0;
  if (!(binsizeymax = fheight%binsizey0))
    binsizeymax = binsizey0;
  QCALLOC(ibuf, PIXTYPE, fwidth);
  nlines = image->nlines;
  for (a=0; a<nchan; a++)
    {
    QMALLOC(fbuf[a], float, width*nlines);
    }

#ifdef USE_THREADS
   static pthread_attr_t	pthread_attr;
   int				p, nproc;

/* Number of active threads */
  nproc = prefs.nthreads;
  if (nproc>1)
    nproc--;		/* Leave one proc free for non-blocking TIFF I/O's */
  pthread_nproc = nproc;
/* Set up multi-threading stuff */
  QPTHREAD_MUTEX_INIT(&tiffmutex, NULL);
  QPTHREAD_ATTR_INIT(&pthread_attr);
  QPTHREAD_ATTR_SETDETACHSTATE(&pthread_attr, PTHREAD_CREATE_JOINABLE);
  pthread_startgate = threads_gate_init(nproc+1, NULL);
  pthread_startwgate = threads_gate_init(2, NULL);
  pthread_stopgate = threads_gate_init(nproc+1, NULL);
  pthread_stopwgate = threads_gate_init(2, NULL);
  QMALLOC(proc, int, nproc);
  QMALLOC(pthread_fsbuf, float *, nproc);
  QMALLOC(thread, pthread_t, nproc);
  QMALLOC(fsbuf, float, width*nproc);
  QMALLOC(extrapix, unsigned char, width*nlines*image->bypp*image->nchan);
  pthread_field = field;
  pthread_image = image;
  pthread_data = fbuf;
  pthread_pix = extrapix;
  pthread_width = width;
  pthread_nchan = nchan;
  pthread_bypp = image->bypp;
  pthread_fflag = image->fflag;
  pthread_endflag = 0;
/* Install the signal-catching routines for temporary file cleanup */
  install_cleanup(pthread_cancel_threads);
/* Start the data conversion / tiling threads */
  for (p=0; p<nproc; p++)
    {
    proc[p] = p;
    pthread_fsbuf[p] = &fsbuf[p*pthread_width];
    QPTHREAD_CREATE(&thread[p], &pthread_attr, &pthread_data_to_pix, &proc[p]);
    }
  p = 0;
  QPTHREAD_CREATE(&tthread, &pthread_attr, &pthread_write_lines, &p);
#else
  QMALLOC(fsbuf, float, width*nlines);
/* Install the signal-catching routines for temporary file cleanup */
  install_cleanup(NULL);
#endif

/* Position the input file(s) at the beginning of image */
  for (a=0; a<nchan; a++)
    QFSEEK(cat[a]->file, tab[a]->bodypos, SEEK_SET, cat[a]->filename);

/* Prepare the output file and position the pointer at the last line */
/* We are going reverse (1st pixel is at top in TIFF, and at bottom in FITS) */

/* OK now we are ready to go! */
  my = fheight;
  ntlines = nlines;
  for (y=0; y<height; y++)
    {
    if (!y || !((y+1)%100))
      NPRINTF(OUTPUT, "\33[1M> Converting line:%7d / %-7d\n\33[1A",
        y+1, height);
    binsizey = ((y+1)<height? binsizey0:binsizeymax);
    my -= binsizey;
    dy = y%nlines;
    if (!dy)
      {
      if (ntlines > height - y)
        ntlines = height - y;
#ifdef USE_THREADS
      memset(fsbuf, 0, width*nproc*sizeof(float));
#else
      memset(fsbuf, 0, width*nlines*sizeof(float));
#endif
      }
    dyflag = (dy == ntlines - 1);
    for (a=0; a<nchan; a++)
      {
      fbuft0 = fbuf[a] + dy*width;
      if (!flipyflag)
        {
        imoffset = tab[a]->bodypos+(OFF_T)fwidth*my*tab[a]->bytepix;
        QFSEEK(cat[a]->file, imoffset, SEEK_SET, cat[a]->filename);
        }
      if (!dy)
        memset(fbuft0, 0, width*ntlines*sizeof(float));
/*---- Bin the pixels */
      for (by=binsizey; by--;)
        {
        read_body(tab[a], ibuf, fwidth);
        ibuft = ibuf;
        if (flipxflag)
          {
          fbuft = fbuft0 + width;
          for (x=width; x--;)
	    {
            fpix = 0;
            binsizex = x>0? binsizex0:binsizexmax;
            for (bx=binsizex; bx--;)
              fpix += *(ibuft++);
            *(--fbuft) += fpix / (binsizex*binsizey);
	    }
          }
        else
          {
          fbuft = fbuft0;
          for (x=width; x--;)
	    {
            fpix = 0;
            binsizex = x>0? binsizex0:binsizexmax;
            for (bx=binsizex; bx--;)
              fpix += *(ibuft++);
            *(fbuft++) += fpix / (binsizex*binsizey);
	    }
          }
        }
      }
    if (dyflag)
      {
#ifdef USE_THREADS
      pthread_bufline = 0;
      pthread_nbuflines = ntlines;
      threads_gate_sync(pthread_startgate);
/*---- ( Slave threads process the current buffer data here ) */
      threads_gate_sync(pthread_stopgate);
      if (y != ntlines-1)
        threads_gate_sync(pthread_stopwgate);
      image->y = y-ntlines+1;
      image->nlines = ntlines;
      memcpy(image->buf, extrapix, width*ntlines*image->bypp*nchan);
      threads_gate_sync(pthread_startwgate);
/*---- ( Writing thread starts processing the current buffer data here ) */
#else
      data_to_pix(field, fbuf, 0, image->buf, width*ntlines, nchan, image->bypp,
		image->fflag, fsbuf);
      switch(prefs.format_type2)
        {
        case FORMAT_TIFF:
          write_tifflines(image);
          break;
        default:
          error(EXIT_FAILURE, "This should not happen!", "");
        }
#endif
      }
    }

#ifdef USE_THREADS
    threads_gate_sync(pthread_stopwgate);

/* Clean up multi-threading stuff */
  pthread_endflag = 1;
/* (Re-)activate existing threads... */
  threads_gate_sync(pthread_startgate);
  threads_gate_sync(pthread_startwgate);
/* ... and shutdown all threads */
  for (p=0; p<nproc; p++)
    QPTHREAD_JOIN(thread[p], NULL);
  QPTHREAD_JOIN(tthread, NULL);
  threads_gate_end(pthread_startgate);
  threads_gate_end(pthread_startwgate);
  threads_gate_end(pthread_stopgate);
  threads_gate_end(pthread_stopwgate);
  QPTHREAD_MUTEX_DESTROY(&tiffmutex);
  QPTHREAD_ATTR_DESTROY(&pthread_attr);
  free(pthread_fsbuf);
  free(extrapix);
  free(proc);
  free(thread);
#endif

  switch(prefs.format_type2)
    {
    case FORMAT_TIFF:
      end_tiff(image);
      break;
    default:
      error(EXIT_FAILURE, "This should not happen!", "");
    }

/* Close file and free memory */
  free(ibuf);
  for (a=0; a<nchan; a++)
    free(fbuf[a]);
  free(fsbuf);
  free(cat);
  free(tab);
  if (prefs.header_flag)
    free(description);

  return;
  }


/****** image_convert_pyramid *************************************************
PROTO	int image_pyramid(char *filename, fieldstruct **field, int nchan)
PURPOSE	Read FITS files and rebin to generate a pyramid of image resolutions.
INPUT	File name,
	array of pointers to the tabstructs of input FITS extensions,
	number of input FITS files.
OUTPUT	Number of pyramid levels.
NOTES	Uses the global preferences.
AUTHOR	E. Bertin (IAP)
VERSION	06/02/2014
 ***/
int	image_convert_pyramid(char *filename, fieldstruct **field, int nchan)
  {
   imagestruct		*image;
   catstruct		**cat;
   tabstruct		**tab;
   float		*data[3],
			*datat,*datatt, *datao, *fbuf, *fbuft,*fbuftt, *fsbuf,
			fpix, fac;
   double		*minvalue, *maxvalue;
   OFF_T		imoffset;
   size_t		ndata,ndatao;
   unsigned char	*pix;
   char			*swapname[3],
			*swapnameo, *description;
   int			a,i,l, w,h, x,y,my,ny,bx,by, nlevels, width,height,
			fwidth,fheight, binsizex0,binsizey0, binsizex,binsizey,
			binsizexmax,binsizeymax, minsizex, minsizey, binx,biny,
			tilesize,tilesizey, flipxflag, flipyflag, bypp;

/* Start by making a few checks */
  if (nchan != 1 && nchan != 3)
    return 0;

  swapnameo = NULL;	/* to avoid gcc -Wall warnings */
  ndatao = ndata = 0;
  fbuft = datao = NULL;
  image = NULL;
  description = NULL;
  QMALLOC(cat, catstruct *, nchan);
  QMALLOC(tab, tabstruct *, nchan);
  fwidth = fheight = 0;

  QMALLOC(minvalue, double, nchan);
  QMALLOC(maxvalue, double, nchan);

  for (a=0; a<nchan; a++)
    {
    swapname[a] = NULL;
    if (!(cat[a]=field[a]->cat))
      error(EXIT_FAILURE, "*Internal error* with ", cat[a]->filename);
    if (!(tab[a]=field[a]->tab))
      error(EXIT_FAILURE, "*Internal error* with ", cat[a]->filename);
    if (tab[a]->naxis<2)
      error(EXIT_FAILURE, "*Error*: not a 2D image in ", cat[a]->filename);
    if (fwidth)
      {
      if (tab[a]->naxisn[0] != fwidth)
        error(EXIT_FAILURE, "*Error*: Image width doesn't match in ",
		cat[a]->filename);
      }
    else
      fwidth = tab[a]->naxisn[0];
    if (fheight)
      {
      if (tab[a]->naxisn[1] != fheight)
        error(EXIT_FAILURE, "*Error*: Image height doesn't match in ",
		cat[a]->filename);
      }
    else
      fheight = tab[a]->naxisn[1];
    QFSEEK(cat[a]->file, tab[a]->bodypos, SEEK_SET, cat[a]->filename);
    minvalue[a] = field[a]->min;
    maxvalue[a] = field[a]->max;
    }

  set_maxdataram(prefs.mem_max);
  set_maxdatavram(prefs.vmem_max);
  set_dataswapdir(prefs.swapdir_name);


/* Compute the number of pyramid levels */
  flipxflag = (prefs.flip_type == FLIP_X) || (prefs.flip_type == FLIP_XY);
  flipyflag = (prefs.flip_type == FLIP_Y) || (prefs.flip_type == FLIP_XY);
  binsizex0 = prefs.bin_size[0];
  minsizex = prefs.min_size[0];
  minsizey = prefs.min_size[1];
  tilesize = prefs.tile_size;
  w = width = binsizex0>1? (fwidth+binsizex0-1)/binsizex0 : fwidth;
  binsizey0 = prefs.bin_size[1];
  h = height = binsizey0>1? (fheight+binsizey0-1)/binsizey0 : fheight;
  QMALLOC(fbuf, PIXTYPE, fwidth);
  binx = biny = 1;

  image = create_tiff(filename, width, height, nchan, prefs.bpp, tilesize,
	minvalue, maxvalue, prefs.bigtiff_type, prefs.compress_type, prefs.compress_quality,
	prefs.copyright,
	prefs.header_flag? (description
		= fitshead_to_desc(tab[0]->headbuf, tab[0]->headnblock,
		width,height, binx *= binsizex0, biny *= binsizey0,
			flipxflag, flipyflag))
		: prefs.description);

  bypp = image->bypp;

#ifdef USE_THREADS
   static pthread_attr_t	pthread_attr;
   int				p, nproc;
/* Number of active threads */
  nproc = prefs.nthreads;
  if (nproc>1)
    nproc--;		/* Leave one proc free for non-blocking TIFF I/O's */
  pthread_nproc = nproc;
/* Set up multi-threading stuff */
  QPTHREAD_MUTEX_INIT(&tiffmutex, NULL);
  QPTHREAD_ATTR_INIT(&pthread_attr);
  QPTHREAD_ATTR_SETDETACHSTATE(&pthread_attr, PTHREAD_CREATE_JOINABLE);
  pthread_startgate = threads_gate_init(nproc+1, NULL);
  pthread_startwgate = threads_gate_init(2, NULL);
  pthread_stopgate = threads_gate_init(nproc+1, NULL);
  pthread_stopwgate = threads_gate_init(2, NULL);
  QMALLOC(proc, int, nproc);
  QMALLOC(pthread_fsbuf, float *, nproc);
  QMALLOC(thread, pthread_t, nproc);
  pthread_field = field;
  pthread_image = image;
  pthread_data = data;
  pthread_nchan = nchan;
  pthread_bypp = bypp;
  pthread_fflag = image->fflag;
  pthread_endflag = 0;
/* Install the signal-catching routines for temporary file cleanup */
  install_cleanup(pthread_cancel_threads);
/* Start the data conversion / tiling threads */
  for (p=0; p<nproc; p++)
    {
    proc[p] = p;
    QPTHREAD_CREATE(&thread[p], &pthread_attr, &pthread_data_to_pix, &proc[p]);
    }
  p = 0;
  QPTHREAD_CREATE(&tthread, &pthread_attr, &pthread_write_tiles, &p);
#else
  install_cleanup(NULL);
#endif

  for (nlevels = 1; w>=minsizex || h>=minsizey ; nlevels++, w/=2, h/=2);
  for (l=1; l<nlevels; l++)
    {
    if (l>1)
      {
      binsizexmax = binsizeymax = binsizex0 = binsizey0 = 2;
      fwidth = width;
      fheight = height;
      width = fwidth/binsizex0;
      height = fheight/binsizey0;
      ndatao = ndata;

      if (prefs.header_flag)
        free(description);
      create_tiffdir(image, width, height, nchan, prefs.bpp, tilesize,
		minvalue, maxvalue, prefs.compress_type, prefs.compress_quality,
		prefs.copyright,
		prefs.header_flag? (description
			= fitshead_to_desc(tab[0]->headbuf, tab[0]->headnblock,
			width,height, binx *= binsizex0, biny *= binsizey0,
			flipxflag, flipyflag))
			: prefs.description);
      }
    else
      {
      if (!(binsizexmax = fwidth%binsizex0))
        binsizexmax = binsizex0;
      if (!(binsizeymax = fheight%binsizey0))
        binsizeymax = binsizey0;

      width = binsizex0>1? (fwidth+binsizex0-1)/binsizex0 : fwidth;
      height = binsizey0>1? (fheight+binsizey0-1)/binsizey0 : fheight;
      }

    ndata = (size_t)width*(size_t)height;

    for (a=0; a<nchan; a++)
      {
      my = fheight;
      if (l>1)
        {
        fbuft = datao = data[a];
        swapnameo = swapname[a];
        }
      datat = data[a] = alloc_data(ndata, &swapname[a]);
      if (!datat)
        error(EXIT_FAILURE, "*Error*: not enough (virtual) memory for loading ",
		field[a]->rfilename);

      for (y=0; y<height; y++)
        {
        if (!y || !((y+1)%100))
        NPRINTF(OUTPUT,
		"\33[1M> Channel %1d/%-1d: Pyramid level %2d/%-2d: "
		"Reducing line %7d/%-7d\n\33[1A",
		a+1, nchan, l, nlevels-1, y+1, height);
        binsizey = ((y+1)<height? binsizey0:binsizeymax);
        my -= binsizey;
        if (!flipyflag && l==1)
          {
          imoffset = tab[a]->bodypos+(OFF_T)fwidth*my*tab[a]->bytepix;
          QFSEEK(cat[a]->file, imoffset, SEEK_SET, cat[a]->filename);
          }
        memset(datat, 0, width*sizeof(float));
/*------ Bin the pixels */
        for (by=binsizey; by--;)
          {
          if (l==1)
            {
            read_body(tab[a], fbuf, fwidth);
            fbuft = fbuf;
            }
          fbuftt = fbuft;
          if (flipxflag && l==1)
            {
            datatt = datat + width;
            for (x=width; x--;)
              {
              fpix = 0;
              binsizex = x>0? binsizex0:binsizexmax;
              fac = 1.0/(binsizex*binsizey);
              for (bx=binsizex; bx--;)
                fpix += *(fbuftt++);
              *(--datatt) += fac*fpix;
	      }
            }
          else
            {
            datatt = datat;
            for (x=width; x--;)
              {
              fpix = 0;
              binsizex = x>0? binsizex0:binsizexmax;
              fac = 1.0/(binsizex*binsizey);
              for (bx=binsizex; bx--;)
                fpix += *(fbuftt++);
              *(datatt++) += fac*fpix;
	      }
            }
          fbuft += fwidth;
          }
        datat += width;
        }
      if (l>1)
        free_data(datao, ndatao, swapnameo);
      }

    ny = image->ntilesy;
    QMALLOC(pix, unsigned char, tilesize*width*nchan*bypp);
    tilesizey = tilesize;
    imoffset = 0;
#ifdef USE_THREADS
    pthread_pix = pix;
    pthread_width = width;
    pthread_nbuflines = tilesizey;
/*-- Try to increase the number of pixels per threads */
    for (i=8; i--;)
      if (!(tilesize%i))
        {
        pthread_width *= i;
        pthread_nbuflines /= i;
        break;
        }
    QMALLOC(fsbuf, float, nproc*pthread_width);
    for (p=0; p<nproc; p++)
      pthread_fsbuf[p] = &fsbuf[p*pthread_width];
#else
    QMALLOC(fsbuf, float, tilesize*width);
#endif
    for (y=0; y<ny; y++)
      {
      NPRINTF(OUTPUT,
		"\33[1M> Pyramid level %2d/%-2d: "
		"Converting and tiling row %3d/%-3d\n\33[1A",
		l, nlevels, y+1, ny);
      if (y==ny-1)
        {
        tilesizey = height - y*tilesize;
#ifdef USE_THREADS
        pthread_width = width;
        pthread_nbuflines = tilesizey;
#endif
        }
#ifdef USE_THREADS
      pthread_bufline = 0;
      pthread_imoffset = imoffset;
      threads_gate_sync(pthread_startgate);
/*---- ( Slave threads process the current buffer data here ) */
      threads_gate_sync(pthread_stopgate);
      if (y)
        threads_gate_sync(pthread_stopwgate);
      raster_to_tiles(pix, image->buf, width, tilesizey, tilesize, nchan*bypp);
      image->tiley = y;
      threads_gate_sync(pthread_startwgate);
/*---- ( Writing thread starts processing the current buffer data here ) */
#else
      data_to_pix(field, data, imoffset, pix, tilesizey*width,nchan,bypp,image->fflag,fsbuf);
      raster_to_tiles(pix, image->buf, width, tilesizey, tilesize, nchan*bypp);
      write_tifftiles(image);
#endif
      imoffset += tilesize*width;
      }
#ifdef USE_THREADS
    threads_gate_sync(pthread_stopwgate);
#endif
    free(fsbuf);
    free(pix);
    }

/* Close file and free memory */
  end_tiff(image);
  free(fbuf);

#ifdef USE_THREADS
/* Clean up multi-threading stuff */
  pthread_endflag = 1;
/* (Re-)activate existing threads... */
  threads_gate_sync(pthread_startgate);
  threads_gate_sync(pthread_startwgate);
/* ... and shutdown all threads */
  for (p=0; p<nproc; p++)
    QPTHREAD_JOIN(thread[p], NULL);
  QPTHREAD_JOIN(tthread, NULL);
  threads_gate_end(pthread_startgate);
  threads_gate_end(pthread_startwgate);
  threads_gate_end(pthread_stopgate);
  threads_gate_end(pthread_stopwgate);
  QPTHREAD_MUTEX_DESTROY(&tiffmutex);
  QPTHREAD_ATTR_DESTROY(&pthread_attr);
  free(pthread_fsbuf);
  free(proc);
  free(thread);
#endif

  for (a=0; a<nchan; a++)
    free_data(data[a], ndata, swapname[a]);
  cleanup_files();
  free(cat);
  free(tab);
  if (prefs.header_flag)
    free(description);

  return nlevels;
  }


/****** data_to_pix ***********************************************************
PROTO	void data_to_pix(fieldstruct **field, float **data,
		unsigned char *outpix, int npix, int nchan, int bypp,
		int fflag, float *buffer)
PURPOSE	Read an array of data and convert it to colour pixel values.
INPUT	Array of field pointers,
	array of data pointers,
	offset to data pointers,
	array of pixels,
	number of pixels,
	number of channels,
	number of bytes per output channel,
        float output flag,
	luminance buffer.
OUTPUT	-..
NOTES	Uses the global preferences.
AUTHOR	E. Bertin (IAP)
VERSION	19/03/2012
 ***/
void	data_to_pix(fieldstruct **field, float **data, size_t offset,
		unsigned char *outpix, int npix, int nchan, int bypp,
		int fflag, float *buffer)
  {
   float		*datap[3],
			dataval[3], fmin[3], scale[3],
			*buffert, *datat, *outfpixt,
			invgammaf, coloursat, fac, sc, fm, fpix,fspix,
			fmax, pmax, pblack,pwhite,pscale,
			invg,goff,gsum,glin,gthresh;
   /* unsigned int		*outipixt; */
   unsigned short	*outspixt;
   unsigned char	*outbpixt;
   int			a,p, colflag, negflag, /*iflag,*/ sflag;

  colflag = (nchan==3);
  coloursat = prefs.colour_sat/3.0;


/* Adjust luminosity and contrast and sum fluxes from the different channels */
  fac = 1.0/nchan;
  sflag = (bypp>1);
  /* iflag = (bypp>2); unsigned integer flag */
  invgammaf = 1.0/prefs.gamma_fac;

  memset(buffer, 0, npix*sizeof(float));

  switch(prefs.gamma_type)
    {
    case GAMMA_POWERLAW:
      pmax = (1<<(bypp*8)) - 1.0;
      pblack = 0.0;
      pwhite = pmax;		/* Full swing */
      invg = 1/prefs.gamma;
      goff = 0.0;
      gsum = 1.0;
      glin = 0.0;
      gthresh = 0.0;
      break;
    case GAMMA_SRGB:
      pmax = (1<<(bypp*8)) - 1.0;
      pblack = 0.0;
      pwhite = pmax;		/* Full swing */
      invg = 1/2.4;
      goff = 0.055;
      gsum = 1.0 + goff;
      glin = 12.92;
      gthresh = 0.0030402;
      break;
    case GAMMA_REC709:
      pmax = (1<<(bypp*8)) - 2.0;
      pblack = 0.063*pmax;
      pwhite = 0.925*pmax;	/* Studio-swing */
      invg = 1/2.222;
      goff = 0.099;
      gsum = 1.0 + goff;
      glin = 4.5;
      gthresh = 0.018;
      break;
    default:
/*---- Avoid gcc -Wall warnings */
      pmax = 0.0;
      pblack = 0.0;
      pwhite = pmax;
      invg = 1.0;
      goff = 0.0;
      gsum = 1.0;
      glin = 0.0;
      gthresh = 0.0;
      error(EXIT_FAILURE, "*Internal Error*: unknown gamma correction in ",
	"data_to_pix()");
    }
  pscale = (pwhite-pblack);
  pblack += 0.5;	/* for symmetric round-off */

  if (fflag)
    {
    outbpixt = NULL;	/* Avoid gcc -Wall warnings */
    outspixt = NULL;
    //outipixt  = NULL;
    outfpixt = (float *)outpix;
    for (p=0; p<npix; p++)
      for (a=0; a<nchan; a++)
        *(outfpixt++) = data[a][p+offset];
    }
  else
    {
    for (a=0; a<nchan; a++)
      {
      fm = fmin[a] = field[a]->min;
      sc = scale[a] = 1.0/(field[a]->max-fm);
      datat = datap[a] = data[a]+offset;
      buffert = buffer;
      for (p=npix; p--;)
        {
        if ((fpix = sc*(*(datat++) - fm)) < 0.0)
          fpix = 0.0;
        *(buffert++) += fpix*fac;
        }
      }

    if (sflag)
      {
      /*if (iflag)
        {
        outbpixt = NULL;	/* Avoid gcc -Wall warnings */
      /*  outspixt = NULL;
        outipixt  = (unsigned int *)outpix;;
        outfpixt = NULL;    
        }
      else
        { */
      outbpixt = NULL;	/* Avoid gcc -Wall warnings */
      outspixt = (unsigned short *)outpix;
      //outipixt  = NULL;
      outfpixt = NULL;    
        //}
      }
    else
      {
      outbpixt = outpix;
      outspixt = NULL;	/* Avoid gcc -Wall warnings */
      //outipixt  = NULL;
      outfpixt = NULL;    
      }

    negflag = prefs.neg_flag;
    buffert = buffer;
    for (p=0; p<npix; p++)
      {
      fspix = *(buffert++);
      fmax = powf(fspix, 1.0 - invgammaf);
      for (a=0; a<nchan; a++)
        if ((dataval[a] = scale[a]*(datap[a][p]-fmin[a])) >= fmax)
          dataval[a] = fmax;

      for (a=0; a<nchan; a++)
        {
        fpix = dataval[a];
        if (colflag)
          {
          fpix = fspix + coloursat*(2.0*fpix-dataval[(a+1)%3]-dataval[(a+2)%3]);
          if (fpix<0.0)
            fpix = 0.0;
          }
        fpix = (colflag && fspix > 1e-15)? fpix/fspix : 1.0;
        fpix *= powf(fspix, invgammaf);
 /*---- Video gamma correction */
        fpix = pscale*(fpix<gthresh? glin*fpix : gsum *powf(fpix, invg)-goff) + pblack;
        if (fpix>=pmax)
          fpix = pmax;
        if (negflag)
          fpix = pmax - fpix;
        if (sflag)
          /*{
          if (iflag)
            *(outipixt++) = (unsigned int)fpix;
          else*/
          *(outspixt++) = (unsigned short)fpix;
          //}
        else
          *(outbpixt++) = (unsigned char)fpix;
        }
      }
    }
  return;
  }


/****** raster_to_tiles *******************************************************
PROTO	int raster_to_tiles(unsigned char *inpix, unsigned char *outpix,
			int width, int tilesize, int nchan, int bypp)
PURPOSE	Read an array of colour pixel values and re-organize it as tiles.
INPUT	Input array of pixels,
	Output array of pixels,
	raster width,
	actual tile height,
	tile size,
	number of channels,
	number of bytes per channel.
OUTPUT	Number of tiles along the x axis.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	17/06/2010
 ***/
int raster_to_tiles(unsigned char *inpix, unsigned char *outpix,
			int width, int tilesizey, int tilesize, int nbytes)
  {
   unsigned char	*inpixt,*outpixt;
   int			x,y, nx, tilesizex,tilesizef;

  nx = (width+tilesize-1)/tilesize;
/* Everything is multiplied by the number of channels */
  width *= nbytes;
  tilesizex = tilesizef = tilesize*nbytes;
  if (tilesizey != tilesize)
    memset(outpix, 0, tilesize*width);
  for (x=0; x<nx; x++)
    {
    inpixt = inpix+x*tilesizef;
    outpixt = outpix+x*tilesizef*tilesize;
    if (x==nx-1)
      {
      tilesizex = width - x*tilesizex;
      if (tilesizex != tilesizef)
        memset(outpixt, 0, tilesizef*tilesize);
      }
    for (y=tilesizey; y--; inpixt += width, outpixt += tilesizef)
      memcpy(outpixt, inpixt, tilesizex);
    }

  return nx;
  }


#ifdef USE_THREADS

/****** pthread_data_to_pix ***************************************************
PROTO   void *pthread_data_to_pix(void *arg)
PURPOSE thread that takes care of converting FITS pixels to TIFF pixels.
INPUT   Pointer to the thread number.
OUTPUT  -.
NOTES   -.
AUTHOR  E. Bertin (IAP)
VERSION 16/03/2012
 ***/
void    *pthread_data_to_pix(void *arg)
  {
   int  bufline, proc;

  bufline = -1;
  proc = *((int *)arg);
  threads_gate_sync(pthread_startgate);
  while (!pthread_endflag)
    {
    QPTHREAD_MUTEX_LOCK(&tiffmutex);
    if (pthread_bufline<pthread_nbuflines)
      {
      bufline = pthread_bufline++;
      QPTHREAD_MUTEX_UNLOCK(&tiffmutex);
      data_to_pix(pthread_field,
		pthread_data,
		pthread_imoffset+bufline*pthread_width,
		pthread_pix+bufline*pthread_width*pthread_nchan*pthread_bypp,
		pthread_width,
		pthread_nchan,
		pthread_bypp,
                pthread_fflag,
		pthread_fsbuf[proc]);
      }
    else
      {
      QPTHREAD_MUTEX_UNLOCK(&tiffmutex);
/*---- Wait for the input buffer to be updated */
      threads_gate_sync(pthread_stopgate);
/* ( Master thread process loads and saves new data here ) */
      threads_gate_sync(pthread_startgate);
      }
    }

  pthread_exit(NULL);

  return (void *)NULL;
  }


/****** pthread_write_lines ***************************************************
PROTO   void *pthread_write_lines(void *arg)
PURPOSE thread that takes care of writing TIFF lines (non-blocking).
INPUT   Pointer to the thread number.
OUTPUT  -.
NOTES   -.
AUTHOR  E. Bertin (IAP)
VERSION 04/01/2010
 ***/
void    *pthread_write_lines(void *arg)
  {
   int  bufline, proc;

  bufline = -1;
  proc = *((int *)arg);
  threads_gate_sync(pthread_startwgate);
  while (!pthread_endflag)
    {
    write_tifflines(pthread_image);
/*-- Wait for the input buffer to be updated */
    threads_gate_sync(pthread_stopwgate);
/*-- ( Master thread process loads and saves new data here ) */
    threads_gate_sync(pthread_startwgate);
    }

  pthread_exit(NULL);

  return (void *)NULL;
  }


/****** pthread_write_tiles ***************************************************
PROTO   void *pthread_write_tiles(void *arg)
PURPOSE thread that takes care of writing TIFF tiles (non-blocking).
INPUT   Pointer to the thread number.
OUTPUT  -.
NOTES   -.
AUTHOR  E. Bertin (IAP)
VERSION 03/01/2010
 ***/
void    *pthread_write_tiles(void *arg)
  {
   int  bufline, proc;

  bufline = -1;
  proc = *((int *)arg);
  threads_gate_sync(pthread_startwgate);
  while (!pthread_endflag)
    {
    write_tifftiles(pthread_image);
/*-- Wait for the input buffer to be updated */
    threads_gate_sync(pthread_stopwgate);
/*-- ( Master thread process loads and saves new data here ) */
    threads_gate_sync(pthread_startwgate);
    }

  pthread_exit(NULL);

  return (void *)NULL;
  }

/****** pthread_cancel ********************************************************
PROTO	void pthread_cancel_threads(void)
PURPOSE	Cancel remaining active threads
INPUT   -.
OUTPUT  -.
NOTES   -.
AUTHOR  E. Bertin (IAP)
VERSION 04/01/2010
 ***/
void    pthread_cancel_threads(void)
  {
   int  p;

  for (p=0; p<pthread_nproc; p++)
    QPTHREAD_CANCEL(thread[p]);
    QPTHREAD_CANCEL(tthread);

  return;
  }

#endif

/****** fitshead_to_desc ******************************************************
PROTO	char	*fitshead_to_desc(char *fitshead, int nheadblock,
		int sizex, int sizey, int binx, int biny,
		int flipxflag, int flipyflag)
PURPOSE	Convert FITS header to CDS-like description field.
INPUT	Pointer to FITS header,
	number of FITS blocks,
	image size in x,
	image size in y,
	binning factor in x,
	binning factor in y,
	x-flipping flag,
	y-flipping flag.
OUTPUT	-.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	08/02/2010
 ***/
char	*fitshead_to_desc(char *fitshead, int nheadblock,
		int sizex, int sizey, int binx, int biny,
		int flipxflag, int flipyflag)
  {
    char	*description;
    double	dval;

  description = NULL;	/* to avoid gcc -Wall warnings */
  QMEMCPY(fitshead, description, char, nheadblock*FBSIZE);

  fitswrite(description, "NAXIS1  ", &sizex, H_INT, T_LONG);
  fitswrite(description, "NAXIS2  ", &sizey, H_INT, T_LONG);

/*---- Scale the WCS information if present */
  if (fitsread(description, "CRPIX1  ", &dval, H_EXPO, T_DOUBLE)==RETURN_OK)
    {
    dval = (dval - 0.5)/binx + 0.5;
    if (flipxflag)
      dval = sizex+1 - dval;
    fitswrite(description, "CRPIX1  ", &dval, H_EXPO, T_DOUBLE);
    }
  if (fitsread(description, "CRPIX2  ", &dval, H_EXPO, T_DOUBLE)==RETURN_OK)
    {
    dval = (dval - 0.5)/biny + 0.5;
    if (flipyflag)
      dval = sizey+1 - dval;
    fitswrite(description, "CRPIX2  ", &dval, H_EXPO, T_DOUBLE);
    }

  if (flipxflag)
    binx = -binx;
  if (flipyflag)
    biny = -biny;

  if (fitsread(description, "CDELT1  ", &dval, H_EXPO, T_DOUBLE)==RETURN_OK)
    {
    dval *= binx;
    fitswrite(description, "CDELT1  ", &dval, H_EXPO, T_DOUBLE);
    }
  if (fitsread(description, "CDELT2  ", &dval, H_EXPO, T_DOUBLE)==RETURN_OK)
    {
    dval *= biny;
    fitswrite(description, "CDELT2  ", &dval, H_EXPO, T_DOUBLE);
    }
  if (fitsread(description, "CD1_1   ", &dval, H_EXPO, T_DOUBLE)==RETURN_OK)
    {
    dval *= binx;
    fitswrite(description, "CD1_1   ", &dval, H_EXPO, T_DOUBLE);
    }
  if (fitsread(description, "CD1_2   ", &dval, H_EXPO, T_DOUBLE)==RETURN_OK)
    {
    dval *= biny;
    fitswrite(description, "CD1_2   ", &dval, H_EXPO, T_DOUBLE);
    }
  if (fitsread(description, "CD2_1   ", &dval, H_EXPO, T_DOUBLE)==RETURN_OK)
    {
    dval *= binx;
    fitswrite(description, "CD2_1   ", &dval, H_EXPO, T_DOUBLE);
    }
  if (fitsread(description, "CD2_2   ", &dval, H_EXPO, T_DOUBLE)==RETURN_OK)
    {
    dval *= biny;
    fitswrite(description, "CD2_2   ", &dval, H_EXPO, T_DOUBLE);
    }

  *(description + 80*fitsfind(description, "END     ")) = '\0';

  return description;
  }


/****** make_imastats *********************************************************
PROTO	void make_imastats(fieldstruct *field,
		int backflag, int minflag, int maxflag)
PURPOSE	Compute image statistics
INPUT	Field structure of the image,
	flag to trigger median background computation,
	flag to trigger min. level quantile computation,
	flag to trigger max. level quantile computation.
OUTPUT	-.
NOTES	Uses the global preferences.
AUTHOR	E. Bertin (IAP)
VERSION	22/06/2012
 ***/
void	make_imastats(fieldstruct *field,
		int backflag, int minflag, int maxflag)
  {
   catstruct	*cat;
   tabstruct	*tab;
   long		n,npix, nsample;
   char		*rfilename;
   float	*med, *min, *max;
   PIXTYPE	*pixbuf;
   int		size;


  if (!((cat=field->cat) && open_cat(cat, READ_ONLY)==RETURN_OK))
    return;
  rfilename = field->rfilename;
  tab = field->tab;
  QFSEEK(cat->file, tab->bodypos, SEEK_SET, cat->filename);
  size = IMAGE_BUFSIZE/sizeof(PIXTYPE);
  QMALLOC(pixbuf, PIXTYPE, size);
  npix = tab->tabsize/tab->bytepix;
  nsample = (npix+size-1) / size;
  QMALLOC(med, float, nsample);
  QMALLOC(min, float, nsample);
  QMALLOC(max, float, nsample);
  for (n=0; n<nsample; npix -= size, n++)
    {
    NPRINTF(OUTPUT, "\33[1M> %s: Computing Image stats: %2.0f%%\n\33[1A",
	rfilename,
	(100.0*((double)n+0.49))/(double)nsample);
    if (size>npix)
      size = npix;
    read_body(tab, pixbuf, size);
    med[n] = fast_median(pixbuf, size);
    if (minflag)
      min[n] = fast_quantile(pixbuf, size/2, field->min);
    if (maxflag)
      max[n] = fast_quantile(pixbuf+size/2, size/2, field->max);
    }
  free(pixbuf);
  if (backflag)
    field->back = (PIXTYPE)fast_median(med, nsample);
  if (minflag)
    field->min = (PIXTYPE)fast_median(min, nsample);
  if (maxflag)
    field->max = (PIXTYPE)fast_median(max, nsample);
  free(med);
  free(min);
  free(max);

  QFSEEK(cat->file, tab->bodypos, SEEK_SET, cat->filename);

  return;
  }


/******* fast_median **********************************************************
PROTO   float fast_median(float *arr, int n)
PURPOSE Fast median from an input array, optimized version based on the
        select() routine (Numerical Recipes, 2nd ed. Section 8.5 and
        http://www.eso.org/~ndevilla/median/). If n is even, then the result
        is the average of the 2 "central" values.
INPUT   Input pixel array ptr,
        number of input pixels,
OUTPUT  Value of the median.
NOTES   n must be >0. Warning: changes the order of data (but does not sort
        them)!
AUTHOR  E. Bertin (IAP), optimized from N.Devillard's code
VERSION 10/04/2003
 ***/
#define MEDIAN_SWAP(a,b) { float t=(a);(a)=(b);(b)=t; }

float fast_median(float *arr, int n)
  {
   float      *alow, *ahigh, *amedian, *amiddle, *all, *ahh,
                val, valmax, valmax2;
   int          i, nless;

  if (!n)
    return 0.0;
  else if (n==1)
    return *arr;
  else if (n==2)
    return 0.5*(*arr+*(arr+1));

  alow = arr;
  ahigh = arr + n - 1;
  amedian = arr + n/2;
  while (ahigh > (all=alow + 1))
    {
/*-- Find median of low, middle and high items; swap into position low */
    amiddle = alow + (ahigh-alow)/2;
    if (*amiddle > *ahigh)
      MEDIAN_SWAP(*amiddle, *ahigh);
    if (*alow > *ahigh)
      MEDIAN_SWAP(*alow, *ahigh);
    if (*amiddle > *alow)
      MEDIAN_SWAP(*amiddle, *alow);

/*-- Swap low item (now in position middle) into position (low+1) */
    MEDIAN_SWAP(*amiddle, *all);

/*-- Nibble from each end towards middle, swapping items when stuck */
    ahh = ahigh;
    for (;;)
      {
      while (*alow > *(++all));
      while (*(--ahh) > *alow);

      if (ahh < all)
        break;

      MEDIAN_SWAP(*all, *ahh);
      }

/*-- Swap middle item (in position low) back into correct position */
    MEDIAN_SWAP(*alow, *ahh) ;

/*-- Re-set active partition */
    if (ahh <= amedian)
      alow = all;
    if (ahh >= amedian)
      ahigh = ahh - 1;
    }

/* One or two elements left */
  if (ahigh == all && *alow > *ahigh)
    MEDIAN_SWAP(*alow, *ahigh);

  if (n&1)
/*-- Odd case */
    return *amedian;
  else
    {
/*-- Even case */
    valmax2 = *amedian;
    valmax = -BIG;
    alow = arr;
    nless = 0;
    for (i=n/2;i--;)
      if ((val=*(alow++))<valmax2)
        {
        nless++;
        if (val > valmax)
          valmax = val;
        }
    return nless<n/2? *amedian : (*amedian+valmax)/2.0;
    }

  }

#undef MEDIAN_SWAP


/******* fast_quantile ********************************************************
PROTO   float fast_quantile(float *arr, int n, float frac)
PURPOSE Fast median from an input array, optimized version based on the
        select() routine (Numerical Recipes, 2nd ed. Section 8.5.
INPUT   Input pixel array ptr,
        number of input pixels,
	quantile fraction (>=0 & <1)
OUTPUT  Value of the quantile.
NOTES   n must be >0. Warning: changes the order of data (but does not sort
        them)!
AUTHOR  E. Bertin (IAP), optimized from Num.Rec. code
VERSION 28/01/2003
 ***/
#define QUANTILE_SWAP(a,b) { register float t=(a);(a)=(b);(b)=t; }

float fast_quantile(float *arr, int n, float frac)
  {
   float      *alow, *ahigh, *amedian, *amiddle, *all, *ahh;

  if (frac>1.0)
    frac = 1.0;
  else if (frac<0.0)
    frac = 0.0; 
  alow = arr;
  ahigh = arr + n - 1;
  amedian = arr + (int)(frac*(n-0.5001));
  while (ahigh > (all=alow + 1))
    {
/*-- Find median of low, middle and high items; swap into position low */
    amiddle = alow + (ahigh-alow)/2;
    if (*amiddle > *ahigh)
      QUANTILE_SWAP(*amiddle, *ahigh);
    if (*alow > *ahigh)
      QUANTILE_SWAP(*alow, *ahigh);
    if (*amiddle > *alow)
      QUANTILE_SWAP(*amiddle, *alow);

/*-- Swap low item (now in position middle) into position (low+1) */
    QUANTILE_SWAP(*amiddle, *all);

/*-- Nibble from each end towards middle, swapping items when stuck */
    ahh = ahigh;
    for (;;)
      {
      while (*alow > *(++all));
      while (*(--ahh) > *alow);

      if (ahh < all)
        break;

      QUANTILE_SWAP(*all, *ahh);
      }

/*-- Swap middle item (in position low) back into correct position */
    QUANTILE_SWAP(*alow, *ahh) ;

/*-- Re-set active partition */
    if (ahh <= amedian)
      alow = all;
    if (ahh >= amedian)
      ahigh = ahh - 1;
    }

/* One or two elements left */
  if (ahigh == all && *alow > *ahigh)
    QUANTILE_SWAP(*alow, *ahigh);

  return *amedian;
  }

#undef QUANTILE_SWAP

