/***************************************
 $Header: /home/amb/CVS/routino/src/optimiser.c,v 1.30 2009-01-23 17:09:41 amb Exp $

 Routing optimiser.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "nodes.h"
#include "ways.h"
#include "segments.h"
#include "results.h"
#include "functions.h"


/*+ Print the progress? +*/
int print_progress=1;


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes.

  Results *FindRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  node_t start The start node.

  node_t finish The finish node.

  transport_t transport The mode of transport.

  int highways[] An array of flags indicating the highway type.

  int all A flag to indicate that a big results structure is required.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute(Nodes *nodes,Segments *segments,Ways *ways,node_t start,node_t finish,transport_t transport,int highways[],int all)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 HalfResult shortestfinish,quickestfinish;
 Result *result1,*result2;
 Segment *segment;
 Way *way;
 wayallow_t mustallow=1<<(transport-1);

 /* Set up the finish conditions */

 shortestfinish.distance=~0;
 shortestfinish.duration=~0;
 quickestfinish.distance=~0;
 quickestfinish.duration=~0;

 /* Insert the first node into the queue */

 if(all)
    results=NewResultsList(65536);
 else
    results=NewResultsList(8);

 result1=InsertResult(results,start);

 result1->node=start;
 result1->shortest.prev=0;
 result1->shortest.next=0;
 result1->shortest.distance=0;
 result1->shortest.duration=0;
 result1->quickest.prev=0;
 result1->quickest.next=0;
 result1->quickest.distance=0;
 result1->quickest.duration=0;

 insert_in_queue(results,result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue(results)))
   {
    node1=result1->node;
    shortest1=result1->shortest;
    quickest1=result1->quickest;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       duration_t segment_duration;

       if(segment->distance&ONEWAY_OPPOSITE)
          goto endloop;

       node2=segment->node2;

       if(shortest1.prev==node2 && quickest1.prev==node2)
          goto endloop;

       way=FindWay(ways,segment->way);

       if(!(way->allow&mustallow))
          goto endloop;

       if(!highways[HIGHWAY(way->type)])
          goto endloop;

       segment_duration=Duration(segment,way,transport);

       shortest2.distance=shortest1.distance+segment->distance;
       shortest2.duration=shortest1.duration+segment_duration;
       quickest2.distance=quickest1.distance+segment->distance;
       quickest2.duration=quickest1.duration+segment_duration;

       if(shortest2.distance>shortestfinish.distance && quickest2.duration>quickestfinish.duration)
          goto endloop;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=node1;
          result2->shortest.next=0;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=node1;
          result2->quickest.next=0;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(node2==finish)
            {
             shortestfinish.distance=shortest2.distance;
             shortestfinish.duration=shortest2.duration;
             quickestfinish.distance=quickest2.distance;
             quickestfinish.duration=quickest2.duration;
            }
          else
             insert_in_queue(results,result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.prev=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(node2==finish)
               {
                shortestfinish.distance=shortest2.distance;
                shortestfinish.duration=shortest2.duration;
               }
             else
                insert_in_queue(results,result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.prev=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(node2==finish)
               {
                quickestfinish.distance=quickest2.distance;
                quickestfinish.duration=quickest2.duration;
               }
             else
                insert_in_queue(results,result2);
            }
         }

      endloop:

       segment=FindNextSegment(segments,segment);
      }

    if(print_progress && !(results->number%10000))
      {
       printf("\rRouting: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin  ",results->number,
              distance_to_km(shortest2.distance),duration_to_minutes(shortest2.duration),
              distance_to_km(quickest2.distance),duration_to_minutes(quickest2.duration));
       fflush(stdout);
      }
   }

 if(print_progress)
   {
    printf("\rRouted: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin\n",results->number,
           distance_to_km(shortestfinish.distance),duration_to_minutes(shortestfinish.duration),
           distance_to_km(quickestfinish.distance),duration_to_minutes(quickestfinish.duration));
    fflush(stdout);
   }

 /* Reverse the results */

 result2=FindResult(results,finish);

 do
   {
    if(result2->shortest.prev)
      {
       node_t node1=result2->shortest.prev;

       result1=FindResult(results,node1);

       result1->shortest.next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 result2=FindResult(results,finish);

 do
   {
    if(result2->quickest.prev)
      {
       node_t node1=result2->quickest.prev;

       result1=FindResult(results,node1);

       result1->quickest.next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes.

  Results *FindRoute3 Returns a set of results.

  Nodes *supernodes The set of supernodes to use.

  Segments *supersegments The set of supersegments to use.

  Ways *superways The set of superways to use.

  node_t start The start node.

  node_t finish The finish node.

  Results *begin The initial portion of the route.

  Results *end The final portion of the route.

  transport_t transport The mode of transport.

  int highways[] An array of flags indicating the highway type.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute3(Nodes *supernodes,Segments *supersegments,Ways *superways,node_t start,node_t finish,Results *begin,Results *end,transport_t transport,int highways[])
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 HalfResult shortestfinish,quickestfinish;
 Result *result1,*result2,*result3;
 Segment *segment;
 Way *way;
 wayallow_t mustallow=1<<(transport-1);
 int j;

 /* Set up the finish conditions */

 shortestfinish.distance=~0;
 shortestfinish.duration=~0;
 quickestfinish.distance=~0;
 quickestfinish.duration=~0;

 /* Insert the start node */

 results=NewResultsList(65536);

 result1=InsertResult(results,start);
 result2=FindResult(begin,start);

 *result1=*result2;

 /* Insert the finish points of the beginning part of the path into the queue */

 for(j=0;j<begin->number;j++)
    if(FindNode(supernodes,begin->results[j].node))
      {
       if(!(result1=FindResult(results,begin->results[j].node)))
         {
          result1=InsertResult(results,begin->results[j].node);

          *result1=begin->results[j];

          result1->shortest.prev=start;
          result1->quickest.prev=start;
         }

       insert_in_queue(results,result1);
      }

 /* Loop across all supernodes in the queue */

 while((result1=pop_from_queue(results)))
   {
    node1=result1->node;
    shortest1=result1->shortest;
    quickest1=result1->quickest;

    segment=FindFirstSegment(supersegments,node1);

    while(segment)
      {
       duration_t segment_duration;

       if(segment->distance&ONEWAY_OPPOSITE)
          goto endloop;

       node2=segment->node2;

       if(shortest1.prev==node2 && quickest1.prev==node2)
          goto endloop;

       way=FindWay(superways,segment->way);

       if(!(way->allow&mustallow))
          goto endloop;

       if(!highways[HIGHWAY(way->type)])
          goto endloop;

       segment_duration=Duration(segment,way,transport);

       shortest2.distance=shortest1.distance+segment->distance;
       shortest2.duration=shortest1.duration+segment_duration;
       quickest2.distance=quickest1.distance+segment->distance;
       quickest2.duration=quickest1.duration+segment_duration;

       if(shortest2.distance>shortestfinish.distance && quickest2.duration>quickestfinish.duration)
          goto endloop;

       result2=FindResult(results,node2);
       result3=NULL;

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=node1;
          result2->shortest.next=0;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=node1;
          result2->quickest.next=0;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(!(result3=FindResult(end,node2)))
             insert_in_queue(results,result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.prev=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(!(result3=FindResult(end,node2)))
                insert_in_queue(results,result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.prev=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!(result3=FindResult(end,node2)))
                insert_in_queue(results,result2);
            }
         }

       if(result3)
         {
          if((shortest2.distance+result3->shortest.distance)<shortestfinish.distance ||
             ((shortest2.distance+result3->shortest.distance)==shortestfinish.distance &&
              (shortest2.duration+result3->shortest.duration)<shortestfinish.duration))
            {
             shortestfinish.distance=shortest2.distance+result3->shortest.distance;
             shortestfinish.duration=shortest2.duration+result3->shortest.duration;
            }
          if((quickest2.duration+result3->quickest.duration)<quickestfinish.duration ||
             ((quickest2.duration+result3->quickest.duration)==quickestfinish.duration &&
              (quickest2.distance+result3->quickest.distance)<quickestfinish.distance))
            {
             quickestfinish.distance=quickest2.distance+result3->quickest.distance;
             quickestfinish.duration=quickest2.duration+result3->quickest.duration;
            }
         }

      endloop:

       segment=FindNextSegment(supersegments,segment);
      }

    if(print_progress && !(results->number%10000))
      {
       printf("\rRouting: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin  ",results->number,
              distance_to_km(shortest2.distance),duration_to_minutes(shortest2.duration),
              distance_to_km(quickest2.distance),duration_to_minutes(quickest2.duration));
       fflush(stdout);
      }
   }

 if(print_progress)
   {
    printf("\rRouted: End Super-Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin\n",results->number,
           distance_to_km(shortestfinish.distance),duration_to_minutes(shortestfinish.duration),
           distance_to_km(quickestfinish.distance),duration_to_minutes(quickestfinish.duration));
    fflush(stdout);
   }

 /* Finish off the end part of the route. */

 if(!FindResult(results,finish))
   {
    result2=InsertResult(results,finish);
    result1=FindResult(end,finish);

    *result2=*result1;

    result2->shortest.distance=~0;
    result2->shortest.duration=~0;
    result2->quickest.distance=~0;
    result2->quickest.duration=~0;

    for(j=0;j<end->number;j++)
       if(FindNode(supernodes,end->results[j].node))
          if((result1=FindResult(results,end->results[j].node)))
            {
             if((result1->shortest.distance+end->results[j].shortest.distance)<result2->shortest.distance ||
                ((result1->shortest.distance+end->results[j].shortest.distance)==result2->shortest.distance &&
                 (result1->shortest.duration+end->results[j].shortest.duration)<result2->shortest.duration))
               {
                result2->shortest.distance=result1->shortest.distance+end->results[j].shortest.distance;
                result2->shortest.duration=result1->shortest.duration+end->results[j].shortest.duration;
                result2->shortest.prev=result1->node;
               }
             if((result1->quickest.duration+end->results[j].quickest.duration)<result2->quickest.duration ||
                ((result1->quickest.duration+end->results[j].quickest.duration)==result2->quickest.duration &&
                 (result1->quickest.distance+end->results[j].quickest.distance)<result2->quickest.distance))
               {
                result2->quickest.distance=result1->quickest.distance+end->results[j].quickest.distance;
                result2->quickest.duration=result1->quickest.duration+end->results[j].quickest.duration;
                result2->quickest.prev=result1->node;
               }
            }
   }

 /* Reverse the results */

 result2=FindResult(results,finish);

 do
   {
    if(result2->shortest.prev)
      {
       node_t node1=result2->shortest.prev;

       result1=FindResult(results,node1);

       result1->shortest.next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 result2=FindResult(results,finish);

 do
   {
    if(result2->quickest.prev)
      {
       node_t node1=result2->quickest.prev;

       result1=FindResult(results,node1);

       result1->quickest.next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  Results *Results The set of results to print.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  Nodes *supernodes The list of super-nodes.

  node_t start The start node.

  node_t finish The finish node.

  transport_t transport The mode of transport.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes,node_t start,node_t finish,transport_t transport)
{
 FILE *textfile,*gpxfile;
 Result *result;

 /* Print the result for the shortest route */

 textfile=fopen("shortest.txt","w");
 gpxfile=fopen("shortest.gpx","w");

 fprintf(gpxfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 fprintf(gpxfile,"<gpx version=\"1.0\" creator=\"Router\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/0\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n");
 fprintf(gpxfile,"<trk>\n");
 fprintf(gpxfile,"<trkseg>\n");

 result=FindResult(results,start);

 do
   {
    Node *node=FindNode(nodes,result->node);

    fprintf(gpxfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"></trkpt>\n",node->latitude,node->longitude);

    if(result->shortest.prev)
      {
       Segment *segment;
       Way *way;

       segment=FindFirstSegment(segments,result->shortest.prev);
       while(segment->node2!=result->node)
          segment=FindNextSegment(segments,segment);

       way=FindWay(ways,segment->way);

       fprintf(textfile,"%8.4f %9.4f %10d%c %5.3f %5.2f %7.2f %5.1f %3d %s\n",node->latitude,node->longitude,
               node->id,supernodes?(FindNode(supernodes,node->id)?'*':' '):' ',
               distance_to_km(segment->distance),duration_to_minutes(Duration(segment,way,transport)),
               distance_to_km(result->shortest.distance),duration_to_minutes(result->shortest.duration),
               WaySpeed(way),WayName(ways,way));
      }
    else
       fprintf(textfile,"%8.4f %9.4f %10d%c %5.3f %5.2f %7.2f %5.1f\n",node->latitude,node->longitude,
               node->id,supernodes?(FindNode(supernodes,node->id)?'*':' '):' ',
               0.0,0.0,0.0,0.0);

    if(result->shortest.next)
       result=FindResult(results,result->shortest.next);
    else
       result=NULL;
   }
 while(result);

 fprintf(gpxfile,"</trkseg>\n");
 fprintf(gpxfile,"</trk>\n");
 fprintf(gpxfile,"</gpx>\n");

 fclose(textfile);
 fclose(gpxfile);

 /* Print the result for the quickest route */

 textfile=fopen("quickest.txt","w");
 gpxfile=fopen("quickest.gpx","w");

 fprintf(gpxfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 fprintf(gpxfile,"<gpx version=\"1.0\" creator=\"Router\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/0\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n");
 fprintf(gpxfile,"<trk>\n");
 fprintf(gpxfile,"<trkseg>\n");

 result=FindResult(results,start);

 do
   {
    Node *node=FindNode(nodes,result->node);

    fprintf(gpxfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"></trkpt>\n",node->latitude,node->longitude);

    if(result->quickest.prev)
      {
       Segment *segment;
       Way *way;

       segment=FindFirstSegment(segments,result->quickest.prev);
       while(segment->node2!=result->node)
          segment=FindNextSegment(segments,segment);

       way=FindWay(ways,segment->way);

       fprintf(textfile,"%8.4f %9.4f %10d%c %5.3f %5.2f %7.2f %5.1f %3d %s\n",node->latitude,node->longitude,
               node->id,supernodes?(FindNode(supernodes,node->id)?'*':' '):' ',
               distance_to_km(segment->distance),duration_to_minutes(Duration(segment,way,transport)),
               distance_to_km(result->quickest.distance),duration_to_minutes(result->quickest.duration),
               WaySpeed(way),WayName(ways,way));
      }
    else
       fprintf(textfile,"%8.4f %9.4f %10d%c %5.3f %5.2f %7.2f %5.1f\n",node->latitude,node->longitude,
               node->id,supernodes?(FindNode(supernodes,node->id)?'*':' '):' ',
               0.0,0.0,0.0,0.0);

    if(result->quickest.next)
       result=FindResult(results,result->quickest.next);
    else
       result=NULL;
   }
 while(result);

 fprintf(gpxfile,"</trkseg>\n");
 fprintf(gpxfile,"</trk>\n");
 fprintf(gpxfile,"</gpx>\n");

 fclose(textfile);
 fclose(gpxfile);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any node in the specified list.

  Results *FindRoutes Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  node_t start The start node.

  Nodes *finish The finishing nodes.

  transport_t transport The mode of transport.

  int highways[] An array of flags indicating the highway type.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoutes(Nodes *nodes,Segments *segments,Ways *ways,node_t start,Nodes *finish,transport_t transport,int highways[])
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;
 wayallow_t mustallow=1<<(transport-1);

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 result1=InsertResult(results,start);

 result1->node=start;
 result1->shortest.prev=0;
 result1->shortest.next=0;
 result1->shortest.distance=0;
 result1->shortest.duration=0;
 result1->quickest.prev=0;
 result1->quickest.next=0;
 result1->quickest.distance=0;
 result1->quickest.duration=0;

 insert_in_queue(results,result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue(results)))
   {
    node1=result1->node;
    shortest1=result1->shortest;
    quickest1=result1->quickest;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       duration_t segment_duration;

       if(segment->distance&ONEWAY_OPPOSITE)
          goto endloop;

       node2=segment->node2;

       if(shortest1.prev==node2 && quickest1.prev==node2)
          goto endloop;

       way=FindWay(ways,segment->way);

       if(!(way->allow&mustallow))
          goto endloop;

       if(!highways[HIGHWAY(way->type)])
          goto endloop;

       segment_duration=Duration(segment,way,transport);

       shortest2.distance=shortest1.distance+segment->distance;
       shortest2.duration=shortest1.duration+segment_duration;
       quickest2.distance=quickest1.distance+segment->distance;
       quickest2.duration=quickest1.duration+segment_duration;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=node1;
          result2->shortest.next=0;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=node1;
          result2->quickest.next=0;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(!FindNode(finish,node2))
             insert_in_queue(results,result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.prev=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(!FindNode(finish,node2))
                insert_in_queue(results,result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.prev=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!FindNode(finish,node2))
                insert_in_queue(results,result2);
            }
         }

      endloop:

       segment=FindNextSegment(segments,segment);
      }
   }

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from any node in the specified list to a specific node.

  Results *FindReverseRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Nodes *start The starting nodes.

  node_t finish The finishing node.

  transport_t transport The mode of transport.

  int highways[] An array of flags indicating the highway type.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindReverseRoutes(Nodes *nodes,Segments *segments,Ways *ways,Nodes *start,node_t finish,transport_t transport,int highways[])
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;
 wayallow_t mustallow=1<<(transport-1);

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 result1=InsertResult(results,finish);

 result1->node=finish;
 result1->shortest.prev=0;
 result1->shortest.next=0;
 result1->shortest.distance=0;
 result1->shortest.duration=0;
 result1->quickest.prev=0;
 result1->quickest.next=0;
 result1->quickest.distance=0;
 result1->quickest.duration=0;

 insert_in_queue(results,result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue(results)))
   {
    node1=result1->node;
    shortest1=result1->shortest;
    quickest1=result1->quickest;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       duration_t reversesegment_duration;

       /* Reverse the segment and check it exists */

       node_t reversenode;
       Segment *reversesegment;

       reversenode=segment->node2;
       reversesegment=FindFirstSegment(segments,reversenode);

       while(reversesegment && reversesegment->node2!=node1)
          reversesegment=FindNextSegment(segments,reversesegment);

       if(!reversesegment)
          goto endloop;

       if(reversesegment->distance&ONEWAY_OPPOSITE)
          goto endloop;

       node2=reversesegment->node1;

       if(shortest1.prev==node2 && quickest1.prev==node2)
          goto endloop;

       way=FindWay(ways,reversesegment->way);

       if(!(way->allow&mustallow))
          goto endloop;

       if(!highways[HIGHWAY(way->type)])
          goto endloop;

       reversesegment_duration=Duration(reversesegment,way,transport);

       shortest2.distance=shortest1.distance+reversesegment->distance;
       shortest2.duration=shortest1.duration+reversesegment_duration;
       quickest2.distance=quickest1.distance+reversesegment->distance;
       quickest2.duration=quickest1.duration+reversesegment_duration;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=0;
          result2->shortest.next=node1;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=0;
          result2->quickest.next=node1;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(!FindNode(start,node2))
             insert_in_queue(results,result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.next=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(!FindNode(start,node2))
                insert_in_queue(results,result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.next=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!FindNode(start,node2))
                insert_in_queue(results,result2);
            }
         }

      endloop:

       segment=FindNextSegment(segments,segment);
      }
   }

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  Results *CombineRoutes Returns the results from joining the super-nodes.

  Results *results The set of results from the super-nodes.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  node_t start The start node.

  node_t finish The finish node.

  transport_t transport The mode of transport.

  int highways[] An array of flags indicating the highway type.
  ++++++++++++++++++++++++++++++++++++++*/

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,node_t start,node_t finish,transport_t transport,int highways[])
{
 Result *result1,*result2,*result3,*result4;
 Results *combined;

 combined=NewResultsList(64);

 print_progress=0;

 /* Sort out the shortest */

 result1=FindResult(results,start);

 result3=InsertResult(combined,start);

 result3->node=result1->node;

 result3->shortest.distance=0;
 result3->shortest.duration=0;
 result3->shortest.next=0;
 result3->shortest.prev=0;

 do
   {
    if(result1->shortest.next)
      {
       Results *results2=FindRoute(nodes,segments,ways,result1->node,result1->shortest.next,transport,highways,0);

       result2=FindResult(results2,result1->node);

       result3->shortest.next=result2->shortest.next;

       result2=FindResult(results2,result2->shortest.next);

       do
         {
          result4=InsertResult(combined,result2->node);

          result4->node=result2->node;

          result4->shortest=result2->shortest;
          result4->shortest.distance+=result3->shortest.distance;
          result4->shortest.duration+=result3->shortest.duration;

          if(result2->shortest.next)
             result2=FindResult(results2,result2->shortest.next);
          else
             result2=NULL;
         }
       while(result2);

       FreeResultsList(results2);

       result1=FindResult(results,result1->shortest.next);

       result3=result4;
      }
    else
       result1=NULL;
   }
 while(result1);

 /* Sort out the quickest */

 result1=FindResult(results,start);

 result3=FindResult(combined,start);

 result3->quickest.distance=0;
 result3->quickest.duration=0;
 result3->quickest.next=0;
 result3->quickest.prev=0;

 do
   {
    if(result1->quickest.next)
      {
       Results *results2=FindRoute(nodes,segments,ways,result1->node,result1->quickest.next,transport,highways,0);

       result2=FindResult(results2,result1->node);

       result3->quickest.next=result2->quickest.next;

       result2=FindResult(results2,result2->quickest.next);

       do
         {
          result4=FindResult(combined,result2->node);

          if(!result4)
            {
             result4=InsertResult(combined,result2->node);

             result4->node=result2->node;
            }

          result4->quickest=result2->quickest;
          result4->quickest.distance+=result3->quickest.distance;
          result4->quickest.duration+=result3->quickest.duration;

          if(result2->quickest.next)
             result2=FindResult(results2,result2->quickest.next);
          else
             result2=NULL;
         }
       while(result2);

       FreeResultsList(results2);

       result1=FindResult(results,result1->quickest.next);

       result3=result4;
      }
    else
       result1=NULL;
   }
 while(result1);

 /* Now print out the result normally */

 print_progress=1;

 return(combined);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any node in the specified list that follows a certain type of way.

  Results *FindRoutesWay Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  node_t start The start node.

  Nodes *finish The finishing nodes.

  Way *match The way that the route must match.

  int iteration The iteration number in Super-Segment generation.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoutesWay(Nodes *nodes,Segments *segments,Ways *ways,node_t start,Nodes *finish,Way *match,int iteration)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 result1=InsertResult(results,start);

 result1->node=start;
 result1->shortest.prev=0;
 result1->shortest.next=0;
 result1->shortest.distance=0;
 result1->shortest.duration=0;
 result1->quickest.prev=0;
 result1->quickest.next=0;
 result1->quickest.distance=0;
 result1->quickest.duration=0;

 insert_in_queue(results,result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue(results)))
   {
    node1=result1->node;
    shortest1=result1->shortest;
    quickest1=result1->quickest;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       duration_t segment_duration;

       if(segment->distance&ONEWAY_OPPOSITE)
          goto endloop;

       node2=segment->node2;

       if(shortest1.prev==node2 && quickest1.prev==node2)
          goto endloop;

       way=FindWay(ways,segment->way);

       if(way->type !=match->type  ||
          way->allow!=match->allow ||
          way->limit!=match->limit)
          goto endloop;

       segment_duration=0;

       shortest2.distance=shortest1.distance+segment->distance;
       shortest2.duration=shortest1.duration+segment_duration;
       quickest2.distance=quickest1.distance+segment->distance;
       quickest2.duration=quickest1.duration+segment_duration;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=node1;
          result2->shortest.next=0;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=node1;
          result2->quickest.next=0;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(!FindNode(finish,node2))
             insert_in_queue(results,result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.prev=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(!FindNode(finish,node2))
                insert_in_queue(results,result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.prev=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!FindNode(finish,node2))
                insert_in_queue(results,result2);
            }
         }

      endloop:

       segment=FindNextSegment(segments,segment);
      }
   }

 return(results);
}
