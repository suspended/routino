/***************************************
 $Header: /home/amb/CVS/routino/src/waysx.h,v 1.1 2009-02-07 15:57:21 amb Exp $

 A header file for the extended Ways structure.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef WAYSX_H
#define WAYSX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "ways.h"


/* Data structures */


/*+ An extended structure containing a single way. +*/
struct _WayX
{
 char    *name;                 /*+ The name of the way. +*/

 Way      way;                  /*+ The real Way data. +*/
};


/*+ A structure containing a set of ways (memory format). +*/
struct _WaysX
{
 uint32_t sorted;               /*+ Is the data sorted? +*/
 uint32_t alloced;              /*+ How many entries are allocated? +*/
 uint32_t number;               /*+ How many entries are used? +*/
 uint32_t length;               /*+ How long is the string of name entries? +*/

 WayX    *idata;                /*+ The extended data for the Ways (sorted by index). +*/
 WayX   **ndata;                /*+ The extended data for the Ways (sorted by name). +*/
 char    *names;                /*+ The array containing all the names. +*/
};


/* Macros */


#define LookupWayX(xxx,yyy) (&(xxx)->idata[yyy])

#define IndexWayX(xxx,yyy)  ((yyy)-&(xxx)->idata[0])


/* Functions */


WaysX *NewWayList(void);

void SaveWayList(WaysX *waysx,const char *filename);

Way *AppendWay(WaysX* waysx,const char *name);

void SortWayList(WaysX *waysx);


#endif /* WAYSX_H */
