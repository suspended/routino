/***************************************
 $Header: /home/amb/CVS/routino/src/ways.h,v 1.12 2009-01-23 16:09:09 amb Exp $

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


/*+ The number of bins for ways - expect ~1,000,000 ways and use 4*sqrt(N) bins. +*/
#define NBINS_WAYS 4096

/*+ The array size increment for ways - expect ~1,000,000 ways. +*/
#define INCREMENT_WAYS 256*1024


/* Simple Types */


/*+ A way identifier. +*/
typedef uint32_t way_t;

/*+ The speed limit of the way. +*/
typedef uint8_t speed_t;

/*+ The type of a way. +*/
typedef uint8_t waytype_t;

/*+ The different types of a way. +*/
typedef enum _Highway
 {
  Way_Motorway   = 1,
  Way_Trunk      = 2,
  Way_Primary    = 3,
  Way_Tertiary   = 4,
  Way_Secondary  = 5,
  Way_Unclassfied= 6,
  Way_Residential= 7,
  Way_Service    = 8,
  Way_Track      = 9,
  Way_Bridleway  =10,
  Way_Cycleway   =11,
  Way_Footway    =12,

  Way_Unknown    =15,

  Way_OneWay     =16,
  Way_Roundabout =32
 }
 Highway;


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
  Transport_PSV       = 6,
  Transport_Goods     = 7,
  Transport_HGV       = 8
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
  Allow_PSV       =1<<(Transport_PSV      -1),
  Allow_Goods     =1<<(Transport_Goods    -1),
  Allow_HGV       =1<<(Transport_HGV      -1),

  Allow_ALL       =255
 }
 Allowed;


/* Data structures */


/*+ A structure containing a single way. +*/
typedef struct _Way
{
 way_t      id;                 /*+ The way identifier. +*/
 uint32_t   name;               /*+ An offset of the name of the way in the ways array. +*/
 speed_t    limit;              /*+ The defined speed limit on the way. +*/
 waytype_t  type;               /*+ The type of the way. +*/
 wayallow_t allow;              /*+ The type of traffic allowed on the way. +*/
}
 Way;

/*+ A structure containing a set of ways (mmap format). +*/
typedef struct _Ways
{
 uint32_t offset[NBINS_WAYS];   /*+ An offset to the first entry in each bin. +*/
 uint32_t number;               /*+ How many entries are used in total? +*/
 Way      ways[1];              /*+ An array of ways whose size is not limited to 1
                                    (i.e. may overflow the end of this structure). +*/
}
 Ways;

/*+ A structure containing a set of ways (memory format). +*/
typedef struct _WaysMem
{
 uint32_t alloced;              /*+ How many entries are allocated? +*/
 uint32_t number;               /*+ How many entries are used? +*/
 uint32_t number_str;           /*+ How many name entries are used? +*/
 uint32_t sorted;               /*+ Is the data sorted and therefore searchable? +*/

 Ways    *ways;                 /*+ The real data that will be memory mapped later. +*/
 char   **names;                /*+ An array of names. +*/
}
 WaysMem;


/* Functions */


WaysMem *NewWayList(void);

Ways *LoadWayList(const char *filename);
Ways *SaveWayList(WaysMem *ways,const char *filename);

Way *FindWay(Ways *ways,way_t id);

Way *AppendWay(WaysMem *ways,way_t id,const char *name);

void SortWayList(WaysMem *ways);

Highway HighwayType(const char *highway);
Transport TransportType(const char *transport);

speed_t WaySpeed(Way *way);

#define LookupWay(xxx,yyy) (&xxx->ways[yyy])

#define WayName(xxx,yyy) ((char*)&xxx->ways[yyy->name])


#endif /* WAYS_H */
