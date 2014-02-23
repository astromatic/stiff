/*
*				preflist.h
*
* Configuration keyword definitions
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
*	Last modified:		23/02/2014
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include "key.h"

#ifndef _PREFS_H_
#include "prefs.h"
#endif

#ifndef _IMAGE_H_
#include "image.h"
#endif

#ifndef _TIFF_H_
#include "tiff.h"
#endif

#ifndef _XML_H_
#include "xml.h"
#endif

#ifdef  USE_THREADS
#define THREADS_PREFMAX THREADS_NMAX
#else
#define THREADS_PREFMAX 65535
#endif

int idummy;

pkeystruct key[] =
 {
  {"BIGTIFF_TYPE", P_KEY, &prefs.bigtiff_type, 0,0, 0.0,0.0,
   {"AUTO","NEVER","ALWAYS",""}},
  {"BINNING", P_INTLIST, prefs.bin_size, 1, 32768, 0.0,0.0,
   {""}, 1, 2, &prefs.nbin_size},
  {"BITS_PER_CHANNEL", P_INT, &prefs.bpp, -32, 16},
  {"COMPRESSION_QUALITY", P_INT, &prefs.compress_quality, 0,100},
  {"COMPRESSION_TYPE", P_KEY, &prefs.compress_type, 0,0, 0.0,0.0,
   {"NONE", "LZW", "JPEG", "DEFLATE", "ADOBE-DEFLATE", ""}},
  {"COLOUR_SAT", P_FLOAT, &prefs.colour_sat, 0,0, 0.0,10.0},
  {"COPY_HEADER", P_BOOL, &prefs.header_flag},
  {"COPYRIGHT", P_STRING, prefs.copyright},
  {"DESCRIPTION", P_STRING, prefs.description},
/*
  {"DOWNSAMPLING_TYPE", P_KEY, &prefs.downsamp_type, 0,0, 0.0,0.0,
   {"4:4:4", "4:2:2", "4:2:0", ""}},
*/
  {"FITS_UNSIGNED", P_BOOL, &prefs.fitsunsigned_flag},
  {"FLIP_TYPE", P_KEY, &prefs.flip_type, 0,0, 0.0,0.0,
   {"NONE", "X", "Y", "XY", ""}},
  {"GAMMA", P_FLOAT, &prefs.gamma, 0,0, 1e-3,10.0},
  {"GAMMA_FAC", P_FLOAT, &prefs.gamma_fac, 0,0, 1e-3,10.0},
  {"GAMMA_TYPE", P_KEY, &prefs.gamma_type, 0,0, 0.0,0.0,
   {"POWER-LAW", "SRGB", "REC.709", ""}},
  {"IMAGE_TYPE", P_KEY, &prefs.format_type, 0,0, 0.0,0.0,
   {"AUTO", "TIFF", "TIFF-PYRAMID", ""}},
  {"MIN_TYPE",  P_KEYLIST, prefs.min_type, 0,0, 0.0,0.0,
   {"QUANTILE", "MANUAL", "GREYLEVEL"}, 1, MAXFILE, &prefs.nmin_type},
  {"MIN_LEVEL",  P_FLOATLIST, prefs.min_val, 0,0, -1e31,1e31,
   {""}, 1, MAXFILE, &prefs.nmin_val},
  {"MAX_TYPE",  P_KEYLIST, prefs.max_type, 0,0, 0.0,0.0,
   {"QUANTILE", "MANUAL"}, 1, MAXFILE, &prefs.nmax_type},
  {"MAX_LEVEL",  P_FLOATLIST, prefs.max_val, 0,0, -1e31,1e31,
   {""}, 1, MAXFILE, &prefs.nmax_val},
  {"MEM_MAX", P_INT, &prefs.mem_max, 1, 1000000000},
  {"NEGATIVE", P_BOOL, &prefs.neg_flag},
  {"NTHREADS", P_INT, &prefs.nthreads, 0, THREADS_PREFMAX},
  {"OUTFILE_NAME", P_STRING, prefs.tiff_name},
  {"PYRAMID_MINSIZE", P_INTLIST, prefs.min_size, 1, 32768, 0.0,0.0,
   {""}, 1, 2, &prefs.nmin_size},
  {"SATUR_LEVEL", P_FLOATLIST, prefs.sat_val, 0,0, -1e31,1e31,
   {""}, 1, MAXFILE, &prefs.nsat_val},
  {"SKY_LEVEL",  P_FLOATLIST, prefs.back_val, 0,0, -1e31,1e31,
   {""}, 1, MAXFILE, &prefs.nback_val},
  {"SKY_TYPE", P_KEYLIST, prefs.back_type, 0,0, 0.0,0.0,
   {"AUTO", "MANUAL", ""}, 1, MAXFILE, &prefs.nback_type},
  {"TILE_SIZE", P_INT, &prefs.tile_size, 16, 32768},
  {"VERBOSE_TYPE", P_KEY, &prefs.verbose_type, 0,0, 0.0,0.0,
   {"QUIET", "NORMAL", "FULL",""}},
  {"VMEM_DIR", P_STRING, prefs.swapdir_name},
  {"VMEM_MAX", P_INT, &prefs.vmem_max, 1, 1000000000},
  {"WRITE_XML", P_BOOL, &prefs.xml_flag},
  {"XML_NAME", P_STRING, prefs.xml_name},
  {"XSL_URL", P_STRING, prefs.xsl_name},
  {""}
 };

char			keylist[sizeof(key)/sizeof(pkeystruct)][32];
const char		notokstr[] = {" \t=,;\n\r"};

char *default_prefs[] =
 {
"# Default configuration file for " BANNER " " MYVERSION,
"# EB " DATE,
"#",
"OUTFILE_NAME           stiff.tif       # Name of the output file",
"IMAGE_TYPE             AUTO            # Output image format: AUTO, TIFF,",
"                                       # or TIFF-PYRAMID",
"BITS_PER_CHANNEL       8               # 8, 16 for int, -32 for float",
"*BIGTIFF_TYPE           AUTO            # Use BigTIFF? NEVER,ALWAYS or AUTO",
"*COMPRESSION_TYPE       LZW             # NONE,LZW,JPEG,DEFLATE or ADOBE-DEFLATE",
"*COMPRESSION_QUALITY    90              # JPEG compression quality (%)",
/*
"*DOWNSAMPLING_TYPE      4:4:4           # Chrominance downsampling for JPEG:",
"*                                       # 4:4:4, 4:2:2 or 4:2:0",
*/
"*TILE_SIZE              256             # TIFF tile-size",
"*PYRAMID_MINSIZE        256             # Minimum plane size in TIFF pyramid",
"BINNING                1               # Binning factor for the data",
"*FLIP_TYPE              NONE            # NONE, or flip about X, Y or XY",
"*FITS_UNSIGNED          N               # Treat FITS integers as unsigned",
" ",
"#------------------------------- Dynamic range ------------------------------",
" ",
"SKY_TYPE               AUTO            # Sky-level: \"AUTO\" or \"MANUAL\"",
"SKY_LEVEL              0.0             # Background level for each image",
"MIN_TYPE               GREYLEVEL       # Min-level: \"QUANTILE\", \"MANUAL\"",
"                                       # or \"GREYLEVEL\"",
"MIN_LEVEL              0.001           # Minimum value, quantile or grey level",
"MAX_TYPE               QUANTILE        # Max-level: \"QUANTILE\" or \"MANUAL\"",
"MAX_LEVEL              0.999           # Maximum value or quantile",
"*SATUR_LEVEL            40000.0         # FITS data saturation level(s)",
"GAMMA_TYPE             POWER-LAW       # Gamma correction: POWER-LAW, SRGB or",
"*                                       # REC.709",
"GAMMA                  2.2             # Display gamma",
"GAMMA_FAC              1.0             # Luminance gamma correction factor",
"COLOUR_SAT             1.0             # Colour saturation (0.0 = B&W)",
"NEGATIVE               N               # Make negative of the image",
" ",
"*#------------------------------ Memory management -----------------------------",
"*",
"*VMEM_DIR               .               # Directory path for swap files",
"*VMEM_MAX               1048576         # Maximum amount of virtual memory (MB)",
"*MEM_MAX                1024            # Maximum amount of usable RAM (MB)",
"*",
"#------------------------------ Miscellaneous ---------------------------------",
" ",
"VERBOSE_TYPE           NORMAL          # QUIET, NORMAL or FULL",
"COPY_HEADER            N               # Copy FITS header to description field?",
"DESCRIPTION            \"STIFF image\"   # Image content description",
"COPYRIGHT              AstrOmatic.net  # Copyright notice",
"WRITE_XML              Y               # Write XML file (Y/N)?",
"XML_NAME               stiff.xml       # Filename for XML output",
"*XSL_URL                " XSL_URL,
"*                                       # Filename for XSL style-sheet",
#ifdef USE_THREADS
"NTHREADS               0               # Number of simultaneous threads for",
"                                       # the SMP version of " BANNER,
"                                       # 0 = automatic",
#else
"NTHREADS              1                # 1 single thread",
#endif
""
 };
