/***************************************
 Super-Segment data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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
#include <stdio.h>

#include "ways.h"

#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "superx.h"

#include "files.h"
#include "logging.h"
#include "results.h"


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

 if(nodesx->number==0 || segmentsx->number==0 || waysx->number==0)
    return;

 /* Print the start message */

 printf_first("Finding Super-Nodes: Nodes=0 Super-Nodes=0");

 /* Map into memory / open the files */

#if !SLIM
 nodesx->xdata=MapFile(nodesx->filename);
 segmentsx->xdata=MapFile(segmentsx->filename);
 waysx->xdata=MapFile(waysx->filename);
#else
 nodesx->fd=ReOpenFile(nodesx->filename);
 segmentsx->fd=ReOpenFile(segmentsx->filename);
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Find super-nodes */

 for(i=0;i<nodesx->number;i++)
   {
    NodeX *nodex=LookupNodeX(nodesx,i,1);
    int difference=0,nsegments=0;
    SegmentX *segmentx=NULL;
    int waycount=0;
    Way prevway[16];

    segmentx=FirstSegmentX2(segmentsx,i,1);

    while(segmentx)
      {
       WayX *wayx=LookupWayX(waysx,segmentx->way,1);

       nsegments++;
       if(segmentx->node1==segmentx->node2)
          nsegments+=1;         /* The segment is stored once but goes both ways */

       /* If the node allows less traffic types than any connecting way ... */

       if((wayx->way.allow&nodex->allow)!=wayx->way.allow)
         {
          difference=1;
          break;
         }

       /* If the ways are different in any attribute and there is a type of traffic that can use both ... */

       if(waycount>0)
         {
          int j;

          for(j=0;j<waycount;j++)
             if(WaysCompare(&prevway[j],&wayx->way))
                if(wayx->way.allow & prevway[j].allow)
                  {
                   difference=1;
                   break;
                  }
         }

       assert(waycount<(sizeof(prevway)/sizeof(prevway[0])));

       prevway[waycount++]=wayx->way;

       if(difference)
          break;

       segmentx=NextSegmentX2(segmentsx,segmentx,i,1);
      }

    /* Store the node if there is a difference in the connected ways that could affect routing. */

    if(difference || nsegments>2 || nodex->flags&(NODE_TURNRSTRCT|NODE_TURNRSTRCT2))
      {
       nodesx->super[i]++;

       nnodes++;
      }

    if(!((i+1)%10000))
       printf_middle("Finding Super-Nodes: Nodes=%d Super-Nodes=%d",i+1,nnodes);
   }

 /* Unmap from memory / close the files */

#if !SLIM
 nodesx->xdata=UnmapFile(nodesx->filename);
 segmentsx->xdata=UnmapFile(segmentsx->filename);
 waysx->xdata=UnmapFile(waysx->filename);
#else
 nodesx->fd=CloseFile(nodesx->fd);
 segmentsx->fd=CloseFile(segmentsx->fd);
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Print the final message */

 printf_last("Found Super-Nodes: Nodes=%d Super-Nodes=%d",nodesx->number,nnodes);
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

 supersegmentsx=NewSegmentList(0);

 if(segmentsx->number==0 || waysx->number==0)
    return(supersegmentsx);

 /* Print the start message */

 printf_first("Creating Super-Segments: Super-Nodes=0 Super-Segments=0");

 /* Map into memory / open the files */

#if !SLIM
 segmentsx->xdata=MapFile(segmentsx->filename);
 waysx->xdata=MapFile(waysx->filename);
#else
 segmentsx->fd=ReOpenFile(segmentsx->filename);
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Create super-segments for each super-node. */

 for(i=0;i<nodesx->number;i++)
   {
    if(nodesx->super[i]>iteration)
      {
       SegmentX *segmentx;
       int waycount=0,match;
       Way prevway[16];

       segmentx=FirstSegmentX2(segmentsx,i,1);

       while(segmentx)
         {
          WayX *wayx=LookupWayX(waysx,segmentx->way,1);

          /* Check that this type of way hasn't already been routed */

          match=0;

          if(waycount>0)
            {
             int j;

             for(j=0;j<waycount;j++)
                if(!WaysCompare(&prevway[j],&wayx->way))
                  {
                   match=1;
                   break;
                  }
            }

          assert(waycount<(sizeof(prevway)/sizeof(prevway[0])));

          prevway[waycount++]=wayx->way;

          /* Route the way and store the super-segments. */

          if(!match)
            {
             Results *results=FindRoutesWay(nodesx,segmentsx,waysx,i,&wayx->way,iteration);
             Result *result=FirstResult(results);

             while(result)
               {
                if(nodesx->super[result->node]>iteration && result->segment!=NO_SEGMENT)
                  {
                   if(wayx->way.type&Way_OneWay && result->node!=i)
                      AppendSegment(supersegmentsx,segmentx->way,i,result->node,DISTANCE((distance_t)result->score)|ONEWAY_1TO2);
                   else
                      AppendSegment(supersegmentsx,segmentx->way,i,result->node,DISTANCE((distance_t)result->score));

                   ss++;
                  }

                result=NextResult(results,result);
               }

             FreeResultsList(results);
            }

          segmentx=NextSegmentX2(segmentsx,segmentx,i,1);
         }

       sn++;

       if(!(sn%10000))
          printf_middle("Creating Super-Segments: Super-Nodes=%d Super-Segments=%d",sn,ss);
      }
   }

 /* Unmap from memory / close the files */

#if !SLIM
 segmentsx->xdata=UnmapFile(segmentsx->filename);
 waysx->xdata=UnmapFile(waysx->filename);
#else
 segmentsx->fd=CloseFile(segmentsx->fd);
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Print the final message */

 printf_last("Created Super-Segments: Super-Nodes=%d Super-Segments=%d",sn,ss);

 return(supersegmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Merge the segments and super-segments into a new segment list.

  SegmentsX* MergeSuperSegments Returns a new set of merged segments.

  SegmentsX* segmentsx The set of segments to process.

  SegmentsX* supersegmentsx The set of super-segments to merge.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *MergeSuperSegments(SegmentsX* segmentsx,SegmentsX* supersegmentsx)
{
 index_t i,j;
 int m=0,a=0;
 SegmentsX* mergedsegmentsx;

 mergedsegmentsx=NewSegmentList(0);

 if(segmentsx->number==0)
    return(mergedsegmentsx);

 /* Print the start message */

 printf_first("Merging Segments: Segments=0 Super-Segments=0 Merged=0 Added=0");

 /* Map into memory / open the files */

#if !SLIM
 segmentsx->xdata=MapFile(segmentsx->filename);
 if(supersegmentsx->number>0)
    supersegmentsx->xdata=MapFile(supersegmentsx->filename);
#else
 segmentsx->fd=ReOpenFile(segmentsx->filename);
 if(supersegmentsx->number>0)
    supersegmentsx->fd=ReOpenFile(supersegmentsx->filename);
#endif

 /* Loop through and create a new list of combined segments */

 for(i=0,j=0;i<segmentsx->number;i++)
   {
    int super=0;
    SegmentX *segmentx=LookupSegmentX(segmentsx,i,1);

    while(j<supersegmentsx->number)
      {
       SegmentX *supersegmentx=LookupSegmentX(supersegmentsx,j,1);

       if(segmentx->node1   ==supersegmentx->node1 &&
          segmentx->node2   ==supersegmentx->node2 &&
          segmentx->distance==supersegmentx->distance)
         {
          m++;
          j++;
          /* mark as super-segment and normal segment */
          super=1;
          break;
         }
       else if((segmentx->node1==supersegmentx->node1 &&
                segmentx->node2==supersegmentx->node2) ||
               (segmentx->node1==supersegmentx->node1 &&
                segmentx->node2>supersegmentx->node2) ||
               (segmentx->node1>supersegmentx->node1))
         {
          /* mark as super-segment */
          AppendSegment(mergedsegmentsx,supersegmentx->way,supersegmentx->node1,supersegmentx->node2,supersegmentx->distance|SEGMENT_SUPER);
          a++;
          j++;
         }
       else
         {
          /* mark as normal segment */
          break;
         }
      }

    if(super)
       AppendSegment(mergedsegmentsx,segmentx->way,segmentx->node1,segmentx->node2,segmentx->distance|SEGMENT_SUPER|SEGMENT_NORMAL);
    else
       AppendSegment(mergedsegmentsx,segmentx->way,segmentx->node1,segmentx->node2,segmentx->distance|SEGMENT_NORMAL);

    if(!((i+1)%10000))
       printf_middle("Merging Segments: Segments=%d Super-Segments=%d Merged=%d Added=%d",i+1,j,m,a);
   }

 /* Unmap from memory / close the files */

#if !SLIM
 segmentsx->xdata=UnmapFile(segmentsx->filename);
 if(supersegmentsx->number>0)
    supersegmentsx->xdata=UnmapFile(supersegmentsx->filename);
#else
 segmentsx->fd=CloseFile(segmentsx->fd);
 if(supersegmentsx->number>0)
    supersegmentsx->fd=CloseFile(supersegmentsx->fd);
#endif

 /* Print the final message */

 printf_last("Merged Segments: Segments=%d Super-Segments=%d Merged=%d Added=%d",segmentsx->number,supersegmentsx->number,m,a);

 return(mergedsegmentsx);
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
 Result *result1,*result2;
 WayX *wayx;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 queue=NewQueueList();

 result1=InsertResult(results,start,NO_SEGMENT);

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    index_t node1,seg1;
    SegmentX *segmentx;

    node1=result1->node;
    seg1=result1->segment;

    segmentx=FirstSegmentX2(segmentsx,node1,2); /* position 1 is already used */

    while(segmentx)
      {
       index_t node2,seg2;
       distance_t cumulative_distance;

       if(segmentx->node1==node1)
         {
          if(segmentx->distance&ONEWAY_2TO1)
             goto endloop;

          node2=segmentx->node2;
         }
       else /* if(segmentx->node2==node1) */
         {
          if(segmentx->distance&ONEWAY_1TO2)
             goto endloop;

          node2=segmentx->node1;
         }

       seg2=IndexSegmentX(segmentsx,segmentx);

       if(result1->prev && result1->prev->node==node2 && node1!=node2)
          goto endloop;

       wayx=LookupWayX(waysx,segmentx->way,2);

       if(WaysCompare(&wayx->way,match))
          goto endloop;

       cumulative_distance=(distance_t)result1->score+DISTANCE(segmentx->distance);

       result2=FindResult(results,node2,seg2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          if(nodesx->super[node2]<=iteration)
             InsertInQueue(queue,result2);
         }
       else if(cumulative_distance<result2->score)
         {
          result2->prev=result1;
          result2->score=cumulative_distance;
          result2->sortby=cumulative_distance;

          if(nodesx->super[node2]<=iteration)
             InsertInQueue(queue,result2);
         }

      endloop:

       segmentx=NextSegmentX2(segmentsx,segmentx,node1,2);
      }
   }

 FreeQueueList(queue);

 return(results);
}
