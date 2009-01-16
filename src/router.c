/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.8 2009-01-16 20:04:47 amb Exp $

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
    Results *results;

    /* Calculate the route */

    results=FindRoute(OSMNodes,OSMSegments,start,finish);

    /* Print the route */

    PrintRoute(results,OSMNodes,OSMSegments,OSMWays,start,finish);
   }
 else
   {
    Results *begin,*middle,*end;

    /* Calculate the beginning of the route */

    if(FindNode(SuperNodes,start))
      {
       Result *result;

       begin=NewResultsList();

       result=InsertResult(begin,start);

       result->node=start;
       result->shortest.prev=0;
       result->shortest.next=0;
       result->shortest.distance=0;
       result->shortest.duration=0;
       result->quickest.prev=0;
       result->quickest.next=0;
       result->quickest.distance=0;
       result->quickest.duration=0;
      }
    else
       begin=FindRoutes(OSMNodes,OSMSegments,start,SuperNodes);

    if(FindResult(begin,finish))
      {
       Results *results;

       /* Calculate the route */

       results=FindRoute(OSMNodes,OSMSegments,start,finish);

       /* Print the route */

       PrintRoute(results,OSMNodes,OSMSegments,OSMWays,start,finish);
      }
    else
      {
       /* Calculate the end of the route */

       if(FindNode(SuperNodes,finish))
         {
          Result *result;

          end=NewResultsList();

          result=InsertResult(end,finish);

          result->node=finish;
          result->shortest.prev=0;
          result->shortest.next=0;
          result->shortest.distance=0;
          result->shortest.duration=0;
          result->quickest.prev=0;
          result->quickest.next=0;
          result->quickest.distance=0;
          result->quickest.duration=0;
         }
       else
          end=FindReverseRoutes(OSMNodes,OSMSegments,SuperNodes,finish);

       /* Calculate the middle of the route */

       middle=FindRoute3(SuperNodes,SuperSegments,start,finish,begin,end);

       /* Print the route */

       PrintRoutes(middle,OSMNodes,OSMSegments,OSMWays,SuperNodes,SuperSegments,start,finish);
      }
   }

 return(0);
}
