/***************************************
 $Header: /home/amb/CVS/routino/src/profiles.c,v 1.38 2010-04-12 17:33:52 amb Exp $

 Load the profiles from a file and the functions for handling them.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2010 Andrew M. Bishop

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

#include "profiles.h"
#include "types.h"
#include "ways.h"
#include "xmlparse.h"
#include "functions.h"


/*+ The profiles that have been loaded from file. +*/
static Profile **loaded_profiles=NULL;

/*+ The number of profiles that have been loaded from file. +*/
static int nloaded_profiles=0;


/* The XML tag processing function prototypes */

//static int xml_function(int _type_,const char *version,const char *encoding);
//static int routino_profiles_function(int _type_);
static int profile_function(int _type_,const char *name,const char *transport);
//static int restrictions_function(int _type_);
static int length_function(int _type_,const char *limit);
static int width_function(int _type_,const char *limit);
static int height_function(int _type_,const char *limit);
static int weight_function(int _type_,const char *limit);
static int oneway_function(int _type_,const char *obey);
//static int properties_function(int _type_);
static int property_function(int _type_,const char *type,const char *percent);
//static int preference_function(int _type_,const char *highway,const char *percent);
static int preference_function(int _type_,const char *highway,const char *percent);
//static int speeds_function(int _type_);
static int speed_function(int _type_,const char *highway,const char *kph);


/* The XML tag definitions */

/*+ The speedType type tag. +*/
static xmltag speed_tag=
              {"speed",
               2, {"highway","kph"},
               speed_function,
               {NULL}};

/*+ The speedsType type tag. +*/
static xmltag speeds_tag=
              {"speeds",
               0, {NULL},
               NULL,
               {&speed_tag,NULL}};

/*+ The preferenceType type tag. +*/
static xmltag preference_tag=
              {"preference",
               2, {"highway","percent"},
               preference_function,
               {NULL}};

/*+ The preferencesType type tag. +*/
static xmltag preferences_tag=
              {"preferences",
               0, {NULL},
               NULL,
               {&preference_tag,NULL}};

/*+ The propertyType type tag. +*/
static xmltag property_tag=
              {"property",
               2, {"type","percent"},
               property_function,
               {NULL}};

/*+ The onewayType type tag. +*/
static xmltag oneway_tag=
              {"oneway",
               1, {"obey"},
               oneway_function,
               {NULL}};

/*+ The propertiesType type tag. +*/
static xmltag properties_tag=
              {"properties",
               0, {NULL},
               NULL,
               {&property_tag,NULL}};

/*+ The weightType type tag. +*/
static xmltag weight_tag=
              {"weight",
               1, {"limit"},
               weight_function,
               {NULL}};

/*+ The heightType type tag. +*/
static xmltag height_tag=
              {"height",
               1, {"limit"},
               height_function,
               {NULL}};

/*+ The widthType type tag. +*/
static xmltag width_tag=
              {"width",
               1, {"limit"},
               width_function,
               {NULL}};

/*+ The lengthType type tag. +*/
static xmltag length_tag=
              {"length",
               1, {"limit"},
               length_function,
               {NULL}};

/*+ The restrictionsType type tag. +*/
static xmltag restrictions_tag=
              {"restrictions",
               0, {NULL},
               NULL,
               {&oneway_tag,&weight_tag,&height_tag,&width_tag,&length_tag,NULL}};

/*+ The profileType type tag. +*/
static xmltag profile_tag=
              {"profile",
               2, {"name","transport"},
               profile_function,
               {&speeds_tag,&preferences_tag,&properties_tag,&restrictions_tag,NULL}};

/*+ The RoutinoProfilesType type tag. +*/
static xmltag routino_profiles_tag=
              {"routino-profiles",
               0, {NULL},
               NULL,
               {&profile_tag,NULL}};

/*+ The xmlType type tag. +*/
static xmltag xml_tag=
              {"xml",
               2, {"version","encoding"},
               NULL,
               {NULL}};


/*+ The complete set of tags at the top level. +*/
static xmltag *xml_toplevel_tags[]={&xml_tag,&routino_profiles_tag,NULL};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the speedType XSD type is seen

  int speed_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *highway The contents of the 'highway' attribute (or NULL if not defined).

  const char *kph The contents of the 'kph' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int speed_function(int _type_,const char *highway,const char *kph)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    double speed;
    Highway highwaytype;

    XMLPARSE_ASSERT_STRING("speed",highway);

    highwaytype=HighwayType(highway);

    if(highwaytype==Way_Count)
       XMLPARSE_INVALID("speed",highway);

    XMLPARSE_ASSERT_FLOATING("speed",kph,speed);

    loaded_profiles[nloaded_profiles-1]->speed[highwaytype]=kph_to_speed(speed);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the speedsType XSD type is seen

  int speeds_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int speeds_function(int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the preferenceType XSD type is seen

  int preference_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *highway The contents of the 'highway' attribute (or NULL if not defined).

  const char *percent The contents of the 'percent' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int preference_function(int _type_,const char *highway,const char *percent)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    Highway highwaytype;
    double p;

    XMLPARSE_ASSERT_STRING("preference",highway);

    highwaytype=HighwayType(highway);

    if(highwaytype==Way_Count)
       XMLPARSE_INVALID("preference",highway);

    XMLPARSE_ASSERT_FLOATING("preference",percent,p);

    loaded_profiles[nloaded_profiles-1]->highway[highwaytype]=p;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the preferencesType XSD type is seen

  int preferences_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int preferences_function(int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the propertyType XSD type is seen

  int property_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *percent The contents of the 'percent' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int property_function(int _type_,const char *type,const char *percent)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    Property property;
    double p;

    XMLPARSE_ASSERT_STRING("property",type);

    property=PropertyType(type);

    if(property==Property_Count)
       XMLPARSE_INVALID("property",type);

    XMLPARSE_ASSERT_FLOATING("property",percent,p);

    loaded_profiles[nloaded_profiles-1]->props_yes[property]=p;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the onewayType XSD type is seen

  int oneway_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *obey The contents of the 'obey' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int oneway_function(int _type_,const char *obey)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    int o;

    XMLPARSE_ASSERT_INTEGER("oneway",obey,o);

    loaded_profiles[nloaded_profiles-1]->oneway=!!o;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the propertiesType XSD type is seen

  int properties_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int properties_function(int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the weightType XSD type is seen

  int weight_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int weight_function(int _type_,const char *limit)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    double l;

    XMLPARSE_ASSERT_FLOATING("weight",limit,l);

    loaded_profiles[nloaded_profiles-1]->weight=tonnes_to_weight(l);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the heightType XSD type is seen

  int height_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int height_function(int _type_,const char *limit)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    double l;

    XMLPARSE_ASSERT_FLOATING("height",limit,l);

    loaded_profiles[nloaded_profiles-1]->height=metres_to_height(l);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the widthType XSD type is seen

  int width_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int width_function(int _type_,const char *limit)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    double l;

    XMLPARSE_ASSERT_FLOATING("width",limit,l);

    loaded_profiles[nloaded_profiles-1]->width=metres_to_width(l);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the lengthType XSD type is seen

  int length_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int length_function(int _type_,const char *limit)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    double l;

    XMLPARSE_ASSERT_FLOATING("length",limit,l);

    loaded_profiles[nloaded_profiles-1]->length=metres_to_length(l);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the restrictionsType XSD type is seen

  int restrictions_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int restrictions_function(int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the profileType XSD type is seen

  int profile_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *name The contents of the 'name' attribute (or NULL if not defined).

  const char *transport The contents of the 'transport' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int profile_function(int _type_,const char *name,const char *transport)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    Transport transporttype;
    int i;

    XMLPARSE_ASSERT_STRING("profile",name);
    XMLPARSE_ASSERT_STRING("profile",transport);

    for(i=0;i<nloaded_profiles;i++)
       if(!strcmp(name,loaded_profiles[i]->name))
          XMLPARSE_MESSAGE("profile","profile name must be unique");

    transporttype=TransportType(transport);

    if(transporttype==Transport_None)
       XMLPARSE_INVALID("profile",transport);

    if((nloaded_profiles%16)==0)
       loaded_profiles=(Profile**)realloc((void*)loaded_profiles,(nloaded_profiles+16)*sizeof(Profile*));

    nloaded_profiles++;

    loaded_profiles[nloaded_profiles-1]=(Profile*)calloc(1,sizeof(Profile));

    loaded_profiles[nloaded_profiles-1]->name=strcpy(malloc(strlen(name)+1),name);
    loaded_profiles[nloaded_profiles-1]->transport=transporttype;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the RoutinoProfilesType XSD type is seen

  int routino_profiles_function Returns 0 if no error occured or something else otherwise.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int routino_profiles_function(int _type_)
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
  The XML profile parser.

  int ParseXMLProfiles Returns 0 if OK or something else in case of an error.

  const char *filename The name of the file to read.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXMLProfiles(const char *filename)
{
 int retval;

 if(!ExistsFile(filename))
   {
    fprintf(stderr,"Error: Specified profiles file '%s' does not exist.\n",filename);
    return(1);
   }

 FILE *file=fopen(filename,"r");

 if(!file)
   {
    fprintf(stderr,"Error: Cannot open profiles file '%s' for reading.\n",filename);
    return(1);
   }

 retval=ParseXML(file,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_ERRNONAME);

 fclose(file);

 if(retval)
   {
    int i;

    for(i=0;i<nloaded_profiles;i++)
       free(loaded_profiles[i]);
    free(loaded_profiles);

    nloaded_profiles=0;

    return(1);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Get the profile for a type of transport.

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
  Update a profile with highway preference scaling factor.

  Profile *profile The profile to be updated.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateProfile(Profile *profile)
{
 score_t hmax=0;
 int i;

 profile->allow=ALLOWED(profile->transport);

 /* Normalise the highway preferences into the range 0 -> 1 */

 for(i=1;i<Way_Count;i++)
   {
    if(profile->highway[i]<0)
       profile->highway[i]=0;

    if(profile->highway[i]>hmax)
       hmax=profile->highway[i];
   }

 for(i=1;i<Way_Count;i++)
    profile->highway[i]/=hmax;

 /* Normalise the attribute preferences into the range 0 -> 1 */

 for(i=1;i<Property_Count;i++)
   {
    if(profile->props_yes[i]<0)
       profile->props_yes[i]=0;

    if(profile->props_yes[i]>100)
       profile->props_yes[i]=100;

    profile->props_yes[i]/=100;
    profile->props_no [i] =1-profile->props_yes[i];
   }

 /* Find the fastest and most preferred highway type */

 profile->max_speed=0;

 for(i=1;i<Way_Count;i++)
    if(profile->speed[i]>profile->max_speed)
       profile->max_speed=profile->speed[i];

 profile->max_pref=1; /* since highway prefs were normalised to 1 */

 for(i=1;i<Property_Count;i++)
    if(profile->props_yes[i]>profile->props_no[i])
       profile->max_pref*=profile->props_yes[i];
    else if(profile->props_no[i]>profile->props_yes[i])
       profile->max_pref*=profile->props_no[i];
}


/*++++++++++++++++++++++++++++++++++++++
  Print out a profile.

  const Profile *profile The profile to print.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfile(const Profile *profile)
{
 unsigned int i;

 printf("Profile\n=======\n");

 printf("\n");

 printf("Transport: %s\n",TransportName(profile->transport));

 printf("\n");

 for(i=1;i<Way_Count;i++)
    printf("Highway %-12s: %3d%%\n",HighwayName(i),(int)profile->highway[i]);

 printf("\n");

 for(i=1;i<Way_Count;i++)
    if(profile->highway[i])
       printf("Speed on %-12s: %3d km/h / %2.0f mph\n",HighwayName(i),profile->speed[i],(double)profile->speed[i]/1.6);

 printf("\n");

 for(i=1;i<Property_Count;i++)
    printf("Highway property %-12s: %3d%%\n",PropertyName(i),(int)profile->props_yes[i]);

 printf("\n");

 printf("Obey one-way  : %s\n",profile->oneway?"yes":"no");
 printf("Minimum weight: %.1f tonnes\n",weight_to_tonnes(profile->weight));
 printf("Minimum height: %.1f metres\n",height_to_metres(profile->height));
 printf("Minimum width : %.1f metres\n",width_to_metres(profile->width));
 printf("Minimum length: %.1f metres\n",length_to_metres(profile->length));
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the profiles as XML for use as program input.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesXML(void)
{
 unsigned int i,j;
 char *padding="                ";

 printf("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
 printf("\n");

 printf("<routino-profiles xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"routino-profiles.xsd\">\n");
 printf("\n");

 for(j=0;j<nloaded_profiles;j++)
   {
    printf("  <profile name=\"%s\" transport=\"%s\">\n",loaded_profiles[j]->name,TransportName(loaded_profiles[j]->transport));

    printf("    <speeds>\n");
    for(i=1;i<Way_Count;i++)
       printf("      <speed highway=\"%s\"%s kph=\"%d\" />\n",HighwayName(i),padding+3+strlen(HighwayName(i)),loaded_profiles[j]->speed[i]);
    printf("    </speeds>\n");

    printf("    <preferences>\n");
    for(i=1;i<Way_Count;i++)
       printf("      <preference highway=\"%s\"%s percent=\"%.0f\" />\n",HighwayName(i),padding+3+strlen(HighwayName(i)),loaded_profiles[j]->highway[i]);
    printf("    </preferences>\n");

    printf("    <properties>\n");
    for(i=1;i<Property_Count;i++)
       printf("      <property type=\"%s\"%s percent=\"%.0f\" />\n",PropertyName(i),padding+6+strlen(PropertyName(i)),loaded_profiles[j]->props_yes[i]);
    printf("    </properties>\n");

    printf("    <restrictions>\n");
    printf("      <oneway obey=\"%d\" /> \n",loaded_profiles[j]->oneway);
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
  Print out the profiles as JavaScript Object Notation for use in a web form.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesJSON(void)
{
 unsigned int i,j;

 printf("var routino={ // contains all default Routino options (generated using \"--help-profile-json\").\n");
 printf("\n");

 printf("  // Default transport type\n");
 printf("  transport: 'motorcar',\n");
 printf("\n");

 printf("  // Transport types\n");
 printf("  transports: {");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),j);
 printf("},\n");
 printf("\n");

 printf("  // Highway types\n");
 printf("  highways: {");
 for(i=1;i<Way_Count;i++)
    printf("%s%s: %d",i==1?"":", ",HighwayName(i),i);
 printf("},\n");
 printf("\n");

 printf("  // Property types\n");
 printf("  properties: {");
 for(i=1;i<Property_Count;i++)
    printf("%s%s: %d",i==1?"":", ",PropertyName(i),i);
 printf("},\n");
 printf("\n");

 printf("  // Restriction types\n");
 printf("  restrictions: {oneway: 1, weight: 2, height: 3, width: 4, length: 5},\n");
 printf("\n");

 printf("  // Allowed highways\n");
 printf("  profile_highway: {\n");
 for(i=1;i<Way_Count;i++)
   {
    printf("    %12s: {",HighwayName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s%s: %3d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),(int)loaded_profiles[j]->highway[i]);
    printf("}%s\n",i==(Way_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  // Speed limits\n");
 printf("  profile_speed: {\n");
 for(i=1;i<Way_Count;i++)
   {
    printf("    %12s: {",HighwayName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s%s: %3d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->speed[i]);
    printf("}%s\n",i==(Way_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  // Highway properties\n");
 printf("  profile_property: {\n");
 for(i=1;i<Property_Count;i++)
   {
    printf("    %12s: {",PropertyName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s%s: %3d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),(int)loaded_profiles[j]->props_yes[i]);
    printf("}%s\n",i==(Property_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  // Restrictions\n");
 printf("  profile_restrictions: {\n");
 printf("    %12s: {","oneway");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->oneway);
 printf("},\n");
 printf("    %12s: {","weight");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4.1f",j==1?"":", ",TransportName(loaded_profiles[j]->transport),weight_to_tonnes(loaded_profiles[j]->weight));
 printf("},\n");
 printf("    %12s: {","height");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4.1f",j==1?"":", ",TransportName(loaded_profiles[j]->transport),height_to_metres(loaded_profiles[j]->height));
 printf("},\n");
 printf("    %12s: {","width");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4.1f",j==1?"":", ",TransportName(loaded_profiles[j]->transport),width_to_metres(loaded_profiles[j]->width));
 printf("},\n");
 printf("    %12s: {","length");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s: %4.1f",j==1?"":", ",TransportName(loaded_profiles[j]->transport),length_to_metres(loaded_profiles[j]->length));
 printf("}\n");
 printf("     }\n");
 printf("\n");

 printf("}; // end of routino variable\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the profiles as Perl for use in a web CGI.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesPerl(void)
{
 unsigned int i,j;

 printf("$routino={ # contains all default Routino options (generated using \"--help-profile-perl\").\n");
 printf("\n");

 printf("  # Default transport type\n");
 printf("  transport => 'motorcar',\n");
 printf("\n");

 printf("  # Transport types\n");
 printf("  transports => {");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s%s => %d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),j);
 printf("},\n");
 printf("\n");

 printf("  # Highway types\n");
 printf("  highways => {");
 for(i=1;i<Way_Count;i++)
    printf("%s%s => %d",i==1?"":", ",HighwayName(i),i);
 printf("},\n");
 printf("\n");

 printf("  # Property types\n");
 printf("  properties => {");
 for(i=1;i<Property_Count;i++)
    printf("%s%s => %d",i==1?"":", ",PropertyName(i),i);
 printf("},\n");
 printf("\n");

 printf("  # Restriction types\n");
 printf("  restrictions => {oneway => 1, weight => 2, height => 3, width => 4, length => 5},\n");
 printf("\n");

 printf("  # Allowed highways\n");
 printf("  profile_highway => {\n");
 for(i=1;i<Way_Count;i++)
   {
    printf("  %12s => {",HighwayName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s %s => %3d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),(int)loaded_profiles[j]->highway[i]);
    printf("}%s\n",i==(Way_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  # Speed limits\n");
 printf("  profile_speed => {\n");
 for(i=1;i<Way_Count;i++)
   {
    printf("  %12s => {",HighwayName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s %s => %3d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->speed[i]);
    printf("}%s\n",i==(Way_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  # Highway properties\n");
 printf("  profile_property => {\n");
 for(i=1;i<Property_Count;i++)
   {
    printf("  %12s => {",PropertyName(i));
    for(j=0;j<nloaded_profiles;j++)
       printf("%s %s => %3d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),(int)loaded_profiles[j]->props_yes[i]);
    printf("}%s\n",i==(Property_Count-1)?"":",");
   }
 printf("     },\n");
 printf("\n");

 printf("  # Restrictions\n");
 printf("  profile_restrictions => {\n");
 printf("    %12s => {","oneway");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4d",j==1?"":", ",TransportName(loaded_profiles[j]->transport),loaded_profiles[j]->oneway);
 printf("},\n");
 printf("    %12s => {","weight");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4.1f",j==1?"":", ",TransportName(loaded_profiles[j]->transport),weight_to_tonnes(loaded_profiles[j]->weight));
 printf("},\n");
 printf("    %12s => {","height");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4.1f",j==1?"":", ",TransportName(loaded_profiles[j]->transport),height_to_metres(loaded_profiles[j]->height));
 printf("},\n");
 printf("    %12s => {","width");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4.1f",j==1?"":", ",TransportName(loaded_profiles[j]->transport),width_to_metres(loaded_profiles[j]->width));
 printf("},\n");
 printf("    %12s => {","length");
 for(j=0;j<nloaded_profiles;j++)
    printf("%s %s => %4.1f",j==1?"":", ",TransportName(loaded_profiles[j]->transport),length_to_metres(loaded_profiles[j]->length));
 printf("}\n");
 printf("     },\n");
 printf("\n");

 printf("}; # end of routino variable\n");
}
