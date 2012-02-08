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
#include <assert.h>

#include "typesx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"

#include "prunex.h"

#include "files.h"
#include "logging.h"


/* Local functions */

static void prune_segment(SegmentsX *segmentsx,SegmentX *segmentx);
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

    if(index==0)
       ;
    else if(lastnode1==node1)
       segmentsx->next1[index-1]=index;
    else
       segmentsx->next1[index-1]=NO_SEGMENT;

    lastnode1=node1;
    index++;

    if(!(index%10000))
       printf_middle("Added Extra Segment Indexes: Segments=%"Pindex_t,index);
   }

 segmentsx->next1[index-1]=NO_SEGMENT;

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
 index_t i,pruned=0;
 int fd;

 free(segmentsx->next1);
 segmentsx->next1=NULL;

 SortSegmentList(segmentsx,1);

 IndexSegments(segmentsx,nodesx);

 /* Print the start message */

 printf_first("Marking Pruned Nodes: Nodes=0 Pruned=0");

 /* Re-open the file read-only and a new file writeable */

 nodesx->fd=ReOpenFile(nodesx->filename);

 DeleteFile(nodesx->filename);

 fd=OpenFileNew(nodesx->filename);

 /* Modify the on-disk image */

 for(i=0;i<nodesx->number;i++)
   {
    NodeX nodex;

    ReadFile(nodesx->fd,&nodex,sizeof(NodeX));

    if(segmentsx->firstnode[i]==NO_SEGMENT)
      {
       pruned++;
       nodex.latitude=NO_LATLONG;
       nodex.longitude=NO_LATLONG;
      }

    WriteFile(fd,&nodex,sizeof(NodeX));

    if(!((i+1)%10000))
       printf_middle("Marking Pruned Nodes: Nodes=%"Pindex_t" Pruned=%"Pindex_t,i+1,pruned);
   }

 /* Close the files */

 nodesx->fd=CloseFile(nodesx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Marked Pruned Nodes: Nodes=%"Pindex_t" Pruned=%"Pindex_t,nodesx->number,pruned);
}


/*++++++++++++++++++++++++++++++++++++++
  Prune out any groups of nodes and segments whose total length is less than a
  specified minimum.

  NodesX *nodesx The set of nodes to use.

  SegmentsX *segmentsx The set of segments to use.

  distance_t minimum The minimum distance to keep.
  ++++++++++++++++++++++++++++++++++++++*/

void PruneIsolatedRegions(NodesX *nodesx,SegmentsX *segmentsx,distance_t minimum)
{
 index_t i,j;
 index_t nregions=0,npruned=0;
 BitMask *connected,*region;
 index_t *regionsegments,*othersegments;
 int nallocregionsegments,nallocothersegments;

 if(nodesx->number==0 || segmentsx->number==0)
    return;

 /* Print the start message */

 printf_first("Pruning Isolated Regions: Segments=0 Pruned=0");

 /* Map into memory / open the files */

#if !SLIM
 nodesx->data=MapFileWriteable(nodesx->filename);
 segmentsx->data=MapFileWriteable(segmentsx->filename);
#else
 nodesx->fd=ReOpenFileWriteable(nodesx->filename);
 segmentsx->fd=ReOpenFileWriteable(segmentsx->filename);
#endif

 connected=AllocBitMask(segmentsx->number);
 region   =AllocBitMask(segmentsx->number);

 assert(connected); /* Check AllocBitMask() worked */
 assert(region);    /* Check AllocBitMask() worked */

 regionsegments=(index_t*)malloc((nallocregionsegments=1024)*sizeof(index_t));
 othersegments =(index_t*)malloc((nallocothersegments =1024)*sizeof(index_t));

 assert(regionsegments); /* Check malloc() worked */
 assert(othersegments);  /* Check malloc() worked */

 /* Loop through the segments and find the disconnected ones */

 for(i=0;i<segmentsx->number;i++)
   {
    if(!IsBitSet(connected,i))
      {
       int nregionsegments=0,nothersegments=0;
       distance_t total=0;

       othersegments[nothersegments++]=i;
       SetBit(region,i);

       do
         {
          SegmentX *segmentx;
          index_t thissegment,nodes[2];

          thissegment=othersegments[--nothersegments];

          if(nregionsegments==nallocregionsegments)
             regionsegments=(index_t*)realloc(regionsegments,(nallocregionsegments+=1024)*sizeof(index_t));

          regionsegments[nregionsegments++]=thissegment;

          segmentx=LookupSegmentX(segmentsx,thissegment,1);

          nodes[0]=segmentx->node1;
          nodes[1]=segmentx->node2;
          total+=DISTANCE(segmentx->distance);

          for(j=0;j<2;j++)
            {
             segmentx=FirstSegmentX(segmentsx,nodes[j],1);

             while(segmentx)
               {
                index_t segment=IndexSegmentX(segmentsx,segmentx);

                if(segment!=thissegment)
                  {
                   if(IsBitSet(connected,segment))
                     {
                      total=minimum;
                      goto foundconnection;
                     }

                   if(!IsBitSet(region,segment))
                     {
                      if(nothersegments==nallocothersegments)
                         othersegments=(index_t*)realloc(othersegments,(nallocothersegments+=1024)*sizeof(index_t));

                      othersegments[nothersegments++]=segment;
                      SetBit(region,segment);
                     }
                  }

                segmentx=NextSegmentX(segmentsx,segmentx,nodes[j]);
               }
            }
         }
       while(nothersegments>0 && total<minimum);

      foundconnection:

       /* Prune the segments or mark them as connected */

       if(total<minimum)
         {
          nregions++;

          for(j=0;j<nregionsegments;j++)
            {
             SegmentX *segmentx=LookupSegmentX(segmentsx,regionsegments[j],1);

             SetBit(connected,regionsegments[j]);

             prune_segment(segmentsx,segmentx);

             npruned++;
            }
         }
       else
         {
          for(j=0;j<nregionsegments;j++)
            {
             SetBit(connected,regionsegments[j]);
             ClearBit(region,regionsegments[j]);
            }

          for(j=0;j<nothersegments;j++)
            {
             SetBit(connected,othersegments[j]);
             ClearBit(region,othersegments[j]);
            }
         }
      }

    if(!((i+1)%10000))
       printf_middle("Pruning Isolated Regions: Segments=%"Pindex_t" Pruned=%"Pindex_t" (%"Pindex_t" Regions)",i+1,npruned,nregions);
   }

 /* Unmap from memory / close the files */

 free(region);
 free(connected);

 free(regionsegments);
 free(othersegments);

#if !SLIM
 nodesx->data=UnmapFile(nodesx->filename);
 segmentsx->data=UnmapFile(segmentsx->filename);
#else
 nodesx->fd=CloseFile(nodesx->fd);
 segmentsx->fd=CloseFile(segmentsx->fd);
#endif

 /* Print the final message */

 printf_last("Pruned Isolated Regions: Segments=%"Pindex_t" Pruned=%"Pindex_t" (%"Pindex_t" Regions)",segmentsx->number,npruned,nregions);
}


/*++++++++++++++++++++++++++++++++++++++
  Prune a segment; unused nodes and ways will get marked for pruning later.

  SegmentsX *segmentsx The set of segments to use.

  SegmentX *segmentx The segment to be pruned.
  ++++++++++++++++++++++++++++++++++++++*/

static void prune_segment(SegmentsX *segmentsx,SegmentX *segmentx)
{
 unlink_segment_node_refs(segmentsx,segmentx,segmentx->node1);
 unlink_segment_node_refs(segmentsx,segmentx,segmentx->node2);

 segmentx->node1=NO_NODE;
 segmentx->node2=NO_NODE;
 segmentx->next2=NO_SEGMENT;

 PutBackSegmentX(segmentsx,segmentx);
}


/*++++++++++++++++++++++++++++++++++++++
  Modify a segment's nodes; unused nodes will get marked for pruning later.

  SegmentsX *segmentsx The set of segments to use.

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

 if(segment==NO_SEGMENT)
    return;

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
    while(segment!=thissegment && segment!=NO_SEGMENT);
   }
}
