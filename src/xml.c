/*
*				xml.c
*
* Include file for tiff.c.
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	This file part of:	STIFF
*
*	Copyright:		(C) 2009-2010 Emmanuel Bertin -- IAP/CNRS/UPMC
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

#ifdef HAVE_CONFIG_H
#include	"config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "define.h"
#include "globals.h"
#include "fits/fitscat.h"
#include "field.h"
#include "key.h"
#include "prefs.h"
#include "xml.h"

extern pkeystruct	key[];			/* from preflist.h */
extern char		keylist[][32];		/* from preflist.h */
extern time_t		thetime,thetime2;	/* from makeit.c */
extern double		dtime;			/* from makeit.c */
fieldstruct		**field_xml;
int                     nxml, nxmlmax;

/****** init_xml ************************************************************
PROTO	int	init_xml(int nchan)
PURPOSE	Initialize a set of meta-data kept in memory before being written to the
	XML file
INPUT	Number of images (channels).
OUTPUT	RETURN_OK if everything went fine, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	30/12/2009
 ***/
int	init_xml(int nchan)
  {
  if (nchan)
    {
    QMALLOC(field_xml, fieldstruct *, nchan);
    }
  else
    field_xml = NULL;
  nxml = 0;
  nxmlmax = nchan;

  return EXIT_SUCCESS;
  }


/****** end_xml ************************************************************
PROTO	int	end_xml(void)
PURPOSE	Free the set of meta-data kept in memory.
INPUT	-.
OUTPUT	RETURN_OK if everything went fine, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	30/12/2009
 ***/
int	end_xml(void)
  {
  free(field_xml);

  return EXIT_SUCCESS;
  }


/****** update_xml ***********************************************************
PROTO	int	update_xml(fieldstruct *field)
PURPOSE	Update a set of meta-data kept in memory before being written to the
	XML file
INPUT	Pointer to XML field array.
OUTPUT	RETURN_OK if everything went fine, RETURN_ERROR otherwise.
NOTES	Global preferences are used.
AUTHOR	E. Bertin (IAP)
VERSION	30/12/2009
 ***/
int	update_xml(fieldstruct *field)
  {
  field_xml[nxml] = field;
  nxml++;

  return EXIT_SUCCESS;
  }


/****** write_xml ************************************************************
PROTO	int	write_xml(char *filename)
PURPOSE	Save meta-data to an XML file/stream.
INPUT	XML file name.
OUTPUT	RETURN_OK if everything went fine, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	26/07/2006
 ***/
int	write_xml(char *filename)
  {
   FILE		*file;
   int			pipe_flag;

  pipe_flag = 0;
  if (!strcmp(prefs.xml_name, "STDOUT"))
    {
    file = stdout;
    pipe_flag = 1;
    }
  else if (!(file = fopen(prefs.xml_name, "w")))
    return RETURN_ERROR;

  write_xml_header(file);
  write_xml_meta(file, (char *)NULL);

  fprintf(file, "</RESOURCE>\n");
  fprintf(file, "</VOTABLE>\n");

  if (!pipe_flag)
    fclose(file);

  return RETURN_OK;
  }


/****** write_xml_header ******************************************************
PROTO	int	write_xml_header(FILE *file)
PURPOSE	Save an XML-VOtable header to an XML file/stream
INPUT	file or stream pointer.
OUTPUT	RETURN_OK if everything went fine, RETURN_ERROR otherwise.
NOTES	Global preferences are used.
AUTHOR	E. Bertin (IAP)
VERSION	13/10/2010
 ***/
int	write_xml_header(FILE *file)
  {
   char		sysname[16],
		*filename, *rfilename;

/* A short, "relative" version of the filename */
  filename = prefs.tiff_name;
  if (!(rfilename = strrchr(filename, '/')))
    rfilename = filename;
  else
    rfilename++;

  fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(file, "<?xml-stylesheet type=\"text/xsl\" href=\"%s\"?>\n",
	prefs.xsl_name);
  fprintf(file, "<VOTABLE version=\"1.2\" "
	"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
	"xsi:noNamespaceSchemaLocation="
	"\"http://www.ivoa.net/xml/VOTable/v1.2\">\n");
  fprintf(file, "<DESCRIPTION>produced by %s</DESCRIPTION>\n", BANNER);
  fprintf(file, "<!-- VOTable description at "
	"http://www.ivoa.net/Documents/VOTable/ -->\n");
  fprintf(file, "<RESOURCE ID=\"%s\" name=\"%s\">\n", BANNER, rfilename);
  fprintf(file, " <DESCRIPTION>Data related to %s"
	"</DESCRIPTION>\n", BANNER);
  fprintf(file, " <INFO name=\"QUERY_STATUS\" value=\"OK\" />\n");
  sprintf(sysname, "ICRS");

  fprintf(file, " <COOSYS ID=\"J2000\" equinox=\"J2000\""
	" epoch=\"2000.0\" system=\"ICRS\"/>\n");

  return RETURN_OK;
  }


/****** write_xml_meta ********************************************************
PROTO	int	write_xml_meta(FILE *file, char *error)
PURPOSE	Save meta-data to an XML-VOTable file or stream
INPUT	Pointer to the output file (or stream),
	Pointer to an error msg (or NULL).
OUTPUT	RETURN_OK if everything went fine, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	05/10/2010
 ***/
int	write_xml_meta(FILE *file, char *error)
  {
   struct tm		*tm;
   char			psuserb[MAXCHAR],
			pshostb[MAXCHAR],
			pspathb[MAXCHAR],
			*pspath,*psuser, *pshost, *str;
   int			n;

/* Processing date and time if msg error present */
  if (error)
    {
    thetime2 = time(NULL);
    tm = localtime(&thetime2);
    sprintf(prefs.sdate_end,"%04d-%02d-%02d",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
    sprintf(prefs.stime_end,"%02d:%02d:%02d",
        tm->tm_hour, tm->tm_min, tm->tm_sec);
    prefs.time_diff = counter_seconds() - dtime;
    }

/* Username and host computer */
  psuser = pshost = pspath = NULL;
#ifdef HAVE_GETENV
  if (!(psuser = getenv("USERNAME")))	/* Cygwin,... */
    if (!(psuser = getenv("LOGNAME")))	/* Linux,... */
      psuser = getenv("USER");
  if (!(pshost = getenv("HOSTNAME")))
    pshost = getenv("HOST");
  pspath = getenv("PWD");
#endif

  if (!psuser)
    {
    getlogin_r(psuserb, MAXCHAR);
    psuser = psuserb;
    }
  if (!pshost)
    {
    gethostname(pshostb, MAXCHAR);
    pshost = pshostb;
    }
  if (!pspath)
    {
    getcwd(pspathb, MAXCHAR);
    pspath = pspathb;
    }

  fprintf(file, " <RESOURCE ID=\"MetaData\" name=\"MetaData\">\n");
  fprintf(file, "  <DESCRIPTION>%s meta-data</DESCRIPTION>\n", BANNER);
  fprintf(file, "  <INFO name=\"QUERY_STATUS\" value=\"OK\" />\n");
  fprintf(file, "  <PARAM name=\"Software\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta.title;meta.software\" value=\"%s\"/>\n",
	BANNER);
  fprintf(file, "  <PARAM name=\"Version\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta.version;meta.software\" value=\"%s\"/>\n",
	MYVERSION);
  fprintf(file, "  <PARAM name=\"Soft_URL\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta.ref.url;meta.software\" value=\"%s\"/>\n",
	WEBSITE);
  fprintf(file, "  <PARAM name=\"Soft_Auth\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta.bib.author;meta.software\" value=\"%s\"/>\n",
	"Emmanuel Bertin");
  fprintf(file, "  <PARAM name=\"Soft_Ref\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta.bib.bibcode;meta.software\" value=\"%s\"/>\n",
	"2002ASPC..281..228B");
  fprintf(file, "  <PARAM name=\"NThreads\" datatype=\"int\""
	" ucd=\"meta.number;meta.software\" value=\"%d\"/>\n",
    	prefs.nthreads);
  fprintf(file, "  <PARAM name=\"Date\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"time.event.end;meta.software\" value=\"%s\"/>\n",
	prefs.sdate_end);
  fprintf(file, "  <PARAM name=\"Time\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"time.event.end;meta.software\" value=\"%s\"/>\n",
	prefs.stime_end);
  fprintf(file, "  <PARAM name=\"Duration\" datatype=\"float\""
	" ucd=\"time.event;meta.software\" value=\"%.0f\" unit=\"s\"/>\n",
	prefs.time_diff);

  fprintf(file, "  <PARAM name=\"User\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta.curation\" value=\"%s\"/>\n",
	psuser);
  fprintf(file, "  <PARAM name=\"Host\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta.curation\" value=\"%s\"/>\n",
	pshost);
  fprintf(file, "  <PARAM name=\"Path\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta.dataset\" value=\"%s\"/>\n",
	pspath);

  fprintf(file,
	"  <PARAM name=\"Image_Name\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"obs.image;meta.fits\" value=\"%s\"/>\n", prefs.tiff_name);

  if (error)
    {
    fprintf(file, "\n  <!-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	"!!!!!!!!!!!!!!!!!!!! -->\n");
    fprintf(file, "  <!-- !!!!!!!!!!!!!!!!!!!!!! an Error occured"
	" !!!!!!!!!!!!!!!!!!!!! -->\n");
    fprintf(file, "  <!-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	"!!!!!!!!!!!!!!!!!!!! -->\n");
    fprintf(file,"  <PARAM name=\"Error_Msg\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta\" value=\"%s\"/>\n", error);
    fprintf(file, "  <!-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	"!!!!!!!!!!!!!!!!!!!! -->\n");
    fprintf(file, "  <!-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	"!!!!!!!!!!!!!!!!!!!! -->\n\n");
    }

/* Meta-data for each input image */
  fprintf(file, "  <TABLE ID=\"Input_Image_Data\" name=\"Input_Image_Data\">\n");
  fprintf(file, "   <DESCRIPTION>Data gathered by %s for every FITS"
	" input image</DESCRIPTION>\n", BANNER);
  fprintf(file, "   <!-- NChannels may be 0"
	" if an error occurred early in the processing -->\n");
  fprintf(file, "   <PARAM name=\"NChannels\" datatype=\"int\""
	" ucd=\"meta.number;meta.dataset\" value=\"%d\"/>\n",
	nxmlmax>0? nxmlmax-1 : 0);
  fprintf(file, "   <!-- CurrChannel may differ from NChannels"
	" if an error occurred -->\n");
  fprintf(file, "   <PARAM name=\"CurrChannel\" datatype=\"int\""
	" ucd=\"meta.number;meta.dataset\" value=\"%d\"/>\n",
	nxml>0? nxml-1 : 0);
  fprintf(file, "   <FIELD name=\"Image_Index\" datatype=\"int\""
        " ucd=\"meta.record\"/>\n");
  fprintf(file, "   <FIELD name=\"Image_Name\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"obs.image;meta.fits\"/>\n");
  fprintf(file, "   <FIELD name=\"Image_Ident\" datatype=\"char\""
	" arraysize=\"*\" ucd=\"meta.id;obs\"/>\n");
  fprintf(file, "   <FIELD name=\"Image_Size\" datatype=\"int\""
	" arraysize=\"2\" ucd=\"pos.wcs.naxis;obs.image\" unit=\"pix\"/>\n");
  fprintf(file, "   <FIELD name=\"Level_Background\" datatype=\"float\""
	" ucd=\"instr.skyLevel;obs.image;stat.median\" unit=\"adu\"/>\n");
  fprintf(file, "   <FIELD name=\"Level_Min\" datatype=\"float\""
	" ucd=\"phot.flux.sb;obs.image;stat.min\" unit=\"adu\"/>\n");
  fprintf(file, "   <FIELD name=\"Level_Max\" datatype=\"float\""
	" ucd=\"phot.flux.sb;obs.image;stat.max\" unit=\"adu\"/>\n");
  fprintf(file, "   <DATA><TABLEDATA>\n");
  for (n=0; n<nxml; n++)
    fprintf(file, "    <TR>\n"
	"     <TD>%d</TD><TD>%s</TD><TD>%s</TD>\n"
	"     <TD>%d %d</TD><TD>%g</TD><TD>%g</TD><TD>%g</TD>\n"
	"    </TR>\n",
	n+1,
	field_xml[n]->rfilename,
	*(field_xml[n]->ident)? field_xml[n]->ident : "(null)",
	field_xml[n]->size[0], field_xml[n]->size[1],
	field_xml[n]->back,
	field_xml[n]->min,
	field_xml[n]->max);
  fprintf(file, "   </TABLEDATA></DATA>\n");
  fprintf(file, "  </TABLE>\n");

/* Warnings */
  fprintf(file, "  <TABLE ID=\"Warnings\" name=\"Warnings\">\n");
  fprintf(file,
	"   <DESCRIPTION>%s warnings (limited to the last %d)</DESCRIPTION>\n",
	BANNER, WARNING_NMAX);
  fprintf(file, "   <FIELD name=\"Date\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta;time.event.end\"/>\n");
  fprintf(file, "   <FIELD name=\"Time\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta;time.event.end\"/>\n");
  fprintf(file, "   <FIELD name=\"Msg\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"meta\"/>\n");
  fprintf(file, "   <DATA><TABLEDATA>\n");
  for (str = warning_history(); *str; str = warning_history())
    fprintf(file, "    <TR><TD>%10.10s</TD><TD>%8.8s</TD><TD>%s</TD></TR>\n",
	str, str+11, str+22);
  fprintf(file, "   </TABLEDATA></DATA>\n");
  fprintf(file, "  </TABLE>\n");

/* Configuration file */
  fprintf(file, "  <RESOURCE ID=\"Config\" name=\"Config\">\n");
  fprintf(file, "   <DESCRIPTION>%s configuration</DESCRIPTION>\n", BANNER);
  fprintf(file,
	"   <PARAM name=\"Command_Line\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"obs.param\" value=\"%s",
	prefs.command_line[0]);
  for (n=1; n<prefs.ncommand_line; n++)
    fprintf(file, " %s", prefs.command_line[n]);
  fprintf(file, "\"/>\n");
  fprintf(file,
	"   <PARAM name=\"Prefs_Name\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"obs.param;meta.file\" value=\"%s\"/>\n",
	prefs.prefs_name);

  if (!error)
    {
    write_xmlconfigparam(file, "OutFile_Name","","meta.dataset;meta.file","%s");
    write_xmlconfigparam(file, "Image_Type", "", "meta.code;meta.file", "%s");
    write_xmlconfigparam(file, "Bits_Per_Channel", "", "meta.number", "%d");
    write_xmlconfigparam(file, "BigTIFF_Type", "", "meta.code;meta.file", "%s");
    write_xmlconfigparam(file,"Compression_Type","","meta.code;meta.file","%s");
    write_xmlconfigparam(file, "Compression_Quality", "","arith.factor","%d");
    write_xmlconfigparam(file, "Tile_Size", "", "meta.number", "%d");
    write_xmlconfigparam(file, "Pyramid_MinSize", "", "meta.number", "%d");
    write_xmlconfigparam(file, "Binning", "", "meta.number", "%d");
    write_xmlconfigparam(file, "Flip_Type", "", "meta.code;pos", "%s");

    write_xmlconfigparam(file, "Sky_Type", "", "meta.code;instr.skyLevel","%s");
    write_xmlconfigparam(file, "Sky_Level", "adu",
				"instr.skyLevel;stat.median;obs.param", "%g");
    write_xmlconfigparam(file, "Min_Type", "", "meta.code;stat.min", "%s");
    write_xmlconfigparam(file, "Min_Level", "adu",
				"phot.flux.sb;stat.min;obs.param", "%g");
    write_xmlconfigparam(file, "Max_Type", "", "meta.code;stat.max", "%s");
    write_xmlconfigparam(file, "Max_Level", "adu",
				"phot.flux.sb;stat.max;obs.param", "%g");
    write_xmlconfigparam(file, "Satur_Level", "adu",
				"instr.saturation;obs.param", "%g");
    write_xmlconfigparam(file, "Gamma", "", "arith.factor", "%g");
    write_xmlconfigparam(file, "Gamma_Fac", "", "arith.factor", "%g");
    write_xmlconfigparam(file, "Colour_Sat", "", "arith.factor", "%g");
    write_xmlconfigparam(file, "Negative", "", "meta.code", "%c");

    write_xmlconfigparam(file, "VMem_Dir", "", "meta", "%s");
    write_xmlconfigparam(file, "VMem_Max", "Mbyte","meta.number;stat.max","%d");
    write_xmlconfigparam(file, "Mem_Max", "Mbyte", "meta.number;stat.max","%d");

    write_xmlconfigparam(file, "Copy_Header", "", "meta.code", "%c");
    write_xmlconfigparam(file, "Description", "", "meta.title", "%s");
    write_xmlconfigparam(file, "Copyright", "", "meta.curation", "%s");

    write_xmlconfigparam(file, "Verbose_Type", "", "meta.code", "%s");
    write_xmlconfigparam(file, "FITS_Unsigned", "", "meta.code;meta.file", "%c");
    write_xmlconfigparam(file, "Write_XML", "", "meta.code", "%c");
    write_xmlconfigparam(file, "NThreads","","meta.number;meta.software","%d");
    }

  fprintf(file, "  </RESOURCE>\n");
  fprintf(file, " </RESOURCE>\n");

  return RETURN_OK;
  }


/****** write_xmlerror ******************************************************
PROTO	int	write_xmlerror(char *error)
PURPOSE	Save meta-data to a simplified XML file in case of a catched error
INPUT	a character string.
OUTPUT	RETURN_OK if everything went fine, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	26/07/2006
 ***/
void	write_xmlerror(char *filename, char *error)
  {
   FILE			*file;
   int			pipe_flag;

  pipe_flag = 0;
  if (!strcmp(filename, "STDOUT"))
    {
    file = stdout;
    pipe_flag = 1;
    }
  else if (!(file = fopen(filename, "w")))
    return;

  write_xml_header(file);
  write_xml_meta(file, error);

  fprintf(file, "</RESOURCE>\n");
  fprintf(file, "</VOTABLE>\n");

  if (!pipe_flag)
    fclose(file);

  return;
  }


/****** write_xmlconfigparam **************************************************
PROTO	int write_xmlconfigparam(FILE *file, char *name, char *unit,
		char *ucd, char *format)
PURPOSE	Write to a VO-table the configuration parameters.
INPUT	Output stream (file) pointer,
	Name of the parameter keyword,
	unit,
	UCD string,
	printf() format to use in "value".
OUTPUT	RETURN_OK if the keyword exists, RETURN_ERROR otherwise.
NOTES	-.
AUTHOR	E. Bertin (IAP)
VERSION	06/10/2006
 ***/
int	write_xmlconfigparam(FILE *file, char *name, char *unit,
		 char *ucd, char *format)
  {
   char		value[MAXCHAR], uunit[MAXCHAR];
   int		i,j,n;

  for (i=0; key[i].name[0] && cistrcmp(name, key[i].name, FIND_STRICT); i++);
  if (!key[i].name[0])
    return RETURN_ERROR;

  if (*unit)
    sprintf(uunit, " unit=\"%s\"", unit);
  else
    *uunit = '\0';
  switch(key[i].type)
    {
    case P_FLOAT:
      sprintf(value, format, *((double *)key[i].ptr));
      fprintf(file, "   <PARAM name=\"%s\"%s datatype=\"double\""
	" ucd=\"%s\" value=\"%s\"/>\n",
	name, uunit, ucd, value);
      break;
    case P_FLOATLIST:
      n = *(key[i].nlistptr);
      if (n)
        {
        sprintf(value, format, ((double *)key[i].ptr)[0]);
        fprintf(file, "   <PARAM name=\"%s\"%s datatype=\"double\""
		" arraysize=\"%d\" ucd=\"%s\" value=\"%s",
		name, uunit, n, ucd, value);
        for (j=1; j<n; j++)
          {
          sprintf(value, format, ((double *)key[i].ptr)[j]);
          fprintf(file, " %s", value);
          }
        fprintf(file, "\"/>\n");
        }
      else
        fprintf(file, "   <PARAM name=\"%s\"%s datatype=\"double\""
		" ucd=\"%s\" value=\"\"/>\n",
		name, uunit, ucd);
      break;
    case P_INT:
      sprintf(value, format, *((int *)key[i].ptr));
      fprintf(file, "   <PARAM name=\"%s\"%s datatype=\"int\""
	" ucd=\"%s\" value=\"%s\"/>\n",
	name, uunit, ucd, value);
      break;
    case P_INTLIST:
      n = *(key[i].nlistptr);
      if (n)
        {
        sprintf(value, format, ((int *)key[i].ptr)[0]);
        fprintf(file, "   <PARAM name=\"%s\"%s datatype=\"int\""
		" arraysize=\"%d\" ucd=\"%s\" value=\"%s",
		name, uunit, n, ucd, value);
        for (j=1; j<n; j++)
          {
          sprintf(value, format, ((int *)key[i].ptr)[j]);
          fprintf(file, " %s", value);
          }
        fprintf(file, "\"/>\n");
        }
      else
        fprintf(file, "   <PARAM name=\"%s\"%s datatype=\"double\""
		" ucd=\"%s\" value=\"\"/>\n",
		name, uunit, ucd);
      break;
    case P_BOOL:
      sprintf(value, "%c", *((int *)key[i].ptr)? 'T':'F');
      fprintf(file, "   <PARAM name=\"%s\" datatype=\"boolean\""
	" ucd=\"%s\" value=\"%s\"/>\n",
	name, ucd, value);
      break;
    case P_BOOLLIST:
      n = *(key[i].nlistptr);
      if (n)
        {
        sprintf(value, "%c", ((int *)key[i].ptr)[0]? 'T':'F');
        fprintf(file, "   <PARAM name=\"%s\" datatype=\"boolean\""
		" arraysize=\"%d\" ucd=\"%s\" value=\"%s",
		name, n, ucd, value);
        for (j=1; j<n; j++)
          {
          sprintf(value, "%c", ((int *)key[i].ptr)[j]? 'T':'F');
          fprintf(file, " %s", value);
          }
        fprintf(file, "\"/>\n");
        }
      else
        fprintf(file, "   <PARAM name=\"%s\" datatype=\"boolean\""
		" ucd=\"%s\" value=\"\"/>\n",
		name, ucd);
      break;
    case P_STRING:
      sprintf(value, (char *)key[i].ptr);
      fprintf(file, "   <PARAM name=\"%s\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"%s\" value=\"%s\"/>\n",
	name, ucd, value);
      break;
    case P_STRINGLIST:
      n = *(key[i].nlistptr);
      if (n)
        {
        sprintf(value, ((char **)key[i].ptr)[0]);
        fprintf(file, "   <PARAM name=\"%s\" datatype=\"char\""
		" arraysize=\"*\" ucd=\"%s\" value=\"%s",
		name, ucd, value);
        for (j=1; j<n; j++)
          {
          sprintf(value, ((char **)key[i].ptr)[j]);
          fprintf(file, ",%s", value);
          }
        fprintf(file, "\"/>\n");
        }
      else
        fprintf(file, "   <PARAM name=\"%s\" datatype=\"char\""
		" arraysize=\"*\" ucd=\"%s\" value=\"\"/>\n",
		name, ucd);
      break;
    case P_KEY:
      sprintf(value, key[i].keylist[*((int *)key[i].ptr)]);
      fprintf(file, "   <PARAM name=\"%s\" datatype=\"char\" arraysize=\"*\""
	" ucd=\"%s\" value=\"%s\"/>\n",
	name, ucd, value);
      break;
    case P_KEYLIST:
      n = *(key[i].nlistptr);
      if (n)
        {
        sprintf(value, key[i].keylist[((int *)key[i].ptr)[0]]);
        fprintf(file, "   <PARAM name=\"%s\" datatype=\"char\""
		" arraysize=\"*\" ucd=\"%s\" value=\"%s",
		name, ucd, value);
        for (j=1; j<n; j++)
          {
          sprintf(value, key[i].keylist[((int *)key[i].ptr)[j]]);
          fprintf(file, ",%s", value);
          }
        fprintf(file, "\"/>\n");
        }
      else
        fprintf(file, "   <PARAM name=\"%s\" datatype=\"char\""
		" arraysize=\"*\" ucd=\"%s\" value=\"\"/>\n",
		name, ucd);
      break;
    default:
        error(EXIT_FAILURE, "*Internal Error*: Type Unknown",
		" in write_xmlconfigparam()");
    }

  return RETURN_OK;
  }

