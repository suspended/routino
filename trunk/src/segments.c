/***************************************
 $Header: /home/amb/CVS/routino/src/segments.c,v 1.26 2009-02-01 17:11:08 amb Exp $

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
#include <stdlib.h>

#include "nodes.h"
#include "segments.h"
#include "functions.h"


/* Constants */

/*+ The array size increment for segments - expect ~8,000,000 segments. +*/
#define INCREMENT_SEGMENTS 1024*1024


/* Functions */

static int sort_by_id(SegmentX *a,SegmentX *b);


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

 /* Copy the Segments structure from the loaded data */

 *segments=*((Segments*)data);

 /* Adjust the pointers in the Segments structure. */

 segments->data =data;
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
 segmentsx->number=0;

 segmentsx->xdata=(SegmentX*)malloc(segmentsx->alloced*sizeof(SegmentX));

 return(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a segment list.

  SegmentsX *segmentsx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeSegmentList(SegmentsX *segmentsx)
{
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
 int i;
 int fd;
 Segments *segments=calloc(1,sizeof(Segments));

 assert(segmentsx->sorted);      /* Must be sorted */

 /* Fill in a Segments structure with the offset of the real data in the file after
    the Segment structure itself. */

 segments->number=segmentsx->number;
 segments->data=NULL;
 segments->segments=(void*)sizeof(Segments);

 /* Write out the Segments structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,segments,sizeof(Segments));

 for(i=0;i<segmentsx->number;i++)
    WriteFile(fd,&segmentsx->xdata[i].segment,sizeof(Segment));

 CloseFile(fd);

 /* Free the fake Segments */

 free(segments);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment with a particular starting node.

  SegmentX *FindFirstSegment Returns the first extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX *FindFirstSegment(SegmentsX* segmentsx,node_t node)
{
 int start=0;
 int end=segmentsx->number-1;
 int mid;
 int found;

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
 else if(node<segmentsx->xdata[start].node1) /* Check key is not before start */
    return(NULL);
 else if(node>segmentsx->xdata[end].node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                          /* Choose mid point */

       if(segmentsx->xdata[mid].node1<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsx->xdata[mid].node1>node) /* Mid point is too high */
          end=mid;
       else                                        /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsx->xdata[start].node1==node)      /* Start is correct */
         {found=start; goto found;}

    if(segmentsx->xdata[end].node1==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segmentsx->xdata[found-1].node1==node)
    found--;

 return(&segmentsx->xdata[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  SegmentX *NextSegment Returns a pointer to the next segment with the same id.

  SegmentsX* segments The set of segments to process.

  SegmentX *segmentex The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX *FindNextSegment(SegmentsX* segmentsx,SegmentX *segmentex)
{
 SegmentX *next=segmentex+1;

 if(IndexSegmentX(segmentsx,next)==segmentsx->number)
    return(NULL);

 if(next->node1==segmentex->node1)
    return(next);

 return(NULL);
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  Segment *NextSegment Returns a pointer to the next segment with the same id.

  Segments* segments The set of segments to process.

  Segment *segment The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *NextSegment(Segments* segments,Segment *segment)
{
 Segment *next=segment+1;

 if(IndexSegment(segments,next)==segments->number)
    return(NULL);

 if(NODE(next->node1)==NODE(segment->node1))
    return(next);

 return(NULL);
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Append a segment to a newly created segment list (unsorted).

  SegmentX *AppendSegment Returns the appended segment.

  SegmentsX* segmentsx The set of segments to process.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.

  index_t way The index of the way that the pair of segments are connected by.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX *AppendSegment(SegmentsX* segmentsx,node_t node1,node_t node2,index_t way)
{
 /* Check that the array has enough space. */

 if(segmentsx->number==segmentsx->alloced)
   {
    segmentsx->alloced+=INCREMENT_SEGMENTS;

    segmentsx->xdata=(SegmentX*)realloc((void*)segmentsx->xdata,segmentsx->alloced*sizeof(SegmentX));
   }

 /* Insert the segment */

 segmentsx->xdata[segmentsx->number].node1=node1;
 segmentsx->xdata[segmentsx->number].node2=node2;
 segmentsx->xdata[segmentsx->number].segment.way=way;
 segmentsx->xdata[segmentsx->number].segment.distance=0;

 segmentsx->number++;

 segmentsx->sorted=0;

 return(&segmentsx->xdata[segmentsx->number-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.

  SegmentsX* segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(SegmentsX* segmentsx)
{
 qsort(segmentsx->xdata,segmentsx->number,sizeof(SegmentX),(int (*)(const void*,const void*))sort_by_id);

 while(segmentsx->xdata[segmentsx->number-1].node1==~0)
    segmentsx->number--;

 segmentsx->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order.

  int sort_by_id Returns the comparison of the node fields.

  SegmentX *a The first Segment.

  SegmentX *b The second Segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(SegmentX *a,SegmentX *b)
{
 node_t a_id1=a->node1,a_id2=a->node2;
 node_t b_id1=b->node1,b_id2=b->node2;

 if(a_id1<b_id1)
    return(-1);
 else if(a_id1>b_id1)
    return(1);
 else /* if(a_id1==b_id1) */
   {
    if(a_id2<b_id2)
       return(-1);
    else
       return(1);
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
 node_t node1=~0,node2=~0;

 for(i=0;i<segmentsx->number;i++)
   {
    if(segmentsx->xdata[i].node1==node1 && segmentsx->xdata[i].node2==node2)
      {
       duplicate++;
       segmentsx->xdata[i].node1=~0;
      }
    else if(segmentsx->xdata[i].node1==segmentsx->xdata[i].node2)
      {
       loop++;
       segmentsx->xdata[i].node1=~0;
      }

    node1=segmentsx->xdata[i].node1;
    node2=segmentsx->xdata[i].node2;

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

 assert(segmentsx->sorted);   /* Must be sorted */

 for(i=0;i<segmentsx->number;i++)
   {
    NodeX *node1=FindNode(nodesx,segmentsx->xdata[i].node1);
    NodeX *node2=FindNode(nodesx,segmentsx->xdata[i].node2);

    /* Set the distance but preserve the ONEWAY_OPPOSITE flag */

    segmentsx->xdata[i].segment.distance|=Distance(&node1->node,&node2->node);

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
  Fix the segment indexes to the nodes.

  SegmentsX* segmentsx The set of segments to process.

  NodesX *nodesx The list of nodes to use.

  SegmentsX* supersegmentsx The set of super-segments to append.
  ++++++++++++++++++++++++++++++++++++++*/

void FixupSegments(SegmentsX* segmentsx,NodesX *nodesx,SegmentsX* supersegmentsx)
{
 int i,j,n;

 assert(segmentsx->sorted);   /* Must be sorted */
 assert(supersegmentsx->sorted);   /* Must be sorted */

 for(i=0;i<segmentsx->number;i++)
   {
    NodeX *node1=FindNode(nodesx,segmentsx->xdata[i].node1);
    NodeX *node2=FindNode(nodesx,segmentsx->xdata[i].node2);

    segmentsx->xdata[i].segment.node1=IndexNodeX(nodesx,node1)|SUPER_FLAG;
    segmentsx->xdata[i].segment.node2=IndexNodeX(nodesx,node2);

    if(!((i+1)%10000))
      {
       printf("\rFixing Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rFixed Segments: Segments=%d \n",segmentsx->number);
 fflush(stdout);

 for(i=0;i<supersegmentsx->number;i++)
   {
    NodeX *node1=FindNode(nodesx,supersegmentsx->xdata[i].node1);
    NodeX *node2=FindNode(nodesx,supersegmentsx->xdata[i].node2);

    supersegmentsx->xdata[i].segment.node1=IndexNodeX(nodesx,node1);
    supersegmentsx->xdata[i].segment.node2=IndexNodeX(nodesx,node2)|SUPER_FLAG;

    if(!((i+1)%10000))
      {
       printf("\rFixing Super-Segments: Super-Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rFixed Super-Segments: Super-Segments=%d \n",supersegmentsx->number);
 fflush(stdout);

 n=segmentsx->number;

 for(i=0,j=0;i<n;i++)
   {
    while(j<supersegmentsx->number)
      {
       if(segmentsx->xdata[i].node1==supersegmentsx->xdata[j].node1 &&
          segmentsx->xdata[i].node2==supersegmentsx->xdata[j].node2)
         {
          segmentsx->xdata[i].segment.node2|=SUPER_FLAG;
          j++;
          break;
         }
       else if(segmentsx->xdata[i].node1==supersegmentsx->xdata[j].node1 &&
               segmentsx->xdata[i].node2>supersegmentsx->xdata[j].node2)
         {
          SegmentX *supersegmentex=AppendSegment(segmentsx,supersegmentsx->xdata[j].node1,supersegmentsx->xdata[j].node2,supersegmentsx->xdata[j].segment.way);

          supersegmentex->segment.node1=supersegmentsx->xdata[j].segment.node1;
          supersegmentex->segment.node2=supersegmentsx->xdata[j].segment.node2;
          supersegmentex->segment.distance=supersegmentsx->xdata[j].segment.distance;
         }
       else if(segmentsx->xdata[i].node1>supersegmentsx->xdata[j].node1)
         {
          SegmentX *supersegmentex=AppendSegment(segmentsx,supersegmentsx->xdata[j].node1,supersegmentsx->xdata[j].node2,supersegmentsx->xdata[j].segment.way);

          supersegmentex->segment.node1=supersegmentsx->xdata[j].segment.node1;
          supersegmentex->segment.node2=supersegmentsx->xdata[j].segment.node2;
          supersegmentex->segment.distance=supersegmentsx->xdata[j].segment.distance;
         }
       else
          break;

       j++;
      }

    if(!((i+1)%10000))
      {
       printf("\rMerging Segments: Segments=%d Super-Segment=%d Total=%d",i+1,j+1,segmentsx->number);
       fflush(stdout);
      }
   }

 printf("\rMerged Segments: Segments=%d Super-Segment=%d Total=%d \n",n,supersegmentsx->number,segmentsx->number);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Calculate the distance between two nodes.

  distance_t Distance Returns the distance between the nodes.

  Node *node1 The starting node.

  Node *node2 The end node.
  ++++++++++++++++++++++++++++++++++++++*/

distance_t Distance(Node *node1,Node *node2)
{
 double radiant = M_PI / 180;

 double dlon = radiant * (node1->longitude - node2->longitude);
 double dlat = radiant * (node1->latitude  - node2->latitude);

 double a1,a2,a,sa,c,d;

 if(dlon==0 && dlat==0)
   return 0;

 a1 = sin (dlat / 2);
 a2 = sin (dlon / 2);
 a = (a1 * a1) + cos (node1->latitude * radiant) * cos (node2->latitude * radiant) * a2 * a2;
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
