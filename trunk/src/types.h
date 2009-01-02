/***************************************
 $Header: /home/amb/CVS/routino/src/types.h,v 1.3 2009-01-02 11:33:47 amb Exp $

 Type definitions
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
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

/*+ A long distance, measured in metres. +*/
typedef uint32_t distance_t;

/*+ A short distance, measured in metres (up to ~65.5km). +*/
typedef uint16_t distance_short_t;

/*+ Conversion from distance_t to kilometres. +*/
#define distance_to_km(xx) ((double)(xx)/1000.0)

/*+ Conversion from metres to distance_t. +*/
#define km_to_distance(xx) ((distance_t)((double)(xx)*1000.0))

/*+ A duration, measured in centiseconds. +*/
typedef uint32_t duration_t;

/*+ A shortt duration, measured in centiseconds (up to ~11 minutes). +*/
typedef uint16_t duration_short_t;

/*+ Conversion from duration_t to minutes. +*/
#define duration_to_minutes(xx) ((double)(xx)/6000.0)

/*+ Conversion from duration_t to hours. +*/
#define duration_to_hours(xx) ((double)(xx)/360000.0)

/*+ Conversion from hours to duration_t. +*/
#define hours_to_duration(xx) ((duration_t)((double)(xx)*360000.0))


/*+ A structure containing a single node. +*/
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
 Node     nodes[1024];          /*+ An array of nodes whose size is not
                                    necessarily limited to 1024 (i.e. may overflow
                                    the end of this structure). +*/
}
 Nodes;


/*+ A structure containing a single way. +*/
typedef struct _Way
{
 way_t     id;                  /*+ The way identifier. +*/
 char     *name;                /*+ An offset into the array of names. +*/
 // waytype_t type;                /*+ The type of the way. +*/
}
 Way;

/*+ A structure containing a set of ways. +*/
typedef struct _Ways
{
 uint32_t alloced;              /*+ The amount of space allocated for ways in the array +*/
 uint32_t number;               /*+ The number of occupied ways in the array +*/
 Way      ways[1024];           /*+ An array of ways whose size is not
                                    necessarily limited to 1024 (i.e. may overflow
                                    the end of this structure). +*/
}
 Ways;


/*+ A structure containing a single segment +*/
typedef struct _Segment
{
 node_t           node1;        /*+ The starting node. +*/
 node_t           node2;        /*+ The finishing node. +*/
 way_t            way;          /*+ The way associated with the segment. +*/
 distance_short_t distance;     /*+ The distance between the nodes. +*/
 duration_short_t duration;     /*+ The time duration to travel between the nodes. +*/
}
 Segment;

/*+ A structure containing a set of segments. +*/
typedef struct _Segments
{
 uint32_t alloced;              /*+ The amount of space allocated for segments in the array +*/
 uint32_t number;               /*+ The number of occupied segments in the array +*/
 Segment  segments[1024];       /*+ An array of segments whose size is not
                                    necessarily limited to 1024 (i.e. may overflow
                                    the end of this structure). +*/
}
 Segments;


#endif /* TYPES_H */
