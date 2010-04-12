/***************************************
 $Header: /home/amb/CVS/routino/src/translations.c,v 1.3 2010-04-12 17:33:52 amb Exp $

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


/* The XML tag processing function prototypes */

//static int xml_function(int _type_,const char *version,const char *encoding);
//static int routino_translations_function(int _type_);
static int language_function(int _type_,const char *lang);
//static int output_gpx_function(int _type_);
static int gpx_final_function(int _type_,const char *text);
static int gpx_step_function(int _type_,const char *text);
static int gpx_name_function(int _type_,const char *text);
static int gpx_desc_function(int _type_,const char *text);
static int gpx_waypoint_function(int _type_,const char *type,const char *string);
static int gpx_route_function(int _type_,const char *type,const char *string);
//static int output_html_function(int _type_);
static int heading_function(int _type_,const char *direction,const char *string);
static int turn_function(int _type_,const char *direction,const char *string);


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

  int turn_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *direction The contents of the 'direction' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int turn_function(int _type_,const char *direction,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    int d;

    XMLPARSE_ASSERT_INTEGER("turn",direction,d);
    XMLPARSE_ASSERT_STRING("turn",string);

    d+=4;

    if(d<0 || d>8)
       XMLPARSE_INVALID("turn",direction);

    translate_turn[d]=strcpy(malloc(strlen(string)+1),string);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HeadingType XSD type is seen

  int heading_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *direction The contents of the 'direction' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int heading_function(int _type_,const char *direction,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    int d;

    XMLPARSE_ASSERT_INTEGER("heading",direction,d);
    XMLPARSE_ASSERT_STRING("heading",string);

    d+=4;

    if(d<0 || d>8)
       XMLPARSE_INVALID("heading",direction);

    translate_heading[d]=strcpy(malloc(strlen(string)+1),string);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLType XSD type is seen

  int output_html_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int output_html_function(int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXRouteType XSD type is seen

  int gpx_route_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int gpx_route_function(int _type_,const char *type,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    XMLPARSE_ASSERT_STRING("route",type);
    XMLPARSE_ASSERT_STRING("route",string);

    if(!strcmp(type,"shortest"))
       translate_gpx_shortest=strcpy(malloc(strlen(string)+1),string);
    else if(!strcmp(type,"quickest"))
       translate_gpx_quickest=strcpy(malloc(strlen(string)+1),string);
    else
       XMLPARSE_INVALID("route",type);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXWaypointType XSD type is seen

  int gpx_waypoint_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int gpx_waypoint_function(int _type_,const char *type,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    XMLPARSE_ASSERT_STRING("waypoint",type);
    XMLPARSE_ASSERT_STRING("waypoint",string);

    if(!strcmp(type,"start"))
       translate_gpx_start=strcpy(malloc(strlen(string)+1),string);
    else if(!strcmp(type,"inter"))
       translate_gpx_inter=strcpy(malloc(strlen(string)+1),string);
    else if(!strcmp(type,"trip"))
       translate_gpx_trip=strcpy(malloc(strlen(string)+1),string);
    else if(!strcmp(type,"finish"))
       translate_gpx_finish=strcpy(malloc(strlen(string)+1),string);
    else
       XMLPARSE_INVALID("waypoint",type);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXDescType XSD type is seen

  int gpx_desc_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int gpx_desc_function(int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    XMLPARSE_ASSERT_STRING("desc",text);

    translate_gpx_desc=strcpy(malloc(strlen(text)+1),text);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXNameType XSD type is seen

  int gpx_name_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int gpx_name_function(int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    XMLPARSE_ASSERT_STRING("name",text);

    translate_gpx_name=strcpy(malloc(strlen(text)+1),text);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXStepType XSD type is seen

  int gpx_step_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int gpx_step_function(int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    XMLPARSE_ASSERT_STRING("step",text);

    translate_gpx_step=strcpy(malloc(strlen(text)+1),text);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXFinalType XSD type is seen

  int gpx_final_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int gpx_final_function(int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    XMLPARSE_ASSERT_STRING("final",text);

    translate_gpx_final=strcpy(malloc(strlen(text)+1),text);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXType XSD type is seen

  int output_gpx_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int output_gpx_function(int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the languageType XSD type is seen

  int language_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *lang The contents of the 'lang' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int language_function(int _type_,const char *lang)
{
 static int first=1;

 if(_type_&XMLPARSE_TAG_START)
   {
    XMLPARSE_ASSERT_STRING("language",lang);

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

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the RoutinoTranslationsType XSD type is seen

  int routino_translations_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int routino_translations_function(int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the xmlType XSD type is seen

  int xml_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *version The contents of the 'version' attribute (or NULL if not defined).

  const char *encoding The contents of the 'encoding' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

//static int xml_function(int _type_,const char *version,const char *encoding)
//{
// return(0);
//}


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

 retval=ParseXML(file,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_ERRNONAME);

 fclose(file);

 if(retval)
    return(1);

 if(language && !stored)
    fprintf(stderr,"Warning: Cannot find translations for language '%s' using English instead.\n",language);

 return(0);
}
