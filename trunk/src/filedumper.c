/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.16 2009-02-04 18:23:32 amb Exp $

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

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "functions.h"


int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Segments *OSMSegments;
 Ways     *OSMWays;

 /* Examine the nodes */

 OSMNodes=LoadNodeList("data/nodes.mem");

 printf("Nodes\n");
 printf("-----\n");

 printf("sizeof(Node)=%9d Bytes\n",sizeof(Node));
 printf("number      =%9d\n",OSMNodes->number);

 printf("Lat bins=%3d\n",OSMNodes->latbins);
 printf("Lon bins=%3d\n",OSMNodes->lonbins);

 printf("Lat zero=%4.6f\n",OSMNodes->latzero);
 printf("Lon zero=%4.6f\n",OSMNodes->lonzero);

 /* Examine the segments */

 OSMSegments=LoadSegmentList("data/segments.mem");

 printf("\n");
 printf("Segments\n");
 printf("--------\n");

 printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
 printf("number         =%9d\n",OSMSegments->number);

 /* Examine the ways */

 OSMWays=LoadWayList("data/ways.mem");

 printf("\n");
 printf("Ways\n");
 printf("----\n");

 printf("sizeof(Way) =%9d Bytes\n",sizeof(Way));
 printf("number      =%9d\n",OSMWays->number);

 return(0);
}
