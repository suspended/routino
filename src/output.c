/***************************************
 $Header: /home/amb/CVS/routino/src/output.c,v 1.6 2009-05-13 18:34:35 amb Exp $

 Routing output generator.

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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "types.h"
#include "functions.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "results.h"


/*+ The option to calculate the quickest route insted of the shortest. +*/
extern int option_quickest;

/*+ The files to write to. +*/
static FILE *gpxtrackfile=NULL,*gpxroutefile=NULL,*textfile=NULL,*textallfile=NULL;

/*+ The final latitude, longitude point. +*/
static float finish_latitude,finish_longitude;

/*+ Heuristics for determining if a junction is important. +*/
static char junction_other_way[Way_Unknown][Way_Unknown]=
 { /* M, T, P, S, T, U, R, S, T, P, B, C, F = Way type of route not taken */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 }, /* Motorway     */
  {   1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Trunk        */
  {   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Primary      */
  {   1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* Secondary    */
  {   1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 }, /* Tertiary     */
  {   1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 }, /* Unclassified */
  {   1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 }, /* Residential  */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 }, /* Service      */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Track        */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Path         */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Bridleway    */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Cycleway     */
  {   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, /* Footway      */
 };


/*++++++++++++++++++++++++++++++++++++++
  Open the files and print the head.

  const char *copyright The name of a file that might exist and contain copyright information.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRouteHead(const char *copyright)
{
 char *source=NULL,*license=NULL;

 if(copyright)
   {
    struct stat buf;

    if(!stat(copyright,&buf))
      {
       FILE *file=fopen(copyright,"r");
       char *string=(char*)malloc(buf.st_size+1);
       char *p;

       fread(string,buf.st_size,1,file);
       string[buf.st_size]=0;

       p=string;
       while(*p)
         {
          if(!strncmp(p,"Source:",7))
            {
             p+=7;
             while(*p==' ' || *p=='t')
                p++;
             source=p;
             while(*p && *p!='\r' && *p!='\n')
                p++;
             while(*p=='\r' || *p=='\n')
                *p++=0;
            }
          else if(!strncmp(p,"License:",8) || !strncmp(p,"Licence:",8))
            {
             p+=8;
             while(*p==' ' || *p=='t')
                p++;
             license=p;
             while(*p && *p!='\r' && *p!='\n')
                p++;
             while(*p=='\r' || *p=='\n')
                *p++=0;
            }
          else
            {
             while(*p && *p!='\r' && *p!='\n')
                p++;
             while(*p=='\r' || *p=='\n')
                *p++=0;
            }
         }

       fclose(file);
      }
   }

 /* Open the files */

 if(option_quickest==0)
   {
    /* Print the result for the shortest route */

    gpxtrackfile=fopen("shortest-track.gpx","w");
    gpxroutefile=fopen("shortest-route.gpx","w");
    textfile    =fopen("shortest.txt","w");
    textallfile =fopen("shortest-all.txt","w");
   }
 else
   {
    /* Print the result for the quickest route */

    gpxtrackfile=fopen("quickest-track.gpx","w");
    gpxroutefile=fopen("quickest-route.gpx","w");
    textfile    =fopen("quickest.txt","w");
    textallfile =fopen("quickest-all.txt","w");
   }

 /* Print the head of the files */

 fprintf(gpxtrackfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 fprintf(gpxtrackfile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

 fprintf(gpxtrackfile,"<metadata>\n");
 fprintf(gpxtrackfile,"<desc><![CDATA[%s route between 'start' and 'finish' waypoints]]></desc>\n",option_quickest?"Quickest":"Shortest");
 if(source)
    fprintf(gpxtrackfile,"<copyright author=\"%s\">\n",source);
 if(license)
    fprintf(gpxtrackfile,"<license>%s</license>\n",license);
 if(source)
    fprintf(gpxtrackfile,"</copyright>\n");
 fprintf(gpxtrackfile,"</metadata>\n");

 fprintf(gpxtrackfile,"<trk>\n");

 fprintf(gpxroutefile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 fprintf(gpxroutefile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

 fprintf(gpxroutefile,"<metadata>\n");
 fprintf(gpxroutefile,"<desc><![CDATA[%s route between 'start' and 'finish' waypoints]]></desc>\n",option_quickest?"Quickest":"Shortest");
 if(source)
    fprintf(gpxroutefile,"<copyright author=\"%s\">\n",source);
 if(license)
    fprintf(gpxroutefile,"<license>%s</license>\n",license);
 if(source)
    fprintf(gpxroutefile,"</copyright>\n");
 fprintf(gpxroutefile,"</metadata>\n");

 fprintf(gpxroutefile,"<rte>\n");
 fprintf(gpxroutefile,"<name>%s route</name>\n",option_quickest?"Quickest":"Shortest");

 if(source)
    fprintf(textfile,"# Source: %s\n",source);
 if(license)
    fprintf(textfile,"# License: %s\n",license);
 if(source || license)
    fprintf(textfile,"#\n");
 fprintf(textfile,"#Latitude\tLongitude\tSegment \tSegment \tTotal   \tTotal  \tHighway\n");
 fprintf(textfile,"#        \t         \tDistance\tDuration\tDistance\tDurat'n\t       \n");
               /* "%10.6f\t%11.6f\t%5.3f km\t%5.1f min\t%5.1f km\t%3.0f min\t%s\n" */

 if(source)
    fprintf(textallfile,"# Source: %s\n",source);
 if(license)
    fprintf(textallfile,"# License: %s\n",license);
 if(source || license)
    fprintf(textallfile,"#\n");
 fprintf(textallfile,"#Latitude\tLongitude\t    Node\tSegment\tSegment\tTotal\tTotal  \tSpeed\tHighway\n");
 fprintf(textallfile,"#        \t         \t        \tDist   \tDurat'n\tDist \tDurat'n\t     \t       \n");
                  /* "%10.6f\t%11.6f\t%8d%c\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%s\n" */
}


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  Results *Results The set of results to print.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile)
{
 static distance_t cum_distance=0;
 static duration_t cum_duration=0;
 static int segment_count=0;
 static int route_count=0;
 float finish_lat,finish_lon;
 float start_lat,start_lon;
 distance_t junc_distance=0;
 duration_t junc_duration=0;
 Result *result;

 fprintf(gpxtrackfile,"<trkseg>\n");

 GetLatLong(nodes,results->start,&start_lat,&start_lon);
 GetLatLong(nodes,results->finish,&finish_lat,&finish_lon);

 result=FindResult(results,results->start);

 do
   {
    float latitude,longitude;

    GetLatLong(nodes,result->node,&latitude,&longitude);

    fprintf(gpxtrackfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"/>\n",
            (180/M_PI)*latitude,(180/M_PI)*longitude);

    if(result->prev)
      {
       distance_t seg_distance=0;
       duration_t seg_duration=0;
       Segment *segment;
       Way *resultway;
       int other=0,change=0;

       /* Get the properties of this segment */

       resultway=LookupWay(ways,result->segment->way);

       seg_distance+=DISTANCE(result->segment->distance);
       seg_duration+=Duration(result->segment,resultway,profile);
       junc_distance+=seg_distance;
       junc_duration+=seg_duration;
       cum_distance+=seg_distance;
       cum_duration+=seg_duration;

       /* Decide if this is an important junction */

       segment=FirstSegment(segments,nodes,result->node);

       do
         {
          index_t othernode=NODE(OtherNode(segment,result->node));

          if(othernode!=result->prev)
             if(IsNormalSegment(segment) && (!profile->oneway || !IsOnewayTo(segment,result->node)))
               {
                Way *way=LookupWay(ways,segment->way);

                if(othernode==result->next) /* the next segment that we follow */
                  {
                   if(HIGHWAY(way->type)!=HIGHWAY(resultway->type))
                      change=1;
                  }
                else /* a segment that we don't follow */
                   if(!(way->type&Way_Roundabout))
                      other+=junction_other_way[HIGHWAY(resultway->type)][HIGHWAY(way->type)];
               }

          segment=NextSegment(segments,segment,result->node);
         }
       while(segment);

       /* Print out the junctions */

       if(!result->next || other || change)
         {
          if(result->next)
             fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>TRIP%03d</name></rtept>\n",
                     (180/M_PI)*latitude,(180/M_PI)*longitude,
                     ++route_count);
          else
            {
             finish_latitude=latitude;
             finish_longitude=longitude;
            }

          fprintf(textfile,"%10.6f\t%11.6f\t%6.3f km\t%4.1f min\t%5.1f km\t%3.0f min\t%s\n",
                  (180/M_PI)*latitude,(180/M_PI)*longitude,
                  distance_to_km(junc_distance),duration_to_minutes(junc_duration),
                  distance_to_km(cum_distance),duration_to_minutes(cum_duration),
                  WayName(ways,resultway));

          junc_distance=0;
          junc_duration=0;
         }

       /* Print out all of the results */

       fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%s\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude,
               result->node,IsSuperNode(nodes,result->node)?'*':' ',
               distance_to_km(seg_distance),duration_to_minutes(seg_duration),
               distance_to_km(cum_distance),duration_to_minutes(cum_duration),
               profile->speed[HIGHWAY(resultway->type)],WayName(ways,resultway));
      }
    else if(!cum_distance)
      {
       /* Print out the start points */

       fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>START</name></rtept>\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude);

       fprintf(textfile,"%10.6f\t%11.6f\t%5.3f km\t%5.1f min\t%5.1f km\t%3.0f min\t\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude,
               0.0,0.0,0.0,0.0);

       fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%5.3f\t%5.2f\t%5.2f\t%5.1f\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude,
               result->node,IsSuperNode(nodes,result->node)?'*':' ',
               0.0,0.0,0.0,0.0);
      }
    else
      {
       /* Print out the intermediate points */

       fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>INTER%d</name></rtept>\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude,
               ++segment_count);
      }

    if(result->next)
       result=FindResult(results,result->next);
    else
       result=NULL;
   }
    while(result);

 fprintf(gpxtrackfile,"</trkseg>\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print the tail and close the files.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRouteTail(void)
{
 /* Print the final point in the route */

 fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>FINISH</name></rtept>\n",
         (180/M_PI)*finish_latitude,(180/M_PI)*finish_longitude);

 /* Print the tail of the files */

 fprintf(gpxtrackfile,"</trk>\n");
 fprintf(gpxtrackfile,"</gpx>\n");

 fprintf(gpxroutefile,"</rte>\n");
 fprintf(gpxroutefile,"</gpx>\n");

 /* Close the files */

 fclose(gpxtrackfile);
 fclose(gpxroutefile);
 fclose(textfile);
 fclose(textallfile);
}
