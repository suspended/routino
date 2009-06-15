/***************************************
 $Header: /home/amb/CVS/routino/src/types.h,v 1.22 2009-06-15 18:52:54 amb Exp $

 Type definitions

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


#ifndef TYPES_H
#define TYPES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>
#include <math.h>


/* Constants and macros for handling them */


/*+ The latitude and longitude conversion factor from float (radians) to integer. +*/
#define LAT_LONG_SCALE  (1024*65536)

/*+ The latitude and longitude integer range within each bin. +*/
#define LAT_LONG_BIN    65536

/*+ Convert a latitude or longitude to a bin number (taking care of rounding). +*/
#define lat_long_to_bin(xxx) ((int32_t)floorf((xxx)*1024))

/*+ Convert a bin number to a latitude or longitude. +*/
#define bin_to_lat_long(xxx) ((float)(xxx)/1024)


/*+ Convert radians to degrees. +*/
#define radians_to_degrees(xxx) ((xxx)*(180.0/M_PI))

/*+ Convert degrees to radians. +*/
#define degrees_to_radians(xxx) ((xxx)*(M_PI/180.0))


/*+ A flag to mark super-nodes and super-segments. +*/
#define SUPER_FLAG   ((index_t)0x80000000)

/*+ A node index excluding the super-node flag. +*/
#define NODE(xxx)    (index_t)((xxx)&(~SUPER_FLAG))

/*+ An undefined node index. +*/
#define NO_NODE      (~(index_t)0)

/*+ A segment index excluding the super-segment flag. +*/
#define SEGMENT(xxx) (index_t)((xxx)&(~SUPER_FLAG))

/*+ An undefined segment index. +*/
#define NO_SEGMENT   (~(index_t)0)


/*+ A flag to mark a distance as only applying from node1 to node2. +*/
#define ONEWAY_1TO2  ((distance_t)0x80000000)

/*+ A flag to mark a distance as only applying from node2 to node1. +*/
#define ONEWAY_2TO1  ((distance_t)0x40000000)

/*+ The real distance ignoring the ONEWAY_* flags. +*/
#define DISTANCE(xx) (distance_t)((xx)&(~(ONEWAY_1TO2|ONEWAY_2TO1)))


/*+ A very large almost infinite score. +*/
#define INF_SCORE    (score_t)1E30


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

/*+ A routing optimisation score. +*/
typedef float score_t;


/*+ Conversion from distance_t to kilometres. +*/
#define distance_to_km(xx) ((double)(xx)/1000.0)

/*+ Conversion from metres to distance_t. +*/
#define km_to_distance(xx) ((distance_t)((double)(xx)*1000.0))

/*+ Conversion from duration_t to minutes. +*/
#define duration_to_minutes(xx) ((double)(xx)/600.0)

/*+ Conversion from duration_t to hours. +*/
#define duration_to_hours(xx)   ((double)(xx)/36000.0)

/*+ Conversion from hours to duration_t. +*/
#define hours_to_duration(xx)   ((duration_t)((double)(xx)*36000.0))

/*+ Conversion from distance_t and speed_t to duration_t. +*/
#define distance_speed_to_duration(xx,yy) ((duration_t)(((double)(xx)/(double)(yy))*(36000.0/1000.0)))


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
  Way_Path        =10,
  Way_Bridleway   =11,
  Way_Cycleway    =12,
  Way_Footway     =13,

  Way_Unknown     =14,

  Way_OneWay      =32,
  Way_Roundabout  =64
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


/*+ The speed limit of a way, measured in km/hour. +*/
typedef uint8_t speed_t;

/*+ The maximum weight of a way, measured in 0.2 tonnes. +*/
typedef uint8_t weight_t;

/*+ The maximum height of a way, measured in 0.1 metres. +*/
typedef uint8_t height_t;

/*+ The maximum width of a way, measured in 0.1 metres. +*/
typedef uint8_t width_t;

/*+ The maximum length of a way, measured in 0.1 metres. +*/
typedef uint8_t length_t;


/*+ Conversion of km/hr to speed_t. +*/
#define kph_to_speed(xxx)      (speed_t)(xxx)

/*+ Conversion of speed_t to km/hr. +*/
#define speed_to_kph(xxx)      (xxx)

/*+ Conversion of tonnes to weight_t. +*/
#define tonnes_to_weight(xxx)  (weight_t)((xxx)*5)

/*+ Conversion of weight_t to tonnes. +*/
#define weight_to_tonnes(xxx)  ((double)(xxx)/5.0)

/*+ Conversion of metres to height_t. +*/
#define metres_to_height(xxx)  (height_t)((xxx)*10)

/*+ Conversion of height_t to metres. +*/
#define height_to_metres(xxx)  ((double)(xxx)/10.0)

/*+ Conversion of metres to width_t. +*/
#define metres_to_width(xxx)   (width_t)((xxx)*10)

/*+ Conversion of width_t to metres. +*/
#define width_to_metres(xxx)   ((double)(xxx)/10.0)

/*+ Conversion of metres to length_t. +*/
#define metres_to_length(xxx)  (length_t)((xxx)*10)

/*+ Conversion of length_t to metres. +*/
#define length_to_metres(xxx)  ((double)(xxx)/10.0)


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
