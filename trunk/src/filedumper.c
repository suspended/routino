/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.13 2009-01-22 19:15:30 amb Exp $

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
 Segments *OSMSegments;
 Ways     *OSMWays;
 Nodes    *SuperNodes;
 Segments *SuperSegments;
 Ways     *SuperWays;

 /* Examine the nodes */

 OSMNodes=LoadNodeList("data/nodes.mem");

 printf("Nodes\n");
 printf("-----\n");

 printf("sizeof(Node)=%9d Bytes\n",sizeof(Node));
 printf("number      =%9d\n",OSMNodes->number);
 printf("total size  =%9d Bytes\n",sizeof(OSMNodes)-sizeof(OSMNodes->nodes)+OSMNodes->number*sizeof(Node));

 /* Examine the segments */

 OSMSegments=LoadSegmentList("data/segments.mem");

 printf("\n");
 printf("Segments\n");
 printf("--------\n");

 printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
 printf("number         =%9d\n",OSMSegments->number);
 printf("total size     =%9d Bytes\n",sizeof(OSMSegments)-sizeof(OSMSegments->segments)+OSMSegments->number*sizeof(Segment));

 /* Examine the ways */

 OSMWays=LoadWayList("data/ways.mem");

 printf("\n");
 printf("Ways\n");
 printf("----\n");

 printf("sizeof(Way) =%9d Bytes\n",sizeof(Way));
 printf("number      =%9d\n",OSMWays->number);
 printf("total size  =%9d Bytes\n",sizeof(OSMWays)-sizeof(OSMWays->ways)+OSMWays->number*sizeof(Way));

 /* Examine the super-nodes */

 SuperNodes=LoadNodeList("data/super-nodes.mem");

 printf("\n");
 printf("SuperNodes\n");
 printf("----------\n");

 printf("sizeof(Node)=%9d Bytes\n",sizeof(Node));
 printf("number      =%9d\n",SuperNodes->number);
 printf("total size  =%9d Bytes\n",sizeof(SuperNodes)-sizeof(SuperNodes->nodes)+SuperNodes->number*sizeof(Node));

 /* Examine the super-segments */

 SuperSegments=LoadSegmentList("data/super-segments.mem");

 printf("\n");
 printf("SuperSegments\n");
 printf("-------------\n");

 printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
 printf("number         =%9d\n",SuperSegments->number);
 printf("total size     =%9d Bytes\n",sizeof(SuperSegments)-sizeof(SuperSegments->segments)+SuperSegments->number*sizeof(Segment));

 /* Examine the super-ways */

 SuperWays=LoadWayList("data/super-ways.mem");

 printf("\n");
 printf("Super-Ways\n");
 printf("----------\n");

 printf("sizeof(Way) =%9d Bytes\n",sizeof(Way));
 printf("number      =%9d\n",SuperWays->number);
 printf("total size  =%9d Bytes\n",sizeof(SuperWays)-sizeof(SuperWays->ways)+SuperWays->number*sizeof(Way));

 return(0);
}
