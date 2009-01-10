/***************************************
 $Header: /home/amb/CVS/routino/src/ways.h,v 1.2 2009-01-10 11:53:49 amb Exp $

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

/*+ The array size increment for ways. +*/
#define INCREMENT_WAYS 256

/*+ The number of bins for ways. +*/
#define NBINS_WAYS 1024

#else

/*+ The array size increment for ways. +*/
#define INCREMENT_WAYS 256*1024

#undef NBINS_WAYS

#endif


/* Simple Types */


/*+ A way identifier. +*/
typedef uint32_t way_t;

/*+ The speed limit of the way. +*/
typedef uint8_t speed_t;


/* Data structures */


/*+ A structure containing a single way. +*/
typedef struct _Way
{
 way_t     id;                  /*+ The way identifier. +*/
 speed_t   limit;               /*+ The defined speed limit on the way. +*/
 speed_t   speed;               /*+ The assumed speed limit on the way. +*/
 uint32_t  name;                /*+ An offset of the name of the way in the ways array. +*/
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

void AppendWay(WaysMem *ways,way_t id,const char *name,speed_t speed);

void SortWayList(WaysMem *ways);

const char *WayName(Ways *ways,Way *way);


#endif /* WAYS_H */
