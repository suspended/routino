/***************************************
 $Header: /home/amb/CVS/routino/src/segments.c,v 1.23 2009-01-29 19:31:52 amb Exp $

 Segment data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "nodes.h"
#include "segments.h"
#include "functions.h"


/* Functions */

static int sort_by_id(SegmentEx *a,SegmentEx *b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new segment list.

  SegmentsMem *NewSegmentList Returns the segment list.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsMem *NewSegmentList(void)
{
 SegmentsMem *segmentsmem;

 segmentsmem=(SegmentsMem*)malloc(sizeof(SegmentsMem));

 segmentsmem->sorted=0;
 segmentsmem->alloced=INCREMENT_SEGMENTS;
 segmentsmem->number=0;

 segmentsmem->xdata=(SegmentEx*)malloc(segmentsmem->alloced*sizeof(SegmentEx));

 return(segmentsmem);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a segment list.

  SegmentsMem *segmentsmem The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeSegmentList(SegmentsMem *segmentsmem)
{
 free(segmentsmem->xdata);
 free(segmentsmem);
}


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
  Save the segment list to a file.

  SegmentsMem* segmentsmem The set of segments to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveSegmentList(SegmentsMem* segmentsmem,const char *filename)
{
 int i;
 int fd;
 Segments *segments=calloc(1,sizeof(Segments));

 assert(segmentsmem->sorted);      /* Must be sorted */

 /* Fill in a Segments structure with the offset of the real data in the file after
    the Segment structure itself. */

 segments->number=segmentsmem->number;
 segments->data=NULL;
 segments->segments=(void*)sizeof(Segments);

 /* Write out the Segments structure and then the real data. */

 fd=OpenFile(filename);

 write(fd,segments,sizeof(Segments));

 for(i=0;i<segmentsmem->number;i++)
    write(fd,&segmentsmem->xdata[i].segment,sizeof(Segment));

 close(fd);

 /* Free the fake Segments */

 free(segments);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment with a particular starting node.

  SegmentEx *FindFirstSegment Returns the first extended segment with the specified id.

  SegmentsMem* segmentsmem The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentEx *FindFirstSegment(SegmentsMem* segmentsmem,node_t node)
{
 int start=0;
 int end=segmentsmem->number-1;
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
 else if(node<segmentsmem->xdata[start].node1) /* Check key is not before start */
    return(NULL);
 else if(node>segmentsmem->xdata[end].node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                          /* Choose mid point */

       if(segmentsmem->xdata[mid].node1<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsmem->xdata[mid].node1>node) /* Mid point is too high */
          end=mid;
       else                                        /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsmem->xdata[start].node1==node)      /* Start is correct */
         {found=start; goto found;}

    if(segmentsmem->xdata[end].node1==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segmentsmem->xdata[found-1].node1==node)
    found--;

 return(&segmentsmem->xdata[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  SegmentEx *NextSegment Returns a pointer to the next segment with the same id.

  SegmentsMem* segments The set of segments to process.

  SegmentEx *segmentex The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentEx *FindNextSegment(SegmentsMem* segmentsmem,SegmentEx *segmentex)
{
 SegmentEx *next=segmentex+1;

 if(IndexSegmentEx(segmentsmem,next)==segmentsmem->number)
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

  SegmentEx *AppendSegment Returns the appended segment.

  SegmentsMem* segmentsmem The set of segments to process.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.

  uint32_t wayindex The index of the way that the pair of segments are connected by.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentEx *AppendSegment(SegmentsMem* segmentsmem,node_t node1,node_t node2,uint32_t wayindex)
{
 /* Check that the array has enough space. */

 if(segmentsmem->number==segmentsmem->alloced)
   {
    segmentsmem->alloced+=INCREMENT_SEGMENTS;

    segmentsmem->xdata=(SegmentEx*)realloc((void*)segmentsmem->xdata,segmentsmem->alloced*sizeof(SegmentEx));
   }

 /* Insert the segment */

 segmentsmem->xdata[segmentsmem->number].node1=node1;
 segmentsmem->xdata[segmentsmem->number].node2=node2;
 segmentsmem->xdata[segmentsmem->number].segment.wayindex=wayindex;
 segmentsmem->xdata[segmentsmem->number].segment.distance=0;

 segmentsmem->number++;

 segmentsmem->sorted=0;

 return(&segmentsmem->xdata[segmentsmem->number-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.

  SegmentsMem* segmentsmem The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(SegmentsMem* segmentsmem)
{
 qsort(segmentsmem->xdata,segmentsmem->number,sizeof(SegmentEx),(int (*)(const void*,const void*))sort_by_id);

 while(segmentsmem->xdata[segmentsmem->number-1].node1==~0)
    segmentsmem->number--;

 segmentsmem->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order.

  int sort_by_id Returns the comparison of the node fields.

  SegmentEx *a The first Segment.

  SegmentEx *b The second Segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(SegmentEx *a,SegmentEx *b)
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

  SegmentsMem *segmentsmem The segments to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveBadSegments(SegmentsMem *segmentsmem)
{
 int i;
 int duplicate=0,loop=0;
 node_t node1=~0,node2=~0;

 for(i=0;i<segmentsmem->number;i++)
   {
    if(segmentsmem->xdata[i].node1==node1 && segmentsmem->xdata[i].node2==node2)
      {
       duplicate++;
       segmentsmem->xdata[i].node1=~0;
      }
    else if(segmentsmem->xdata[i].node1==segmentsmem->xdata[i].node2)
      {
       loop++;
       segmentsmem->xdata[i].node1=~0;
      }

    node1=segmentsmem->xdata[i].node1;
    node2=segmentsmem->xdata[i].node2;

    if(!((i+1)%10000))
      {
       printf("\rChecking: Segments=%d Duplicate=%d Loop=%d",i+1,duplicate,loop);
       fflush(stdout);
      }
   }

 printf("\rChecked: Segments=%d Duplicate=%d Loop=%d  \n",segmentsmem->number,duplicate,loop);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Measure the segments.

  SegmentsMem* segmentsmem The set of segments to process.

  NodesMem *nodesmem The list of nodes to use.
  ++++++++++++++++++++++++++++++++++++++*/

void MeasureSegments(SegmentsMem* segmentsmem,NodesMem *nodesmem)
{
 int i;

 assert(segmentsmem->sorted);   /* Must be sorted */

 for(i=0;i<segmentsmem->number;i++)
   {
    NodeEx *node1=FindNode(nodesmem,segmentsmem->xdata[i].node1);
    NodeEx *node2=FindNode(nodesmem,segmentsmem->xdata[i].node2);

    /* Set the distance but preserve the ONEWAY_OPPOSITE flag */

    segmentsmem->xdata[i].segment.distance|=Distance(&node1->node,&node2->node);

    if(!((i+1)%10000))
      {
       printf("\rMeasuring Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rMeasured Segments: Segments=%d \n",segmentsmem->number);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Fix the segment indexes to the nodes.

  SegmentsMem* segmentsmem The set of segments to process.

  NodesMem *nodesmem The list of nodes to use.

  SegmentsMem* supersegmentsmem The set of super-segments to append.
  ++++++++++++++++++++++++++++++++++++++*/

void FixupSegments(SegmentsMem* segmentsmem,NodesMem *nodesmem,SegmentsMem* supersegmentsmem)
{
 int i,j,n;

 assert(segmentsmem->sorted);   /* Must be sorted */
 assert(supersegmentsmem->sorted);   /* Must be sorted */

 for(i=0;i<segmentsmem->number;i++)
   {
    NodeEx *node1=FindNode(nodesmem,segmentsmem->xdata[i].node1);
    NodeEx *node2=FindNode(nodesmem,segmentsmem->xdata[i].node2);

    segmentsmem->xdata[i].segment.node1=IndexNodeEx(nodesmem,node1);
    segmentsmem->xdata[i].segment.node2=IndexNodeEx(nodesmem,node2);

    if(!((i+1)%10000))
      {
       printf("\rFixing Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rFixed Segments: Segments=%d \n",segmentsmem->number);
 fflush(stdout);

 for(i=0;i<supersegmentsmem->number;i++)
   {
    NodeEx *node1=FindNode(nodesmem,supersegmentsmem->xdata[i].node1);
    NodeEx *node2=FindNode(nodesmem,supersegmentsmem->xdata[i].node2);

    supersegmentsmem->xdata[i].segment.node1=IndexNodeEx(nodesmem,node1)|SUPER_SEGMENT;
    supersegmentsmem->xdata[i].segment.node2=IndexNodeEx(nodesmem,node2)|SUPER_SEGMENT;

    if(!((i+1)%10000))
      {
       printf("\rFixing Super-Segments: Super-Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rFixed Super-Segments: Super-Segments=%d \n",supersegmentsmem->number);
 fflush(stdout);

 n=segmentsmem->number;

 for(i=0,j=0;i<n;i++)
   {
    //    printf("segment %d %d\n",segmentsmem->xdata[i].node1,segmentsmem->xdata[i].node2);

    while(j<supersegmentsmem->number)
      {
       //       printf("supersegment %d %d\n",supersegmentsmem->xdata[j].node1,supersegmentsmem->xdata[j].node2);

       if(segmentsmem->xdata[i].node1==supersegmentsmem->xdata[j].node1 &&
          segmentsmem->xdata[i].node2==supersegmentsmem->xdata[j].node2)
         {
          //printf("%d %d\n",supersegmentsmem->xdata[j].node1,supersegmentsmem->xdata[j].node2);
          //printf("skipped segment %d -> %d\n",segmentsmem->xdata[i].node1,segmentsmem->xdata[i].node2);
          j++;
          break;
         }
       else if(segmentsmem->xdata[i].node1==supersegmentsmem->xdata[j].node1 &&
               segmentsmem->xdata[i].node2>supersegmentsmem->xdata[j].node2)
         {
          SegmentEx *supersegmentex=AppendSegment(segmentsmem,supersegmentsmem->xdata[j].node1,supersegmentsmem->xdata[j].node2,supersegmentsmem->xdata[j].segment.wayindex);

          //printf("%d %d\n",supersegmentsmem->xdata[j].node1,supersegmentsmem->xdata[j].node2);
          //printf("append1 %d segment %d -> %d\n",j,supersegmentsmem->xdata[j].node1,supersegmentsmem->xdata[j].node2);

          supersegmentex->segment.node1=supersegmentsmem->xdata[j].segment.node1;
          supersegmentex->segment.node2=supersegmentsmem->xdata[j].segment.node2;
         }
       else if(segmentsmem->xdata[i].node1>supersegmentsmem->xdata[j].node1)
         {
          SegmentEx *supersegmentex=AppendSegment(segmentsmem,supersegmentsmem->xdata[j].node1,supersegmentsmem->xdata[j].node2,supersegmentsmem->xdata[j].segment.wayindex);

          //printf("%d %d\n",supersegmentsmem->xdata[j].node1,supersegmentsmem->xdata[j].node2);
          //printf("append2 %d segment %d -> %d\n",j,supersegmentsmem->xdata[j].node1,supersegmentsmem->xdata[j].node2);

          supersegmentex->segment.node1=supersegmentsmem->xdata[j].segment.node1;
          supersegmentex->segment.node2=supersegmentsmem->xdata[j].segment.node2;
         }
       else
          break;

       j++;
      }

    if(!((i+1)%10000))
      {
       printf("\rMerging Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 for(i=0;i<segmentsmem->number;i++)
   {
    segmentsmem->xdata[i].segment.xnode1=segmentsmem->xdata[i].node1;
    segmentsmem->xdata[i].segment.xnode2=segmentsmem->xdata[i].node2;
   }

 printf("\rMerged Segments: Segments=%d \n",segmentsmem->number);
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
