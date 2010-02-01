 /*
 				main.c

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	STIFF
*
*	Author:		E.BERTIN (IAP)
*
*	Contents:	Parsing of the command line.
*
*	Last modify:	01/01/2010
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include	<ctype.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tiffio.h>

#include	"define.h"
#include	"globals.h"
#include	"fits/fitscat.h"
#include	"prefs.h"

#define		SYNTAX \
EXECUTABLE " [<fits_file1>] [<fits_file2> <fits_file3>]\n"\
"      [-c <configuration_file>] [-<keyword> <value>]\n"\
"> to dump a default configuration file: " EXECUTABLE " -d \n" \
"> to dump a default extended configuration file: " EXECUTABLE " -dd \n"

extern const char	notokstr[];

/********************************** main ************************************/
int	main(int argc, char *argv[])

  {
   double	tdiff, lines,mpix;
   float	ver;
   char		**argkey, **argval, *str;
   int		a, narg, nim, opt,opt2;

  if (argc<2)
    {
    fprintf(OUTPUT, "\n         %s  version %s (%s)\n", BANNER,MYVERSION,DATE);
    fprintf(OUTPUT, "\nby %s\n", COPYRIGHT);
    fprintf(OUTPUT, "visit %s\n", WEBSITE);
    if ((ver=atof(TIFFGetVersion() + 16)) >= 4.0)
      fprintf(OUTPUT, "\nBigTIFF support is: ON (libTIFF V%3.1f)\n", ver);
    else
      fprintf(OUTPUT, "\nBigTIFF support is: OFF (libTIFF V%3.1f)\n", ver);
    error(EXIT_SUCCESS, "SYNTAX: ", SYNTAX);
    }
  QMALLOC(argkey, char *, argc);
  QMALLOC(argval, char *, argc);

/*default parameters */
  prefs.command_line = argv;
  prefs.ncommand_line = argc;
  prefs.nfile = 1;
  prefs.file_name[0] = "image";
  strcpy(prefs.prefs_name, "stiff.conf");
  narg = nim = 0;

  for (a=1; a<argc; a++)
    {
    if (*(argv[a]) == '-')
      {
      opt = (int)argv[a][1];
      if (strlen(argv[a])<4 || opt == '-')
        {
        opt2 = (int)tolower((int)argv[a][2]);
        if (opt == '-')
          {
          opt = opt2;
          opt2 = (int)tolower((int)argv[a][3]);
          }
        switch(opt)
          {
          case 'c':
            if (a<(argc-1))
              strcpy(prefs.prefs_name, argv[++a]);
            break;
          case 'd':
            dumpprefs(opt2=='d' ? 1 : 0);
            exit(EXIT_SUCCESS);
            break;
          case 'v':
            printf("%s version %s (%s)\n", BANNER,MYVERSION,DATE);
            exit(EXIT_SUCCESS);
            break;
          case 'h':
          default:
            error(EXIT_SUCCESS,"SYNTAX: ", SYNTAX);
          }
        }
      else
        {
        argkey[narg] = &argv[a][1];
        argval[narg++] = argv[++a];
        }       
      }
    else
      {
/*---- The input image filename(s) */
      for(; (a<argc) && (*argv[a]!='-'); a++)
        for (str=NULL;(str=strtok(str?NULL:argv[a], notokstr)); nim++)
          if (nim<MAXFILE)
            prefs.file_name[nim] = str;
          else
            error(EXIT_FAILURE, "*Error*: Too many input images: ", str);
      prefs.nfile = nim;
      a--;
      }
    }

  if ((ver=atof(TIFFGetVersion() + 16)) >= 4.0)
    fprintf(OUTPUT, "\nBigTIFF support is: ON (libTIFF V%3.1f)\n", ver);
  else
    fprintf(OUTPUT, "\nBigTIFF support is: OFF (libTIFF V%3.1f)\n", ver);

  readprefs(prefs.prefs_name, argkey, argval, narg);
  preprefs();
  useprefs();

  free(argkey);
  free(argval);


  makeit();

  NFPRINTF(OUTPUT, "");
  tdiff = prefs.time_diff>0.0? prefs.time_diff : 0.001;
  lines = prefs.nlines/tdiff;
  mpix = prefs.npix/tdiff/1e6;
  NPRINTF(OUTPUT,
        "> All done (in %.1f s: %.1f line%s/s , %.1f Mpixel%s/s)\n",
        prefs.time_diff, lines, lines>1.0? "s":"", mpix, mpix>1.0? "s":"");

  exit(EXIT_SUCCESS);
  }

