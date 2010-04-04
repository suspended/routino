/***************************************
 $Header: /home/amb/CVS/routino/src/profiles.c,v 1.35 2010-04-04 14:29:33 amb Exp $

 The pre-defined profiles and the functions for handling them.

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
#include <ctype.h>

#include "profiles.h"
#include "types.h"
#include "ways.h"
#include "xmlparse.h"
#include "functions.h"


/*+ The profiles that have been loaded from file. +*/
static Profile **loaded_profiles=NULL;

/*+ The number of profiles that have been loaded from file. +*/
static int nloaded_profiles=0;

/*+ The number of errors found in the XML file. +*/
static int profile_parse_error=0;


/* The XML tag processing function prototypes */

static void profile_function(int _type_,char *name,char *transport);
static void length_function(int _type_,char *limit);
static void width_function(int _type_,char *limit);
static void height_function(int _type_,char *limit);
static void weight_function(int _type_,char *limit);
static void oneway_function(int _type_,char *obey);
static void property_function(int _type_,char *type,char *percent);
static void preference_function(int _type_,char *highway,char *percent);
static void speed_function(int _type_,char *highway,char *kph);


/* The XML tag definitions */

/*+ The speedType type tag. +*/
static xmltag speed_tag=
              {"speed",
               {"highway","kph",NULL},
               speed_function,
               {NULL}};

/*+ The speedsType type tag. +*/
static xmltag speeds_tag=
              {"speeds",
               {NULL},
               NULL,
               {&speed_tag,NULL}};

/*+ The preferenceType type tag. +*/
static xmltag preference_tag=
              {"preference",
               {"highway","percent",NULL},
               preference_function,
               {NULL}};

/*+ The preferencesType type tag. +*/
static xmltag preferences_tag=
              {"preferences",
               {NULL},
               NULL,
               {&preference_tag,NULL}};

/*+ The propertyType type tag. +*/
static xmltag property_tag=
              {"property",
               {"type","percent",NULL},
               property_function,
               {NULL}};

/*+ The onewayType type tag. +*/
static xmltag oneway_tag=
              {"oneway",
               {"obey",NULL},
               oneway_function,
               {NULL}};

/*+ The propertiesType type tag. +*/
static xmltag properties_tag=
              {"properties",
               {NULL},
               NULL,
               {&property_tag,NULL}};

/*+ The weightType type tag. +*/
static xmltag weight_tag=
              {"weight",
               {"limit",NULL},
               weight_function,
               {NULL}};

/*+ The heightType type tag. +*/
static xmltag height_tag=
              {"height",
               {"limit",NULL},
               height_function,
               {NULL}};

/*+ The widthType type tag. +*/
static xmltag width_tag=
              {"width",
               {"limit",NULL},
               width_function,
               {NULL}};

/*+ The lengthType type tag. +*/
static xmltag length_tag=
              {"length",
               {"limit",NULL},
               length_function,
               {NULL}};

/*+ The restrictionsType type tag. +*/
static xmltag restrictions_tag=
              {"restrictions",
               {NULL},
               NULL,
               {&oneway_tag,&weight_tag,&height_tag,&width_tag,&length_tag,NULL}};

/*+ The profileType type tag. +*/
static xmltag profile_tag=
              {"profile",
               {"name","transport",NULL},
               profile_function,
               {&speeds_tag,&preferences_tag,&properties_tag,&restrictions_tag,NULL}};

/*+ The RoutinoProfilesType type tag. +*/
static xmltag routino_profiles_tag=
              {"routino-profiles",
               {NULL},
               NULL,
               {&profile_tag,NULL}};

/*+ The ?xmlType type tag. +*/
static xmltag xml_tag=
              {"xml",
               {"version","encoding",NULL},
               NULL,
               {NULL}};


/*+ The complete set of tags at the top level. +*/
static xmltag *xml_toplevel_tags[]={&xml_tag,&routino_profiles_tag,NULL};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the speedType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *highway The contents of the 'highway' attribute (or NULL if not defined).

  char *kph The contents of the 'kph' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void speed_function(int _type_,char *highway,char *kph)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    Highway highwaytype;

    if(!highway || !kph)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'highway' and 'kph' attributes must be specified in <speed> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    highwaytype=HighwayType(highway);

    if(highwaytype==Way_Count)
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid highway type '%s' in <speed> tag.\n",ParseXML_LineNumber(),highway);
       profile_parse_error++;
       return;
      }

    if(!isdigit(*kph))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid speed '%s' in <speed> tag.\n",ParseXML_LineNumber(),kph);
       profile_parse_error++;
       return;
      }

    loaded_profiles[nloaded_profiles-1]->speed[highwaytype]=kph_to_speed(atoi(kph));
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the preferenceType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *highway The contents of the 'highway' attribute (or NULL if not defined).

  char *percent The contents of the 'percent' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void preference_function(int _type_,char *highway,char *percent)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    Highway highwaytype;

    if(!highway || !percent)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'highway' and 'percent' attributes must be specified in <preference> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    highwaytype=HighwayType(highway);

    if(highwaytype==Way_Count)
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid highway type '%s' in <preference> tag.\n",ParseXML_LineNumber(),highway);
       profile_parse_error++;
       return;
      }

    if(!isdigit(*percent))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid percentage '%s' in <preference> tag.\n",ParseXML_LineNumber(),percent);
       profile_parse_error++;
       return;
      }

    loaded_profiles[nloaded_profiles-1]->highway[highwaytype]=atoi(percent);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the propertyType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *type The contents of the 'type' attribute (or NULL if not defined).

  char *percent The contents of the 'percent' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void property_function(int _type_,char *type,char *percent)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    Property property;

    if(!type || !percent)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'property' and 'percent' attributes must be specified in <property> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    property=PropertyType(type);

    if(property==Property_Count)
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid property type '%s' in <property> tag.\n",ParseXML_LineNumber(),type);
       profile_parse_error++;
       return;
      }

    if(!isdigit(*percent))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid percentage '%s' in <property> tag.\n",ParseXML_LineNumber(),percent);
       profile_parse_error++;
       return;
      }

    loaded_profiles[nloaded_profiles-1]->props_yes[property]=atoi(percent);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the onewayType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *obey The contents of the 'obey' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void oneway_function(int _type_,char *obey)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    if(!obey)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'obey' attribute must be specified in <oneway> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    if(!isdigit(*obey))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid value for 'obey' attribute '%s' in <oneway> tag.\n",ParseXML_LineNumber(),obey);
       profile_parse_error++;
       return;
      }

    loaded_profiles[nloaded_profiles-1]->oneway=!!atoi(obey);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the weightType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void weight_function(int _type_,char *limit)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    if(!limit)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'limit' attribute must be specified in <weight> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    if(!isdigit(*limit))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid value for 'limit' attribute '%s' in <weight> tag.\n",ParseXML_LineNumber(),limit);
       profile_parse_error++;
       return;
      }

    loaded_profiles[nloaded_profiles-1]->weight=tonnes_to_weight(atof(limit));
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the heightType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void height_function(int _type_,char *limit)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    if(!limit)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'limit' attribute must be specified in <height> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    if(!isdigit(*limit))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid value for 'limit' attribute '%s' in <height> tag.\n",ParseXML_LineNumber(),limit);
       profile_parse_error++;
       return;
      }

    loaded_profiles[nloaded_profiles-1]->height=metres_to_height(atof(limit));
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the widthType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void width_function(int _type_,char *limit)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    if(!limit)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'limit' attribute must be specified in <width> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    if(!isdigit(*limit))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid value for 'limit' attribute '%s' in <width> tag.\n",ParseXML_LineNumber(),limit);
       profile_parse_error++;
       return;
      }

    loaded_profiles[nloaded_profiles-1]->width=metres_to_width(atof(limit));
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the lengthType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *limit The contents of the 'limit' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void length_function(int _type_,char *limit)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    if(!limit)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'limit' attribute must be specified in <length> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    if(!isdigit(*limit))
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid value for 'limit' attribute '%s' in <length> tag.\n",ParseXML_LineNumber(),limit);
       profile_parse_error++;
       return;
      }

    loaded_profiles[nloaded_profiles-1]->length=metres_to_length(atof(limit));
   }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the profileType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *name The contents of the 'name' attribute (or NULL if not defined).

  char *transport The contents of the 'transport' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void profile_function(int _type_,char *name,char *transport)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    Transport transporttype;
    int i;

    if(!name || !transport)
      {
       fprintf(stderr,"XML Parser: Error on line %d: 'name' and 'transport' attributes must be specified in <profile> tag.\n",ParseXML_LineNumber());
       profile_parse_error++;
       return;
      }

    for(i=0;i<nloaded_profiles;i++)
       if(!strcmp(name,loaded_profiles[i]->name))
         {
          fprintf(stderr,"XML Parser: Error on line %d: profile name '%s' must be unique in <profile> tag.\n",ParseXML_LineNumber(),name);
          profile_parse_error++;
          return;
         }

    transporttype=TransportType(transport);

    if(transporttype==Transport_None)
      {
       fprintf(stderr,"XML Parser: Error on line %d: invalid transport type '%s' in <profile> tag.\n",ParseXML_LineNumber(),transport);
       profile_parse_error++;
       return;
      }

    if((nloaded_profiles%16)==0)
       loaded_profiles=(Profile**)realloc((void*)loaded_profiles,(nloaded_profiles+16)*sizeof(Profile*));

    nloaded_profiles++;

    loaded_profiles[nloaded_profiles-1]=(Profile*)calloc(1,sizeof(Profile));

    loaded_profiles[nloaded_profiles-1]->name=strcpy(malloc(strlen(name)+1),name);
    loaded_profiles[nloaded_profiles-1]->transport=transporttype;
   }
}


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

 profile_parse_error=0;

 retval=ParseXML(file,xml_toplevel_tags,2);

 fclose(file);

 if(retval || profile_parse_error)
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
