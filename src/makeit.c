/*
                                  makeit.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*       Part of:        STIFF
*
*       Author:         E.BERTIN (IAP)
*
*       Contents:       Main loop
*
*       Last modify:    09/01/2005
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "define.h"
#include "globals.h"
#include "fits/fitscat.h"
#include "field.h"
#include "image.h"
#include "prefs.h"

void	print_tabinfo(tabstruct *tab, PIXTYPE back, PIXTYPE max);

/********************************** makeit ***********************************/
void	makeit(void)
  {
  PIXTYPE		grey;
   fieldstruct		**field;
   char			*rfilename;
   int			a, narg;

/* Load input images */
  narg = prefs.nfile;
  QMALLOC(field, fieldstruct *, narg);
/* Go argument by argument */
  NFPRINTF(OUTPUT, "Examining input data...")
  NFPRINTF(OUTPUT, "")
  if (narg != 1 && narg != 3)
    error(EXIT_FAILURE, "*Error*: Only 1 or 3 FITS images must be provided ",
				"");
/* Read the FITS files */
  QPRINTF(OUTPUT, "----- Inputs:\n");
  for (a=0; a<narg; a++)
    {
    field[a] = load_field(prefs.file_name[a]);
    field[a]->back = prefs.back_val[a];
    field[a]->max = prefs.max_val[a];
    field[a]->min = prefs.min_val[a];
    if (prefs.back_type[a]!=BACK_MANUAL || prefs.max_type[a]==MAX_QUANTILE
	 ||  prefs.min_type[a]==MIN_QUANTILE)
      make_imastats(field[a],
	prefs.back_type[a]!=BACK_MANUAL, prefs.min_type[a]==MIN_QUANTILE,
	prefs.max_type[a]==MAX_QUANTILE);
    if (field[a]->max > prefs.sat_val[a])
      warning("MAX_LEVEL exceeds SATUR_LEVEL in ", field[a]->rfilename);

    if (prefs.min_type[a] == MIN_GREYLEVEL)
      {
      grey = pow(prefs.min_val[a], prefs.gamma_fac);
      field[a]->min = (field[a]->max*grey - field[a]->back) / (grey - 1.0);
      }
    print_fieldinfo(field[a]);
    }
/* Display Output image characteristics */
/* A short, "relative" version of the filename */
  if (!(rfilename = strrchr(prefs.tiff_name, '/')))
    rfilename = prefs.tiff_name;
  else
    rfilename++;
  QPRINTF(OUTPUT, "----- Output:\n");
  QPRINTF(OUTPUT, "%s: %dx%d   %d bits   gamma factor = x%4.2f\n",
        rfilename,
	prefs.bin_size>1?
	(field[0]->tab->naxisn[0]+prefs.bin_size-1)/prefs.bin_size
	:field[0]->tab->naxisn[0],
	prefs.bin_size>1?
	(field[0]->tab->naxisn[1]+prefs.bin_size-1)/prefs.bin_size
	:field[0]->tab->naxisn[1],
	narg*8,
	prefs.gamma_fac);

/* Do the conversion */
  image_convert(prefs.tiff_name, field, narg);

  for (a=0; a<narg; a++)
    end_field(field[a]);
  free(field);

  return;
  }


