/***************************************
 $Header: /home/amb/CVS/routino/src/results.h,v 1.4 2009-01-17 17:49:58 amb Exp $

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

#include "nodes.h"
#include "segments.h"


/* Constants */


#if 1 /* set to 0 to use a flat array, 1 for indexed. */

/*+ The array size increment for results. +*/
#define INCREMENT_RESULTS 256

/*+ The number of bins for results. +*/
#define NBINS_RESULTS 1024

#else

/*+ The array size increment for results. +*/
#define INCREMENT_RESULTS 256*1024

#undef NBINS_RESULTS

#endif


/* Data structures */

/*+ One part of the result for a node. +*/
typedef struct _HalfResult
{
 node_t      prev;              /*+ The previous node following the shortest path. +*/
 node_t      next;              /*+ The next node following the shortest path. +*/
 distance_t  distance;          /*+ The distance travelled to the node following the shortest path. +*/
 duration_t  duration;          /*+ The time taken to the node following the shortest path. +*/
}
 HalfResult;

/*+ One complete result for a node. +*/
typedef struct _Result
{
 node_t     node;               /*+ The node for which this result applies. +*/
 HalfResult shortest;           /*+ The result for the shortest path. +*/
 HalfResult quickest;           /*+ The result for the quickest path. +*/
}
 Result;

/*+ A list of results. +*/
typedef struct _Results
{
 uint32_t alloced;              /*+ The amount of space allocated for results in the array. +*/
 uint32_t number;               /*+ The total number of occupied results. +*/
#ifdef NBINS_RESULTS
 uint32_t numbin[NBINS_RESULTS]; /*+ The number of occupied results in the array. +*/
 uint32_t *offsets[NBINS_RESULTS];/*+ An array of pointers to arrays of results. +*/
#else
 uint32_t number;               /*+ The number of occupied results in the array. +*/
 uint32_t *offsets;             /*+ An array of pointers to results. +*/
#endif
 Result *results;               /*+ An array containing the actual results. +*/
}
 Results;


/* Functions */

Results *NewResultsList(void);
void FreeResultsList(Results *results);

Result *InsertResult(Results *results,node_t node);

Result *FindResult(Results *results,node_t node);

#define LookupResult(xxx,yyy) (&xxx->results[yyy])


#endif /* RESULTS_H */
