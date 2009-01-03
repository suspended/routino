/***************************************
 $Header: /home/amb/CVS/routino/src/ways.c,v 1.1 2009-01-03 12:25:36 amb Exp $

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
#include "types.h"

#define INCREMENT 256*1024

/*+ The list of ways +*/
Ways *OSMWays=NULL;

/*+ Is the data sorted and therefore searchable? +*/
static int sorted=0;

/*+ The list of names for the ways before they are merged into the main data structure. +*/
static char **names=NULL;

/* Functions */

static void sort_way_list(void);
static int sort_by_id(Way *a,Way *b);
static int sort_by_name(Way *a,Way *b);


/*++++++++++++++++++++++++++++++++++++++
  Load in a way list from a file.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

int LoadWayList(const char *filename)
{
 OSMWays=(Ways*)MapFile(filename);

 if(OSMWays)
    sorted=1;

 return(!OSMWays);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the way list to a file.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

int SaveWayList(const char *filename)
{
 int retval;
 size_t alloced;

 if(!sorted)
    sort_way_list();

 alloced=OSMWays->alloced;
 OSMWays->alloced=OSMWays->number+OSMWays->number_str;

 retval=WriteFile(filename,OSMWays,sizeof(Ways)-sizeof(OSMWays->ways)+(OSMWays->number+OSMWays->number_str)*sizeof(Way));

 OSMWays->alloced=alloced;

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular way.

  Way *FindWay Returns a pointer to the way with the specified id.

  way_t id The way id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

Way *FindWay(way_t id)
{
 int start=0;
 int end=OSMWays->number-1;
 int mid;

 if(!sorted)
    sort_way_list();

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

 if(OSMWays->number==0)              /* There are no ways */
    return(NULL);
 else if(id<OSMWays->ways[start].id) /* Check key is not before start */
    return(NULL);
 else if(id>OSMWays->ways[end].id)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                /* Choose mid point */

       if(OSMWays->ways[mid].id<id)      /* Mid point is too low */
          start=mid+1;
       else if(OSMWays->ways[mid].id>id) /* Mid point is too high */
          end=mid-1;
       else                              /* Mid point is correct */
          return(&OSMWays->ways[mid]);
      }
    while((end-start)>1);

    if(OSMWays->ways[start].id==id)      /* Start is correct */
       return(&OSMWays->ways[start]);

    if(OSMWays->ways[end].id==id)        /* End is correct */
       return(&OSMWays->ways[end]);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Return the name of the way.

  const char *WayName Returns the name.

  Way *way The way whose name is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

const char *WayName(Way *way)
{
 assert(sorted);

 return((char*)&OSMWays->ways[(off_t)way->name]);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a way to a newly created way list (unsorted).

  way_t id The way identification.

  const char *name The name or reference of the way.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendWay(way_t id,const char *name)
{
 assert(!sorted);

 /* Check that the whole thing is allocated. */

 if(!OSMWays)
   {
    OSMWays=(Ways*)malloc(sizeof(Ways));

    OSMWays->alloced=sizeof(OSMWays->ways)/sizeof(OSMWays->ways[0]);
    OSMWays->number=0;
    OSMWays->number_str=0;

    names=(char**)malloc(OSMWays->alloced*sizeof(char*));
   }

 /* Check that the arrays have enough space. */

 if(OSMWays->number==OSMWays->alloced)
   {
    OSMWays=(Ways*)realloc((void*)OSMWays,sizeof(Ways)-sizeof(OSMWays->ways)+(OSMWays->alloced+INCREMENT)*sizeof(Way));

    OSMWays->alloced+=INCREMENT;

    names=(char**)realloc((void*)names,OSMWays->alloced*sizeof(char*));
   }

 /* Insert the way */

 names[OSMWays->number]=strcpy((char*)malloc(strlen(name)+1),name);

 OSMWays->ways[OSMWays->number].id=id;
 OSMWays->ways[OSMWays->number].name=names[OSMWays->number];

 OSMWays->number++;

 sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the way list.
  ++++++++++++++++++++++++++++++++++++++*/

static void sort_way_list(void)
{
 char *name=NULL;
 int i;

 /* Sort the ways by name */

 qsort(OSMWays->ways,OSMWays->number,sizeof(Way),(int (*)(const void*,const void*))sort_by_name);

 for(i=0;i<OSMWays->number;i++)
   {
    if(name && !strcmp(name,OSMWays->ways[i].name)) /* Same name */
      {
       free(OSMWays->ways[i].name);
       OSMWays->ways[i].name=OSMWays->ways[i-1].name;
      }
    else                                            /* Different name */
      {
       name=OSMWays->ways[i].name;

       if((OSMWays->number+OSMWays->number_str+strlen(name)/sizeof(Way)+1)>=OSMWays->alloced)
         {
          OSMWays=(Ways*)realloc((void*)OSMWays,sizeof(Ways)-sizeof(OSMWays->ways)+(OSMWays->alloced+INCREMENT)*sizeof(Way));

          OSMWays->alloced+=INCREMENT;
         }

       strcpy((char*)&OSMWays->ways[OSMWays->number+OSMWays->number_str],name);
       free(name);

       OSMWays->ways[i].name=(char*)OSMWays->number+OSMWays->number_str;
       name=(char*)&OSMWays->ways[(off_t)OSMWays->ways[i].name];

       OSMWays->number_str+=strlen(name)/sizeof(Way)+1;
      }
   }

 /* Sort the ways by id */

 qsort(OSMWays->ways,OSMWays->number,sizeof(Way),(int (*)(const void*,const void*))sort_by_id);

 sorted=1;
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
 char *a_name=a->name;
 char *b_name=b->name;

 return(strcmp(a_name,b_name));
}
