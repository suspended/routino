/***************************************
 $Header: /home/amb/CVS/routino/src/profiles.c,v 1.11 2009-04-08 16:54:34 amb Exp $

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


static Profile builtin_profiles[]=
 {
  /* The profile for travel by Foot */

  [Transport_Foot] = {
                      .transport=Transport_Foot,
                      .allow    =Allow_Foot,
                      .highways = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = 1,
                                   [Way_Primary     ] = 1,
                                   [Way_Secondary   ] = 1,
                                   [Way_Tertiary    ] = 1,
                                   [Way_Unclassified] = 1,
                                   [Way_Residential ] = 1,
                                   [Way_Service     ] = 1,
                                   [Way_Track       ] = 1,
                                   [Way_Path        ] = 1,
                                   [Way_Bridleway   ] = 1,
                                   [Way_Cycleway    ] = 1,
                                   [Way_Footway     ] = 1,
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
                                   [Way_Path        ] = kph_to_speed(4),
                                   [Way_Bridleway   ] = kph_to_speed(4),
                                   [Way_Cycleway    ] = kph_to_speed(4),
                                   [Way_Footway     ] = kph_to_speed(4),
                                  },
                      .oneway   = 0,
                      .weight   = 0,
                      .height   = 0,
                      .width    = 0,
                      .length   = 0,
                     },

  /* The profile for travel by Bicycle */

  [Transport_Bicycle] = {
                      .transport=Transport_Bicycle,
                      .allow    =Allow_Bicycle,
                      .highways = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = 0,
                                   [Way_Primary     ] = 1,
                                   [Way_Secondary   ] = 1,
                                   [Way_Tertiary    ] = 1,
                                   [Way_Unclassified] = 1,
                                   [Way_Residential ] = 1,
                                   [Way_Service     ] = 1,
                                   [Way_Track       ] = 1,
                                   [Way_Path        ] = 1,
                                   [Way_Bridleway   ] = 1,
                                   [Way_Cycleway    ] = 1,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = 0,
                                   [Way_Primary     ] = kph_to_speed(20),
                                   [Way_Secondary   ] = kph_to_speed(20),
                                   [Way_Tertiary    ] = kph_to_speed(20),
                                   [Way_Unclassified] = kph_to_speed(20),
                                   [Way_Residential ] = kph_to_speed(20),
                                   [Way_Service     ] = kph_to_speed(20),
                                   [Way_Track       ] = kph_to_speed(20),
                                   [Way_Path        ] = kph_to_speed(20),
                                   [Way_Bridleway   ] = kph_to_speed(20),
                                   [Way_Cycleway    ] = kph_to_speed(20),
                                   [Way_Footway     ] = 0,
                                  },
                      .oneway   = 1,
                      .weight   = 0,
                      .height   = 0,
                      .width    = 0,
                      .length   = 0,
                     },

  /* The profile for travel by Horse */

  [Transport_Horse] = {
                      .transport=Transport_Horse,
                      .allow    =Allow_Horse,
                      .highways = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = 1,
                                   [Way_Primary     ] = 1,
                                   [Way_Secondary   ] = 1,
                                   [Way_Tertiary    ] = 1,
                                   [Way_Unclassified] = 1,
                                   [Way_Residential ] = 1,
                                   [Way_Service     ] = 1,
                                   [Way_Track       ] = 1,
                                   [Way_Path        ] = 1,
                                   [Way_Bridleway   ] = 1,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = 0,
                                   [Way_Primary     ] = kph_to_speed(10),
                                   [Way_Secondary   ] = kph_to_speed(10),
                                   [Way_Tertiary    ] = kph_to_speed(10),
                                   [Way_Unclassified] = kph_to_speed(10),
                                   [Way_Residential ] = kph_to_speed(10),
                                   [Way_Service     ] = kph_to_speed(10),
                                   [Way_Track       ] = kph_to_speed(10),
                                   [Way_Path        ] = kph_to_speed(10),
                                   [Way_Bridleway   ] = kph_to_speed(10),
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                      .highways = {
                                   [Way_Motorway    ] = 1,
                                   [Way_Trunk       ] = 1,
                                   [Way_Primary     ] = 1,
                                   [Way_Secondary   ] = 1,
                                   [Way_Tertiary    ] = 1,
                                   [Way_Unclassified] = 1,
                                   [Way_Residential ] = 1,
                                   [Way_Service     ] = 1,
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                      .highways = {
                                   [Way_Motorway    ] = 1,
                                   [Way_Trunk       ] = 1,
                                   [Way_Primary     ] = 1,
                                   [Way_Secondary   ] = 1,
                                   [Way_Tertiary    ] = 1,
                                   [Way_Unclassified] = 1,
                                   [Way_Residential ] = 1,
                                   [Way_Service     ] = 1,
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                      .highways = {
                                   [Way_Motorway    ] = 1,
                                   [Way_Trunk       ] = 1,
                                   [Way_Primary     ] = 1,
                                   [Way_Secondary   ] = 1,
                                   [Way_Tertiary    ] = 1,
                                   [Way_Unclassified] = 1,
                                   [Way_Residential ] = 1,
                                   [Way_Service     ] = 1,
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                      .highways = {
                                   [Way_Motorway    ] = 1,
                                   [Way_Trunk       ] = 1,
                                   [Way_Primary     ] = 1,
                                   [Way_Secondary   ] = 1,
                                   [Way_Tertiary    ] = 1,
                                   [Way_Unclassified] = 1,
                                   [Way_Residential ] = 1,
                                   [Way_Service     ] = 1,
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                      .highways = {
                                   [Way_Motorway    ] = 1,
                                   [Way_Trunk       ] = 1,
                                   [Way_Primary     ] = 1,
                                   [Way_Secondary   ] = 1,
                                   [Way_Tertiary    ] = 1,
                                   [Way_Unclassified] = 1,
                                   [Way_Residential ] = 1,
                                   [Way_Service     ] = 1,
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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
                                   [Way_Track       ] = 0,
                                   [Way_Path        ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
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

 for(i=1;i<Way_Unknown;i++)
    printf("Highway %-12s: %s\n",HighwayName(i),profile->highways[i]?"yes":"no");

 printf("\n");

 for(i=1;i<Way_Unknown;i++)
    if(profile->highways[i])
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
 int i,j;

 printf("// Transport types\n");
 printf("router_transports={");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %d",j==1?"":",",TransportName(j),j-1);
 printf("};\n");
 printf("\n");

 printf("// Highway types\n");
 printf("router_highways={");
 for(i=1;i<Way_Unknown;i++)
    printf("%s%s: %d",i==1?"":",",HighwayName(i),i-1);
 printf("};\n");
 printf("\n");

 printf("// Restriction types\n");
 printf("router_restrictions={oneway: 1, weight: 2, height: 3, width: 4, length: 5};\n");
 printf("\n");

 printf("// Allowed highways\n");
 printf("router_profile_highway={\n");
 for(i=1;i<Way_Unknown;i++)
   {
    printf("  %12s: {",HighwayName(i));
    for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
       printf("%s%s: %d",j==1?"":",",TransportName(j),builtin_profiles[j].highways[i]);
    printf("}%s\n",i==(Way_Unknown-1)?"":",");
   }
 printf("   };\n");
 printf("\n");

 printf("// Speed limits\n");
 printf("router_profile_speed={\n");
 for(i=1;i<Way_Unknown;i++)
   {
    printf("  %12s: {",HighwayName(i));
    for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
       printf("%s%s: %3d",j==1?"":",",TransportName(j),builtin_profiles[j].speed[i]);
    printf("}%s\n",i==(Way_Unknown-1)?"":",");
   }
 printf("   };\n");
 printf("\n");

 printf("// Restrictions\n");
 printf("router_profile_restrictions={\n");
 printf("oneway: {");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %d",j==1?"":",",TransportName(j),builtin_profiles[j].oneway);
 printf("},\n");
 printf("  %12s: {","weight");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %.1f",j==1?"":",",TransportName(j),weight_to_tonnes(builtin_profiles[j].weight));
 printf("},\n");
 printf("  %12s: {","height");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %.1f",j==1?"":",",TransportName(j),height_to_metres(builtin_profiles[j].height));
 printf("},\n");
 printf("  %12s: {","width");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %.1f",j==1?"":",",TransportName(j),width_to_metres(builtin_profiles[j].width));
 printf("},\n");
 printf("  %12s: {","length");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %.1f",j==1?"":",",TransportName(j),length_to_metres(builtin_profiles[j].length));
 printf("}\n");
 printf("};\n");
 printf("\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the profiles as Perl for use in a web CGI.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintProfilesPerl(void)
{
 int i,j;

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
       printf("%s %s => %d",j==1?"":",",TransportName(j),builtin_profiles[j].highways[i]);
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
       printf("%s %s => %3d",j==1?"":",",TransportName(j),builtin_profiles[j].speed[i]);
    printf("}%s\n",i==(Way_Unknown-1)?"":",");
   }
 printf("   );\n");
 printf("\n");

 printf("# Restrictions\n");
 printf("%%router_profile_restrictions=(\n");
 printf("oneway => {");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %d",j==1?"":",",TransportName(j),builtin_profiles[j].oneway);
 printf("},\n");
 printf("  %12s => {","weight");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %.1f",j==1?"":",",TransportName(j),weight_to_tonnes(builtin_profiles[j].weight));
 printf("},\n");
 printf("  %12s => {","height");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %.1f",j==1?"":",",TransportName(j),height_to_metres(builtin_profiles[j].height));
 printf("},\n");
 printf("  %12s => {","width");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %.1f",j==1?"":",",TransportName(j),width_to_metres(builtin_profiles[j].width));
 printf("},\n");
 printf("  %12s => {","length");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s %s => %.1f",j==1?"":",",TransportName(j),length_to_metres(builtin_profiles[j].length));
 printf("}\n");
 printf(");\n");
 printf("\n");
}
