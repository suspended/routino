/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.3 2009-01-04 17:51:23 amb Exp $

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
 Node    *node;
 Way     *way;
 Segment *segment;

 /* Check the nodes */

 LoadNodeList("data/nodes.mem");

 printf("Nodes\n");
 printf("-----\n");

 printf("sizeof(Node)=%d\n",sizeof(Node));
 printf("number=%d\n",OSMNodes->number);

 node=FindNode(OSMNodes->nodes[0].id-1);
 printf("%s find node %d = %d\n",node?"Did":"Didn't",OSMNodes->nodes[0].id-1,node?node->id:0);

 node=FindNode(OSMNodes->nodes[0].id);
 printf("%s find node %d = %d\n",node?"Did":"Didn't",OSMNodes->nodes[0].id,node?node->id:0);

 node=FindNode(OSMNodes->nodes[OSMNodes->number-1].id);
 printf("%s find node %d = %d\n",node?"Did":"Didn't",OSMNodes->nodes[OSMNodes->number-1].id,node?node->id:0);

 node=FindNode(OSMNodes->nodes[OSMNodes->number-1].id+1);
 printf("%s find node %d = %d\n",node?"Did":"Didn't",OSMNodes->nodes[OSMNodes->number-1].id+1,node?node->id:0);

 node=FindNode(296954441);
 printf("%s find node %d = %d\n",node?"Did":"Didn't",296954441,node?node->id:0);

 node=FindNode(296954440);
 printf("%s find node %d = %d\n",node?"Did":"Didn't",296954440,node?node->id:0);

 /* Check the ways */

 LoadWayList("data/ways.mem");

 printf("\n");
 printf("Ways\n");
 printf("----\n");

 printf("sizeof(Way)=%d\n",sizeof(Way));
 printf("number=%d\n",OSMWays->number);
 printf("strings=%d\n",OSMWays->number_str);

 way=FindWay(OSMWays->ways[0].id-1);
 printf("%s find way %d = %d\n",way?"Did":"Didn't",OSMWays->ways[0].id-1,way?way->id:0);

 way=FindWay(OSMWays->ways[0].id);
 printf("%s find way %d = %d\n",way?"Did":"Didn't",OSMWays->ways[0].id,way?way->id:0);

 way=FindWay(OSMWays->ways[OSMWays->number-1].id);
 printf("%s find way %d = %d\n",way?"Did":"Didn't",OSMWays->ways[OSMWays->number-1].id,way?way->id:0);

 way=FindWay(OSMWays->ways[OSMWays->number-1].id+1);
 printf("%s find way %d = %d\n",way?"Did":"Didn't",OSMWays->ways[OSMWays->number-1].id+1,way?way->id:0);

 way=FindWay(296954441);
 printf("%s find way %d = %d\n",way?"Did":"Didn't",296954441,way?way->id:0);

 way=FindWay(296954440);
 printf("%s find way %d = %d\n",way?"Did":"Didn't",296954440,way?way->id:0);

 /* Check the segments */

 LoadSegmentList("data/segments.mem");

 printf("\n");
 printf("Segments\n");
 printf("--------\n");

 printf("sizeof(Segment)=%d\n",sizeof(Segment));
 printf("number=%d\n",OSMSegments->number);

 segment=FindFirstSegment(OSMSegments->segments[0].node1-1);
 printf("%s find segment %d = %d\n",segment?"Did":"Didn't",OSMSegments->segments[0].node1-1,segment?segment->node1:0);

 segment=FindFirstSegment(OSMSegments->segments[0].node1);
 printf("%s find segment %d = %d\n",segment?"Did":"Didn't",OSMSegments->segments[0].node1,segment?segment->node1:0);

 segment=FindFirstSegment(OSMSegments->segments[OSMSegments->number-1].node1);
 printf("%s find segment %d = %d\n",segment?"Did":"Didn't",OSMSegments->segments[OSMSegments->number-1].node1,segment?segment->node1:0);

 segment=FindFirstSegment(OSMSegments->segments[OSMSegments->number-1].node1+1);
 printf("%s find segment %d = %d\n",segment?"Did":"Didn't",OSMSegments->segments[OSMSegments->number-1].node1+1,segment?segment->node1:0);

 segment=FindFirstSegment(296954441);
 printf("%s find segment %d = %d\n",segment?"Did":"Didn't",296954441,segment?segment->node1:0);

 segment=FindFirstSegment(296954440);
 printf("%s find segment %d = %d\n",segment?"Did":"Didn't",296954440,segment?segment->node1:0);

 return(0);
}
