/***************************************
 $Header: /home/amb/CVS/routino/src/ways.c,v 1.14 2009-01-25 10:58:52 amb Exp $

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


/*+ A temporary variable used when sorting +*/
static char **sort_names;

/* Functions */

static int sort_by_id(Way *a,Way *b);
static int sort_by_name(Way *a,Way *b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new way list.

  WaysMem *NewWayList Returns the way list.
  ++++++++++++++++++++++++++++++++++++++*/

WaysMem *NewWayList(void)
{
 WaysMem *ways;

 ways=(WaysMem*)malloc(sizeof(WaysMem));

 ways->alloced=INCREMENT_WAYS;
 ways->number=0;
 ways->number_str=0;
 ways->sorted=0;

 ways->ways=(Ways*)malloc(sizeof(Ways)+ways->alloced*sizeof(Way));
 ways->names=(char**)malloc(ways->alloced*sizeof(char*));

 return(ways);
}


/*++++++++++++++++++++++++++++++++++++++
  Load in a way list from a file.

  Ways* LoadWayList Returns the way list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Ways *LoadWayList(const char *filename)
{
 return((Ways*)MapFile(filename));
}


/*++++++++++++++++++++++++++++++++++++++
  Save the way list to a file.

  Ways* SaveWayList Returns the way list that has just been saved.

  WaysMem* ways The set of ways to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

Ways *SaveWayList(WaysMem* ways,const char *filename)
{
 assert(ways->sorted);          /* Must be sorted */

 if(WriteFile(filename,(void*)ways->ways,sizeof(Ways)-sizeof(ways->ways->ways)+(ways->number+ways->number_str)*sizeof(Way)))
    assert(0);

 free(ways->names);
 free(ways->ways);
 free(ways);

 return(LoadWayList(filename));
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular way.

  Way *FindWay Returns a pointer to the way with the specified id.

  Ways* ways The set of ways to process.

  way_t id The way id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

Way *FindWay(Ways* ways,way_t id)
{
 int bin=id%NBINS_WAYS;
 int start=ways->offset[bin];
 int end=ways->offset[bin+1]-1; /* &offset[NBINS+1] == &number */
 int mid;

 /* Binary search - search key exact match only is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 if(end<start)                    /* There are no ways */
    return(NULL);
 else if(id<ways->ways[start].id) /* Check key is not before start */
    return(NULL);
 else if(id>ways->ways[end].id)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;             /* Choose mid point */

       if(ways->ways[mid].id<id)      /* Mid point is too low */
          start=mid+1;
       else if(ways->ways[mid].id>id) /* Mid point is too high */
          end=mid-1;
       else                           /* Mid point is correct */
          return(&ways->ways[mid]);
      }
    while((end-start)>1);

    if(ways->ways[start].id==id)      /* Start is correct */
       return(&ways->ways[start]);

    if(ways->ways[end].id==id)        /* End is correct */
       return(&ways->ways[end]);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a way to a newly created way list (unsorted).

  Way *AppendWay Returns the newly appended way.

  WaysMem* ways The set of ways to process.

  way_t id The way identification.

  const char *name The name or reference of the way.

  speed_t speed The speed on the way.
  ++++++++++++++++++++++++++++++++++++++*/

Way *AppendWay(WaysMem* ways,way_t id,const char *name)
{
 /* Check that the array has enough space. */

 if(ways->number==ways->alloced)
   {
    ways->alloced+=INCREMENT_WAYS;

    ways->ways=(Ways*)realloc((void*)ways->ways,sizeof(Ways)+ways->alloced*sizeof(Way));
    ways->names=(char**)realloc((void*)ways->names,ways->alloced*sizeof(char*));
   }

 assert(!ways->sorted);         /* Must be sorted */

 /* Insert the way */

 ways->names[ways->number]=strcpy((char*)malloc(strlen(name)+1),name);

 ways->ways->ways[ways->number].id=id;
 ways->ways->ways[ways->number].name=ways->number;

 ways->number++;

 ways->sorted=0;

 return(&ways->ways->ways[ways->number-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the way list.

  WaysMem* ways The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortWayList(WaysMem* ways)
{
 char *name=NULL;
 int bin=0;
 int i;

 /* Sort the ways by name */

 sort_names=ways->names;

 qsort(ways->ways->ways,ways->number,sizeof(Way),(int (*)(const void*,const void*))sort_by_name);

 /* Setup the offsets for the names in the way array */

 for(i=0;i<ways->number;i++)
   {
    if(name && !strcmp(name,ways->names[ways->ways->ways[i].name])) /* Same name */
      {
       free(ways->names[ways->ways->ways[i].name]);
       ways->ways->ways[i].name=ways->ways->ways[i-1].name;
      }
    else                                            /* Different name */
      {
       name=ways->names[ways->ways->ways[i].name];

       if((ways->number+ways->number_str+strlen(name)/sizeof(Way)+1)>=ways->alloced)
         {
          ways->alloced+=INCREMENT_WAYS;

          ways->ways=(Ways*)realloc((void*)ways->ways,sizeof(Ways)+ways->alloced*sizeof(Way));
         }

       strcpy((char*)&ways->ways->ways[ways->number+ways->number_str],name);
       free(name);

       ways->ways->ways[i].name=ways->number+ways->number_str;
       name=(char*)&ways->ways->ways[ways->ways->ways[i].name];

       ways->number_str+=strlen(name)/sizeof(Way)+1;
      }
   }

 /* Sort the ways by id */

 qsort(ways->ways->ways,ways->number,sizeof(Way),(int (*)(const void*,const void*))sort_by_id);

 while(ways->ways->ways[ways->number-1].id==~0)
    ways->number--;

 ways->sorted=1;

 /* Make it searchable */

 ways->ways->number=ways->number;

 for(i=0;i<ways->number;i++)
    for(;bin<=(ways->ways->ways[i].id%NBINS_WAYS);bin++)
       ways->ways->offset[bin]=i;

 for(;bin<NBINS_WAYS;bin++)
    ways->ways->offset[bin]=ways->number;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into id order.

  int sort_by_id Returns the comparison of the id fields.

  Way *a The first Way.

  Way *b The second Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(Way *a,Way *b)
{
 way_t a_id=a->id;
 way_t b_id=b->id;

 int a_bin=a->id%NBINS_WAYS;
 int b_bin=b->id%NBINS_WAYS;

 if(a_bin!=b_bin)
    return(a_bin-b_bin);

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else /* if(a_id==b_id) */
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into name order.

  int sort_by_name Returns the comparison of the name fields.

  Way *a The first Way.

  Way *b The second Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name(Way *a,Way *b)
{
 char *a_name=sort_names[a->name];
 char *b_name=sort_names[b->name];

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
    if(!strcmp(highway,"minor")) return(Way_Unclassfied);
    return(Way_Unknown);

   case 'p':
    if(!strncmp(highway,"primary",7)) return(Way_Primary);
    if(!strcmp(highway,"path")) return(Way_Footway);
    if(!strcmp(highway,"pedestrian")) return(Way_Footway);
    return(Way_Unknown);

   case 'r':
    if(!strcmp(highway,"road")) return(Way_Unclassfied);
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
    if(!strcmp(highway,"unclassified")) return(Way_Unclassfied);
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
   case Way_Unclassfied:
    return("unclassfied");
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
 return "    motorway     = Motorway    \n"
        "    trunk        = Trunk       \n"
        "    primary      = Primary     \n"
        "    secondary    = Secondary   \n"
        "    tertiary     = Tertiary    \n"
        "    unclassified = Unclassified\n"
        "    residential  = Residential \n"
        "    service      = Service     \n"
        "    track        = Track       \n"
        "    bridleway    = Bridleway   \n"
        "    cycleway     = Cycleway    \n"
        "    footway      = Footway     \n";
}


/*++++++++++++++++++++++++++++++++++++++
  Returns a list of all the transport types.

  const char *TransportList Return a list of all the transport types.
  ++++++++++++++++++++++++++++++++++++++*/

const char *TransportList(void)
{
 return "    foot      = Foot     \n"
        "    bicycle   = Bicycle  \n"
        "    horse     = Horse    \n"
        "    motorbike = Motorbike\n"
        "    motorcar  = Motorcar \n"
        "    goods     = Goods    \n"
        "    hgv       = HGV      \n"
        "    psv       = PSV      \n";
}
