/***************************************
 $Header: /home/amb/CVS/routino/src/superx.c,v 1.28 2009-09-03 17:51:03 amb Exp $

 Super-Segment data type functions.

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


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "results.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "superx.h"
#include "ways.h"


/* Local Functions */

static Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,Way *match,int iteration);


/*++++++++++++++++++++++++++++++++++++++
  Select the super-segments from the list of segments.

  NodesX *nodesx The nodes.

  SegmentsX *segmentsx The segments.

  WaysX *waysx The ways.
  ++++++++++++++++++++++++++++++++++++++*/

void ChooseSuperNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx)
{
 index_t i;
 int nnodes=0;

 assert(nodesx->idata);         /* Must have idata filled in => sorted by id */

 printf("Finding Super-Nodes: Nodes=0 Super-Nodes=0");
 fflush(stdout);

 /* Find super-nodes */

 for(i=0;i<nodesx->number;i++)
   {
    int      difference=0;
    SegmentX **segmentx;

    segmentx=FindFirstSegmentX(segmentsx,nodesx->idata[i]->id);

    if(segmentx)
      {
       WayX *wayx1=FindWayX(waysx,(*segmentx)->way);

       segmentx=FindNextSegmentX(segmentsx,segmentx);

       if(segmentx)
         {
          WayX *wayx2=FindWayX(waysx,(*segmentx)->way);

          if(WaysCompare(wayx2->way,wayx1->way))
             difference=1;

          segmentx=FindNextSegmentX(segmentsx,segmentx);
         }

       /* Store the node if there is a difference in the first two ways that could affect routing.
          Store the node if it has at least three segments. */

       if(difference || segmentx)
         {
          nodesx->super[i]++;

          nnodes++;
         }
      }

    if(!((i+1)%10000))
      {
       printf("\rFinding Super-Nodes: Nodes=%d Super-Nodes=%d",i+1,nnodes);
       fflush(stdout);
      }
   }

 printf("\rFound Super-Nodes: Nodes=%d Super-Nodes=%d  \n",nodesx->number,nnodes);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the super-segments.

  SegmentsX *CreateSuperSegments Creates the super segments.

  NodesX *nodesx The nodes.

  SegmentsX *segmentsx The segments.

  WaysX *waysx The ways.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *CreateSuperSegments(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,int iteration)
{
 index_t i;
 SegmentsX *supersegmentsx;
 int sn=0,ss=0;

 assert(nodesx->idata);         /* Must have idata filled in => sorted by id */

 printf("Creating Super-Segments: Super-Nodes=0 Super-Segments=0");
 fflush(stdout);

 supersegmentsx=NewSegmentList();

 /* Create super-segments for each super-node. */

 for(i=0;i<nodesx->number;i++)
   {
    if(nodesx->super[i]>iteration)
      {
       SegmentX **segmentx,**first;

       segmentx=first=FindFirstSegmentX(segmentsx,nodesx->idata[i]->id);

       while(segmentx)
         {
          WayX *wayx=FindWayX(waysx,(*segmentx)->way);

          /* Check that this type of way hasn't already been routed */

          if(segmentx!=first)
            {
             SegmentX **othersegmentx=first;

             while(othersegmentx && othersegmentx!=segmentx)
               {
                WayX *otherwayx=FindWayX(waysx,(*othersegmentx)->way);

                if(!WaysCompare(otherwayx->way,wayx->way))
                  {
                   wayx=NULL;
                   break;
                  }

                othersegmentx=FindNextSegmentX(segmentsx,othersegmentx);
               }
            }

          /* Route the way and store the super-segments. */

          if(wayx)
            {
             Results *results=FindRoutesWay(nodesx,segmentsx,waysx,nodesx->idata[i]->id,wayx->way,iteration);
             Result *result=FirstResult(results);

             while(result)
               {
                index_t index=IndexNodeX(nodesx,result->node);
                WayX   *wayx =FindWayX (waysx ,(*segmentx)->way);

                if(result->node!=nodesx->idata[i]->id && nodesx->super[index]>iteration)
                  {
                   if(wayx->way->type&Way_OneWay)
                      AppendSegment(supersegmentsx,wayx->id,nodesx->idata[i]->id,result->node,DISTANCE((distance_t)result->score)|ONEWAY_1TO2);
                   else
                      AppendSegment(supersegmentsx,wayx->id,nodesx->idata[i]->id,result->node,DISTANCE((distance_t)result->score));

                   ss++;
                  }

                result=NextResult(results,result);
               }

             FreeResultsList(results);
            }

          segmentx=FindNextSegmentX(segmentsx,segmentx);
         }

       sn++;

       if(!(sn%10000))
         {
          printf("\rCreating Super-Segments: Super-Nodes=%d Super-Segments=%d",sn,ss);
          fflush(stdout);
         }
      }
   }

 printf("\rCreated Super-Segments: Super-Nodes=%d Super-Segments=%d \n",sn,ss);
 fflush(stdout);

 return(supersegmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Merge the super-segments into the segments.

  SegmentsX* segmentsx The set of segments to process.

  SegmentsX* supersegmentsx The set of super-segments to merge.
  ++++++++++++++++++++++++++++++++++++++*/

void MergeSuperSegments(SegmentsX* segmentsx,SegmentsX* supersegmentsx)
{
 index_t i,j;
 int m=0,a=0;

 assert(segmentsx->n1data);      /* Must have n1data filled in => sorted by node 1 */
 assert(supersegmentsx->n1data); /* Must have n1data filled in => sorted by node 1 */

 printf("Merging: Segments=0 Super-Segments=0 Merged=0 Added=0");
 fflush(stdout);

 for(i=0,j=0;i<segmentsx->number;i++)
   {
    while(j<supersegmentsx->number)
      {
       if(segmentsx->n1data[i]->node1==supersegmentsx->n1data[j]->node1 &&
          segmentsx->n1data[i]->node2==supersegmentsx->n1data[j]->node2 &&
          segmentsx->n1data[i]->distance==supersegmentsx->n1data[j]->distance)
         {
          segmentsx->n1data[i]->distance|=SEGMENT_SUPER; /* mark as super-segment */
          m++;
          j++;
          break;
         }
       else if((segmentsx->n1data[i]->node1==supersegmentsx->n1data[j]->node1 &&
                segmentsx->n1data[i]->node2==supersegmentsx->n1data[j]->node2) ||
               (segmentsx->n1data[i]->node1==supersegmentsx->n1data[j]->node1 &&
                segmentsx->n1data[i]->node2>supersegmentsx->n1data[j]->node2) ||
               (segmentsx->n1data[i]->node1>supersegmentsx->n1data[j]->node1))
         {
          supersegmentsx->n1data[j]->distance|=SEGMENT_SUPER; /* mark as super-segment */
          AppendSegment(segmentsx,supersegmentsx->n1data[j]->way,supersegmentsx->n1data[j]->node1,supersegmentsx->n1data[j]->node2,supersegmentsx->n1data[j]->distance);
          a++;
          j++;
         }
       else
          break;
      }

    segmentsx->n1data[i]->distance|=SEGMENT_NORMAL; /* mark as normal segment (as well) */

    if(!((i+1)%10000))
      {
       printf("\rMerging: Segments=%d Super-Segments=%d Merged=%d Added=%d",i+1,j,m,a);
       fflush(stdout);
      }
   }

 printf("\rMerged: Segments=%d Super-Segments=%d Merged=%d Added=%d \n",segmentsx->number,supersegmentsx->number,m,a);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any node in the specified list that follows a certain type of way.

  Results *FindRoutesWay Returns a set of results.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.

  node_t start The start node.

  Way *match The way that the route must match.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,Way *match,int iteration)
{
 Results *results;
 Queue *queue;
 index_t node1,node2;
 Result *result1,*result2;
 SegmentX **segmentx;
 WayX *wayx;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 queue=NewQueueList();

 result1=InsertResult(results,start);

 ZeroResult(result1);

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    node1=result1->node;

    segmentx=FindFirstSegmentX(segmentsx,node1);

    while(segmentx)
      {
       distance_t cumulative_distance;

       if((*segmentx)->node1==node1) /* Correct way round */
         {
          if((*segmentx)->distance&ONEWAY_2TO1)
             goto endloop;

          node2=(*segmentx)->node2;
         }
       else                          /* Opposite way round */
         {
          if((*segmentx)->distance&ONEWAY_1TO2)
             goto endloop;

          node2=(*segmentx)->node1;
         }

       if(result1->prev==node2)
          goto endloop;

       wayx=FindWayX(waysx,(*segmentx)->way);

       if(WaysCompare(wayx->way,match))
          goto endloop;

       cumulative_distance=(distance_t)result1->score+DISTANCE((*segmentx)->distance);

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev=node1;
          result2->next=NO_NODE;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          if(nodesx->super[IndexNodeX(nodesx,node2)]<=iteration)
             InsertInQueue(queue,result2);
         }
       else if(cumulative_distance<result2->score)
         {
          result2->prev=node1;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          if(nodesx->super[IndexNodeX(nodesx,node2)]<=iteration)
             InsertInQueue(queue,result2);
         }

      endloop:

       segmentx=FindNextSegmentX(segmentsx,segmentx);
      }
   }

 FreeQueueList(queue);

 return(results);
}
