/***************************************
 $Header: /home/amb/CVS/routino/src/profiles.c,v 1.6 2009-02-27 20:16:47 amb Exp $

 The pre-defined profiles and the functions for handling them.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
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
                                   [Way_Bridleway   ] = 1,
                                   [Way_Cycleway    ] = 1,
                                   [Way_Footway     ] = 1,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = 4,
                                   [Way_Primary     ] = 4,
                                   [Way_Secondary   ] = 4,
                                   [Way_Tertiary    ] = 4,
                                   [Way_Unclassified] = 4,
                                   [Way_Residential ] = 4,
                                   [Way_Service     ] = 4,
                                   [Way_Track       ] = 4,
                                   [Way_Bridleway   ] = 4,
                                   [Way_Cycleway    ] = 4,
                                   [Way_Footway     ] = 4,
                                  },
                      .oneway   = 0,
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
                                   [Way_Bridleway   ] = 1,
                                   [Way_Cycleway    ] = 1,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = 0,
                                   [Way_Primary     ] =20,
                                   [Way_Secondary   ] =20,
                                   [Way_Tertiary    ] =20,
                                   [Way_Unclassified] =20,
                                   [Way_Residential ] =20,
                                   [Way_Service     ] =20,
                                   [Way_Track       ] =20,
                                   [Way_Bridleway   ] =20,
                                   [Way_Cycleway    ] =20,
                                   [Way_Footway     ] = 0,
                                  },
                      .oneway   = 1,
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
                                   [Way_Bridleway   ] = 1,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] = 0,
                                   [Way_Trunk       ] = 0,
                                   [Way_Primary     ] =10,
                                   [Way_Secondary   ] =10,
                                   [Way_Tertiary    ] =10,
                                   [Way_Unclassified] =10,
                                   [Way_Residential ] =10,
                                   [Way_Service     ] =10,
                                   [Way_Track       ] =10,
                                   [Way_Bridleway   ] =10,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .oneway   = 1,
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
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] =70*1.6,
                                   [Way_Trunk       ] =60*1.6,
                                   [Way_Primary     ] =60*1.6,
                                   [Way_Secondary   ] =55*1.6,
                                   [Way_Tertiary    ] =50*1.6,
                                   [Way_Unclassified] =40*1.6,
                                   [Way_Residential ] =30*1.6,
                                   [Way_Service     ] =20*1.6,
                                   [Way_Track       ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .oneway   = 1,
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
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] =70*1.6,
                                   [Way_Trunk       ] =60*1.6,
                                   [Way_Primary     ] =60*1.6,
                                   [Way_Secondary   ] =55*1.6,
                                   [Way_Tertiary    ] =50*1.6,
                                   [Way_Unclassified] =40*1.6,
                                   [Way_Residential ] =30*1.6,
                                   [Way_Service     ] =20*1.6,
                                   [Way_Track       ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .oneway   = 1,
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
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] =60*1.6,
                                   [Way_Trunk       ] =60*1.6,
                                   [Way_Primary     ] =60*1.6,
                                   [Way_Secondary   ] =55*1.6,
                                   [Way_Tertiary    ] =50*1.6,
                                   [Way_Unclassified] =40*1.6,
                                   [Way_Residential ] =30*1.6,
                                   [Way_Service     ] =20*1.6,
                                   [Way_Track       ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .oneway   = 1,
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
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] =56*1.6,
                                   [Way_Trunk       ] =50*1.6,
                                   [Way_Primary     ] =50*1.6,
                                   [Way_Secondary   ] =50*1.6,
                                   [Way_Tertiary    ] =50*1.6,
                                   [Way_Unclassified] =40*1.6,
                                   [Way_Residential ] =30*1.6,
                                   [Way_Service     ] =20*1.6,
                                   [Way_Track       ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .oneway   = 1,
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
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .speed    = {
                                   [Way_Motorway    ] =56*1.6,
                                   [Way_Trunk       ] =50*1.6,
                                   [Way_Primary     ] =50*1.6,
                                   [Way_Secondary   ] =50*1.6,
                                   [Way_Tertiary    ] =50*1.6,
                                   [Way_Unclassified] =40*1.6,
                                   [Way_Residential ] =30*1.6,
                                   [Way_Service     ] =20*1.6,
                                   [Way_Track       ] = 0,
                                   [Way_Bridleway   ] = 0,
                                   [Way_Cycleway    ] = 0,
                                   [Way_Footway     ] = 0,
                                  },
                      .oneway   = 1,
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

 printf("Ignore one-way: %s\n",profile->oneway?"no":"yes");
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
 printf("router_highways={\n");
 for(i=1;i<Way_Unknown;i++)
    printf("%s%s: %d",i==1?"":",",HighwayName(i),i-1);
 printf("};\n");
 printf("\n");

 printf("// Allowed highways\n");
 printf("router_profile_highway={\n");
 for(i=1;i<Way_Unknown;i++)
   {
    printf("  %16s: {",HighwayName(i));
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
    printf("  %16s: {",HighwayName(i));
    for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
       printf("%s%s: %3d",j==1?"":",",TransportName(j),builtin_profiles[j].speed[i]);
    printf("}%s\n",i==(Way_Unknown-1)?"":",");
   }
 printf("   };\n");
 printf("\n");

 printf("// Must obey oneway\n");
 printf("router_profile_oneway={");
 for(j=1;j<sizeof(builtin_profiles)/sizeof(builtin_profiles[0]);j++)
    printf("%s%s: %d",j==1?"":",",TransportName(j),builtin_profiles[j].oneway);
 printf("};\n");
 printf("\n");
}
