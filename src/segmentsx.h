/***************************************
 $Header: /home/amb/CVS/routino/src/segmentsx.h,v 1.1 2009-02-07 15:56:59 amb Exp $

 A header file for the extended segments.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef SEGMENTSX_H
#define SEGMENTSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "segments.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _SegmentX
{
 node_t    node1;               /*+ The starting node. +*/
 node_t    node2;               /*+ The finishing node. +*/

 Segment   segment;             /*+ The real segment data. +*/
};


/*+ A structure containing a set of segments (memory format). +*/
struct _SegmentsX
{
 uint32_t   sorted;             /*+ Is the data sorted and therefore searchable? +*/
 uint32_t   alloced;            /*+ How many entries are allocated? +*/
 uint32_t   number;             /*+ How many entries are used from those allocated? +*/
 uint32_t   xnumber;            /*+ How many entries are still useful? +*/

 SegmentX **sdata;              /*+ The extended segment data (sorted by node). +*/
 SegmentX  *xdata;              /*+ The extended segment data (unsorted). +*/
};


/* Macros */


#define LookupSegmentX(xxx,yyy) ((xxx)->sdata[yyy])


/* Functions */


SegmentsX *NewSegmentList(void);
void FreeSegmentList(SegmentsX *segmentsx);

void SaveSegmentList(SegmentsX *segmentsx,const char *filename);

SegmentX **FindFirstSegmentX(SegmentsX* segmentsx,node_t node);
SegmentX **FindNextSegmentX(SegmentsX* segmentsx,SegmentX **segmentx);

Segment *AppendSegment(SegmentsX* segmentsx,node_t node1,node_t node2);

void SortSegmentList(SegmentsX *segmentsx);

void RemoveBadSegments(SegmentsX *segmentsx);

void MeasureSegments(SegmentsX *segmentsx,NodesX *nodesx);

void RotateSegments(SegmentsX* segmentsx,NodesX *nodesx);

void DeduplicateSegments(SegmentsX* segmentsx,NodesX *nodesx,WaysX *waysx);

void IndexSegments(SegmentsX* segmentsx,NodesX *nodesx);

distance_t DistanceX(NodeX *nodex1,NodeX *nodex2);


#endif /* SEGMENTSX_H */
