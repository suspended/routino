/***************************************
 $Header: /home/amb/CVS/routino/src/optimiser.c,v 1.22 2009-01-20 17:37:20 amb Exp $

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


#define INCREMENT 1024


/*+ A queue results. +*/
typedef struct _Queue
{
 uint32_t alloced;              /*+ The amount of space allocated for results in the array. +*/
 uint32_t number;               /*+ The number of occupied results in the array. +*/
 uint32_t *queue;               /*+ An array of offsets into the results array. +*/
}
 Queue;


/*+ The queue of nodes. +*/
Queue OSMQueue={0,0,NULL};

/*+ Print the progress? +*/
int print_progress=1;


/* Functions */

static void insert_in_queue(Results *results,Result *result);


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes.

  Results *FindRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  node_t start The start node.

  node_t finish The finish node.

  wayallow_t transport The mode of transport.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute(Nodes *nodes,Segments *segments,Ways *ways,node_t start,node_t finish,wayallow_t transport)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 HalfResult shortestfinish,quickestfinish;
 Result *result1,*result2;
 Segment *segment;
 Way *way;
 int nresults=0;

 /* Set up the finish conditions */

 shortestfinish.distance=INVALID_DISTANCE;
 shortestfinish.duration=INVALID_DURATION;
 quickestfinish.distance=INVALID_DISTANCE;
 quickestfinish.duration=INVALID_DURATION;

 /* Insert the first node into the queue */

 results=NewResultsList();

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

 while(OSMQueue.number>0)
   {
    result1=LookupResult(results,OSMQueue.queue[--OSMQueue.number]);
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       duration_t segment_duration;
       speed_t segment_speed;

       node2=segment->node2;

       way=FindWay(ways,segment->way);

       if((way->allow&transport)!=transport)
          goto endloop;

       if(segment->distance==INVALID_DISTANCE)
          goto endloop;

       if(way->limit)
          segment_speed=way->limit;
       else
          segment_speed=way->speed;

       segment_duration=hours_to_duration(distance_to_km(segment->distance)/segment_speed);

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

          nresults++;

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

    if(print_progress && !(nresults%1000))
      {
       printf("\rRouting: End Nodes=%d Queue=%d Journey=%.1fkm,%.0fmin  ",nresults,OSMQueue.number,
              distance_to_km(shortest2.distance),duration_to_minutes(quickest2.duration));
       fflush(stdout);
      }
   }

 if(print_progress)
   {
    printf("\rRouted: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin\n",nresults,
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

  wayallow_t transport The mode of transport.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute3(Nodes *supernodes,Segments *supersegments,Ways *superways,node_t start,node_t finish,Results *begin,Results *end,wayallow_t transport)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 HalfResult shortestfinish,quickestfinish;
 Result *result1,*result2,*result3;
 Segment *segment;
 Way *way;
 int nresults=0;
 int j;

 /* Set up the finish conditions */

 shortestfinish.distance=INVALID_DISTANCE;
 shortestfinish.duration=INVALID_DURATION;
 quickestfinish.distance=INVALID_DISTANCE;
 quickestfinish.duration=INVALID_DURATION;

 /* Insert the start node */

 results=NewResultsList();

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

 while(OSMQueue.number>0)
   {
    result1=LookupResult(results,OSMQueue.queue[--OSMQueue.number]);
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(supersegments,node1);

    while(segment)
      {
       duration_t segment_duration;
       speed_t segment_speed;

       node2=segment->node2;

       way=FindWay(superways,segment->way);

       if((way->allow&transport)!=transport)
          goto endloop;

       if(segment->distance==INVALID_DISTANCE)
          goto endloop;

       if(way->limit)
          segment_speed=way->limit;
       else
          segment_speed=way->speed;

       segment_duration=hours_to_duration(distance_to_km(segment->distance)/segment_speed);

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

          nresults++;

          if((result3=FindResult(end,node2)))
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

             if((result3=FindResult(end,node2)))
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

             if((result3=FindResult(end,node2)))
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
             else
                insert_in_queue(results,result2);
            }
         }

      endloop:

       segment=FindNextSegment(supersegments,segment);
      }

    if(print_progress && !(nresults%1000))
      {
       printf("\rRouting: End Super-Nodes=%d Queue=%d Journey=%.1fkm,%.0fmin  ",nresults,OSMQueue.number,
              distance_to_km(shortest2.distance),duration_to_minutes(quickest2.duration));
       fflush(stdout);
      }
   }

 if(print_progress)
   {
    printf("\rRouted: End Super-Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin\n",nresults,
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

    result2->shortest.distance=INVALID_DISTANCE;
    result2->shortest.duration=INVALID_DURATION;
    result2->quickest.distance=INVALID_DISTANCE;
    result2->quickest.duration=INVALID_DURATION;

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
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes,node_t start,node_t finish)
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
               distance_to_km(segment->distance),duration_to_minutes(0),
               distance_to_km(result->shortest.distance),duration_to_minutes(result->shortest.duration),
               (way->limit?way->limit:way->speed),WayName(ways,way));
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
               distance_to_km(segment->distance),duration_to_minutes(0),
               distance_to_km(result->quickest.distance),duration_to_minutes(result->quickest.duration),
               (way->limit?way->limit:way->speed),WayName(ways,way));
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

  wayallow_t transport The mode of transport.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoutes(Nodes *nodes,Segments *segments,Ways *ways,node_t start,Nodes *finish,wayallow_t transport)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;
 int nresults=0;

 /* Insert the first node into the queue */

 results=NewResultsList();

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

 while(OSMQueue.number>0)
   {
    result1=LookupResult(results,OSMQueue.queue[--OSMQueue.number]);
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       duration_t segment_duration;
       speed_t segment_speed;

       node2=segment->node2;

       way=FindWay(ways,segment->way);

       if((way->allow&transport)!=transport)
          goto endloop;

       if(segment->distance==INVALID_DISTANCE)
          goto endloop;

       if(way->limit)
          segment_speed=way->limit;
       else
          segment_speed=way->speed;

       segment_duration=hours_to_duration(distance_to_km(segment->distance)/segment_speed);

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

          nresults++;

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

  wayallow_t transport The mode of transport.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindReverseRoutes(Nodes *nodes,Segments *segments,Ways *ways,Nodes *start,node_t finish,wayallow_t transport)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;
 int nresults=0;

 /* Insert the first node into the queue */

 results=NewResultsList();

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

 while(OSMQueue.number>0)
   {
    result1=LookupResult(results,OSMQueue.queue[--OSMQueue.number]);
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       duration_t reversesegment_duration;
       speed_t reversesegment_speed;

       /* Reverse the segment and check it exists */

       node_t reversenode;
       Segment *reversesegment;

       reversenode=segment->node2;
       reversesegment=FindFirstSegment(segments,reversenode);

       while(reversesegment && reversesegment->node2!=node1)
          reversesegment=FindNextSegment(segments,reversesegment);

       if(!reversesegment)
          goto endloop;

       node2=reversesegment->node1;

       way=FindWay(ways,reversesegment->way);

       if((way->allow&transport)!=transport)
          goto endloop;

       if(reversesegment->distance==INVALID_DISTANCE)
          goto endloop;

       if(way->limit)
          reversesegment_speed=way->limit;
       else
          reversesegment_speed=way->speed;

       reversesegment_duration=hours_to_duration(distance_to_km(reversesegment->distance)/reversesegment_speed);

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

          nresults++;

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

  wayallow_t transport The mode of transport.
  ++++++++++++++++++++++++++++++++++++++*/

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,node_t start,node_t finish,wayallow_t transport)
{
 Result *result1,*result2,*result3,*result4;
 Results *combined;

 combined=NewResultsList();

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
       Results *results2=FindRoute(nodes,segments,ways,result1->node,result1->shortest.next,transport);

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
       Results *results2=FindRoute(nodes,segments,ways,result1->node,result1->quickest.next,transport);

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
 int nresults=0;

 /* Insert the first node into the queue */

 results=NewResultsList();

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

 while(OSMQueue.number>0)
   {
    result1=LookupResult(results,OSMQueue.queue[--OSMQueue.number]);
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       duration_t segment_duration;
       speed_t segment_speed;

       node2=segment->node2;

       way=FindWay(ways,segment->way);

       if(Way_TYPE(way->type)!=Way_TYPE(match->type) ||
          way->allow!=match->allow ||
          way->limit!=match->limit)
          goto endloop;

       if(segment->distance==INVALID_DISTANCE)
          goto endloop;

       if(way->limit)
          segment_speed=way->limit;
       else
          segment_speed=way->speed;

       segment_duration=hours_to_duration(distance_to_km(segment->distance)/segment_speed);

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

          nresults++;

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
  Insert an item into the queue in the right order.

  Results *results The set of results.

  Result *result The result to insert into the queue.
  ++++++++++++++++++++++++++++++++++++++*/

static void insert_in_queue(Results *results,Result *result)
{
 int start=0;
 int end=OSMQueue.number-1;
 int mid;
 int insert=-1;

 /* Check that the whole thing is allocated. */

 if(!OSMQueue.queue)
   {
    OSMQueue.alloced=INCREMENT;
    OSMQueue.number=0;
    OSMQueue.queue=(uint32_t*)malloc(OSMQueue.alloced*sizeof(uint32_t));
   }

 /* Check that the arrays have enough space. */

 if(OSMQueue.number==OSMQueue.alloced)
   {
    OSMQueue.alloced+=INCREMENT;
    OSMQueue.queue=(uint32_t*)realloc((void*)OSMQueue.queue,OSMQueue.alloced*sizeof(uint32_t));
   }

 /* Binary search - search key may not match, new insertion point required
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since there may not be an exact match we must set end=mid
  *  # <- mid    |  or start=mid because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually end=start+1 and the insertion point is before
  *  # <- end    |  end (since it cannot be before the initial start or end).
  */

 if(OSMQueue.number==0)                                                                        /* There is nothing in the queue */
    insert=0;
 else if(result->shortest.distance>results->results[OSMQueue.queue[start]].shortest.distance) /* Check key is not before start */
    insert=start;
 else if(result->shortest.distance<results->results[OSMQueue.queue[end]].shortest.distance)   /* Check key is not after end */
    insert=end+1;
 else
   {
    do
      {
       mid=(start+end)/2;                                                                          /* Choose mid point */

       if(results->results[OSMQueue.queue[mid]].shortest.distance>result->shortest.distance)      /* Mid point is too low */
          start=mid;
       else if(results->results[OSMQueue.queue[mid]].shortest.distance<result->shortest.distance) /* Mid point is too high */
          end=mid;
       else                                                                                        /* Mid point is correct */
         {
          if(&results->results[OSMQueue.queue[mid]]==result)
             return;

          insert=mid;
          break;
         }
      }
    while((end-start)>1);

    if(insert==-1)
       insert=end;
   }

 /* Shuffle the array up */

 if(insert!=OSMQueue.number)
    memmove(&OSMQueue.queue[insert+1],&OSMQueue.queue[insert],(OSMQueue.number-insert)*sizeof(uint32_t));

 /* Insert the new entry */

 OSMQueue.queue[insert]=result-results->results;

 OSMQueue.number++;
}
