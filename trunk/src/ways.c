/***************************************
 $Header: /home/amb/CVS/routino/src/ways.c,v 1.22 2009-02-04 18:26:29 amb Exp $

 Way data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "ways.h"


/* Constants */

/*+ The array size increment for ways - expect ~1,000,000 ways. +*/
#define INCREMENT_WAYS 256*1024


/* Functions */

static int sort_by_index(WayX *a,WayX *b);
static int sort_by_name(WayX *a,WayX *b);


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
  Allocate a new way list.

  WaysX *NewWayList Returns the way list.
  ++++++++++++++++++++++++++++++++++++++*/

WaysX *NewWayList(void)
{
 WaysX *waysx;

 waysx=(WaysX*)malloc(sizeof(WaysX));

 waysx->sorted=0;
 waysx->alloced=INCREMENT_WAYS;
 waysx->number=0;
 waysx->length=0;

 waysx->xdata=(WayX*)malloc(waysx->alloced*sizeof(WayX));

 return(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the way list to a file.

  WaysX* waysx The set of ways to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveWayList(WaysX* waysx,const char *filename)
{
 int i;
 int fd;
 Ways *ways=calloc(1,sizeof(Ways));

 assert(waysx->sorted);       /* Must be sorted */

 /* Fill in a Ways structure with the offset of the real data in the file after
    the Way structure itself. */

 ways->number=waysx->number;
 ways->data=NULL;
 ways->ways=(void*)sizeof(Ways);
 ways->names=(void*)sizeof(Ways)+ways->number*sizeof(Way);

 /* Write out the Ways structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,ways,sizeof(Ways));

 for(i=0;i<waysx->number;i++)
    WriteFile(fd,&waysx->xdata[i].way,sizeof(Way));

 WriteFile(fd,waysx->names,waysx->length);

 CloseFile(fd);

 /* Free the fake Ways */

 free(ways);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a way to a way list.

  Way *AppendWay Returns the newly appended way.

  WaysX* waysx The set of ways to process.

  const char *name The name or reference of the way.
  ++++++++++++++++++++++++++++++++++++++*/

Way *AppendWay(WaysX* waysx,const char *name)
{
 assert(!waysx->sorted);      /* Must not be sorted */

 waysx->sorted=0;

 /* Check that the array has enough space. */

 if(waysx->number==waysx->alloced)
   {
    waysx->alloced+=INCREMENT_WAYS;

    waysx->xdata=(WayX*)realloc((void*)waysx->xdata,waysx->alloced*sizeof(WayX));
   }

 /* Insert the way */

 waysx->xdata[waysx->number].name=strcpy((char*)malloc(strlen(name)+1),name);

 waysx->xdata[waysx->number].index=waysx->number;

 waysx->number++;

 return(&waysx->xdata[waysx->number-1].way);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of ways and fix the names.

  WaysX* waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortWayList(WaysX* waysx)
{
 int i;

 assert(!waysx->sorted);      /* Must not be sorted */

 waysx->sorted=1;

 /* Sort the ways by name */

 qsort(waysx->xdata,waysx->number,sizeof(WayX),(int (*)(const void*,const void*))sort_by_name);

 /* Allocate the new data */

 waysx->names=(char*)malloc(waysx->alloced*sizeof(char));

 /* Setup the offsets for the names in the way array */

 for(i=0;i<waysx->number;i++)
   {
    if(i && !strcmp(waysx->xdata[i].name,waysx->xdata[i-1].name)) /* Same name */
       waysx->xdata[i].way.name=waysx->xdata[i-1].way.name;
    else                                                              /* Different name */
      {
       if((waysx->length+strlen(waysx->xdata[i].name)+1)>=waysx->alloced)
         {
          waysx->alloced+=INCREMENT_WAYS;

          waysx->names=(char*)realloc((void*)waysx->names,waysx->alloced*sizeof(char));
         }

       strcpy(&waysx->names[waysx->length],waysx->xdata[i].name);

       waysx->xdata[i].way.name=waysx->length;

       waysx->length+=strlen(waysx->xdata[i].name)+1;
      }
   }

 /* Sort the ways by id */

 qsort(waysx->xdata,waysx->number,sizeof(WayX),(int (*)(const void*,const void*))sort_by_index);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into index order.

  int sort_by_index Returns the comparison of the index fields.

  WayX *a The first WayX.

  WayX *b The second WayX.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_index(WayX *a,WayX *b)
{
 index_t a_index=a->index;
 index_t b_index=b->index;

 if(a_index<b_index)
    return(-1);
 else
    return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into name order.

  int sort_by_name Returns the comparison of the name fields.

  Way *a The first Way.

  Way *b The second Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name(WayX *a,WayX *b)
{
 char *a_name=a->name;
 char *b_name=b->name;

 return(strcmp(a_name,b_name));
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

   case 'm':
    if(!strncmp(highway,"motorway",8)) return(Way_Motorway);
    if(!strcmp(highway,"minor")) return(Way_Unclassified);
    return(Way_Unknown);

   case 'p':
    if(!strncmp(highway,"primary",7)) return(Way_Primary);
    if(!strcmp(highway,"path")) return(Way_Footway);
    if(!strcmp(highway,"pedestrian")) return(Way_Footway);
    return(Way_Unknown);

   case 'r':
    if(!strcmp(highway,"road")) return(Way_Unclassified);
    if(!strcmp(highway,"residential")) return(Way_Residential);
    return(Way_Unknown);

   case 's':
    if(!strncmp(highway,"secondary",9)) return(Way_Secondary);
    if(!strcmp(highway,"service")) return(Way_Service);
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
