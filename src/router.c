/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.29 2009-02-04 19:11:38 amb Exp $

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

#include "types.h"
#include "functions.h"
#include "profiles.h"


int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Segments *OSMSegments;
 Ways     *OSMWays;
 node_t    start,finish;
 int       help_profile=0,all=0,only_super=0,no_print=0;
 char     *dirname=NULL,*prefix=NULL,*filename;
 Transport transport=Transport_None;
 Profile   profile;
 int i;

 /* Parse the command line arguments */

 if(argc<5)
   {
   usage:

    fprintf(stderr,"Usage: router <start-lat> <start-lon> <finish-lat> <finish-lon>\n"
                   "              [--help] [--help-profile]\n"
                   "              [--dir=<name>] [--prefix=<name>]\n"
                   "              [--all] [--only-super]\n"
                   "              [--no-print]\n"
                   "              [--transport=<transport>]\n"
                   "              [--not-highway=<highway> ...]\n"
                   "              [--speed-<highway>=<speed> ...]\n"
                   "              [--ignore-oneway]\n"
                   "\n"
                   "<transport> defaults to motorcar but can be set to:\n"
                   "%s"
                   "\n"
                   "<highway> can be selected from:\n"
                   "%s"
                   "\n"
                   "<speed> is a speed in km/hour\n"
                   "\n",
                   TransportList(),HighwayList());

    return(1);
   }

 /* Get the transport type if specified and fill in the profile */

 for(i=3;i<argc;i++)
    if(!strncmp(argv[i],"--transport=",12))
      {
       transport=TransportType(&argv[i][12]);

       if(transport==Transport_None)
          goto usage;
      }

 if(transport==Transport_None)
    transport=Transport_Motorcar;

 profile=*GetProfile(transport);

 /* Parse the other command line arguments */

 while(--argc>=5)
   {
    if(!strcmp(argv[argc],"--help"))
       goto usage;
    else if(!strcmp(argv[argc],"--help-profile"))
       help_profile=1;
    else if(!strncmp(argv[argc],"--dir=",6))
       dirname=&argv[argc][6];
    else if(!strncmp(argv[argc],"--prefix=",9))
       prefix=&argv[argc][9];
    else if(!strcmp(argv[argc],"--all"))
       all=1;
    else if(!strcmp(argv[argc],"--only-super"))
       only_super=1;
    else if(!strcmp(argv[argc],"--no-print"))
       no_print=1;
    else if(!strncmp(argv[argc],"--transport=",12))
       ; /* Done this already*/
    else if(!strncmp(argv[argc],"--not-highway=",14))
      {
       Highway highway=HighwayType(&argv[argc][14]);

       if(highway==Way_Unknown)
          goto usage;

       profile.highways[highway]=0;
      }
    else if(!strncmp(argv[argc],"--speed-",8))
      {
       Highway highway;
       char *equal=strchr(argv[argc],'=');
       char *string;

       if(!equal)
          goto usage;

       string=strcpy((char*)malloc(strlen(argv[argc])),argv[argc]+8);
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

 filename=(char*)malloc((dirname?strlen(dirname):0)+(prefix?strlen(prefix):0)+16);

 sprintf(filename,"%s%s%s%snodes.mem",dirname?dirname:"",dirname?"/":"",prefix?prefix:"",prefix?"-":"");
 OSMNodes=LoadNodeList(filename);

 if(!OSMNodes)
   {
    fprintf(stderr,"Cannot open nodes file '%s'.\n",filename);
    return(1);
   }

 sprintf(filename,"%s%s%s%ssegments.mem",dirname?dirname:"",dirname?"/":"",prefix?prefix:"",prefix?"-":"");
 OSMSegments=LoadSegmentList(filename);

 if(!OSMSegments)
   {
    fprintf(stderr,"Cannot open segments file '%s'.\n",filename);
    return(1);
   }

 sprintf(filename,"%s%s%s%sways.mem",dirname?dirname:"",dirname?"/":"",prefix?prefix:"",prefix?"-":"");
 OSMWays=LoadWayList(filename);

 if(!OSMWays)
   {
    fprintf(stderr,"Cannot open ways file '%s'.\n",filename);
    return(1);
   }

 /* Get the start and finish */

 start=IndexNode(OSMNodes,FindNode(OSMNodes,atof(argv[1]),atof(argv[2])));
 finish=IndexNode(OSMNodes,FindNode(OSMNodes,atof(argv[3]),atof(argv[4])));

 /* Calculate the route. */

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

    /* Calculate the beginning of the route */

    if(IsSuperNode(LookupNode(OSMNodes,start)))
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
       begin=FindRoutes(OSMNodes,OSMSegments,OSMWays,start,&profile);

    if(FindResult(begin,finish))
      {
       /* Print the route */

       if(!no_print)
          PrintRoute(begin,OSMNodes,OSMSegments,OSMWays,start,finish,&profile);
      }
    else
      {
       Results *superresults;

       /* Calculate the end of the route */

       if(IsSuperNode(LookupNode(OSMNodes,finish)))
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
          end=FindReverseRoutes(OSMNodes,OSMSegments,OSMWays,finish,&profile);

       /* Calculate the middle of the route */

       superresults=FindRoute3(OSMNodes,OSMSegments,OSMWays,start,finish,begin,end,&profile);

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
             PrintRoute(superresults,OSMNodes,OSMSegments,OSMWays,start,finish,&profile);
            }
          else
            {
             Results *results=CombineRoutes(superresults,OSMNodes,OSMSegments,OSMWays,start,finish,&profile);

             PrintRoute(results,OSMNodes,OSMSegments,OSMWays,start,finish,&profile);
            }
         }
      }
   }

 return(0);
}
