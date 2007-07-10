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
*	Last modify:	10/07/2007
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
   static char	prefsname[MAXCHAR];
   char		**argkey, **argval, *str;
   int		a, narg, nim, opt,opt2;

  if (argc<2)
    {
    fprintf(OUTPUT, "\n         %s  version %s (%s)\n", BANNER,MYVERSION,DATE);
    fprintf(OUTPUT, "\nby %s\n", COPYRIGHT);
    fprintf(OUTPUT, "visit %s\n", WEBSITE);
    error(EXIT_SUCCESS, "SYNTAX: ", SYNTAX);
    }
  QMALLOC(argkey, char *, argc);
  QMALLOC(argval, char *, argc);

/*default parameters */
  prefs.nfile = 1;
  prefs.file_name[0] = "image";
  strcpy(prefsname, "stiff.conf");
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
              strcpy(prefsname, argv[++a]);
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

  readprefs(prefsname, argkey, argval, narg);
  useprefs();

  free(argkey);
  free(argval);

  makeit();

  NFPRINTF(OUTPUT, "All done");
  NPRINTF(OUTPUT, "\n");

  exit(EXIT_SUCCESS);
  }

