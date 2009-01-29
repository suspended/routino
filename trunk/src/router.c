/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.21 2009-01-29 19:31:52 amb Exp $

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
#include "segments.h"
#include "ways.h"
#include "functions.h"
#include "profiles.h"


int main(int argc,char** argv)
{
 Nodes    *OSMNodes,*SuperNodes;
 Segments *OSMSegments,*SuperSegments;
 Ways     *OSMWays,*SuperWays;
 node_t    start,finish;
 int       help_profile=0,all=0,only_super=0,no_print=0;
 Transport transport=Transport_None;
 Profile   profile;
 int i;

 /* Parse the command line arguments */

 if(argc<3)
   {
   usage:

    fprintf(stderr,"Usage: router <start-node> <finish-node>\n"
                   "              [-help] [-help-profile]\n"
                   "              [-all] [-only-super]\n"
                   "              [-no-print]\n"
                   "              [-transport=<transport>]\n"
                   "              [-not-highway=<highway> ...]\n"
                   "              [-speed-<highway>=<speed> ...]\n"
                   "              [-ignore-oneway]\n"
                   "\n"
                   "<transport> defaults to motorcar but can be set to:\n"
                   "%s"
                   "\n"
                   "<highway> can be selected from:\n"
                   "%s"
                   "<speed> is a speed in km/hour\n"
                   "\n",
                   TransportList(),HighwayList());

    return(1);
   }

 /* Get the start and finish */

 start=atoll(argv[1]);
 finish=atoll(argv[2]);

 /* Get the transport type if specified and fill in the profile */

 for(i=3;i<argc;i++)
    if(!strncmp(argv[i],"-transport=",11))
      {
       transport=TransportType(&argv[i][11]);

       if(transport==Transport_None)
          goto usage;
      }

 if(transport==Transport_None)
    transport=Transport_Motorcar;

 profile=*GetProfile(transport);

 /* Parse the other command line arguments */

 while(--argc>=3)
   {
    if(!strcmp(argv[argc],"-help"))
       goto usage;
    else if(!strcmp(argv[argc],"-help-profile"))
       help_profile=1;
    else if(!strcmp(argv[argc],"-all"))
       all=1;
    else if(!strcmp(argv[argc],"-only-super"))
       only_super=1;
    else if(!strcmp(argv[argc],"-no-print"))
       no_print=1;
    else if(!strncmp(argv[argc],"-transport=",11))
       ; /* Done this already*/
    else if(!strncmp(argv[argc],"-not-highway=",13))
      {
       Highway highway=HighwayType(&argv[argc][13]);

       if(highway==Way_Unknown)
          goto usage;

       profile.highways[highway]=0;
      }
    else if(!strncmp(argv[argc],"-speed-",7))
      {
       Highway highway;
       char *equal=strchr(argv[argc],'=');
       char *string;

       if(!equal)
          goto usage;

       string=strcpy((char*)malloc(strlen(argv[argc])),argv[argc]+7);
       string[equal-argv[argc]]=0;

       highway=HighwayType(string);

       free(string);

       if(highway==Way_Unknown)
          goto usage;

       profile.speed[highway]=atoi(equal+1);
      }
    else if(!strcmp(argv[argc],"-ignore-oneway"))
       profile.oneway=0;
    else
       goto usage;
   }

 if(help_profile)
   {
    PrintProfile(&profile);

    return(0);
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

    results=FindRoute(OSMNodes,OSMSegments,OSMWays,start,finish,&profile,all);

    /* Print the route */

    if(!results)
      {
       fprintf(stderr,"No route found.\n");
       return(1);
      }
    else if(!no_print)
       PrintRoute(results,OSMNodes,OSMSegments,OSMWays,start,finish,&profile);
   }
 else
   {
    Results *begin,*end;

#if 0

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
       begin=FindRoutes(OSMNodes,OSMSegments,OSMWays,start,SuperNodes,&profile);

    if(FindResult(begin,finish))
      {
       /* Print the route */

       if(!no_print)
          PrintRoute(begin,OSMNodes,OSMSegments,OSMWays,NULL,start,finish,&profile);
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
          end=FindReverseRoutes(OSMNodes,OSMSegments,OSMWays,SuperNodes,finish,&profile);

       /* Calculate the middle of the route */

       superresults=FindRoute3(SuperNodes,SuperSegments,SuperWays,start,finish,begin,end,&profile);

       /* Print the route */

       if(!superresults)
         {
          fprintf(stderr,"No route found.\n");
          return(1);
         }
       else if(!no_print)
         {
          if(only_super)
            {
             PrintRoute(superresults,SuperNodes,SuperSegments,SuperWays,NULL,start,finish,&profile);
            }
          else
            {
             Results *results=CombineRoutes(superresults,OSMNodes,OSMSegments,OSMWays,start,finish,&profile);

             PrintRoute(results,OSMNodes,OSMSegments,OSMWays,SuperNodes,start,finish,&profile);
            }
         }
      }

#endif
   }

 return(0);
}
