/***************************************
 $Header: /home/amb/CVS/routino/src/ways.h,v 1.4 2009-01-10 15:59:59 amb Exp $

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


#if 1 /* set to 0 to use a flat array, 1 for indexed. */

/*+ The number of bins for ways. +*/
#define NBINS_WAYS 1024

#else

#undef NBINS_WAYS

#endif

/*+ The array size increment for ways. +*/
#define INCREMENT_WAYS 256*1024


/* Simple Types */


/*+ A way identifier. +*/
typedef uint32_t way_t;

/*+ The speed limit of the way. +*/
typedef uint8_t speed_t;

/*+ A way type identifier. +*/
typedef uint16_t waytype_t;

typedef enum _WayType
 {
  Way_Motorway   =1,
  Way_Trunk      =2,
  Way_Primary    =3,
  Way_Tertiary   =4,
  Way_Secondary  =5,
  Way_Unclassfied=6,
  Way_Residential=7,

  Way_HighestRoutable=7,

  Way_Service    =8,
  Way_Track      =9,
  Way_Bridleway  =10,
  Way_Cycleway   =11,
  Way_Footway    =12,
  Way_Unknown    =15,
 }
 WayType;

#define Way_TYPE(xx) ((xx)&0x0f)

#define Way_ONEWAY       16
#define Way_ROUNDABOUT   32
#define Way_NOTROUTABLE 128


/* Data structures */


/*+ A structure containing a single way. +*/
typedef struct _Way
{
 way_t     id;                  /*+ The way identifier. +*/
 uint32_t  name;                /*+ An offset of the name of the way in the ways array. +*/
 speed_t   limit;               /*+ The defined speed limit on the way. +*/
 speed_t   speed;               /*+ The assumed speed limit on the way. +*/
 waytype_t type;                /*+ The type of the way. +*/
}
 Way;

/*+ A structure containing a set of ways (mmap format). +*/
typedef struct _Ways
{
 uint32_t number;               /*+ How many entries are used? +*/
#ifdef NBINS_WAYS
 uint32_t offset[NBINS_WAYS+1]; /*+ An offset to the first entry in each bin. +*/
#endif
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

 Ways    *ways;                 /*+ The real data +*/
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

const char *WayName(Ways *ways,Way *way);
WayType TypeOfWay(const char *type);


#endif /* WAYS_H */
