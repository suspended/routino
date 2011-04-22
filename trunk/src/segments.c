/***************************************
 Segment data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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


#include <sys/types.h>
#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "fakes.h"

#include "files.h"
#include "profiles.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a segment list from a file.

  Segments* LoadSegmentList Returns the segment list that has just been loaded.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Segments *LoadSegmentList(const char *filename)
{
 Segments *segments;

 segments=(Segments*)malloc(sizeof(Segments));

#if !SLIM

 segments->data=MapFile(filename);

 /* Copy the SegmentsFile structure from the loaded data */

 segments->file=*((SegmentsFile*)segments->data);

 /* Set the pointers in the Segments structure. */

 segments->segments=(Segment*)(segments->data+sizeof(SegmentsFile));

#else

 segments->fd=ReOpenFile(filename);

 /* Copy the SegmentsFile header structure from the loaded data */

 ReadFile(segments->fd,&segments->file,sizeof(SegmentsFile));

 segments->incache[0]=NO_SEGMENT;
 segments->incache[1]=NO_SEGMENT;
 segments->incache[2]=NO_SEGMENT;

#endif

 return(segments);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  Segment *NextSegment Returns a pointer to the next segment with the same id.

  Segments* segments The set of segments to process.

  Segment *segment The current segment.

  index_t node The current node.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *NextSegment(Segments* segments,Segment *segment,index_t node)
{
 if(segment->node1==node)
   {
#if SLIM
    index_t index=IndexSegment(segments,segment);
    index++;

    if(index>=segments->file.number)
       return(NULL);
    segment=LookupSegment(segments,index,1);
    if(segment->node1!=node)
       return(NULL);
    else
       return(segment);
#else
    segment++;
    if(IndexSegment(segments,segment)>=segments->file.number || segment->node1!=node)
       return(NULL);
    else
       return(segment);
#endif
   }
 else
   {
    if(segment->next2==NO_NODE)
       return(NULL);
    else
       return(LookupSegment(segments,segment->next2,1));
   }
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Calculate the distance between two locations.

  distance_t Distance Returns the distance between the locations.

  double lat1 The latitude of the first location.

  double lon1 The longitude of the first location.

  double lat2 The latitude of the second location.

  double lon2 The longitude of the second location.
  ++++++++++++++++++++++++++++++++++++++*/

distance_t Distance(double lat1,double lon1,double lat2,double lon2)
{
 double dlon = lon1 - lon2;
 double dlat = lat1 - lat2;

 double a1,a2,a,sa,c,d;

 if(dlon==0 && dlat==0)
   return 0;

 a1 = sin (dlat / 2);
 a2 = sin (dlon / 2);
 a = (a1 * a1) + cos (lat1) * cos (lat2) * a2 * a2;
 sa = sqrt (a);
 if (sa <= 1.0)
   {c = 2 * asin (sa);}
 else
   {c = 2 * asin (1.0);}
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
 speed_t    speed1=way->speed;
 speed_t    speed2=profile->speed[HIGHWAY(way->type)];
 distance_t distance=DISTANCE(segment->distance);

 if(speed1==0)
   {
    if(speed2==0)
       return(hours_to_duration(10));
    else
       return distance_speed_to_duration(distance,speed2);
   }
 else /* if(speed1!=0) */
   {
    if(speed2==0)
       return distance_speed_to_duration(distance,speed1);
    else if(speed1<=speed2)
       return distance_speed_to_duration(distance,speed1);
    else
       return distance_speed_to_duration(distance,speed2);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the angle to turn at a junction from segment1 to segment2 at node.

  double TurnAngle Returns a value in the range -180 to +180 indicating the angle to turn.

  Nodes *nodes The set of nodes.

  Segment *segment1 The current segment.

  Segment *segment2 The next segment.

  index_t node The node at which they join.

  Straight ahead is zero, turning to the right is positive (e.g. +90 degrees) and turning to the left is negative (e.g. -90 degrees).
  Angles are calculated using flat Cartesian lat/long grid approximation (after scaling longitude due to latitude).
  ++++++++++++++++++++++++++++++++++++++*/

double TurnAngle(Nodes *nodes,Segment *segment1,Segment *segment2,index_t node)
{
 double lat1,latm,lat2;
 double lon1,lonm,lon2;
 double angle1,angle2,angle;
 index_t node1,node2;

 node1=OtherNode(segment1,node);
 node2=OtherNode(segment2,node);

 if(IsFakeNode(node1))
    GetFakeLatLong(node1,&lat1,&lon1);
 else
    GetLatLong(nodes,node1,&lat1,&lon1);

 if(IsFakeNode(node))
    GetFakeLatLong(node,&latm,&lonm);
 else
    GetLatLong(nodes,node,&latm,&lonm);

 if(IsFakeNode(node2))
    GetFakeLatLong(node2,&lat2,&lon2);
 else
    GetLatLong(nodes,node2,&lat2,&lon2);

 angle1=atan2((lonm-lon1)*cos(latm),(latm-lat1));
 angle2=atan2((lon2-lonm)*cos(latm),(lat2-latm));

 angle=angle2-angle1;

 angle=radians_to_degrees(angle);

 if(angle<-180) angle+=360;
 if(angle> 180) angle-=360;

 return(angle);
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the bearing of a segment from the given node.

  double BearingAngle Returns a value in the range 0 to 359 indicating the bearing.

  Nodes *nodes The set of nodes.

  Segment *segment The segment.

  index_t node The node to start.

  Angles are calculated using flat Cartesian lat/long grid approximation (after scaling longitude due to latitude).
  ++++++++++++++++++++++++++++++++++++++*/

double BearingAngle(Nodes *nodes,Segment *segment,index_t node)
{
 double lat1,lat2;
 double lon1,lon2;
 double angle;
 index_t node1,node2;

 node1=node;
 node2=OtherNode(segment,node);

 if(IsFakeNode(node1))
    GetFakeLatLong(node1,&lat1,&lon1);
 else
    GetLatLong(nodes,node1,&lat1,&lon1);

 if(IsFakeNode(node2))
    GetFakeLatLong(node2,&lat2,&lon2);
 else
    GetLatLong(nodes,node2,&lat2,&lon2);

 angle=atan2((lat2-lat1),(lon2-lon1)*cos(lat1));

 angle=radians_to_degrees(angle);

 angle=270-angle;

 if(angle<  0) angle+=360;
 if(angle>360) angle-=360;

 return(angle);
}
