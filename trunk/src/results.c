/***************************************
 $Header: /home/amb/CVS/routino/src/results.c,v 1.1 2009-01-10 13:40:04 amb Exp $

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

#if NBINS_RESULTS
 for(i=0;i<NBINS_RESULTS;i++)
   {
    results->number[i]=0;
    results->results[i]=(Result**)malloc(results->alloced*sizeof(Result*));
   }
#else
 results->number=0;
 results->results=(Results**)malloc(results->alloced*sizeof(Result));
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
    free(results->results[i]);
#else
 free(results->results);
#endif

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
 int end=results->number[bin]-1;
 Result **resultsp=results->results[bin];
 uint32_t *numberp=&results->number[bin];
 int i;
#else
 int start=0;
 int end=results->number-1;
 Result **resultsp=results->results;
 uint32_t numberp=&results->number;
#endif
 int mid;
 int insert=-1;

 /* Check that the arrays have enough space. */

 if(*numberp==results->alloced)
   {
    results->alloced+=INCREMENT_RESULTS;

#ifdef NBINS_RESULTS
    for(i=0;i<NBINS_RESULTS;i++)
       results->results[i]=(Result**)realloc((void*)results->results[i],results->alloced*sizeof(Result*));

    resultsp=results->results[bin];
#else
    results->results=(Result**)realloc((void*)results->results[i],results->alloced*sizeof(Result*));

    resultsp=results->results;
#endif
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

 if(end<start)                       /* There are no results */
    insert=start;
 else if(node<resultsp[start]->node) /* Check key is not before start */
    insert=start;
 else if(node>resultsp[end]->node)   /* Check key is not after end */
    insert=end+1;
 else
   {
    do
      {
       mid=(start+end)/2;                /* Choose mid point */

       if(resultsp[mid]->node<node)      /* Mid point is too low */
          start=mid;
       else if(resultsp[mid]->node>node) /* Mid point is too high */
          end=mid;
       else
          assert(0);
      }
    while((end-start)>1);

    insert=end;
   }

 /* Shuffle the array up */

 if(insert!=*numberp)
    memmove(&resultsp[insert+1],&resultsp[insert],(*numberp-insert)*sizeof(Result*));

 /* Insert the new entry */

 (*numberp)++;

 resultsp[insert]=(Result*)malloc(sizeof(Result));

 return(resultsp[insert]);
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
 int end=results->number[bin]-1;
 Result **resultsp=results->results[bin];
#else
 int start=0;
 int end=results->number-1;
 Result **resultsp=results->results;
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

 if(end<start)                       /* There are no results */
    return(NULL);
 else if(node<resultsp[start]->node) /* Check key is not before start */
    return(NULL);
 else if(node>resultsp[end]->node)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                /* Choose mid point */

       if(resultsp[mid]->node<node)      /* Mid point is too low */
          start=mid+1;
       else if(resultsp[mid]->node>node) /* Mid point is too high */
          end=mid-1;
       else                              /* Mid point is correct */
          return(resultsp[mid]);
      }
    while((end-start)>1);

    if(resultsp[start]->node==node)      /* Start is correct */
       return(resultsp[start]);

    if(resultsp[end]->node==node)        /* End is correct */
       return(resultsp[end]);
   }

 return(NULL);
}
