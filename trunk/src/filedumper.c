/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.8 2009-01-11 09:28:31 amb Exp $

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


int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Ways     *OSMWays;
 Segments *OSMSegments;
 Nodes    *Junctions;
 Segments *SuperSegments;
// int i;
// distance_t longest=0;
// duration_t slowest=0;

 /* Examine the nodes */

 OSMNodes=LoadNodeList("data/nodes.mem");

 printf("Nodes\n");
 printf("-----\n");

 printf("sizeof(Node)=%9d Bytes\n",sizeof(Node));
 printf("number      =%9d\n",OSMNodes->number);
 printf("total size  =%9d Bytes\n",sizeof(OSMNodes)-sizeof(OSMNodes->nodes)+OSMNodes->number*sizeof(Node));

#ifdef NBINS_NODES
// for(i=0;i<=NBINS_NODES;i++)
//    printf("offset[%4d] is %ld\n",i,OSMNodes->offset[i]);
#endif

 /* Examine the ways */

 OSMWays=LoadWayList("data/ways.mem");

 printf("\n");
 printf("Ways\n");
 printf("----\n");

 printf("sizeof(Way) =%9d Bytes\n",sizeof(Way));
 printf("number      =%9d\n",OSMWays->number);
 printf("total size  =%9d Bytes\n",sizeof(OSMWays)-sizeof(OSMWays->ways)+OSMWays->number*sizeof(Way));

#ifdef NBINS_WAYS
// for(i=0;i<=NBINS_WAYS;i++)
//    printf("offset[%4d] is %ld\n",i,OSMWays->offset[i]);
#endif

 /* Examine the segments */

 OSMSegments=LoadSegmentList("data/segments.mem");

 printf("\n");
 printf("Segments\n");
 printf("--------\n");

 printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
 printf("number         =%9d\n",OSMSegments->number);
 printf("total size     =%9d Bytes\n",sizeof(OSMSegments)-sizeof(OSMSegments->segments)+OSMSegments->number*sizeof(Segment));

#ifdef NBINS_NODES
// for(i=0;i<=NBINS_NODES;i++)
//    printf("offset[%4d] is %ld\n",i,OSMNodes->offset[i]);
#endif

// for(i=0;i<OSMSegments->number;i++)
//   {
//    if(OSMSegments->segments[i].distance>longest)
//       longest=OSMSegments->segments[i].distance;
//    if(OSMSegments->segments[i].duration>slowest)
//       slowest=OSMSegments->segments[i].duration;
//   }
//
// printf("Longest distance = %.1f km\n",distance_to_km(longest));
// printf("Longest duration = %.1f min\n",duration_to_minutes(slowest));

 /* Examine the nodes */

 Junctions=LoadNodeList("data/junctions.mem");

 printf("Junctions\n");
 printf("---------\n");

 printf("sizeof(Node)=%9d Bytes\n",sizeof(Node));
 printf("number      =%9d\n",Junctions->number);
 printf("total size  =%9d Bytes\n",sizeof(Junctions)-sizeof(Junctions->nodes)+Junctions->number*sizeof(Node));

#ifdef NBINS_NODES
// for(i=0;i<=NBINS_NODES;i++)
//    printf("offset[%4d] is %ld\n",i,OSMNodes->offset[i]);
#endif

 /* Examine the super-segments */

 SuperSegments=LoadSegmentList("data/super-segments.mem");

 printf("\n");
 printf("SuperSegments\n");
 printf("-------------\n");

 printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
 printf("number         =%9d\n",SuperSegments->number);
 printf("total size     =%9d Bytes\n",sizeof(SuperSegments)-sizeof(SuperSegments->segments)+SuperSegments->number*sizeof(Segment));

// for(i=0;i<SuperSegments->number;i++)
//   {
//    if(SuperSegments->segments[i].distance>longest)
//       longest=SuperSegments->segments[i].distance;
//    if(SuperSegments->segments[i].duration>slowest)
//       slowest=SuperSegments->segments[i].duration;
//   }
//
// printf("Longest distance = %.1f km\n",distance_to_km(longest));
// printf("Longest duration = %.1f min\n",duration_to_minutes(slowest));

 return(0);
}
