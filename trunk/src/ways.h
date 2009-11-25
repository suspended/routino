/***************************************
 $Header: /home/amb/CVS/routino/src/ways.h,v 1.34 2009-11-25 15:00:37 amb Exp $

 A header file for the ways.

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


#ifndef WAYS_H
#define WAYS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"


/* Data structures */


/*+ A structure containing a single way (members ordered to minimise overall size). +*/
struct _Way
{
 index_t    name;               /*+ The offset of the name of the way in the names array. +*/

 wayallow_t allow;              /*+ The type of traffic allowed on the way. +*/

 waytype_t  type;               /*+ The highway type of the way. +*/

 wayprop_t  props;              /*+ The properties of the way. +*/

 speed_t    speed;              /*+ The defined maximum speed limit of the way. +*/

 weight_t   weight;             /*+ The defined maximum weight of traffic on the way. +*/
 height_t   height;             /*+ The defined maximum height of traffic on the way. +*/
 width_t    width;              /*+ The defined maximum width of traffic on the way. +*/
 length_t   length;             /*+ The defined maximum length of traffic on the way. +*/
};


/*+ A structure containing a set of ways (mmap format). +*/
struct _Ways
{
 uint32_t   number;             /*+ How many ways are stored? +*/
 uint32_t   onumber;            /*+ How many ways were there originally? +*/

 wayallow_t allow;              /*+ The type of traffic allowed when parsing. +*/
 wayprop_t  props;              /*+ The properties that were included when parsing. +*/

 Way       *ways;               /*+ An array of ways. +*/
 char      *names;              /*+ An array of characters containing the names. +*/

 void      *data;               /*+ The memory mapped data. +*/
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
Property PropertyType(const char *property);

const char *HighwayName(Highway highway);
const char *TransportName(Transport transport);
const char *PropertyName(Property property);

const char *AllowedNameList(wayallow_t allowed);
const char *PropertiesNameList(wayprop_t properties);

const char *HighwayList(void);
const char *TransportList(void);
const char *PropertyList(void);

int WaysCompare(Way *way1,Way *way2);


#endif /* WAYS_H */
