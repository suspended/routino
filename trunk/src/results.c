/***************************************
 $Header: /home/amb/CVS/routino/src/results.c,v 1.13 2009-05-13 17:45:31 amb Exp $

 Result data type functions.

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
#include <string.h>
#include <stdlib.h>

#include "results.h"


#define RESULTS_INCREMENT    16
#define QUEUE_INCREMENT   10240


/*+ A queue of results. +*/
typedef struct _Queue
{
 uint32_t  alloced;             /*+ The amount of space allocated for results in the array. +*/
 uint32_t  number;              /*+ The number of occupied results in the array. +*/
 Result  **xqueue;              /*+ An array of pointers to parts of the results structure. +*/
}
 Queue;

/*+ The queue of nodes. +*/
static Queue queue={0,0,NULL};


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *NewResultsList Returns the results list.

  int nbins The number of bins in the results array.
  ++++++++++++++++++++++++++++++++++++++*/

Results *NewResultsList(int nbins)
{
 Results *results;
 int i;

 results=(Results*)malloc(sizeof(Results));

 results->nbins=1;
 results->mask=~0;

 while(nbins>>=1)
   {
    results->mask<<=1;
    results->nbins<<=1;
   }

 results->mask=~results->mask;

 results->alloced=RESULTS_INCREMENT;
 results->number=0;

 results->count=(uint32_t*)malloc(results->nbins*sizeof(uint32_t));
 results->point=(Result***)malloc(results->nbins*sizeof(Result**));

 for(i=0;i<results->nbins;i++)
   {
    results->count[i]=0;

    results->point[i]=(Result**)malloc(results->alloced*sizeof(Result*));
   }

 results->data=(Result**)malloc(1*sizeof(Result*));
 results->data[0]=(Result*)malloc(results->nbins*RESULTS_INCREMENT*sizeof(Result));

 results->start=~0;
 results->finish=~0;

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *results The results list to be destroyed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeResultsList(Results *results)
{
 int c=(results->number-1)/(results->nbins*RESULTS_INCREMENT);
 int i;

 for(i=c;i>=0;i--)
    free(results->data[i]);

 free(results->data);

 for(i=0;i<results->nbins;i++)
    free(results->point[i]);

 free(results->point);

 free(results->count);

 free(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert a new result into the results data structure in the right order.

  Result *insert_result Returns the result that has been inserted.

  Results *results The results structure to insert into.

  index_t node The node that is to be inserted into the results.
  ++++++++++++++++++++++++++++++++++++++*/

Result *InsertResult(Results *results,index_t node)
{
 int bin=node&results->mask;
 int i;

 /* Check that the arrays have enough space. */

 if(results->count[bin]==results->alloced)
   {
    results->alloced+=RESULTS_INCREMENT;

    for(i=0;i<results->nbins;i++)
       results->point[i]=(Result**)realloc((void*)results->point[i],results->alloced*sizeof(Result*));
   }

 if(results->number && (results->number%RESULTS_INCREMENT)==0 && (results->number%(RESULTS_INCREMENT*results->nbins))==0)
   {
    int c=results->number/(results->nbins*RESULTS_INCREMENT);

    results->data=(Result**)realloc((void*)results->data,(c+1)*sizeof(Result*));
    results->data[c]=(Result*)malloc(results->nbins*RESULTS_INCREMENT*sizeof(Result));
   }

 /* Insert the new entry */

 results->point[bin][results->count[bin]]=&results->data[results->number/(results->nbins*RESULTS_INCREMENT)][results->number%(results->nbins*RESULTS_INCREMENT)];

 results->number++;

 results->count[bin]++;

 results->point[bin][results->count[bin]-1]->node=node;

 return(results->point[bin][results->count[bin]-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Zero the values in a result structure.

  Result *result The result to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void ZeroResult(Result *result)
{
 result->prev=0;
 result->next=0;

 result->score=0;
 result->sortby=0;

 result->segment=NULL;
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result; search by node.

  Result *insert_result Returns the result that has been found.

  Results *results The results structure to search.

  index_t node The node that is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FindResult(Results *results,index_t node)
{
 int bin=node&results->mask;
 int i;

 for(i=results->count[bin]-1;i>=0;i--)
    if(results->point[bin][i]->node==node)
       return(results->point[bin][i]);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result from a set of results.

  Result *FirstResult Returns the first results from a set of results.

  Results *results The set of results.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FirstResult(Results *results)
{
 return(&results->data[0][0]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result from a set of results.

  Result *NextResult Returns the next result from a set of results.

  Results *results The set of results.

  Result *result The previous result.
  ++++++++++++++++++++++++++++++++++++++*/

Result *NextResult(Results *results,Result *result)
{
 int c=(results->number-1)/(results->nbins*RESULTS_INCREMENT);
 int i,j=0;

 for(i=0;i<=c;i++)
   {
    j=(result-results->data[i]);

    if(j>=0 && j<(results->nbins*RESULTS_INCREMENT))
       break;
   }

 if(++j>=(results->nbins*RESULTS_INCREMENT))
   {i++;j=0;}

 if((i*(results->nbins*RESULTS_INCREMENT)+j)>=results->number)
    return(NULL);

 return(&results->data[i][j]);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert an item into the queue in the right order.

  Result *result The result to insert into the queue.
  ++++++++++++++++++++++++++++++++++++++*/

void insert_in_queue(Result *result)
{
 int start=0;
 int end=queue.number-1;
 int mid;
 int insert=-1;

 /* Check that the array is allocated. */

 if(!queue.xqueue)
   {
    queue.alloced=QUEUE_INCREMENT;
    queue.number=0;
    queue.xqueue=(Result**)malloc(queue.alloced*sizeof(Result*));
   }

 /* Check that the arrays have enough space. */

 if(queue.number==queue.alloced)
   {
    queue.alloced+=QUEUE_INCREMENT;
    queue.xqueue=(Result**)realloc((void*)queue.xqueue,queue.alloced*sizeof(Result*));
   }

 /* Binary search - search key may not match, new insertion point required
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since there may not be an exact match we must set end=mid
  *  # <- mid    |  or start=mid because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually end=start+1 and the insertion point is before
  *  # <- end    |  end (since it cannot be before the initial start or end).
  */

 if(queue.number==0)                                 /* There is nothing in the queue */
    insert=0;
 else if(result->sortby>queue.xqueue[start]->sortby) /* Check key is not before start */
    insert=start;
 else if(result->sortby<queue.xqueue[end]->sortby)   /* Check key is not after end */
    insert=end+1;
 else if(queue.number==2)                            /* Must be between them */
    insert=1;
 else
   {
    do
      {
       mid=(start+end)/2;                                /* Choose mid point */

       if(queue.xqueue[mid]->sortby>result->sortby)      /* Mid point is too low */
          start=mid;
       else if(queue.xqueue[mid]->sortby<result->sortby) /* Mid point is too high */
          end=mid;
       else                                              /* Mid point is correct */
         {
          if(queue.xqueue[mid]==result)
             return;

          insert=mid;
          break;
         }
      }
    while((end-start)>1);

    if(insert==-1)
       insert=end;
   }

 /* Shuffle the array up */

 if(insert!=queue.number)
    memmove(&queue.xqueue[insert+1],&queue.xqueue[insert],(queue.number-insert)*sizeof(Result*));

 /* Insert the new entry */

 queue.xqueue[insert]=result;

 queue.number++;
}


/*++++++++++++++++++++++++++++++++++++++
  Pop an item from the end of the queue.

  Result *pop_from_queue Returns the top item.

  Results *results The set of results that the queue is processing.
  ++++++++++++++++++++++++++++++++++++++*/

Result *pop_from_queue()
{
 if(queue.number)
    return(queue.xqueue[--queue.number]);
 else
    return(NULL);
}
