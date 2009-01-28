/***************************************
 $Header: /home/amb/CVS/routino/src/segments.h,v 1.19 2009-01-28 18:46:55 amb Exp $

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

#include "nodes.h"
#include "ways.h"
#include "profiles.h"


/* Constants */


/*+ The number of bins for segments - expect ~8,000,000 segments and use 4*sqrt(N) bins. +*/
#define NBINS_SEGMENTS 8192

/*+ The array size increment for segments - expect ~8,000,000 segments. +*/
#define INCREMENT_SEGMENTS 1024*1024


/* Simple Types */

/*+ A long distance, measured in metres. +*/
typedef uint32_t distance_t;

/*+ A duration, measured in 1/10th seconds. +*/
typedef uint32_t duration_t;


/*+ A flag to mark a distance as only applying for the other direction. +*/
#define ONEWAY_OPPOSITE (distance_t)(0x80000000)

/*+ The real distance ignoring the ONEWAY_OPPOSITE flag. +*/
#define DISTANCE(xx)  (distance_t)((xx)&(~ONEWAY_OPPOSITE))


/*+ Conversion from distance_t to kilometres. +*/
#define distance_to_km(xx) ((double)(xx)/1000.0)

/*+ Conversion from metres to distance_t. +*/
#define km_to_distance(xx) ((distance_t)((double)(xx)*1000.0))

/*+ Conversion from duration_t to minutes. +*/
#define duration_to_minutes(xx) ((double)(xx)/600.0)

/*+ Conversion from duration_t to hours. +*/
#define duration_to_hours(xx) ((double)(xx)/36000.0)

/*+ Conversion from hours to duration_t. +*/
#define hours_to_duration(xx) ((duration_t)((double)(xx)*36000.0))

/*+ Conversion from distance_t and speed_t to duration_t. +*/
#define distance_speed_to_duration(xx,yy) ((duration_t)(((double)(xx)/(double)(yy))*(36000.0/1000.0)))


/* Data structures */


/*+ A structure containing a single segment. +*/
typedef struct _Segment
{
 uint32_t     node1;            /*+ The index of the starting node. +*/
 uint32_t     node2;            /*+ The index of the finishing node. +*/
 uint32_t     wayindex;         /*+ The index of the way associated with the segment. +*/
 distance_t   distance;         /*+ The distance between the nodes. +*/
}
 Segment;

/*+ An extended structure used for processing. +*/
typedef struct _SegmentEx
{
 node_t       node1;            /*+ The starting node. +*/
 node_t       node2;            /*+ The finishing node. +*/
 int          super;            /*+ A marker for super segments. +*/

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

Segments *LoadSegmentList(const char *filename);
void SaveSegmentList(SegmentsMem *segmentsmem,const char *filename);

SegmentEx *FindFirstSegment(SegmentsMem* segmentsem,node_t node);
SegmentEx *FindNextSegment(SegmentsMem* segmentsem,SegmentEx *segmentex);

Segment *NextSegment(Segments *segments,Segment *segment);

SegmentEx *AppendSegment(SegmentsMem *segmentsmem,node_t node1,node_t node2,uint32_t wayindex);

void SortSegmentList(SegmentsMem *segmentsmem);

void RemoveBadSegments(SegmentsMem *segmentsmem);

void MeasureSegments(SegmentsMem *segmentsmem,NodesMem *nodesmem);
void FixupSegments(SegmentsMem *segmentsmem,NodesMem *nodesmem);

distance_t Distance(Node *node1,Node *node2);

duration_t Duration(Segment *segment,Way *way,Profile *profile);

#define LookupSegment(xxx,yyy)   (&(xxx)->segments[yyy])

#define LookupSegmentEx(xxx,yyy) (&(xxx)->xdata[yyy])

#define IndexSegment(xxx,yyy)    ((yyy)-&(xxx)->segments[0])

#define IndexSegmentEx(xxx,yyy)  ((yyy)-&(xxx)->xdata[0])


#endif /* SEGMENTS_H */
