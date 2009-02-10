/***************************************
 $Header: /home/amb/CVS/routino/src/results.h,v 1.10 2009-02-10 19:49:26 amb Exp $

 A header file for the results.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef RESULTS_H
#define RESULTS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"


/* Data structures */

/*+ The result for a node. +*/
typedef struct _Result
{
 index_t     node;              /*+ The node for which this result applies. +*/

 index_t      prev;             /*+ The previous node following the best path. +*/
 index_t      next;             /*+ The next node following the best path. +*/

 distance_t  distance;          /*+ The distance travelled to the node following the best path. +*/
 duration_t  duration;          /*+ The time taken to the node following the best path. +*/

 distance_t  sortby;            /*+ The value to sort the results by in the queue. +*/
}
 Result;

/*+ A list of results. +*/
typedef struct _Results
{
 uint32_t nbins;                /*+ The number of bins. +*/
 uint32_t mask;                 /*+ A bit mask to select the bottom 'nbins' bits. +*/

 uint32_t alloced;              /*+ The amount of space allocated for results
                                    (the length of the number and pointers arrays and
                                     1/nbins times the amount in the real results). +*/
 uint32_t number;               /*+ The total number of occupied results. +*/

 uint32_t *count;               /*+ An array of nbins counters of results in each array. +*/
 Result ***point;               /*+ An array of nbins arrays of pointers to actual results. +*/

 Result  **data;                /*+ An array of arrays containing the actual results
                                    (don't need to realloc the array of data when adding more,
                                    only realloc the array that points to the array of data).
                                    Most importantly pointers into the real data don't change
                                    as more space is allocated (since realloc is not being used). +*/
}
 Results;


/* Functions */

Results *NewResultsList(int nbins);
void FreeResultsList(Results *results);

Result *InsertResult(Results *results,index_t node);

Result *FindResult(Results *results,index_t node);

Result *FirstResult(Results *results);
Result *NextResult(Results *results,Result *result);

/* Queue Functions */

void insert_in_queue(Result *result);
Result *pop_from_queue(void);


#endif /* RESULTS_H */
