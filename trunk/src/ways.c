/***************************************
 $Header: /home/amb/CVS/routino/src/ways.c,v 1.29 2009-05-31 12:30:12 amb Exp $

 Way data type functions.

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


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "ways.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a way list from a file.

  Ways* LoadWayList Returns the way list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Ways *LoadWayList(const char *filename)
{
 void *data;
 Ways *ways;

 ways=(Ways*)malloc(sizeof(Ways));

 data=MapFile(filename);

 if(!data)
    return(NULL);

 /* Copy the Ways structure from the loaded data */

 *ways=*((Ways*)data);

 /* Adjust the pointers in the Ways structure. */

 ways->data =data;
 ways->ways =(Way *)(data+(off_t)ways->ways);
 ways->names=(char*)(data+(off_t)ways->names);

 return(ways);
}


/*++++++++++++++++++++++++++++++++++++++
  Decide on the type of a way given the "highway" parameter.

  Highway HighwayType Returns the highway type of the way.

  const char *highway The string containing the type of the way.
  ++++++++++++++++++++++++++++++++++++++*/

Highway HighwayType(const char *highway)
{
 switch(*highway)
   {
   case 'b':
    if(!strcmp(highway,"byway")) return(Way_Track);
    if(!strcmp(highway,"bridleway")) return(Way_Bridleway);
    return(Way_Unknown);

   case 'c':
    if(!strcmp(highway,"cycleway")) return(Way_Cycleway);
    return(Way_Unknown);

   case 'f':
    if(!strcmp(highway,"footway")) return(Way_Footway);
    return(Way_Unknown);

   case 'l':
    if(!strcmp(highway,"living_street")) return(Way_Residential);
    return(Way_Unknown);

   case 'm':
    if(!strncmp(highway,"motorway",8)) return(Way_Motorway);
    if(!strcmp(highway,"minor")) return(Way_Unclassified);
    return(Way_Unknown);

   case 'p':
    if(!strncmp(highway,"primary",7)) return(Way_Primary);
    if(!strcmp(highway,"path")) return(Way_Path);
    if(!strcmp(highway,"pedestrian")) return(Way_Footway);
    return(Way_Unknown);

   case 'r':
    if(!strcmp(highway,"road")) return(Way_Unclassified);
    if(!strcmp(highway,"residential")) return(Way_Residential);
    return(Way_Unknown);

   case 's':
    if(!strncmp(highway,"secondary",9)) return(Way_Secondary);
    if(!strcmp(highway,"service")) return(Way_Service);
    if(!strcmp(highway,"services")) return(Way_Service);
    if(!strcmp(highway,"steps")) return(Way_Footway);
    return(Way_Unknown);

   case 't':
    if(!strncmp(highway,"trunk",5)) return(Way_Trunk);
    if(!strcmp(highway,"tertiary")) return(Way_Tertiary);
    if(!strcmp(highway,"track")) return(Way_Track);
    return(Way_Unknown);

   case 'u':
    if(!strcmp(highway,"unclassified")) return(Way_Unclassified);
    if(!strcmp(highway,"unsurfaced")) return(Way_Track);
    if(!strcmp(highway,"unpaved")) return(Way_Track);
    return(Way_Unknown);

   case 'w':
    if(!strcmp(highway,"walkway")) return(Way_Footway);
    return(Way_Unknown);

   default:
    ;
   }

 return(Way_Unknown);
}


/*++++++++++++++++++++++++++++++++++++++
  Decide on the allowed type of transport given the name of it.

  Transport TransportType Returns the type of the transport.

  const char *transport The string containing the method of transport.
  ++++++++++++++++++++++++++++++++++++++*/

Transport TransportType(const char *transport)
{
 switch(*transport)
   {
   case 'b':
    if(!strcmp(transport,"bicycle"))
       return(Transport_Bicycle);
    break;

   case 'f':
    if(!strcmp(transport,"foot"))
       return(Transport_Foot);
    break;

   case 'g':
    if(!strcmp(transport,"goods"))
       return(Transport_Goods);
    break;

   case 'h':
    if(!strcmp(transport,"horse"))
       return(Transport_Horse);
    if(!strcmp(transport,"hgv"))
       return(Transport_HGV);
    break;

   case 'm':
    if(!strcmp(transport,"motorbike"))
       return(Transport_Motorbike);
    if(!strcmp(transport,"motorcar"))
       return(Transport_Motorcar);
    break;

   case 'p':
    if(!strcmp(transport,"psv"))
       return(Transport_PSV);
    break;

   default:
    return(Transport_None);
   }

 return(Transport_None);
}


/*++++++++++++++++++++++++++++++++++++++
  A string containing the name of a type of highway.

  const char *HighwayName Returns the name.

  Highway highway The highway type.
  ++++++++++++++++++++++++++++++++++++++*/

const char *HighwayName(Highway highway)
{
 switch(highway)
   {
   case Way_Motorway:
    return("motorway");
   case Way_Trunk:
    return("trunk");
   case Way_Primary:
    return("primary");
   case Way_Secondary:
    return("secondary");
   case Way_Tertiary:
    return("tertiary");
   case Way_Unclassified:
    return("unclassified");
   case Way_Residential:
    return("residential");
   case Way_Service:
    return("service");
   case Way_Track:
    return("track");
   case Way_Path:
    return("path");
   case Way_Bridleway:
    return("bridleway");
   case Way_Cycleway:
    return("cycleway");
   case Way_Footway:
    return("footway");

   case Way_Unknown:
   case Way_OneWay:
   case Way_Roundabout:
    return(NULL);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  A string containing the name of a type of transport.

  const char *TransportName Returns the name.

  Transport transport The transport type.
  ++++++++++++++++++++++++++++++++++++++*/

const char *TransportName(Transport transport)
{
 switch(transport)
   {
   case Transport_None:
    return("NONE");

   case Transport_Foot:
    return("foot");
   case Transport_Bicycle:
    return("bicycle");
   case Transport_Horse:
    return("horse");
   case Transport_Motorbike:
    return("motorbike");
   case Transport_Motorcar:
    return("motorcar");
   case Transport_Goods:
    return("goods");
   case Transport_HGV:
    return("hgv");
   case Transport_PSV:
    return("psv");
  }
 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Returns a list of all the highway types.

  const char *HighwayList Return a list of all the highway types.
  ++++++++++++++++++++++++++++++++++++++*/

const char *HighwayList(void)
{
 return "    motorway     = Motorway\n"
        "    trunk        = Trunk\n"
        "    primary      = Primary\n"
        "    secondary    = Secondary\n"
        "    tertiary     = Tertiary\n"
        "    unclassified = Unclassified\n"
        "    residential  = Residential\n"
        "    service      = Service\n"
        "    track        = Track\n"
        "    path         = Path\n"
        "    bridleway    = Bridleway\n"
        "    cycleway     = Cycleway\n"
        "    footway      = Footway\n";
}


/*++++++++++++++++++++++++++++++++++++++
  Returns a list of all the transport types.

  const char *TransportList Return a list of all the transport types.
  ++++++++++++++++++++++++++++++++++++++*/

const char *TransportList(void)
{
 return "    foot      = Foot\n"
        "    bicycle   = Bicycle\n"
        "    horse     = Horse\n"
        "    motorbike = Motorbike\n"
        "    motorcar  = Motorcar\n"
        "    goods     = Goods     (Small lorry, van)\n"
        "    hgv       = HGV       (Heavy Goods Vehicle - large lorry)\n"
        "    psv       = PSV       (Public Service Vehicle - bus, coach)\n";
}


/*++++++++++++++++++++++++++++++++++++++
  Return 1 if the two ways are the same (in respect of their types and limits).

  int WaysSame Returns a comparison.

  Way *way1 The first way.

  Way *way2 The second way.
  ++++++++++++++++++++++++++++++++++++++*/

int WaysSame(Way *way1,Way *way2)
{
 if(way1==way2)
    return(1);

 if(way1->type  ==way2->type   &&
    way1->allow ==way2->allow  &&
    way1->speed ==way2->speed  &&
    way1->weight==way2->weight &&
    way1->height==way2->height &&
    way1->width ==way2->width  &&
    way1->length==way2->length)
    return(1);

 return(0);
}
