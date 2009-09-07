/***************************************
 $Header: /home/amb/CVS/routino/src/segmentsx.c,v 1.34 2009-09-07 19:01:58 amb Exp $

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
#include <string.h>

#include "types.h"
#include "functions.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"


/* Variables */

extern int option_slim;
extern char *tmpdirname;

/* Local Functions */

static int sort_by_id1_and_distance(index_t *a,index_t *b);
static int sort_by_id2_and_distance(index_t *a,index_t *b);
static index_t *index_first1_segmentx(SegmentsX* segmentsx,node_t node);
static index_t *index_first2_segmentx(SegmentsX* segmentsx,node_t node);
static distance_t DistanceX(NodeX *nodex1,NodeX *nodex2);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new segment list.

  SegmentsX *NewSegmentList Returns the segment list.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *NewSegmentList(void)
{
 SegmentsX *segmentsx;

 segmentsx=(SegmentsX*)calloc(1,sizeof(SegmentsX));

 assert(segmentsx); /* Check calloc() worked */

 segmentsx->filename=(char*)malloc(strlen(tmpdirname)+24);
 sprintf(segmentsx->filename,"%s/segments.%p.tmp",tmpdirname,segmentsx);

 segmentsx->fd=OpenFile(segmentsx->filename);

 return(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a segment list.

  SegmentsX *segmentsx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeSegmentList(SegmentsX *segmentsx)
{
 if(segmentsx->xdata)
    UnmapFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

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
 Segments *segments;
 int super_number=0,normal_number=0;

 assert(segmentsx->sdata);      /* Must have sdata filled in => real segments */

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

 segments=calloc(1,sizeof(Segments));

 assert(segments); /* Check calloc() worked */

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
  Lookup a particular segment.

  SegmentX *LookupSegmentX Returns a pointer to the extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  index_t index The segment index to look for.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX *LookupSegmentX(SegmentsX* segmentsx,index_t index)
{
 assert(index!=NO_SEGMENT);     /* Must be a valid segment */

 if(option_slim)
   {
    SeekFile(segmentsx->fd,index*sizeof(SegmentX));

    ReadFile(segmentsx->fd,&segmentsx->cached,sizeof(SegmentX));

    return(&segmentsx->cached);
   }
 else
   {
    return(&segmentsx->xdata[index]);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment index with a particular starting node in either n1data or n2data.
 
  index_t *IndexFirstSegmentX Returns a pointer to the index of the first extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t *IndexFirstSegmentX(SegmentsX* segmentsx,node_t node)
{
 index_t *first=index_first1_segmentx(segmentsx,node);

 if(first)
    return(first);

 return(index_first2_segmentx(segmentsx,node));
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment index with a particular starting node in n1data.
 
  index_t *index_first1_segmentx Returns a pointer to the index of the first extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

static index_t *index_first1_segmentx(SegmentsX* segmentsx,node_t node)
{
 int start=0;
 int end=segmentsx->number-1;
 int mid;
 int found;

 assert(segmentsx->n1data);     /* Must have n1data filled in => sorted by node 1 */
 assert(segmentsx->xdata);      /* Must have xdata filled in => mapped from file */

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

 if(end<start)                                                  /* There are no nodes */
    return(NULL);
 else if(node<segmentsx->xdata[segmentsx->n1data[start]].node1) /* Check key is not before start */
    return(NULL);
 else if(node>segmentsx->xdata[segmentsx->n1data[end]].node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                                           /* Choose mid point */

       if(segmentsx->xdata[segmentsx->n1data[mid]].node1<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsx->xdata[segmentsx->n1data[mid]].node1>node) /* Mid point is too high */
          end=mid;
       else                                                         /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsx->xdata[segmentsx->n1data[start]].node1==node)      /* Start is correct */
      {found=start; goto found;}

    if(segmentsx->xdata[segmentsx->n1data[end]].node1==node)        /* End is correct */
      {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segmentsx->xdata[segmentsx->n1data[found-1]].node1==node)
    found--;

 return(&segmentsx->n1data[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment index with a particular starting node in n1data.
 
  index_t *index_first2_segmentx Returns a pointer to the index of the first extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

static index_t *index_first2_segmentx(SegmentsX* segmentsx,node_t node)
{
 int start=0;
 int end=segmentsx->number-1;
 int mid;
 int found;

 assert(segmentsx->n2data);     /* Must have n2data filled in => sorted by node 2 */
 assert(segmentsx->xdata);      /* Must have xdata filled in => mapped from file */

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

 if(end<start)                                                  /* There are no nodes */
    return(NULL);
 else if(node<segmentsx->xdata[segmentsx->n2data[start]].node2) /* Check key is not before start */
    return(NULL);
 else if(node>segmentsx->xdata[segmentsx->n2data[end]].node2)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                                           /* Choose mid point */

       if(segmentsx->xdata[segmentsx->n2data[mid]].node2<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsx->xdata[segmentsx->n2data[mid]].node2>node) /* Mid point is too high */
          end=mid;
       else                                                         /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsx->xdata[segmentsx->n2data[start]].node2==node)      /* Start is correct */
      {found=start; goto found;}

    if(segmentsx->xdata[segmentsx->n2data[end]].node2==node)        /* End is correct */
      {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segmentsx->xdata[segmentsx->n2data[found-1]].node2==node)
    found--;

 return(&segmentsx->n2data[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment index with a particular starting node.

  index_t *IndexNextSegmentX Returns a pointer to the index of the next segment with the same id.

  SegmentsX* segmentsx The set of segments to process.

  SegmentX *segmentx The current segment.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t *IndexNextSegmentX(SegmentsX* segmentsx,index_t *index,node_t node)
{
 if((segmentsx->n1data<segmentsx->n2data && index< segmentsx->n2data) ||
    (segmentsx->n1data>segmentsx->n2data && index>=segmentsx->n1data))
   {
    index++;

    if((index-segmentsx->n1data)==segmentsx->number)
       return(index_first2_segmentx(segmentsx,node));

    if(segmentsx->xdata[*index].node1==node)
       return(index);

    return(index_first2_segmentx(segmentsx,node));
   }
 else
   {
    index++;

    if((index-segmentsx->n2data)==segmentsx->number)
       return(NULL);

    if(segmentsx->xdata[*index].node2==node)
       return(index);

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
 SegmentX segmentx;

 assert(!segmentsx->n1data);    /* Must not have n1data filled in => unsorted */

 segmentx.way=way;
 segmentx.distance=distance;

 if(node1>node2)
   {
    segmentx.node1=node2;
    segmentx.node2=node1;
    if(distance&(ONEWAY_2TO1|ONEWAY_1TO2))
       segmentx.distance^=(ONEWAY_2TO1|ONEWAY_1TO2);
   }
 else
   {
    segmentx.node1=node1;
    segmentx.node2=node2;
   }

 WriteFile(segmentsx->fd,&segmentx,sizeof(SegmentX));

 segmentsx->xnumber++;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list for the first time (i.e. create the sortable indexes).

  SegmentsX* segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void InitialSortSegmentList(SegmentsX* segmentsx)
{
 index_t i;

 assert(!segmentsx->n1data);    /* Must not have n1data filled in => unsorted */

 printf("Sorting Segments (pre-sort)");
 fflush(stdout);

 /* Allocate the array of indexes */

 segmentsx->n1data=(index_t*)malloc(segmentsx->xnumber*sizeof(index_t));

 assert(segmentsx->n1data); /* Check malloc() worked */

 CloseFile(segmentsx->fd);
 segmentsx->fd=ReOpenFile(segmentsx->filename);

 segmentsx->xdata=MapFile(segmentsx->filename);

 for(i=0;i<segmentsx->xnumber;i++)
    segmentsx->n1data[i]=i;

 segmentsx->number=segmentsx->xnumber;

 printf("\rSorted Segments (pre-sort) \n");
 fflush(stdout);

 ReSortSegmentList(segmentsx);
}


/*+ A temporary file-local variable for use by the sort function. +*/
static SegmentsX *sortsegmentsx;


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list again.

  SegmentsX* segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void ReSortSegmentList(SegmentsX* segmentsx)
{
 assert(segmentsx->n1data);   /* Must have n1data filled in => initially sorted */
 assert(segmentsx->xdata);      /* Must have xdata filled in => mapped from file */

 printf("Sorting Segments");
 fflush(stdout);

 sortsegmentsx=segmentsx;

 qsort(segmentsx->n1data,segmentsx->number,sizeof(index_t),(int (*)(const void*,const void*))sort_by_id1_and_distance);

 while(segmentsx->n1data[segmentsx->number-1]==NO_SEGMENT)
    segmentsx->number--;

 printf("\rSorted Segments \n");
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list for the first time (i.e. create the sortable indexes).

  SegmentsX* segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void FinalSortSegmentList(SegmentsX* segmentsx)
{
 index_t i;

 assert(segmentsx->n1data);    /* Must have n1data filled in => initially sorted */
 assert(!segmentsx->n2data);   /* Must not have n2data filled in => not finally sorted */

 ReSortSegmentList(segmentsx);

 printf("Sorting Segments (post-sort)");
 fflush(stdout);

 /* Allocate the array of indexes */

 segmentsx->n2data=(index_t*)malloc(segmentsx->xnumber*sizeof(index_t));

 assert(segmentsx->n2data); /* Check malloc() worked */

 for(i=0;i<segmentsx->number;i++)
    segmentsx->n2data[i]=segmentsx->n1data[i];

 sortsegmentsx=segmentsx;

 qsort(segmentsx->n2data,segmentsx->number,sizeof(index_t),(int (*)(const void*,const void*))sort_by_id2_and_distance);

 printf("\rSorted Segments (post-sort) \n");
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order (node1 then node2) and then distance order.

  int sort_by_id1_and_distance Returns the comparison of the node fields.

  index_t *a The first segment index.

  index_t *b The second segment index.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id1_and_distance(index_t *a,index_t *b)
{
 if(*a==NO_SEGMENT)
    return(1);
 else if(*b==NO_SEGMENT)
    return(-1);
 else
   {
    SegmentX *segmentx_a=&sortsegmentsx->xdata[*a];
    SegmentX *segmentx_b=&sortsegmentsx->xdata[*b];

    node_t a_id1=segmentx_a->node1;
    node_t b_id1=segmentx_b->node1;

    if(a_id1<b_id1)
       return(-1);
    else if(a_id1>b_id1)
       return(1);
    else /* if(a_id1==b_id1) */
      {
       node_t a_id2=segmentx_a->node2;
       node_t b_id2=segmentx_b->node2;

       if(a_id2<b_id2)
          return(-1);
       else if(a_id2>b_id2)
          return(1);
       else
         {
          distance_t a_distance=segmentx_a->distance;
          distance_t b_distance=segmentx_b->distance;

          if(a_distance<b_distance)
             return(-1);
          else if(a_distance>b_distance)
             return(1);
          else
             return(0);
         }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order (node2 then node1) and then distance order.

  int sort_by_id2_and_distance Returns the comparison of the node fields.

  index_t *a The first segment index.

  index_t *b The second segment index.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id2_and_distance(index_t *a,index_t *b)
{
 SegmentX *segmentx_a=&sortsegmentsx->xdata[*a];
 SegmentX *segmentx_b=&sortsegmentsx->xdata[*b];

 node_t a_id2=segmentx_a->node2;
 node_t b_id2=segmentx_b->node2;

 if(a_id2<b_id2)
    return(-1);
 else if(a_id2>b_id2)
    return(1);
 else /* if(a_id2==b_id2) */
   {
    node_t a_id1=segmentx_a->node1;
    node_t b_id1=segmentx_b->node1;

    if(a_id1<b_id1)
       return(-1);
    else if(a_id1>b_id1)
       return(1);
    else
      {
       distance_t a_distance=segmentx_a->distance;
       distance_t b_distance=segmentx_b->distance;

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
 SegmentX *prevsegmentx=NULL;

 assert(segmentsx->n1data);     /* Must have n1data filled in => sorted by node 1 */
 assert(segmentsx->xdata);      /* Must have xdata filled in => mapped from file */

 printf("Checking: Segments=0 Duplicate=0 Loop=0 Missing-Node=0");
 fflush(stdout);

 for(i=0;i<segmentsx->number;i++)
   {
    SegmentX *segmentx=&segmentsx->xdata[segmentsx->n1data[i]];

    if(segmentx->node1==segmentx->node2)
      {
       loop++;
       segmentsx->n1data[i]=NO_SEGMENT;
      }
    else if(i && segmentx->node1==prevsegmentx->node1 &&
                 segmentx->node2==prevsegmentx->node2)
      {
       duplicate++;
       segmentsx->n1data[i-1]=NO_SEGMENT;
      }
    else if(IndexNodeX(nodesx,segmentx->node1)==NO_NODE ||
            IndexNodeX(nodesx,segmentx->node2)==NO_NODE)
      {
       missing++;
       segmentsx->n1data[i]=NO_SEGMENT;
      }

    prevsegmentx=segmentx;

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
 index_t i=0;
 int fd;
 SegmentX segmentx;

 printf("Measuring Segments: Segments=0");
 fflush(stdout);

 if(option_slim)
    nodesx->xdata=MapFile(nodesx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFile(segmentsx->filename);
 SeekFile(segmentsx->fd,0);

 while(!ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX)))
   {
    index_t index1=IndexNodeX(nodesx,segmentx.node1);
    index_t index2=IndexNodeX(nodesx,segmentx.node2);

    if(index1!=NO_NODE && index2!=NO_NODE)
      {
       NodeX *nodex1=&nodesx->xdata[IndexNodeX(nodesx,segmentx.node1)];
       NodeX *nodex2=&nodesx->xdata[IndexNodeX(nodesx,segmentx.node2)];

       /* Set the distance but preserve the ONEWAY_* flags */

       segmentx.distance|=DISTANCE(DistanceX(nodex1,nodex2));
      }

    WriteFile(fd,&segmentx,sizeof(SegmentX));

    i++;

    if(!(i%10000))
      {
       printf("\rMeasuring Segments: Segments=%d",i);
       fflush(stdout);
      }
   }

 CloseFile(segmentsx->fd);
 CloseFile(fd);

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 if(!option_slim)
   {
    UnmapFile(segmentsx->filename);
    segmentsx->xdata=MapFile(segmentsx->filename);
   }

 if(option_slim)
    nodesx->xdata=UnmapFile(nodesx->filename);

 printf("\rMeasured Segments: Segments=%d \n",segmentsx->xnumber);
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
 SegmentX *prevsegmentx;

 assert(segmentsx->n1data);     /* Must have n1data filled in => sorted by node 1 */
 assert(segmentsx->xdata);      /* Must have xdata filled in => mapped from file */

 printf("Deduplicating Segments: Segments=0 Duplicate=0");
 fflush(stdout);

 prevsegmentx=&segmentsx->xdata[segmentsx->n1data[0]];

 for(i=1;i<segmentsx->number;i++)
   {
    SegmentX *segmentx=&segmentsx->xdata[segmentsx->n1data[i]];

    if(segmentx->node1==prevsegmentx->node1 &&
       segmentx->node2==prevsegmentx->node2 &&
       DISTFLAG(segmentx->distance)==DISTFLAG(prevsegmentx->distance))
      {
       WayX *wayx1=FindWayX(waysx,prevsegmentx->way);
       WayX *wayx2=FindWayX(waysx,    segmentx->way);

       if(!WaysCompare(wayx1->way,wayx2->way))
         {
          segmentsx->n1data[i-1]=NO_SEGMENT;

          duplicate++;
         }
      }

    prevsegmentx=segmentx;

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

 assert(segmentsx->n1data);     /* Must have n1data filled in => sorted by node 1 */
 assert(!segmentsx->sdata);     /* Must not have sdata filled in => no real segments */

 printf("Creating Real Segments: Segments=0");
 fflush(stdout);

 /* Allocate the memory */

 segmentsx->sdata=(Segment*)malloc(segmentsx->number*sizeof(Segment));

 assert(segmentsx->sdata); /* Check malloc() worked */

 /* Loop through and allocate. */

 for(i=0;i<segmentsx->number;i++)
   {
    SegmentX *segmentx=LookupSegmentX(segmentsx,segmentsx->n1data[i]);
    WayX *wayx=FindWayX(waysx,segmentx->way);

    segmentsx->sdata[i].node1=0;
    segmentsx->sdata[i].node2=0;
    segmentsx->sdata[i].next2=NO_NODE;
    segmentsx->sdata[i].way=IndexWayInWaysX(waysx,wayx);
    segmentsx->sdata[i].distance=segmentx->distance;

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

 assert(segmentsx->n1data);     /* Must have n1data filled in => sorted by node 1 */
 assert(segmentsx->sdata);      /* Must have sdata filled in => real segments */
 assert(nodesx->gdata);         /* Must have gdata filled in => sorted geographically */

 printf("Indexing Nodes: Nodes=0");
 fflush(stdout);

 /* Index the segments */

 for(i=0;i<nodesx->number;i++)
   {
    Node   *node =&nodesx->ndata[nodesx->gdata[i]];
    index_t index=SEGMENT(node->firstseg);

    do
      {
       SegmentX *segmentx=LookupSegmentX(segmentsx,segmentsx->n1data[index]);

       if(segmentx->node1==nodesx->idata[nodesx->gdata[i]])
         {
          segmentsx->sdata[index].node1=i;

          index++;

          if(index>=segmentsx->number)
             break;

          segmentx=LookupSegmentX(segmentsx,segmentsx->n1data[index]);

          if(segmentx->node1!=nodesx->idata[nodesx->gdata[i]])
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
