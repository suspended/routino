/***************************************
 $Header: /home/amb/CVS/routino/src/ways.h,v 1.21 2009-02-01 17:11:08 amb Exp $

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

#include "ways.h"


/* Data structures */


/*+ An extended structure containing a single way. +*/
typedef struct _WayX
{
 index_t   index;              /*+ The index of the way. +*/
 char      *name;               /*+ The name of the way. +*/

 Way        way;                /*+ The real Way data. +*/
}
 WayX;

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
typedef struct _WaysX
{
 uint32_t sorted;               /*+ Is the data sorted? +*/
 uint32_t alloced;              /*+ How many entries are allocated? +*/
 uint32_t number;               /*+ How many entries are used? +*/
 uint32_t length;               /*+ How long is the string of name entries? +*/

 WayX   *xdata;                /*+ The extended data for the Ways. +*/
 char    *names;                /*+ The array containing all the names. +*/
}
 WaysX;


/* Functions */


WaysX *NewWayList(void);

Ways *LoadWayList(const char *filename);
void SaveWayList(WaysX *waysx,const char *filename);

WayX *AppendWay(WaysX *waysx,const char *name);

void SortWayList(WaysX *waysx);

Highway HighwayType(const char *highway);
Transport TransportType(const char *transport);

const char *HighwayName(Highway highway);
const char *TransportName(Transport transport);

const char *HighwayList(void);
const char *TransportList(void);

#define LookupWayX(xxx,yyy) (&(xxx)->xdata[yyy])

#define IndexWayX(xxx,yyy)  ((yyy)-&(xxx)->xdata[0])


#endif /* WAYS_H */
