/***************************************
 $Header: /home/amb/CVS/routino/src/typesx.h,v 1.1 2009-06-15 18:56:43 amb Exp $

 Type definitions for eXtended types.

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


#ifndef TYPESX_H
#define TYPESX_H    /*+ To stop multiple inclusions. +*/


#include <stdint.h>


/* Simple Types */

/*+ A node identifier. +*/
typedef uint32_t node_t;


/* Data structures */

typedef struct _NodeX NodeX;

typedef struct _NodesX NodesX;

typedef struct _SegmentX SegmentX;

typedef struct _SegmentsX SegmentsX;

typedef struct _WayX WayX;

typedef struct _WaysX WaysX;


#endif /* TYPESX_H */
