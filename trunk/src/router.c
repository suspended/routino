/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.41 2009-03-01 17:24:21 amb Exp $

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
#include <math.h>
#include <ctype.h>

#include "types.h"
#include "functions.h"
#include "profiles.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"


/*+ The option not to print any progress information. +*/
int option_quiet=0;

/*+ The option to calculate the quickest route insted of the shortest. +*/
int option_quickest=0;


int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Segments *OSMSegments;
 Ways     *OSMWays;
 index_t   start,finish;
 float     lon_start=999,lat_start=999,lon_finish=999,lat_finish=999;
 int       help_profile=0,help_profile_js=0,all=0,super=0,no_print=0;
 char     *dirname=NULL,*prefix=NULL,*filename;
 Transport transport=Transport_None;
 Profile   profile;
 int i;

 /* Parse the command line arguments */

 if(argc<5)
   {
   usage:

    fprintf(stderr,"Usage: router [--lon1=]<start-lon>  [--lat1=]<start-lat>\n"
                   "              [--lon2=]<finish-lon> [--lon2=]<finish-lat>\n"
                   "              [--help | --help-profile | -help-profile-js]\n"
                   "              [--dir=<name>] [--prefix=<name>]\n"
                   "              [--shortest | --quickest]\n"
                   "              [--all | --super]\n"
                   "              [--no-print] [--quiet]\n"
                   "              [--transport=<transport>]\n"
                   "              [--highway-<highway>=[0|1] ...]\n"
                   "              [--speed-<highway>=<speed> ...]\n"
                   "              [--oneway=[0|1]]\n"
                   "              [--weight=<weight>]\n"
                   "              [--height=<height>] [--width=<width>] [--length=<length>]\n"
                   "\n"
                   "<transport> defaults to motorcar but can be set to:\n"
                   "%s"
                   "\n"
                   "<highway> can be selected from:\n"
                   "%s"
                   "\n"
                   "<speed> is a speed in km/hour\n"
                   "<weight> is a weight in tonnes\n"
                   "<height>, <width>, <length> are dimensions in metres\n"
                   "\n",
                   TransportList(),HighwayList());

    return(1);
   }

 /* Get the transport type if specified and fill in the default profile */

 for(i=1;i<argc;i++)
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

 while(--argc>=1)
   {
    if(isdigit(argv[argc][0]) ||
       ((argv[argc][0]=='-' || argv[argc][0]=='+') && isdigit(argv[argc][1])))
      {
       if(lon_finish==999)
          lon_finish=(M_PI/180)*atof(argv[argc]);
       else if(lat_finish==999)
          lat_finish=(M_PI/180)*atof(argv[argc]);
       else if(lon_start==999)
          lon_start=(M_PI/180)*atof(argv[argc]);
       else if(lat_start==999)
          lat_start=(M_PI/180)*atof(argv[argc]);
       else
          goto usage;
      }
    else if(!strncmp(argv[argc],"--lat1=",7))
       lat_start=(M_PI/180)*atof(&argv[argc][7]);
    else if(!strncmp(argv[argc],"--lon1=",7))
       lon_start=(M_PI/180)*atof(&argv[argc][7]);
    else if(!strncmp(argv[argc],"--lat2=",7))
       lat_finish=(M_PI/180)*atof(&argv[argc][7]);
    else if(!strncmp(argv[argc],"--lon2=",7))
       lon_finish=(M_PI/180)*atof(&argv[argc][7]);
    else if(!strcmp(argv[argc],"--help"))
       goto usage;
    else if(!strcmp(argv[argc],"--help-profile"))
       help_profile=1;
    else if(!strcmp(argv[argc],"--help-profile-js"))
       help_profile_js=1;
    else if(!strncmp(argv[argc],"--dir=",6))
       dirname=&argv[argc][6];
    else if(!strncmp(argv[argc],"--prefix=",9))
       prefix=&argv[argc][9];
    else if(!strcmp(argv[argc],"--shortest"))
       option_quickest=0;
    else if(!strcmp(argv[argc],"--quickest"))
       option_quickest=1;
    else if(!strcmp(argv[argc],"--all"))
       all=1;
    else if(!strcmp(argv[argc],"--super"))
       super=1;
    else if(!strcmp(argv[argc],"--no-print"))
       no_print=1;
    else if(!strcmp(argv[argc],"--quiet"))
       option_quiet=1;
    else if(!strncmp(argv[argc],"--transport=",12))
       ; /* Done this already*/
    else if(!strncmp(argv[argc],"--highway-",10))
      {
       Highway highway;
       char *equal=strchr(argv[argc],'=');
       char *string;

       if(!equal)
          goto usage;

       string=strcpy((char*)malloc(strlen(argv[argc])),argv[argc]+10);
       string[equal-argv[argc]-10]=0;

       highway=HighwayType(string);

       free(string);

       if(highway==Way_Unknown)
          goto usage;

       profile.highways[highway]=atoi(equal+1);
      }
    else if(!strncmp(argv[argc],"--speed-",8))
      {
       Highway highway;
       char *equal=strchr(argv[argc],'=');
       char *string;

       if(!equal)
          goto usage;

       string=strcpy((char*)malloc(strlen(argv[argc])),argv[argc]+8);
       string[equal-argv[argc]-8]=0;

       highway=HighwayType(string);

       free(string);

       if(highway==Way_Unknown)
          goto usage;

       profile.speed[highway]=atoi(equal+1);
      }
    else if(!strncmp(argv[argc],"--oneway=",9))
       profile.oneway=atoi(&argv[argc][9]);
    else if(!strncmp(argv[argc],"--weight=",9))
       profile.weight=tonnes_to_weight(atof(&argv[argc][9]));
    else if(!strncmp(argv[argc],"--height=",9))
       profile.height=metres_to_height(atof(&argv[argc][9]));
    else if(!strncmp(argv[argc],"--width=",8))
       profile.width=metres_to_width(atof(&argv[argc][8]));
    else if(!strncmp(argv[argc],"--length=",9))
       profile.length=metres_to_length(atof(&argv[argc][9]));
    else
       goto usage;
   }

 if(help_profile)
   {
    PrintProfile(&profile);

    return(0);
   }
 else if(help_profile_js)
   {
    PrintProfilesJS();

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

   {
    distance_t dist_start=km_to_distance(10),dist_finish=km_to_distance(10);

    Node *start_node =FindNode(OSMNodes,lat_start ,lon_start ,&dist_start );
    Node *finish_node=FindNode(OSMNodes,lat_finish,lon_finish,&dist_finish);

    if(!start_node)
      {
       fprintf(stderr,"Cannot find start node.\n");
       return(1);
      }

    if(!finish_node)
      {
       fprintf(stderr,"Cannot find finish node.\n");
       return(1);
      }

    if(!option_quiet)
      {
       float lat,lon;

       GetLatLong(OSMNodes,start_node,&lat,&lon);

       printf("Start node : %3.6f %4.6f = %2.3f km\n",(180/M_PI)*lat,(180/M_PI)*lon,distance_to_km(dist_start));

       GetLatLong(OSMNodes,finish_node,&lat,&lon);

       printf("Finish node: %3.6f %4.6f = %2.3f km\n",(180/M_PI)*lat,(180/M_PI)*lon,distance_to_km(dist_finish));
      }

    start =IndexNode(OSMNodes,start_node );
    finish=IndexNode(OSMNodes,finish_node);

    if(super && !IsSuperNode(start_node) && !IsSuperNode(finish_node))
      {
       fprintf(stderr,"Start and/or finish nodes are not super-nodes.\n");
       return(1);
      }
   }

 /* Calculate the route. */

 if(all)
   {
    Results *results;

    /* Calculate the route */

    results=FindRoute(OSMNodes,OSMSegments,OSMWays,start,finish,&profile,all);

    /* Print the route */

    if(!results)
      {
       fprintf(stderr,"Cannot find route compatible with profile.\n");
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
       result->prev=0;
       result->next=0;
       result->distance=0;
       result->duration=0;
      }
    else
      {
       begin=FindStartRoutes(OSMNodes,OSMSegments,OSMWays,start,&profile);

       if(!begin)
         {
          fprintf(stderr,"Cannot find initial section of route compatible with profile.\n");
          return(1);
         }
      }

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
          result->prev=0;
          result->next=0;
          result->distance=0;
          result->duration=0;
         }
       else
         {
          end=FindFinishRoutes(OSMNodes,OSMSegments,OSMWays,finish,&profile);

          if(!end)
            {
             fprintf(stderr,"Cannot find final section of route compatible with profile.\n");
             return(1);
            }
         }

       /* Calculate the middle of the route */

       superresults=FindRoute3(OSMNodes,OSMSegments,OSMWays,start,finish,begin,end,&profile);

       /* Print the route */

       if(!superresults)
         {
          fprintf(stderr,"Cannot find route compatible with profile.\n");
          return(1);
         }
       else if(!no_print)
         {
          if(super)
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
