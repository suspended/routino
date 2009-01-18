/***************************************
 $Header: /home/amb/CVS/routino/src/segments.h,v 1.8 2009-01-18 16:03:45 amb Exp $

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


/* Constants */


#if 1 /* set to 0 to use a flat array, 1 for indexed. */

/*+ The number of bins for segments. +*/
#define NBINS_SEGMENTS 2048

#else

#undef NBINS_SEGMENTS

#endif

/*+ The array size increment for segments. +*/
#define INCREMENT_SEGMENTS 1024*1024


/* Simple Types */


/*+ A short distance, measured in metres (up to ~65.5km). +*/
typedef uint16_t distance_short_t;

/*+ A short duration, measured in 1/10th seconds (up to ~110 minutes). +*/
typedef uint16_t duration_short_t;

/*+ An invalid short distance +*/
#define INVALID_SHORT_DISTANCE (distance_short_t)(~0)

/*+ An invalid duration +*/
#define INVALID_SHORT_DURATION (duration_short_t)(~0)


/*+ A long distance, measured in metres. +*/
typedef uint32_t distance_t;

/*+ A duration, measured in 1/10th seconds. +*/
typedef uint32_t duration_t;


/*+ An invalid distance +*/
#define INVALID_DISTANCE (distance_t)(~0)

/*+ An invalid duration +*/
#define INVALID_DURATION (duration_t)(~0)


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


/* Data structures */


/*+ A structure containing a single segment. +*/
typedef struct _Segment
{
 node_t           node1;        /*+ The starting node. +*/
 node_t           node2;        /*+ The finishing node. +*/
 way_t            way;          /*+ The way associated with the segment. +*/
 distance_short_t distance;     /*+ The distance between the nodes. +*/
 duration_short_t duration;     /*+ The time duration to travel between the nodes. +*/
}
 Segment;

/*+ A structure containing a set of segments (mmap format). +*/
typedef struct _Segments
{
 uint32_t number;               /*+ How many entries are used? +*/
#ifdef NBINS_SEGMENTS
 uint32_t offset[NBINS_SEGMENTS+1]; /*+ An offset to the first entry in each bin. +*/
#endif
 Segment  segments[1];          /*+ An array of segments whose size is not limited to 1
                                    (i.e. may overflow the end of this structure). +*/
}
 Segments;

/*+ A structure containing a set of segments (memory format). +*/
typedef struct _SegmentsMem
{
 uint32_t  alloced;             /*+ How many entries are allocated? +*/
 uint32_t  number;              /*+ How many entries are used? +*/
 uint32_t  sorted;              /*+ Is the data sorted and therefore searchable? +*/

 Segments *segments;            /*+ The real data +*/
}
 SegmentsMem;


/* Functions in segments.c */


SegmentsMem *NewSegmentList(void);

Segments *LoadSegmentList(const char *filename);
Segments *SaveSegmentList(SegmentsMem *segments,const char *filename);

Segment *FindFirstSegment(Segments *segments,node_t node);
Segment *FindNextSegment(Segments *segments,Segment *segment);

Segment *AppendSegment(SegmentsMem *segments,node_t node1,node_t node2,way_t way);

void SortSegmentList(SegmentsMem *segments);

void RemoveBadSegments(SegmentsMem *segments);

void FixupSegmentLengths(SegmentsMem *segments,Nodes *nodes,Ways *ways);

distance_t Distance(Node *node1,Node *node2);

#define LookupSegment(xxx,yyy) (&xxx->segments[yyy])


#endif /* SEGMENTS_H */
