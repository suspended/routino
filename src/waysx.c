/***************************************
 $Header: /home/amb/CVS/routino/src/waysx.c,v 1.9 2009-07-02 16:33:31 amb Exp $

 Extended Way data type functions.

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
#include "waysx.h"


/* Constants */

/*+ The array size increment for ways - expect ~1,000,000 ways. +*/
#define INCREMENT_WAYS 256*1024


/* Functions */

static int sort_by_id(WayX **a,WayX **b);
static int sort_by_name_and_properties(WayX **a,WayX **b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new way list.

  WaysX *NewWayList Returns the way list.
  ++++++++++++++++++++++++++++++++++++++*/

WaysX *NewWayList(void)
{
 WaysX *waysx;

 waysx=(WaysX*)calloc(1,sizeof(WaysX));

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
 assert(waysx->wdata);        /* Must have wdata filled in */

 /* Fill in a Ways structure with the offset of the real data in the file after
    the Way structure itself. */

 ways->number=waysx->wnumber;
 ways->data=NULL;
 ways->ways=(void*)sizeof(Ways);
 ways->names=(void*)sizeof(Ways)+ways->number*sizeof(Way);

 /* Write out the Ways structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,ways,sizeof(Ways));

 for(i=0;i<ways->number;i++)
   {
    WriteFile(fd,&waysx->wdata[i],sizeof(Way));

    if(!((i+1)%10000))
      {
       printf("\rWriting Ways: Ways=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rWrote Ways: Ways=%d  \n",ways->number);
 fflush(stdout);

 WriteFile(fd,waysx->names,waysx->length);

 CloseFile(fd);

 /* Free the fake Ways */

 free(ways);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular way.

  WayX *FindWayX Returns the extended way with the specified id.

  WaysX* waysx The set of ways to process.

  way_t id The way id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

WayX *FindWayX(WaysX* waysx,way_t id)
{
 int start=0;
 int end=waysx->number-1;
 int mid;

 assert(waysx->sorted);        /* Must be sorted */
 assert(waysx->idata);         /* Must have idata filled in */

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

 if(end<start)                       /* There are no ways */
    return(NULL);
 else if(id<waysx->idata[start]->id) /* Check key is not before start */
    return(NULL);
 else if(id>waysx->idata[end]->id)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                /* Choose mid point */

       if(waysx->idata[mid]->id<id)      /* Mid point is too low */
          start=mid+1;
       else if(waysx->idata[mid]->id>id) /* Mid point is too high */
          end=mid-1;
       else                              /* Mid point is correct */
          return(waysx->idata[mid]);
      }
    while((end-start)>1);

    if(waysx->idata[start]->id==id)      /* Start is correct */
       return(waysx->idata[start]);

    if(waysx->idata[end]->id==id)        /* End is correct */
       return(waysx->idata[end]);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a way to a way list.

  Way *AppendWay Returns the newly appended way.

  WaysX* waysx The set of ways to process.

  way_t id The ID of the way.

  const char *name The name or reference of the way.
  ++++++++++++++++++++++++++++++++++++++*/

Way *AppendWay(WaysX* waysx,way_t id,const char *name)
{
 /* Check that the arrays have enough space. */

 if(waysx->number==waysx->alloced)
   {
    waysx->alloced+=INCREMENT_WAYS;

    waysx->xdata=(WayX*)realloc((void*)waysx->xdata,waysx->alloced*sizeof(WayX));

    waysx->wdata=(Way*)realloc((void*)waysx->wdata,waysx->alloced*sizeof(Way));
   }

 /* Insert the way */

 waysx->xdata[waysx->number].id=id;
 waysx->xdata[waysx->number].name=strcpy((char*)malloc(strlen(name)+1),name);
 waysx->xdata[waysx->number].way=&waysx->wdata[waysx->number];

 memset(&waysx->wdata[waysx->number],0,sizeof(Way));

 waysx->number++;

 return(&waysx->wdata[waysx->number-1]);

 waysx->sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of ways and fix the names.

  WaysX* waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortWayList(WaysX* waysx)
{
 Way *uniq_ways;
 int i,j;

 assert(waysx->xdata);         /* Must have xdata filled in */

 printf("Sorting Ways"); fflush(stdout);

 /* Sort the ways by id */

 waysx->idata=malloc(waysx->number*sizeof(WayX*));

 for(i=0;i<waysx->number;i++)
    waysx->idata[i]=&waysx->xdata[i];

 qsort(waysx->idata,waysx->number,sizeof(WayX*),(int (*)(const void*,const void*))sort_by_id);

 /* Sort the ways by name and way properties */

 waysx->ndata=malloc(waysx->number*sizeof(WayX*));

 for(i=0;i<waysx->number;i++)
    waysx->ndata[i]=&waysx->xdata[i];

 qsort(waysx->ndata,waysx->number,sizeof(WayX*),(int (*)(const void*,const void*))sort_by_name_and_properties);

 /* Allocate the new data for names */

 waysx->names=(char*)malloc(waysx->alloced*sizeof(char));

 /* Allocate the data for unique ways */

 uniq_ways=(Way*)malloc(waysx->number*sizeof(Way));

 /* Setup the offsets for the names in the way array */

 for(i=0,j=0;i<waysx->number;i++)
   {
    if(i && !strcmp(waysx->ndata[i]->name,waysx->ndata[i-1]->name)) /* Same name */
      {
       waysx->ndata[i]->way->name=waysx->ndata[i-1]->way->name;

       if(!WaysCompare(waysx->ndata[i]->way,waysx->ndata[i-1]->way)) /* Same properties */
          waysx->ndata[i]->way=waysx->ndata[i-1]->way;
       else
         {
          uniq_ways[j]=*waysx->ndata[i]->way;
          waysx->ndata[i]->way=&uniq_ways[j++];
         }
      }
    else                                                          /* Different name */
      {
       if((waysx->length+strlen(waysx->ndata[i]->name)+1)>=waysx->alloced)
         {
          waysx->alloced+=INCREMENT_WAYS;

          waysx->names=(char*)realloc((void*)waysx->names,waysx->alloced*sizeof(char));
         }

       strcpy(&waysx->names[waysx->length],waysx->ndata[i]->name);

       waysx->ndata[i]->way->name=waysx->length;

       waysx->length+=strlen(waysx->ndata[i]->name)+1;

       uniq_ways[j]=*waysx->ndata[i]->way;
       waysx->ndata[i]->way=&uniq_ways[j++];
      }
   }

 waysx->wnumber=j;

 free(waysx->wdata);

 waysx->wdata=uniq_ways;

 printf("\rSorted Ways \n"); fflush(stdout);

 waysx->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into id order.

  int sort_by_id Returns the comparison of the id fields.

  Way **a The first Way.

  Way **b The second Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(WayX **a,WayX **b)
{
 way_t a_id=(*a)->id;
 way_t b_id=(*b)->id;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into name and properties order.

  int sort_by_name_and_properties Returns the comparison of the name and properties fields.

  Way **a The first Way.

  Way **b The second Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name_and_properties(WayX **a,WayX **b)
{
 char *a_name=(*a)->name;
 char *b_name=(*b)->name;
 int name_compare;

 name_compare=strcmp(a_name,b_name);

 if(name_compare)
    return(name_compare);
 else
   {
    Way *a_way=(*a)->way;
    Way *b_way=(*b)->way;

    return(WaysCompare(a_way,b_way));
   }
}
