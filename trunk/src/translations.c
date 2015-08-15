/***************************************
 Load the translations from a file and the functions for handling them.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010-2015 Andrew M. Bishop

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

#include "files.h"
#include "translations.h"
#include "xmlparse.h"


/* Default English translations - Must not require any UTF-8 encoding */

static Translation default_translation=
{
 .language = "--",

 .raw_copyright_creator = {"Creator","Routino - http://www.routino.org/"},
 .raw_copyright_source  = {NULL,NULL},
 .raw_copyright_license = {NULL,NULL},

 .xml_copyright_creator = {"Creator","Routino - http://www.routino.org/"},
 .xml_copyright_source  = {NULL,NULL},
 .xml_copyright_license = {NULL,NULL},

 .xml_heading = {"South","South-West","West","North-West","North","North-East","East","South-East","South"},
 .xml_turn    = {"Very sharp left","Sharp left","Left","Slight left","Straight on","Slight right","Right","Sharp right","Very sharp right"},
 .xml_ordinal = {"First","Second","Third","Fourth","Fifth","Sixth","Seventh","Eighth","Ninth","Tenth"},

 .notxml_heading = {"South","South-West","West","North-West","North","North-East","East","South-East","South"},
 .notxml_turn    = {"Very sharp left","Sharp left","Left","Slight left","Straight on","Slight right","Right","Sharp right","Very sharp right"},
 .notxml_ordinal = {"First","Second","Third","Fourth","Fifth","Sixth","Seventh","Eighth","Ninth","Tenth"},

 .raw_highway = {"","motorway","trunk road","primary road","secondary road","tertiary road","unclassified road","residential road","service road","track","cycleway","path","steps","ferry"},

 .xml_route_shortest = "Shortest",
 .xml_route_quickest = "Quickest",

 .html_waypoint   = "<span class='w'>Waypoint</span>", /* span tag added when reading XML translations file */
 .html_junction   = "Junction",
 .html_roundabout = "Roundabout",

 .html_title   = "%s Route",
 .html_start   = "<tr class='n'><td class='l'>Start<td class='r'>at %s, head <span class='b'>%s</span>\n", /* span tags added when reading XML translations file */
 .html_node    = "<tr class='n'><td class='l'>At<td class='r'>%s, go <span class='t'>%s</span> heading <span class='b'>%s</span>\n", /* span tags added when reading XML translations file */
 .html_rbnode  = "<tr class='n'><td class='l'>Leave<td class='r'>%s, take the <span class='b'>%s</span> exit heading <span class='b'>%s</span>\n", /* span tags added when reading XML translations file */
 .html_segment = "<tr class='s'><td class='l'>Follow<td class='r'><span class='h'>%s</span> for <span class='d'>%.3f km, %.1f min</span>", /* span tags added when reading XML translations file */
 .html_stop    = "<tr class='n'><td class='l'>Stop<td class='r'>at %s\n",
 .html_total   = "<tr class='t'><td class='l'>Total<td class='r'><span class='j'>%.1f km, %.0f minutes</span>\n",/* span tags added when reading XML translations file */
 .html_subtotal= "<span class='j'>%.1f km, %.0f minutes</span>\n",/* span tag added when reading XML translations file */

 .nothtml_waypoint   = "Waypoint",
 .nothtml_junction   = "Junction",
 .nothtml_roundabout = "Roundabout",

 .nothtml_title   = "%s Route",
 .nothtml_start   = "Start at %s, head %s",
 .nothtml_node    = "At %s, go %s heading %s",
 .nothtml_rbnode  = "Leave %s, take the %s exit heading %s",
 .nothtml_segment = "Follow %s for %.3f km, %.1f min",
 .nothtml_stop    = "Stop at %s",
 .nothtml_total   = "Total %.1f km, %.0f minutes",
 .nothtml_subtotal= "%.1f km, %.0f minutes",

 .gpx_desc  = "%s route between 'start' and 'finish' waypoints",
 .gpx_name  = "%s route",
 .gpx_step  = "%s on '%s' for %.3f km, %.1f min",
 .gpx_final = "Total Journey %.1f km, %.0f minutes",

 .gpx_start  = "START",
 .gpx_inter  = "INTER",
 .gpx_trip   = "TRIP",
 .gpx_finish = "FINISH"
};


/* Local variables (re-intialised by FreeXMLTranslations() function) */

/*+ The translations that have been loaded from file. +*/
static Translation **loaded_translations=NULL;

/*+ The number of translations that have been loaded from file. +*/
static int nloaded_translations=0;


/* Local variables (re-initialised for each file) */

/*+ Store all of the translations. +*/
static int store_all;

/*+ The translation language that is to be stored. +*/
static const char *store_lang;

/*+ This current language is to be stored. +*/
static int store;

/*+ The chosen language has been stored. +*/
static int stored;


/* The XML tag processing function prototypes */

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding);
//static int RoutinoTranslationsType_function(const char *_tag_,int _type_);
static int LanguageType_function(const char *_tag_,int _type_,const char *lang);
//static int CopyrightType_function(const char *_tag_,int _type_);
static int TurnType_function(const char *_tag_,int _type_,const char *direction,const char *string);
static int HeadingType_function(const char *_tag_,int _type_,const char *direction,const char *string);
static int OrdinalType_function(const char *_tag_,int _type_,const char *number,const char *string);
static int HighwayType_function(const char *_tag_,int _type_,const char *type,const char *string);
static int RouteType_function(const char *_tag_,int _type_,const char *type,const char *string);
//static int HTMLType_function(const char *_tag_,int _type_);
//static int GPXType_function(const char *_tag_,int _type_);
static int CopyrightCreatorType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int CopyrightSourceType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int CopyrightLicenseType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int HTMLWaypointType_function(const char *_tag_,int _type_,const char *type,const char *string);
static int HTMLTitleType_function(const char *_tag_,int _type_,const char *text);
static int HTMLStartType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int HTMLNodeType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int HTMLRBNodeType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int HTMLSegmentType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int HTMLStopType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int HTMLTotalType_function(const char *_tag_,int _type_,const char *string,const char *text);
static int GPXWaypointType_function(const char *_tag_,int _type_,const char *type,const char *string);
static int GPXDescType_function(const char *_tag_,int _type_,const char *text);
static int GPXNameType_function(const char *_tag_,int _type_,const char *text);
static int GPXStepType_function(const char *_tag_,int _type_,const char *text);
static int GPXFinalType_function(const char *_tag_,int _type_,const char *text);


/* The XML tag definitions (forward declarations) */

static const xmltag xmlDeclaration_tag;
static const xmltag RoutinoTranslationsType_tag;
static const xmltag LanguageType_tag;
static const xmltag CopyrightType_tag;
static const xmltag TurnType_tag;
static const xmltag HeadingType_tag;
static const xmltag OrdinalType_tag;
static const xmltag HighwayType_tag;
static const xmltag RouteType_tag;
static const xmltag HTMLType_tag;
static const xmltag GPXType_tag;
static const xmltag CopyrightCreatorType_tag;
static const xmltag CopyrightSourceType_tag;
static const xmltag CopyrightLicenseType_tag;
static const xmltag HTMLWaypointType_tag;
static const xmltag HTMLTitleType_tag;
static const xmltag HTMLStartType_tag;
static const xmltag HTMLNodeType_tag;
static const xmltag HTMLRBNodeType_tag;
static const xmltag HTMLSegmentType_tag;
static const xmltag HTMLStopType_tag;
static const xmltag HTMLTotalType_tag;
static const xmltag GPXWaypointType_tag;
static const xmltag GPXDescType_tag;
static const xmltag GPXNameType_tag;
static const xmltag GPXStepType_tag;
static const xmltag GPXFinalType_tag;


/* The XML tag definition values */

/*+ The complete set of tags at the top level. +*/
static const xmltag * const xml_toplevel_tags[]={&xmlDeclaration_tag,&RoutinoTranslationsType_tag,NULL};

/*+ The xmlDeclaration type tag. +*/
static const xmltag xmlDeclaration_tag=
              {"xml",
               2, {"version","encoding"},
               NULL,
               {NULL}};

/*+ The RoutinoTranslationsType type tag. +*/
static const xmltag RoutinoTranslationsType_tag=
              {"routino-translations",
               0, {NULL},
               NULL,
               {&LanguageType_tag,NULL}};

/*+ The LanguageType type tag. +*/
static const xmltag LanguageType_tag=
              {"language",
               1, {"lang"},
               LanguageType_function,
               {&CopyrightType_tag,&TurnType_tag,&HeadingType_tag,&OrdinalType_tag,&HighwayType_tag,&RouteType_tag,&HTMLType_tag,&GPXType_tag,NULL}};

/*+ The CopyrightType type tag. +*/
static const xmltag CopyrightType_tag=
              {"copyright",
               0, {NULL},
               NULL,
               {&CopyrightCreatorType_tag,&CopyrightSourceType_tag,&CopyrightLicenseType_tag,NULL}};

/*+ The TurnType type tag. +*/
static const xmltag TurnType_tag=
              {"turn",
               2, {"direction","string"},
               TurnType_function,
               {NULL}};

/*+ The HeadingType type tag. +*/
static const xmltag HeadingType_tag=
              {"heading",
               2, {"direction","string"},
               HeadingType_function,
               {NULL}};

/*+ The OrdinalType type tag. +*/
static const xmltag OrdinalType_tag=
              {"ordinal",
               2, {"number","string"},
               OrdinalType_function,
               {NULL}};

/*+ The HighwayType type tag. +*/
static const xmltag HighwayType_tag=
              {"highway",
               2, {"type","string"},
               HighwayType_function,
               {NULL}};

/*+ The RouteType type tag. +*/
static const xmltag RouteType_tag=
              {"route",
               2, {"type","string"},
               RouteType_function,
               {NULL}};

/*+ The HTMLType type tag. +*/
static const xmltag HTMLType_tag=
              {"output-html",
               0, {NULL},
               NULL,
               {&HTMLWaypointType_tag,&HTMLTitleType_tag,&HTMLStartType_tag,&HTMLNodeType_tag,&HTMLRBNodeType_tag,&HTMLSegmentType_tag,&HTMLStopType_tag,&HTMLTotalType_tag,NULL}};

/*+ The GPXType type tag. +*/
static const xmltag GPXType_tag=
              {"output-gpx",
               0, {NULL},
               NULL,
               {&GPXWaypointType_tag,&GPXDescType_tag,&GPXNameType_tag,&GPXStepType_tag,&GPXFinalType_tag,NULL}};

/*+ The CopyrightCreatorType type tag. +*/
static const xmltag CopyrightCreatorType_tag=
              {"creator",
               2, {"string","text"},
               CopyrightCreatorType_function,
               {NULL}};

/*+ The CopyrightSourceType type tag. +*/
static const xmltag CopyrightSourceType_tag=
              {"source",
               2, {"string","text"},
               CopyrightSourceType_function,
               {NULL}};

/*+ The CopyrightLicenseType type tag. +*/
static const xmltag CopyrightLicenseType_tag=
              {"license",
               2, {"string","text"},
               CopyrightLicenseType_function,
               {NULL}};

/*+ The HTMLWaypointType type tag. +*/
static const xmltag HTMLWaypointType_tag=
              {"waypoint",
               2, {"type","string"},
               HTMLWaypointType_function,
               {NULL}};

/*+ The HTMLTitleType type tag. +*/
static const xmltag HTMLTitleType_tag=
              {"title",
               1, {"text"},
               HTMLTitleType_function,
               {NULL}};

/*+ The HTMLStartType type tag. +*/
static const xmltag HTMLStartType_tag=
              {"start",
               2, {"string","text"},
               HTMLStartType_function,
               {NULL}};

/*+ The HTMLNodeType type tag. +*/
static const xmltag HTMLNodeType_tag=
              {"node",
               2, {"string","text"},
               HTMLNodeType_function,
               {NULL}};

/*+ The HTMLRBNodeType type tag. +*/
static const xmltag HTMLRBNodeType_tag=
              {"rbnode",
               2, {"string","text"},
               HTMLRBNodeType_function,
               {NULL}};

/*+ The HTMLSegmentType type tag. +*/
static const xmltag HTMLSegmentType_tag=
              {"segment",
               2, {"string","text"},
               HTMLSegmentType_function,
               {NULL}};

/*+ The HTMLStopType type tag. +*/
static const xmltag HTMLStopType_tag=
              {"stop",
               2, {"string","text"},
               HTMLStopType_function,
               {NULL}};

/*+ The HTMLTotalType type tag. +*/
static const xmltag HTMLTotalType_tag=
              {"total",
               2, {"string","text"},
               HTMLTotalType_function,
               {NULL}};

/*+ The GPXWaypointType type tag. +*/
static const xmltag GPXWaypointType_tag=
              {"waypoint",
               2, {"type","string"},
               GPXWaypointType_function,
               {NULL}};

/*+ The GPXDescType type tag. +*/
static const xmltag GPXDescType_tag=
              {"desc",
               1, {"text"},
               GPXDescType_function,
               {NULL}};

/*+ The GPXNameType type tag. +*/
static const xmltag GPXNameType_tag=
              {"name",
               1, {"text"},
               GPXNameType_function,
               {NULL}};

/*+ The GPXStepType type tag. +*/
static const xmltag GPXStepType_tag=
              {"step",
               1, {"text"},
               GPXStepType_function,
               {NULL}};

/*+ The GPXFinalType type tag. +*/
static const xmltag GPXFinalType_tag=
              {"final",
               1, {"text"},
               GPXFinalType_function,
               {NULL}};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the XML declaration is seen

  int xmlDeclaration_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *version The contents of the 'version' attribute (or NULL if not defined).

  const char *encoding The contents of the 'encoding' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the RoutinoTranslationsType XSD type is seen

  int RoutinoTranslationsType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int RoutinoTranslationsType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the LanguageType XSD type is seen

  int LanguageType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *lang The contents of the 'lang' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int LanguageType_function(const char *_tag_,int _type_,const char *lang)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    XMLPARSE_ASSERT_STRING(_tag_,lang);

    if(store_all)
       store=1;
    else if(!store_lang && !stored)
       store=1;
    else if(store_lang && !strcmp(store_lang,lang))
       store=1;
    else
       store=0;

    if(store)
      {
       int i;

       for(i=0;i<nloaded_translations;i++)
          if(!strcmp(lang,loaded_translations[i]->language))
             XMLPARSE_MESSAGE(_tag_,"translation name must be unique");

       if((nloaded_translations%16)==0)
          loaded_translations=(Translation**)realloc((void*)loaded_translations,(nloaded_translations+16)*sizeof(Translation*));

       nloaded_translations++;

       loaded_translations[nloaded_translations-1]=(Translation*)calloc(1,sizeof(Translation));

       *loaded_translations[nloaded_translations-1]=default_translation;

       loaded_translations[nloaded_translations-1]->language=strcpy(malloc(strlen(lang)+1),lang);
      }
   }

 if(_type_&XMLPARSE_TAG_END && store)
   {
    store=0;
    stored=1;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the CopyrightType XSD type is seen

  int CopyrightType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int CopyrightType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the TurnType XSD type is seen

  int TurnType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *direction The contents of the 'direction' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int TurnType_function(const char *_tag_,int _type_,const char *direction,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring;
    int d;

    XMLPARSE_ASSERT_INTEGER(_tag_,direction); d=atoi(direction);
    XMLPARSE_ASSERT_STRING(_tag_,string);

    d+=4;

    if(d<0 || d>8)
       XMLPARSE_INVALID(_tag_,direction);

    loaded_translations[nloaded_translations-1]->notxml_turn[d]=strcpy(malloc(strlen(string)+1),string);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->xml_turn[d]=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HeadingType XSD type is seen

  int HeadingType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *direction The contents of the 'direction' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HeadingType_function(const char *_tag_,int _type_,const char *direction,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring;
    int d;

    XMLPARSE_ASSERT_INTEGER(_tag_,direction); d=atoi(direction);
    XMLPARSE_ASSERT_STRING(_tag_,string);

    d+=4;

    if(d<0 || d>8)
       XMLPARSE_INVALID(_tag_,direction);

    loaded_translations[nloaded_translations-1]->notxml_heading[d]=strcpy(malloc(strlen(string)+1),string);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->xml_heading[d]=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the OrdinalType XSD type is seen

  int OrdinalType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *number The contents of the 'number' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int OrdinalType_function(const char *_tag_,int _type_,const char *number,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring;
    int n;

    XMLPARSE_ASSERT_INTEGER(_tag_,number); n=atoi(number);
    XMLPARSE_ASSERT_STRING(_tag_,string);

    if(n<1 || n>10)
       XMLPARSE_INVALID(_tag_,number);

    loaded_translations[nloaded_translations-1]->notxml_ordinal[n-1]=strcpy(malloc(strlen(string)+1),string);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->xml_ordinal[n-1]=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HighwayType XSD type is seen

  int HighwayType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HighwayType_function(const char *_tag_,int _type_,const char *type,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    Highway highway;

    XMLPARSE_ASSERT_STRING(_tag_,type);
    XMLPARSE_ASSERT_STRING(_tag_,string);

    highway=HighwayType(type);

    if(highway==Highway_None)
       XMLPARSE_INVALID(_tag_,type);

    loaded_translations[nloaded_translations-1]->raw_highway[highway]=strcpy(malloc(strlen(string)+1),string);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the RouteType XSD type is seen

  int RouteType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int RouteType_function(const char *_tag_,int _type_,const char *type,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring;

    XMLPARSE_ASSERT_STRING(_tag_,type);
    XMLPARSE_ASSERT_STRING(_tag_,string);

    xmlstring=ParseXML_Encode_Safe_XML(string);

    if(!strcmp(type,"shortest"))
       loaded_translations[nloaded_translations-1]->xml_route_shortest=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
    else if(!strcmp(type,"quickest"))
       loaded_translations[nloaded_translations-1]->xml_route_quickest=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
    else
       XMLPARSE_INVALID(_tag_,type);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLType XSD type is seen

  int HTMLType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int HTMLType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXType XSD type is seen

  int GPXType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int GPXType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the CopyrightCreatorType XSD type is seen

  int CopyrightCreatorType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int CopyrightCreatorType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->raw_copyright_creator[0]=strcpy(malloc(strlen(string)+1),string);
    loaded_translations[nloaded_translations-1]->raw_copyright_creator[1]=strcpy(malloc(strlen(text)+1)  ,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->xml_copyright_creator[0]=strcpy(malloc(strlen(xmlstring)+1),xmlstring);

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->xml_copyright_creator[1]=strcpy(malloc(strlen(xmltext)+1),xmltext);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the CopyrightSourceType XSD type is seen

  int CopyrightSourceType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int CopyrightSourceType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->raw_copyright_source[0]=strcpy(malloc(strlen(string)+1),string);
    loaded_translations[nloaded_translations-1]->raw_copyright_source[1]=strcpy(malloc(strlen(text)+1)  ,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->xml_copyright_source[0]=strcpy(malloc(strlen(xmlstring)+1),xmlstring);

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->xml_copyright_source[1]=strcpy(malloc(strlen(xmltext)+1),xmltext);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the CopyrightLicenseType XSD type is seen

  int CopyrightLicenseType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int CopyrightLicenseType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->raw_copyright_license[0]=strcpy(malloc(strlen(string)+1),string);
    loaded_translations[nloaded_translations-1]->raw_copyright_license[1]=strcpy(malloc(strlen(text)+1)  ,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->xml_copyright_license[0]=strcpy(malloc(strlen(xmlstring)+1),xmlstring);

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->xml_copyright_license[1]=strcpy(malloc(strlen(xmltext)+1),xmltext);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLWaypointType XSD type is seen

  int HTMLWaypointType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HTMLWaypointType_function(const char *_tag_,int _type_,const char *type,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring;

    XMLPARSE_ASSERT_STRING(_tag_,type);
    XMLPARSE_ASSERT_STRING(_tag_,string);

    xmlstring=ParseXML_Encode_Safe_XML(string);

    if(!strcmp(type,"waypoint"))
      {
       loaded_translations[nloaded_translations-1]->nothtml_waypoint=strcpy(malloc(strlen(string)+1),string);

       loaded_translations[nloaded_translations-1]->html_waypoint=malloc(strlen(xmlstring)+1+sizeof("<span class='w'>")+sizeof("</span>"));
       sprintf(loaded_translations[nloaded_translations-1]->html_waypoint,"<span class='w'>%s</span>",xmlstring);
      }
    else if(!strcmp(type,"junction"))
      {
       loaded_translations[nloaded_translations-1]->nothtml_junction=strcpy(malloc(strlen(string)+1),string);

       loaded_translations[nloaded_translations-1]->html_junction=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
      }
    else if(!strcmp(type,"roundabout"))
      {
       loaded_translations[nloaded_translations-1]->nothtml_roundabout=strcpy(malloc(strlen(string)+1),string);

       loaded_translations[nloaded_translations-1]->html_roundabout=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
      }
    else
       XMLPARSE_INVALID(_tag_,type);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLTitleType XSD type is seen

  int HTMLTitleType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HTMLTitleType_function(const char *_tag_,int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->nothtml_title=strcpy(malloc(strlen(text)+1),text);

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->html_title=strcpy(malloc(strlen(xmltext)+1),xmltext);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLStartType XSD type is seen

  int HTMLStartType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HTMLStartType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;
    const char *p;
    char *q;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->nothtml_start=malloc(strlen(string)+1+strlen(text)+1);
    strcpy(loaded_translations[nloaded_translations-1]->nothtml_start,string);
    strcat(loaded_translations[nloaded_translations-1]->nothtml_start," ");
    strcat(loaded_translations[nloaded_translations-1]->nothtml_start,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->html_start=malloc(sizeof("<tr class='n'><td class='l'>")+strlen(xmlstring)+sizeof(":<td class='r'>")+1);
    strcpy(loaded_translations[nloaded_translations-1]->html_start,"<tr class='n'><td class='l'>");
    strcat(loaded_translations[nloaded_translations-1]->html_start,xmlstring);
    strcat(loaded_translations[nloaded_translations-1]->html_start,":<td class='r'>");

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->html_start=realloc(loaded_translations[nloaded_translations-1]->html_start,
                                                                    strlen(loaded_translations[nloaded_translations-1]->html_start)+
                                                                    strlen(xmltext)+sizeof("<span class='b'>")+sizeof("</span>")+1+1);

    p=xmltext;
    q=loaded_translations[nloaded_translations-1]->html_start+strlen(loaded_translations[nloaded_translations-1]->html_start);

    while(*p!='%')
       *q++=*p++;

    *q++=*p++;

    while(*p!='%')
       *q++=*p++;

    p+=2;
    strcpy(q,"<span class='b'>%s</span>"); q+=sizeof("<span class='b'>%s</span>")-1;

    strcpy(q,p);
    strcat(q,"\n");
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLNodeType XSD type is seen

  int HTMLNodeType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HTMLNodeType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;
    const char *p;
    char *q;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->nothtml_node=malloc(strlen(string)+1+strlen(text)+1);
    strcpy(loaded_translations[nloaded_translations-1]->nothtml_node,string);
    strcat(loaded_translations[nloaded_translations-1]->nothtml_node," ");
    strcat(loaded_translations[nloaded_translations-1]->nothtml_node,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->html_node=malloc(sizeof("<tr class='n'><td class='l'>")+strlen(xmlstring)+sizeof(":<td class='r'>")+1);
    strcpy(loaded_translations[nloaded_translations-1]->html_node,"<tr class='n'><td class='l'>");
    strcat(loaded_translations[nloaded_translations-1]->html_node,xmlstring);
    strcat(loaded_translations[nloaded_translations-1]->html_node,":<td class='r'>");

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->html_node=realloc(loaded_translations[nloaded_translations-1]->html_node,
                                                                   strlen(loaded_translations[nloaded_translations-1]->html_node)+
                                                                   strlen(xmltext)+2*sizeof("<span class='b'>")+2*sizeof("</span>")+1+1);

    p=xmltext;
    q=loaded_translations[nloaded_translations-1]->html_node+strlen(loaded_translations[nloaded_translations-1]->html_node);

    while(*p!='%')
       *q++=*p++;

    *q++=*p++;

    while(*p!='%')
       *q++=*p++;

    p+=2;
    strcpy(q,"<span class='t'>%s</span>"); q+=sizeof("<span class='t'>%s</span>")-1;

    while(*p!='%')
       *q++=*p++;

    p+=2;
    strcpy(q,"<span class='b'>%s</span>"); q+=sizeof("<span class='b'>%s</span>")-1;

    strcpy(q,p);
    strcat(q,"\n");
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLRBNodeType XSD type is seen

  int HTMLRBNodeType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HTMLRBNodeType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;
    const char *p;
    char *q;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->nothtml_rbnode=malloc(strlen(string)+1+strlen(text)+1);
    strcpy(loaded_translations[nloaded_translations-1]->nothtml_rbnode,string);
    strcat(loaded_translations[nloaded_translations-1]->nothtml_rbnode," ");
    strcat(loaded_translations[nloaded_translations-1]->nothtml_rbnode,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->html_rbnode=malloc(sizeof("<tr class='n'><td class='l'>")+strlen(xmlstring)+sizeof(":<td class='r'>")+1);
    strcpy(loaded_translations[nloaded_translations-1]->html_rbnode,"<tr class='n'><td class='l'>");
    strcat(loaded_translations[nloaded_translations-1]->html_rbnode,xmlstring);
    strcat(loaded_translations[nloaded_translations-1]->html_rbnode,":<td class='r'>");

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->html_rbnode=realloc(loaded_translations[nloaded_translations-1]->html_rbnode,
                                                                     strlen(loaded_translations[nloaded_translations-1]->html_rbnode)+
                                                                     strlen(xmltext)+2*sizeof("<span class='b'>")+2*sizeof("</span>")+1+1);

    p=xmltext;
    q=loaded_translations[nloaded_translations-1]->html_rbnode+strlen(loaded_translations[nloaded_translations-1]->html_rbnode);

    while(*p!='%')
       *q++=*p++;

    *q++=*p++;

    while(*p!='%')
       *q++=*p++;

    p+=2;
    strcpy(q,"<span class='t'>%s</span>"); q+=sizeof("<span class='t'>%s</span>")-1;

    while(*p!='%')
       *q++=*p++;

    p+=2;
    strcpy(q,"<span class='b'>%s</span>"); q+=sizeof("<span class='b'>%s</span>")-1;

    strcpy(q,p);
    strcat(q,"\n");
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLSegmentType XSD type is seen

  int HTMLSegmentType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HTMLSegmentType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;
    const char *p;
    char *q;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->nothtml_segment=malloc(strlen(string)+1+strlen(text)+1);
    strcpy(loaded_translations[nloaded_translations-1]->nothtml_segment,string);
    strcat(loaded_translations[nloaded_translations-1]->nothtml_segment," ");
    strcat(loaded_translations[nloaded_translations-1]->nothtml_segment,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->html_segment=malloc(sizeof("<tr class='s'><td class='l'>")+strlen(xmlstring)+sizeof(":<td class='r'>")+1);
    strcpy(loaded_translations[nloaded_translations-1]->html_segment,"<tr class='s'><td class='l'>");
    strcat(loaded_translations[nloaded_translations-1]->html_segment,xmlstring);
    strcat(loaded_translations[nloaded_translations-1]->html_segment,":<td class='r'>");

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->html_segment=realloc(loaded_translations[nloaded_translations-1]->html_segment,
                                                                      strlen(loaded_translations[nloaded_translations-1]->html_segment)+
                                                                      strlen(xmltext)+2*sizeof("<span class='b'>")+2*sizeof("</span>")+1);

    p=xmltext;
    q=loaded_translations[nloaded_translations-1]->html_segment+strlen(loaded_translations[nloaded_translations-1]->html_segment);

    while(*p!='%')
       *q++=*p++;

    p+=2;
    strcpy(q,"<span class='h'>%s</span>"); q+=sizeof("<span class='h'>%s</span>")-1;

    while(*p!='%')
       *q++=*p++;

    strcpy(q,"<span class='d'>"); q+=sizeof("<span class='d'>")-1;

    strcpy(q,p);
    strcat(q,"</span>");
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLStopType XSD type is seen

  int HTMLStopType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HTMLStopType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->nothtml_stop=malloc(strlen(string)+1+strlen(text)+1);
    strcpy(loaded_translations[nloaded_translations-1]->nothtml_stop,string);
    strcat(loaded_translations[nloaded_translations-1]->nothtml_stop," ");
    strcat(loaded_translations[nloaded_translations-1]->nothtml_stop,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->html_stop=malloc(sizeof("<tr class='n'><td class='l'>")+strlen(xmlstring)+sizeof(":<td class='r'>")+1);
    strcpy(loaded_translations[nloaded_translations-1]->html_stop,"<tr class='n'><td class='l'>");
    strcat(loaded_translations[nloaded_translations-1]->html_stop,xmlstring);
    strcat(loaded_translations[nloaded_translations-1]->html_stop,":<td class='r'>");

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->html_stop=realloc(loaded_translations[nloaded_translations-1]->html_stop,
                                                                   strlen(loaded_translations[nloaded_translations-1]->html_stop)+
                                                                   strlen(xmltext)+1+1);

    strcat(loaded_translations[nloaded_translations-1]->html_stop,xmltext);
    strcat(loaded_translations[nloaded_translations-1]->html_stop,"\n");
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the HTMLTotalType XSD type is seen

  int HTMLTotalType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *string The contents of the 'string' attribute (or NULL if not defined).

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int HTMLTotalType_function(const char *_tag_,int _type_,const char *string,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring,*xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,string);
    XMLPARSE_ASSERT_STRING(_tag_,text);

    loaded_translations[nloaded_translations-1]->nothtml_total=malloc(strlen(string)+1+strlen(text)+1);
    strcpy(loaded_translations[nloaded_translations-1]->nothtml_total,string);
    strcat(loaded_translations[nloaded_translations-1]->nothtml_total," ");
    strcat(loaded_translations[nloaded_translations-1]->nothtml_total,text);

    xmlstring=ParseXML_Encode_Safe_XML(string);
    loaded_translations[nloaded_translations-1]->html_total=malloc(sizeof("<tr class='t'><td class='l'>")+strlen(xmlstring)+sizeof(":<td class='r'>")+1);
    strcpy(loaded_translations[nloaded_translations-1]->html_total,"<tr class='t'><td class='l'>");
    strcat(loaded_translations[nloaded_translations-1]->html_total,xmlstring);
    strcat(loaded_translations[nloaded_translations-1]->html_total,":<td class='r'>");

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->html_total=realloc(loaded_translations[nloaded_translations-1]->html_total,
                                                                   strlen(loaded_translations[nloaded_translations-1]->html_total)+
                                                                   sizeof("<span class='j'>")+strlen(xmltext)+sizeof("</span>")+1+1);

    strcat(loaded_translations[nloaded_translations-1]->html_total,"<span class='j'>");
    strcat(loaded_translations[nloaded_translations-1]->html_total,xmltext);
    strcat(loaded_translations[nloaded_translations-1]->html_total,"</span>");
    strcat(loaded_translations[nloaded_translations-1]->html_total,"\n");


    loaded_translations[nloaded_translations-1]->nothtml_subtotal=strcpy(malloc(strlen(text)+1),text);

    loaded_translations[nloaded_translations-1]->html_subtotal=malloc(sizeof(" [<span class='j'>")+strlen(xmltext)+sizeof("</span>]")+1+1);

    strcpy(loaded_translations[nloaded_translations-1]->html_subtotal," [<span class='j'>");
    strcat(loaded_translations[nloaded_translations-1]->html_subtotal,xmltext);
    strcat(loaded_translations[nloaded_translations-1]->html_subtotal,"</span>]");
    strcat(loaded_translations[nloaded_translations-1]->html_subtotal,"\n");
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXWaypointType XSD type is seen

  int GPXWaypointType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *string The contents of the 'string' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int GPXWaypointType_function(const char *_tag_,int _type_,const char *type,const char *string)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmlstring;

    XMLPARSE_ASSERT_STRING(_tag_,type);
    XMLPARSE_ASSERT_STRING(_tag_,string);

    xmlstring=ParseXML_Encode_Safe_XML(string);

    if(!strcmp(type,"start"))
       loaded_translations[nloaded_translations-1]->gpx_start=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
    else if(!strcmp(type,"inter"))
       loaded_translations[nloaded_translations-1]->gpx_inter=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
    else if(!strcmp(type,"trip"))
       loaded_translations[nloaded_translations-1]->gpx_trip=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
    else if(!strcmp(type,"finish"))
       loaded_translations[nloaded_translations-1]->gpx_finish=strcpy(malloc(strlen(xmlstring)+1),xmlstring);
    else
       XMLPARSE_INVALID(_tag_,type);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXDescType XSD type is seen

  int GPXDescType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int GPXDescType_function(const char *_tag_,int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,text);

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->gpx_desc=strcpy(malloc(strlen(xmltext)+1),xmltext);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXNameType XSD type is seen

  int GPXNameType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int GPXNameType_function(const char *_tag_,int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,text);

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->gpx_name=strcpy(malloc(strlen(xmltext)+1),xmltext);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXStepType XSD type is seen

  int GPXStepType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int GPXStepType_function(const char *_tag_,int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,text);

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->gpx_step=strcpy(malloc(strlen(xmltext)+1),xmltext);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the GPXFinalType XSD type is seen

  int GPXFinalType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *text The contents of the 'text' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int GPXFinalType_function(const char *_tag_,int _type_,const char *text)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    char *xmltext;

    XMLPARSE_ASSERT_STRING(_tag_,text);

    xmltext=ParseXML_Encode_Safe_XML(text);
    loaded_translations[nloaded_translations-1]->gpx_final=strcpy(malloc(strlen(xmltext)+1),xmltext);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The XML translation parser.

  int ParseXMLTranslations Returns 0 if OK or something else in case of an error.

  const char *filename The name of the file to read.

  const char *language The language to search for (NULL means first in file).

  int all Set to true to load all the translations.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXMLTranslations(const char *filename,const char *language,int all)
{
 int fd;
 int retval;

 if(!ExistsFile(filename))
    return(1);

 fd=OpenFile(filename);

 /* Delete the existing translations */

 if(nloaded_translations)
    FreeXMLTranslations();

 /* Initialise variables used for parsing */

 store_all=all;

 store_lang=language;

 store=0;
 stored=0;

 /* Parse the file */

 retval=ParseXML(fd,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_ERRNONAME|XMLPARSE_RETURN_ATTR_ENCODED);

 CloseFile(fd);

 if(retval)
   {
    FreeXMLTranslations();

    return(2);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Return a list of the languages that have been loaded from the XML file.

  char **GetTranslationLanguages Returns a NULL terminated list of strings - all allocated.
  ++++++++++++++++++++++++++++++++++++++*/

char **GetTranslationLanguages(void)
{
 char **list=calloc(1+nloaded_translations,sizeof(char*));
 int i;

 for(i=0;i<nloaded_translations;i++)
    list[i]=strcpy(malloc(strlen(loaded_translations[i]->language)+1),loaded_translations[i]->language);

 return(list);
}


/*++++++++++++++++++++++++++++++++++++++
  Get a named translation.

  Translation *GetTranslation Returns a pointer to the translation.

  const char *language The language of the translation or NULL to get the default or an empty string to get the first one.
  ++++++++++++++++++++++++++++++++++++++*/

Translation *GetTranslation(const char *language)
{
 int i;

 if(!language)
    return(&default_translation);

 if(!*language && nloaded_translations>0)
    return(loaded_translations[0]);

 for(i=0;i<nloaded_translations;i++)
    if(!strcmp(loaded_translations[i]->language,language))
       return(loaded_translations[i]);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Free the memory that has been allocated for the translations.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeXMLTranslations()
{
 int i,j;

 if(!loaded_translations)
    return;

 for(i=0;i<nloaded_translations;i++)
   {
    free(loaded_translations[i]->language);

    for(j=0;j<2;j++)
      {
       if(loaded_translations[i]->raw_copyright_creator[j] != default_translation.raw_copyright_creator[j]) free(loaded_translations[i]->raw_copyright_creator[j]);
       if(loaded_translations[i]->raw_copyright_source[j]  != default_translation.raw_copyright_source[j])  free(loaded_translations[i]->raw_copyright_source[j]);
       if(loaded_translations[i]->raw_copyright_license[j] != default_translation.raw_copyright_license[j]) free(loaded_translations[i]->raw_copyright_license[j]);

       if(loaded_translations[i]->xml_copyright_creator[j] != default_translation.xml_copyright_creator[j]) free(loaded_translations[i]->xml_copyright_creator[j]);
       if(loaded_translations[i]->xml_copyright_source[j]  != default_translation.xml_copyright_source[j])  free(loaded_translations[i]->xml_copyright_source[j]);
       if(loaded_translations[i]->xml_copyright_license[j] != default_translation.xml_copyright_license[j]) free(loaded_translations[i]->xml_copyright_license[j]);
      }

    for(j=0;j<9;j++)
      {
       if(loaded_translations[i]->xml_heading[j] != default_translation.xml_heading[j]) free(loaded_translations[i]->xml_heading[j]);
       if(loaded_translations[i]->xml_turn[j]    != default_translation.xml_turn[j])    free(loaded_translations[i]->xml_turn[j]);
      }

    for(j=0;j<10;j++)
       if(loaded_translations[i]->xml_ordinal[j] != default_translation.xml_ordinal[j]) free(loaded_translations[i]->xml_ordinal[j]);

    for(j=0;j<9;j++)
      {
       if(loaded_translations[i]->notxml_heading[j] != default_translation.notxml_heading[j]) free(loaded_translations[i]->notxml_heading[j]);
       if(loaded_translations[i]->notxml_turn[j]    != default_translation.notxml_turn[j])    free(loaded_translations[i]->notxml_turn[j]);
      }

    for(j=0;j<10;j++)
       if(loaded_translations[i]->notxml_ordinal[j] != default_translation.notxml_ordinal[j]) free(loaded_translations[i]->notxml_ordinal[j]);

    for(j=0;j<Highway_Count;j++)
       if(loaded_translations[i]->raw_highway[j] != default_translation.raw_highway[j]) free(loaded_translations[i]->raw_highway[j]);

    if(loaded_translations[i]->xml_route_shortest != default_translation.xml_route_shortest) free(loaded_translations[i]->xml_route_shortest);
    if(loaded_translations[i]->xml_route_quickest != default_translation.xml_route_quickest) free(loaded_translations[i]->xml_route_quickest);

    if(loaded_translations[i]->html_waypoint   != default_translation.html_waypoint)   free(loaded_translations[i]->html_waypoint);
    if(loaded_translations[i]->html_junction   != default_translation.html_junction)   free(loaded_translations[i]->html_junction);
    if(loaded_translations[i]->html_roundabout != default_translation.html_roundabout) free(loaded_translations[i]->html_roundabout);

    if(loaded_translations[i]->html_title != default_translation.html_title) free(loaded_translations[i]->html_title);

    if(loaded_translations[i]->html_start   != default_translation.html_start)   free(loaded_translations[i]->html_start);
    if(loaded_translations[i]->html_node    != default_translation.html_node)    free(loaded_translations[i]->html_node);
    if(loaded_translations[i]->html_rbnode  != default_translation.html_rbnode)  free(loaded_translations[i]->html_rbnode);
    if(loaded_translations[i]->html_segment != default_translation.html_segment) free(loaded_translations[i]->html_segment);
    if(loaded_translations[i]->html_stop    != default_translation.html_stop)    free(loaded_translations[i]->html_stop);
    if(loaded_translations[i]->html_total   != default_translation.html_total)   free(loaded_translations[i]->html_total);
    if(loaded_translations[i]->html_subtotal!= default_translation.html_subtotal)free(loaded_translations[i]->html_subtotal);

    if(loaded_translations[i]->nothtml_waypoint   != default_translation.nothtml_waypoint)   free(loaded_translations[i]->nothtml_waypoint);
    if(loaded_translations[i]->nothtml_junction   != default_translation.nothtml_junction)   free(loaded_translations[i]->nothtml_junction);
    if(loaded_translations[i]->nothtml_roundabout != default_translation.nothtml_roundabout) free(loaded_translations[i]->nothtml_roundabout);

    if(loaded_translations[i]->nothtml_title != default_translation.nothtml_title) free(loaded_translations[i]->nothtml_title);

    if(loaded_translations[i]->nothtml_start   != default_translation.nothtml_start)   free(loaded_translations[i]->nothtml_start);
    if(loaded_translations[i]->nothtml_node    != default_translation.nothtml_node)    free(loaded_translations[i]->nothtml_node);
    if(loaded_translations[i]->nothtml_rbnode  != default_translation.nothtml_rbnode)  free(loaded_translations[i]->nothtml_rbnode);
    if(loaded_translations[i]->nothtml_segment != default_translation.nothtml_segment) free(loaded_translations[i]->nothtml_segment);
    if(loaded_translations[i]->nothtml_stop    != default_translation.nothtml_stop)    free(loaded_translations[i]->nothtml_stop);
    if(loaded_translations[i]->nothtml_total   != default_translation.nothtml_total)   free(loaded_translations[i]->nothtml_total);
    if(loaded_translations[i]->nothtml_subtotal!= default_translation.nothtml_subtotal)free(loaded_translations[i]->nothtml_subtotal);

    if(loaded_translations[i]->gpx_desc  != default_translation.gpx_desc)  free(loaded_translations[i]->gpx_desc);
    if(loaded_translations[i]->gpx_name  != default_translation.gpx_name)  free(loaded_translations[i]->gpx_name);
    if(loaded_translations[i]->gpx_step  != default_translation.gpx_step)  free(loaded_translations[i]->gpx_step);
    if(loaded_translations[i]->gpx_final != default_translation.gpx_final) free(loaded_translations[i]->gpx_final);

    if(loaded_translations[i]->gpx_start  != default_translation.gpx_start)  free(loaded_translations[i]->gpx_start);
    if(loaded_translations[i]->gpx_inter  != default_translation.gpx_inter)  free(loaded_translations[i]->gpx_inter);
    if(loaded_translations[i]->gpx_trip   != default_translation.gpx_trip)   free(loaded_translations[i]->gpx_trip);
    if(loaded_translations[i]->gpx_finish != default_translation.gpx_finish) free(loaded_translations[i]->gpx_finish);

    free(loaded_translations[i]);
   }

 free(loaded_translations);

 loaded_translations=NULL;
 nloaded_translations=0;
}
