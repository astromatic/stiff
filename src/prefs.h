 /*
 				prefs.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	STIFF
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	Include for prefs.c.
*
*	Last modify:	10/07/2007
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifndef _PREFS_H_
#define _PREFS_H_

/*--------------------------------- typedefs --------------------------------*/
/*------------------------------- preferences -------------------------------*/
typedef struct
  {
  char		*(file_name[MAXFILE]);	/* Filename(s) of input images */
  int		nfile;			/* Number of input images */
  int		bin_size;		/* Binning factor */
  char		tiff_name[MAXCHAR];	/* Output filename */
  enum {FORMAT_TIFF}
		format_type;		/* Output image format */
  double	gamma;     		/* Video gamma */
  double	gamma_fac;     		/* Luminance gamma correction factor */
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
  enum {QUIET, NORM, WARN, FULL}	verbose_type;	/* display type */
/* Multithreading */
  int		nthreads;			/* Number of active threads */
  }	prefstruct;

prefstruct	prefs;

/*----------------------------- Internal constants --------------------------*/

#define		MAXLIST		32	/* max. nb of list members */

/*-------------------------------- protos -----------------------------------*/
extern int	cistrcmp(char *cs, char *ct, int mode);

extern void	dumpprefs(int state),
		readprefs(char *filename,char **argkey,char **argval,int narg),
		useprefs(void);


#endif

