/***************************************
 $Header: /home/amb/CVS/routino/src/types.h,v 1.12 2009-02-07 10:06:37 amb Exp $

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


/* Constants */


/*+ A flag to mark super-nodes and super-segments. +*/
#define SUPER_FLAG 0x80000000

/*+ A flag to mark a distance as only applying from node1 to node2. +*/
#define ONEWAY_1TO2 0x80000000

/*+ A flag to mark a distance as only applying from node2 to node1. +*/
#define ONEWAY_2TO1 0x40000000


/* Simple Types */


/*+ A node, segment or way index. +*/
typedef uint32_t index_t;

/*+ A node index excluding the super-node flag +*/
#define NODE(xxx)    (index_t)((xxx)&(~SUPER_FLAG))

/*+ A segment index excluding the super-segment flag +*/
#define SEGMENT(xxx) (index_t)((xxx)&(~SUPER_FLAG))


/*+ A node latitude or longitude offset. +*/
typedef uint16_t ll_off_t;


/*+ A distance, measured in metres. +*/
typedef uint32_t distance_t;

/*+ A duration, measured in 1/10th seconds. +*/
typedef uint32_t duration_t;


/*+ The real distance ignoring the ONEWAY_OPPOSITE flag. +*/
#define DISTANCE(xx)  (distance_t)((xx)&(~(ONEWAY_1TO2|ONEWAY_2TO1)))

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


/*+ The speed limit of a way. +*/
typedef uint8_t speed_t;

/*+ The type of a way. +*/
typedef uint8_t waytype_t;

/*+ The different types of a way. +*/
typedef enum _Highway
 {
  Way_Motorway    = 1,
  Way_Trunk       = 2,
  Way_Primary     = 3,
  Way_Secondary   = 4,
  Way_Tertiary    = 5,
  Way_Unclassified= 6,
  Way_Residential = 7,
  Way_Service     = 8,
  Way_Track       = 9,
  Way_Bridleway   =10,
  Way_Cycleway    =11,
  Way_Footway     =12,

  Way_Unknown     =13,

  Way_OneWay      =16,
  Way_Roundabout  =32
 }
 Highway;

#define HIGHWAY(xx) ((xx)&0x0f)


/*+ The method of transport. +*/
typedef uint8_t transport_t;

/*+ The different methods of transport. +*/
typedef enum _Transport
 {
  Transport_None      = 0,

  Transport_Foot      = 1,
  Transport_Bicycle   = 2,
  Transport_Horse     = 3,
  Transport_Motorbike = 4,
  Transport_Motorcar  = 5,
  Transport_Goods     = 6,
  Transport_HGV       = 7,
  Transport_PSV       = 8
 }
 Transport;


/*+ The allowed traffic on a way. +*/
typedef uint8_t wayallow_t;

/*+ The different allowed traffic on a way. +*/
typedef enum _Allowed
 {
  Allow_Foot      =1<<(Transport_Foot     -1),
  Allow_Bicycle   =1<<(Transport_Bicycle  -1),
  Allow_Horse     =1<<(Transport_Horse    -1),
  Allow_Motorbike =1<<(Transport_Motorbike-1),
  Allow_Motorcar  =1<<(Transport_Motorcar -1),
  Allow_Goods     =1<<(Transport_Goods    -1),
  Allow_HGV       =1<<(Transport_HGV      -1),
  Allow_PSV       =1<<(Transport_PSV      -1),

  Allow_ALL       =255
 }
 Allowed;


/* Data structures */


/*+ A structure containing a single node. +*/
typedef struct _Node
{
 index_t    firstseg;           /*+ The index of the first segment. +*/

 ll_off_t   latoffset;          /*+ The node latitude offset within its bin. +*/
 ll_off_t   lonoffset;          /*+ The node longitude offset within its bin. +*/
}
 Node;


/*+ A structure containing a single segment. +*/
typedef struct _Segment
{
 index_t    node1;              /*+ The index of the starting node. +*/
 index_t    node2;              /*+ The index of the finishing node. +*/

 index_t    next1;              /*+ The index of the next segment sharing node1. +*/
 index_t    next2;              /*+ The index of the next segment sharing node2. +*/

 index_t    way;                /*+ The index of the way associated with the segment. +*/

 distance_t distance;           /*+ The distance between the nodes. +*/
}
 Segment;


/*+ A structure containing a single way. +*/
typedef struct _Way
{
 index_t    name;               /*+ The offset of the name of the way in the names array. +*/

 speed_t    limit;              /*+ The defined speed limit on the way. +*/

 waytype_t  type;               /*+ The type of the way. +*/

 wayallow_t allow;              /*+ The type of traffic allowed on the way. +*/
}
 Way;


/* Node Functions */


/*+ Return a Node pointer given a set of nodes and an index. +*/
#define LookupNode(xxx,yyy)    (&(xxx)->nodes[yyy])

/*+ Return an index for a Node pointer given a set of nodes. +*/
#define IndexNode(xxx,yyy)     ((yyy)-&(xxx)->nodes[0])

/*+ Return a Segment points given a Node pointer and a set of segments. +*/
#define FirstSegment(xxx,yyy)  LookupSegment((xxx),SEGMENT((yyy)->firstseg))

/*+ Return true if this is a super-node. +*/
#define IsSuperNode(xxx)       (((xxx)->firstseg)&SUPER_FLAG)


/* Segment Functions */


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


/* Way Functions */


/*+ Return a Way* pointer given a set of ways and an index. +*/
#define LookupWay(xxx,yyy)     (&(xxx)->ways[yyy])

/*+ Return the name of a way given the Way pointer and a set of ways. +*/
#define WayName(xxx,yyy)       (&(xxx)->names[(yyy)->name])


#endif /* TYPES_H */
