/***************************************
 $Header: /home/amb/CVS/routino/src/results.c,v 1.4 2009-01-23 15:22:31 amb Exp $

 Result data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "results.h"


#define RESULTS_INCREMENT   16
#define QUEUE_INCREMENT   1024


/*+ A queue of results. +*/
typedef struct _Queue
{
 uint32_t alloced;              /*+ The amount of space allocated for results in the array. +*/
 uint32_t number;               /*+ The number of occupied results in the array. +*/
 uint32_t *queue;               /*+ An array of offsets into the results array. +*/
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

 results->numbin=(uint32_t*)malloc(results->nbins*sizeof(uint32_t));
 results->offsets=(uint32_t**)malloc(results->nbins*sizeof(uint32_t*));

 for(i=0;i<results->nbins;i++)
   {
    results->numbin[i]=0;

    results->offsets[i]=(uint32_t*)malloc(results->alloced*sizeof(uint32_t));
   }

 results->results=(Result*)malloc(results->alloced*results->nbins*sizeof(Result));

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *results The results list to be destroyed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeResultsList(Results *results)
{
 int i;

 free(results->results);

 for(i=0;i<results->nbins;i++)
    free(results->offsets[i]);

 free(results->offsets);
 free(results->numbin);

 free(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert a new result into the results data structure in the right order.

  Result *insert_result Returns the result that has been inserted.

  Results *results The results structure to insert into.

  node_t node The node that is to be inserted into the results.
  ++++++++++++++++++++++++++++++++++++++*/

Result *InsertResult(Results *results,node_t node)
{
 int bin=node&results->mask;
 int start=0;
 int end=results->numbin[bin]-1;
 int i;
 int mid;
 int insert=-1;

 /* Check that the arrays have enough space. */

 if(results->numbin[bin]==results->alloced)
   {
    results->alloced+=RESULTS_INCREMENT;

    for(i=0;i<results->nbins;i++)
       results->offsets[i]=(uint32_t*)realloc((void*)results->offsets[i],results->alloced*sizeof(uint32_t));

    results->results=(Result*)realloc((void*)results->results,results->alloced*results->nbins*sizeof(Result));
   }

 /* Binary search - search key may not match, if not then insertion point required
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since there may not be an exact match we must set end=mid
  *  # <- mid    |  or start=mid because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually end=start+1 and the insertion point is before
  *  # <- end    |  end (since it cannot be before the initial start or end).
  */

 if(end<start)                                                     /* There are no results */
    insert=start;
 else if(node<results->results[results->offsets[bin][start]].node) /* Check key is not before start */
    insert=start;
 else if(node>results->results[results->offsets[bin][end]].node)   /* Check key is not after end */
    insert=end+1;
 else
   {
    do
      {
       mid=(start+end)/2;                                              /* Choose mid point */

       if(results->results[results->offsets[bin][mid]].node<node)      /* Mid point is too low */
          start=mid;
       else if(results->results[results->offsets[bin][mid]].node>node) /* Mid point is too high */
          end=mid;
       else
          assert(0);
      }
    while((end-start)>1);

    insert=end;
   }

 /* Shuffle the array up */

 if(insert!=results->numbin[bin])
    memmove(&results->offsets[bin][insert+1],&results->offsets[bin][insert],(results->numbin[bin]-insert)*sizeof(uint32_t));

 /* Insert the new entry */

 results->offsets[bin][insert]=results->number;

 results->number++;

 results->numbin[bin]++;

 return(&results->results[results->offsets[bin][insert]]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result, ordered by node.

  Result *insert_result Returns the result that has been found.

  Results *results The results structure to search.

  node_t node The node that is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FindResult(Results *results,node_t node)
{
 int bin=node&results->mask;
 int start=0;
 int end=results->numbin[bin]-1;
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

 if(end<start)                                                     /* There are no results */
    return(NULL);
 else if(node<results->results[results->offsets[bin][start]].node) /* Check key is not before start */
    return(NULL);
 else if(node>results->results[results->offsets[bin][end]].node)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                                              /* Choose mid point */

       if(results->results[results->offsets[bin][mid]].node<node)      /* Mid point is too low */
          start=mid+1;
       else if(results->results[results->offsets[bin][mid]].node>node) /* Mid point is too high */
          end=mid-1;
       else                                                            /* Mid point is correct */
          return(&results->results[results->offsets[bin][mid]]);
      }
    while((end-start)>1);

    if(results->results[results->offsets[bin][start]].node==node)      /* Start is correct */
       return(&results->results[results->offsets[bin][start]]);

    if(results->results[results->offsets[bin][end]].node==node)        /* End is correct */
       return(&results->results[results->offsets[bin][end]]);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert an item into the queue in the right order.

  Results *results The set of results.

  Result *result The result to insert into the queue.
  ++++++++++++++++++++++++++++++++++++++*/

void insert_in_queue(Results *results,Result *result)
{
 int start=0;
 int end=queue.number-1;
 int mid;
 int insert=-1;

 /* Check that the whole thing is allocated. */

 if(!queue.queue)
   {
    queue.alloced=QUEUE_INCREMENT;
    queue.number=0;
    queue.queue=(uint32_t*)malloc(queue.alloced*sizeof(uint32_t));
   }

 /* Check that the arrays have enough space. */

 if(queue.number==queue.alloced)
   {
    queue.alloced+=QUEUE_INCREMENT;
    queue.queue=(uint32_t*)realloc((void*)queue.queue,queue.alloced*sizeof(uint32_t));
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

 if(queue.number==0)                                                                       /* There is nothing in the queue */
    insert=0;
 else if(result->shortest.distance>results->results[queue.queue[start]].shortest.distance) /* Check key is not before start */
    insert=start;
 else if(result->shortest.distance<results->results[queue.queue[end]].shortest.distance)   /* Check key is not after end */
    insert=end+1;
 else
   {
    do
      {
       mid=(start+end)/2;                                                                      /* Choose mid point */

       if(results->results[queue.queue[mid]].shortest.distance>result->shortest.distance)      /* Mid point is too low */
          start=mid;
       else if(results->results[queue.queue[mid]].shortest.distance<result->shortest.distance) /* Mid point is too high */
          end=mid;
       else                                                                                    /* Mid point is correct */
         {
          if(&results->results[queue.queue[mid]]==result)
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
    memmove(&queue.queue[insert+1],&queue.queue[insert],(queue.number-insert)*sizeof(uint32_t));

 /* Insert the new entry */

 queue.queue[insert]=result-results->results;

 queue.number++;
}


/*++++++++++++++++++++++++++++++++++++++
  Pop an item from the end of the queue.

  Result *pop_from_queue Returns the top item.

  Results *results The set of results that the queue is processing.
  ++++++++++++++++++++++++++++++++++++++*/

Result *pop_from_queue(Results *results)
{
 if(queue.number)
    return LookupResult(results,queue.queue[--queue.number]);
 else
    return NULL;
}
