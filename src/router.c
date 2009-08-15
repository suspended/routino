/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.57 2009-08-15 15:28:27 amb Exp $

 OSM router.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008,2009 Andrew M. Bishop

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
 Results  *results[10]={NULL};
 int       point_used[10]={0};
 double    point_lon[10],point_lat[10];
 int       help_profile=0,help_profile_js=0,help_profile_pl=0;
 char     *dirname=NULL,*prefix=NULL,*filename;
 Transport transport=Transport_None;
 Profile   profile;
 index_t   start=NO_NODE,finish=NO_NODE;
 int       arg,node;

 /* Parse the command line arguments */

 if(argc<2)
   {
   usage:

    fprintf(stderr,"Usage: router [--lon1=]<longitude> [--lat1=]<latitude>\n"
                   "              [--lon2=]<longitude> [--lon2=]<latitude>\n"
                   "              [ ... [--lon9=]<longitude> [--lon9=]<latitude> ]\n"
                   "              [--help | --help-profile | --help-profile-js | --help-profile-pl]\n"
                   "              [--dir=<name>] [--prefix=<name>]\n"
                   "              [--shortest | --quickest]\n"
                   "              [--quiet]\n"
                   "              [--transport=<transport>]\n"
                   "              [--highway-<highway>=<preference> ...]\n"
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
                   "<preference> is a preference expressed as a percentage\n"
                   "<speed> is a speed in km/hour\n"
                   "<weight> is a weight in tonnes\n"
                   "<height>, <width>, <length> are dimensions in metres\n"
                   "\n",
                   TransportList(),HighwayList());

    return(1);
   }

 /* Get the transport type if specified and fill in the default profile */

 for(arg=1;arg<argc;arg++)
    if(!strncmp(argv[arg],"--transport=",12))
      {
       transport=TransportType(&argv[arg][12]);

       if(transport==Transport_None)
          goto usage;
      }

 if(transport==Transport_None)
    transport=Transport_Motorcar;

 profile=*GetProfile(transport);

 /* Parse the other command line arguments */

 for(arg=1;arg<argc;arg++)
   {
    if(isdigit(argv[arg][0]) ||
       ((argv[arg][0]=='-' || argv[arg][0]=='+') && isdigit(argv[arg][1])))
      {
       for(node=1;node<sizeof(point_used)/sizeof(point_used[0]);node++)
          if(point_used[node]!=3)
            {
             if(point_used[node]==0)
               {
                point_lon[node]=degrees_to_radians(atof(argv[arg]));
                point_used[node]=1;
               }
             else /* if(point_used[node]==1) */
               {
                point_lat[node]=degrees_to_radians(atof(argv[arg]));
                point_used[node]=3;
               }
             break;
            }
      }
    else if(!strncmp(argv[arg],"--lon",5) && isdigit(argv[arg][5]) && argv[arg][6]=='=')
      {
       node=atoi(&argv[arg][5]);
       if(point_used[node]&1)
          goto usage;
       point_lon[node]=degrees_to_radians(atof(&argv[arg][7]));
       point_used[node]+=1;
      }
    else if(!strncmp(argv[arg],"--lat",5) && isdigit(argv[arg][5]) && argv[arg][6]=='=')
      {
       node=atoi(&argv[arg][5]);
       if(point_used[node]&2)
          goto usage;
       point_lat[node]=degrees_to_radians(atof(&argv[arg][7]));
       point_used[node]+=2;
      }
    else if(!strcmp(argv[arg],"--help"))
       goto usage;
    else if(!strcmp(argv[arg],"--help-profile"))
       help_profile=1;
    else if(!strcmp(argv[arg],"--help-profile-js"))
       help_profile_js=1;
    else if(!strcmp(argv[arg],"--help-profile-pl"))
       help_profile_pl=1;
    else if(!strncmp(argv[arg],"--dir=",6))
       dirname=&argv[arg][6];
    else if(!strncmp(argv[arg],"--prefix=",9))
       prefix=&argv[arg][9];
    else if(!strcmp(argv[arg],"--shortest"))
       option_quickest=0;
    else if(!strcmp(argv[arg],"--quickest"))
       option_quickest=1;
    else if(!strcmp(argv[arg],"--quiet"))
       option_quiet=1;
    else if(!strncmp(argv[arg],"--transport=",12))
       ; /* Done this already*/
    else if(!strncmp(argv[arg],"--highway-",10))
      {
       Highway highway;
       char *equal=strchr(argv[arg],'=');
       char *string;

       if(!equal)
          goto usage;

       string=strcpy((char*)malloc(strlen(argv[arg])),argv[arg]+10);
       string[equal-argv[arg]-10]=0;

       highway=HighwayType(string);

       free(string);

       if(highway==Way_Unknown)
          goto usage;

       profile.highway[highway]=atof(equal+1);
      }
    else if(!strncmp(argv[arg],"--speed-",8))
      {
       Highway highway;
       char *equal=strchr(argv[arg],'=');
       char *string;

       if(!equal)
          goto usage;

       string=strcpy((char*)malloc(strlen(argv[arg])),argv[arg]+8);
       string[equal-argv[arg]-8]=0;

       highway=HighwayType(string);

       free(string);

       if(highway==Way_Unknown)
          goto usage;

       profile.speed[highway]=kph_to_speed(atof(equal+1));
      }
    else if(!strncmp(argv[arg],"--oneway=",9))
       profile.oneway=!!atoi(&argv[arg][9]);
    else if(!strncmp(argv[arg],"--weight=",9))
       profile.weight=tonnes_to_weight(atof(&argv[arg][9]));
    else if(!strncmp(argv[arg],"--height=",9))
       profile.height=metres_to_height(atof(&argv[arg][9]));
    else if(!strncmp(argv[arg],"--width=",8))
       profile.width=metres_to_width(atof(&argv[arg][8]));
    else if(!strncmp(argv[arg],"--length=",9))
       profile.length=metres_to_length(atof(&argv[arg][9]));
    else
       goto usage;
   }

 for(node=0;node<sizeof(point_used)/sizeof(point_used[0]);node++)
    if(point_used[node]==1 || point_used[node]==2)
       goto usage;

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
 else if(help_profile_pl)
   {
    PrintProfilesPerl();

    return(0);
   }

 UpdateProfile(&profile);

 /* Load in the data */

 OSMNodes=LoadNodeList(filename=FileName(dirname,prefix,"nodes.mem"));

 if(!OSMNodes)
   {
    fprintf(stderr,"Error: Cannot open nodes file '%s'.\n",filename);
    return(1);
   }

 OSMSegments=LoadSegmentList(filename=FileName(dirname,prefix,"segments.mem"));

 if(!OSMSegments)
   {
    fprintf(stderr,"Error: Cannot open segments file '%s'.\n",filename);
    return(1);
   }

 OSMWays=LoadWayList(filename=FileName(dirname,prefix,"ways.mem"));

 if(!OSMWays)
   {
    fprintf(stderr,"Error: Cannot open ways file '%s'.\n",filename);
    return(1);
   }

 /* Loop through all pairs of nodes */

 for(node=1;node<sizeof(point_used)/sizeof(point_used[0]);node++)
   {
    Results *begin,*end;
    distance_t dist=km_to_distance(10);

    if(point_used[node]!=3)
       continue;

    /* Find the node */

    start=finish;

    finish=FindNode(OSMNodes,OSMSegments,OSMWays,point_lat[node],point_lon[node],&dist,&profile);

    if(finish==NO_NODE)
      {
       fprintf(stderr,"Error: Cannot find node close to specified point %d.\n",node);
       return(1);
      }

    if(!option_quiet)
      {
       double lat,lon;

       GetLatLong(OSMNodes,finish,&lat,&lon);

       printf("Node %d: %3.6f %4.6f = %2.3f km\n",node,radians_to_degrees(lon),radians_to_degrees(lat),distance_to_km(dist));
      }

    if(start==NO_NODE)
       continue;

    /* Calculate the beginning of the route */

    if(IsSuperNode(OSMNodes,start))
      {
       Result *result;

       begin=NewResultsList(1);

       begin->start=start;

       result=InsertResult(begin,start);

       ZeroResult(result);
      }
    else
      {
       begin=FindStartRoutes(OSMNodes,OSMSegments,OSMWays,start,&profile);

       if(!begin)
         {
          fprintf(stderr,"Error: Cannot find initial section of route compatible with profile.\n");
          return(1);
         }
      }

    if(FindResult(begin,finish))
      {
       results[node]=begin;

       results[node]->finish=finish;
      }
    else
      {
       Results *superresults;

       /* Calculate the end of the route */

       if(IsSuperNode(OSMNodes,finish))
         {
          Result *result;

          end=NewResultsList(1);

          end->finish=finish;

          result=InsertResult(end,finish);

          ZeroResult(result);
         }
       else
         {
          end=FindFinishRoutes(OSMNodes,OSMSegments,OSMWays,finish,&profile);

          if(!end)
            {
             fprintf(stderr,"Error: Cannot find final section of route compatible with profile.\n");
             return(1);
            }
         }

       /* Calculate the middle of the route */

       superresults=FindMiddleRoute(OSMNodes,OSMSegments,OSMWays,begin,end,&profile);

       if(!superresults)
         {
          fprintf(stderr,"Error: Cannot find route compatible with profile.\n");
          return(1);
         }

       results[node]=CombineRoutes(superresults,OSMNodes,OSMSegments,OSMWays,&profile);
      }
   }

 /* Print out the combined route */

 PrintRouteHead(FileName(dirname,prefix,"copyright.txt"));

 for(node=1;node<sizeof(point_used)/sizeof(point_used[0]);node++)
    if(results[node])
       PrintRoute(results[node],OSMNodes,OSMSegments,OSMWays,&profile);

 PrintRouteTail();

 return(0);
}
