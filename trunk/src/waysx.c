/***************************************
 $Header: /home/amb/CVS/routino/src/waysx.c,v 1.3 2009-03-01 17:23:30 amb Exp $

 Extended Way data type functions.
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
#include "waysx.h"


/* Constants */

/*+ The array size increment for ways - expect ~1,000,000 ways. +*/
#define INCREMENT_WAYS 256*1024


/* Functions */

static int sort_by_name(WayX **a,WayX **b);


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

 waysx->idata=(WayX*)malloc(waysx->alloced*sizeof(WayX));
 waysx->ndata=NULL;

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
   {
    WriteFile(fd,&waysx->idata[i].way,sizeof(Way));

    if(!((i+1)%10000))
      {
       printf("\rWriting Ways: Ways=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rWrote Ways: Ways=%d  \n",waysx->number);
 fflush(stdout);

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

    waysx->idata=(WayX*)realloc((void*)waysx->idata,waysx->alloced*sizeof(WayX));
   }

 /* Insert the way */

 waysx->idata[waysx->number].name=strcpy((char*)malloc(strlen(name)+1),name);

 memset(&waysx->idata[waysx->number].way,0,sizeof(Way));

 waysx->number++;

 return(&waysx->idata[waysx->number-1].way);
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

 printf("Sorting Ways"); fflush(stdout);

 /* Sort the ways by name */

 waysx->ndata=malloc(waysx->number*sizeof(WayX*));

 for(i=0;i<waysx->number;i++)
    waysx->ndata[i]=&waysx->idata[i];

 qsort(waysx->ndata,waysx->number,sizeof(WayX*),(int (*)(const void*,const void*))sort_by_name);

 /* Allocate the new data */

 waysx->names=(char*)malloc(waysx->alloced*sizeof(char));

 /* Setup the offsets for the names in the way array */

 for(i=0;i<waysx->number;i++)
   {
    if(i && !strcmp(waysx->ndata[i]->name,waysx->ndata[i-1]->name)) /* Same name */
       waysx->ndata[i]->way.name=waysx->ndata[i-1]->way.name;
    else                                                          /* Different name */
      {
       if((waysx->length+strlen(waysx->ndata[i]->name)+1)>=waysx->alloced)
         {
          waysx->alloced+=INCREMENT_WAYS;

          waysx->names=(char*)realloc((void*)waysx->names,waysx->alloced*sizeof(char));
         }

       strcpy(&waysx->names[waysx->length],waysx->ndata[i]->name);

       waysx->ndata[i]->way.name=waysx->length;

       waysx->length+=strlen(waysx->ndata[i]->name)+1;
      }
   }

 printf("\rSorted Ways \n"); fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into name order.

  int sort_by_name Returns the comparison of the name fields.

  Way **a The first Way.

  Way **b The second Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name(WayX **a,WayX **b)
{
 char *a_name=(*a)->name;
 char *b_name=(*b)->name;

 return(strcmp(a_name,b_name));
}


/*++++++++++++++++++++++++++++++++++++++
  Return 1 if the two ways are the same (in respect of their types and limits).

  int WaysSame Returns a comparison.

  Way *way1 The first way.

  Way *way2 The second way.
  ++++++++++++++++++++++++++++++++++++++*/

int WaysSame(Way *way1,Way *way2)
{
 if(way1->type  ==way2->type   &&
    way1->allow ==way2->allow  &&
    way1->speed ==way2->speed  &&
    way1->weight==way2->weight &&
    way1->height==way2->height &&
    way1->width ==way2->width  &&
    way1->length==way2->length)
    return(1);
 else
    return(0);
}
