/***************************************
 $Header: /home/amb/CVS/routino/src/superx.c,v 1.16 2009-07-01 18:23:26 amb Exp $

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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "results.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "superx.h"


/*++++++++++++++++++++++++++++++++++++++
  Select the super-segments from the list of segments.

  NodesX *nodesx The nodes.

  SegmentsX *segmentsx The segments.

  WaysX *waysx The ways.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

void ChooseSuperNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,int iteration)
{
 int i;
 int    segcount=0,difference=0,nnodes=0;
 node_t node=0;
 WayX  *wayx1;

 /* Find super-nodes */

 node=segmentsx->ndata[0]->node1;
 wayx1=FindWayX(waysx,segmentsx->ndata[0]->way);

 for(i=0;i<segmentsx->number;i++)
   {
    WayX *wayx2=FindWayX(waysx,segmentsx->ndata[i]->way);

    if(segmentsx->ndata[i]->node1!=node)
      {
       /* Store the node if there is a difference in the ways that could affect routing.
          Store the node if it is not a dead-end and if it isn't just the middle of a way. */

       if(difference || segcount>2)
         {
          NodeX **nodex=FindNodeX(nodesx,node);

          (*nodex)->super++;

          nnodes++;
         }

       segcount=1;
       difference=0;

       node=segmentsx->ndata[i]->node1;
       wayx1=wayx2;
      }
    else                        /* Same starting node */
      {
       if(WaysCompare(wayx2->way,wayx1->way))
          difference=1;

       segcount+=1;
      }

    if(!((i+1)%10000))
      {
       printf("\rFinding Super-Nodes: Segments=%d Super-Nodes=%d",i+1,nnodes);
       fflush(stdout);
      }
   }

 printf("\rFound Super-Nodes: Segments=%d Super-Nodes=%d  \n",segmentsx->number,nnodes);
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
 int i;
 SegmentsX *supersegmentsx;

 supersegmentsx=NewSegmentList();

 /* Create super-segments for each super-node. */

 for(i=0;i<nodesx->number;i++)
   {
    if(nodesx->idata[i]->super>iteration)
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
                NodeX **nodex=FindNodeX(nodesx,result->node);
                WayX   *wayx =FindWayX (waysx ,(*segmentx)->way);

                if(result->node!=nodesx->idata[i]->id && (*nodex)->super>iteration)
                  {
                   if(wayx->way->type&Way_OneWay)
                     {
                      AppendSegment(supersegmentsx,wayx->id,nodesx->idata[i]->id,result->node,DISTANCE((distance_t)result->score)|ONEWAY_1TO2);

                      AppendSegment(supersegmentsx,wayx->id,result->node,nodesx->idata[i]->id,DISTANCE((distance_t)result->score)|ONEWAY_2TO1);
                     }
                   else
                      AppendSegment(supersegmentsx,wayx->id,result->node,nodesx->idata[i]->id,DISTANCE((distance_t)result->score));
                  }

                result=NextResult(results,result);
               }

             FreeResultsList(results);
            }

          segmentx=FindNextSegmentX(segmentsx,segmentx);
         }
      }

    if(!((i+1)%10000))
      {
       printf("\rCreating Super-Segments: Nodes=%d Super-Segments=%d",i+1,supersegmentsx->xnumber);
       fflush(stdout);
      }
   }

 printf("\rCreated Super-Segments: Nodes=%d Super-Segments=%d \n",nodesx->number,supersegmentsx->xnumber);
 fflush(stdout);

 /* Append the new supersegments onto the segments. */

 return(supersegmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Merge the super-segments into the segments.

  SegmentsX* segmentsx The set of segments to process.

  SegmentsX* supersegmentsx The set of super-segments to merge.
  ++++++++++++++++++++++++++++++++++++++*/

void MergeSuperSegments(SegmentsX* segmentsx,SegmentsX* supersegmentsx)
{
 int i,j,n;

 assert(segmentsx->sorted);      /* Must be sorted */
 assert(supersegmentsx->sorted); /* Must be sorted */

 n=segmentsx->number;

 for(i=0,j=0;i<n;i++)
   {
    while(j<supersegmentsx->number)
      {
       if(segmentsx->ndata[i]->node1==supersegmentsx->ndata[j]->node1 &&
          segmentsx->ndata[i]->node2==supersegmentsx->ndata[j]->node2 &&
          segmentsx->ndata[i]->distance==supersegmentsx->ndata[j]->distance)
         {
          segmentsx->ndata[i]->distance|=SEGMENT_SUPER; /* mark as super-segment */
          supersegmentsx->ndata[j]=NULL;
          j++;
          break;
         }
       else if(segmentsx->ndata[i]->node1==supersegmentsx->ndata[j]->node1 &&
               segmentsx->ndata[i]->node2==supersegmentsx->ndata[j]->node2)
         {
          supersegmentsx->ndata[j]->distance|=SEGMENT_SUPER; /* mark as super-segment */
         }
       else if(segmentsx->ndata[i]->node1==supersegmentsx->ndata[j]->node1 &&
               segmentsx->ndata[i]->node2>supersegmentsx->ndata[j]->node2)
         {
          supersegmentsx->ndata[j]->distance|=SEGMENT_SUPER; /* mark as super-segment */
         }
       else if(segmentsx->ndata[i]->node1>supersegmentsx->ndata[j]->node1)
         {
          supersegmentsx->ndata[j]->distance|=SEGMENT_SUPER; /* mark as super-segment */
         }
       else
          break;

       j++;
      }

    segmentsx->ndata[i]->distance|=SEGMENT_NORMAL; /* mark as normal segment */

    if(!((i+1)%10000))
      {
       printf("\rMerging Segments: Segments=%d Super-Segment=%d Total=%d",i+1,j+1,segmentsx->xnumber);
       fflush(stdout);
      }
   }

 for(j=0;j<supersegmentsx->number;j++)
    if(supersegmentsx->ndata[j])
       AppendSegment(segmentsx,supersegmentsx->ndata[j]->way,supersegmentsx->ndata[j]->node1,supersegmentsx->ndata[j]->node2,supersegmentsx->ndata[j]->distance);

 printf("\rMerged Segments: Segments=%d Super-Segment=%d Total=%d \n",n,supersegmentsx->number,segmentsx->xnumber);
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

Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,Way *match,int iteration)
{
 Results *results;
 index_t node1,node2;
 Result *result1,*result2;
 NodeX **nodex;
 SegmentX **segmentx;
 WayX *wayx;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 result1=InsertResult(results,start);

 ZeroResult(result1);

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    node1=result1->node;

    segmentx=FindFirstSegmentX(segmentsx,node1);

    while(segmentx)
      {
       distance_t cumulative_distance;

       if((*segmentx)->distance&ONEWAY_2TO1)
          goto endloop;

       node2=(*segmentx)->node2;

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

          nodex=FindNodeX(nodesx,node2);

          if((*nodex)->super<=iteration)
             insert_in_queue(result2);
         }
       else if(cumulative_distance<result2->score)
         {
          result2->prev=node1;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          nodex=FindNodeX(nodesx,node2);

          if((*nodex)->super<=iteration)
             insert_in_queue(result2);
         }

      endloop:

       segmentx=FindNextSegmentX(segmentsx,segmentx);
      }
   }

 return(results);
}
