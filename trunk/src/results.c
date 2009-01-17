/***************************************
 $Header: /home/amb/CVS/routino/src/results.c,v 1.2 2009-01-17 17:49:58 amb Exp $

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


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *NewResultsList Returns the results list.
  ++++++++++++++++++++++++++++++++++++++*/

Results *NewResultsList(void)
{
 Results *results;
#if NBINS_RESULTS
 int i;
#endif

 results=(Results*)malloc(sizeof(Results));

 results->alloced=INCREMENT_RESULTS;
 results->number=0;

#if NBINS_RESULTS
 for(i=0;i<NBINS_RESULTS;i++)
   {
    results->numbin[i]=0;

    results->offsets[i]=(uint32_t*)malloc(results->alloced*sizeof(uint32_t));
   }

 results->results=(Result*)malloc(results->alloced*NBINS_RESULTS*sizeof(Result));
#else
 results->offsets=(uint32_t*)malloc(results->alloced*sizeof(uint32_t));

 results->data=(Result*)malloc(results->alloced*sizeof(Result));
#endif

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *results The results list to be destroyed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeResultsList(Results *results)
{
#if NBINS_RESULTS
 int i;
#endif

#if NBINS_RESULTS
 for(i=0;i<NBINS_RESULTS;i++)
    free(results->offsets[i]);
#else
 free(results->offsets);
#endif

 free(results->results);

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
#ifdef NBINS_RESULTS
 int bin=node%NBINS_RESULTS;
 int start=0;
 int end=results->numbin[bin]-1;
 uint32_t *offsetsp=results->offsets[bin];
 int i;
#else
 int start=0;
 int end=results->number-1;
 uint32_t *offsetsp=results->offsets;
#endif
 int mid;
 int insert=-1;

 /* Check that the arrays have enough space. */

#ifdef NBINS_RESULTS
 if(results->numbin[bin]==results->alloced)
   {
    results->alloced+=INCREMENT_RESULTS;

    for(i=0;i<NBINS_RESULTS;i++)
       results->offsets[i]=(uint32_t*)realloc((void*)results->offsets[i],results->alloced*sizeof(uint32_t));

    offsetsp=results->offsets[bin];

    results->results=(Result*)realloc((void*)results->results,results->alloced*NBINS_RESULTS*sizeof(Result));
   }
#else
 if(results->number==results->alloced)
   {
    results->alloced+=INCREMENT_RESULTS;

    results->offsets=(uint32_t*)realloc((void*)results->offsets[i],results->alloced*sizeof(uint32_t));

    offsetsp=results->offsets;

    results->results=(Result*)realloc((void*)results->results,results->alloced*sizeof(Result));
   }
#endif

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

 if(end<start)                                         /* There are no results */
    insert=start;
 else if(node<results->results[offsetsp[start]].node) /* Check key is not before start */
    insert=start;
 else if(node>results->results[offsetsp[end]].node)   /* Check key is not after end */
    insert=end+1;
 else
   {
    do
      {
       mid=(start+end)/2;                                  /* Choose mid point */

       if(results->results[offsetsp[mid]].node<node)      /* Mid point is too low */
          start=mid;
       else if(results->results[offsetsp[mid]].node>node) /* Mid point is too high */
          end=mid;
       else
          assert(0);
      }
    while((end-start)>1);

    insert=end;
   }

 /* Shuffle the array up */

#ifdef NBINS_RESULTS
 if(insert!=results->numbin[bin])
    memmove(&offsetsp[insert+1],&offsetsp[insert],(results->numbin[bin]-insert)*sizeof(uint32_t));
#else
 if(insert!=results->number)
    memmove(&offsetsp[insert+1],&offsetsp[insert],(results->number-insert)*sizeof(uint32_t));
#endif

 /* Insert the new entry */

 offsetsp[insert]=results->number;

 results->number++;

#ifdef NBINS_RESULTS
 results->numbin[bin]++;
#endif

 return(&results->results[offsetsp[insert]]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result, ordered by node.

  Result *insert_result Returns the result that has been found.

  Results *results The results structure to search.

  node_t node The node that is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FindResult(Results *results,node_t node)
{
#ifdef NBINS_RESULTS
 int bin=node%NBINS_RESULTS;
 int start=0;
 int end=results->numbin[bin]-1;
 uint32_t *offsetsp=results->offsets[bin];
#else
 int start=0;
 int end=results->number-1;
 uint32_t *offsetsp=results->offsets;
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

 if(end<start)                                         /* There are no results */
    return(NULL);
 else if(node<results->results[offsetsp[start]].node) /* Check key is not before start */
    return(NULL);
 else if(node>results->results[offsetsp[end]].node)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                                  /* Choose mid point */

       if(results->results[offsetsp[mid]].node<node)      /* Mid point is too low */
          start=mid+1;
       else if(results->results[offsetsp[mid]].node>node) /* Mid point is too high */
          end=mid-1;
       else                                                /* Mid point is correct */
          return(&results->results[offsetsp[mid]]);
      }
    while((end-start)>1);

    if(results->results[offsetsp[start]].node==node)      /* Start is correct */
       return(&results->results[offsetsp[start]]);

    if(results->results[offsetsp[end]].node==node)        /* End is correct */
       return(&results->results[offsetsp[end]]);
   }

 return(NULL);
}
