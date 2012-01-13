/***************************************
 Data pruning functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2011-2012 Andrew M. Bishop

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
#include <assert.h>

#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "prunex.h"

#include "files.h"
#include "logging.h"


/* Local functions */

static void prune_node(NodesX *nodesx,NodeX *nodex);
static void prune_segment(NodesX *nodesx,SegmentsX *segmentsx,SegmentX *segmentx);
static void modify_segment(SegmentsX *segmentsx,SegmentX *segmentx,index_t newnode1,index_t newnode2);

static void unlink_segment_node_refs(SegmentsX *segmentsx,SegmentX *segmentx,index_t node);


/*++++++++++++++++++++++++++++++++++++++
  Initialise the data structures needed for pruning.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void StartPruning(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx)
{
 SegmentX segmentx;
 index_t index=0,lastnode1=NO_NODE;

 /* Print the start message */

 printf_first("Adding Extra Segment Indexes: Segments=0");

 /* Allocate the array of next segment */

 segmentsx->next1=(index_t*)calloc(segmentsx->number,sizeof(index_t));

 assert(segmentsx->next1); /* Check malloc() worked */

 /* Open the file read-only */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 /* Read the on-disk image */

 while(!ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX)))
   {
    index_t node1=segmentx.node1;

    if(lastnode1==node1)
       segmentsx->next1[index]=index+1;
    else
       segmentsx->next1[index]=NO_SEGMENT;

    lastnode1=node1;
    index++;

    if(!(index%10000))
       printf_middle("Added Extra Segment Indexes: Segments=%"Pindex_t,index);
   }

 segmentsx->next1[index]=NO_SEGMENT;

 /* Close the file */

 segmentsx->fd=CloseFile(segmentsx->fd);

 /* Print the final message */

 printf_last("Added Extra Segment Indexes: Segments=%"Pindex_t,segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Delete the data structures needed for pruning.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  WaysX *waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void FinishPruning(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx)
{
 free(segmentsx->next1);
 segmentsx->next1=NULL;

 SortSegmentList(segmentsx,1);

 IndexSegments(segmentsx,nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Prune a node - all segment references to this node must already have gone.

  NodesX *nodesx The set of nodes to use.

  NodeX *nodex The node to be pruned.
  ++++++++++++++++++++++++++++++++++++++*/

static void prune_node(NodesX *nodesx,NodeX *nodex)
{
 nodex->flags|=NODE_PRUNED;

 PutBackNodeX(nodesx,nodex);
}


/*++++++++++++++++++++++++++++++++++++++
  Prune a segment.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  SegmentX *segmentx The segment to be pruned.
  ++++++++++++++++++++++++++++++++++++++*/

static void prune_segment(NodesX *nodesx,SegmentsX *segmentsx,SegmentX *segmentx)
{
 unlink_segment_node_refs(segmentsx,segmentx,segmentx->node1);
 unlink_segment_node_refs(segmentsx,segmentx,segmentx->node2);

 segmentx->node1=NO_NODE;
 segmentx->node2=NO_NODE;
 segmentx->next2=NO_SEGMENT;

 PutBackSegmentX(segmentsx,segmentx);
}


/*++++++++++++++++++++++++++++++++++++++
  Modify a segment's nodes.

  NodesX *nodesx The set of nodes to use.

  SegmentX *segmentx The segment to be modified.

  index_t newnode1 The new value of node1.

  index_t newnode2 The new value of node2.
  ++++++++++++++++++++++++++++++++++++++*/

static void modify_segment(SegmentsX *segmentsx,SegmentX *segmentx,index_t newnode1,index_t newnode2)
{
 index_t thissegment=IndexSegmentX(segmentsx,segmentx);

 if(newnode1!=segmentx->node1)
   {
    unlink_segment_node_refs(segmentsx,segmentx,segmentx->node1);

    segmentx->node1=newnode1;

    segmentsx->next1[thissegment]=segmentsx->firstnode[newnode1];
    segmentsx->firstnode[newnode1]=thissegment;
   }

 if(newnode1!=segmentx->node1)
   {
    unlink_segment_node_refs(segmentsx,segmentx,segmentx->node2);

    segmentx->node2=newnode2;

    segmentx->next2=segmentsx->firstnode[newnode2];
    segmentsx->firstnode[newnode2]=thissegment;
   }

 PutBackSegmentX(segmentsx,segmentx);
}


/*++++++++++++++++++++++++++++++++++++++
  Unlink one node from a segment by modifying the linked list type arrangement of node references.

  SegmentsX *segmentsx The set of segments to use.

  SegmentX *segmentx The segment to be pruned.

  index_t node The node index of the end of the segment being modified here.
  ++++++++++++++++++++++++++++++++++++++*/

static void unlink_segment_node_refs(SegmentsX *segmentsx,SegmentX *segmentx,index_t node)
{
 index_t thissegment=IndexSegmentX(segmentsx,segmentx);
 index_t segment=segmentsx->firstnode[node];

 if(segment==thissegment)
   {
    if(segmentx->node1==node)
       segmentsx->firstnode[node]=segmentsx->next1[thissegment];
    else
       segmentsx->firstnode[node]=segmentx->next2;
   }
 else
   {
    index_t nextsegment;

    do
      {
       SegmentX *segx=LookupSegmentX(segmentsx,segment,2);

       if(segx->node1==node)
         {
          nextsegment=segmentsx->next1[segment];

          if(thissegment==nextsegment)
            {
             if(segmentx->node1==node)
                segmentsx->next1[segment]=segmentsx->next1[thissegment];
             else
                segmentsx->next1[segment]=segmentx->next2;
            }
         }
       else
         {
          nextsegment=segx->next2;

          if(thissegment==nextsegment)
            {
             if(segmentx->node1==node)
                segx->next2=segmentsx->next1[thissegment];
             else
                segx->next2=segmentx->next2;

             PutBackSegmentX(segmentsx,segx);
            }
         }

       segment=nextsegment;
      }
    while(segment!=thissegment);
   }
}
