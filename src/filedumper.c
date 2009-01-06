/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.6 2009-01-06 18:32:16 amb Exp $

 Memory file dumper.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdio.h>
#include <stdlib.h>

#include "functions.h"
#include "types.h"


extern Nodes         *OSMNodes;
extern Ways          *OSMWays;
extern Segments      *OSMSegments;
extern SuperSegments *OSMSuperSegments;


int main(int argc,char** argv)
{
 int i;
 distance_t longest=0;
 duration_t slowest=0;

 /* Examine the nodes */

 LoadNodeList("data/nodes.mem");

 printf("Nodes\n");
 printf("-----\n");

 printf("sizeof(Node)=%9d Bytes\n",sizeof(Node));
 printf("number      =%9d\n",OSMNodes->number);
 printf("total size  =%9d Bytes\n",OSMNodes->number*sizeof(Node));

 /* Examine the ways */

 LoadWayList("data/ways.mem");

 printf("\n");
 printf("Ways\n");
 printf("----\n");

 printf("sizeof(Way)=%9d Bytes\n",sizeof(Way));
 printf("number     =%9d\n",OSMWays->number);
 printf("strings    =%9d\n",OSMWays->number_str);
 printf("total size =%9d Bytes\n",(OSMWays->number+OSMWays->number_str)*sizeof(Way));

 /* Examine the segments */

 LoadSegmentList("data/segments.mem");

 printf("\n");
 printf("Segments\n");
 printf("--------\n");

 printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
 printf("number         =%9d\n",OSMSegments->number);
 printf("total size     =%9d Bytes\n",OSMSegments->number*sizeof(Segment));

 for(i=0;i<OSMSegments->number;i++)
   {
    if(OSMSegments->segments[i].distance>longest)
       longest=OSMSegments->segments[i].distance;
    if(OSMSegments->segments[i].duration>slowest)
       slowest=OSMSegments->segments[i].duration;
   }

 printf("Longest distance = %.1f km\n",distance_to_km(longest));
 printf("Longest duration = %.1f min\n",duration_to_minutes(slowest));

 /* Examine the super-segments */

 LoadSuperSegmentList("data/supersegments.mem");

 printf("\n");
 printf("SuperSegments\n");
 printf("-------------\n");

 printf("sizeof(SuperSegment)=%9d Bytes\n",sizeof(SuperSegment));
 printf("number              =%9d\n",OSMSuperSegments->number);
 printf("total size          =%9d Bytes\n",OSMSuperSegments->number*sizeof(SuperSegment));

 for(i=0;i<OSMSuperSegments->number;i++)
   {
    if(OSMSuperSegments->segments[i].distance>longest)
       longest=OSMSuperSegments->segments[i].distance;
    if(OSMSuperSegments->segments[i].duration>slowest)
       slowest=OSMSuperSegments->segments[i].duration;
   }

 printf("Longest distance = %.1f km\n",distance_to_km(longest));
 printf("Longest duration = %.1f min\n",duration_to_minutes(slowest));

 return(0);
}
