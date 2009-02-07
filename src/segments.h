/***************************************
 $Header: /home/amb/CVS/routino/src/segments.h,v 1.27 2009-02-07 15:56:07 amb Exp $

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
#include "profiles.h"


/* Data structures */


/*+ A structure containing a single segment. +*/
struct _Segment
{
 index_t    node1;              /*+ The index of the starting node. +*/
 index_t    node2;              /*+ The index of the finishing node. +*/

 index_t    next1;              /*+ The index of the next segment sharing node1. +*/
 index_t    next2;              /*+ The index of the next segment sharing node2. +*/

 index_t    way;                /*+ The index of the way associated with the segment. +*/

 distance_t distance;           /*+ The distance between the nodes. +*/
};


/*+ A structure containing a set of segments (mmap format). +*/
struct _Segments
{
 uint32_t  number;              /*+ How many entries are used in total? +*/

 Segment  *segments;            /*+ An array of segments. +*/

 void     *data;                /*+ The memory mapped data. +*/
};


/* Macros */


/*+ Return a Segment pointer given a set of segments and an index. +*/
#define LookupSegment(xxx,yyy) (&(xxx)->segments[yyy])

/*+ Return true if this is a normal segment. +*/
#define IsNormalSegment(xxx)   (((xxx)->node1)&SUPER_FLAG)

/*+ Return true if this is a super-segment. +*/
#define IsSuperSegment(xxx)    (((xxx)->node2)&SUPER_FLAG)

/*+ Return true if the segment is oneway towards the specified node. +*/
#define IsOnewayTo(xxx,yyy)    ((NODE((xxx)->node1)==(yyy))?((xxx)->distance&ONEWAY_2TO1):((xxx)->distance&ONEWAY_1TO2))

/*+ Return true if the segment is oneway from the specified node. +*/
#define IsOnewayFrom(xxx,yyy)  ((NODE((xxx)->node2)==(yyy))?((xxx)->distance&ONEWAY_2TO1):((xxx)->distance&ONEWAY_1TO2))

/*+ Return the other node in the segment that is not the specified node. +*/
#define OtherNode(xxx,yyy)     ((NODE((xxx)->node1)==(yyy))?NODE((xxx)->node2):NODE((xxx)->node1))


/* Functions */


Segments *LoadSegmentList(const char *filename);

Segment *NextSegment(Segments* segments,Segment *segment,index_t node);

float Distance(float lat1,float lon1,float lat2,float lon2);

duration_t Duration(Segment *segment,Way *way,Profile *profile);


#endif /* SEGMENTS_H */
