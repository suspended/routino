/***************************************
 $Header: /home/amb/CVS/routino/src/segments.c,v 1.16 2009-01-23 16:09:08 amb Exp $

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


/* Functions */

static int sort_by_id(Segment *a,Segment *b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new segment list.

  SegmentsMem *NewSegmentList Returns the segment list.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsMem *NewSegmentList(void)
{
 SegmentsMem *segments;

 segments=(SegmentsMem*)malloc(sizeof(SegmentsMem));

 segments->alloced=INCREMENT_SEGMENTS;
 segments->number=0;
 segments->sorted=0;

 segments->segments=(Segments*)malloc(sizeof(Segments)+segments->alloced*sizeof(Segment));

 return(segments);
}


/*++++++++++++++++++++++++++++++++++++++
  Load in a segment list from a file.

  Segments* SaveSegmentList Returns the segment list that has just been loaded.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Segments *LoadSegmentList(const char *filename)
{
 return((Segments*)MapFile(filename));
}


/*++++++++++++++++++++++++++++++++++++++
  Save the segment list to a file.

  Segments* SaveSegmentList Returns the segment list that has just been saved.

  SegmentsMem* segments The set of segments to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

Segments *SaveSegmentList(SegmentsMem* segments,const char *filename)
{
 assert(segments->sorted);      /* Must be sorted */

 if(WriteFile(filename,(void*)segments->segments,sizeof(Segments)-sizeof(segments->segments->segments)+segments->number*sizeof(Segment)))
    assert(0);

 free(segments->segments);
 free(segments);

 return(LoadSegmentList(filename));
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment with a particular starting node.

  Segment *FindFirstSegment Returns a pointer to the first segment with the specified id.

  Segments* segments The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *FindFirstSegment(Segments* segments,node_t node)
{
 int bin=node%NBINS_SEGMENTS;
 int start=segments->offset[bin];
 int end=segments->offset[bin+1]-1; /* &offset[NBINS+1] == &number */
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
 else if(node<segments->segments[start].node1) /* Check key is not before start */
    return(NULL);
 else if(node>segments->segments[end].node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                          /* Choose mid point */

       if(segments->segments[mid].node1<node)      /* Mid point is too low */
          start=mid;
       else if(segments->segments[mid].node1>node) /* Mid point is too high */
          end=mid;
       else                                        /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segments->segments[start].node1==node)      /* Start is correct */
         {found=start; goto found;}

    if(segments->segments[end].node1==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && segments->segments[found-1].node1==node)
    found--;

 return(&segments->segments[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  Segment *FindNextSegment Returns a pointer to the next segment with the same id.

  Segments* segments The set of segments to process.

  Segment *segment The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *FindNextSegment(Segments* segments,Segment *segment)
{
 Segment *next=segment+1;

 if((next-segments->segments)==segments->number)
    return(NULL);

 if(next->node1==segment->node1)
    return(next);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a segment to a newly created segment list (unsorted).

  Segment *AppendSegment Returns the appended segment.

  SegmentsMem* segments The set of segments to process.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.

  way_t way The way that the pair of segments are connected by.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *AppendSegment(SegmentsMem* segments,node_t node1,node_t node2,way_t way)
{
 /* Check that the array has enough space. */

 if(segments->number==segments->alloced)
   {
    segments->alloced+=INCREMENT_SEGMENTS;

    segments->segments=(Segments*)realloc((void*)segments->segments,sizeof(Segments)+segments->alloced*sizeof(Segment));
   }

 /* Insert the segment */

 segments->segments->segments[segments->number].node1=node1;
 segments->segments->segments[segments->number].node2=node2;
 segments->segments->segments[segments->number].way=way;
 segments->segments->segments[segments->number].distance=0;

 segments->number++;

 segments->sorted=0;

 return(&segments->segments->segments[segments->number-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.

  SegmentsMem* segments The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(SegmentsMem* segments)
{
 int i,bin=0;

 qsort(segments->segments->segments,segments->number,sizeof(Segment),(int (*)(const void*,const void*))sort_by_id);

 while(segments->segments->segments[segments->number-1].node1==~0)
    segments->number--;

 segments->sorted=1;

 /* Make it searchable */

 segments->segments->number=segments->number;

 for(i=0;i<segments->number;i++)
    for(;bin<=(segments->segments->segments[i].node1%NBINS_SEGMENTS);bin++)
       segments->segments->offset[bin]=i;

 for(;bin<NBINS_SEGMENTS;bin++)
    segments->segments->offset[bin]=segments->number;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order.

  int sort_by_id Returns the comparison of the node fields.

  Segment *a The first Segment.

  Segment *b The second Segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(Segment *a,Segment *b)
{
 node_t a_id1=a->node1,a_id2=a->node2;
 node_t b_id1=b->node1,b_id2=b->node2;

 int a_bin=a->node1%NBINS_SEGMENTS;
 int b_bin=b->node1%NBINS_SEGMENTS;

 if(a_bin!=b_bin)
    return(a_bin-b_bin);

 if(a_id1<b_id1)
    return(-1);
 else if(a_id1>b_id1)
    return(1);
 else /* if(a_id1==b_id1) */
   {
    if(a_id2<b_id2)
       return(-1);
    else if(a_id2>b_id2)
       return(1);
    else
       return(0);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Remove bad segments (zero length or duplicated).

  SegmentsMem *segments The segments to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveBadSegments(SegmentsMem *segments)
{
 int i;
 int duplicate=0,loop=0;
 node_t node1=~0,node2=~0;

 for(i=0;i<segments->number;i++)
   {
    Segment *segment=&segments->segments->segments[i];

    if(segment->node1==node1 && segment->node2==node2)
      {
       duplicate++;
       segment->node1=~0;
      }
    else if(segment->node1==segment->node2)
      {
       loop++;
       segment->node1=~0;
      }

    node1=segment->node1;
    node2=segment->node2;

    if(!((i+1)%10000))
      {
       printf("\rChecking: Segments=%d Duplicate=%d Loop=%d",i+1,duplicate,loop);
       fflush(stdout);
      }
   }

 printf("\rChecked: Segments=%d Duplicate=%d Loop=%d  \n",segments->number,duplicate,loop);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Assign the lengths and durations to all of the segments.

  SegmentsMem* segments The set of segments to process.

  Nodes *nodes The list of nodes to use.

  Ways *ways The list of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void FixupSegmentLengths(SegmentsMem* segments,Nodes *nodes,Ways *ways)
{
 int i;

 assert(segments->sorted);      /* Must be sorted */

 for(i=0;i<segments->number;i++)
   {
    Node *node1=FindNode(nodes,segments->segments->segments[i].node1);
    Node *node2=FindNode(nodes,segments->segments->segments[i].node2);

    /* Set the distance but preserve the INVALID_DISTANCE flag */

    segments->segments->segments[i].distance|=Distance(node1,node2);

    if(!((i+1)%10000))
      {
       printf("\rMeasuring Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rMeasured Segments: Segments=%d \n",segments->number);
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

  Transport transport The type of transport being used.
  ++++++++++++++++++++++++++++++++++++++*/

duration_t Duration(Segment *segment,Way *way,Transport transport)
{
 speed_t    speed=WaySpeed(way);
 distance_t distance=segment->distance;

 if(transport==Transport_Foot)
    speed=5;
 else if(transport==Transport_Horse)
    speed=8;
 else if(transport==Transport_Bicycle)
    speed=20;

 return hours_to_duration(distance_to_km(distance)/speed);
}
