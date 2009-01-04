/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.4 2009-01-04 19:00:37 amb Exp $

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

 return(0);
}
