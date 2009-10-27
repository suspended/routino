/***************************************
 $Header: /home/amb/CVS/routino/src/profiles.c,v 1.23 2009-10-27 17:31:44 amb Exp $

 The pre-defined profiles and the functions for handling them.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008,2009 Andrew M. Bishop

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

#include "profiles.h"
#include "types.h"
#include "ways.h"


/*+ The set of built-in profiles for different transport types. +*/
static Profile builtin_profiles[]=
 {
  /* The profile for travel by Foot */

  [Transport_Foot] = {
                      .transport=Transport_Foot,
                      .allow    =Allow_Foot,
                      .highway  = {
                                   [Way_Motorway    ] =   0,
                                   [Way_Trunk       ] =  40,
                                   [Way_Primary     ] =  50,
                                   [Way_Secondary   ] =  60,
                                   [Way_Tertiary    ] =  70,
                                   [Way_Unclassified] =  80,
                                   [Way_Residential ] =  90,
                                   [Way_Service     ] =  90,
                                   [Way_Track       ] =  95,
                                   [Way_Cycleway    ] =  95,
                                   [Way_Path        ] = 100,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = kph_to_speed(4),
                                   [Way_Primary     ] = kph_to_speed(4),
                                   [Way_Secondary   ] = kph_to_speed(4),
                                   [Way_Tertiary    ] = kph_to_speed(4),
                                   [Way_Unclassified] = kph_to_speed(4),
                                   [Way_Residential ] = kph_to_speed(4),
                                   [Way_Service     ] = kph_to_speed(4),
                                   [Way_Track       ] = kph_to_speed(4),
                                   [Way_Cycleway    ] = kph_to_speed(4),
                                   [Way_Path        ] = kph_to_speed(4),
                                  },
                      .oneway   = 0,
                      .weight   = 0,
                      .height   = 0,
                      .width    = 0,
                      .length   = 0,
                     },

  /* The profile for travel by Horse */

  [Transport_Horse] = {
                      .transport=Transport_Horse,
                      .allow    =Allow_Horse,
                      .highway  = {
                                   [Way_Motorway    ] =   0,
                                   [Way_Trunk       ] =  25,
                                   [Way_Primary     ] =  50,
                                   [Way_Secondary   ] =  50,
                                   [Way_Tertiary    ] =  75,
                                   [Way_Unclassified] =  75,
                                   [Way_Residential ] =  75,
                                   [Way_Service     ] =  75,
                                   [Way_Track       ] = 100,
                                   [Way_Cycleway    ] =  90,
                                   [Way_Path        ] = 100,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = kph_to_speed(8),
                                   [Way_Primary     ] = kph_to_speed(8),
                                   [Way_Secondary   ] = kph_to_speed(8),
                                   [Way_Tertiary    ] = kph_to_speed(8),
                                   [Way_Unclassified] = kph_to_speed(8),
                                   [Way_Residential ] = kph_to_speed(8),
                                   [Way_Service     ] = kph_to_speed(8),
                                   [Way_Track       ] = kph_to_speed(8),
                                   [Way_Cycleway    ] = kph_to_speed(8),
                                   [Way_Path        ] = kph_to_speed(8),
                                  },
                      .oneway   = 1,
                      .weight   = 0,
                      .height   = 0,
                      .width    = 0,
                      .length   = 0,
                     },

  /* The profile for travel by Bicycle */

  [Transport_Bicycle] = {
                      .transport=Transport_Bicycle,
                      .allow    =Allow_Bicycle,
                      .highway  = {
                                   [Way_Motorway    ] =   0,
                                   [Way_Trunk       ] =  30,
                                   [Way_Primary     ] =  70,
                                   [Way_Secondary   ] =  80,
                                   [Way_Tertiary    ] =  90,
                                   [Way_Unclassified] =  90,
                                   [Way_Residential ] =  90,
                                   [Way_Service     ] =  90,
                                   [Way_Track       ] =  90,
                                   [Way_Cycleway    ] = 100,
                                   [Way_Path        ] =  90,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = kph_to_speed(20),
                                   [Way_Primary     ] = kph_to_speed(20),
                                   [Way_Secondary   ] = kph_to_speed(20),
                                   [Way_Tertiary    ] = kph_to_speed(20),
                                   [Way_Unclassified] = kph_to_speed(20),
                                   [Way_Residential ] = kph_to_speed(20),
                                   [Way_Service     ] = kph_to_speed(20),
                                   [Way_Track       ] = kph_to_speed(20),
                                   [Way_Cycleway    ] = kph_to_speed(20),
                                   [Way_Path        ] = kph_to_speed(20),
                                  },
                      .oneway   = 1,
                      .weight   = 0,
                      .height   = 0,
                      .width    = 0,
                      .length   = 0,
                     },

  /* The profile for travel by Moped */

  [Transport_Moped] = {
                      .transport=Transport_Moped,
                      .allow    =Allow_Moped,
                      .highway  = {
                                   [Way_Motorway    ] =   0,
                                   [Way_Trunk       ] =  90,
                                   [Way_Primary     ] = 100,
                                   [Way_Secondary   ] =  90,
                                   [Way_Tertiary    ] =  80,
                                   [Way_Unclassified] =  70,
                                   [Way_Residential ] =  60,
                                   [Way_Service     ] =  80,
                                   [Way_Track       ] =   0,
                                   [Way_Cycleway    ] =   0,
                                   [Way_Path        ] =   0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = kph_to_speed(30*1.6),
                                   [Way_Trunk       ] = kph_to_speed(30*1.6),
                                   [Way_Primary     ] = kph_to_speed(30*1.6),
                                   [Way_Secondary   ] = kph_to_speed(30*1.6),
                                   [Way_Tertiary    ] = kph_to_speed(30*1.6),
                                   [Way_Unclassified] = kph_to_speed(30*1.6),
                                   [Way_Residential ] = kph_to_speed(30*1.6),
                                   [Way_Service     ] = kph_to_speed(20*1.6),
                                   [Way_Track       ] = kph_to_speed(10*1.6),
                                   [Way_Cycleway    ] = 0,
                                   [Way_Path        ] = 0,
                                  },
                      .oneway   = 1,
                      .weight   = 0,
                      .height   = 0,
                      .width    = 0,
                      .length   = 0,
                     },

  /* The profile for travel by Motorbike */

  [Transport_Motorbike] = {
                      .transport=Transport_Motorbike,
                      .allow    =Allow_Motorbike,
                      .highway  = {
                                   [Way_Motorway    ] = 100,
                                   [Way_Trunk       ] = 100,
                                   [Way_Primary     ] =  90,
                                   [Way_Secondary   ] =  80,
                                   [Way_Tertiary    ] =  70,
                                   [Way_Unclassified] =  60,
                                   [Way_Residential ] =  50,
                                   [Way_Service     ] =  80,
                                   [Way_Track       ] =   0,
                                   [Way_Cycleway    ] =   0,
                                   [Way_Path        ] =   0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = kph_to_speed(70*1.6),
                                   [Way_Trunk       ] = kph_to_speed(60*1.6),
                                   [Way_Primary     ] = kph_to_speed(60*1.6),
                                   [Way_Secondary   ] = kph_to_speed(55*1.6),
                                   [Way_Tertiary    ] = kph_to_speed(50*1.6),
                                   [Way_Unclassified] = kph_to_speed(40*1.6),
                                   [Way_Residential ] = kph_to_speed(30*1.6),
                                   [Way_Service     ] = kph_to_speed(20*1.6),
                                   [Way_Track       ] = kph_to_speed(10*1.6),
                                   [Way_Cycleway    ] = 0,
                                   [Way_Path        ] = 0,
                                  },
                      .oneway   = 1,
                      .weight   = 0,
                      .height   = 0,
                      .width    = 0,
                      .length   = 0,
                     },

  /* The profile for travel by Motorcar */

  [Transport_Motorcar] = {
                      .transport=Transport_Motorcar,
                      .allow    =Allow_Motorcar,
                      .highway  = {
                                   [Way_Motorway    ] = 100,
                                   [Way_Trunk       ] = 100,
                                   [Way_Primary     ] =  90,
                                   [Way_Secondary   ] =  80,
                                   [Way_Tertiary    ] =  70,
                                   [Way_Unclassified] =  60,
                                   [Way_Residential ] =  50,
                                   [Way_Service     ] =  80,
                                   [Way_Track       ] =   0,
                                   [Way_Cycleway    ] =   0,
                                   [Way_Path        ] =   0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = kph_to_speed(70*1.6),
                                   [Way_Trunk       ] = kph_to_speed(60*1.6),
                                   [Way_Primary     ] = kph_to_speed(60*1.6),
                                   [Way_Secondary   ] = kph_to_speed(55*1.6),
                                   [Way_Tertiary    ] = kph_to_speed(50*1.6),
                                   [Way_Unclassified] = kph_to_speed(40*1.6),
                                   [Way_Residential ] = kph_to_speed(30*1.6),
                                   [Way_Service     ] = kph_to_speed(20*1.6),
                                   [Way_Track       ] = kph_to_speed(10*1.6),
                                   [Way_Cycleway    ] = 0,
                                   [Way_Path        ] = 0,
                                  },
                      .oneway   = 1,
                      .weight   = 0,
                      .height   = 0,
                      .width    = 0,
                      .length   = 0,
                     },

  /* The profile for travel by Goods */

  [Transport_Goods] = {
                      .transport=Transport_Goods,
                      .allow    =Allow_Goods,
                      .highway  = {
                                   [Way_Motorway    ] = 100,
                                   [Way_Trunk       ] = 100,
                                   [Way_Primary     ] =  90,
                                   [Way_Secondary   ] =  80,
                                   [Way_Tertiary    ] =  70,
                                   [Way_Unclassified] =  60,
                                   [Way_Residential ] =  50,
                                   [Way_Service     ] =  80,
                                   [Way_Track       ] =   0,
                                   [Way_Cycleway    ] =   0,
                                   [Way_Path        ] =   0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = kph_to_speed(60*1.6),
                                   [Way_Trunk       ] = kph_to_speed(60*1.6),
                                   [Way_Primary     ] = kph_to_speed(60*1.6),
                                   [Way_Secondary   ] = kph_to_speed(55*1.6),
                                   [Way_Tertiary    ] = kph_to_speed(50*1.6),
                                   [Way_Unclassified] = kph_to_speed(40*1.6),
                                   [Way_Residential ] = kph_to_speed(30*1.6),
                                   [Way_Service     ] = kph_to_speed(20*1.6),
                                   [Way_Track       ] = kph_to_speed(10*1.6),
                                   [Way_Cycleway    ] = 0,
                                   [Way_Path        ] = 0,
                                  },
                      .oneway   = 1,
                      .weight   = tonnes_to_weight(5),
                      .height   = metres_to_height(2.5),
                      .width    = metres_to_width (2),
                      .length   = metres_to_length(5),
                     },

  /* The profile for travel by HGV */

  [Transport_HGV] = {
                      .transport=Transport_HGV,
                      .allow    =Allow_HGV,
                      .highway  = {
                                   [Way_Motorway    ] = 100,
                                   [Way_Trunk       ] = 100,
                                   [Way_Primary     ] =  90,
                                   [Way_Secondary   ] =  80,
                                   [Way_Tertiary    ] =  70,
                                   [Way_Unclassified] =  60,
                                   [Way_Residential ] =  50,
                                   [Way_Service     ] =  80,
                                   [Way_Track       ] =   0,
                                   [Way_Cycleway    ] =   0,
                                   [Way_Path        ] =   0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = kph_to_speed(56*1.6),
                                   [Way_Trunk       ] = kph_to_speed(50*1.6),
                                   [Way_Primary     ] = kph_to_speed(50*1.6),
                                   [Way_Secondary   ] = kph_to_speed(50*1.6),
                                   [Way_Tertiary    ] = kph_to_speed(50*1.6),
                                   [Way_Unclassified] = kph_to_speed(40*1.6),
                                   [Way_Residential ] = kph_to_speed(30*1.6),
                                   [Way_Service     ] = kph_to_speed(20*1.6),
                                   [Way_Track       ] = kph_to_speed(10*1.6),
                                   [Way_Cycleway    ] = 0,
                                   [Way_Path        ] = 0,
                                  },
                      .oneway   = 1,
                      .weight   = tonnes_to_weight(10),
                      .height   = metres_to_height(3),
                      .width    = metres_to_width (2.5),
                      .length   = metres_to_length(6),
                     },

  /* The profile for travel by PSV */

  [Transport_PSV] = {
                      .transport=Transport_PSV,
                      .allow    =Allow_PSV,
                      .highway  = {
                                   [Way_Motorway    ] = 100,
                                   [Way_Trunk       ] = 100,
                                   [Way_Primary     ] =  90,
                                   [Way_Secondary   ] =  80,
                                   [Way_Tertiary    ] =  70,
                                   [Way_Unclassified] =  60,
                                   [Way_Residential ] =  50,
                                   [Way_Service     ] =  80,
                                   [Way_Track       ] =   0,
                                   [Way_Cycleway    ] =   0,
                                   [Way_Path        ] =   0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = kph_to_speed(56*1.6),
                                   [Way_Trunk       ] = kph_to_speed(50*1.6),
                                   [Way_Primary     ] = kph_to_speed(50*1.6),
                                   [Way_Secondary   ] = kph_to_speed(50*1.6),
                                   [Way_Tertiary    ] = kph_to_speed(50*1.6),
                                   [Way_Unclassified] = kph_to_speed(40*1.6),
                                   [Way_Residential ] = kph_to_speed(30*1.6),
                                   [Way_Service     ] = kph_to_speed(20*1.6),
                                   [Way_Track       ] = kph_to_speed(10*1.6),
                                   [Way_Cycleway    ] = 0,
                                   [Way_Path        ] = 0,
                                  },
                      .oneway   = 1,
                      .weight   = tonnes_to_weight(15),
                      .height   = metres_to_height(3),
                      .width    = metres_to_width (2.5),
                      .length   = metres_to_length(6),
                     },
 };


/*++++++++++++++++++++++++++++++++++++++
  Get the profile for a type of transport.

  Profile *GetProfile Returns a pointer to the profile.

  Transport transport The type of transport.
  ++++++++++++++++++++++++++++++++++++++*/

Profile *GetProfile(Transport transport)
{
 return(&builtin_profiles[transport]);
}


/*++++++++++++++++++++++++++++++++++++++
  Update a profile with highway preference scaling factor.

  Profile *profile The profile to be updated.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateProfile(Profile *profile)
{
 score_t hmax=0;
 int i;

 /* Normalise the highway preferences into the range 0 -> 1 */

 for(i=1;i<Way_Unknown;i++)
    if(profile->highway[i]>hmax)
       hmax=profile->highway[i];

 for(i=1;i<Way_Unknown;i++)
    if(profile->highway[i]>0)
       profile->highway[i]/=hmax;
    else
       profile->highway[i]=0;

 /* Find the fastest and most preferred highway type */

 profile->max_speed=0;

 for(i=0;i<Way_Unknown;i++)
    if(profile->speed[i]>profile->max_speed)
       profile->max_speed=profile->speed[i];

 profile->max_pref=0;

 for(i=0;i<Way_Unknown;i++)
    if(profile->highway[i]>profile->max_pref)
       profile->max_pref=profile->highway[i];
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

 for(i=1;i<Way_Unknown;i++)
    printf("Highway %-12s: %3d%%\n",HighwayName(i),(int)profile->highway[i]);

 printf("\n");

 for(i=1;i<Way_Unknown;i++)
    if(profile->highway[i])
       printf("Speed on %-12s: %3d km/h / %2.0f mph\n",HighwayName(i),profile->speed[i],(double)profile->speed[i]/1.6);

 printf("\n");

 printf("Obey one-way  : %s\n",profile->oneway?"yes":"no");
 printf("Minimum weight: %.1f tonnes\n",weight_to_tonnes(profile->weight));
 printf("Minimum height: %.1f metres\n",height_to_metres(profile->height));
 printf("Minimum width : %.1f metres\n",width_to_metres(profile->width));
 printf("Minimum length: %.1f metres\n",length_to_metres(profile->length));
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the profiles as Javascript for use in a web form.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesJS(void)
{
 unsigned int i,j;

 printf("// Transport types\n");
 printf("var router_transports={");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %d",j==1?"":", ",TransportName(j),j);
 printf("};\n");
 printf("\n");

 printf("// Highway types\n");
 printf("var router_highways={");
 for(i=1;i<Way_Unknown;i++)
    printf("%s%s: %d",i==1?"":", ",HighwayName(i),i);
 printf("};\n");
 printf("\n");

 printf("// Restriction types\n");
 printf("var router_restrictions={oneway: 1, weight: 2, height: 3, width: 4, length: 5};\n");
 printf("\n");

 printf("// Allowed highways\n");
 printf("var router_profile_highway={\n");
 for(i=1;i<Way_Unknown;i++)
   {
    printf("  %12s: {",HighwayName(i));
    for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
       printf("%s%s: %3d",j==1?"":", ",TransportName(j),(int)builtin_profiles[j].highway[i]);
    printf("}%s\n",i==(Way_Unknown-1)?"":",");
   }
 printf("   };\n");
 printf("\n");

 printf("// Speed limits\n");
 printf("var router_profile_speed={\n");
 for(i=1;i<Way_Unknown;i++)
   {
    printf("  %12s: {",HighwayName(i));
    for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
       printf("%s%s: %3d",j==1?"":", ",TransportName(j),builtin_profiles[j].speed[i]);
    printf("}%s\n",i==(Way_Unknown-1)?"":",");
   }
 printf("   };\n");
 printf("\n");

 printf("// Restrictions\n");
 printf("var router_profile_restrictions={\n");
 printf("  %12s: {","oneway");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %4d",j==1?"":", ",TransportName(j),builtin_profiles[j].oneway);
 printf("},\n");
 printf("  %12s: {","weight");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %4.1f",j==1?"":", ",TransportName(j),weight_to_tonnes(builtin_profiles[j].weight));
 printf("},\n");
 printf("  %12s: {","height");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %4.1f",j==1?"":", ",TransportName(j),height_to_metres(builtin_profiles[j].height));
 printf("},\n");
 printf("  %12s: {","width");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %4.1f",j==1?"":", ",TransportName(j),width_to_metres(builtin_profiles[j].width));
 printf("},\n");
 printf("  %12s: {","length");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %4.1f",j==1?"":", ",TransportName(j),length_to_metres(builtin_profiles[j].length));
 printf("}\n");
 printf("   };\n");
 printf("\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the profiles as Perl for use in a web CGI.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesPerl(void)
{
 unsigned int i,j;

 printf("# Transport types\n");
 printf("@router_transports=(");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s'%s'",j==1?"":", ",TransportName(j));
 printf(");\n");
 printf("\n");

 printf("# Highway types\n");
 printf("@router_highways=(");
 for(i=1;i<Way_Unknown;i++)
    printf("%s'%s'",i==1?"":", ",HighwayName(i));
 printf(");\n");
 printf("\n");

 printf("# Restriction types\n");
 printf("@router_restrictions=('oneway', 'weight', 'height', 'width', 'length');\n");
 printf("\n");

 printf("# Allowed highways\n");
 printf("%%router_profile_highway=(\n");
 for(i=1;i<Way_Unknown;i++)
   {
    printf("  %12s => {",HighwayName(i));
    for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
       printf("%s %s => %3d",j==1?"":", ",TransportName(j),(int)builtin_profiles[j].highway[i]);
    printf("}%s\n",i==(Way_Unknown-1)?"":",");
   }
 printf("   );\n");
 printf("\n");

 printf("# Speed limits\n");
 printf("%%router_profile_speed=(\n");
 for(i=1;i<Way_Unknown;i++)
   {
    printf("  %12s => {",HighwayName(i));
    for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
       printf("%s %s => %3d",j==1?"":", ",TransportName(j),builtin_profiles[j].speed[i]);
    printf("}%s\n",i==(Way_Unknown-1)?"":",");
   }
 printf("   );\n");
 printf("\n");

 printf("# Restrictions\n");
 printf("%%router_profile_restrictions=(\n");
 printf("  %12s => {","oneway");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %4d",j==1?"":", ",TransportName(j),builtin_profiles[j].oneway);
 printf("},\n");
 printf("  %12s => {","weight");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %4.1f",j==1?"":", ",TransportName(j),weight_to_tonnes(builtin_profiles[j].weight));
 printf("},\n");
 printf("  %12s => {","height");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %4.1f",j==1?"":", ",TransportName(j),height_to_metres(builtin_profiles[j].height));
 printf("},\n");
 printf("  %12s => {","width");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %4.1f",j==1?"":", ",TransportName(j),width_to_metres(builtin_profiles[j].width));
 printf("},\n");
 printf("  %12s => {","length");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %4.1f",j==1?"":", ",TransportName(j),length_to_metres(builtin_profiles[j].length));
 printf("}\n");
 printf("   );\n");
 printf("\n");
}
