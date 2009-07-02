/***************************************
 $Header: /home/amb/CVS/routino/src/segmentsx.c,v 1.18 2009-07-02 17:49:16 amb Exp $

 Extended Segment data type functions.

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

#include "types.h"
#include "functions.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"


/* Constants */

/*+ The array size increment for segments - expect ~8,000,000 segments. +*/
#define INCREMENT_SEGMENTS 1024*1024


/* Functions */

static int sort_by_id_and_distance(SegmentX **a,SegmentX **b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new segment list.

  SegmentsX *NewSegmentList Returns the segment list.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *NewSegmentList(void)
{
 SegmentsX *segmentsx;

 segmentsx=(SegmentsX*)calloc(1,sizeof(SegmentsX));

 return(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a segment list (needed by planetsplitter for temporary super-segment lists).

  SegmentsX *segmentsx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeSegmentList(SegmentsX *segmentsx)
{
 if(segmentsx->sdata)
    free(segmentsx->sdata);
 if(segmentsx->ndata)
    free(segmentsx->ndata);
 if(segmentsx->xdata)
    free(segmentsx->xdata);
 free(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the segment list to a file.

  SegmentsX* segmentsx The set of segments to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveSegmentList(SegmentsX* segmentsx,const char *filename)
{
 index_t i;
 int fd;
 Segments *segments=calloc(1,sizeof(Segments));

 assert(segmentsx->sorted);     /* Must be sorted */
 assert(segmentsx->sdata);      /* Must have sdata filled in */

 /* Fill in a Segments structure with the offset of the real data in the file after
    the Segment structure itself. */

 segments->number=segmentsx->number;
 segments->data=NULL;
 segments->segments=(void*)sizeof(Segments);

 /* Write out the Segments structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,segments,sizeof(Segments));

 for(i=0;i<segments->number;i++)
   {
    WriteFile(fd,&segmentsx->sdata[i],sizeof(Segment));

    if(!((i+1)%10000))
      {
       printf("\rWriting Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rWrote Segments: Segments=%d  \n",segments->number);
 fflush(stdout);

 CloseFile(fd);

 /* Free the fake Segments */

 free(segments);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment with a particular starting node.

  SegmentX **FindFirstSegmentX Returns a pointer to the first extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX **FindFirstSegmentX(SegmentsX* segmentsx,node_t node)
{
 int start=0;
 int end=segmentsx->number-1;
 int mid;
 int found;

 assert(segmentsx->sorted);     /* Must be sorted */
 assert(segmentsx->ndata);      /* Must have ndata filled in */

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

 if(end<start)                                /* There are no nodes */
    return(NULL);
 else if(node<segmentsx->ndata[start]->node1) /* Check key is not before start */
    return(NULL);
 else if(node>segmentsx->ndata[end]->node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                         /* Choose mid point */

       if(segmentsx->ndata[mid]->node1<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsx->ndata[mid]->node1>node) /* Mid point is too high */
          end=mid;
       else                                       /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsx->ndata[start]->node1==node)      /* Start is correct */
         {found=start; goto found;}

    if(segmentsx->ndata[end]->node1==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segmentsx->ndata[found-1]->node1==node)
    found--;

 return(&segmentsx->ndata[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  SegmentX **FindNextSegmentX Returns a pointer to the next segment with the same id.

  SegmentsX* segmentsx The set of segments to process.

  SegmentX **segmentx The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX **FindNextSegmentX(SegmentsX* segmentsx,SegmentX **segmentx)
{
 SegmentX **next=segmentx+1;

 if((next-segmentsx->ndata)==segmentsx->number)
    return(NULL);

 if((*next)->node1==(*segmentx)->node1)
    return(next);

 return(NULL);
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Append a segment to a segment list.

  SegmentsX* segmentsx The set of segments to process.

  way_t way The way that the segment belongs to.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendSegment(SegmentsX* segmentsx,way_t way,node_t node1,node_t node2,distance_t distance)
{
 /* Check that the array has enough space. */

 if(segmentsx->xnumber==segmentsx->alloced)
   {
    segmentsx->alloced+=INCREMENT_SEGMENTS;

    segmentsx->xdata=(SegmentX*)realloc((void*)segmentsx->xdata,segmentsx->alloced*sizeof(SegmentX));
   }

 /* Insert the segment */

 segmentsx->xdata[segmentsx->xnumber].way=way;
 segmentsx->xdata[segmentsx->xnumber].node1=node1;
 segmentsx->xdata[segmentsx->xnumber].node2=node2;
 segmentsx->xdata[segmentsx->xnumber].distance=distance;

 segmentsx->xnumber++;

 segmentsx->sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.

  SegmentsX* segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(SegmentsX* segmentsx)
{
 index_t i;

 assert(segmentsx->xdata);      /* Must have xdata filled in */

 printf("Sorting Segments"); fflush(stdout);

 /* Allocate the array of pointers and sort them */

 segmentsx->ndata=(SegmentX**)realloc(segmentsx->ndata,segmentsx->xnumber*sizeof(SegmentX*));

 segmentsx->number=0;

 for(i=0;i<segmentsx->xnumber;i++)
    if(segmentsx->xdata[i].node1!=NO_NODE)
      {
       segmentsx->ndata[segmentsx->number]=&segmentsx->xdata[i];
       segmentsx->number++;
      }

 qsort(segmentsx->ndata,segmentsx->number,sizeof(SegmentX*),(int (*)(const void*,const void*))sort_by_id_and_distance);

 printf("\rSorted Segments \n"); fflush(stdout);

 segmentsx->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order and then distance order.

  int sort_by_id_and_distance Returns the comparison of the node fields.

  SegmentX **a The first Segment.

  SegmentX **b The second Segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id_and_distance(SegmentX **a,SegmentX **b)
{
 node_t a_id1=(*a)->node1;
 node_t b_id1=(*b)->node1;

 if(a_id1<b_id1)
    return(-1);
 else if(a_id1>b_id1)
    return(1);
 else /* if(a_id1==b_id1) */
   {
    node_t a_id2=(*a)->node2;
    node_t b_id2=(*b)->node2;

    if(a_id2<b_id2)
       return(-1);
    else if(a_id2>b_id2)
       return(1);
    else
      {
       distance_t a_distance=DISTANCE((*a)->distance);
       distance_t b_distance=DISTANCE((*b)->distance);

       if(a_distance<b_distance)
          return(-1);
       else if(a_distance>b_distance)
          return(1);
       else
          return(0);
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Remove bad segments (zero length or duplicated).

  NodesX *nodesx The nodes to check.

  SegmentsX *segmentsx The segments to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveBadSegments(NodesX *nodesx,SegmentsX *segmentsx)
{
 index_t i;
 int duplicate=0,loop=0,missing=0;

 assert(segmentsx->ndata);      /* Must have ndata filled in */

 for(i=0;i<segmentsx->number;i++)
   {
    if(i && segmentsx->ndata[i]->node1==segmentsx->ndata[i-1]->node1 &&
            segmentsx->ndata[i]->node2==segmentsx->ndata[i-1]->node2)
      {
       duplicate++;
       segmentsx->ndata[i-1]->node1=NO_NODE;
      }
    else if(segmentsx->ndata[i]->node1==segmentsx->ndata[i]->node2)
      {
       loop++;
       segmentsx->ndata[i]->node1=NO_NODE;
      }
    else if(!FindNodeX(nodesx,segmentsx->ndata[i]->node1) ||
            !FindNodeX(nodesx,segmentsx->ndata[i]->node2))
      {
       missing++;
       segmentsx->ndata[i]->node1=NO_NODE;
      }

    if(!((i+1)%10000))
      {
       printf("\rChecking: Segments=%d Duplicate=%d Loop=%d Missing-Node=%d",i+1,duplicate,loop,missing);
       fflush(stdout);
      }
   }

 printf("\rChecked: Segments=%d Duplicate=%d Loop=%d Missing-Node=%d \n",segmentsx->number,duplicate,loop,missing);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Measure the segments.

  SegmentsX* segmentsx The set of segments to process.

  NodesX *nodesx The list of nodes to use.
  ++++++++++++++++++++++++++++++++++++++*/

void MeasureSegments(SegmentsX* segmentsx,NodesX *nodesx)
{
 index_t i;

 assert(segmentsx->ndata);      /* Must have ndata filled in */

 for(i=0;i<segmentsx->number;i++)
   {
    NodeX **nodex1=FindNodeX(nodesx,segmentsx->ndata[i]->node1);
    NodeX **nodex2=FindNodeX(nodesx,segmentsx->ndata[i]->node2);

    /* Set the distance but preserve the ONEWAY_* flags */

    segmentsx->ndata[i]->distance|=DISTANCE(DistanceX(*nodex1,*nodex2));

    if(!((i+1)%10000))
      {
       printf("\rMeasuring Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rMeasured Segments: Segments=%d \n",segmentsx->number);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Make the segments all point the same way (node1<node2).

  SegmentsX* segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void RotateSegments(SegmentsX* segmentsx)
{
 index_t i;
 int rotated=0;

 assert(segmentsx->ndata);      /* Must have ndata filled in */

 for(i=0;i<segmentsx->number;i++)
   {
    if(segmentsx->ndata[i]->node1>segmentsx->ndata[i]->node2)
      {
       segmentsx->ndata[i]->node1^=segmentsx->ndata[i]->node2;
       segmentsx->ndata[i]->node2^=segmentsx->ndata[i]->node1;
       segmentsx->ndata[i]->node1^=segmentsx->ndata[i]->node2;

       if(segmentsx->ndata[i]->distance&(ONEWAY_2TO1|ONEWAY_1TO2))
          segmentsx->ndata[i]->distance^=ONEWAY_2TO1|ONEWAY_1TO2;

       rotated++;
      }

    if(!((i+1)%10000))
      {
       printf("\rRotating Segments: Segments=%d Rotated=%d",i+1,rotated);
       fflush(stdout);
      }
   }

 printf("\rRotated Segments: Segments=%d Rotated=%d \n",segmentsx->number,rotated);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Mark the duplicate segments.

  SegmentsX* segmentsx The set of segments to process.

  WaysX *waysx The list of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void DeduplicateSegments(SegmentsX* segmentsx,WaysX *waysx)
{
 index_t i;
 int duplicate=0;

 assert(segmentsx->ndata);      /* Must have ndata filled in */

 for(i=1;i<segmentsx->number;i++)
   {
    if(segmentsx->ndata[i]->node1==segmentsx->ndata[i-1]->node1 &&
       segmentsx->ndata[i]->node2==segmentsx->ndata[i-1]->node2 &&
       segmentsx->ndata[i]->distance==segmentsx->ndata[i-1]->distance)
      {
       WayX *wayx1=FindWayX(waysx,segmentsx->ndata[i-1]->way);
       WayX *wayx2=FindWayX(waysx,segmentsx->ndata[i  ]->way);

       if(!WaysCompare(wayx1->way,wayx2->way))
         {
          segmentsx->ndata[i-1]->node1=NO_NODE;
          segmentsx->ndata[i-1]->node2=NO_NODE;

          duplicate++;
         }
      }

    if(!((i+1)%10000))
      {
       printf("\rDeduplicating Segments: Segments=%d Duplicate=%d",i+1,duplicate);
       fflush(stdout);
      }
   }

 printf("\rDeduplicated Segments: Segments=%d Duplicate=%d \n",segmentsx->number,duplicate);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the real segments data.

  SegmentsX* segmentsx The set of segments to use.

  WaysX* waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateRealSegments(SegmentsX *segmentsx,WaysX *waysx)
{
 index_t i;

 assert(segmentsx->ndata);      /* Must have ndata filled in */

 /* Allocate the memory */

 segmentsx->sdata=(Segment*)malloc(segmentsx->number*sizeof(Segment));

 /* Loop through and allocate. */

 for(i=0;i<segmentsx->number;i++)
   {
    WayX *wayx=FindWayX(waysx,segmentsx->ndata[i]->way);

    segmentsx->sdata[i].node1=0;
    segmentsx->sdata[i].node2=0;
    segmentsx->sdata[i].next2=NO_NODE;
    segmentsx->sdata[i].way=wayx->way-waysx->wdata;
    segmentsx->sdata[i].distance=segmentsx->ndata[i]->distance;

    if(!((i+1)%10000))
      {
       printf("\rCreating Real Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rCreating Real Segments: Segments=%d \n",segmentsx->number);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Assign the nodes indexes to the segments.

  SegmentsX* segmentsx The set of segments to process.

  NodesX *nodesx The list of nodes to use.
  ++++++++++++++++++++++++++++++++++++++*/

void IndexSegments(SegmentsX* segmentsx,NodesX *nodesx)
{
 index_t i;

 assert(segmentsx->sorted);     /* Must be sorted */
 assert(segmentsx->ndata);      /* Must have ndata filled in */
 assert(segmentsx->sdata);      /* Must have sdata filled in */
 assert(nodesx->sorted);        /* Must be sorted */
 assert(nodesx->gdata);         /* Must have gdata filled in */

 /* Index the segments */

 for(i=0;i<nodesx->number;i++)
   {
    NodeX **nodex=FindNodeX(nodesx,nodesx->gdata[i]->id);
    Node   *node =&nodesx->ndata[nodex-nodesx->idata];
    index_t index=SEGMENT(node->firstseg);

    do
      {
       if(segmentsx->ndata[index]->node1==nodesx->gdata[i]->id)
         {
          segmentsx->sdata[index].node1=i;

          index++;

          if(index>=segmentsx->number || segmentsx->ndata[index]->node1!=nodesx->gdata[i]->id)
             break;
         }
       else
         {
          segmentsx->sdata[index].node2=i;

          if(segmentsx->sdata[index].next2==NO_NODE)
             break;
          else
             index=segmentsx->sdata[index].next2;
         }
      }
    while(1);

    if(!((i+1)%10000))
      {
       printf("\rIndexing Nodes: Nodes=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rIndexed Nodes: Nodes=%d \n",nodesx->number);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the distance between two nodes.

  distance_t DistanceX Returns the distance between the extended nodes.

  NodeX *nodex1 The starting node.

  NodeX *nodex2 The end node.
  ++++++++++++++++++++++++++++++++++++++*/

distance_t DistanceX(NodeX *nodex1,NodeX *nodex2)
{
 float dlon = nodex1->longitude - nodex2->longitude;
 float dlat = nodex1->latitude  - nodex2->latitude;

 float a1,a2,a,sa,c,d;

 if(dlon==0 && dlat==0)
   return 0;

 a1 = sinf (dlat / 2);
 a2 = sinf (dlon / 2);
 a = (a1 * a1) + cosf (nodex1->latitude) * cosf (nodex2->latitude) * a2 * a2;
 sa = sqrtf (a);
 if (sa <= 1.0)
   {c = 2 * asinf (sa);}
 else
   {c = 2 * asinf (1.0);}
 d = 6378.137 * c;

 return km_to_distance(d);
}
