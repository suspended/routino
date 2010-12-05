/***************************************
 $Header: /home/amb/CVS/routino/src/typesx.h,v 1.5 2010-12-05 16:19:24 amb Exp $

 Type definitions for eXtended types.

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


#ifndef TYPESX_H
#define TYPESX_H    /*+ To stop multiple inclusions. +*/


#include <stdint.h>


/* Simple Types */

/*+ A node identifier - must be at least as large as index_t. +*/
typedef uint32_t node_t;

/*+ A way identifier - must be at least as large as index_t. +*/
typedef uint32_t way_t;

/*+ A relation identifier - must be at least as large as index_t. +*/
typedef uint32_t relation_t;


/* Enumerated types */

/*+ Turn restrictions. +*/
typedef enum _TurnRestriction
 {
  Restrict_None              =0,

  Restrict_no_right_turn,
  Restrict_no_left_turn,
  Restrict_no_u_turn,
  Restrict_no_straight_on,
  Restrict_only_right_turn,
  Restrict_only_left_turn,
  Restrict_only_straight_on
 }
 TurnRestriction;


/* Data structures */

typedef struct _NodeX NodeX;

typedef struct _NodesX NodesX;

typedef struct _SegmentX SegmentX;

typedef struct _SegmentsX SegmentsX;

typedef struct _WayX WayX;

typedef struct _WaysX WaysX;

typedef struct _RouteRelX RouteRelX;

typedef struct _TurnRestrictRelX TurnRestrictRelX;

typedef struct _RelationsX RelationsX;


#endif /* TYPESX_H */
