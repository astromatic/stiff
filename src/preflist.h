 /*
 				preflist.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	STIFF
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	Keywords for the configuration file.
*
*	Last modify:	10/01/2005
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#include "key.h"

#ifndef _PREFS_H_
#include "prefs.h"
#endif

#ifndef _IMAGE_H_
#include "image.h"
#endif

int idummy;

pkeystruct key[] =
 {
  {"BINNING", P_INT, &prefs.bin_size, 1, 32768, 0.0,0.0},
  {"COLOUR_SAT", P_FLOAT, &prefs.colour_sat, 0,0, 0.0,10.0},
  {"GAMMA", P_FLOAT, &prefs.gamma, 0,0, 1e-3,10.0},
  {"GAMMA_FAC", P_FLOAT, &prefs.gamma_fac, 0,0, 1e-3,10.0},
  {"IMAGE_TYPE", P_KEY, &prefs.format_type, 0,0, 0.0,0.0,
   {"TIFF",""}},
  {"NEGATIVE", P_BOOL, &prefs.neg_flag},
  {"OUTFILE_NAME", P_STRING, prefs.tiff_name},
  {"MIN_TYPE",  P_KEYLIST, prefs.min_type, 0,0, 0.0,0.0,
   {"QUANTILE", "MANUAL", "GREYLEVEL"}, 1, MAXFILE, &prefs.nmin_type},
  {"MIN_LEVEL",  P_FLOATLIST, prefs.min_val, 0,0, -1e31,1e31,
   {""}, 1, MAXFILE, &prefs.nmin_val},
  {"MAX_TYPE",  P_KEYLIST, prefs.max_type, 0,0, 0.0,0.0,
   {"QUANTILE", "MANUAL"}, 1, MAXFILE, &prefs.nmax_type},
  {"MAX_LEVEL",  P_FLOATLIST, prefs.max_val, 0,0, -1e31,1e31,
   {""}, 1, MAXFILE, &prefs.nmax_val},
  {"SATUR_LEVEL", P_FLOATLIST, prefs.sat_val, 0,0, -1e31,1e31,
   {""}, 1, MAXFILE, &prefs.nsat_val},
  {"SKY_LEVEL",  P_FLOATLIST, prefs.back_val, 0,0, -1e31,1e31,
   {""}, 1, MAXFILE, &prefs.nback_val},
  {"SKY_TYPE", P_KEYLIST, prefs.back_type, 0,0, 0.0,0.0,
   {"AUTO","MANUAL",""}, 1, MAXFILE, &prefs.nback_type},
  {"VERBOSE_TYPE", P_KEY, &prefs.verbose_type, 0,0, 0.0,0.0,
   {"QUIET","NORMAL","FULL",""}},
  {""}
 };

char			keylist[sizeof(key)/sizeof(pkeystruct)][16];
const char		notokstr[] = {" \t=,;\n\r\""};

char *default_prefs[] =
 {
"# Default configuration file for " BANNER " " MYVERSION,
"# EB " DATE,
"#",
"OUTFILE_NAME    stiff.tif               # Name of the output file",
"*IMAGE_TYPE      TIFF                    # Output image format",
"BINNING         1                       # Binning factor for the data",
"GAMMA           2.2                     # Display gamma",
"GAMMA_FAC       1.0                     # Luminance gamma correction factor",
"COLOUR_SAT      1.0                     # Colour saturation (0.0 = B&W)",
"NEGATIVE        N                       # Make negative of the image",
" ",
"#------------------------------- Dynamic range ------------------------------",
" ",
"SKY_TYPE        AUTO                    # Sky-level: \"AUTO\" or \"MANUAL\"",
"SKY_LEVEL       0.0                     # Background level for each image",
"MIN_TYPE        GREYLEVEL               # Min-level: \"QUANTILE\", \"MANUAL\"",
"                                        # or \"GREYLEVEL\"",
"MIN_LEVEL       0.005                   # Minimum value or quantile",
"MAX_TYPE        QUANTILE                # Max-level: \"QUANTILE\" or \"MANUAL\"",
"MAX_LEVEL       0.995                   # Maximum value or quantile",
"*SATUR_LEVEL     40000.0                 # FITS data saturation level(s)",
" ",
"#------------------------------ Miscellaneous ---------------------------------",
" ",
"VERBOSE_TYPE    NORMAL                  # \"QUIET\",\"NORMAL\" or \"FULL\"",
  ""
 };
