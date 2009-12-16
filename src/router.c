/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.66 2009-12-16 19:30:27 amb Exp $

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


/*+ The number of waypoints allowed to be specified. +*/
#define NWAYPOINTS 99

/*+ The maximum distance from the specified point to search for a node or segment (in km). +*/
#define MAXSEARCH  5

/*+ The minimum distance along a segment from a node to insert a fake node. (in km). +*/
#define MINSEGMENT 0.005


/*+ A set of fake segments to allow start/finish in the middle of a segment. +*/
static Segment fake_segments[2*NWAYPOINTS];

/*+ A set of fake node latitudes and longitudes. +*/
static double point_lon[NWAYPOINTS+1],point_lat[NWAYPOINTS+1];

/*+ The option not to print any progress information. +*/
int option_quiet=0;

/*+ The option to calculate the quickest route insted of the shortest. +*/
int option_quickest=0;


int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Segments *OSMSegments;
 Ways     *OSMWays;
 Results  *results[NWAYPOINTS+1]={NULL};
 int       point_used[NWAYPOINTS+1]={0};
 int       help_profile=0,help_profile_js=0,help_profile_pl=0;
 char     *dirname=NULL,*prefix=NULL,*filename;
 int       exactnodes=0;
 Transport transport=Transport_None;
 Profile   profile;
 index_t   start=NO_NODE,finish=NO_NODE;
 int       arg,point;

 /* Parse the command line arguments */

 if(argc<2)
   {
   usage:

    fprintf(stderr,"Usage: router [--lon1=]<longitude> [--lat1=]<latitude>\n"
                   "              [--lon2=]<longitude> [--lon2=]<latitude>\n"
                   "              [ ... [--lon99=]<longitude> [--lon99=]<latitude>]\n"
                   "              [--help | --help-profile | --help-profile-js | --help-profile-pl]\n"
                   "              [--dir=<name>] [--prefix=<name>]\n"
                   "              [--shortest | --quickest]\n"
                   "              [--exact-nodes-only]\n"
                   "              [--quiet]\n"
                   "              [--transport=<transport>]\n"
                   "              [--highway-<highway>=<preference> ...]\n"
                   "              [--speed-<highway>=<speed> ...]\n"
                   "              [--property-<property>=<preference> ...]\n"
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
                   "<property> can be selected from:\n"
                   "%s"
                   "\n"
                   "<preference> is a preference expressed as a percentage\n"
                   "<speed> is a speed in km/hour\n"
                   "<weight> is a weight in tonnes\n"
                   "<height>, <width>, <length> are dimensions in metres\n"
                   "\n",
            TransportList(),HighwayList(),PropertyList());

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
       for(point=1;point<=NWAYPOINTS;point++)
          if(point_used[point]!=3)
            {
             if(point_used[point]==0)
               {
                point_lon[point]=degrees_to_radians(atof(argv[arg]));
                point_used[point]=1;
               }
             else /* if(point_used[point]==1) */
               {
                point_lat[point]=degrees_to_radians(atof(argv[arg]));
                point_used[point]=3;
               }
             break;
            }
      }
     else if(!strncmp(argv[arg],"--lon",5) && isdigit(argv[arg][5]))
       {
        char *p=&argv[arg][6];
        while(isdigit(*p)) p++;
        if(*p++!='=')
           goto usage;
 
        point=atoi(&argv[arg][5]);
        if(point>NWAYPOINTS || point_used[point]&1)
           goto usage;
 
       point_lon[point]=degrees_to_radians(atof(p));
       point_used[point]+=1;
      }
     else if(!strncmp(argv[arg],"--lat",5) && isdigit(argv[arg][5]))
       {
        char *p=&argv[arg][6];
        while(isdigit(*p)) p++;
        if(*p++!='=')
           goto usage;
 
        point=atoi(&argv[arg][5]);
        if(point>NWAYPOINTS || point_used[point]&2)
           goto usage;
 
       point_lat[point]=degrees_to_radians(atof(p));
       point_used[point]+=2;
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
    else if(!strcmp(argv[arg],"--exact-nodes-only"))
       exactnodes=1;
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

       if(highway==Way_Count)
          goto usage;

       profile.highway[highway]=atof(equal+1);

       free(string);
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

       if(highway==Way_Count)
          goto usage;

       profile.speed[highway]=kph_to_speed(atof(equal+1));

       free(string);
      }
    else if(!strncmp(argv[arg],"--property-",11))
      {
       Property property;
       char *equal=strchr(argv[arg],'=');
       char *string;

       if(!equal)
          goto usage;

       string=strcpy((char*)malloc(strlen(argv[arg])),argv[arg]+11);
       string[equal-argv[arg]-11]=0;

       property=PropertyType(string);

       if(property==Way_Count)
          goto usage;

       profile.props_yes[property]=atof(equal+1);

       free(string);
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

 for(point=1;point<=NWAYPOINTS;point++)
    if(point_used[point]==1 || point_used[point]==2)
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

 if(!(profile.allow & OSMWays->allow))
   {
    fprintf(stderr,"Error: Database was not generated for selected transport.\n");
    return(1);
   }

 /* Loop through all pairs of points */

 for(point=1;point<=NWAYPOINTS;point++)
   {
    Results *begin,*end;
    distance_t distmax=km_to_distance(MAXSEARCH);
    distance_t distmin;
    Segment *segment=NULL;
    index_t node1,node2;

    if(point_used[point]!=3)
       continue;

    /* Find the closest point */

    start=finish;

    if(exactnodes)
      {
       finish=FindClosestNode(OSMNodes,OSMSegments,OSMWays,point_lat[point],point_lon[point],distmax,&profile,&distmin);
      }
    else
      {
       distance_t dist1,dist2;

       segment=FindClosestSegment(OSMNodes,OSMSegments,OSMWays,point_lat[point],point_lon[point],distmax,&profile,&distmin,&node1,&node2,&dist1,&dist2);

       finish=CreateFakes(OSMNodes,point,segment,node1,node2,dist1,dist2);
      }

    if(finish==NO_NODE)
      {
       fprintf(stderr,"Error: Cannot find node close to specified point %d.\n",point);
       return(1);
      }

    if(!option_quiet)
      {
       double lat,lon;

       if(IsFakeNode(finish))
          GetFakeLatLong(finish,&lat,&lon);
       else
          GetLatLong(OSMNodes,finish,&lat,&lon);

       if(IsFakeNode(finish))
          printf("Point %d is segment %d (node %d -> %d): %3.6f %4.6f = %2.3f km\n",point,IndexSegment(OSMSegments,segment),node1,node2,
                 radians_to_degrees(lon),radians_to_degrees(lat),distance_to_km(distmin));
       else
          printf("Point %d is node %d: %3.6f %4.6f = %2.3f km\n",point,finish,
                 radians_to_degrees(lon),radians_to_degrees(lat),distance_to_km(distmin));
      }

    if(start==NO_NODE)
       continue;

    /* Calculate the beginning of the route */

    if(!IsFakeNode(start) && IsSuperNode(OSMNodes,start))
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
       FixForwardRoute(begin,finish);

       results[point]=begin;

       if(!option_quiet)
         {
          printf("\rRouted: Super-Nodes Checked = %d\n",begin->number);
          fflush(stdout);
         }
      }
    else
      {
       Results *superresults;

       /* Calculate the end of the route */

       if(!IsFakeNode(finish) && IsSuperNode(OSMNodes,finish))
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

       results[point]=CombineRoutes(superresults,OSMNodes,OSMSegments,OSMWays,&profile);
      }
   }

 /* Print out the combined route */

 PrintRouteHead(FileName(dirname,prefix,"copyright.txt"));

 for(point=1;point<=NWAYPOINTS;point++)
    if(results[point])
       PrintRoute(results[point],OSMNodes,OSMSegments,OSMWays,&profile);

 PrintRouteTail();

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Create a pair of fake segments corresponding to the given segment split in two.

  index_t CreateFakes Returns the fake node index (or a real one in special cases).

  Nodes *nodes The set of nodes to use.

  int point Which of the waypoints is this.

  Segment *segment The segment to split.

  index_t node1 The first node at the end of this segment.

  index_t node2 The second node at the end of this segment.

  distance_t dist1 The distance to the first node.

  distance_t dist2 The distance to the second node.
  ++++++++++++++++++++++++++++++++++++++*/

index_t CreateFakes(Nodes *nodes,int point,Segment *segment,index_t node1,index_t node2,distance_t dist1,distance_t dist2)
{
 index_t fakenode;
 double lat1,lon1,lat2,lon2;

 /* Check if we are actually close enough to an existing node */

 if(dist1<km_to_distance(MINSEGMENT) && dist2>km_to_distance(MINSEGMENT))
    return(node1);

 if(dist2<km_to_distance(MINSEGMENT) && dist1>km_to_distance(MINSEGMENT))
    return(node2);

 if(dist1<km_to_distance(MINSEGMENT) && dist2<km_to_distance(MINSEGMENT))
   {
    if(dist1<dist2)
       return(node1);
    else
       return(node2);
   }

 /* Create the fake node */

 fakenode=point|NODE_SUPER;

 GetLatLong(nodes,node1,&lat1,&lon1);
 GetLatLong(nodes,node2,&lat2,&lon2);

 if(lat1>3 && lat2<-3)
    lat2+=2*M_PI;
 else if(lat1<-3 && lat2>3)
    lat1+=2*M_PI;

 point_lat[point]=lat1+(lat2-lat1)*(double)dist1/(double)(dist1+dist2);
 point_lon[point]=lon1+(lon2-lon1)*(double)dist1/(double)(dist1+dist2);

 if(point_lat[point]>M_PI) point_lat[point]-=2*M_PI;

 /* Create the first fake segment */

 fake_segments[2*point-2]=*segment;

 if(segment->node1==node1)
    fake_segments[2*point-2].node1=fakenode;
 else
    fake_segments[2*point-2].node2=fakenode;

 fake_segments[2*point-2].distance=DISTANCE(dist1)|DISTFLAG(segment->distance);

 /* Create the second fake segment */

 fake_segments[2*point-1]=*segment;

 if(segment->node1==node2)
    fake_segments[2*point-1].node1=fakenode;
 else
    fake_segments[2*point-1].node2=fakenode;

 fake_segments[2*point-1].distance=DISTANCE(dist2)|DISTFLAG(segment->distance);

 return(fakenode);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup the latitude and longitude of a fake node.

  index_t fakenode The node to lookup.

  double *latitude Returns the latitude

  double *longitude Returns the longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void GetFakeLatLong(index_t fakenode, double *latitude,double *longitude)
{
 index_t realnode=fakenode&(~NODE_SUPER);

 *latitude =point_lat[realnode];
 *longitude=point_lon[realnode];
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the first fake segment associated to a fake node.

  Segment *FirstFakeSegment Returns the first fake segment.

  index_t fakenode The node to lookup.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *FirstFakeSegment(index_t fakenode)
{
 index_t realnode=fakenode&(~NODE_SUPER);

 return(&fake_segments[2*realnode-2]);
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the next (there can only be two) fake segment associated to a fake node.

  Segment *NextFakeSegment Returns the second fake segment.

  Segment *segment The first fake segment.

  index_t fakenode The node to lookup.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *NextFakeSegment(Segment *segment,index_t fakenode)
{
 index_t realnode=fakenode&(~NODE_SUPER);

 if(segment==&fake_segments[2*realnode-2])
    return(&fake_segments[2*realnode-1]);
 else
    return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Finds the next (there can only be two) fake segment associated to a fake node.

  Segment *ExtraFakeSegment Returns a segment between the two specified nodes if it exists.

  index_t node The real node.

  index_t fakenode The fake node to lookup.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *ExtraFakeSegment(index_t node,index_t fakenode)
{
 index_t realnode=fakenode&(~NODE_SUPER);

 if(fake_segments[2*realnode-2].node1==node || fake_segments[2*realnode-2].node2==node)
    return(&fake_segments[2*realnode-2]);

 if(fake_segments[2*realnode-1].node1==node || fake_segments[2*realnode-1].node2==node)
    return(&fake_segments[2*realnode-1]);

 return(NULL);
}
