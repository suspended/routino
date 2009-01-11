/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.6 2009-01-11 09:42:26 amb Exp $

 OSM router.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nodes.h"
#include "ways.h"
#include "segments.h"
#include "functions.h"


int main(int argc,char** argv)
{
 Nodes    *OSMNodes,*SuperNodes;
 Ways     *OSMWays;
 Segments *OSMSegments,*SuperSegments;
 Results  *results;
 node_t start,finish;

 /* Parse the command line aarguments */

 if(argc!=3 && argc!=4)
   {
    fprintf(stderr,"Usage: %s <start-node> <finish-node>\n",argv[0]);
    return(1);
   }

 start=atoll(argv[1]);
 finish=atoll(argv[2]);

 /* Load in the data */

 OSMNodes=LoadNodeList("data/nodes.mem");
 SuperNodes=LoadNodeList("data/super-nodes.mem");

 OSMWays=LoadWayList("data/ways.mem");

 OSMSegments=LoadSegmentList("data/segments.mem");
 SuperSegments=LoadSegmentList("data/super-segments.mem");

 if(argc>3 && !strcmp(argv[3],"-all"))
   {
    /* Calculate the route */

    results=FindRoute(OSMNodes,OSMSegments,start,finish);

    /* Print the route */

    PrintRoute(results,OSMNodes,OSMSegments,OSMWays,start,finish);
   }
 else
   {
    /* Calculate the route */

    results=FindRoute(SuperNodes,SuperSegments,start,finish);

    /* Print the route */

    PrintRoutes(results,OSMNodes,OSMSegments,OSMWays,SuperNodes,SuperSegments,start,finish);
   }

 return(0);
}
