/***************************************
 $Header: /home/amb/CVS/routino/src/segmentsx.h,v 1.7 2009-06-25 18:17:58 amb Exp $

 A header file for the extended segments.

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


#ifndef SEGMENTSX_H
#define SEGMENTSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "typesx.h"
#include "types.h"
#include "segments.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _SegmentX
{
 node_t    node1;               /*+ The id of the starting node. +*/
 node_t    node2;               /*+ The id of the finishing node. +*/

 way_t     way;                 /*+ The id of the way. +*/

 Segment   segment;             /*+ The real segment data. +*/
};


/*+ A structure containing a set of segments (memory format). +*/
struct _SegmentsX
{
 uint32_t   sorted;             /*+ Is the data sorted and therefore searchable? +*/
 uint32_t   alloced;            /*+ How many entries are allocated? +*/
 uint32_t   xnumber;            /*+ How many entries are used from those allocated? +*/
 uint32_t   number;             /*+ How many entries are still useful? +*/

 SegmentX **sdata;              /*+ The extended segment data (sorted by node). +*/
 SegmentX  *xdata;              /*+ The extended segment data (unsorted). +*/
};


/* Macros */


#define LookupSegmentX(xxx,yyy) (&(xxx)->sdata[yyy])


/* Functions */


SegmentsX *NewSegmentList(void);
void FreeSegmentList(SegmentsX *segmentsx);

void SaveSegmentList(SegmentsX *segmentsx,const char *filename);

SegmentX **FindFirstSegmentX(SegmentsX* segmentsx,node_t node);
SegmentX **FindNextSegmentX(SegmentsX* segmentsx,SegmentX **segmentx);

Segment *AppendSegment(SegmentsX* segmentsx,way_t way,node_t node1,node_t node2);

void SortSegmentList(SegmentsX *segmentsx);

void RemoveBadSegments(NodesX *nodesx,SegmentsX *segmentsx);

void MeasureSegments(SegmentsX *segmentsx,NodesX *nodesx);

void RotateSegments(SegmentsX* segmentsx,NodesX *nodesx);

void DeduplicateSegments(SegmentsX* segmentsx,NodesX *nodesx,WaysX *waysx);

void IndexSegments(SegmentsX* segmentsx,NodesX *nodesx);

distance_t DistanceX(NodeX *nodex1,NodeX *nodex2);


#endif /* SEGMENTSX_H */
