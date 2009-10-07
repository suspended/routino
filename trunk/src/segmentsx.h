/***************************************
 $Header: /home/amb/CVS/routino/src/segmentsx.h,v 1.18 2009-10-07 18:03:48 amb Exp $

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


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _SegmentX
{
 node_t     node1;              /*+ The id of the starting node. +*/
 node_t     node2;              /*+ The id of the finishing node. +*/

 way_t      way;                /*+ The id of the way. +*/

 distance_t distance;           /*+ The distance between the nodes. +*/
};


/*+ A structure containing a set of segments (memory format). +*/
struct _SegmentsX
{
 char      *filename;           /*+ The name of the temporary file. +*/
 int        fd;                 /*+ The file descriptor of the temporary file. +*/

 uint32_t   xnumber;            /*+ The number of unsorted extended nodes. +*/

 SegmentX  *xdata;              /*+ The extended segment data (unsorted). +*/
 SegmentX   cached[2];          /*+ Two cached segments read from the file in slim mode. +*/

 uint32_t   number;             /*+ How many entries are still useful? +*/

 node_t   *idata;               /*+ The extended segment data (sorted by node1 then node2). +*/

 Segment   *sdata;              /*+ The segment data (same order as n1data). +*/
};


/* Functions */


SegmentsX *NewSegmentList(void);
void FreeSegmentList(SegmentsX *segmentsx);

void SaveSegmentList(SegmentsX *segmentsx,const char *filename);

SegmentX *LookupSegmentX(SegmentsX* segmentsx,index_t index,int position);

index_t IndexFirstSegmentX(SegmentsX* segmentsx,node_t node);
index_t IndexNextSegmentX(SegmentsX* segmentsx,index_t index);

void AppendSegment(SegmentsX* segmentsx,way_t way,node_t node1,node_t node2,distance_t distance);

void SortSegmentList(SegmentsX *segmentsx);

void RemoveBadSegments(NodesX *nodesx,SegmentsX *segmentsx);

void MeasureSegments(SegmentsX *segmentsx,NodesX *nodesx);

void RotateSegments(SegmentsX* segmentsx);

void DeduplicateSegments(SegmentsX* segmentsx,WaysX *waysx);

void CreateRealSegments(SegmentsX *segmentsx,WaysX *waysx);

void IndexSegments(SegmentsX* segmentsx,NodesX *nodesx);


#endif /* SEGMENTSX_H */
