/***************************************
 $Header: /home/amb/CVS/routino/src/optimiser.c,v 1.4 2009-01-03 12:25:23 amb Exp $

 Routing optimiser.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <string.h>
#include <stdlib.h>

#include "functions.h"
#include "types.h"

#define INCREMENT 10*1024
#define NBINS     64

/*+ One item in the results. +*/
typedef struct _Result
{
 node_t     node;               /*+ The end node. +*/
 Node       *Node;              /*+ The end Node. +*/
 Node       *shortest_Prev;     /*+ The previous Node following the shortest path. +*/
 distance_t shortest_distance;  /*+ The distance travelled to the node following the shortest path. +*/
 duration_t shortest_duration;  /*+ The time taken to the node following the shortest path. +*/
 Node       *quickest_Prev;     /*+ The previous Node following the quickest path. +*/
 distance_t quickest_distance;  /*+ The distance travelled to the node following the quickest path. +*/
 duration_t quickest_duration;  /*+ The time taken to the node following the quickest path. +*/
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
static Result *find_insert_result(node_t node);
static Result *find_result(node_t node);


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes.

  node_t start The start node.

  node_t finish The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

void FindRoute(node_t start,node_t finish)
{
 Node *Start,*Finish;
 node_t node1,node2;
 Node *Node1,*Node2;
 distance_t shortest_distance1,shortest_distance2=0;
 duration_t shortest_duration1,shortest_duration2=0;
 distance_t quickest_distance1,quickest_distance2=0;
 duration_t quickest_duration1,quickest_duration2=0;
 distance_short_t deltadistance;
 duration_short_t deltaduration;
 distance_t totalcrow,crow;
 distance_t shortest_distancefinish=~0,quickest_distancefinish=~0;
 duration_t shortest_durationfinish=~0,quickest_durationfinish=~0;
 Result *result1,*result2;
 Segment *segment;
 int nresults=0;

 /* Work out the distance as the crow flies */

 Start=FindNode(start);
 Finish=FindNode(finish);

 totalcrow=SegmentLength(Start,Finish);

 /* Insert the first node into the queue */

 result1=find_insert_result(start);

 result1->node=start;
 result1->Node=Start;
 result1->shortest_Prev=NULL;
 result1->shortest_distance=0;
 result1->shortest_duration=0;
 result1->quickest_Prev=NULL;
 result1->quickest_distance=0;
 result1->quickest_duration=0;

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while(OSMQueue->number>0)
   {
    result1=OSMQueue->queue[--OSMQueue->number];
    node1=result1->node;
    Node1=result1->Node;
    shortest_distance1=result1->shortest_distance;
    shortest_duration1=result1->shortest_duration;
    quickest_distance1=result1->quickest_distance;
    quickest_duration1=result1->quickest_duration;

    segment=FindFirstSegment(node1);

    while(segment)
      {
       node2=segment->node2;

       deltadistance=segment->distance;
       deltaduration=segment->duration;

       shortest_distance2=shortest_distance1+deltadistance;
       shortest_duration2=shortest_duration1+deltaduration;
       quickest_distance2=quickest_distance1+deltadistance;
       quickest_duration2=quickest_duration1+deltaduration;

       result2=find_result(node2);
       if(result2)
          Node2=result2->Node;
       else
          Node2=FindNode(node2);

       crow=SegmentLength(Node2,Finish);

       if((crow+shortest_distance2)>(km_to_distance(10)+1.4*totalcrow))
          goto endloop;

       if(shortest_distance2>shortest_distancefinish && quickest_duration2>quickest_durationfinish)
          goto endloop;

       if(!result2)                         /* New end node */
         {
          result2=find_insert_result(node2);
          result2->node=node2;
          result2->Node=Node2;
          result2->shortest_Prev=Node1;
          result2->shortest_distance=shortest_distance2;
          result2->shortest_duration=shortest_duration2;
          result2->quickest_Prev=Node1;
          result2->quickest_distance=quickest_distance2;
          result2->quickest_duration=quickest_duration2;

          nresults++;

          if(node2==finish)
            {
             shortest_distancefinish=shortest_distance2;
             shortest_durationfinish=shortest_duration2;
             quickest_distancefinish=quickest_distance2;
             quickest_durationfinish=quickest_duration2;
            }
          else
             insert_in_queue(result2);
         }
       else
         {
          if(shortest_distance2<result2->shortest_distance ||
             (shortest_distance2==result2->shortest_distance &&
              shortest_duration2<result2->shortest_duration)) /* New end node is shorter */
            {
             result2->shortest_Prev=Node1;
             result2->shortest_distance=shortest_distance2;
             result2->shortest_duration=shortest_duration2;

             if(node2==finish)
               {
                shortest_distancefinish=shortest_distance2;
                shortest_durationfinish=shortest_duration2;
               }
             else
                insert_in_queue(result2);
            }

          if(quickest_duration2<result2->quickest_duration ||
             (quickest_duration2==result2->quickest_duration &&
              quickest_distance2<result2->quickest_distance)) /* New end node is quicker */
            {
             result2->quickest_Prev=Node1;
             result2->quickest_distance=quickest_distance2;
             result2->quickest_duration=quickest_duration2;

             if(node2==finish)
               {
                quickest_distancefinish=quickest_distance2;
                quickest_durationfinish=quickest_duration2;
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
              distance_to_km(shortest_distance2),duration_to_minutes(quickest_duration2));
       fflush(stdout);
      }
   }

 printf("\rRouted: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin\n",nresults,
        distance_to_km(shortest_distancefinish),duration_to_minutes(shortest_durationfinish),
        distance_to_km(quickest_distancefinish),duration_to_minutes(quickest_durationfinish));
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
    if(result->shortest_Prev)
      {
       Segment *segment;
       Way *way;

       segment=FindFirstSegment(result->shortest_Prev->id);
       while(segment->node2!=result->Node->id)
          segment=FindNextSegment(segment);

       way=FindWay(segment->way);

       fprintf(file,"%9.5f %9.5f %9d %5.3f %5.2f %3.0f %s\n",result->Node->latitude,result->Node->longitude,result->node,
               distance_to_km(segment->distance),duration_to_minutes(segment->duration),
               distance_to_km(segment->distance)/duration_to_hours(segment->duration),
               WayName(way));

       result=find_result(result->shortest_Prev->id);
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
    if(result->quickest_Prev)
      {
       Segment *segment;
       Way *way;

       segment=FindFirstSegment(result->quickest_Prev->id);
       while(segment->node2!=result->Node->id)
          segment=FindNextSegment(segment);

       way=FindWay(segment->way);

       fprintf(file,"%9.5f %9.5f %9d %5.3f %5.2f %3.0f %s\n",result->Node->latitude,result->Node->longitude,result->node,
               distance_to_km(segment->distance),duration_to_minutes(segment->duration),
               distance_to_km(segment->distance)/duration_to_hours(segment->duration),
               WayName(way));

       result=find_result(result->quickest_Prev->id);
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
//               distance_to_km(result->shortest_distance));
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
//               duration_to_minutes(result->quickest_duration));
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
    OSMQueue=(Queue*)calloc(1,sizeof(Queue));
    OSMQueue->alloced=sizeof(OSMQueue->queue)/sizeof(OSMQueue->queue[0]);
   }

 /* Check that the arrays have enough space. */

 if(OSMQueue->number==OSMQueue->alloced)
   {
    OSMQueue=(Queue*)realloc((void*)OSMQueue,sizeof(Queue)-sizeof(OSMQueue->queue)+(OSMQueue->alloced+INCREMENT)*sizeof(Result*));

    OSMQueue->alloced+=INCREMENT;
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
 else if(result->shortest_distance>OSMQueue->queue[start]->shortest_distance) /* Check key is not before start */
    insert=start;
 else if(result->shortest_distance<OSMQueue->queue[end]->shortest_distance)   /* Check key is not after end */
    insert=end+1;
 else
   {
    do
      {
       mid=(start+end)/2;                                                         /* Choose mid point */

       if(OSMQueue->queue[mid]->shortest_distance>result->shortest_distance)      /* Mid point is too low */
          start=mid;
       else if(OSMQueue->queue[mid]->shortest_distance<result->shortest_distance) /* Mid point is too high */
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
  Find an existing result or insert a new one into the results in the right order.

  node_t node The node that is to be inserted into the results.
  ++++++++++++++++++++++++++++++++++++++*/

static Result *find_insert_result(node_t node)
{
 int start;
 int end;
 int mid;
 int insert=-1,found=-1;
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
       else                                              /* Mid point is correct */
         {
          found=mid;
          break;
         }
      }
    while((end-start)>1);

    if(found==-1)
      {
       if(OSMResults->results[bin][start]->node==node)   /* Start is correct */
          found=start;
       else if(OSMResults->results[bin][end]->node==node)/* End is correct */
          found=end;
       else
          insert=end;
      }
   }

 /* Shuffle the array up */

 if(insert!=-1 && insert!=OSMResults->number[bin])
    memmove(&OSMResults->results[bin][insert+1],&OSMResults->results[bin][insert],(OSMResults->number[bin]-insert)*sizeof(Result*));

 if(insert!=-1)
    found=insert;

 /* Insert the insert entry */

 if(insert!=-1)
   {
    OSMResults->number[bin]++;

    OSMResults->results[bin][found]=(Result*)malloc(sizeof(Result));
   }

 return(OSMResults->results[bin][found]);
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
