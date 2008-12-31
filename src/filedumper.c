/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.1 2008-12-31 12:19:54 amb Exp $

 Memory file dumper.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdio.h>
#include <stdlib.h>

#include "functions.h"
#include "types.h"


extern Nodes *OSMNodes;
extern Segments *OSMSegments;


int main(int argc,char** argv)
{
 int i;
 Node *node;
 Segment *segment;

 /* Check the nodes */

 LoadNodeList("data/nodes.mem");

 printf("alloced=%d\n",OSMNodes->alloced);
 printf("number=%d\n",OSMNodes->number);

 // for(i=0;i<OSMNodes->number;i++)
 //    printf("%8d %7.3f %7.3f\n",OSMNodes->nodes[i].id,OSMNodes->nodes[i].latitude,OSMNodes->nodes[i].longitude);

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

 /* Check the segments */

 LoadSegmentList("data/segments.mem");

 printf("alloced=%d\n",OSMSegments->alloced);
 printf("number=%d\n",OSMSegments->number);

 // for(i=0;i<OSMSegments->number;i++)
 //    printf("%8d %7.3f %7.3f\n",OSMSegments->segments[i].id,OSMSegments->segments[i].latitude,OSMSegments->segments[i].longitude);

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
