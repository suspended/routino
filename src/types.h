/***************************************
 $Header: /home/amb/CVS/routino/src/types.h,v 1.7 2009-01-10 11:14:25 amb Exp $

 Type definitions
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef TYPES_H
#define TYPES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "nodes.h"
#include "ways.h"
#include "segments.h"



/*+ A structure containing a single super-segment +*/
typedef struct _SuperSegment
{
 node_t     node1;        /*+ The starting node. +*/
 node_t     node2;        /*+ The finishing node. +*/
 distance_t distance;     /*+ The distance between the nodes. +*/
 duration_t duration;     /*+ The time duration to travel between the nodes. +*/
}
 SuperSegment;

/*+ A structure containing a set of segments. +*/
typedef struct _SuperSegments
{
 uint32_t      number;          /*+ The number of occupied segments in the array. +*/
 SuperSegment  segments[1];     /*+ An array of segments whose size is not limited to 1
                                    (i.e. may overflow the end of this structure). +*/
}
 SuperSegments;


#endif /* TYPES_H */
