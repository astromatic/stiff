/*
                                  image.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*       Part of:        STIFF
*
*       Author:         E.BERTIN (IAP)
*
*       Contents:       Convert FITS data to 8-bit format
*
*       Last modify:    10/01/2005
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifdef HAVE_CONFIG_H
#include	"config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "define.h"
#include "globals.h"
#include "field.h"
#include "image.h"
#include "prefs.h"
#include "fits/fitscat.h"
#include "tiff.h"

/****** image_convert *********************************************************
PROTO	void image_convert(char *filename, fieldstruct **field, PIXTYPE *back,
			int ncat)
PURPOSE	Read FITS files, rebin, and convert them to TIFF format.
INPUT	Output filename,
	array of pointers to the tabstructs of input FITS extensions,
	number of input FITS files.
OUTPUT	-..
NOTES	Uses the global preferences.
AUTHOR	E. Bertin (IAP)
VERSION	09/01/2005
 ***/
void	image_convert(char *filename, fieldstruct **field, int ncat)
  {
   imagestruct		*image;
   catstruct		**cat;
   tabstruct		**tab;
   unsigned char	*buf, *buft;
   double		invgamma, invgammaf, coloursat, dfpix, dfspix,
			dncat, fmax,fmax2;
   float		*fbuf[3],
			fmin[MAXFILE], fscale[MAXFILE], sat_level[3],
			*fbuft, *fsbuf, *fsbuft, *fbuft2, *fbuft3,
			min,scale;
   PIXTYPE		*ibuf,*ibuft,
			fpix;
   long			offset;
   int			a, x,y, bx,by, width,height, fwidth,fheight,
			binsize,binsizexmax,binsizeymax,binsizey,
			negflag, colflag, owidth;

/* Start by making a few checks */
  if (ncat != 1 && ncat != 3)
    return;

  QMALLOC(cat, catstruct *, ncat);
  QMALLOC(tab, tabstruct *, ncat);
  fwidth = fheight = 0;

  for (a=0; a<ncat; a++)
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
    }

  binsize = prefs.bin_size;
  invgammaf = 1.0/prefs.gamma_fac;
  invgamma = 1.0/prefs.gamma;
  coloursat = prefs.colour_sat/3.0;
  colflag = (ncat==3);
  width = binsize>1? (fwidth+binsize-1)/binsize : fwidth;
  owidth = width*ncat;
  offset = -2*owidth;
  height = binsize>1? (fheight+binsize-1)/binsize : fheight;

/* Adjust luminosity and contrast */
  for (a=0; a<ncat; a++)
    {
    fmin[a] = field[a]->min;
    fscale[a] = pow(255.5, prefs.gamma*prefs.gamma_fac)
		/ (field[a]->max - fmin[a]);
    sat_level[a] = fscale[a]*(prefs.sat_val[a] - fmin[a]);
    }

/* Create the output header file and prepare things */
  switch(prefs.format_type)
    {
    case FORMAT_TIFF:
      image = create_tiff(filename, width, height, ncat);
     break;
   default:
     image = NULL; /* To avoid gcc -Wall warnings */
     error(EXIT_FAILURE, "This should not happen!", "");
     }

  if (!(binsizexmax = fwidth%binsize))
    binsizexmax = binsize;
  if (!(binsizeymax = fheight%binsize))
    binsizeymax = binsize;
  QCALLOC(ibuf, PIXTYPE, binsize*width);
  for (a=0; a<ncat; a++)
    {
    QMALLOC(fbuf[a], float, width);
    }
  QMALLOC(fsbuf, float, width);
  QCALLOC(buf, unsigned char, owidth);

/* Position the input file(s) at the beginning of image */
  for (a=0; a<ncat; a++)
    QFSEEK(cat[a]->file, tab[a]->bodypos, SEEK_SET, cat[a]->filename);

/* Prepare the output file and position the pointer at the last line */
/* We are going reverse (1st pixel is at top in TIFF, and at bottom in FITS) */
  for (y=height; y--;)
    write_tiff(image, buf, owidth);
  QFSEEK(image->file, offset/2, SEEK_CUR, image->filename);

/* OK now we are ready to go! */
  fbuft2 = fbuft3 = NULL; /* To avoid gcc -Wall warnings */
  negflag = prefs.neg_flag;
  dncat = (double)ncat;
  for (y=0; y<height; y++)
    {
    if (!y || !((y+1)%16))
      NPRINTF(OUTPUT, "\33[1M> Converting line:%7d / %-7d\n\33[1A",
        y+1, height);
    binsizey = ((y+1)<height? binsize:binsizeymax);
    memset(fsbuf, 0, width*sizeof(float));
    for (a=0; a<ncat; a++)
      {
      memset(fbuf[a], 0, width*sizeof(float));
/*---- Bin the pixels */
      for (by=binsizey; by--;)
        {
        read_body(tab[a], ibuf, fwidth);
        ibuft = ibuf;
        fbuft = fbuf[a];
        for (x=width; x--;)
	  {
          fpix = 0;
          for (bx=binsize; bx--;)
            fpix += *(ibuft++);
          *(fbuft++) += fpix/(x>0? binsize:binsizexmax);
	  }
        }
/*---- Sum the fluxes from the different channels */
      min = fmin[a];
      scale = fscale[a];
      fsbuft = fsbuf;
      fbuft = fbuf[a];
      for (x=width; x--;)
        {
        if ((fpix = scale*(*fbuft/binsizey - min))<0.0)
          fpix = 0.0;
        *(fsbuft++) += (*(fbuft++) = fpix)/dncat;
        }
      }

    fmax = pow(255.5, prefs.gamma);

    fsbuft = fsbuf;
    buft = buf;
    for (x=0; x<width; x++)
      {
      dfspix = *(fsbuft++);
      for (a=0; a<ncat; a++)
        {
        dfpix = fbuf[a][x];
        fmax2 = fmax*pow(dfspix, 1.0 - invgammaf);
        if (dfpix >= fmax2)
          fbuf[a][x] = fmax2;
        }

      for (a=0; a<ncat; a++)
        {
        dfpix = fbuf[a][x];
        if (colflag)
          {
          fbuft2 = fbuf[(a+1)%3];
          fbuft3 = fbuf[(a+2)%3];
          dfpix = dfspix + coloursat*(2.0*dfpix
			- fbuf[(a+1)%3][x] - fbuf[(a+2)%3][x]);
          }
        dfpix = (colflag && dfspix > 1e-15)? dfpix/dfspix : 1.0;
        dfpix *= pow(dfspix, invgammaf);
/*------ Video gamma correction */
        dfpix = pow((double)dfpix, invgamma)+0.5;
        if (dfpix>=255.5)
          dfpix = 255.5;
        if (negflag)
          dfpix = 255.5 - dfpix;
        *(buft++) = (unsigned char)dfpix;
        }
      }
    switch(prefs.format_type)
      {
      case FORMAT_TIFF:
        if (y)
          {
          QFSEEK(image->file, offset, SEEK_CUR, image->filename);
          }
        write_tiff(image, buf, owidth);
      break;
     default:
       error(EXIT_FAILURE, "This should not happen!", "");
     }
    }

/* Close file and free memory */
  free(ibuf);
  for (a=0; a<ncat; a++)
    free(fbuf[a]);
  free(fsbuf);
  free(buf);
  free(cat);
  free(tab);
  switch(prefs.format_type)
    {
    case FORMAT_TIFF:
      end_tiff(image);
      break;
    default:
      error(EXIT_FAILURE, "This should not happen!", "");
    }

  return;
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
VERSION	14/11/2004
 ***/
void	make_imastats(fieldstruct *field,
		int backflag, int minflag, int maxflag)
  {
   catstruct	*cat;
   tabstruct	*tab;
   KINGLONG	npix;
   char		*rfilename;
   float	*med, *min, *max;
   PIXTYPE	*pixbuf;
   int		n, nsample, size;


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

