/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.5 2009-01-05 18:53:28 amb Exp $

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


extern Nodes    *OSMNodes;
extern Ways     *OSMWays;
extern Segments *OSMSegments;


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

 return(0);
}
