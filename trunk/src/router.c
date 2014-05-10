/***************************************
 OSM router.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2014 Andrew M. Bishop

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
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "relations.h"

#include "files.h"
#include "logging.h"
#include "functions.h"
#include "fakes.h"
#include "translations.h"
#include "profiles.h"


/*+ To help when debugging +*/
#define DEBUG 0

/*+ The maximum distance from the specified point to search for a node or segment (in km). +*/
#define MAXSEARCH  1


/* Global variables */

/*+ The option not to print any progress information. +*/
int option_quiet=0;

/*+ The options to select the format of the output. +*/
int option_html=0,option_gpx_track=0,option_gpx_route=0,option_text=0,option_text_all=0,option_none=0,option_stdout=0;

/*+ The option to calculate the quickest route insted of the shortest. +*/
int option_quickest=0;


/* Local functions */

static void print_usage(int detail,const char *argerr,const char *err);

static Results *CalculateRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,
                               index_t start_node,index_t prev_segment,index_t finish_node,
                               int start_waypoint,int finish_waypoint);


/*++++++++++++++++++++++++++++++++++++++
  The main program for the router.
  ++++++++++++++++++++++++++++++++++++++*/

int main(int argc,char** argv)
{
 Nodes     *OSMNodes;
 Segments  *OSMSegments;
 Ways      *OSMWays;
 Relations *OSMRelations;
 Results   *results[NWAYPOINTS+1]={NULL};
 int        point_used[NWAYPOINTS+1]={0};
 double     point_lon[NWAYPOINTS+1],point_lat[NWAYPOINTS+1];
 double     heading=-999;
 int        help_profile=0,help_profile_xml=0,help_profile_json=0,help_profile_pl=0;
 char      *dirname=NULL,*prefix=NULL;
 char      *profiles=NULL,*profilename=NULL;
 char      *translations=NULL,*language=NULL;
 int        exactnodes=0,reverse=0,loop=0;
 Transport  transport=Transport_None;
 Profile   *profile=NULL;
 index_t    start_node,finish_node=NO_NODE,first_node=NO_NODE;
 index_t    join_segment=NO_SEGMENT;
 int        arg,nresults=0;
 waypoint_t start_waypoint,finish_waypoint=NO_WAYPOINT;
 waypoint_t first_waypoint=NWAYPOINTS,last_waypoint=1,inc_dec_waypoint,waypoint;

 /* Parse the command line arguments */

 if(argc<2)
    print_usage(0,NULL,NULL);

 /* Get the non-routing, general program options */

 for(arg=1;arg<argc;arg++)
   {
    if(!strcmp(argv[arg],"--help"))
       print_usage(1,NULL,NULL);
    else if(!strcmp(argv[arg],"--help-profile"))
       help_profile=1;
    else if(!strcmp(argv[arg],"--help-profile-xml"))
       help_profile_xml=1;
    else if(!strcmp(argv[arg],"--help-profile-json"))
       help_profile_json=1;
    else if(!strcmp(argv[arg],"--help-profile-perl"))
       help_profile_pl=1;
    else if(!strncmp(argv[arg],"--dir=",6))
       dirname=&argv[arg][6];
    else if(!strncmp(argv[arg],"--prefix=",9))
       prefix=&argv[arg][9];
    else if(!strncmp(argv[arg],"--profiles=",11))
       profiles=&argv[arg][11];
    else if(!strncmp(argv[arg],"--translations=",15))
       translations=&argv[arg][15];
    else if(!strcmp(argv[arg],"--exact-nodes-only"))
       exactnodes=1;
    else if(!strcmp(argv[arg],"--reverse"))
       reverse=1;
    else if(!strcmp(argv[arg],"--loop"))
       loop=1;
    else if(!strcmp(argv[arg],"--quiet"))
       option_quiet=1;
    else if(!strcmp(argv[arg],"--loggable"))
       option_loggable=1;
    else if(!strcmp(argv[arg],"--output-html"))
       option_html=1;
    else if(!strcmp(argv[arg],"--output-gpx-track"))
       option_gpx_track=1;
    else if(!strcmp(argv[arg],"--output-gpx-route"))
       option_gpx_route=1;
    else if(!strcmp(argv[arg],"--output-text"))
       option_text=1;
    else if(!strcmp(argv[arg],"--output-text-all"))
       option_text_all=1;
    else if(!strcmp(argv[arg],"--output-none"))
       option_none=1;
    else if(!strcmp(argv[arg],"--output-stdout"))
      { option_stdout=1; option_quiet=1; }
    else if(!strncmp(argv[arg],"--profile=",10))
       profilename=&argv[arg][10];
    else if(!strncmp(argv[arg],"--language=",11))
       language=&argv[arg][11];
    else if(!strncmp(argv[arg],"--transport=",12))
      {
       transport=TransportType(&argv[arg][12]);

       if(transport==Transport_None)
          print_usage(0,argv[arg],NULL);
      }
    else
       continue;

    argv[arg]=NULL;
   }

 if(option_stdout && (option_html+option_gpx_track+option_gpx_route+option_text+option_text_all)!=1)
   {
    fprintf(stderr,"Error: The '--output-stdout' option requires exactly one other output option (but not '--output-none').\n");
    exit(EXIT_FAILURE);
   }

 /* Load in the profiles */

 if(transport==Transport_None)
    transport=Transport_Motorcar;

 if(profiles)
   {
    if(!ExistsFile(profiles))
      {
       fprintf(stderr,"Error: The '--profiles' option specifies a file that does not exist.\n");
       exit(EXIT_FAILURE);
      }
   }
 else
   {
    if(ExistsFile(FileName(dirname,prefix,"profiles.xml")))
       profiles=FileName(dirname,prefix,"profiles.xml");
    else if(ExistsFile(FileName(DATADIR,NULL,"profiles.xml")))
       profiles=FileName(DATADIR,NULL,"profiles.xml");
    else
      {
       fprintf(stderr,"Error: The '--profiles' option was not used and the default 'profiles.xml' does not exist.\n");
       exit(EXIT_FAILURE);
      }
   }

 if(ParseXMLProfiles(profiles))
   {
    fprintf(stderr,"Error: Cannot read the profiles in the file '%s'.\n",profiles);
    exit(EXIT_FAILURE);
   }

 /* Choose the selected profile. */

 if(profilename)
   {
    profile=GetProfile(profilename);

    if(!profile)
      {
       fprintf(stderr,"Error: Cannot find a profile called '%s' in '%s'.\n",profilename,profiles);
       exit(EXIT_FAILURE);
      }
   }
 else
    profile=GetProfile(TransportName(transport));

 if(!profile)
   {
    profile=(Profile*)calloc(1,sizeof(Profile));
    profile->transport=transport;
   }

 /* Parse the other command line arguments */

 for(arg=1;arg<argc;arg++)
   {
    if(!argv[arg])
       continue;
    else if(!strcmp(argv[arg],"--shortest"))
       option_quickest=0;
    else if(!strcmp(argv[arg],"--quickest"))
       option_quickest=1;
    else if(!strncmp(argv[arg],"--lon",5) && isdigit(argv[arg][5]))
      {
       int point;
       char *p=&argv[arg][6];

       while(isdigit(*p)) p++;
       if(*p++!='=')
          print_usage(0,argv[arg],NULL);
 
       point=atoi(&argv[arg][5]);
       if(point>NWAYPOINTS || point_used[point]&1)
          print_usage(0,argv[arg],NULL);
 
       point_lon[point]=degrees_to_radians(atof(p));
       point_used[point]+=1;

       if(point<first_waypoint)
          first_waypoint=point;
       if(point>last_waypoint)
          last_waypoint=point;
      }
    else if(!strncmp(argv[arg],"--lat",5) && isdigit(argv[arg][5]))
      {
       int point;
       char *p=&argv[arg][6];

       while(isdigit(*p)) p++;
       if(*p++!='=')
          print_usage(0,argv[arg],NULL);
 
       point=atoi(&argv[arg][5]);
       if(point>NWAYPOINTS || point_used[point]&2)
          print_usage(0,argv[arg],NULL);
 
       point_lat[point]=degrees_to_radians(atof(p));
       point_used[point]+=2;

       if(point<first_waypoint)
          first_waypoint=point;
       if(point>last_waypoint)
          last_waypoint=point;
      }
    else if(!strncmp(argv[arg],"--heading=",10))
      {
       double h=atof(&argv[arg][10]);

       if(h>=-360 && h<=360)
         {
          heading=h;

          if(heading<0) heading+=360;
         }
      }
    else if(!strncmp(argv[arg],"--transport=",12))
       ; /* Done this already */
    else if(!strncmp(argv[arg],"--highway-",10))
      {
       Highway highway;
       char *equal=strchr(argv[arg],'=');
       char *string;

       if(!equal)
           print_usage(0,argv[arg],NULL);

       string=strcpy((char*)malloc(strlen(argv[arg])),argv[arg]+10);
       string[equal-argv[arg]-10]=0;

       highway=HighwayType(string);

       if(highway==Highway_None)
          print_usage(0,argv[arg],NULL);

       profile->highway[highway]=atof(equal+1);

       free(string);
      }
    else if(!strncmp(argv[arg],"--speed-",8))
      {
       Highway highway;
       char *equal=strchr(argv[arg],'=');
       char *string;

       if(!equal)
          print_usage(0,argv[arg],NULL);

       string=strcpy((char*)malloc(strlen(argv[arg])),argv[arg]+8);
       string[equal-argv[arg]-8]=0;

       highway=HighwayType(string);

       if(highway==Highway_None)
          print_usage(0,argv[arg],NULL);

       profile->speed[highway]=kph_to_speed(atof(equal+1));

       free(string);
      }
    else if(!strncmp(argv[arg],"--property-",11))
      {
       Property property;
       char *equal=strchr(argv[arg],'=');
       char *string;

       if(!equal)
          print_usage(0,argv[arg],NULL);

       string=strcpy((char*)malloc(strlen(argv[arg])),argv[arg]+11);
       string[equal-argv[arg]-11]=0;

       property=PropertyType(string);

       if(property==Property_None)
          print_usage(0,argv[arg],NULL);

       profile->props_yes[property]=atof(equal+1);

       free(string);
      }
    else if(!strncmp(argv[arg],"--oneway=",9))
       profile->oneway=!!atoi(&argv[arg][9]);
    else if(!strncmp(argv[arg],"--turns=",8))
       profile->turns=!!atoi(&argv[arg][8]);
    else if(!strncmp(argv[arg],"--weight=",9))
       profile->weight=tonnes_to_weight(atof(&argv[arg][9]));
    else if(!strncmp(argv[arg],"--height=",9))
       profile->height=metres_to_height(atof(&argv[arg][9]));
    else if(!strncmp(argv[arg],"--width=",8))
       profile->width=metres_to_width(atof(&argv[arg][8]));
    else if(!strncmp(argv[arg],"--length=",9))
       profile->length=metres_to_length(atof(&argv[arg][9]));
    else
       print_usage(0,argv[arg],NULL);
   }

 /* Print one of the profiles if requested */

 if(help_profile)
   {
    PrintProfile(profile);

    return(0);
   }
 else if(help_profile_xml)
   {
    PrintProfilesXML();

    return(0);
   }
 else if(help_profile_json)
   {
    PrintProfilesJSON();

    return(0);
   }
 else if(help_profile_pl)
   {
    PrintProfilesPerl();

    return(0);
   }

 /* Check the waypoints are valid */

 for(waypoint=1;waypoint<=NWAYPOINTS;waypoint++)
    if(point_used[waypoint]==1 || point_used[waypoint]==2)
       print_usage(0,NULL,"All waypoints must have latitude and longitude.");

 if(first_waypoint>=last_waypoint)
    print_usage(0,NULL,"At least two waypoints must be specified.");

 /* Load in the translations */

 if(option_html==0 && option_gpx_track==0 && option_gpx_route==0 && option_text==0 && option_text_all==0 && option_none==0)
    option_html=option_gpx_track=option_gpx_route=option_text=option_text_all=1;

 if(option_html || option_gpx_route || option_gpx_track)
   {
    if(translations)
      {
       if(!ExistsFile(translations))
         {
          fprintf(stderr,"Error: The '--translations' option specifies a file that does not exist.\n");
          exit(EXIT_FAILURE);
         }
      }
    else
      {
       if(ExistsFile(FileName(dirname,prefix,"translations.xml")))
          translations=FileName(dirname,prefix,"translations.xml");
       else if(ExistsFile(FileName(DATADIR,NULL,"translations.xml")))
          translations=FileName(DATADIR,NULL,"translations.xml");
       else
         {
          fprintf(stderr,"Error: The '--translations' option was not used and the default 'translations.xml' does not exist.\n");
          exit(EXIT_FAILURE);
         }
      }

    if(ParseXMLTranslations(translations,language))
      {
       fprintf(stderr,"Error: Cannot read the translations in the file '%s'.\n",translations);
       exit(EXIT_FAILURE);
      }
   }

 /* Load in the data - Note: No error checking because Load*List() will call exit() in case of an error. */

 OSMNodes=LoadNodeList(FileName(dirname,prefix,"nodes.mem"));

 OSMSegments=LoadSegmentList(FileName(dirname,prefix,"segments.mem"));

 OSMWays=LoadWayList(FileName(dirname,prefix,"ways.mem"));

 OSMRelations=LoadRelationList(FileName(dirname,prefix,"relations.mem"));

 if(UpdateProfile(profile,OSMWays))
   {
    fprintf(stderr,"Error: Profile is invalid or not compatible with database.\n");
    exit(EXIT_FAILURE);
   }

 /* Check for reverse direction */

 if(reverse)
   {
    waypoint_t temp;

    temp=first_waypoint;
    first_waypoint=last_waypoint;
    last_waypoint=temp;

    last_waypoint--;

    inc_dec_waypoint=-1;
   }
 else
   {
    last_waypoint++;

    inc_dec_waypoint=1;
   }

 /* Loop through all pairs of waypoints */

 for(waypoint=first_waypoint;waypoint!=last_waypoint;waypoint+=inc_dec_waypoint)
   {
    distance_t distmax=km_to_distance(MAXSEARCH);
    distance_t distmin;
    index_t segment=NO_SEGMENT;
    index_t node1,node2;

    if(point_used[waypoint]!=3)
       continue;

    /* Find the closest point */

    start_node=finish_node;
    start_waypoint=finish_waypoint;

    if(exactnodes)
      {
       finish_node=FindClosestNode(OSMNodes,OSMSegments,OSMWays,point_lat[waypoint],point_lon[waypoint],distmax,profile,&distmin);
      }
    else
      {
       distance_t dist1,dist2;

       segment=FindClosestSegment(OSMNodes,OSMSegments,OSMWays,point_lat[waypoint],point_lon[waypoint],distmax,profile,&distmin,&node1,&node2,&dist1,&dist2);

       if(segment!=NO_SEGMENT)
          finish_node=CreateFakes(OSMNodes,OSMSegments,waypoint,LookupSegment(OSMSegments,segment,1),node1,node2,dist1,dist2);
       else
          finish_node=NO_NODE;
      }

    if(finish_node==NO_NODE)
      {
       fprintf(stderr,"Error: Cannot find node close to specified point %d.\n",waypoint);
       exit(EXIT_FAILURE);
      }

    finish_waypoint=waypoint;

    if(!option_quiet)
      {
       double lat,lon;

       if(IsFakeNode(finish_node))
          GetFakeLatLong(finish_node,&lat,&lon);
       else
          GetLatLong(OSMNodes,finish_node,NULL,&lat,&lon);

       if(IsFakeNode(finish_node))
          printf("Waypoint %d is segment %"Pindex_t" (node %"Pindex_t" -> %"Pindex_t"): %3.6f %4.6f = %2.3f km\n",waypoint,segment,node1,node2,
                 radians_to_degrees(lon),radians_to_degrees(lat),distance_to_km(distmin));
       else
          printf("Waypoint %d is node %"Pindex_t": %3.6f %4.6f = %2.3f km\n",waypoint,finish_node,
                 radians_to_degrees(lon),radians_to_degrees(lat),distance_to_km(distmin));
      }

    /* Check the nodes */

    if(start_node==NO_NODE)
       continue;

    if(first_node==NO_NODE)
       first_node=start_node;

    if(heading!=-999 && join_segment==NO_SEGMENT)
       join_segment=FindClosestSegmentHeading(OSMNodes,OSMSegments,OSMWays,start_node,heading,profile);

    /* Calculate the route */

    results[nresults]=CalculateRoute(OSMNodes,OSMSegments,OSMWays,OSMRelations,profile,start_node,join_segment,finish_node,start_waypoint,finish_waypoint);

    join_segment=results[nresults]->last_segment;

    nresults++;
   }

 /* Finish the loop */

 if(loop && finish_node!=NO_NODE)
   {
    results[nresults]=CalculateRoute(OSMNodes,OSMSegments,OSMWays,OSMRelations,profile,finish_node,join_segment,first_node,last_waypoint,first_waypoint);

    nresults++;
   }

 if(!option_quiet)
   {
    printf("Routed OK\n");
    fflush(stdout);
   }

 /* Print out the combined route */

 if(!option_none)
    PrintRoute(results,nresults,OSMNodes,OSMSegments,OSMWays,profile);

 /* Destroy the remaining results lists and data structures */

#if 0

 for(waypoint=0;waypoint<=nresults;waypoint++)
    FreeResultsList(results[waypoint]);

 DestroyNodeList(OSMNodes);
 DestroySegmentList(OSMSegments);
 DestroyWayList(OSMWays);
 DestroyRelationList(OSMRelations);

#endif

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a complete route from a specified node to another node.

  Results *CalculateRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  index_t start_node The start node.

  index_t prev_segment The previous segment before the start node.

  index_t finish_node The finish node.

  int start_waypoint The starting waypoint.

  int finish_waypoint The finish waypoint.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *CalculateRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,
                               index_t start_node,index_t prev_segment,index_t finish_node,
                               int start_waypoint,int finish_waypoint)
{
 Results *complete=NULL;

 /* A special case if the first and last nodes are the same */

 if(start_node==finish_node)
   {
    index_t fake_segment;
    Result *result1,*result2;

    complete=NewResultsList(8);

    if(prev_segment==NO_SEGMENT)
      {
       double lat,lon;
       distance_t distmin,dist1,dist2;
       index_t node1,node2;

       GetLatLong(nodes,start_node,NULL,&lat,&lon);

       prev_segment=FindClosestSegment(nodes,segments,ways,lat,lon,1,profile,&distmin,&node1,&node2,&dist1,&dist2);
      }

    fake_segment=CreateFakeNullSegment(segments,start_node,prev_segment,finish_waypoint);

    result1=InsertResult(complete,start_node,prev_segment);
    result2=InsertResult(complete,finish_node,fake_segment);

    result1->next=result2;

    complete->start_node=start_node;
    complete->prev_segment=prev_segment;

    complete->finish_node=finish_node;
    complete->last_segment=prev_segment;

    complete->last_segment=result2->segment;
   }
 else
   {
    Results *begin,*end;

    /* Calculate the beginning of the route */

    begin=FindStartRoutes(nodes,segments,ways,relations,profile,start_node,prev_segment,finish_node);

    if(begin)
      {
       /* Check if the end of the route was reached */

       if(begin->finish_node!=NO_NODE)
          complete=ExtendStartRoutes(nodes,segments,ways,relations,profile,begin,finish_node);
      }
    else
      {
       if(prev_segment!=NO_SEGMENT)
         {
          /* Try again but allow a U-turn at the start waypoint -
             this solves the problem of facing a dead-end that contains no super-nodes. */

          prev_segment=NO_SEGMENT;

          begin=FindStartRoutes(nodes,segments,ways,relations,profile,start_node,prev_segment,finish_node);
         }

       if(begin)
         {
          /* Check if the end of the route was reached */

          if(begin->finish_node!=NO_NODE)
             complete=ExtendStartRoutes(nodes,segments,ways,relations,profile,begin,finish_node);
         }
       else
         {
          fprintf(stderr,"Error: Cannot find initial section of route compatible with profile.\n");
          exit(EXIT_FAILURE);
         }
      }

    /* Calculate the rest of the route */

    if(!complete)
      {
       Results *middle;

       /* Calculate the end of the route */

       end=FindFinishRoutes(nodes,segments,ways,relations,profile,finish_node);

       if(!end)
         {
          fprintf(stderr,"Error: Cannot find final section of route compatible with profile.\n");
          exit(EXIT_FAILURE);
         }

       /* Calculate the middle of the route */

       middle=FindMiddleRoute(nodes,segments,ways,relations,profile,begin,end);

       if(!middle && prev_segment!=NO_SEGMENT)
         {
          /* Try again but allow a U-turn at the start waypoint -
             this solves the problem of facing a dead-end that contains some super-nodes. */

          FreeResultsList(begin);

          begin=FindStartRoutes(nodes,segments,ways,relations,profile,start_node,NO_SEGMENT,finish_node);

          if(begin)
             middle=FindMiddleRoute(nodes,segments,ways,relations,profile,begin,end);
         }

       FreeResultsList(end);

       if(!middle)
         {
          fprintf(stderr,"Error: Cannot find super-route compatible with profile.\n");
          exit(EXIT_FAILURE);
         }

       complete=CombineRoutes(nodes,segments,ways,relations,profile,begin,middle);

       if(!complete)
         {
          fprintf(stderr,"Error: Cannot create combined route following super-route.\n");
          exit(EXIT_FAILURE);
         }

       FreeResultsList(begin);

       FreeResultsList(middle);
      }
   }

 complete->start_waypoint=start_waypoint;
 complete->finish_waypoint=finish_waypoint;

#if DEBUG
 Result *r=FindResult(complete,complete->start_node,complete->prev_segment);

 printf("The final route is:\n");

 while(r)
   {
    printf("  node=%"Pindex_t" segment=%"Pindex_t" score=%f\n",r->node,r->segment,r->score);

    r=r->next;
   }
#endif

 return(complete);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the usage information.

  int detail The level of detail to use - 0 = low, 1 = high.

  const char *argerr The argument that gave the error (if there is one).

  const char *err Other error message (if there is one).
  ++++++++++++++++++++++++++++++++++++++*/

static void print_usage(int detail,const char *argerr,const char *err)
{
 fprintf(stderr,
         "Usage: router [--help | --help-profile | --help-profile-xml |\n"
         "                        --help-profile-json | --help-profile-perl ]\n"
         "              [--dir=<dirname>] [--prefix=<name>]\n"
         "              [--profiles=<filename>] [--translations=<filename>]\n"
         "              [--exact-nodes-only]\n"
         "              [--loggable | --quiet]\n"
         "              [--language=<lang>]\n"
         "              [--output-html]\n"
         "              [--output-gpx-track] [--output-gpx-route]\n"
         "              [--output-text] [--output-text-all]\n"
         "              [--output-none] [--output-stdout]\n"
         "              [--profile=<name>]\n"
         "              [--transport=<transport>]\n"
         "              [--shortest | --quickest]\n"
         "              --lon1=<longitude> --lat1=<latitude>\n"
         "              --lon2=<longitude> --lon2=<latitude>\n"
         "              [ ... --lon99=<longitude> --lon99=<latitude>]\n"
         "              [--reverse] [--loop]\n"
         "              [--highway-<highway>=<preference> ...]\n"
         "              [--speed-<highway>=<speed> ...]\n"
         "              [--property-<property>=<preference> ...]\n"
         "              [--oneway=(0|1)] [--turns=(0|1)]\n"
         "              [--weight=<weight>]\n"
         "              [--height=<height>] [--width=<width>] [--length=<length>]\n");

 if(argerr)
    fprintf(stderr,
            "\n"
            "Error with command line parameter: %s\n",argerr);

 if(err)
    fprintf(stderr,
            "\n"
            "Error: %s\n",err);

 if(detail)
    fprintf(stderr,
            "\n"
            "--help                  Prints this information.\n"
            "--help-profile          Prints the information about the selected profile.\n"
            "--help-profile-xml      Prints all loaded profiles in XML format.\n"
            "--help-profile-json     Prints all loaded profiles in JSON format.\n"
            "--help-profile-perl     Prints all loaded profiles in Perl format.\n"
            "\n"
            "--dir=<dirname>         The directory containing the routing database.\n"
            "--prefix=<name>         The filename prefix for the routing database.\n"
            "--profiles=<filename>   The name of the XML file containing the profiles\n"
            "                        (defaults to 'profiles.xml' with '--dir' and\n"
            "                         '--prefix' options or the file installed in\n"
            "                         '" DATADIR "').\n"
            "--translations=<fname>  The name of the XML file containing the translations\n"
            "                        (defaults to 'translations.xml' with '--dir' and\n"
            "                         '--prefix' options or the file installed in\n"
            "                         '" DATADIR "').\n"
            "\n"
            "--exact-nodes-only      Only route between nodes (don't find closest segment).\n"
            "\n"
            "--loggable              Print progress messages suitable for logging to file.\n"
            "--quiet                 Don't print any screen output when running.\n"
            "\n"
            "--language=<lang>       Use the translations for specified language.\n"
            "--output-html           Write an HTML description of the route.\n"
            "--output-gpx-track      Write a GPX track file with all route points.\n"
            "--output-gpx-route      Write a GPX route file with interesting junctions.\n"
            "--output-text           Write a plain text file with interesting junctions.\n"
            "--output-text-all       Write a plain test file with all route points.\n"
            "--output-none           Don't write any output files or read any translations.\n"
            "                        (If no output option is given then all are written.)\n"
            "--output-stdout         Write to stdout instead of a file (requires exactly\n"
            "                        one output format option, implies '--quiet').\n"
            "\n"
            "--profile=<name>        Select the loaded profile with this name.\n"
            "--transport=<transport> Select the transport to use (selects the profile\n"
            "                        named after the transport if '--profile' is not used.)\n"
            "\n"
            "--shortest              Find the shortest route between the waypoints.\n"
            "--quickest              Find the quickest route between the waypoints.\n"
            "\n"
            "--lon<n>=<longitude>    Specify the longitude of the n'th waypoint.\n"
            "--lat<n>=<latitude>     Specify the latitude of the n'th waypoint.\n"
            "\n"
            "--reverse               Find a route between the waypoints in reverse order.\n"
            "--loop                  Find a route that returns to the first waypoint.\n"
            "\n"
            "--heading=<bearing>     Initial compass bearing at lowest numbered waypoint.\n"
            "\n"
            "                                   Routing preference options\n"
            "--highway-<highway>=<preference>   * preference for highway type (%%).\n"
            "--speed-<highway>=<speed>          * speed for highway type (km/h).\n"
            "--property-<property>=<preference> * preference for proprty type (%%).\n"
            "--oneway=(0|1)                     * oneway restrictions are to be obeyed.\n"
            "--turns=(0|1)                      * turn restrictions are to be obeyed.\n"
            "--weight=<weight>                  * maximum weight limit (tonnes).\n"
            "--height=<height>                  * maximum height limit (metres).\n"
            "--width=<width>                    * maximum width limit (metres).\n"
            "--length=<length>                  * maximum length limit (metres).\n"
            "\n"
            "<transport> defaults to motorcar but can be set to:\n"
            "%s"
            "\n"
            "<highway> can be selected from:\n"
            "%s"
            "\n"
            "<property> can be selected from:\n"
            "%s",
            TransportList(),HighwayList(),PropertyList());

 exit(!detail);
}
