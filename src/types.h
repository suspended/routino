/***************************************
 $Header: /home/amb/CVS/routino/src/types.h,v 1.1 2008-12-31 12:21:27 amb Exp $

 Type definitions
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef TYPES_H
#define TYPES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>


/*+ A node identifier. +*/
typedef uint32_t node_t;

/*+ A way identifier. +*/
typedef uint32_t way_t;

/*+ A node latitude or longitude. +*/
typedef float latlong_t;

/*+ A distance, measured in metres. +*/
typedef uint32_t distance_t;

/*+ A distance, measured in milliseconds. +*/
typedef uint32_t duration_t;


/*+ A structure containing a single nodes. +*/
typedef struct _Node
{
 node_t    id;                  /*+ The node identifier. +*/
 latlong_t latitude;            /*+ The node latitude. +*/
 latlong_t longitude;           /*+ The node longitude. +*/
}
 Node;

/*+ A structure containing a set of nodes. +*/
typedef struct _Nodes
{
 uint32_t alloced;              /*+ The amount of space allocated for nodes in the array +*/
 uint32_t number;               /*+ The number of occupied nodes in the array +*/
 Node nodes[1024];              /*+ An array of nodes whose size is not
                                    necessarily limited to 1024 (i.e. may overflow
                                    the end of this structure). +*/
}
 Nodes;


/*+ A structure containing a single segment +*/
typedef struct _Segment
{
 node_t node1;                  /*+ The starting node. +*/
 node_t node2;                  /*+ The finishing node. +*/
 way_t  way;                    /*+ The way associated with the segment. +*/
 distance_t distance;           /*+ The distance between the nodes. +*/
 duration_t duration;           /*+ The time duration to travel between the nodes. +*/
}
 Segment;

/*+ A structure containing a set of segments. +*/
typedef struct _Segments
{
 uint32_t alloced;              /*+ The amount of space allocated for segments in the array +*/
 uint32_t number;               /*+ The number of occupied segments in the array +*/
 Segment segments[1024];        /*+ An array of segments whose size is not
                                    necessarily limited to 1024 (i.e. may overflow
                                    the end of this structure). +*/
}
 Segments;


#endif /* TYPES_H */
