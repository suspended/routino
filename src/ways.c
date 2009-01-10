/***************************************
 $Header: /home/amb/CVS/routino/src/ways.c,v 1.5 2009-01-10 15:59:59 amb Exp $

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
#ifdef NBINS_WAYS
 int i,bin=0;
#endif

 assert(ways->sorted);          /* Must be sorted */

 ways->ways->number=ways->number;

#ifdef NBINS_WAYS
 for(i=0;i<ways->number;i++)
    for(;bin<=(ways->ways->ways[i].id%NBINS_WAYS);bin++)
       ways->ways->offset[bin]=i;

 for(;bin<=NBINS_WAYS;bin++)
    ways->ways->offset[bin]=ways->number;
#endif

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
#ifdef NBINS_WAYS
 int bin=id%NBINS_WAYS;
 int start=ways->offset[bin];
 int end=ways->offset[bin+1]-1;
#else
 int start=0;
 int end=ways->number-1;
#endif
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

 ways->sorted=1;
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

#ifdef NBINS_WAYS
 int a_bin=a->id%NBINS_WAYS;
 int b_bin=b->id%NBINS_WAYS;

 if(a_bin!=b_bin)
    return(a_bin-b_bin);
#endif

 return(a_id-b_id);
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
  Return the name of the way.

  const char *WayName Returns the name.

  Ways* ways The set of ways to process.

  Way *way The way whose name is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

const char *WayName(Ways* ways,Way *way)
{
 return((char*)&ways->ways[way->name]);
}


/*++++++++++++++++++++++++++++++++++++++
  Decide on the type of a way given the "highway" parameter.

  WayType TypeOfWay Returns the type of the way.

  const char *type The string containing the type of the way.
  ++++++++++++++++++++++++++++++++++++++*/

WayType TypeOfWay(const char *type)
{
 switch(*type)
   {
   case 'b':
    if(!strcmp(type,"byway")) return(Way_Track);
    if(!strcmp(type,"bridleway")) return(Way_Bridleway);
    return(Way_Unknown);

   case 'c':
    if(!strcmp(type,"cycleway")) return(Way_Cycleway);
    return(Way_Unknown);

   case 'f':
    if(!strcmp(type,"footway")) return(Way_Footway);
    return(Way_Unknown);

   case 'm':
    if(!strncmp(type,"motorway",8)) return(Way_Motorway);
    if(!strcmp(type,"minor")) return(Way_Unclassfied);
    return(Way_Unknown);

   case 'p':
    if(!strncmp(type,"primary",7)) return(Way_Primary);
    if(!strcmp(type,"path")) return(Way_Footway);
    if(!strcmp(type,"pedestrian")) return(Way_Footway);
    return(Way_Unknown);

   case 'r':
    if(!strcmp(type,"road")) return(Way_Unclassfied);
    if(!strcmp(type,"residential")) return(Way_Residential);
    return(Way_Unknown);

   case 's':
    if(!strncmp(type,"secondary",9)) return(Way_Secondary);
    if(!strcmp(type,"service")) return(Way_Service);
    if(!strcmp(type,"steps")) return(Way_Footway);
    return(Way_Unknown);

   case 't':
    if(!strncmp(type,"trunk",5)) return(Way_Trunk);
    if(!strcmp(type,"tertiary")) return(Way_Tertiary);
    if(!strcmp(type,"track")) return(Way_Track);
    return(Way_Unknown);

   case 'u':
    if(!strcmp(type,"unclassified")) return(Way_Unclassfied);
    if(!strcmp(type,"unsurfaced")) return(Way_Track);
    if(!strcmp(type,"unpaved")) return(Way_Track);
    return(Way_Unknown);

   case 'w':
    if(!strcmp(type,"walkway")) return(Way_Footway);
    return(Way_Unknown);

   default:
    ;
   }

 return(Way_Unknown);
}
