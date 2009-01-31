/***************************************
 $Header: /home/amb/CVS/routino/src/segments.h,v 1.22 2009-01-31 15:32:42 amb Exp $

 A header file for the segments.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef SEGMENTS_H
#define SEGMENTS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "nodes.h"
#include "profiles.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
typedef struct _SegmentEx
{
 node_t       node1;            /*+ The starting node. +*/
 node_t       node2;            /*+ The finishing node. +*/

 Segment      segment;          /*+ The real segment data. +*/
}
 SegmentEx;

/*+ A structure containing a set of segments (mmap format). +*/
typedef struct _Segments
{
 uint32_t number;               /*+ How many entries are used in total? +*/

 Segment *segments;             /*+ An array of segments. +*/

 void    *data;                 /*+ The memory mapped data. +*/
}
 Segments;

/*+ A structure containing a set of segments (memory format). +*/
typedef struct _SegmentsMem
{
 uint32_t  sorted;              /*+ Is the data sorted and therefore searchable? +*/
 uint32_t  alloced;             /*+ How many entries are allocated? +*/
 uint32_t  number;              /*+ How many entries are used? +*/

 SegmentEx *xdata;              /*+ The extended segment data. +*/
}
 SegmentsMem;


/* Functions in segments.c */


SegmentsMem *NewSegmentList(void);
void FreeSegmentList(SegmentsMem *segmentsmem);

Segments *LoadSegmentList(const char *filename);
void SaveSegmentList(SegmentsMem *segmentsmem,const char *filename);

SegmentEx *FindFirstSegment(SegmentsMem* segmentsem,node_t node);
SegmentEx *FindNextSegment(SegmentsMem* segmentsem,SegmentEx *segmentex);

Segment *NextSegment(Segments *segments,Segment *segment);

SegmentEx *AppendSegment(SegmentsMem *segmentsmem,node_t node1,node_t node2,index_t way);

void SortSegmentList(SegmentsMem *segmentsmem);

void RemoveBadSegments(SegmentsMem *segmentsmem);

void MeasureSegments(SegmentsMem *segmentsmem,NodesMem *nodesmem);
void FixupSegments(SegmentsMem* segmentsmem,NodesMem *nodesmem,SegmentsMem* supersegmentsmem);

distance_t Distance(Node *node1,Node *node2);

duration_t Duration(Segment *segment,Way *way,Profile *profile);

#define LookupSegmentEx(xxx,yyy) (&(xxx)->xdata[yyy])

#define IndexSegmentEx(xxx,yyy)  ((yyy)-&(xxx)->xdata[0])


#endif /* SEGMENTS_H */
