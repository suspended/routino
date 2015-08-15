/***************************************
 Routing output generator.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2015 Andrew M. Bishop

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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"

#include "functions.h"
#include "fakes.h"
#include "translations.h"
#include "results.h"
#include "xmlparse.h"

#include "routino.h"


/*+ To help when debugging +*/
#define DEBUG 0

/* Constants */

#define ROUTINO_POINT_IGNORE      -1      /*+ Ignore this point. +*/


/* Global variables */

/*+ The option to calculate the quickest route insted of the shortest. +*/
int option_quickest=0;

/*+ The options to select the format of the file output. +*/
int option_file_html=0,option_file_gpx_track=0,option_file_gpx_route=0,option_file_text=0,option_file_text_all=0,option_file_stdout=0;

/*+ The options to select the format of the linked list output. +*/
int option_list_html=0,option_list_text=0,option_list_text_all=0;


/* Local variables */

/*+ Heuristics for determining if a junction is important. +*/
static const char junction_other_way[Highway_Count][Highway_Count]=
 { /* M, T, P, S, T, U, R, S, T, C, P, S, F = Way type of route not taken */
  {   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Motorway     */
  {   1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Trunk        */
  {   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Primary      */
  {   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Secondary    */
  {   1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 }, /* Tertiary     */
  {   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 }, /* Unclassified */
  {   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1 }, /* Residential  */
  {   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1 }, /* Service      */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1 }, /* Track        */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1 }, /* Cycleway     */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Path         */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Steps        */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Ferry        */
 };


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  Routino_Output *PrintRoute Returns a linked list of data structures representing the route if required.

  Results **results The set of results to print (consecutive in array even if not consecutive waypoints).

  int nresults The number of results in the list.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  Translation *translation The set of translated strings.
  ++++++++++++++++++++++++++++++++++++++*/

Routino_Output *PrintRoute(Results **results,int nresults,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile,Translation *translation)
{
 FILE                          *htmlfile=NULL,*gpxtrackfile=NULL,*gpxroutefile=NULL,*textfile=NULL,*textallfile=NULL;
 Routino_Output *listhead=NULL,*htmllist=NULL,                                      *textlist=NULL,*textalllist=NULL;

 char *prev_bearing=NULL,*prev_wayname=NULL,*prev_waynameraw=NULL;
 index_t prev_node=NO_NODE;
 distance_t cum_distance=0;
 duration_t cum_duration=0;

 int point=0;
 int segment_count=0,route_count=0;
 int point_count=0;
 int roundabout=0;

 /* Open the files */

 if(option_file_stdout)
   {
    if(option_file_html)
       htmlfile    =stdout;
    if(option_file_gpx_track)
       gpxtrackfile=stdout;
    if(option_file_gpx_route)
       gpxroutefile=stdout;
    if(option_file_text)
       textfile    =stdout;
    if(option_file_text_all)
       textallfile =stdout;
   }
 else
   {
#if defined(_MSC_VER) || defined(__MINGW32__)
    const char *open_mode="wb";
#else
    const char *open_mode="w";
#endif

    if(option_quickest==0)
      {
       /* Print the result for the shortest route */

       if(option_file_html)
          htmlfile    =fopen("shortest.html",open_mode);
       if(option_file_gpx_track)
          gpxtrackfile=fopen("shortest-track.gpx",open_mode);
       if(option_file_gpx_route)
          gpxroutefile=fopen("shortest-route.gpx",open_mode);
       if(option_file_text)
          textfile    =fopen("shortest.txt",open_mode);
       if(option_file_text_all)
          textallfile =fopen("shortest-all.txt",open_mode);

#ifndef LIBROUTINO
       if(option_file_html && !htmlfile)
          fprintf(stderr,"Warning: Cannot open file 'shortest.html' for writing [%s].\n",strerror(errno));
       if(option_file_gpx_track && !gpxtrackfile)
          fprintf(stderr,"Warning: Cannot open file 'shortest-track.gpx' for writing [%s].\n",strerror(errno));
       if(option_file_gpx_route && !gpxroutefile)
          fprintf(stderr,"Warning: Cannot open file 'shortest-route.gpx' for writing [%s].\n",strerror(errno));
       if(option_file_text && !textfile)
          fprintf(stderr,"Warning: Cannot open file 'shortest.txt' for writing [%s].\n",strerror(errno));
       if(option_file_text_all && !textallfile)
          fprintf(stderr,"Warning: Cannot open file 'shortest-all.txt' for writing [%s].\n",strerror(errno));
#endif
      }
    else
      {
       /* Print the result for the quickest route */

       if(option_file_html)
          htmlfile    =fopen("quickest.html",open_mode);
       if(option_file_gpx_track)
          gpxtrackfile=fopen("quickest-track.gpx",open_mode);
       if(option_file_gpx_route)
          gpxroutefile=fopen("quickest-route.gpx",open_mode);
       if(option_file_text)
          textfile    =fopen("quickest.txt",open_mode);
       if(option_file_text_all)
          textallfile =fopen("quickest-all.txt",open_mode);

#ifndef LIBROUTINO
       if(option_file_html && !htmlfile)
          fprintf(stderr,"Warning: Cannot open file 'quickest.html' for writing [%s].\n",strerror(errno));
       if(option_file_gpx_track && !gpxtrackfile)
          fprintf(stderr,"Warning: Cannot open file 'quickest-track.gpx' for writing [%s].\n",strerror(errno));
       if(option_file_gpx_route && !gpxroutefile)
          fprintf(stderr,"Warning: Cannot open file 'quickest-route.gpx' for writing [%s].\n",strerror(errno));
       if(option_file_text && !textfile)
          fprintf(stderr,"Warning: Cannot open file 'quickest.txt' for writing [%s].\n",strerror(errno));
       if(option_file_text_all && !textallfile)
          fprintf(stderr,"Warning: Cannot open file 'quickest-all.txt' for writing [%s].\n",strerror(errno));
#endif
      }
   }

 /* Print the head of the files */

 if(htmlfile)
   {
    fprintf(htmlfile,"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n");
    fprintf(htmlfile,"<html>\n");
    if(translation->xml_copyright_creator[0] && translation->xml_copyright_creator[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translation->xml_copyright_creator[0],translation->xml_copyright_creator[1]);
    if(translation->xml_copyright_source[0] && translation->xml_copyright_source[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translation->xml_copyright_source[0],translation->xml_copyright_source[1]);
    if(translation->xml_copyright_license[0] && translation->xml_copyright_license[1])
       fprintf(htmlfile,"<!-- %s : %s -->\n",translation->xml_copyright_license[0],translation->xml_copyright_license[1]);
    fprintf(htmlfile,"<head>\n");
    fprintf(htmlfile,"<title>");
    fprintf(htmlfile,translation->html_title,option_quickest?translation->xml_route_quickest:translation->xml_route_shortest);
    fprintf(htmlfile,"</title>\n");
    fprintf(htmlfile,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
    fprintf(htmlfile,"<style type=\"text/css\">\n");
    fprintf(htmlfile,"<!--\n");
    fprintf(htmlfile,"   table   {table-layout: fixed; border: none; border-collapse: collapse;}\n");
    fprintf(htmlfile,"   table.c {color: grey; font-size: x-small;} /* copyright */\n");
    fprintf(htmlfile,"   tr      {border: 0px;}\n");
    fprintf(htmlfile,"   tr.c    {display: none;} /* coords */\n");
    fprintf(htmlfile,"   tr.n    {} /* node */\n");
    fprintf(htmlfile,"   tr.s    {} /* segment */\n");
    fprintf(htmlfile,"   tr.t    {font-weight: bold;} /* total */\n");
    fprintf(htmlfile,"   td.l    {font-weight: bold;}\n");
    fprintf(htmlfile,"   td.r    {}\n");
    fprintf(htmlfile,"   span.w  {font-weight: bold;} /* waypoint */\n");
    fprintf(htmlfile,"   span.h  {text-decoration: underline;} /* highway */\n");
    fprintf(htmlfile,"   span.d  {} /* segment distance */\n");
    fprintf(htmlfile,"   span.j  {font-style: italic;} /* total journey distance */\n");
    fprintf(htmlfile,"   span.t  {font-variant: small-caps;} /* turn */\n");
    fprintf(htmlfile,"   span.b  {font-variant: small-caps;} /* bearing */\n");
    fprintf(htmlfile,"-->\n");
    fprintf(htmlfile,"</style>\n");
    fprintf(htmlfile,"</head>\n");
    fprintf(htmlfile,"<body>\n");
    fprintf(htmlfile,"<h1>");
    fprintf(htmlfile,translation->html_title,option_quickest?translation->xml_route_quickest:translation->xml_route_shortest);
    fprintf(htmlfile,"</h1>\n");
    fprintf(htmlfile,"<table>\n");
   }

 if(gpxtrackfile)
   {
    fprintf(gpxtrackfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(gpxtrackfile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

    fprintf(gpxtrackfile,"<metadata>\n");
    fprintf(gpxtrackfile,"<desc>%s : %s</desc>\n",translation->xml_copyright_creator[0],translation->xml_copyright_creator[1]);
    if(translation->xml_copyright_source[1])
      {
       fprintf(gpxtrackfile,"<copyright author=\"%s\">\n",translation->xml_copyright_source[1]);

       if(translation->xml_copyright_license[1])
          fprintf(gpxtrackfile,"<license>%s</license>\n",translation->xml_copyright_license[1]);

       fprintf(gpxtrackfile,"</copyright>\n");
      }
    fprintf(gpxtrackfile,"</metadata>\n");

    fprintf(gpxtrackfile,"<trk>\n");
    fprintf(gpxtrackfile,"<name>");
    fprintf(gpxtrackfile,translation->gpx_name,option_quickest?translation->xml_route_quickest:translation->xml_route_shortest);
    fprintf(gpxtrackfile,"</name>\n");
    fprintf(gpxtrackfile,"<desc>");
    fprintf(gpxtrackfile,translation->gpx_desc,option_quickest?translation->xml_route_quickest:translation->xml_route_shortest);
    fprintf(gpxtrackfile,"</desc>\n");
   }

 if(gpxroutefile)
   {
    fprintf(gpxroutefile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(gpxroutefile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

    fprintf(gpxroutefile,"<metadata>\n");
    fprintf(gpxroutefile,"<desc>%s : %s</desc>\n",translation->xml_copyright_creator[0],translation->xml_copyright_creator[1]);
    if(translation->xml_copyright_source[1])
      {
       fprintf(gpxroutefile,"<copyright author=\"%s\">\n",translation->xml_copyright_source[1]);

       if(translation->xml_copyright_license[1])
          fprintf(gpxroutefile,"<license>%s</license>\n",translation->xml_copyright_license[1]);

       fprintf(gpxroutefile,"</copyright>\n");
      }
    fprintf(gpxroutefile,"</metadata>\n");

    fprintf(gpxroutefile,"<rte>\n");
    fprintf(gpxroutefile,"<name>");
    fprintf(gpxroutefile,translation->gpx_name,option_quickest?translation->xml_route_quickest:translation->xml_route_shortest);
    fprintf(gpxroutefile,"</name>\n");
    fprintf(gpxroutefile,"<desc>");
    fprintf(gpxroutefile,translation->gpx_desc,option_quickest?translation->xml_route_quickest:translation->xml_route_shortest);
    fprintf(gpxroutefile,"</desc>\n");
   }

 if(textfile)
   {
    if(translation->raw_copyright_creator[0] && translation->raw_copyright_creator[1])
       fprintf(textfile,"# %s : %s\n",translation->raw_copyright_creator[0],translation->raw_copyright_creator[1]);
    if(translation->raw_copyright_source[0] && translation->raw_copyright_source[1])
       fprintf(textfile,"# %s : %s\n",translation->raw_copyright_source[0],translation->raw_copyright_source[1]);
    if(translation->raw_copyright_license[0] && translation->raw_copyright_license[1])
       fprintf(textfile,"# %s : %s\n",translation->raw_copyright_license[0],translation->raw_copyright_license[1]);
    if((translation->raw_copyright_creator[0] && translation->raw_copyright_creator[1]) ||
       (translation->raw_copyright_source[0]  && translation->raw_copyright_source[1]) ||
       (translation->raw_copyright_license[0] && translation->raw_copyright_license[1]))
       fprintf(textfile,"#\n");

    fprintf(textfile,"#Latitude\tLongitude\tSection \tSection \tTotal   \tTotal   \tPoint\tTurn\tBearing\tHighway\n");
    fprintf(textfile,"#        \t         \tDistance\tDuration\tDistance\tDuration\tType \t    \t       \t       \n");
                     /* "%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t %+d\t %+d\t%s\n" */
   }

 if(textallfile)
   {
    if(translation->raw_copyright_creator[0] && translation->raw_copyright_creator[1])
       fprintf(textallfile,"# %s : %s\n",translation->raw_copyright_creator[0],translation->raw_copyright_creator[1]);
    if(translation->raw_copyright_source[0] && translation->raw_copyright_source[1])
       fprintf(textallfile,"# %s : %s\n",translation->raw_copyright_source[0],translation->raw_copyright_source[1]);
    if(translation->raw_copyright_license[0] && translation->raw_copyright_license[1])
       fprintf(textallfile,"# %s : %s\n",translation->raw_copyright_license[0],translation->raw_copyright_license[1]);
    if((translation->raw_copyright_creator[0] && translation->raw_copyright_creator[1]) ||
       (translation->raw_copyright_source[0]  && translation->raw_copyright_source[1]) ||
       (translation->raw_copyright_license[0] && translation->raw_copyright_license[1]))
       fprintf(textallfile,"#\n");

    fprintf(textallfile,"#Latitude\tLongitude\t    Node\tType\tSegment\tSegment\tTotal\tTotal  \tSpeed\tBearing\tHighway\n");
    fprintf(textallfile,"#        \t         \t        \t    \tDist   \tDurat'n\tDist \tDurat'n\t     \t       \t       \n");
                        /* "%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%4d\t%s\n" */
   }

 /* Create the head of the linked list */

 if(option_list_html)
    listhead=htmllist=calloc(sizeof(Routino_Output),1);
 if(option_list_text)
    listhead=textlist=calloc(sizeof(Routino_Output),1);
 if(option_list_text_all)
    listhead=textalllist=calloc(sizeof(Routino_Output),1);

 /* Loop through all the sections of the route and print them */

 do
   {
    int first=1;
    int next_point=point;
    distance_t junc_distance=0;
    duration_t junc_duration=0;
    Result *result;

#if DEBUG
    printf("Route section %d - waypoint %d to waypoint %d\n",point,results[point]->start_waypoint,results[point]->finish_waypoint);
    printf("  start_node=%"Pindex_t" prev_segment=%"Pindex_t"\n",results[point]->start_node,results[point]->prev_segment);
    printf("  finish_node=%"Pindex_t" last_segment=%"Pindex_t"\n",results[point]->finish_node,results[point]->last_segment);

    Result *r=FindResult(results[point],results[point]->start_node,results[point]->prev_segment);

    while(r)
      {
       printf("    node=%"Pindex_t" segment=%"Pindex_t" score=%f\n",r->node,r->segment,r->score);

       r=r->next;
      }
#endif

    result=FindResult(results[point],results[point]->start_node,results[point]->prev_segment);

    /* Print the start of the segment */

    if(gpxtrackfile)
       fprintf(gpxtrackfile,"<trkseg>\n");

    /* Loop through all the points within a section of the route and print them */

    do
      {
       double latitude,longitude;
       Node *resultnodep=NULL;
       index_t realsegment=NO_SEGMENT,next_realsegment=NO_SEGMENT;
       Segment *resultsegmentp=NULL,*next_resultsegmentp=NULL;
       Way *resultwayp=NULL,*next_resultwayp=NULL;
       Result *next_result;
       int important=ROUTINO_POINT_UNIMPORTANT;

       distance_t seg_distance=0;
       duration_t seg_duration=0;
       speed_t seg_speed=0;
       char *waynameraw=NULL,*wayname=NULL,*next_waynameraw=NULL,*next_wayname=NULL;
       int bearing_int=0,turn_int=0,next_bearing_int=0;
       char *turn=NULL,*turnraw=NULL,*next_bearing=NULL,*next_bearingraw=NULL;

       /* Calculate the information about this point */

       if(IsFakeNode(result->node))
          GetFakeLatLong(result->node,&latitude,&longitude);
       else
         {
          resultnodep=LookupNode(nodes,result->node,6);

          GetLatLong(nodes,result->node,resultnodep,&latitude,&longitude);
         }

       /* Calculate the next result */

       next_result=result->next;

       if(!next_result)
         {
          next_point++;

          if(next_point<nresults)
            {
             next_result=FindResult(results[next_point],results[next_point]->start_node,results[next_point]->prev_segment);
             next_result=next_result->next;
            }
         }

       /* Calculate the information about this segment */

       if(!first)               /* not first point of a section of the route */
         {
          if(IsFakeSegment(result->segment))
            {
             resultsegmentp=LookupFakeSegment(result->segment);
             realsegment=IndexRealSegment(result->segment);
            }
          else
            {
             resultsegmentp=LookupSegment(segments,result->segment,2);
             realsegment=result->segment;
            }

          resultwayp=LookupWay(ways,resultsegmentp->way,1);

          seg_distance+=DISTANCE(resultsegmentp->distance);
          seg_duration+=Duration(resultsegmentp,resultwayp,profile);

          /* Calculate the cumulative distance/duration */

          junc_distance+=seg_distance;
          junc_duration+=seg_duration;
          cum_distance+=seg_distance;
          cum_duration+=seg_duration;
         }

       /* Calculate the information about the next segment */

       if(next_result)
         {
          if(IsFakeSegment(next_result->segment))
            {
             next_resultsegmentp=LookupFakeSegment(next_result->segment);
             next_realsegment=IndexRealSegment(next_result->segment);
            }
          else
            {
             next_resultsegmentp=LookupSegment(segments,next_result->segment,1);
             next_realsegment=next_result->segment;
            }
         }

       /* Decide if this is a roundabout */

       if(next_result)
         {
          next_resultwayp=LookupWay(ways,next_resultsegmentp->way,2);

          if(next_resultwayp->type&Highway_Roundabout)
            {
             if(roundabout==0)
               {
                roundabout++;
                important=ROUTINO_POINT_RB_ENTRY;
               }
             else
               {
                Segment *segmentp;

                if(resultnodep)
                   segmentp=FirstSegment(segments,resultnodep,3);
                else
                   segmentp=FirstFakeSegment(result->node);

                do
                  {
                   index_t othernode=OtherNode(segmentp,result->node);
                   index_t thissegment;

                   if(IsFakeNode(result->node))
                      thissegment=IndexFakeSegment(segmentp);
                   else
                      thissegment=IndexSegment(segments,segmentp);

                   if(othernode!=prev_node && othernode!=next_result->node &&
                      thissegment!=realsegment && IsNormalSegment(segmentp))
                     {
                      int canexit=1;

                      if(profile->oneway && IsOnewayTo(segmentp,result->node))
                        {
                         if(profile->allow!=Transports_Bicycle)
                            canexit=0;
                         else
                           {
                            Way *wayp=LookupWay(ways,segmentp->way,3);

                            if(!(wayp->type&Highway_CycleBothWays))
                               canexit=0;
                           }
                        }

                      if(canexit)
                        {
                         Way *wayp=LookupWay(ways,segmentp->way,3);

                         if(!(wayp->type&Highway_Roundabout))
                           {
                            roundabout++;
                            important=ROUTINO_POINT_RB_NOT_EXIT;
                           }
                        }
                     }

                   if(resultnodep)
                      segmentp=NextSegment(segments,segmentp,result->node);
                   else
                      segmentp=NextFakeSegment(segmentp,result->node);
                  }
                while(segmentp);
               }
            }
          else
             if(roundabout)
               {
                roundabout++;
                important=ROUTINO_POINT_RB_EXIT;
               }
         }

       /* Decide if this is an important junction */

       if(point_count==0)  /* first point overall = Waypoint */
          important=ROUTINO_POINT_WAYPOINT;
       else if(result->next==NULL) /* Waypoint */
          important=ROUTINO_POINT_WAYPOINT;
       else if(first)           /* first point of a section of the route */
          important=ROUTINO_POINT_IGNORE;
       else if(roundabout)      /* roundabout */
          ;
       else if(realsegment==next_realsegment) /* U-turn */
          important=ROUTINO_POINT_UTURN;
       else if(resultnodep && (resultnodep->flags&NODE_MINIRNDBT))
          important=ROUTINO_POINT_MINI_RB; /* mini-roundabout */
       else
         {
          Segment *segmentp=FirstSegment(segments,resultnodep,3);

          do
            {
             index_t seg=IndexSegment(segments,segmentp);

             if(seg!=realsegment && IsNormalSegment(segmentp))
               {
                int cango=1;

                if(profile->oneway && IsOnewayTo(segmentp,result->node))
                  {
                   if(profile->allow!=Transports_Bicycle)
                      cango=0;
                   else
                     {
                      Way *wayp=LookupWay(ways,segmentp->way,3);

                      if(!(wayp->type&Highway_CycleBothWays))
                         cango=0;
                     }
                  }

                if(cango)
                  {
                   Way *wayp=LookupWay(ways,segmentp->way,3);

                   if(seg==next_realsegment) /* the next segment that we follow */
                     {
                      if(HIGHWAY(wayp->type)!=HIGHWAY(resultwayp->type))
                         if(important<ROUTINO_POINT_CHANGE)
                            important=ROUTINO_POINT_CHANGE;
                     }
                   else /* a segment that we don't follow */
                     {
                      if(junction_other_way[HIGHWAY(resultwayp->type)-1][HIGHWAY(wayp->type)-1])
                         if(important<ROUTINO_POINT_JUNCT_IMPORT)
                            important=ROUTINO_POINT_JUNCT_IMPORT;

                      if(important<ROUTINO_POINT_JUNCT_CONT)
                         important=ROUTINO_POINT_JUNCT_CONT;
                     }
                  }
               }

             segmentp=NextSegment(segments,segmentp,result->node);
            }
          while(segmentp);
         }

       /* Calculate the strings to be used */

       if(!first && (textallfile || textalllist))
         {
          waynameraw=WayName(ways,resultwayp);
          if(!*waynameraw)
             waynameraw=translation->raw_highway[HIGHWAY(resultwayp->type)];

          bearing_int=(int)BearingAngle(nodes,resultsegmentp,result->node);

          seg_speed=profile->speed[HIGHWAY(resultwayp->type)];
         }

       if(next_result && important>ROUTINO_POINT_JUNCT_CONT)
         {
          if(!first && (htmlfile || htmllist || textfile || textlist))
            {
             if(DISTANCE(resultsegmentp->distance)==0 || DISTANCE(next_resultsegmentp->distance)==0)
                turn_int=0;
             else
                turn_int=(int)TurnAngle(nodes,resultsegmentp,next_resultsegmentp,result->node);

             turn   =translation->xml_turn[((202+turn_int)/45)%8];
             turnraw=translation->notxml_turn[((202+turn_int)/45)%8];
            }

          if(gpxroutefile || htmlfile || htmllist)
            {
             next_waynameraw=WayName(ways,next_resultwayp);
             if(!*next_waynameraw)
                next_waynameraw=translation->raw_highway[HIGHWAY(next_resultwayp->type)];

             next_wayname=ParseXML_Encode_Safe_XML(next_waynameraw);
            }

          if(htmlfile || htmllist || gpxroutefile || textfile || textlist)
            {
             if(!first && DISTANCE(next_resultsegmentp->distance)==0)
                next_bearing_int=(int)BearingAngle(nodes,resultsegmentp,result->node);
             else
                next_bearing_int=(int)BearingAngle(nodes,next_resultsegmentp,next_result->node);

             next_bearing   =translation->xml_heading[(4+(22+next_bearing_int)/45)%8];
             next_bearingraw=translation->notxml_heading[(4+(22+next_bearing_int)/45)%8];
            }
         }

       /* Print out the important points (junctions / waypoints) */

       if(important>ROUTINO_POINT_JUNCT_CONT)
         {
          if(htmlfile)
            {
             char *type;

             if(important==ROUTINO_POINT_WAYPOINT)
                type=translation->html_waypoint;
             else if(important==ROUTINO_POINT_MINI_RB)
                type=translation->html_roundabout;
             else
                type=translation->html_junction;

             if(point_count>0)  /* not the first point */
               {
                /* <tr class='s'><td class='l'>Follow:<td class='r'><span class='h'>*highway name*</span> for <span class='d'>*distance* km, *time* min</span> [<span class='j'>*distance* km, *time* minutes</span>] */
                fprintf(htmlfile,translation->html_segment,
                                 (roundabout>1?translation->html_roundabout:prev_wayname),
                                 distance_to_km(junc_distance),duration_to_minutes(junc_duration));
                fprintf(htmlfile,translation->html_subtotal,
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration));
               }

             /* <tr class='c'><td class='l'>*N*:<td class='r'>*latitude* *longitude* */
             fprintf(htmlfile,"<tr class='c'><td class='l'>%d:<td class='r'>%.6f %.6f\n",
                              point_count+1,
                              radians_to_degrees(latitude),radians_to_degrees(longitude));

             if(point_count==0) /* first point */
               {
                /* <tr class='n'><td class='l'>Start:<td class='r'>At <span class='w'>Waypoint</span>, head <span class='b'>*heading*</span> */
                fprintf(htmlfile,translation->html_start,
                                 translation->html_waypoint,
                                 next_bearing);
               }
             else if(next_result) /* middle point */
               {
                if(roundabout>1 && important!=ROUTINO_POINT_WAYPOINT)
                  {
                   /* <tr class='n'><td class='l'>Leave:<td class='r'>Roundabout, take <span class='t'>the *Nth* exit</span> heading <span class='b'>*heading*</span> */
                   fprintf(htmlfile,translation->html_rbnode,
                                    translation->html_roundabout,
                                    translation->xml_ordinal[roundabout-2],
                                    next_bearing);
                  }
                else
                  {
                   /* <tr class='n'><td class='l'>At:<td class='r'>Junction, go <span class='t'>*direction*</span> heading <span class='b'>*heading*</span> */
                   fprintf(htmlfile,translation->html_node,
                                    type,
                                    turn,
                                    next_bearing);
                  }
               }
             else            /* end point */
               {
                /* <tr class='n'><td class='l'>Stop:<td class='r'>At <span class='w'>Waypoint</span> */
                fprintf(htmlfile,translation->html_stop,
                                 translation->html_waypoint);

                /* <tr class='t'><td class='l'>Total:<td class='r'><span class='j'>*distance* km, *time* minutes</span> */
                fprintf(htmlfile,translation->html_total,
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration));
               }
            }

          if(htmllist)
            {
             int strl;
             char *type;

             if(important==ROUTINO_POINT_WAYPOINT)
                type=translation->nothtml_waypoint;
             else if(important==ROUTINO_POINT_MINI_RB)
                type=translation->nothtml_roundabout;
             else
                type=translation->nothtml_junction;

             if(point_count>0)  /* not the first point */
               {
                /* Follow: *highway name* for *distance* km, *time* min */
                strl=strlen(translation->nothtml_segment)+
                     strlen(roundabout>1?translation->nothtml_roundabout:prev_waynameraw)+8+8+1;

                htmllist->desc2=malloc(strl);

                sprintf(htmllist->desc2,translation->nothtml_segment,
                                        (roundabout>1?translation->nothtml_roundabout:prev_waynameraw),
                                        distance_to_km(junc_distance),duration_to_minutes(junc_duration));

                /* *distance* km, *time* minutes */
                strl=strlen(translation->nothtml_subtotal)+8+8+1;

                htmllist->desc3=malloc(strl);

                sprintf(htmllist->desc3,translation->nothtml_subtotal,
                                        distance_to_km(cum_distance),duration_to_minutes(cum_duration));

                htmllist->dist=distance_to_km(cum_distance);
                htmllist->time=duration_to_minutes(cum_duration);

                htmllist->next=calloc(sizeof(Routino_Output),1);
                htmllist=htmllist->next;
               }

             htmllist->lon=longitude;
             htmllist->lat=latitude;
             htmllist->type=important;

             if(point_count==0) /* first point */
               {
                /* Start: At Waypoint, head *heading* */
                strl=strlen(translation->nothtml_start)+
                     strlen(translation->nothtml_waypoint)+strlen(next_bearingraw)+1;

                htmllist->desc1=malloc(strl);

                sprintf(htmllist->desc1,translation->nothtml_start,
                                        translation->nothtml_waypoint,
                                        next_bearingraw);

                htmllist->name=strcpy(malloc(strlen(next_waynameraw)+1),next_waynameraw);
               }
             else if(next_result) /* middle point */
               {
                if(roundabout>1 && important!=ROUTINO_POINT_WAYPOINT)
                  {
                   /* At: Roundabout, take the *Nth* exit heading *heading* */
                   strl=strlen(translation->nothtml_rbnode)+
                        strlen(translation->nothtml_roundabout)+strlen(translation->notxml_ordinal[roundabout-2])+strlen(next_bearingraw)+1;

                   htmllist->desc1=malloc(strl);

                   sprintf(htmllist->desc1,translation->nothtml_rbnode,
                                           translation->nothtml_roundabout,
                                           translation->notxml_ordinal[roundabout-2],
                                           next_bearingraw);
                  }
                else
                  {
                   /* At: Junction, go *direction* heading *heading* */
                   strl=strlen(translation->nothtml_node)+
                        strlen(type)+strlen(turnraw)+strlen(next_bearingraw)+1;

                   htmllist->desc1=malloc(strl);

                   sprintf(htmllist->desc1,translation->nothtml_node,
                                           type,
                                           turnraw,
                                           next_bearingraw);
                  }

                htmllist->turn=turn_int;
                htmllist->name=strcpy(malloc(strlen(next_waynameraw)+1),next_waynameraw);
               }
             else            /* end point */
               {
                /* Stop: At Waypoint */
                strl=strlen(translation->nothtml_stop)+
                     strlen(translation->nothtml_waypoint)+1;

                htmllist->desc1=malloc(strl);

                sprintf(htmllist->desc1,translation->nothtml_stop,
                                        translation->nothtml_waypoint);

                /* Total: *distance* km, *time* minutes */
                strl=strlen(translation->nothtml_total)+8+8+1;

                htmllist->desc2=malloc(strl);

                sprintf(htmllist->desc2,translation->nothtml_total,
                                        distance_to_km(cum_distance),duration_to_minutes(cum_duration));

                htmllist->turn=turn_int;
               }

             htmllist->bearing=next_bearing_int;
            }

          if(gpxroutefile)
            {
             if(point_count>0) /* not first point */
               {
                fprintf(gpxroutefile,"<desc>");
                fprintf(gpxroutefile,translation->gpx_step,
                                     prev_bearing,
                                     prev_wayname,
                                     distance_to_km(junc_distance),duration_to_minutes(junc_duration));
                fprintf(gpxroutefile,"</desc></rtept>\n");
               }

             if(point_count==0) /* first point */
               {
                fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s</name>\n",
                                     radians_to_degrees(latitude),radians_to_degrees(longitude),
                                     translation->gpx_start);
               }
             else if(!next_result) /* end point */
               {
                fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s</name>\n",
                                     radians_to_degrees(latitude),radians_to_degrees(longitude),
                                     translation->gpx_finish);
                fprintf(gpxroutefile,"<desc>");
                fprintf(gpxroutefile,translation->gpx_final,
                                     distance_to_km(cum_distance),duration_to_minutes(cum_duration));
                fprintf(gpxroutefile,"</desc></rtept>\n");
               }
             else            /* middle point */
               {
                if(important==ROUTINO_POINT_WAYPOINT)
                   fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s%d</name>\n",
                                        radians_to_degrees(latitude),radians_to_degrees(longitude),
                                        translation->gpx_inter,++segment_count);
                else
                   fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>%s%03d</name>\n",
                                        radians_to_degrees(latitude),radians_to_degrees(longitude),
                                        translation->gpx_trip,++route_count);
               }
            }

          if(textfile)
            {
             char *type;

             if(important==ROUTINO_POINT_WAYPOINT)
                type="Waypt";
             else
                type="Junct";

             if(point_count==0) /* first point */
               {
                fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t\t %+d\t%s\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude),
                                 0.0,0.0,0.0,0.0,
                                 type,
                                 ((22+next_bearing_int)/45+4)%8-4,
                                 next_waynameraw);
               }
             else if(!next_result) /* end point */
               {
                fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t\t\t\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude),
                                 distance_to_km(junc_distance),duration_to_minutes(junc_duration),
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                 type);
               }
             else               /* middle point */
               {
                fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%4.0f min\t%s\t %+d\t %+d\t%s\n",
                                 radians_to_degrees(latitude),radians_to_degrees(longitude),
                                 distance_to_km(junc_distance),duration_to_minutes(junc_duration),
                                 distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                 type,
                                 (22+turn_int)/45,
                                 ((22+next_bearing_int)/45+4)%8-4,
                                 next_waynameraw);
               }
            }

          if(textlist)
            {
             textlist->lon=longitude;
             textlist->lat=latitude;
             textlist->type=important;

             if(point_count==0) /* first point */
               {
                textlist->next=calloc(sizeof(Routino_Output),1);

                textlist->bearing=next_bearing_int;
                textlist->name=strcpy(malloc(strlen(next_waynameraw)+1),next_waynameraw);
               }
             else if(!next_result) /* end point */
               {
                textlist->next=NULL;

                textlist->dist=distance_to_km(cum_distance);
                textlist->time=duration_to_minutes(cum_duration);
               }
             else               /* middle point */
               {
                textlist->next=calloc(sizeof(Routino_Output),1);

                textlist->dist=distance_to_km(cum_distance);
                textlist->time=duration_to_minutes(cum_duration);
                textlist->turn=turn_int;
                textlist->bearing=next_bearing_int;
                textlist->name=strcpy(malloc(strlen(next_waynameraw)+1),next_waynameraw);
               }

             textlist=textlist->next;
            }

          junc_distance=0;
          junc_duration=0;

          if(htmlfile || htmllist || gpxroutefile)
            {
             if(prev_wayname)
                free(prev_wayname);

             if(next_wayname)
                prev_wayname=strcpy((char*)malloc(strlen(next_wayname)+1),next_wayname);
             else
                prev_wayname=NULL;

             if(prev_waynameraw)
                free(prev_waynameraw);

             if(next_waynameraw)
                prev_waynameraw=strcpy((char*)malloc(strlen(next_waynameraw)+1),next_waynameraw);
             else
                prev_waynameraw=NULL;
            }

          if(gpxroutefile)
             prev_bearing=next_bearing;

          if(roundabout>1)
             roundabout=0;
         }

       /* Print out all of the results */

       if(gpxtrackfile)
          fprintf(gpxtrackfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"/>\n",
                               radians_to_degrees(latitude),radians_to_degrees(longitude));

       if(important>ROUTINO_POINT_IGNORE)
         {
          if(textallfile)
            {
             char *type;

             if(important==ROUTINO_POINT_WAYPOINT)
                type="Waypt";
             else if(important==ROUTINO_POINT_UTURN)
                type="U-turn";
             else if(important==ROUTINO_POINT_MINI_RB)
                type="Mini-RB";
             else if(important==ROUTINO_POINT_CHANGE)
                type="Change";
             else if(important==ROUTINO_POINT_JUNCT_CONT || important==ROUTINO_POINT_RB_NOT_EXIT)
                type="Junct-";
             else if(important==ROUTINO_POINT_UNIMPORTANT)
                type="Inter";
             else
                type="Junct";

             if(point_count==0) /* first point */
               {
                fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t\t\t\n",
                                    radians_to_degrees(latitude),radians_to_degrees(longitude),
                                    IsFakeNode(result->node)?(NODE_FAKE-result->node):result->node,
                                    (resultnodep && IsSuperNode(resultnodep))?'*':' ',type,
                                    0.0,0.0,0.0,0.0);
               }
             else               /* not the first point */
               {
                fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%s\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%4d\t%s\n",
                                    radians_to_degrees(latitude),radians_to_degrees(longitude),
                                    IsFakeNode(result->node)?(NODE_FAKE-result->node):result->node,
                                    (resultnodep && IsSuperNode(resultnodep))?'*':' ',type,
                                    distance_to_km(seg_distance),duration_to_minutes(seg_duration),
                                    distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                                    speed_to_kph(seg_speed),
                                    bearing_int,
                                    waynameraw);
               }
            }

          if(textalllist)
            {
             if(point_count==0) /* first point */
                ;
             else               /* not the first point */
               {
                textalllist->next=calloc(sizeof(Routino_Output),1);
                textalllist=textalllist->next;

                textalllist->dist=distance_to_km(cum_distance);
                textalllist->time=duration_to_minutes(cum_duration);
                textalllist->speed=speed_to_kph(seg_speed);
                textalllist->bearing=next_bearing_int;
                textalllist->name=strcpy(malloc(strlen(waynameraw)+1),waynameraw);
               }

             textalllist->lon=longitude;
             textalllist->lat=latitude;
             textalllist->type=important;
            }
         }

       if(wayname && wayname!=waynameraw)
          free(wayname);

       result=next_result;

       if(important>ROUTINO_POINT_JUNCT_CONT)
          point_count++;

       first=0;
      }
    while(point==next_point);

    /* Print the end of the segment */

    if(gpxtrackfile)
       fprintf(gpxtrackfile,"</trkseg>\n");

    point=next_point;

    if(result)
       prev_node=result->node;
    else
       prev_node=NO_NODE;
   }
 while(point<nresults);

 /* Print the tail of the files */

 if(htmlfile)
   {
    fprintf(htmlfile,"</table>\n");

    if((translation->xml_copyright_creator[0] && translation->xml_copyright_creator[1]) ||
       (translation->xml_copyright_source[0]  && translation->xml_copyright_source[1]) ||
       (translation->xml_copyright_license[0] && translation->xml_copyright_license[1]))
      {
       fprintf(htmlfile,"<p>\n");
       fprintf(htmlfile,"<table class='c'>\n");
       if(translation->xml_copyright_creator[0] && translation->xml_copyright_creator[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translation->xml_copyright_creator[0],translation->xml_copyright_creator[1]);
       if(translation->xml_copyright_source[0] && translation->xml_copyright_source[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translation->xml_copyright_source[0],translation->xml_copyright_source[1]);
       if(translation->xml_copyright_license[0] && translation->xml_copyright_license[1])
          fprintf(htmlfile,"<tr><td class='l'>%s:<td class='r'>%s\n",translation->xml_copyright_license[0],translation->xml_copyright_license[1]);
       fprintf(htmlfile,"</table>\n");
      }

    fprintf(htmlfile,"</body>\n");
    fprintf(htmlfile,"</html>\n");
   }

 if(gpxtrackfile)
   {
    fprintf(gpxtrackfile,"</trk>\n");
    fprintf(gpxtrackfile,"</gpx>\n");
   }

 if(gpxroutefile)
   {
    fprintf(gpxroutefile,"</rte>\n");
    fprintf(gpxroutefile,"</gpx>\n");
   }

 /* Close the files */

 if(!option_file_stdout)
   {
    if(htmlfile)
       fclose(htmlfile);
    if(gpxtrackfile)
       fclose(gpxtrackfile);
    if(gpxroutefile)
       fclose(gpxroutefile);
    if(textfile)
       fclose(textfile);
    if(textallfile)
       fclose(textallfile);
   }

 return(listhead);
}
