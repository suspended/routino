/***************************************
 $Header: /home/amb/CVS/routino/src/translations.c,v 1.1 2010-04-10 18:33:58 amb Exp $

 Load the translations from a file and the functions for handling them.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010 Andrew M. Bishop

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "functions.h"
#include "translations.h"
#include "xmlparse.h"


/* Global variables - default English values */

char *translate_heading[9] ={"South","South-West","West","North-West","North","North-East","East","South-East","South"};
char *translate_turn[9]    ={"Very sharp left","Sharp left","Left","Slight left","Straight on","Slight right","Right","Sharp right","Very sharp right"};

char *translate_gpx_desc ="%s between 'start' and 'finish' waypoints";
char *translate_gpx_name ="%s Route";
char *translate_gpx_step ="%s on '%s' for %.3f km, %.1 min";
char *translate_gpx_final="Total Journey %.1f km, %d minutes";

char *translate_gpx_shortest="Shortest";
char *translate_gpx_quickest="Quickest";

char *translate_gpx_start="START";
char *translate_gpx_inter="INTER";
char *translate_gpx_trip="TRIP";
char *translate_gpx_finish="FINISH";


/* Local variables */

/*+ The language that is to be stored. +*/
static const char *store_lang=NULL;

/*+ This current language is to be stored. +*/
static int store=0;

/*+ The chosen language has been stored. +*/
static int stored=0;

/*+ The number of errors found in the XML file. +*/
static int translations_parse_error=0;


/* The XML tag processing function prototypes */

static void language_function(int _type_,const char *lang);

static void gpx_final_function(int _type_,const char *text);
static void gpx_step_function(int _type_,const char *text);
static void gpx_name_function(int _type_,const char *text);
static void gpx_desc_function(int _type_,const char *text);
static void gpx_waypoint_function(int _type_,const char *type,const char *string);
static void gpx_route_function(int _type_,const char *type,const char *string);

static void heading_function(int _type_,const char *direction,const char *string);
static void turn_function(int _type_,const char *direction,const char *string);


/* The XML tag definitions */

/*+ The TurnType type tag. +*/
static xmltag turn_tag=
              {"turn",
               2, {"direction","string"},
               turn_function,
               {NULL}};

/*+ The HeadingType type tag. +*/
static xmltag heading_tag=
              {"heading",
               2, {"direction","string"},
               heading_function,
               {NULL}};

/*+ The HTMLType type tag. +*/
static xmltag output_html_tag=
              {"output-html",
               0, {NULL},
               NULL,
               {NULL}};

/*+ The GPXRouteType type tag. +*/
static xmltag gpx_route_tag=
              {"route",
               2, {"type","string"},
               gpx_route_function,
               {NULL}};

/*+ The GPXWaypointType type tag. +*/
static xmltag gpx_waypoint_tag=
              {"waypoint",
               2, {"type","string"},
               gpx_waypoint_function,
               {NULL}};

/*+ The GPXDescType type tag. +*/
static xmltag gpx_desc_tag=
              {"desc",
               1, {"text"},
               gpx_desc_function,
               {NULL}};

/*+ The GPXNameType type tag. +*/
static xmltag gpx_name_tag=
              {"name",
               1, {"text"},
               gpx_name_function,
               {NULL}};

/*+ The GPXStepType type tag. +*/
static xmltag gpx_step_tag=
              {"step",
               1, {"text"},
               gpx_step_function,
               {NULL}};

/*+ The GPXFinalType type tag. +*/
static xmltag gpx_final_tag=
              {"final",
               1, {"text"},
               gpx_final_function,
               {NULL}};

/*+ The GPXType type tag. +*/
static xmltag output_gpx_tag=
              {"output-gpx",
               0, {NULL},
               NULL,
               {&gpx_route_tag,&gpx_waypoint_tag,&gpx_desc_tag,&gpx_name_tag,&gpx_step_tag,&gpx_final_tag,NULL}};

/*+ The languageType type tag. +*/
static xmltag language_tag=
              {"language",
               1, {"lang"},
               language_function,
               {&turn_tag,&heading_tag,&output_html_tag,&output_gpx_tag,NULL}};

/*+ The RoutinoTranslationsType type tag. +*/
static xmltag routino_translations_tag=
              {"routino-translations",
               0, {NULL},
               NULL,
               {&language_tag,NULL}};

/*+ The xmlType type tag. +*/
static xmltag xml_tag=
              {"xml",
               2, {"version","encoding"},
               NULL,
               {NULL}};


/*+ The complete set of tags at the top level. +*/
static xmltag *xml_toplevel_tags[]={&xml_tag,&routino_translations_tag,NULL};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the TurnType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *direction The contents of the 'direction' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void turn_function(int _type_,const char *direction,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    int d;

    if(!direction || !string)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'direction' and 'string' attributes must be specified in <turn> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    if(!isdigit(direction[0]) && (direction[0]!='-' || !isdigit(direction[1])))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid direction '%s' in <turn> tag.\n",ParseXML_LineNumber(),direction);
       translations_parse_error=1;
       return;
      }

    d=atoi(direction)+4;

    if(d<0 || d>8)
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid direction '%s' in <turn> tag.\n",ParseXML_LineNumber(),direction);
       translations_parse_error=1;
       return;
      }

    translate_turn[d]=strcpy(malloc(strlen(string)+1),string);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HeadingType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *direction The contents of the 'direction' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void heading_function(int _type_,const char *direction,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    int d;

    if(!direction || !string)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'direction' and 'string' attributes must be specified in <heading> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    if(!isdigit(direction[0]) && (direction[0]!='-' || !isdigit(direction[1])))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid direction '%s' in <heading> tag.\n",ParseXML_LineNumber(),direction);
       translations_parse_error=1;
       return;
      }

    d=atoi(direction)+4;

    if(d<0 || d>8)
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid direction '%s' in <heading> tag.\n",ParseXML_LineNumber(),direction);
       translations_parse_error=1;
       return;
      }

    translate_heading[d]=strcpy(malloc(strlen(string)+1),string);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXRouteType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void gpx_route_function(int _type_,const char *type,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    if(!type || !*type)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'type' attribute must be specified in <route> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    if(!string || !*string)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'string' attribute must be specified in <route> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    if(!strcmp(type,"shortest"))
       translate_gpx_shortest=strcpy(malloc(strlen(string)+1),string);
    else if(!strcmp(type,"quickest"))
       translate_gpx_quickest=strcpy(malloc(strlen(string)+1),string);
    else
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'type' attribute must be 'shortest' or 'quickest' in <route> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXWaypointType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void gpx_waypoint_function(int _type_,const char *type,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    if(!type || !*type)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'type' attribute must be specified in <waypoint> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    if(!string || !*string)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'string' attribute must be specified in <waypoint> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    if(!strcmp(type,"start"))
       translate_gpx_start=strcpy(malloc(strlen(string)+1),string);
    else if(!strcmp(type,"inter"))
       translate_gpx_inter=strcpy(malloc(strlen(string)+1),string);
    else if(!strcmp(type,"trip"))
       translate_gpx_trip=strcpy(malloc(strlen(string)+1),string);
    else if(!strcmp(type,"finish"))
       translate_gpx_finish=strcpy(malloc(strlen(string)+1),string);
    else
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'type' attribute must be 'start', 'inter', 'trip' or 'finish' in <waypoint> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXDescType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void gpx_desc_function(int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    if(!text || !*text)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'text' attribute must be specified in <desc> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    translate_gpx_desc=strcpy(malloc(strlen(text)+1),text);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXNameType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void gpx_name_function(int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    if(!text || !*text)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'text' attribute must be specified in <name> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    translate_gpx_name=strcpy(malloc(strlen(text)+1),text);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXStepType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void gpx_step_function(int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    if(!text || !*text)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'text' attribute must be specified in <step> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    translate_gpx_step=strcpy(malloc(strlen(text)+1),text);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXFinalType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void gpx_final_function(int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    if(!text || !*text)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'text' attribute must be specified in <final> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    translate_gpx_final=strcpy(malloc(strlen(text)+1),text);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the languageType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *lang The contents of the 'lang' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void language_function(int _type_,const char *lang)
{
 static int first=1;

 if(_type_&XMLPARSE_TAG_START)
   {
    if(!lang || !*lang)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'lang' attribute must be specified in <language> tag.\n",ParseXML_LineNumber());
       translations_parse_error=1;
       return;
      }

    if(!store_lang && first)
       store=1;
    else if(!strcmp(store_lang,lang))
       store=1;
    else
       store=0;

    first=0;
   }

 if(_type_&XMLPARSE_TAG_END && store)
   {
    store=0;
    stored=1;
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The XML translation parser.

  int ParseXMLTranslations Returns 0 if OK or something else in case of an error.

  const char *filename The name of the file to read.

  const char *language The language to search for (NULL means first in file).
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXMLTranslations(const char *filename,const char *language)
{
 int retval;

 store_lang=language;

 if(!ExistsFile(filename))
   {
    fprintf(stderr,"Error: Specified translations file '%s' does not exist.\n",filename);
    return(1);
   }

 FILE *file=fopen(filename,"r");

 if(!file)
   {
    fprintf(stderr,"Error: Cannot open translations file '%s' for reading.\n",filename);
    return(1);
   }

 translations_parse_error=0;

 retval=ParseXML(file,xml_toplevel_tags,2);

 fclose(file);

 if(retval || translations_parse_error)
    return(1);

 if(language && !stored)
    fprintf(stderr,"Warning: Cannot find translations for language '%s' using English instead.\n",language);

 return(0);
}
