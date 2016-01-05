/***************************************
 Load the profiles from a file and the functions for handling them.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2015 Andrew M. Bishop

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
#include <math.h>

#include "types.h"
#include "ways.h"

#include "files.h"
#include "profiles.h"
#include "xmlparse.h"


/* Local variables (re-intialised by FreeXMLProfiles() function) */

/*+ The profiles that have been loaded from file. +*/
static Profile **loaded_profiles=NULL;

/*+ The number of profiles that have been loaded from file. +*/
static int nloaded_profiles=0;


/* Local variables (re-initialised for each file) */

/*+ Store all of the profiles. +*/
static int store_all;

/*+ The profile name that is to be stored. +*/
static const char *store_name;

/*+ This current profile is to be stored. +*/
static int store;


/* The XML tag processing function prototypes */

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding);
//static int RoutinoProfilesType_function(const char *_tag_,int _type_);
static int profileType_function(const char *_tag_,int _type_,const char *name,const char *transport);
//static int speedsType_function(const char *_tag_,int _type_);
//static int preferencesType_function(const char *_tag_,int _type_);
//static int propertiesType_function(const char *_tag_,int _type_);
//static int restrictionsType_function(const char *_tag_,int _type_);
static int speedType_function(const char *_tag_,int _type_,const char *highway,const char *kph);
static int preferenceType_function(const char *_tag_,int _type_,const char *highway,const char *percent);
static int propertyType_function(const char *_tag_,int _type_,const char *type,const char *percent);
static int onewayType_function(const char *_tag_,int _type_,const char *obey);
static int turnsType_function(const char *_tag_,int _type_,const char *obey);
static int weightType_function(const char *_tag_,int _type_,const char *limit);
static int heightType_function(const char *_tag_,int _type_,const char *limit);
static int widthType_function(const char *_tag_,int _type_,const char *limit);
static int lengthType_function(const char *_tag_,int _type_,const char *limit);


/* The XML tag definitions (forward declarations) */

static const xmltag xmlDeclaration_tag;
static const xmltag RoutinoProfilesType_tag;
static const xmltag profileType_tag;
static const xmltag speedsType_tag;
static const xmltag preferencesType_tag;
static const xmltag propertiesType_tag;
static const xmltag restrictionsType_tag;
static const xmltag speedType_tag;
static const xmltag preferenceType_tag;
static const xmltag propertyType_tag;
static const xmltag onewayType_tag;
static const xmltag turnsType_tag;
static const xmltag weightType_tag;
static const xmltag heightType_tag;
static const xmltag widthType_tag;
static const xmltag lengthType_tag;


/* The XML tag definition values */

/*+ The complete set of tags at the top level. +*/
static const xmltag * const xml_toplevel_tags[]={&xmlDeclaration_tag,&RoutinoProfilesType_tag,NULL};

/*+ The xmlDeclaration type tag. +*/
static const xmltag xmlDeclaration_tag=
              {"xml",
               2, {"version","encoding"},
               NULL,
               {NULL}};

/*+ The RoutinoProfilesType type tag. +*/
static const xmltag RoutinoProfilesType_tag=
              {"routino-profiles",
               0, {NULL},
               NULL,
               {&profileType_tag,NULL}};

/*+ The profileType type tag. +*/
static const xmltag profileType_tag=
              {"profile",
               2, {"name","transport"},
               profileType_function,
               {&speedsType_tag,&preferencesType_tag,&propertiesType_tag,&restrictionsType_tag,NULL}};

/*+ The speedsType type tag. +*/
static const xmltag speedsType_tag=
              {"speeds",
               0, {NULL},
               NULL,
               {&speedType_tag,NULL}};

/*+ The preferencesType type tag. +*/
static const xmltag preferencesType_tag=
              {"preferences",
               0, {NULL},
               NULL,
               {&preferenceType_tag,NULL}};

/*+ The propertiesType type tag. +*/
static const xmltag propertiesType_tag=
              {"properties",
               0, {NULL},
               NULL,
               {&propertyType_tag,NULL}};

/*+ The restrictionsType type tag. +*/
static const xmltag restrictionsType_tag=
              {"restrictions",
               0, {NULL},
               NULL,
               {&onewayType_tag,&turnsType_tag,&weightType_tag,&heightType_tag,&widthType_tag,&lengthType_tag,NULL}};

/*+ The speedType type tag. +*/
static const xmltag speedType_tag=
              {"speed",
               2, {"highway","kph"},
               speedType_function,
               {NULL}};

/*+ The preferenceType type tag. +*/
static const xmltag preferenceType_tag=
              {"preference",
               2, {"highway","percent"},
               preferenceType_function,
               {NULL}};

/*+ The propertyType type tag. +*/
static const xmltag propertyType_tag=
              {"property",
               2, {"type","percent"},
               propertyType_function,
               {NULL}};

/*+ The onewayType type tag. +*/
static const xmltag onewayType_tag=
              {"oneway",
               1, {"obey"},
               onewayType_function,
               {NULL}};

/*+ The turnsType type tag. +*/
static const xmltag turnsType_tag=
              {"turns",
               1, {"obey"},
               turnsType_function,
               {NULL}};

/*+ The weightType type tag. +*/
static const xmltag weightType_tag=
              {"weight",
               1, {"limit"},
               weightType_function,
               {NULL}};

/*+ The heightType type tag. +*/
static const xmltag heightType_tag=
              {"height",
               1, {"limit"},
               heightType_function,
               {NULL}};

/*+ The widthType type tag. +*/
static const xmltag widthType_tag=
              {"width",
               1, {"limit"},
               widthType_function,
               {NULL}};

/*+ The lengthType type tag. +*/
static const xmltag lengthType_tag=
              {"length",
               1, {"limit"},
               lengthType_function,
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
  The function that is called when the RoutinoProfilesType XSD type is seen

  int RoutinoProfilesType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int RoutinoProfilesType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the profileType XSD type is seen

  int profileType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *name The contents of the 'name' attribute (or NULL if not defined).

  const char *transport The contents of the 'transport' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int profileType_function(const char *_tag_,int _type_,const char *name,const char *transport)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    Transport transporttype;
    int i;

    XMLPARSE_ASSERT_STRING(_tag_,name);

    if(store_all)
       store=1;
    else if(store_name && !strcmp(store_name,name))
       store=1;
    else
       store=0;

    if(store)
      {
       for(i=0;i<nloaded_profiles;i++)
          if(!strcmp(name,loaded_profiles[i]->name))
             XMLPARSE_MESSAGE(_tag_,"profile name must be unique");

       XMLPARSE_ASSERT_STRING(_tag_,transport);

       transporttype=TransportType(transport);

       if(transporttype==Transport_None)
          XMLPARSE_INVALID(_tag_,transport);

       if((nloaded_profiles%16)==0)
          loaded_profiles=(Profile**)realloc((void*)loaded_profiles,(nloaded_profiles+16)*sizeof(Profile*));

       nloaded_profiles++;

       loaded_profiles[nloaded_profiles-1]=(Profile*)calloc(1,sizeof(Profile));

       loaded_profiles[nloaded_profiles-1]->name=strcpy(malloc(strlen(name)+1),name);
       loaded_profiles[nloaded_profiles-1]->transport=transporttype;
      }
   }

 if(_type_&XMLPARSE_TAG_END && store)
    store=0;

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the speedsType XSD type is seen

  int speedsType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int speedsType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the preferencesType XSD type is seen

  int preferencesType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int preferencesType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the propertiesType XSD type is seen

  int propertiesType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int propertiesType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the restrictionsType XSD type is seen

  int restrictionsType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int restrictionsType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the speedType XSD type is seen

  int speedType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *highway The contents of the 'highway' attribute (or NULL if not defined).

  const char *kph The contents of the 'kph' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int speedType_function(const char *_tag_,int _type_,const char *highway,const char *kph)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    double speed;
    Highway highwaytype;

    XMLPARSE_ASSERT_STRING(_tag_,highway);

    highwaytype=HighwayType(highway);

    if(highwaytype==Highway_None)
       XMLPARSE_INVALID(_tag_,highway);

    XMLPARSE_ASSERT_FLOATING(_tag_,kph); speed=atof(kph);

    if(speed<0)
       XMLPARSE_INVALID(_tag_,kph);

    loaded_profiles[nloaded_profiles-1]->speed[highwaytype]=kph_to_speed(speed);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the preferenceType XSD type is seen

  int preferenceType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *highway The contents of the 'highway' attribute (or NULL if not defined).

  const char *percent The contents of the 'percent' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int preferenceType_function(const char *_tag_,int _type_,const char *highway,const char *percent)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    Highway highwaytype;
    double p;

    XMLPARSE_ASSERT_STRING(_tag_,highway);

    highwaytype=HighwayType(highway);

    if(highwaytype==Highway_None)
       XMLPARSE_INVALID(_tag_,highway);

    XMLPARSE_ASSERT_FLOATING(_tag_,percent); p=atof(percent);

    if(p<0 || p>100)
       XMLPARSE_INVALID(_tag_,percent);

    loaded_profiles[nloaded_profiles-1]->highway[highwaytype]=(score_t)(p/100);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the propertyType XSD type is seen

  int propertyType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *percent The contents of the 'percent' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int propertyType_function(const char *_tag_,int _type_,const char *type,const char *percent)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    Property property;
    double p;

    XMLPARSE_ASSERT_STRING(_tag_,type);

    property=PropertyType(type);

    if(property==Property_None)
       XMLPARSE_INVALID(_tag_,type);

    XMLPARSE_ASSERT_FLOATING(_tag_,percent); p=atof(percent);

    if(p<0 || p>100)
       XMLPARSE_INVALID(_tag_,percent);

    loaded_profiles[nloaded_profiles-1]->props[property]=(score_t)(p/100);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the onewayType XSD type is seen

  int onewayType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *obey The contents of the 'obey' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int onewayType_function(const char *_tag_,int _type_,const char *obey)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    int o;

    XMLPARSE_ASSERT_INTEGER(_tag_,obey); o=atoi(obey);

    loaded_profiles[nloaded_profiles-1]->oneway=!!o;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the turnsType XSD type is seen

  int turnsType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *obey The contents of the 'obey' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int turnsType_function(const char *_tag_,int _type_,const char *obey)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    int o;

    XMLPARSE_ASSERT_INTEGER(_tag_,obey); o=atoi(obey);

    loaded_profiles[nloaded_profiles-1]->turns=!!o;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the weightType XSD type is seen

  int weightType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int weightType_function(const char *_tag_,int _type_,const char *limit)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    double l;

    XMLPARSE_ASSERT_FLOATING(_tag_,limit); l=atof(limit);

    if(l<0)
       XMLPARSE_INVALID(_tag_,limit);

    loaded_profiles[nloaded_profiles-1]->weight=tonnes_to_weight(l);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the heightType XSD type is seen

  int heightType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int heightType_function(const char *_tag_,int _type_,const char *limit)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    double l;

    XMLPARSE_ASSERT_FLOATING(_tag_,limit); l=atof(limit);

    if(l<0)
       XMLPARSE_INVALID(_tag_,limit);

    loaded_profiles[nloaded_profiles-1]->height=metres_to_height(l);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the widthType XSD type is seen

  int widthType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int widthType_function(const char *_tag_,int _type_,const char *limit)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    double l;

    XMLPARSE_ASSERT_FLOATING(_tag_,limit); l=atof(limit);

    if(l<0)
       XMLPARSE_INVALID(_tag_,limit);

    loaded_profiles[nloaded_profiles-1]->width=metres_to_width(l);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the lengthType XSD type is seen

  int lengthType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int lengthType_function(const char *_tag_,int _type_,const char *limit)
{
 if(_type_&XMLPARSE_TAG_START && store)
   {
    double l;

    XMLPARSE_ASSERT_FLOATING(_tag_,limit); l=atof(limit);

    if(l<0)
       XMLPARSE_INVALID(_tag_,limit);

    loaded_profiles[nloaded_profiles-1]->length=metres_to_length(l);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The XML profile parser.

  int ParseXMLProfiles Returns 0 if OK or something else in case of an error.

  const char *filename The name of the file to read.

  const char *name The name of the profile to read.

  int all Set to true to load all the profiles.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXMLProfiles(const char *filename,const char *name,int all)
{
 int fd;
 int retval;

 if(!ExistsFile(filename))
    return(1);

 fd=OpenFile(filename);

 /* Delete the existing profiles */

 if(nloaded_profiles)
    FreeXMLProfiles();

 /* Initialise variables used for parsing */

 store_all=all;

 store_name=name;

 store=0;

 /* Parse the file */

 retval=ParseXML(fd,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_ERRNONAME);

 CloseFile(fd);

 if(retval)
   {
    FreeXMLProfiles();

    return(2);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Return a list of the profile names that have been loaded from the XML file.

  char **GetProfileNames Returns a NULL terminated list of strings - all allocated.
  ++++++++++++++++++++++++++++++++++++++*/

char **GetProfileNames(void)
{
 char **list=calloc(1+nloaded_profiles,sizeof(char*));
 int i;

 for(i=0;i<nloaded_profiles;i++)
    list[i]=strcpy(malloc(strlen(loaded_profiles[i]->name)+1),loaded_profiles[i]->name);

 return(list);
}


/*++++++++++++++++++++++++++++++++++++++
  Get a named profile.

  Profile *GetProfile Returns a pointer to the profile.

  const char *name The name of the profile.
  ++++++++++++++++++++++++++++++++++++++*/

Profile *GetProfile(const char *name)
{
 int i;

 for(i=0;i<nloaded_profiles;i++)
    if(!strcmp(loaded_profiles[i]->name,name))
       return(loaded_profiles[i]);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Free the memory allocated when reading the profiles.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeXMLProfiles(void)
{
 int i;

 if(!loaded_profiles)
    return;

 for(i=0;i<nloaded_profiles;i++)
   {
    if(loaded_profiles[i]->name)
       free(loaded_profiles[i]->name);

    free(loaded_profiles[i]);
   }

 free(loaded_profiles);

 loaded_profiles=NULL;
 nloaded_profiles=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Update a profile with the highway preference scaling factors.

  int UpdateProfile Returns 1 in case of a problem.

  Profile *profile The profile to be updated.

  Ways *ways The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

int UpdateProfile(Profile *profile,Ways *ways)
{
 int i;

 /* Check the allowed transport type */

 profile->allow=TRANSPORTS(profile->transport);

 if(!(profile->allow & ways->file.allow))
    return(1);

 /* Normalise the highway preferences into the range ~0 -> 1 */

 profile->max_pref=0;

 for(i=1;i<Highway_Count;i++)
   {
    if(profile->highway[i]<0)
       profile->highway[i]=0;

    if(profile->highway[i]>1)
       profile->highway[i]=1;

    if(profile->highway[i]>profile->max_pref)
       profile->max_pref=profile->highway[i];
   }

 if(profile->max_pref==0)
    return(1);

 /* Normalise the property preferences into the range ~0 -> 1 */

 for(i=1;i<Property_Count;i++)
   {
    if(profile->props[i]<0)
       profile->props[i]=0;

    if(profile->props[i]>1)
       profile->props[i]=1;

    profile->props_yes[i]=profile->props[i];
    profile->props_no [i]=1-profile->props_yes[i];

    /* Squash the properties; selecting 60% preference without the sqrt() allows
       routes 50% longer on highways with the property compared to ones without.
       With the sqrt() function the ratio is only 22% allowing finer control. */

    profile->props_yes[i]=(score_t)sqrt(profile->props_yes[i]);
    profile->props_no [i]=(score_t)sqrt(profile->props_no[i] );

    if(profile->props_yes[i]<0.01f)
       profile->props_yes[i]=0.01f;

    if(profile->props_no[i]<0.01f)
       profile->props_no[i]=0.01f;
   }

 /* Find the fastest preferred speed */

 profile->max_speed=0;

 for(i=1;i<Highway_Count;i++)
    if(profile->speed[i]>profile->max_speed)
       profile->max_speed=profile->speed[i];

 if(profile->max_speed==0)
    return(1);

 /* Find the most preferred property combination */

 for(i=1;i<Property_Count;i++)
    if(ways->file.props & PROPERTIES(i))
      {
       if(profile->props_yes[i]>profile->props_no[i])
          profile->max_pref*=profile->props_yes[i];
       else
          profile->max_pref*=profile->props_no[i];
      }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out a profile.

  const Profile *profile The profile to print.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfile(const Profile *profile)
{
 int i;

 printf("Profile\n=======\n");

 printf("\n");

 printf("Transport: %s\n",TransportName(profile->transport));

 printf("\n");

 for(i=1;i<Highway_Count;i++)
    printf("Highway %-12s: %3d%%\n",HighwayName(i),(int)(0.5+profile->highway[i]*100));

 printf("\n");

 for(i=1;i<Highway_Count;i++)
    if(profile->highway[i])
       printf("Speed on %-12s: %3d km/h / %2.0f mph\n",HighwayName(i),profile->speed[i],(double)profile->speed[i]/1.6);

 printf("\n");

 for(i=1;i<Property_Count;i++)
    printf("Highway property %-12s: %3d%%\n",PropertyName(i),(int)(0.5+profile->props[i]*100));

 printf("\n");

 printf("Obey one-way  : %s\n",profile->oneway?"yes":"no");
 printf("Obey turns    : %s\n",profile->turns?"yes":"no");
 printf("Minimum weight: %.1f tonnes\n",weight_to_tonnes(profile->weight));
 printf("Minimum height: %.1f metres\n",height_to_metres(profile->height));
 printf("Minimum width : %.1f metres\n",width_to_metres(profile->width));
 printf("Minimum length: %.1f metres\n",length_to_metres(profile->length));
}


/*++++++++++++++++++++++++++++++++++++++
  Print out all of the loaded profiles as XML for use as program input.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesXML(void)
{
 int i,j;
 char *padding="                    ";

 printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
 printf("\n");

 printf("<routino-profiles xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"routino-profiles.xsd\">\n");
 printf("\n");

 for(j=0;j<nloaded_profiles;j++)
   {
    printf("  <profile name=\"%s\" transport=\"%s\">\n",loaded_profiles[j]->name,TransportName(loaded_profiles[j]->transport));

    printf("    <speeds>\n");
    for(i=1;i<Highway_Count;i++)
       printf("      <speed highway=\"%s\"%s kph=\"%d\" />\n",HighwayName(i),padding+8+strlen(HighwayName(i)),loaded_profiles[j]->speed[i]);
    printf("    </speeds>\n");

    printf("    <preferences>\n");
    for(i=1;i<Highway_Count;i++)
       printf("      <preference highway=\"%s\"%s percent=\"%.0f\" />\n",HighwayName(i),padding+8+strlen(HighwayName(i)),loaded_profiles[j]->highway[i]*100);
    printf("    </preferences>\n");

    printf("    <properties>\n");
    for(i=1;i<Property_Count;i++)
       printf("      <property type=\"%s\"%s percent=\"%.0f\" />\n",PropertyName(i),padding+8+strlen(PropertyName(i)),loaded_profiles[j]->props[i]*100);
    printf("    </properties>\n");

    printf("    <restrictions>\n");
    printf("      <oneway obey=\"%d\" /> \n",loaded_profiles[j]->oneway);
    printf("      <turns  obey=\"%d\" /> \n",loaded_profiles[j]->turns);
    printf("      <weight limit=\"%.1f\" />\n",weight_to_tonnes(loaded_profiles[j]->weight));
    printf("      <height limit=\"%.1f\" />\n",height_to_metres(loaded_profiles[j]->height));
    printf("      <width  limit=\"%.1f\" />\n",width_to_metres(loaded_profiles[j]->width));
    printf("      <length limit=\"%.1f\" />\n",length_to_metres(loaded_profiles[j]->length));
    printf("    </restrictions>\n");

    printf("  </profile>\n");
    printf("\n");
   }

 printf("</routino-profiles>\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out all of the loaded profiles as JavaScript Object Notation for use in a web page.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesJSON(void)
{
 int i,j;

 printf("var routino={ // contains all default Routino options (generated using \"--help-profile-json\").\n");
 printf("\n");

 printf("  // Default transport type\n");
 printf("  transport: \"motorcar\",\n");
 printf("\n");

 printf("  // Transport types\n");
 printf("  transports: { ");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),j+1);
 printf(" },\n");
 printf("\n");

 printf("  // Highway types\n");
 printf("  highways: { ");
 for(i=1;i<Highway_Count;i++)
    printf("%s%s: %d",i==1?"":", ",HighwayName(i),i);
 printf(" },\n");
 printf("\n");

 printf("  // Property types\n");
 printf("  properties: { ");
 for(i=1;i<Property_Count;i++)
    printf("%s%s: %d",i==1?"":", ",PropertyName(i),i);
 printf(" },\n");
 printf("\n");

 printf("  // Restriction types\n");
 printf("  restrictions: { oneway: 1, turns: 2, weight: 3, height: 4, width: 5, length: 6 },\n");
 printf("\n");

 printf("  // Allowed highways\n");
 printf("  profile_highway: {\n");
 for(i=1;i<Highway_Count;i++)
   {
    printf("    %12s: { ",HighwayName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s%s: %3d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),(int)(0.5+loaded_profiles[j]->highway[i]*100));
    printf(" }%s\n",i==(Highway_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  // Speed limits\n");
 printf("  profile_speed: {\n");
 for(i=1;i<Highway_Count;i++)
   {
    printf("    %12s: { ",HighwayName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s%s: %3d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->speed[i]);
    printf(" }%s\n",i==(Highway_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  // Highway properties\n");
 printf("  profile_property: {\n");
 for(i=1;i<Property_Count;i++)
   {
    printf("    %13s: { ",PropertyName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s%s: %3d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),(int)(0.5+loaded_profiles[j]->props[i]*100));
    printf(" }%s\n",i==(Property_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  // Restrictions\n");
 printf("  profile_restrictions: {\n");
 printf("    %12s: { ","oneway");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->oneway);
 printf(" },\n");
 printf("    %12s: { ","turns");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->turns);
 printf(" },\n");
 printf("    %12s: { ","weight");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4.1f",j==0?"":", ",TransportName(loaded_profiles[j]->transport),weight_to_tonnes(loaded_profiles[j]->weight));
 printf(" },\n");
 printf("    %12s: { ","height");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4.1f",j==0?"":", ",TransportName(loaded_profiles[j]->transport),height_to_metres(loaded_profiles[j]->height));
 printf(" },\n");
 printf("    %12s: { ","width");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4.1f",j==0?"":", ",TransportName(loaded_profiles[j]->transport),width_to_metres(loaded_profiles[j]->width));
 printf(" },\n");
 printf("    %12s: { ","length");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4.1f",j==0?"":", ",TransportName(loaded_profiles[j]->transport),length_to_metres(loaded_profiles[j]->length));
 printf(" }\n");
 printf("     }\n");
 printf("\n");

 printf("}; // end of routino variable\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out all of the loaded profiles as Perl for use in a web CGI.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesPerl(void)
{
 int i,j;

 printf("$routino={ # contains all default Routino options (generated using \"--help-profile-perl\").\n");
 printf("\n");

 printf("  # Default transport type\n");
 printf("  transport => \"motorcar\",\n");
 printf("\n");

 printf("  # Transport types\n");
 printf("  transports => { ");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s => %d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),j+1);
 printf(" },\n");
 printf("\n");

 printf("  # Highway types\n");
 printf("  highways => { ");
 for(i=1;i<Highway_Count;i++)
    printf("%s%s => %d",i==1?"":", ",HighwayName(i),i);
 printf(" },\n");
 printf("\n");

 printf("  # Property types\n");
 printf("  properties => { ");
 for(i=1;i<Property_Count;i++)
    printf("%s%s => %d",i==1?"":", ",PropertyName(i),i);
 printf(" },\n");
 printf("\n");

 printf("  # Restriction types\n");
 printf("  restrictions => { oneway => 1, turns => 2, weight => 3, height => 4, width => 5, length => 6 },\n");
 printf("\n");

 printf("  # Allowed highways\n");
 printf("  profile_highway => {\n");
 for(i=1;i<Highway_Count;i++)
   {
    printf("  %12s => {",HighwayName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s %s => %3d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),(int)(0.5+loaded_profiles[j]->highway[i]*100));
    printf(" }%s\n",i==(Highway_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  # Speed limits\n");
 printf("  profile_speed => {\n");
 for(i=1;i<Highway_Count;i++)
   {
    printf("  %12s => {",HighwayName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s %s => %3d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->speed[i]);
    printf(" }%s\n",i==(Highway_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  # Highway properties\n");
 printf("  profile_property => {\n");
 for(i=1;i<Property_Count;i++)
   {
    printf("  %13s => {",PropertyName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s %s => %3d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),(int)(0.5+loaded_profiles[j]->props[i]*100));
    printf(" }%s\n",i==(Property_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  # Restrictions\n");
 printf("  profile_restrictions => {\n");
 printf("    %12s => {","oneway");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->oneway);
 printf(" },\n");
 printf("    %12s => {","turns");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4d",j==0?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->turns);
 printf(" },\n");
 printf("    %12s => {","weight");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4.1f",j==0?"":", ",TransportName(loaded_profiles[j]->transport),weight_to_tonnes(loaded_profiles[j]->weight));
 printf(" },\n");
 printf("    %12s => {","height");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4.1f",j==0?"":", ",TransportName(loaded_profiles[j]->transport),height_to_metres(loaded_profiles[j]->height));
 printf(" },\n");
 printf("    %12s => {","width");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4.1f",j==0?"":", ",TransportName(loaded_profiles[j]->transport),width_to_metres(loaded_profiles[j]->width));
 printf(" },\n");
 printf("    %12s => {","length");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4.1f",j==0?"":", ",TransportName(loaded_profiles[j]->transport),length_to_metres(loaded_profiles[j]->length));
 printf(" }\n");
 printf("     }\n");
 printf("\n");

 printf("}; # end of routino variable\n");
}
