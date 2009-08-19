/***************************************
 $Header: /home/amb/CVS/routino/src/waysx.h,v 1.15 2009-08-19 18:02:08 amb Exp $

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

#include "typesx.h"
#include "types.h"


/* Data structures */


/*+ An extended structure containing a single way. +*/
struct _WayX
{
 way_t    id;                   /*+ The way identifier. +*/

 char    *name;                 /*+ The name of the way. +*/

 Way     *way;                  /*+ A pointer to the real Way data. +*/
};


/*+ A structure containing a set of ways (memory format). +*/
struct _WaysX
{
 int32_t  row;                  /*+ How many rows are allocated? +*/
 uint32_t col;                  /*+ How many columns are used in the last row? +*/

 WayX   **xdata;                /*+ The extended data for the Ways (unsorted). +*/

 uint32_t number;               /*+ How many entries are still useful? +*/

 WayX   **idata;                /*+ The extended data for the Ways (sorted by ID). +*/
 WayX   **ndata;                /*+ The extended data for the Ways (sorted by name and way properties). +*/

 int32_t  wrow;                 /*+ How many rows are allocated? +*/
 uint32_t wcol;                 /*+ How many columns are used in the last row? +*/

 Way    **wxdata;               /*+ The data for the Ways (unsorted). +*/
 Way    **wdata;                /*+ The data for the Ways (sorted like ndata and compacted). +*/

 char    *names;                /*+ The array containing all the names. +*/

 uint32_t length;               /*+ How long is the string of name entries? +*/
};


/* Functions */


WaysX *NewWayList(void);
void FreeWayList(WaysX *waysx);

void SaveWayList(WaysX *waysx,const char *filename);

WayX *FindWayX(WaysX* waysx,way_t id);

Way *AppendWay(WaysX* waysx,way_t id,const char *name);

void SortWayList(WaysX *waysx);

void CompactWays(WaysX* waysx);

index_t IndexWayInWaysX(WaysX *waysx,WayX *wayx);

#endif /* WAYSX_H */
