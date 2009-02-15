/***************************************
 $Header: /home/amb/CVS/routino/src/types.h,v 1.14 2009-02-15 13:45:54 amb Exp $

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
#include <math.h>


/* Constants and macros for handling them */


/*+ The latitude and longitude conversion factor from float to integer. +*/
#define LAT_LONG_SCALE  (1024*1024)

/*+ The latitude and longitude integer range within each bin. +*/
#define LAT_LONG_BIN    65536

/*+ Convert a latitude or longitude to a bin number (taking care of rounding). +*/
#define lat_long_to_bin(xxx) ((int32_t)floorf((xxx)*(LAT_LONG_SCALE/LAT_LONG_BIN)))

/*+ Convert a bin number to a latitude or longitude. +*/
#define bin_to_lat_long(xxx) ((float)(xxx)/(LAT_LONG_SCALE/LAT_LONG_BIN))


/*+ A flag to mark super-nodes and super-segments. +*/
#define SUPER_FLAG 0x80000000

/*+ A node index excluding the super-node flag +*/
#define NODE(xxx)    (index_t)((xxx)&(~SUPER_FLAG))

/*+ A segment index excluding the super-segment flag +*/
#define SEGMENT(xxx) (index_t)((xxx)&(~SUPER_FLAG))


/*+ A flag to mark a distance as only applying from node1 to node2. +*/
#define ONEWAY_1TO2 0x80000000

/*+ A flag to mark a distance as only applying from node2 to node1. +*/
#define ONEWAY_2TO1 0x40000000

/*+ The real distance ignoring the ONEWAY_* flags. +*/
#define DISTANCE(xx)  (distance_t)((xx)&(~(ONEWAY_1TO2|ONEWAY_2TO1)))


/* Simple Types */


/*+ A node identifier. +*/
typedef uint32_t node_t;


/*+ A node, segment or way index. +*/
typedef uint32_t index_t;


/*+ A node latitude or longitude offset. +*/
typedef uint16_t ll_off_t;


/*+ A distance, measured in metres. +*/
typedef uint32_t distance_t;

/*+ A duration, measured in 1/10th seconds. +*/
typedef uint32_t duration_t;


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

typedef struct _Node Node;

typedef struct _Nodes Nodes;

typedef struct _Segment Segment;

typedef struct _Segments Segments;

typedef struct _Way Way;

typedef struct _Ways Ways;

typedef struct _NodeX NodeX;

typedef struct _NodesX NodesX;

typedef struct _SegmentX SegmentX;

typedef struct _SegmentsX SegmentsX;

typedef struct _WayX WayX;

typedef struct _WaysX WaysX;


#endif /* TYPES_H */
