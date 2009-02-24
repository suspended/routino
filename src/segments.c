/***************************************
 $Header: /home/amb/CVS/routino/src/segments.c,v 1.35 2009-02-24 19:59:36 amb Exp $

 Segment data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "functions.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "profiles.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a segment list from a file.

  Segments* SaveSegmentList Returns the segment list that has just been loaded.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Segments *LoadSegmentList(const char *filename)
{
 void *data;
 Segments *segments;

 segments=(Segments*)malloc(sizeof(Segments));

 data=MapFile(filename);

 if(!data)
    return(NULL);

 /* Copy the Segments structure from the loaded data */

 *segments=*((Segments*)data);

 /* Adjust the pointers in the Segments structure. */

 segments->data=data;
 segments->segments=(Segment*)(data+(off_t)segments->segments);

 return(segments);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  Segment *NextSegment Returns a pointer to the next segment with the same id.

  Segments* segments The set of segments to process.

  Segment *segment The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *NextSegment(Segments* segments,Segment *segment,index_t node)
{
 if(NODE(segment->node1)==node)
   {
    segment++;
    if(NODE(segment->node1)!=node || (segment-segments->segments)>=segments->number)
       return(NULL);
    else
       return(segment);
   }
 else
   {
    if(segment->next2==~0)
       return(NULL);
    else
       return(LookupSegment(segments,segment->next2));
   }
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Calculate the distance between two locations.

  distance_t Distance Returns the distance between the locations.

  float lat1 The latitude of the first location.

  float lon1 The longitude of the first location.

  float lat2 The latitude of the second location.

  float lon2 The longitude of the second location.
  ++++++++++++++++++++++++++++++++++++++*/

distance_t Distance(float lat1,float lon1,float lat2,float lon2)
{
 float dlon = lon1 - lon2;
 float dlat = lat1 - lat2;

 float a1,a2,a,sa,c,d;

 if(dlon==0 && dlat==0)
   return 0;

 a1 = sinf (dlat / 2);
 a2 = sinf (dlon / 2);
 a = (a1 * a1) + cosf (lat1) * cosf (lat2) * a2 * a2;
 sa = sqrtf (a);
 if (sa <= 1.0)
   {c = 2 * asinf (sa);}
 else
   {c = 2 * asinf (1.0);}
 d = 6378.137 * c;

 return km_to_distance(d);
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the duration of segment.

  duration_t Duration Returns the duration of travel between the nodes.

  Segment *segment The segment to traverse.

  Way *way The way that the segment belongs to.

  Profile *profile The profile of the transport being used.
  ++++++++++++++++++++++++++++++++++++++*/

duration_t Duration(Segment *segment,Way *way,Profile *profile)
{
 speed_t    speed=profile->speed[HIGHWAY(way->type)];
 distance_t distance=DISTANCE(segment->distance);

 return distance_speed_to_duration(distance,speed);
}
