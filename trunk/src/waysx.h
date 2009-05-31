/***************************************
 $Header: /home/amb/CVS/routino/src/waysx.h,v 1.4 2009-05-31 12:30:12 amb Exp $

 A header file for the extended Ways structure.

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
