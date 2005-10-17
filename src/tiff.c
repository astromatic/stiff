/*
                                  tiff.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*       Part of:        STIFF
*
*       Author:         E.BERTIN (IAP)
*
*       Contents:       Handle TIFF format
*
*       Last modify:    18/11/2004
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifdef HAVE_CONFIG_H
#include	"config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "define.h"
#include "globals.h"
#include "fits/fitscat.h"
#include "image.h"
#include "tiff.h"

tagstruct	ImageWidth = {0x0100, 4, 1, {0}},
		ImageLength = {0x0101, 4, 1, {0}},
		BitsPerSample = {0x0102, 3, 1, {0}},
		Compression = {0x0103, 3, 1, {0}},
		PhotometricInterpretation = {0x0106, 3, 1, {0}},
		StripOffsets = {0x0111, 4, 1, {0}},
		SamplesPerPixel = {0x0115, 3, 1, {0}},
		RowsPerStrip = {0x0116, 4, 1, {0}},
		StripByteCounts = {0x0117, 4, 1, {0}},
		XResolution = {0x011A, 5, 1, {72}},
		YResolution = {0x011B, 5, 1, {72}},
		ResolutionUnit = {0x0128, 3, 1, {0}};

/****** create_tiff ***********************************************************
PROTO	imagestruct *create_tiff(char *filename, int width, int height,
			int nbytes)
PURPOSE	Create a TIFF image (write a TIFF header and return an imagestruct).
INPUT	Output filename,
	Image width in pixels,
	Image height in pixels,
	Number of channels (bytes).
OUTPUT	Pointer to an imagestruct.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	18/11/2004
 ***/
imagestruct *create_tiff(char *filename, int width, int height,
			int nbytes)
  {
   imagestruct	*image;
   FILE		*file;
   short	ashort;
   int		anint, anint2, ntag, offset;

  if ((file = fopen(filename, "wb")) == NULL)
    error(EXIT_FAILURE, "*Error*: cannot open for writing ", filename);
/* Image file header */
  ashort = (bswapflag?0x4949:0x4D4D);	/* little or big endian */
  QFWRITE(&ashort, 2, file, filename);
  ashort = 42;				/* It means this is TIFF */
  QFWRITE(&ashort, 2, file, filename);
  anint = offset = 8;			/* Next IFD begins immediately */
  QFWRITE(&anint, 4, file, filename);

/* Image File Directory */
  ashort = ntag = 12;
  QFWRITE(&ashort, 2, file, filename);
  offset += 2 + ntag*sizeof(tagstruct) + 4;
  ImageWidth.value.uint = width;
  QFWRITE(&ImageWidth, sizeof(tagstruct), file, filename);
  ImageLength.value.uint = height;
  QFWRITE(&ImageLength, sizeof(tagstruct), file, filename);
  BitsPerSample.value.ushort = 8;
  QFWRITE(&BitsPerSample, sizeof(tagstruct), file, filename);
  Compression.value.ushort = 1;
  QFWRITE(&Compression, sizeof(tagstruct), file, filename);
  PhotometricInterpretation.value.ushort = (nbytes==1?1:2);
  QFWRITE(&PhotometricInterpretation, sizeof(tagstruct), file, filename);
  StripOffsets.value.uint = offset + 8*2;
  QFWRITE(&StripOffsets, sizeof(tagstruct), file, filename);
  SamplesPerPixel.value.ushort = nbytes;
  QFWRITE(&SamplesPerPixel, sizeof(tagstruct), file, filename);
  RowsPerStrip.value.uint = height;
  QFWRITE(&RowsPerStrip, sizeof(tagstruct), file, filename);
  StripByteCounts.value.uint = width*height*nbytes;
  QFWRITE(&StripByteCounts, sizeof(tagstruct), file, filename);
  XResolution.value.rational = offset;
  QFWRITE(&XResolution, sizeof(tagstruct), file, filename);
  YResolution.value.rational = offset + 8;
  QFWRITE(&YResolution, sizeof(tagstruct), file, filename);
  ResolutionUnit.value.ushort = 2;
  QFWRITE(&ResolutionUnit, sizeof(tagstruct), file, filename);
/* Next Image File Directory (none) */
  anint = 0;
  QFWRITE(&anint, 4, file, filename);
/* Now the (dummy) resolutions */
  anint = height;
  anint2 = 8;
  QFWRITE(&anint, 4, file, filename);
  QFWRITE(&anint2, 4, file, filename);
  QFWRITE(&anint, 4, file, filename);
  QFWRITE(&anint2, 4, file, filename);

/* Start by making a few checks */
  if (nbytes != 1 && nbytes != 3)
    return NULL;

  QMALLOC(image, imagestruct, 1);
  strcpy(image->filename, filename);
  image->file = file;
  image->width = width;
  image->height = height;
  image->nbytes = nbytes;

  return image;
  }


/****** write_tiff ***********************************************************
PROTO	void	write_tiff(imagestruct *image, unsigned char *pixbuf, int npix)
PURPOSE	Write a bunch of pixels in a TIFF image
INPUT	Pointer to the image structure,
	Pointer to the array of pixels,
	Number of pixels to be written.
OUTPUT	-.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	18/01/2003
 ***/
void	write_tiff(imagestruct *image, unsigned char *pixbuf, int npix)
  {
  QFWRITE(pixbuf, npix, image->file, image->filename);

  return;
  }

/****** end_tiff ***********************************************************
PROTO	void	end_tiff(imagestruct *image)
PURPOSE	Terminate everything related to a TIFF file.
INPUT	Pointer to the image structure.
OUTPUT	Computed background.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	18/01/2003
 ***/
void	end_tiff(imagestruct *image)
  {
  if (!image)
    return;
  fclose(image->file);
  free(image);

  return;
  }

