/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.17 2009-01-23 17:09:41 amb Exp $

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
 Segments *OSMSegments,*SuperSegments;
 Ways     *OSMWays,*SuperWays;
 node_t    start,finish;
 int       all=0,noprint=0;
 Transport transport=Transport_Motorcar;
 int       highways[Way_Unknown+1];
 int i;

 /* Parse the command line arguments */

 if(argc<3)
   {
   usage:

    fprintf(stderr,"Usage: router <start-node> <finish-node>\n"
                   "              [-help]\n"
                   "              [-all]\n"
                   "              [-no-print]\n"
                   "              [-transport=<transport>]\n"
                   "              [-not-highway=<highway> ...]\n"
                   "\n"
                   "<transport> can be:\n"
                   "%s"
                   "\n"
                   "<highway> can be:\n"
                   "%s",
                   TransportList(),HighwayList());

    return(1);
   }

 start=atoll(argv[1]);
 finish=atoll(argv[2]);

 for(i=0;i<Way_Unknown;i++)
    highways[i]=1;

 highways[Way_Unknown]=0;

 while(--argc>=3)
   {
    if(!strcmp(argv[argc],"-help"))
       goto usage;
    else if(!strcmp(argv[argc],"-all"))
       all=1;
    else if(!strcmp(argv[argc],"-no-print"))
       noprint=1;
    else if(!strncmp(argv[argc],"-transport=",11))
       transport=TransportType(&argv[argc][11]);
    else if(!strncmp(argv[argc],"-not-highway=",13))
      {
       Highway highway=HighwayType(&argv[argc][13]);
       highways[highway]=0;
      }
    else
       goto usage;
   }

 /* Load in the data */

 OSMNodes=LoadNodeList("data/nodes.mem");
 SuperNodes=LoadNodeList("data/super-nodes.mem");

 OSMSegments=LoadSegmentList("data/segments.mem");
 SuperSegments=LoadSegmentList("data/super-segments.mem");

 OSMWays=LoadWayList("data/ways.mem");
 SuperWays=LoadWayList("data/super-ways.mem");

 if(all)
   {
    Results *results;

    /* Calculate the route */

    results=FindRoute(OSMNodes,OSMSegments,OSMWays,start,finish,transport,highways,all);

    /* Print the route */

    if(!FindResult(results,finish))
       fprintf(stderr,"No route found.\n");
    else if(!noprint)
       PrintRoute(results,OSMNodes,OSMSegments,OSMWays,NULL,start,finish,transport);
   }
 else
   {
    Results *begin,*end;

    /* Calculate the beginning of the route */

    if(FindNode(SuperNodes,start))
      {
       Result *result;

       begin=NewResultsList(1);

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
       begin=FindRoutes(OSMNodes,OSMSegments,OSMWays,start,SuperNodes,transport,highways);

    if(FindResult(begin,finish))
      {
       /* Print the route */

       if(!noprint)
          PrintRoute(begin,OSMNodes,OSMSegments,OSMWays,NULL,start,finish,transport);
      }
    else
      {
       Results *superresults;

       /* Calculate the end of the route */

       if(FindNode(SuperNodes,finish))
         {
          Result *result;

          end=NewResultsList(1);

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
          end=FindReverseRoutes(OSMNodes,OSMSegments,OSMWays,SuperNodes,finish,transport,highways);

       /* Calculate the middle of the route */

       superresults=FindRoute3(SuperNodes,SuperSegments,SuperWays,start,finish,begin,end,transport,highways);

       /* Print the route */

       if(!FindResult(superresults,finish))
          fprintf(stderr,"No route found.\n");
       else if(!noprint)
         {
          Results *results=CombineRoutes(superresults,OSMNodes,OSMSegments,OSMWays,start,finish,transport,highways);

          PrintRoute(results,OSMNodes,OSMSegments,OSMWays,SuperNodes,start,finish,transport);
         }
      }
   }

 return(0);
}
