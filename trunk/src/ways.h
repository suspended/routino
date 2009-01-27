/***************************************
 $Header: /home/amb/CVS/routino/src/ways.h,v 1.18 2009-01-27 18:22:37 amb Exp $

 A header file for the ways.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef WAYS_H
#define WAYS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>


/* Constants */


/*+ The array size increment for ways - expect ~1,000,000 ways. +*/
#define INCREMENT_WAYS 256*1024


/* Simple Types */


/*+ The speed limit of the way. +*/
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


/*+ The type of a method of transport. +*/
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


/*+ A structure containing a single way. +*/
typedef struct _Way
{
 uint32_t   name;               /*+ The offset of the name of the way in the names array. +*/
 speed_t    limit;              /*+ The defined speed limit on the way. +*/
 waytype_t  type;               /*+ The type of the way. +*/
 wayallow_t allow;              /*+ The type of traffic allowed on the way. +*/
}
 Way;

/*+ An extended structure containing a single way. +*/
typedef struct _WayEx
{
 uint32_t   index;              /*+ The index of the way. +*/
 char      *name;               /*+ The name of the way. +*/

 Way        way;                /*+ The real Way data. +*/
}
 WayEx;

/*+ A structure containing a set of ways (mmap format). +*/
typedef struct _Ways
{
 uint32_t number;               /*+ How many entries are used? +*/

 Way     *ways;                 /*+ An array of ways. */
 char    *names;                /*+ An array of characters containing the names. +*/

 void    *data;                 /*+ The memory mapped data. +*/
}
 Ways;

/*+ A structure containing a set of ways (memory format). +*/
typedef struct _WaysMem
{
 uint32_t sorted;               /*+ Is the data sorted? +*/
 uint32_t alloced;              /*+ How many entries are allocated? +*/
 uint32_t number;               /*+ How many entries are used? +*/
 uint32_t length;               /*+ How long is the string of name entries? +*/

 WayEx   *xdata;                /*+ The extended data for the Ways. +*/
 char    *names;                /*+ The array containing all the names. +*/
}
 WaysMem;


/* Functions */


WaysMem *NewWayList(void);

Ways *LoadWayList(const char *filename);
Ways *SaveWayList(WaysMem *waysmem,const char *filename);

void DropWayList(Ways *ways);

Way *AppendWay(WaysMem *waysmem,const char *name);

void SortWayList(WaysMem *waysmem);

Highway HighwayType(const char *highway);
Transport TransportType(const char *transport);

const char *HighwayName(Highway highway);
const char *TransportName(Transport transport);

const char *HighwayList(void);
const char *TransportList(void);

#define LookupWay(xxx,yyy) (&(xxx)->ways[yyy])

#define IndexWay(xxx,yyy) ((yyy)-&(xxx)->ways[0])

#define WayName(xxx,yyy) (&(xxx)->names[(yyy)->name])


#endif /* WAYS_H */
