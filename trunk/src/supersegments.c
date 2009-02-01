/***************************************
 $Header: /home/amb/CVS/routino/src/supersegments.c,v 1.24 2009-02-01 17:11:08 amb Exp $

 Super-Segment data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "nodes.h"
#include "ways.h"
#include "segments.h"
#include "functions.h"


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
 int        segcount=0,difference=0,nnodes=0;
 node_t     node=0;
 speed_t    limit=0;
 waytype_t  type=0;
 wayallow_t allow=0;

 /* Find super-nodes */

 node=segmentsx->xdata[0].node1;

 for(i=0;i<segmentsx->number;i++)
   {
    SegmentX *segmentx=LookupSegmentX(segmentsx,i);
    WayX *wayx=LookupWayX(waysx,segmentx->segment.way);

    if(segmentx->node1!=node)
      {
       /* Store the node if there is a difference in the ways that could affect routing.
          Store the node if it is not a dead-end and if it isn't just the middle of a way. */

       if(difference || segcount>2)
         {
          NodeX *nodex=FindNode(nodesx,node);

          nodex->super++;

          nnodes++;
         }

       segcount=1;
       difference=0;

       node=segmentx->node1;
       type=wayx->way.type;
       limit=wayx->way.limit;
       allow=wayx->way.allow;
      }
    else                        /* Same starting node */
      {
       if(wayx->way.type!=type)
          difference=1;

       if(wayx->way.limit!=limit)
          difference=1;

       if(wayx->way.allow!=allow)
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
    if(nodesx->xdata[i].super>iteration)
      {
       SegmentX *segmentx,*first;

       segmentx=first=FindFirstSegment(segmentsx,nodesx->xdata[i].id);

       while(segmentx)
         {
          WayX *wayx=LookupWayX(waysx,segmentx->segment.way);

          /* Check that this type of way hasn't already been routed */

          if(segmentx!=first)
            {
             SegmentX *othersegmentx=first;

             while(othersegmentx && othersegmentx!=segmentx)
               {
                WayX *otherwayx=LookupWayX(waysx,othersegmentx->segment.way);

                if(otherwayx->way.type ==wayx->way.type  &&
                   otherwayx->way.allow==wayx->way.allow &&
                   otherwayx->way.limit==wayx->way.limit)
                  {
                   wayx=NULL;
                   break;
                  }

                othersegmentx=FindNextSegment(segmentsx,othersegmentx);
               }
            }

          /* Route the way and store the super-segments. */

          if(wayx)
            {
             Results *results=FindRoutesWay(nodesx,segmentsx,waysx,nodesx->xdata[i].id,wayx,iteration);
             Result *result=FirstResult(results);

             while(result)
               {
                NodeX *nodex=FindNode(nodesx,result->node);

                if(result->node!=nodesx->xdata[i].id && nodex->super>iteration)
                  {
                   SegmentX *supersegmentx=AppendSegment(supersegmentsx,nodesx->xdata[i].id,result->node,IndexWayX(waysx,wayx));

                   supersegmentx->segment.distance=result->shortest.distance;

                   if(wayx->way.type&Way_OneWay)
                     {
                      supersegmentx=AppendSegment(supersegmentsx,result->node,nodesx->xdata[i].id,IndexWayX(waysx,wayx));

                      supersegmentx->segment.distance=ONEWAY_OPPOSITE|result->shortest.distance;
                     }
                  }

                result=NextResult(results,result);
               }

             FreeResultsList(results);
            }

          segmentx=FindNextSegment(segmentsx,segmentx);
         }
      }

    if(!((i+1)%10000))
      {
       printf("\rCreating Super-Segments: Nodes=%d Super-Segments=%d",i+1,supersegmentsx->number);
       fflush(stdout);
      }
   }

 printf("\rCreated Super-Segments: Nodes=%d Super-Segments=%d \n",nodesx->number,supersegmentsx->number);
 fflush(stdout);

 /* Append the new supersegments onto the segments. */

 return(supersegmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any node in the specified list that follows a certain type of way.

  Results *FindRoutesWay Returns a set of results.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.

  node_t start The start node.

  WayX *match The way that the route must match.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,WayX *match,int iteration)
{
 Results *results;
 index_t node1,node2;
 HalfResult shortest2;
 Result *result1,*result2;
 NodeX *nodex;
 SegmentX *segmentx;
 WayX *wayx;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 result1=InsertResult(results,start);

 result1->node=start;
 result1->shortest.prev=0;
 result1->shortest.next=0;
 result1->shortest.distance=0;

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    node1=result1->node;

    segmentx=FindFirstSegment(segmentsx,node1);

    while(segmentx)
      {
       if(segmentx->segment.distance&ONEWAY_OPPOSITE)
          goto endloop;

       node2=segmentx->node2;

       if(result1->shortest.prev==node2)
          goto endloop;

       wayx=LookupWayX(waysx,segmentx->segment.way);

       if(wayx->way.type !=match->way.type  ||
          wayx->way.allow!=match->way.allow ||
          wayx->way.limit!=match->way.limit)
          goto endloop;

       shortest2.distance=result1->shortest.distance+DISTANCE(segmentx->segment.distance);

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=node1;
          result2->shortest.next=0;
          result2->shortest.distance=shortest2.distance;

          nodex=FindNode(nodesx,node2);

          if(nodex->super<=iteration)
             insert_in_queue(result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance)
            {
             result2->shortest.prev=node1;
             result2->shortest.distance=shortest2.distance;

             nodex=FindNode(nodesx,node2);

             if(nodex->super<=iteration)
                insert_in_queue(result2);
            }
         }

      endloop:

       segmentx=FindNextSegment(segmentsx,segmentx);
      }
   }

 return(results);
}
