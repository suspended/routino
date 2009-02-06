/***************************************
 $Header: /home/amb/CVS/routino/src/segments.c,v 1.30 2009-02-06 20:23:33 amb Exp $

 Segment data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "nodes.h"
#include "segments.h"
#include "functions.h"


/* Constants */

/*+ The array size increment for segments - expect ~8,000,000 segments. +*/
#define INCREMENT_SEGMENTS 1024*1024


/* Functions */

static int sort_by_id(SegmentX **a,SegmentX **b);


/*++++++++++++++++++++++++++++++++++++++
  Load in a segment list from a file.

  Segments* SaveSegmentList Returns the segment list that has just been loaded.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Segments *LoadSegmentList(const char *filename)
{
 void *data;
 Segments *segments;

 segments=(Segments*)malloc(sizeof(Segments));

 data=MapFile(filename);

 if(!data)
    return(NULL);

 /* Copy the Segments structure from the loaded data */

 *segments=*((Segments*)data);

 /* Adjust the pointers in the Segments structure. */

 segments->data=data;
 segments->segments=(Segment*)(data+(off_t)segments->segments);

 return(segments);
}


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new segment list.

  SegmentsX *NewSegmentList Returns the segment list.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *NewSegmentList(void)
{
 SegmentsX *segmentsx;

 segmentsx=(SegmentsX*)malloc(sizeof(SegmentsX));

 segmentsx->sorted=0;
 segmentsx->alloced=INCREMENT_SEGMENTS;
 segmentsx->xnumber=0;

 segmentsx->xdata=(SegmentX*)malloc(segmentsx->alloced*sizeof(SegmentX));
 segmentsx->sdata=NULL;

 return(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a segment list.

  SegmentsX *segmentsx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeSegmentList(SegmentsX *segmentsx)
{
 free(segmentsx->xdata);
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
 int i;
 int fd;
 Segments *segments=calloc(1,sizeof(Segments));

 assert(segmentsx->sorted);     /* Must be sorted */

 /* Fill in a Segments structure with the offset of the real data in the file after
    the Segment structure itself. */

 segments->number=segmentsx->number;
 segments->data=NULL;
 segments->segments=(void*)sizeof(Segments);

 /* Write out the Segments structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,segments,sizeof(Segments));

 for(i=0;i<segmentsx->number;i++)
    WriteFile(fd,&segmentsx->sdata[i]->segment,sizeof(Segment));

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
 else if(node<segmentsx->sdata[start]->node1) /* Check key is not before start */
    return(NULL);
 else if(node>segmentsx->sdata[end]->node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                         /* Choose mid point */

       if(segmentsx->sdata[mid]->node1<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsx->sdata[mid]->node1>node) /* Mid point is too high */
          end=mid;
       else                                       /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsx->sdata[start]->node1==node)      /* Start is correct */
         {found=start; goto found;}

    if(segmentsx->sdata[end]->node1==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segmentsx->sdata[found-1]->node1==node)
    found--;

 return(&segmentsx->sdata[found]);
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

 if((next-segmentsx->sdata)==segmentsx->number)
    return(NULL);

 if((*next)->node1==(*segmentx)->node1)
    return(next);

 return(NULL);
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  Segment *NextSegment Returns a pointer to the next segment with the same id.

  Segments* segments The set of segments to process.

  Segment *segment The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *NextSegment(Segments* segments,Segment *segment,index_t node)
{
 index_t next;

 if(NODE(segment->node1)==node)
    next=segment->next1;
 else
    next=segment->next2;

 if(next==~0)
    return(NULL);
 else
    return(LookupSegment(segments,next));
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Append a segment to a segment list.

  Segment *AppendSegment Returns the appended segment.

  SegmentsX* segmentsx The set of segments to process.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *AppendSegment(SegmentsX* segmentsx,node_t node1,node_t node2)
{
 /* Check that the array has enough space. */

 if(segmentsx->xnumber==segmentsx->alloced)
   {
    segmentsx->alloced+=INCREMENT_SEGMENTS;

    segmentsx->xdata=(SegmentX*)realloc((void*)segmentsx->xdata,segmentsx->alloced*sizeof(SegmentX));
   }

 /* Insert the segment */

 segmentsx->xdata[segmentsx->xnumber].node1=node1;
 segmentsx->xdata[segmentsx->xnumber].node2=node2;

 memset(&segmentsx->xdata[segmentsx->xnumber].segment,0,sizeof(Segment));

 segmentsx->xnumber++;

 segmentsx->sorted=0;

 return(&segmentsx->xdata[segmentsx->xnumber-1].segment);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.

  SegmentsX* segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(SegmentsX* segmentsx)
{
 int i;

 /* Allocate the arrays of pointers */

 if(segmentsx->sorted)
    segmentsx->sdata=realloc(segmentsx->sdata,segmentsx->xnumber*sizeof(SegmentX*));
 else
    segmentsx->sdata=malloc(segmentsx->xnumber*sizeof(SegmentX*));

 segmentsx->number=0;

 for(i=0;i<segmentsx->xnumber;i++)
    if(segmentsx->xdata[i].node1!=~0)
      {
       segmentsx->sdata[segmentsx->number]=&segmentsx->xdata[i];
       segmentsx->number++;
      }

 qsort(segmentsx->sdata,segmentsx->number,sizeof(SegmentX*),(int (*)(const void*,const void*))sort_by_id);

 segmentsx->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order.

  int sort_by_id Returns the comparison of the node fields.

  SegmentX **a The first Segment.

  SegmentX **b The second Segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(SegmentX **a,SegmentX **b)
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
       distance_t a_distance=(*a)->segment.distance;
       distance_t b_distance=(*b)->segment.distance;

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

  SegmentsX *segmentsx The segments to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveBadSegments(SegmentsX *segmentsx)
{
 int i;
 int duplicate=0,loop=0;

 assert(segmentsx->sorted);     /* Must be sorted */

 for(i=0;i<segmentsx->number;i++)
   {
    if(i && segmentsx->sdata[i]->node1==segmentsx->sdata[i-1]->node1 &&
            segmentsx->sdata[i]->node2==segmentsx->sdata[i-1]->node2)
      {
       duplicate++;
       segmentsx->sdata[i-1]->node1=~0;
      }
    else if(segmentsx->sdata[i]->node1==segmentsx->sdata[i]->node2)
      {
       loop++;
       segmentsx->sdata[i]->node1=~0;
      }

    if(!((i+1)%10000))
      {
       printf("\rChecking: Segments=%d Duplicate=%d Loop=%d",i+1,duplicate,loop);
       fflush(stdout);
      }
   }

 printf("\rChecked: Segments=%d Duplicate=%d Loop=%d  \n",segmentsx->number,duplicate,loop);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Measure the segments.

  SegmentsX* segmentsx The set of segments to process.

  NodesX *nodesx The list of nodes to use.
  ++++++++++++++++++++++++++++++++++++++*/

void MeasureSegments(SegmentsX* segmentsx,NodesX *nodesx)
{
 int i;

 assert(segmentsx->sorted);     /* Must be sorted */

 for(i=0;i<segmentsx->number;i++)
   {
    NodeX *node1=FindNodeX(nodesx,segmentsx->sdata[i]->node1);
    NodeX *node2=FindNodeX(nodesx,segmentsx->sdata[i]->node2);

    /* Set the distance but preserve the ONEWAY_* flags */

    segmentsx->sdata[i]->segment.distance|=DistanceX(node1,node2);

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

  NodesX *nodesx The list of nodes to use.
  ++++++++++++++++++++++++++++++++++++++*/

void RotateSegments(SegmentsX* segmentsx,NodesX *nodesx)
{
 int i,rotated=0;

 assert(segmentsx->sorted);     /* Must be sorted */

 for(i=0;i<segmentsx->number;i++)
   {
    if(segmentsx->sdata[i]->node1>segmentsx->sdata[i]->node2)
      {
       segmentsx->sdata[i]->node1^=segmentsx->sdata[i]->node2;
       segmentsx->sdata[i]->node2^=segmentsx->sdata[i]->node1;
       segmentsx->sdata[i]->node1^=segmentsx->sdata[i]->node2;

       if(segmentsx->sdata[i]->segment.distance&(ONEWAY_2TO1|ONEWAY_1TO2))
          segmentsx->sdata[i]->segment.distance^=ONEWAY_2TO1|ONEWAY_1TO2;

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

  NodesX *nodesx The list of nodes to use.

  WaysX *waysx The list of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void DeduplicateSegments(SegmentsX* segmentsx,NodesX *nodesx,WaysX *waysx)
{
 int i,duplicate=0;

 assert(segmentsx->sorted);     /* Must be sorted */

 for(i=1;i<segmentsx->number;i++)
   {
    if(segmentsx->sdata[i]->node1==segmentsx->sdata[i-1]->node1 &&
       segmentsx->sdata[i]->node2==segmentsx->sdata[i-1]->node2 &&
       segmentsx->sdata[i]->segment.node1==segmentsx->sdata[i-1]->segment.node1 &&
       segmentsx->sdata[i]->segment.node2==segmentsx->sdata[i-1]->segment.node2 &&
       segmentsx->sdata[i]->segment.distance==segmentsx->sdata[i-1]->segment.distance)
      {
       WayX *wayx1=LookupWayX(waysx,segmentsx->sdata[i-1]->segment.way);
       WayX *wayx2=LookupWayX(waysx,segmentsx->sdata[i]->segment.way);

       if(wayx1==wayx2 ||
          (wayx1->way.type==wayx2->way.type && wayx1->way.allow==wayx2->way.allow && wayx1->way.limit==wayx2->way.limit))
         {
          segmentsx->sdata[i-1]->node1=~0;
          segmentsx->sdata[i-1]->node2=~0;

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
  Assign the nodes indexes to the segments.

  SegmentsX* segmentsx The set of segments to process.

  NodesX *nodesx The list of nodes to use.
  ++++++++++++++++++++++++++++++++++++++*/

void IndexSegments(SegmentsX* segmentsx,NodesX *nodesx)
{
 int i;

 assert(segmentsx->sorted);     /* Must be sorted */
 assert(nodesx->sorted);        /* Must be sorted */

 /* Index the segments */

 for(i=0;i<nodesx->number;i++)
   {
    SegmentX *segmentx=LookupSegmentX(segmentsx,SEGMENT(nodesx->gdata[i]->node.firstseg));

    do
      {
       if(segmentx->node1==nodesx->gdata[i]->id)
         {
          segmentx->segment.node1|=i;

          if(segmentx->segment.next1==~0)
             segmentx=NULL;
          else
             segmentx=LookupSegmentX(segmentsx,segmentx->segment.next1);
         }
       else
         {
          segmentx->segment.node2|=i;

          if(segmentx->segment.next2==~0)
             segmentx=NULL;
          else
             segmentx=LookupSegmentX(segmentsx,segmentx->segment.next2);
         }
      }
    while(segmentx);

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
  Calculate the distance between two locations.

  float Distance Returns the distance between the locations.

  float lat1 The latitude of the first location.

  float lon1 The longitude of the first location.

  float lat2 The latitude of the second location.

  float lon2 The longitude of the second location.
  ++++++++++++++++++++++++++++++++++++++*/

float Distance(float lat1,float lon1,float lat2,float lon2)
{
 double radiant = M_PI / 180;

 double dlon = radiant * (lon1 - lon2);
 double dlat = radiant * (lat1 - lat2);

 double a1,a2,a,sa,c,d;

 if(dlon==0 && dlat==0)
   return 0;

 a1 = sin (dlat / 2);
 a2 = sin (dlon / 2);
 a = (a1 * a1) + cos (lat1 * radiant) * cos (lat2 * radiant) * a2 * a2;
 sa = sqrt (a);
 if (sa <= 1.0)
   {c = 2 * asin (sa);}
 else
   {c = 2 * asin (1.0);}
 d = 6378.137 * c;

 return d;
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the distance between two nodes.

  distance_t DistanceX Returns the distance between the extended nodes.

  NodeX *nodex1 The starting node.

  NodeX *nodex2 The end node.
  ++++++++++++++++++++++++++++++++++++++*/

distance_t DistanceX(NodeX *nodex1,NodeX *nodex2)
{
 double radiant = M_PI / 180;

 double dlon = radiant * (nodex1->longitude - nodex2->longitude);
 double dlat = radiant * (nodex1->latitude  - nodex2->latitude);

 double a1,a2,a,sa,c,d;

 if(dlon==0 && dlat==0)
   return 0;

 a1 = sin (dlat / 2);
 a2 = sin (dlon / 2);
 a = (a1 * a1) + cos (nodex1->latitude * radiant) * cos (nodex2->latitude * radiant) * a2 * a2;
 sa = sqrt (a);
 if (sa <= 1.0)
   {c = 2 * asin (sa);}
 else
   {c = 2 * asin (1.0);}
 d = 6378.137 * c;

 return km_to_distance(d);
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the duration of segment.

  duration_t Duration Returns the duration of travel between the nodes.

  Segment *segment The segment to traverse.

  Way *way The way that the segment belongs to.

  Profile *profile The profile of the transport being used.
  ++++++++++++++++++++++++++++++++++++++*/

duration_t Duration(Segment *segment,Way *way,Profile *profile)
{
 speed_t    speed=profile->speed[HIGHWAY(way->type)];
 distance_t distance=DISTANCE(segment->distance);

 return distance_speed_to_duration(distance,speed);
}
