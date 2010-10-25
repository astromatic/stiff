/*
*				prefs.h
*
* Include file for prefs.c.
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2003-2010 Emmanuel Bertin -- IAP/CNRS/UPMC
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

#ifndef _PREFS_H_
#define _PREFS_H_

/*--------------------------------- typedefs --------------------------------*/
/*------------------------------- preferences -------------------------------*/
typedef struct
  {
  char		**command_line;		/* Command line */
  int 		ncommand_line;		/* nb of params */
  char		prefs_name[MAXCHAR];	/* prefs filename */
  char		*(file_name[MAXFILE]);	/* Filename(s) of input images */
  int		nfile;			/* Number of input images */
  int		header_flag;		/* Copy FITS header to desc. field? */
  char		copyright[MAXCHAR];	/* Copyright notice */
  char		description[MAXCHAR];	/* Image description */
  int		bin_size[2];		/* Binning factor */
  int		nbin_size;		/* Number of parameters */
  int		tile_size;		/* Dimension of TIFF tiles */
  int		min_size[2];		/* Minimum size of pyramid plane */
  int		nmin_size;		/* Number of parameters */
  char		tiff_name[MAXCHAR];	/* Output filename */
  enum {FORMAT_AUTO, FORMAT_TIFF, FORMAT_TIFF_PYRAMID}
		format_type,
		format_type2;		/* Output image format */
  int		bigtiff_type;		/* BigTIFF support option */
  int		bpp;			/* Number of bits per pixels */
  int		compress_type;		/* TIFF compression algorithm */
  int		compress_quality;	/* (JPEG) compression quality index */
/*
  int		downsamp_type;		* Chrominance downsampling *
*/
  enum {FLIP_NONE, FLIP_X, FLIP_Y, FLIP_XY}
		flip_type;		/* Image flip type */
  double	gamma;     		/* Video gamma */
  double	gamma_fac;     		/* Luminance gamma correction factor */
  enum {GAMMA_POWERLAW, GAMMA_SRGB, GAMMA_REC709}
		gamma_type;		/* Gamma correction type */
  double	colour_sat;    		/* Saturation factor in output */
  int		neg_flag;		/* Negate image? */
  double	sky_intensity;		/* Intensity of the background in % */
  enum {BACK_AUTO,BACK_MANUAL}
		back_type[MAXFILE];	/* Manual or Automatic back sub. */
  int		nback_type;		/* Number of parameters */
  double	back_val[MAXFILE];	/* Background value to subtract */
  int		nback_val;		/* Number of parameters */
  enum {MIN_QUANTILE,MIN_MANUAL, MIN_GREYLEVEL}
		min_type[MAXFILE];	/* Manual or quantile minimum level */
  int		nmin_type;		/* Number of parameters */
  double	min_val[MAXFILE];	/* Minimum level */
  int		nmin_val;		/* Number of parameters */
  enum {MAX_QUANTILE,MAX_MANUAL}
		max_type[MAXFILE];	/* Manual or quantile maximum level */
  int		nmax_type;		/* Number of parameters */
  double	max_val[MAXFILE];	/* Maximum level */
  int		nmax_val;		/* Number of parameters */
  double	sat_val[MAXFILE];	/* FITS saturation level */
  int		nsat_val;		/* Number of parameters */
/* Virtual memory handling */
  int		mem_max;		/* Max amount of allocatable RAM */ 
  int		vmem_max;		/* Max amount of allocatable VMEM */ 
  char          swapdir_name[MAXCHAR];  /* Name of virtual mem directory */
/* Multithreading */
  int		nthreads;		/* Number of active threads */
/* Misc */
  enum {QUIET, NORM, WARN, FULL}	verbose_type;	/* display type */
  double	nlines;			/* Image height in pixels */
  double	npix;			/* Number of image pixels */
  int		fitsunsigned_flag;	/* Force unsign FITS */
/* XML */
  int 		xml_flag;		/* Write XML file? */
  char		xml_name[MAXCHAR];	/* XML file name */
  char		xsl_name[MAXCHAR];	/* XSL file name (or URL) */
/* Date and Time */
  char		sdate_start[12];	/* STIFF start date */
  char		stime_start[12];	/* STIFF start time */
  char		sdate_end[12];		/* STIFF end date */
  char		stime_end[12];		/* STIFF end time */
  double	time_diff;		/* Execution time */
  }	prefstruct;

prefstruct	prefs;

/*----------------------------- Internal constants --------------------------*/

#define		MAXLIST		32	/* max. nb of list members */

/*-------------------------------- protos -----------------------------------*/
extern int	cistrcmp(char *cs, char *ct, int mode);

extern char	*mystrtok(char *str, const char *delim);

extern void	dumpprefs(int state),
		preprefs(void),
		readprefs(char *filename,char **argkey,char **argval,int narg),
		useprefs(void);


#endif

