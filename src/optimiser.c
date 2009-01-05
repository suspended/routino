/***************************************
 $Header: /home/amb/CVS/routino/src/optimiser.c,v 1.7 2009-01-05 18:43:31 amb Exp $

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

#include "functions.h"
#include "types.h"

#define INCREMENT 1024
#define NBINS     256

/*+ One part of the result for a node. +*/
typedef struct _HalfResult
{
 Node       *Prev;              /*+ The previous Node following the shortest path. +*/
 distance_t  distance;          /*+ The distance travelled to the node following the shortest path. +*/
 duration_t  duration;          /*+ The time taken to the node following the shortest path. +*/
}
 HalfResult;

/*+ One complete result for a node. +*/
typedef struct _Result
{
 node_t     node;               /*+ The end node. +*/
 Node      *Node;               /*+ The end Node. +*/
 HalfResult shortest;           /*+ The result for the shortest path. +*/
 HalfResult quickest;           /*+ The result for the quickest path. +*/
}
 Result;

/*+ A list of results. +*/
typedef struct _Results
{
 uint32_t alloced;              /*+ The amount of space allocated for results in the array +*/
 uint32_t number[NBINS];        /*+ The number of occupied results in the array +*/
 Result **results[NBINS];       /*+ An array of pointers to arrays of results +*/
}
 Results;


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

/*+ The list of results. +*/
Results *OSMResults=NULL;

/* Functions */

static void insert_in_queue(Result *result);
static Result *insert_result(node_t node);
static Result *find_result(node_t node);


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes.

  node_t start The start node.

  node_t finish The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

void FindRoute(node_t start,node_t finish)
{
 Node *Start,*Finish;
 node_t node2;
 Node *Node1,*Node2;
 HalfResult shortest1,quickest1;
 HalfResult shortest2,quickest2;
 HalfResult shortestfinish,quickestfinish;
 distance_t totalcrow,crow;
 Result *result1,*result2;
 Segment *segment;
 int nresults=0;

 /* Set up the finish conditions */

 shortestfinish.distance=~0;
 shortestfinish.duration=~0;
 quickestfinish.distance=~0;
 quickestfinish.duration=~0;

 /* Work out the distance as the crow flies */

 Start=FindNode(start);
 Finish=FindNode(finish);

 totalcrow=Distance(Start,Finish);

 /* Insert the first node into the queue */

 result1=insert_result(start);

 result1->node=start;
 result1->Node=Start;
 result1->shortest.Prev=NULL;
 result1->shortest.distance=0;
 result1->shortest.duration=0;
 result1->quickest.Prev=NULL;
 result1->quickest.distance=0;
 result1->quickest.duration=0;

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while(OSMQueue->number>0)
   {
    result1=OSMQueue->queue[--OSMQueue->number];
    Node1=result1->Node;
    shortest1.distance=result1->shortest.distance;
    shortest1.duration=result1->shortest.duration;
    quickest1.distance=result1->quickest.distance;
    quickest1.duration=result1->quickest.duration;

    segment=FindFirstSegment(Node1->id);

    while(segment)
      {
       node2=segment->node2;

       shortest2.distance=shortest1.distance+segment->distance;
       shortest2.duration=shortest1.duration+segment->duration;
       quickest2.distance=quickest1.distance+segment->distance;
       quickest2.duration=quickest1.duration+segment->duration;

       result2=find_result(node2);
       if(result2)
          Node2=result2->Node;
       else
          Node2=FindNode(node2);

       crow=Distance(Node2,Finish);

       if((crow+shortest2.distance)>(km_to_distance(10)+1.4*totalcrow))
          goto endloop;

       if(shortest2.distance>shortestfinish.distance && quickest2.duration>quickestfinish.duration)
          goto endloop;

       if(!result2)                         /* New end node */
         {
          result2=insert_result(node2);
          result2->node=node2;
          result2->Node=Node2;
          result2->shortest.Prev=Node1;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.Prev=Node1;
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
             result2->shortest.Prev=Node1;
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
             result2->quickest.Prev=Node1;
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

       segment=FindNextSegment(segment);
      }

    if(!(nresults%1000))
      {
       printf("\rRouting: End Nodes=%d Queue=%d Journey=%.1fkm,%.0fmin  ",nresults,OSMQueue->number,
              distance_to_km(shortest2.distance),duration_to_minutes(quickest2.duration));
       fflush(stdout);
      }
   }

 printf("\rRouted: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin\n",nresults,
        distance_to_km(shortestfinish.distance),duration_to_minutes(shortestfinish.duration),
        distance_to_km(quickestfinish.distance),duration_to_minutes(quickestfinish.duration));
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  node_t start The start node.

  node_t finish The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(node_t start,node_t finish)
{
 FILE *file;
 Result *result;
// int i,j;

 /* Print the result for the shortest route */

 file=fopen("shortest.txt","w");

 result=find_result(finish);

 do
   {
    if(result->shortest.Prev)
      {
       Segment *segment;
       Way *way;

       segment=FindFirstSegment(result->shortest.Prev->id);
       while(segment->node2!=result->Node->id)
          segment=FindNextSegment(segment);

       way=FindWay(segment->way);

       fprintf(file,"%9.5f %9.5f %9d %5.3f %5.2f %3.0f %s\n",result->Node->latitude,result->Node->longitude,result->node,
               distance_to_km(segment->distance),duration_to_minutes(segment->duration),
               distance_to_km(segment->distance)/duration_to_hours(segment->duration),
               WayName(way));

       result=find_result(result->shortest.Prev->id);
      }
    else
      {
       fprintf(file,"%9.5f %9.5f %9d\n",result->Node->latitude,result->Node->longitude,result->node);

       result=NULL;
      }
   }
 while(result);

 fclose(file);

 /* Print the result for the quickest route */

 file=fopen("quickest.txt","w");

 result=find_result(finish);

 do
   {
    if(result->quickest.Prev)
      {
       Segment *segment;
       Way *way;

       segment=FindFirstSegment(result->quickest.Prev->id);
       while(segment->node2!=result->Node->id)
          segment=FindNextSegment(segment);

       way=FindWay(segment->way);

       fprintf(file,"%9.5f %9.5f %9d %5.3f %5.2f %3.0f %s\n",result->Node->latitude,result->Node->longitude,result->node,
               distance_to_km(segment->distance),duration_to_minutes(segment->duration),
               distance_to_km(segment->distance)/duration_to_hours(segment->duration),
               WayName(way));

       result=find_result(result->quickest.Prev->id);
      }
    else
      {
       fprintf(file,"%9.5f %9.5f %9d\n",result->Node->latitude,result->Node->longitude,result->node);

       result=NULL;
      }
   }
 while(result);

 /* Print all the distance results. */

// file=fopen("distance.txt","w");
//
// for(i=0;i<NBINS;i++)
//    for(j=0;j<OSMResults->number[i];j++)
//      {
//       result=OSMResults->results[i][j];
//
//       fprintf(file,"%9.5f %9.5f 0 %5.3f\n",result->Node->latitude,result->Node->longitude,
//               distance_to_km(result->shortest.distance));
//      }
//
// fclose(file);

 /* Print all the duration results. */

// file=fopen("duration.txt","w");
//
// for(i=0;i<NBINS;i++)
//    for(j=0;j<OSMResults->number[i];j++)
//      {
//       result=OSMResults->results[i][j];
//
//       fprintf(file,"%9.5f %9.5f 0 %5.3f\n",result->Node->latitude,result->Node->longitude,
//               duration_to_minutes(result->quickest.duration));
//      }
//
// fclose(file);
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


/*++++++++++++++++++++++++++++++++++++++
  Insert a new result into the results data structure in the right order.

  node_t node The node that is to be inserted into the results.
  ++++++++++++++++++++++++++++++++++++++*/

static Result *insert_result(node_t node)
{
 int start;
 int end;
 int mid;
 int insert=-1;
 int bin=node%NBINS;

 /* Check that the whole thing is allocated. */

 if(!OSMResults)
   {
    int i;

    OSMResults=(Results*)calloc(1,sizeof(Results));
    OSMResults->alloced=INCREMENT;

    for(i=0;i<NBINS;i++)
       OSMResults->results[i]=(Result**)malloc(OSMResults->alloced*sizeof(Result*));
   }

 /* Check that the arrays have enough space. */

 if(OSMResults->number[bin]==OSMResults->alloced)
   {
    int i;

    OSMResults->alloced+=INCREMENT;

    for(i=0;i<NBINS;i++)
       OSMResults->results[i]=(Result**)realloc((void*)OSMResults->results[i],OSMResults->alloced*sizeof(Result*));
   }

 /* Binary search - search key may not match, if not then insertion point required
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
 end=OSMResults->number[bin]-1;

 if(OSMResults->number[bin]==0)                      /* There are no results */
    insert=start;
 else if(node<OSMResults->results[bin][start]->node) /* Check key is not before start */
    insert=start;
 else if(node>OSMResults->results[bin][end]->node)   /* Check key is not after end */
    insert=end+1;
 else
   {
    do
      {
       mid=(start+end)/2;                                /* Choose mid point */

       if(OSMResults->results[bin][mid]->node<node)      /* Mid point is too low */
          start=mid;
       else if(OSMResults->results[bin][mid]->node>node) /* Mid point is too high */
          end=mid;
       else
          assert(0);
      }
    while((end-start)>1);

    insert=end;
   }

 /* Shuffle the array up */

 if(insert!=OSMResults->number[bin])
    memmove(&OSMResults->results[bin][insert+1],&OSMResults->results[bin][insert],(OSMResults->number[bin]-insert)*sizeof(Result*));

 /* Insert the new entry */

 OSMResults->number[bin]++;

 OSMResults->results[bin][insert]=(Result*)malloc(sizeof(Result));

 return(OSMResults->results[bin][insert]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result, ordered by node.

  node_t node The node that is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

static Result *find_result(node_t node)
{
 int start;
 int end;
 int mid;
 int bin=node%NBINS;

 /* Binary search - search key exact match only is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 start=0;
 end=OSMResults->number[bin]-1;

 if(OSMResults->number[bin]==0)                      /* There are no results */
    return(NULL);
 else if(node<OSMResults->results[bin][start]->node) /* Check key is not before start */
    return(NULL);
 else if(node>OSMResults->results[bin][end]->node)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                                /* Choose mid point */

       if(OSMResults->results[bin][mid]->node<node)      /* Mid point is too low */
          start=mid+1;
       else if(OSMResults->results[bin][mid]->node>node) /* Mid point is too high */
          end=mid-1;
       else                                              /* Mid point is correct */
          return(OSMResults->results[bin][mid]);
      }
    while((end-start)>1);

    if(OSMResults->results[bin][start]->node==node)      /* Start is correct */
       return(OSMResults->results[bin][start]);

    if(OSMResults->results[bin][end]->node==node)        /* End is correct */
       return(OSMResults->results[bin][end]);
   }

 return(NULL);
}
