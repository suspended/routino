/***************************************
 $Header: /home/amb/CVS/routino/src/waysx.c,v 1.14 2009-07-11 19:29:19 amb Exp $

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

/*+ The array size increment for WaysX (UK is ~1.3M raw ways, this is ~79 increments). +*/
#define INCREMENT_WAYSX (16*1024)

/*+ The array size increment for Ways (UK is ~240k compacted ways, this is ~59 increments). +*/
#define INCREMENT_WAYS   (4*1024)

/*+ The array size increment for names (UK is ~2.9MBytes of compacted names, this is ~22 increments). +*/
#define INCREMENT_NAMES (128*1024)


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

 waysx->row=-1;
 waysx->wrow=-1;

 return(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a way list.

  WaysX *waysx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeWayList(WaysX *waysx)
{
 if(waysx->xdata)
   {
    int i;
    for(i=0;i<(waysx->row*INCREMENT_WAYSX+waysx->col);i++)
       free(waysx->xdata[i/INCREMENT_WAYSX][i%INCREMENT_WAYSX].name);
    for(i=0;i<=waysx->row;i++)
       free(waysx->xdata[i]);
    free(waysx->xdata);
   }

 if(waysx->wxdata)
   {
    int i;
    for(i=0;i<=waysx->row;i++)
       free(waysx->wxdata[i]);
    free(waysx->wxdata);
   }

 if(waysx->ndata)
    free(waysx->ndata);
 if(waysx->idata)
    free(waysx->idata);

 if(waysx->wdata)
   {
    int i;
    for(i=0;i<=waysx->wrow;i++)
       free(waysx->wdata[i]);
    free(waysx->wdata);
   }

 if(waysx->names)
    free(waysx->names);

 free(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the way list to a file.

  WaysX* waysx The set of ways to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveWayList(WaysX* waysx,const char *filename)
{
 index_t i;
 int fd;
 Ways *ways=calloc(1,sizeof(Ways));

 assert(waysx->sorted);       /* Must be sorted */
 assert(waysx->wdata);        /* Must have wdata filled in */

 /* Fill in a Ways structure with the offset of the real data in the file after
    the Way structure itself. */

 ways->number=waysx->wrow*INCREMENT_WAYS+waysx->wcol;
 ways->data=NULL;
 ways->ways=(void*)sizeof(Ways);
 ways->names=(void*)(sizeof(Ways)+ways->number*sizeof(Way));

 /* Write out the Ways structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,ways,sizeof(Ways));

 for(i=0;i<ways->number;i++)
   {
    WriteFile(fd,&waysx->wdata[i/INCREMENT_WAYS][i%INCREMENT_WAYS],sizeof(Way));

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
 assert(!waysx->wdata);       /* Must not have wdata filled in */

 /* Check that the arrays have enough space. */

 if(waysx->row==-1 || waysx->col==INCREMENT_WAYSX)
   {
    waysx->row++;
    waysx->col=0;

    if((waysx->row%16)==0)
      {
       waysx->xdata=(WayX**)realloc((void*)waysx->xdata,(waysx->row+16)*sizeof(WayX*));
       waysx->wxdata=(Way**)realloc((void*)waysx->wxdata,(waysx->row+16)*sizeof(Way*));
      }

    waysx->xdata[waysx->row]=(WayX*)malloc(INCREMENT_WAYSX*sizeof(WayX));
    waysx->wxdata[waysx->row]=(Way*)malloc(INCREMENT_WAYSX*sizeof(Way));
   }

 /* Insert the way */

 waysx->xdata[waysx->row][waysx->col].id=id;
 waysx->xdata[waysx->row][waysx->col].name=strcpy((char*)malloc(strlen(name)+1),name);
 waysx->xdata[waysx->row][waysx->col].way=&waysx->wxdata[waysx->row][waysx->col];

 memset(&waysx->wxdata[waysx->row][waysx->col],0,sizeof(Way));

 waysx->col++;

 waysx->sorted=0;

 return(&waysx->wxdata[waysx->row][waysx->col-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of ways.

  WaysX* waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortWayList(WaysX* waysx)
{
 index_t i;
 int duplicate;

 assert(waysx->xdata);          /* Must have xdata filled in */
 assert(waysx->wxdata);         /* Must have wxdata filled in */
 assert(!waysx->wdata);         /* Must not have wdata filled in */

 printf("Sorting Ways"); fflush(stdout);

 /* Allocate the array of pointers and sort them */

 waysx->idata=(WayX**)malloc((waysx->row*INCREMENT_WAYSX+waysx->col)*sizeof(WayX*));

 do
   {
    waysx->number=0;

    for(i=0;i<(waysx->row*INCREMENT_WAYSX+waysx->col);i++)
       if(waysx->xdata[i/INCREMENT_WAYSX][i%INCREMENT_WAYSX].id!=NO_WAY)
         {
          waysx->idata[waysx->number]=&waysx->xdata[i/INCREMENT_WAYSX][i%INCREMENT_WAYSX];
          waysx->number++;
         }

    qsort(waysx->idata,waysx->number,sizeof(WayX*),(int (*)(const void*,const void*))sort_by_id);

    duplicate=0;

    for(i=1;i<waysx->number;i++)
      {
       if(waysx->idata[i]->id==waysx->idata[i-1]->id &&
          waysx->idata[i]->id!=NO_WAY)
         {
          waysx->idata[i-1]->id=NO_WAY;
          duplicate++;
         }
      }

    if(duplicate)
       printf(" - %d duplicates found; trying again.\nSorting Ways",duplicate); fflush(stdout);
   }
 while(duplicate);

 /* Sort the ways by name and way properties */

 waysx->ndata=(WayX**)malloc(waysx->number*sizeof(WayX*));

 for(i=0;i<waysx->number;i++)
    waysx->ndata[i]=waysx->idata[i];

 qsort(waysx->ndata,waysx->number,sizeof(WayX*),(int (*)(const void*,const void*))sort_by_name_and_properties);

 printf("\rSorted Ways \n"); fflush(stdout);

 waysx->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Compact the list of ways and reduce the names.

  WaysX* waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void CompactWays(WaysX* waysx)
{
 index_t i;

 assert(waysx->sorted);         /* Must be sorted */
 assert(waysx->ndata);          /* Must have ndata filled in */
 assert(waysx->wxdata);         /* Must have wxdata filled in */

 printf("Sorting Ways"); fflush(stdout);

 /* Allocate the new data for names */

 waysx->names=(char*)malloc(INCREMENT_NAMES*sizeof(char));

 /* Setup the offsets for the names in the way array */

 for(i=0;i<waysx->number;i++)
   {
    int copy=0;

    if(i && !strcmp(waysx->ndata[i]->name,waysx->ndata[i-1]->name)) /* Same name */
      {
       waysx->ndata[i]->way->name=waysx->ndata[i-1]->way->name;

       if(!WaysCompare(waysx->ndata[i]->way,waysx->ndata[i-1]->way)) /* Same properties */
          waysx->ndata[i]->way=waysx->ndata[i-1]->way;
       else
          copy=1;
      }
    else                                                          /* Different name */
      {
       if(((waysx->length+strlen(waysx->ndata[i]->name)+INCREMENT_NAMES)/INCREMENT_NAMES)>(waysx->length/INCREMENT_NAMES))
          waysx->names=(char*)realloc((void*)waysx->names,(waysx->length/INCREMENT_NAMES+1)*INCREMENT_NAMES*sizeof(char));

       strcpy(&waysx->names[waysx->length],waysx->ndata[i]->name);

       waysx->ndata[i]->way->name=waysx->length;

       waysx->length+=strlen(waysx->ndata[i]->name)+1;

       copy=1;
      }

    if(copy)
      {
       /* Check that the array has enough space. */

       if(waysx->wrow==-1 || waysx->wcol==INCREMENT_WAYS)
         {
          waysx->wrow++;
          waysx->wcol=0;

          if((waysx->wrow%16)==0)
             waysx->wdata=(Way **)realloc((void*)waysx->wdata,(waysx->wrow+16)*sizeof(Way*));

          waysx->wdata[waysx->wrow]=(Way *)malloc(INCREMENT_WAYSX*sizeof(Way));
         }

       waysx->wdata[waysx->wrow][waysx->wcol]=*waysx->ndata[i]->way;

       waysx->ndata[i]->way=&waysx->wdata[waysx->wrow][waysx->wcol];

       waysx->wcol++;
      }

    if(!((i+1)%10000))
      {
       printf("\rCompacting Ways: Ways=%d Compacted=%d Names=%d",i+1,waysx->wrow*INCREMENT_WAYS+waysx->wcol,waysx->length);
       fflush(stdout);
      }
   }

 printf("\rCompacted Ways: Ways=%d Compacted=%d Names=%d \n",waysx->number,waysx->wrow*INCREMENT_WAYS+waysx->wcol,waysx->length);
 fflush(stdout);

 for(i=0;i<=waysx->row;i++)
    free(waysx->wxdata[i]);
 free(waysx->wxdata);
 waysx->wxdata=NULL;
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


/*++++++++++++++++++++++++++++++++++++++
  Work out the index of the Way in the final data.

  index_t IndexWayInWaysX Returns the index of the Way.

  WaysX *waysx The Ways to use.

  WayX *wayx The extended way attached to the way.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexWayInWaysX(WaysX *waysx,WayX *wayx)
{
 int row;

 assert(waysx->wdata);          /* Must have wdata filled in */

 for(row=waysx->wrow;row>=0;row--)
    if((wayx->way-waysx->wdata[row])>=0 &&
       (wayx->way-waysx->wdata[row])<INCREMENT_WAYSX)
       break;

 return(row*INCREMENT_WAYS+(off_t)(wayx->way-waysx->wdata[row]));
}
