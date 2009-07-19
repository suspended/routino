/***************************************
 $Header: /home/amb/CVS/routino/src/segmentsx.c,v 1.29 2009-07-19 14:10:27 amb Exp $

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
#include "nodes.h"
#include "segments.h"
#include "ways.h"


/* Constants */

/*+ The array size increment for SegmentsX (UK is ~7.1M raw segments, this is ~53 increments). +*/
#define INCREMENT_SEGMENTSX (128*1024)


/* Local Functions */

static int sort_by_id1_and_distance(SegmentX **a,SegmentX **b);
static int sort_by_id2_and_distance(SegmentX **a,SegmentX **b);
static SegmentX **find_first1_segmentx(SegmentsX* segmentsx,node_t node);
static SegmentX **find_first2_segmentx(SegmentsX* segmentsx,node_t node);
static distance_t DistanceX(NodeX *nodex1,NodeX *nodex2);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new segment list.

  SegmentsX *NewSegmentList Returns the segment list.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *NewSegmentList(void)
{
 SegmentsX *segmentsx;

 segmentsx=(SegmentsX*)calloc(1,sizeof(SegmentsX));

 segmentsx->row=-1;

 return(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a segment list.

  SegmentsX *segmentsx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeSegmentList(SegmentsX *segmentsx)
{
 if(segmentsx->xdata)
   {
    int i;
    for(i=0;i<=segmentsx->row;i++)
       free(segmentsx->xdata[i]);
    free(segmentsx->xdata);
   }

 if(segmentsx->n1data)
    free(segmentsx->n1data);

 if(segmentsx->n2data)
    free(segmentsx->n2data);

 if(segmentsx->sdata)
    free(segmentsx->sdata);

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
 int super_number=0,normal_number=0;

 assert(segmentsx->sorted);     /* Must be sorted */
 assert(segmentsx->sdata);      /* Must have sdata filled in */

 printf("Writing Segments: Segments=0");
 fflush(stdout);

 for(i=0;i<segmentsx->number;i++)
   {
    if(IsSuperSegment(&segmentsx->sdata[i]))
       super_number++;
    if(IsNormalSegment(&segmentsx->sdata[i]))
       normal_number++;
   }

 /* Fill in a Segments structure with the offset of the real data in the file after
    the Segment structure itself. */

 segments->number=segmentsx->number;
 segments->snumber=super_number;
 segments->nnumber=normal_number;

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
 SegmentX **first=find_first1_segmentx(segmentsx,node);

 if(first)
    return(first);

 return(find_first2_segmentx(segmentsx,node));
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment with a particular starting node in either n1data or n2data.

  SegmentX **find_first1_segmentx Returns a pointer to the first extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

static SegmentX **find_first1_segmentx(SegmentsX* segmentsx,node_t node)
{
 int start=0;
 int end=segmentsx->number-1;
 int mid;
 int found;

 assert(segmentsx->sorted);     /* Must be sorted */
 assert(segmentsx->n1data);     /* Must have n1data filled in */

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

 if(end<start)                                 /* There are no nodes */
    return(NULL);
 else if(node<segmentsx->n1data[start]->node1) /* Check key is not before start */
    return(NULL);
 else if(node>segmentsx->n1data[end]->node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                          /* Choose mid point */

       if(segmentsx->n1data[mid]->node1<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsx->n1data[mid]->node1>node) /* Mid point is too high */
          end=mid;
       else                                        /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsx->n1data[start]->node1==node)      /* Start is correct */
         {found=start; goto found;}

    if(segmentsx->n1data[end]->node1==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segmentsx->n1data[found-1]->node1==node)
    found--;

 return(&segmentsx->n1data[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment with a particular starting node in node2.

  SegmentX **find_first2_segmentx Returns a pointer to the first extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

static SegmentX **find_first2_segmentx(SegmentsX* segmentsx,node_t node)
{
 int start=0;
 int end=segmentsx->number-1;
 int mid;
 int found;

 assert(segmentsx->sorted);     /* Must be sorted */
 assert(segmentsx->n2data);     /* Must have n2data filled in */

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

 if(end<start)                                 /* There are no nodes */
    return(NULL);
 else if(node<segmentsx->n2data[start]->node2) /* Check key is not before start */
    return(NULL);
 else if(node>segmentsx->n2data[end]->node2)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                          /* Choose mid point */

       if(segmentsx->n2data[mid]->node2<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsx->n2data[mid]->node2>node) /* Mid point is too high */
          end=mid;
       else                                        /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsx->n2data[start]->node2==node)      /* Start is correct */
         {found=start; goto found;}

    if(segmentsx->n2data[end]->node2==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segmentsx->n2data[found-1]->node2==node)
    found--;

 return(&segmentsx->n2data[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  SegmentX **FindNextSegmentX Returns a pointer to the next segment with the same id.

  SegmentsX* segmentsx The set of segments to process.

  SegmentX **segmentx The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX **FindNextSegmentX(SegmentsX* segmentsx,SegmentX **segmentx)
{
 if((segmentsx->n1data<segmentsx->n2data && segmentx<segmentsx->n2data) ||
    (segmentsx->n1data>segmentsx->n2data && segmentx>segmentsx->n1data))
   {
    SegmentX **next=segmentx+1;

    if((next-segmentsx->n1data)==segmentsx->number)
       return(find_first2_segmentx(segmentsx,(*segmentx)->node1));

    if((*next)->node1==(*segmentx)->node1)
       return(next);

    return(find_first2_segmentx(segmentsx,(*segmentx)->node1));
   }
 else
   {
    SegmentX **next=segmentx+1;

    if((next-segmentsx->n2data)==segmentsx->number)
       return(NULL);

    if((*next)->node2==(*segmentx)->node2)
       return(next);

    return(NULL);
   }
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Append a segment to a segment list.

  SegmentsX* segmentsx The set of segments to process.

  way_t way The way that the segment belongs to.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.

  distance_t distance The distance between the nodes (or just the flags).
  ++++++++++++++++++++++++++++++++++++++*/

void AppendSegment(SegmentsX* segmentsx,way_t way,node_t node1,node_t node2,distance_t distance)
{
 /* Check that the array has enough space. */

 if(segmentsx->row==-1 || segmentsx->col==INCREMENT_SEGMENTSX)
   {
    segmentsx->row++;
    segmentsx->col=0;

    if((segmentsx->row%16)==0)
       segmentsx->xdata=(SegmentX**)realloc((void*)segmentsx->xdata,(segmentsx->row+16)*sizeof(SegmentX*));

    segmentsx->xdata[segmentsx->row]=(SegmentX*)malloc(INCREMENT_SEGMENTSX*sizeof(SegmentX));
   }

 /* Insert the segment */

 segmentsx->xdata[segmentsx->row][segmentsx->col].way=way;
 segmentsx->xdata[segmentsx->row][segmentsx->col].distance=distance;

 if(node1>node2)
   {
    segmentsx->xdata[segmentsx->row][segmentsx->col].node1=node2;
    segmentsx->xdata[segmentsx->row][segmentsx->col].node2=node1;
    if(distance&(ONEWAY_2TO1|ONEWAY_1TO2))
       segmentsx->xdata[segmentsx->row][segmentsx->col].distance^=(ONEWAY_2TO1|ONEWAY_1TO2);
   }
 else
   {
    segmentsx->xdata[segmentsx->row][segmentsx->col].node1=node1;
    segmentsx->xdata[segmentsx->row][segmentsx->col].node2=node2;
   }

 segmentsx->col++;

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

 printf("Sorting Segments");
 fflush(stdout);

 /* Allocate the array of pointers and sort them */

 segmentsx->n1data=(SegmentX**)realloc(segmentsx->n1data,(segmentsx->row*INCREMENT_SEGMENTSX+segmentsx->col)*sizeof(SegmentX*));
 segmentsx->n2data=(SegmentX**)realloc(segmentsx->n2data,(segmentsx->row*INCREMENT_SEGMENTSX+segmentsx->col)*sizeof(SegmentX*));

 segmentsx->number=0;

 for(i=0;i<(segmentsx->row*INCREMENT_SEGMENTSX+segmentsx->col);i++)
    if(segmentsx->xdata[i/INCREMENT_SEGMENTSX][i%INCREMENT_SEGMENTSX].node1!=NO_NODE)
      {
       segmentsx->n1data[segmentsx->number]=&segmentsx->xdata[i/INCREMENT_SEGMENTSX][i%INCREMENT_SEGMENTSX];
       segmentsx->n2data[segmentsx->number]=&segmentsx->xdata[i/INCREMENT_SEGMENTSX][i%INCREMENT_SEGMENTSX];
       segmentsx->number++;
      }

 qsort(segmentsx->n1data,segmentsx->number,sizeof(SegmentX*),(int (*)(const void*,const void*))sort_by_id1_and_distance);
 qsort(segmentsx->n2data,segmentsx->number,sizeof(SegmentX*),(int (*)(const void*,const void*))sort_by_id2_and_distance);

 printf("\rSorted Segments \n");
 fflush(stdout);

 segmentsx->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order (node1 then node2) and then distance order.

  int sort_by_id1_and_distance Returns the comparison of the node fields.

  SegmentX **a The first Segment.

  SegmentX **b The second Segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id1_and_distance(SegmentX **a,SegmentX **b)
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
       distance_t a_distance=(*a)->distance;
       distance_t b_distance=(*b)->distance;

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
  Sort the segments into id order (node2 then node1) and then distance order.

  int sort_by_id2_and_distance Returns the comparison of the node fields.

  SegmentX **a The first Segment.

  SegmentX **b The second Segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id2_and_distance(SegmentX **a,SegmentX **b)
{
 node_t a_id2=(*a)->node2;
 node_t b_id2=(*b)->node2;

 if(a_id2<b_id2)
    return(-1);
 else if(a_id2>b_id2)
    return(1);
 else /* if(a_id2==b_id2) */
   {
    node_t a_id1=(*a)->node1;
    node_t b_id1=(*b)->node1;

    if(a_id1<b_id1)
       return(-1);
    else if(a_id1>b_id1)
       return(1);
    else
      {
       distance_t a_distance=(*a)->distance;
       distance_t b_distance=(*b)->distance;

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

 assert(segmentsx->n1data);     /* Must have n1data filled in */

 printf("Checking: Segments=0 Duplicate=0 Loop=0 Missing-Node=0");
 fflush(stdout);

 for(i=0;i<segmentsx->number;i++)
   {
    if(i && segmentsx->n1data[i]->node1==segmentsx->n1data[i-1]->node1 &&
            segmentsx->n1data[i]->node2==segmentsx->n1data[i-1]->node2)
      {
       duplicate++;
       segmentsx->n1data[i-1]->node1=NO_NODE;
      }
    else if(segmentsx->n1data[i]->node1==segmentsx->n1data[i]->node2)
      {
       loop++;
       segmentsx->n1data[i]->node1=NO_NODE;
      }
    else if(!FindNodeX(nodesx,segmentsx->n1data[i]->node1) ||
            !FindNodeX(nodesx,segmentsx->n1data[i]->node2))
      {
       missing++;
       segmentsx->n1data[i]->node1=NO_NODE;
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

 assert(segmentsx->n1data);     /* Must have n1data filled in */

 printf("Measuring Segments: Segments=0");
 fflush(stdout);

 for(i=0;i<segmentsx->number;i++)
   {
    NodeX **nodex1=FindNodeX(nodesx,segmentsx->n1data[i]->node1);
    NodeX **nodex2=FindNodeX(nodesx,segmentsx->n1data[i]->node2);

    /* Set the distance but preserve the ONEWAY_* flags */

    segmentsx->n1data[i]->distance|=DISTANCE(DistanceX(*nodex1,*nodex2));

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
  Mark the duplicate segments.

  SegmentsX* segmentsx The set of segments to process.

  WaysX *waysx The list of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void DeduplicateSegments(SegmentsX* segmentsx,WaysX *waysx)
{
 index_t i;
 int duplicate=0;

 assert(segmentsx->n1data);     /* Must have n1data filled in */

 printf("Deduplicating Segments: Segments=0 Duplicate=0");
 fflush(stdout);

 for(i=1;i<segmentsx->number;i++)
   {
    if(segmentsx->n1data[i]->node1==segmentsx->n1data[i-1]->node1 &&
       segmentsx->n1data[i]->node2==segmentsx->n1data[i-1]->node2 &&
       DISTFLAG(segmentsx->n1data[i]->distance)==DISTFLAG(segmentsx->n1data[i-1]->distance))
      {
       WayX *wayx1=FindWayX(waysx,segmentsx->n1data[i-1]->way);
       WayX *wayx2=FindWayX(waysx,segmentsx->n1data[i  ]->way);

       if(!WaysCompare(wayx1->way,wayx2->way))
         {
          segmentsx->n1data[i-1]->node1=NO_NODE;

          duplicate++;
         }
      }

    if(!((i+1)%10000))
      {
       printf("\rDeduplicating Segments: Segments=%d Duplicate=%d",i+1,duplicate);
       fflush(stdout);
      }
   }

 printf("\rDeduplicated Segments: Segments=%d Duplicate=%d Unique=%d\n",segmentsx->number,duplicate,segmentsx->number-duplicate);
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

 assert(segmentsx->n1data);     /* Must have n1data filled in */

 printf("Creating Real Segments: Segments=0");
 fflush(stdout);

 /* Allocate the memory */

 segmentsx->sdata=(Segment*)malloc(segmentsx->number*sizeof(Segment));

 /* Loop through and allocate. */

 for(i=0;i<segmentsx->number;i++)
   {
    WayX *wayx=FindWayX(waysx,segmentsx->n1data[i]->way);

    segmentsx->sdata[i].node1=0;
    segmentsx->sdata[i].node2=0;
    segmentsx->sdata[i].next2=NO_NODE;
    segmentsx->sdata[i].way=IndexWayInWaysX(waysx,wayx);
    segmentsx->sdata[i].distance=segmentsx->n1data[i]->distance;

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
 assert(segmentsx->n1data);     /* Must have n1data filled in */
 assert(segmentsx->sdata);      /* Must have sdata filled in */
 assert(nodesx->sorted);        /* Must be sorted */
 assert(nodesx->gdata);         /* Must have gdata filled in */

 printf("Indexing Nodes: Nodes=0");
 fflush(stdout);

 /* Index the segments */

 for(i=0;i<nodesx->number;i++)
   {
    NodeX **nodex=FindNodeX(nodesx,nodesx->gdata[i]->id);
    Node   *node =&nodesx->ndata[nodex-nodesx->idata];
    index_t index=SEGMENT(node->firstseg);

    do
      {
       if(segmentsx->n1data[index]->node1==nodesx->gdata[i]->id)
         {
          segmentsx->sdata[index].node1=i;

          index++;

          if(index>=segmentsx->number || segmentsx->n1data[index]->node1!=nodesx->gdata[i]->id)
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

static distance_t DistanceX(NodeX *nodex1,NodeX *nodex2)
{
 double dlon = latlong_to_radians(nodex1->longitude) - latlong_to_radians(nodex2->longitude);
 double dlat = latlong_to_radians(nodex1->latitude)  - latlong_to_radians(nodex2->latitude);
 double lat1 = latlong_to_radians(nodex1->latitude);
 double lat2 = latlong_to_radians(nodex2->latitude);

 double a1,a2,a,sa,c,d;

 if(dlon==0 && dlat==0)
   return 0;

 a1 = sin (dlat / 2);
 a2 = sin (dlon / 2);
 a = (a1 * a1) + cos (lat1) * cos (lat2) * a2 * a2;
 sa = sqrt (a);
 if (sa <= 1.0)
   {c = 2 * asin (sa);}
 else
   {c = 2 * asin (1.0);}
 d = 6378.137 * c;

 return km_to_distance(d);
}
