/***************************************
 $Header: /home/amb/CVS/routino/src/results.h,v 1.7 2009-01-23 15:22:31 amb Exp $

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
 uint32_t nbins;                /*+ The number of bins. +*/
 uint32_t mask;                 /*+ A bit mask to select the bottom 'nbins' bits. +*/

 uint32_t alloced;              /*+ The amount of space allocated for results in the array. +*/
 uint32_t number;               /*+ The total number of occupied results. +*/

 uint32_t *numbin;              /*+ The number of occupied results in the array. +*/
 uint32_t **offsets;            /*+ An array of pointers to arrays of results. +*/

 Result *results;               /*+ An array containing the actual results. +*/
}
 Results;


/* Functions */

Results *NewResultsList(int nbins);
void FreeResultsList(Results *results);

Result *InsertResult(Results *results,node_t node);

Result *FindResult(Results *results,node_t node);

#define LookupResult(xxx,yyy) (&xxx->results[yyy])


/* Queue Functions */

void insert_in_queue(Results *results,Result *result);
Result *pop_from_queue(Results *results);


#endif /* RESULTS_H */
