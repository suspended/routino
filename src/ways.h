/***************************************
 $Header: /home/amb/CVS/routino/src/ways.h,v 1.24 2009-02-07 15:56:08 amb Exp $

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

#include "types.h"


/* Data structures */


/*+ A structure containing a single way. +*/
struct _Way
{
 index_t    name;               /*+ The offset of the name of the way in the names array. +*/

 speed_t    limit;              /*+ The defined speed limit on the way. +*/

 waytype_t  type;               /*+ The type of the way. +*/

 wayallow_t allow;              /*+ The type of traffic allowed on the way. +*/
};


/*+ A structure containing a set of ways (mmap format). +*/
struct _Ways
{
 uint32_t number;               /*+ How many entries are used? +*/

 Way     *ways;                 /*+ An array of ways. */
 char    *names;                /*+ An array of characters containing the names. +*/

 void    *data;                 /*+ The memory mapped data. +*/
};


/* Macros */


/*+ Return a Way* pointer given a set of ways and an index. +*/
#define LookupWay(xxx,yyy)     (&(xxx)->ways[yyy])

/*+ Return the name of a way given the Way pointer and a set of ways. +*/
#define WayName(xxx,yyy)       (&(xxx)->names[(yyy)->name])


/* Functions */


Ways *LoadWayList(const char *filename);

Highway HighwayType(const char *highway);
Transport TransportType(const char *transport);

const char *HighwayName(Highway highway);
const char *TransportName(Transport transport);

const char *HighwayList(void);
const char *TransportList(void);


#endif /* WAYS_H */
