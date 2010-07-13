/***************************************
 $Header: /home/amb/CVS/routino/src/waysx.h,v 1.23 2010-07-13 17:43:51 amb Exp $

 A header file for the extended Ways structure.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2010 Andrew M. Bishop

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

#include <assert.h>
#include <stdint.h>

#include "types.h"

#include "typesx.h"
#include "ways.h"

#include "files.h"


/* Data structures */


/*+ An extended structure containing a single way. +*/
struct _WayX
{
 way_t    id;                   /*+ The way identifier. +*/

 index_t  prop;                 /*+ The index of the properties of the way in the compacted list. +*/

 Way      way;                  /*+ The real Way data. +*/
};


/*+ A structure containing a set of ways (memory format). +*/
struct _WaysX
{
 char    *filename;             /*+ The name of the temporary file (for the WaysX). +*/
 int      fd;                   /*+ The file descriptor of the temporary file (for the WaysX). +*/

 uint32_t xnumber;              /*+ The number of unsorted extended ways. +*/

 WayX    *xdata;                /*+ The extended data for the Ways (sorted). +*/
 WayX     cached[2];            /*+ Two cached ways read from the file in slim mode. +*/

 uint32_t number;               /*+ How many entries are still useful? +*/

 uint32_t cnumber;              /*+ How many entries are there after compacting? +*/

 index_t *idata;                /*+ The index of the extended data for the Ways (sorted by ID). +*/

 char    *nfilename;            /*+ The name of the temporary file (for the names). +*/

 uint32_t nlength;              /*+ How long is the string of name entries? +*/
};


/* Functions */


WaysX *NewWayList(int append);
void FreeWayList(WaysX *waysx,int keep);

void SaveWayList(WaysX *waysx,const char *filename);

index_t IndexWayX(WaysX* waysx,way_t id);
static WayX *LookupWayX(WaysX* waysx,index_t index,int position);

void AppendWay(WaysX* waysx,way_t id,Way *way,const char *name);

void SortWayList(WaysX *waysx);


/* Inline the frequently called functions */

/*+ The command line '--slim' option. +*/
extern int option_slim;

/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular way.

  WayX *LookupWayX Returns a pointer to the extended way with the specified id.

  WaysX* waysx The set of ways to process.

  index_t index The way index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

static inline WayX *LookupWayX(WaysX* waysx,index_t index,int position)
{
 assert(index!=NO_WAY);     /* Must be a valid way */

 if(option_slim)
   {
    SeekFile(waysx->fd,index*sizeof(WayX));

    ReadFile(waysx->fd,&waysx->cached[position-1],sizeof(WayX));

    return(&waysx->cached[position-1]);
   }
 else
   {
    return(&waysx->xdata[index]);
   }
}


#endif /* WAYSX_H */
