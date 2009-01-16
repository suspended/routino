/***************************************
 $Header: /home/amb/CVS/routino/src/optimiser.c,v 1.15 2009-01-16 20:04:47 amb Exp $

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
#define NBINS     256


/*+ A queue results. +*/
typedef struct _Queue
{
 uint32_t alloced;              /*+ The amount of space allocated for results in the array +*/
 uint32_t number;               /*+ The number of occupied results in the array +*/
 Result *queue[1024];           /*+ An array of results whose size is not
                                    necessarily limited to 1024 (i.e. may
                                    overflow the end of this structure). +*/
}
 Queue;


/*+ The queue of nodes. +*/
Queue *OSMQueue=NULL;

/*+ Print the progress? +*/
int print_progress=1;


/* Functions */

static void insert_in_queue(Result *result);


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes.

  Results *FindRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  node_t start The start node.

  node_t finish The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute(Nodes *nodes,Segments *segments,node_t start,node_t finish)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 HalfResult shortestfinish,quickestfinish;
 Result *result1,*result2;
 Segment *segment;
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

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while(OSMQueue->number>0)
   {
    result1=OSMQueue->queue[--OSMQueue->number];
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       node2=segment->node2;

       if(segment->distance==INVALID_SHORT_DISTANCE ||
          segment->duration==INVALID_SHORT_DURATION)
          goto endloop;

       shortest2.distance=shortest1.distance+segment->distance;
       shortest2.duration=shortest1.duration+segment->duration;
       quickest2.distance=quickest1.distance+segment->distance;
       quickest2.duration=quickest1.duration+segment->duration;

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
             insert_in_queue(result2);
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
                insert_in_queue(result2);
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
                insert_in_queue(result2);
            }
         }

      endloop:

       segment=FindNextSegment(segments,segment);
      }

    if(print_progress && !(nresults%1000))
      {
       printf("\rRouting: End Nodes=%d Queue=%d Journey=%.1fkm,%.0fmin  ",nresults,OSMQueue->number,
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

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  node_t start The start node.

  node_t finish The finish node.

  Results *begin The initial portion of the route.

  Results *end The final portion of the route.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute3(Nodes *nodes,Segments *segments,node_t start,node_t finish,Results *begin,Results *end)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 HalfResult shortestfinish,quickestfinish;
 Result *result1,*result2,*result3;
 Segment *segment;
 int nresults=0;
 int j,k;

 /* Set up the finish conditions */

 shortestfinish.distance=INVALID_DISTANCE;
 shortestfinish.duration=INVALID_DURATION;
 quickestfinish.distance=INVALID_DISTANCE;
 quickestfinish.duration=INVALID_DURATION;

 /* Calculate the start and finish nodes */

 results=NewResultsList();

 result1=InsertResult(results,start);
 result2=FindResult(begin,start);

 *result1=*result2;

 result1=InsertResult(results,finish);
 result2=FindResult(end,finish);

 *result1=*result2;

 /* Insert the finish points of the beginning part of the path into the queue */

 for(j=0;j<NBINS_RESULTS;j++)
    for(k=0;k<begin->number[j];k++)
       if(FindNode(nodes,begin->results[j][k]->node))
         {
          if(!(result1=FindResult(results,begin->results[j][k]->node)))
            {
             result1=InsertResult(results,begin->results[j][k]->node);

             *result1=*begin->results[j][k];
            }

          if(result1->node!=start)
            {
             result1->shortest.prev=start;
             result1->quickest.prev=start;

             insert_in_queue(result1);
            }
         }

 /* Loop across all nodes in the queue */

 while(OSMQueue->number>0)
   {
    result1=OSMQueue->queue[--OSMQueue->number];
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       node2=segment->node2;

       if(segment->distance==INVALID_SHORT_DISTANCE ||
          segment->duration==INVALID_SHORT_DURATION)
          goto endloop;

       shortest2.distance=shortest1.distance+segment->distance;
       shortest2.duration=shortest1.duration+segment->duration;
       quickest2.distance=quickest1.distance+segment->distance;
       quickest2.duration=quickest1.duration+segment->duration;

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
             insert_in_queue(result2);
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
                insert_in_queue(result2);
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
                insert_in_queue(result2);
            }
         }

      endloop:

       segment=FindNextSegment(segments,segment);
      }

    if(print_progress && !(nresults%1000))
      {
       printf("\rRouting: End Nodes=%d Queue=%d Journey=%.1fkm,%.0fmin  ",nresults,OSMQueue->number,
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

 /* Finish off the end part of the route. */

 result2=FindResult(results,finish);
 result2->shortest.distance=INVALID_DISTANCE;
 result2->shortest.duration=INVALID_DURATION;
 result2->quickest.distance=INVALID_DISTANCE;
 result2->quickest.duration=INVALID_DURATION;

 for(j=0;j<NBINS_RESULTS;j++)
    for(k=0;k<end->number[j];k++)
       if(FindNode(nodes,end->results[j][k]->node))
          if((result1=FindResult(results,end->results[j][k]->node)))
            {
             if((result1->shortest.distance+end->results[j][k]->shortest.distance)<result2->shortest.distance ||
                ((result1->shortest.distance+end->results[j][k]->shortest.distance)==result2->shortest.distance &&
                 (result1->shortest.duration+end->results[j][k]->shortest.duration)<result2->shortest.duration))
               {
                result2->shortest.distance=result1->shortest.distance+end->results[j][k]->shortest.distance;
                result2->shortest.duration=result1->shortest.duration+end->results[j][k]->shortest.duration;
                result2->shortest.prev=result1->node;
               }
             if((result1->quickest.duration+end->results[j][k]->quickest.duration)<result2->quickest.duration ||
                ((result1->quickest.duration+end->results[j][k]->quickest.duration)==result2->quickest.duration &&
                 (result1->quickest.distance+end->results[j][k]->quickest.distance)<result2->quickest.distance))
               {
                result2->quickest.distance=result1->quickest.distance+end->results[j][k]->quickest.distance;
                result2->quickest.duration=result1->quickest.duration+end->results[j][k]->quickest.duration;
                result2->quickest.prev=result1->node;
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

  node_t start The start node.

  node_t finish The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,node_t start,node_t finish)
{
 FILE *file;
 Result *result;

 /* Print the result for the shortest route */

 file=fopen("shortest.txt","w");

 result=FindResult(results,start);

 do
   {
    Node *node=FindNode(nodes,result->node);

    if(result->shortest.prev)
      {
       Segment *segment;
       Way *way;

       segment=FindFirstSegment(segments,result->shortest.prev);
       while(segment->node2!=result->node)
          segment=FindNextSegment(segments,segment);

       way=FindWay(ways,segment->way);

       fprintf(file,"%9.5f %9.5f %5.3f %5.2f %6.1f %5.1f %3d %s\n",node->latitude,node->longitude,
               distance_to_km(segment->distance),duration_to_minutes(segment->duration),
               distance_to_km(result->shortest.distance),duration_to_minutes(result->shortest.duration),
               (way->limit?way->limit:way->speed),WayName(ways,way));
      }
    else
       fprintf(file,"%9.5f %9.5f %5.3f %5.2f %6.1f %5.1f\n",node->latitude,node->longitude,
               0.0,0.0,0.0,0.0);

    if(result->shortest.next)
       result=FindResult(results,result->shortest.next);
    else
       result=NULL;
   }
 while(result);

 fclose(file);

 /* Print the result for the quickest route */

 file=fopen("quickest.txt","w");

 result=FindResult(results,start);

 do
   {
    Node *node=FindNode(nodes,result->node);

    if(result->quickest.prev)
      {
       Segment *segment;
       Way *way;

       segment=FindFirstSegment(segments,result->quickest.prev);
       while(segment->node2!=result->node)
          segment=FindNextSegment(segments,segment);

       way=FindWay(ways,segment->way);

       fprintf(file,"%9.5f %9.5f %5.3f %5.2f %6.1f %5.1f %3d %s\n",node->latitude,node->longitude,
               distance_to_km(segment->distance),duration_to_minutes(segment->duration),
               distance_to_km(result->quickest.distance),duration_to_minutes(result->quickest.duration),
               (way->limit?way->limit:way->speed),WayName(ways,way));
      }
    else
       fprintf(file,"%9.5f %9.5f %5.3f %5.2f %6.1f %5.1f\n",node->latitude,node->longitude,
               0.0,0.0,0.0,0.0);

    if(result->quickest.next)
       result=FindResult(results,result->quickest.next);
    else
       result=NULL;
   }
 while(result);

 fclose(file);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any node in the specified list.

  Results *FindRoute2 Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  node_t start The start node.

  Nodes *finish The finishing nodes.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoutes(Nodes *nodes,Segments *segments,node_t start,Nodes *finish)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
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

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while(OSMQueue->number>0)
   {
    result1=OSMQueue->queue[--OSMQueue->number];
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
       node2=segment->node2;

       if(segment->distance==INVALID_SHORT_DISTANCE ||
          segment->duration==INVALID_SHORT_DURATION)
          goto endloop;

       shortest2.distance=shortest1.distance+segment->distance;
       shortest2.duration=shortest1.duration+segment->duration;
       quickest2.distance=quickest1.distance+segment->distance;
       quickest2.duration=quickest1.duration+segment->duration;

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
             insert_in_queue(result2);
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
                insert_in_queue(result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.prev=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!FindNode(finish,node2))
                insert_in_queue(result2);
            }
         }

      endloop:

       segment=FindNextSegment(segments,segment);
      }
   }

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any node in the specified list.

  Results *FindRoute2 Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Nodes *start The starting nodes.

  node_t finish The finishing node.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindReverseRoutes(Nodes *nodes,Segments *segments,Nodes *start,node_t finish)
{
 Results *results;
 node_t node1,node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
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

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while(OSMQueue->number>0)
   {
    result1=OSMQueue->queue[--OSMQueue->number];
    node1=result1->node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(segments,node1);

    while(segment)
      {
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

       if(reversesegment->distance==INVALID_SHORT_DISTANCE ||
          reversesegment->duration==INVALID_SHORT_DURATION)
          goto endloop;

       shortest2.distance=shortest1.distance+reversesegment->distance;
       shortest2.duration=shortest1.duration+reversesegment->duration;
       quickest2.distance=quickest1.distance+reversesegment->distance;
       quickest2.duration=quickest1.duration+reversesegment->duration;

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
             insert_in_queue(result2);
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
                insert_in_queue(result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.next=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!FindNode(start,node2))
                insert_in_queue(result2);
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

  Results *results The set of results.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  Nodes *supernodes The list of supernodes.

  Segments *supersegments The list of supersegments.

  node_t start The start node.

  node_t finish The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes,Segments *supersegments,node_t start,node_t finish)
{
 Result *result1,*result2,*result3,*result4;
 Results *combined;

 combined=NewResultsList();

 print_progress=0;

 /* Sort out the shortest */

 result1=FindResult(results,start);
 result4=NULL;

 do
   {
    result3=InsertResult(combined,result1->node);

    result3->node=result1->node;

    result3->shortest=result1->shortest;
    result3->shortest.next=0;
    if(result4)
       result3->shortest.prev=result4->node;
    else
       result3->shortest.prev=0;

    result4=result3;

    if(result1->shortest.next)
      {
       Results *results2=FindRoute(nodes,segments,result1->node,result1->shortest.next);

       result2=FindResult(results2,result1->node);

       result3->shortest.next=result2->shortest.next;

       result2=FindResult(results2,result2->shortest.next);

       do
         {
          if(result2->shortest.prev && result2->shortest.next)
            {
             result4=InsertResult(combined,result2->node);

             result4->node=result2->node;

             result4->shortest=result2->shortest;
             result4->shortest.distance+=result3->shortest.distance;
             result4->shortest.duration+=result3->shortest.duration;
            }

          if(result2->shortest.next)
             result2=FindResult(results2,result2->shortest.next);
          else
             result2=NULL;
         }
       while(result2);

       FreeResultsList(results2);

       result1=FindResult(results,result1->shortest.next);
      }
    else
       result1=NULL;
   }
 while(result1);

 /* Sort out the quickest */

 result1=FindResult(results,start);
 result4=NULL;

 do
   {
    result3=FindResult(combined,result1->node);

    if(!result3)
      {
       result3=InsertResult(combined,result1->node);

       result3->node=result1->node;
      }

    result3->quickest=result1->quickest;
    result3->quickest.next=0;
    if(result4)
       result3->quickest.prev=result4->node;
    else
       result3->quickest.prev=0;

    result4=result3;

    if(result1->quickest.next)
      {
       Results *results2=FindRoute(nodes,segments,result1->node,result1->quickest.next);

       result2=FindResult(results2,result1->node);

       result3->quickest.next=result2->quickest.next;

       result2=FindResult(results2,result2->quickest.next);

       do
         {
          if(result2->quickest.prev && result2->quickest.next)
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
            }

          if(result2->quickest.next)
             result2=FindResult(results2,result2->quickest.next);
          else
             result2=NULL;
         }
       while(result2);

       FreeResultsList(results2);

       result1=FindResult(results,result1->quickest.next);
      }
    else
       result1=NULL;
   }
 while(result1);

 /* Now print out the result normally */

 print_progress=1;

 PrintRoute(combined,nodes,segments,ways,start,finish);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert an item into the queue in the right order.

  Result *result The result to insert into the queue.
  ++++++++++++++++++++++++++++++++++++++*/

static void insert_in_queue(Result *result)
{
 int start;
 int end;
 int mid;
 int insert=-1;

 /* Check that the whole thing is allocated. */

 if(!OSMQueue)
   {
    OSMQueue=(Queue*)malloc(sizeof(Queue)-sizeof(OSMQueue->queue)+INCREMENT*sizeof(Result*));

    OSMQueue->alloced=INCREMENT;
    OSMQueue->number=0;
   }

 /* Check that the arrays have enough space. */

 if(OSMQueue->number==OSMQueue->alloced)
   {
    OSMQueue->alloced+=INCREMENT;
    OSMQueue=(Queue*)realloc((void*)OSMQueue,sizeof(Queue)-sizeof(OSMQueue->queue)+OSMQueue->alloced*sizeof(Result*));
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

 start=0;
 end=OSMQueue->number-1;

 if(OSMQueue->number==0)                                                      /* There is nothing in the queue */
    insert=start;
 else if(result->shortest.distance>OSMQueue->queue[start]->shortest.distance) /* Check key is not before start */
    insert=start;
 else if(result->shortest.distance<OSMQueue->queue[end]->shortest.distance)   /* Check key is not after end */
    insert=end+1;
 else
   {
    do
      {
       mid=(start+end)/2;                                                         /* Choose mid point */

       if(OSMQueue->queue[mid]->shortest.distance>result->shortest.distance)      /* Mid point is too low */
          start=mid;
       else if(OSMQueue->queue[mid]->shortest.distance<result->shortest.distance) /* Mid point is too high */
          end=mid;
       else                                                                       /* Mid point is correct */
         {
          if(OSMQueue->queue[mid]==result)
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

 if(insert!=OSMQueue->number)
    memmove(&OSMQueue->queue[insert+1],&OSMQueue->queue[insert],(OSMQueue->number-insert)*sizeof(Result*));

 /* Insert the new entry */

 OSMQueue->queue[insert]=result;

 OSMQueue->number++;
}
