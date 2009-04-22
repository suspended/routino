/***************************************
 $Header: /home/amb/CVS/routino/src/output.c,v 1.1 2009-04-22 18:51:39 amb Exp $

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


#include <string.h>
#include <stdio.h>

#include "types.h"
#include "functions.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "results.h"


/*+ The option to calculate the quickest route insted of the shortest. +*/
extern int option_quickest;


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  Results *Results The set of results to print.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  index_t start The start node.

  index_t finish The finish node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile)
{
 FILE *gpxtrackfile,*gpxroutefile,*textfile,*textallfile;
 float finish_lat,finish_lon;
 float start_lat,start_lon;
 distance_t distance=0;
 duration_t duration=0;
 char *prev_way_name=NULL;
 Result *result;
 int routecount=0;

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

 GetLatLong(nodes,LookupNode(nodes,start),&start_lat,&start_lon);
 GetLatLong(nodes,LookupNode(nodes,finish),&finish_lat,&finish_lon);

 fprintf(gpxtrackfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 fprintf(gpxtrackfile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

 fprintf(gpxtrackfile,"<metadata>\n");
 fprintf(gpxtrackfile,"<desc><![CDATA[%s route between 'start' and 'finish' waypoints]]></desc>\n",option_quickest?"Quickest":"Shortest");
 fprintf(gpxtrackfile,"</metadata>\n");

 fprintf(gpxtrackfile,"<trk>\n");
 fprintf(gpxtrackfile,"<trkseg>\n");

 fprintf(gpxroutefile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 fprintf(gpxroutefile,"<gpx version=\"1.1\" creator=\"Routino\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/1\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n");

 fprintf(gpxroutefile,"<metadata>\n");
 fprintf(gpxroutefile,"<desc><![CDATA[%s route between 'start' and 'finish' waypoints]]></desc>\n",option_quickest?"Quickest":"Shortest");
 fprintf(gpxroutefile,"</metadata>\n");

 fprintf(gpxroutefile,"<rte>\n");
 fprintf(gpxroutefile,"<name>%s route</name>\n",option_quickest?"Quickest":"Shortest");

               /* "%10.6f\t%11.6f\t%5.3f km\t%5.1f min\t%5.1f km\t%3.0f min\t%s\n" */
 fprintf(textfile,"#Latitude\tLongitude\tSegment \tSegment \tTotal   \tTotal  \tHighway\n");
 fprintf(textfile,"#        \t         \tDistance\tDuration\tDistance\tDurat'n\t       \n");

                  /* "%10.6f\t%11.6f\t%8d%c\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%s\n" */
 fprintf(textallfile,"#Latitude\tLongitude\t    Node\tSegment\tSegment\tTotal\tTotal  \tSpeed\tHighway\n");
 fprintf(textallfile,"#        \t         \t        \tDist   \tDurat'n\tDist \tDurat'n\t     \t       \n");

 result=FindResult(results,start);

 do
   {
    float latitude,longitude;
    Node *node=LookupNode(nodes,result->node);

    GetLatLong(nodes,node,&latitude,&longitude);

    fprintf(gpxtrackfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"/>\n",
            (180/M_PI)*latitude,(180/M_PI)*longitude);

    if(result->prev)
      {
       Segment *segment;
       Way *way;
       char *way_name;

       segment=FirstSegment(segments,LookupNode(nodes,result->prev));
       while(OtherNode(segment,result->prev)!=result->node)
          segment=NextSegment(segments,segment,result->prev);

       way=LookupWay(ways,segment->way);

       distance+=DISTANCE(segment->distance);
       duration+=Duration(segment,way,profile);
       way_name=WayName(ways,way);

       if(!result->next || (IsSuperNode(node) && way_name!=prev_way_name))
         {
          if(result->next)
             fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>TRIP%03d</name></rtept>\n",
                     (180/M_PI)*latitude,(180/M_PI)*longitude,
                     ++routecount);
          else
             fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>FINISH</name></rtept>\n",
                     (180/M_PI)*latitude,(180/M_PI)*longitude);

          fprintf(textfile,"%10.6f\t%11.6f\t%5.3f km\t%5.1f min\t%5.1f km\t%3.0f min\t%s\n",
                  (180/M_PI)*latitude,(180/M_PI)*longitude,
                  distance_to_km(distance),duration_to_minutes(duration),
                  distance_to_km(result->distance),duration_to_minutes(result->duration),
                  way_name);

          prev_way_name=way_name;
          distance=0;
          duration=0;
         }

       fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%5.3f\t%5.2f\t%5.2f\t%5.1f\t%3d\t%s\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude,
               result->node,IsSuperNode(node)?'*':' ',
               distance_to_km(DISTANCE(segment->distance)),duration_to_minutes(Duration(segment,way,profile)),
               distance_to_km(result->distance),duration_to_minutes(result->duration),
               profile->speed[HIGHWAY(way->type)],way_name);
      }
    else
      {
       fprintf(gpxroutefile,"<rtept lat=\"%.6f\" lon=\"%.6f\"><name>START</name></rtept>\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude);

       fprintf(textfile,"%10.6f\t%11.6f\t%5.3f km\t%5.1f min\t%5.1f km\t%3.0f min\t\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude,
               0.0,0.0,0.0,0.0);

       fprintf(textallfile,"%10.6f\t%11.6f\t%8d%c\t%5.3f\t%5.2f\t%5.2f\t%5.1f\n",
               (180/M_PI)*latitude,(180/M_PI)*longitude,
               result->node,IsSuperNode(node)?'*':' ',
               0.0,0.0,0.0,0.0);
      }

    if(result->next)
       result=FindResult(results,result->next);
    else
       result=NULL;
   }
    while(result);

 fprintf(gpxtrackfile,"</trkseg>\n");
 fprintf(gpxtrackfile,"</trk>\n");
 fprintf(gpxtrackfile,"</gpx>\n");

 fprintf(gpxroutefile,"</rte>\n");
 fprintf(gpxroutefile,"</gpx>\n");

 fclose(gpxtrackfile);
 fclose(gpxroutefile);
 fclose(textfile);
 fclose(textallfile);
}
