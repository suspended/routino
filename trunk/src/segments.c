/***************************************
 $Header: /home/amb/CVS/routino/src/segments.c,v 1.7 2009-01-10 15:59:58 amb Exp $

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
#ifdef NBINS_SEGMENTS
 int i,bin=0;
#endif

 assert(segments->sorted);      /* Must be sorted */

 segments->segments->number=segments->number;

#ifdef NBINS_SEGMENTS
 for(i=0;i<segments->number;i++)
    for(;bin<=(segments->segments->segments[i].node1%NBINS_SEGMENTS);bin++)
       segments->segments->offset[bin]=i;

 for(;bin<=NBINS_SEGMENTS;bin++)
    segments->segments->offset[bin]=segments->number;
#endif

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
#ifdef NBINS_SEGMENTS
 int bin=node%NBINS_SEGMENTS;
 int start=segments->offset[bin];
 int end=segments->offset[bin+1]-1;
#else
 int start=0;
 int end=segments->number-1;
#endif
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

  SegmentsMem* segments The set of segments to process.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.

  way_t way The way that the pair of segments are connected by.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendSegment(SegmentsMem* segments,node_t node1,node_t node2,way_t way)
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
 segments->segments->segments[segments->number].duration=0;

 segments->number++;

 segments->sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.

  SegmentsMem* segments The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(SegmentsMem* segments)
{
 qsort(segments->segments->segments,segments->number,sizeof(Segment),(int (*)(const void*,const void*))sort_by_id);

 segments->sorted=1;
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

#ifdef NBINS_SEGMENTS
 int a_bin=a->node1%NBINS_SEGMENTS;
 int b_bin=b->node1%NBINS_SEGMENTS;

 if(a_bin!=b_bin)
    return(a_bin-b_bin);
#endif

 if(a_id1==b_id1)
    return(a_id2-b_id2);
 else
    return(a_id1-b_id1);
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
    speed_t    speed;
    distance_t distance;
    duration_t duration;
    Node *node1=FindNode(nodes,segments->segments->segments[i].node1);
    Node *node2=FindNode(nodes,segments->segments->segments[i].node2);
    Way  *way=FindWay(ways,segments->segments->segments[i].way);

    if(way->limit)
       speed=way->limit;
    else
       speed=way->speed;

    if(way->type&Way_NOTROUTABLE || Way_TYPE(way->type)>Way_HighestRoutable)
      {
       distance=(distance_short_t)~0;
       duration=(duration_short_t)~0;
      }
    else
      {
       distance=Distance(node1,node2);
       duration=hours_to_duration(distance_to_km(distance)/speed);

       if(distance>(distance_short_t)~0)
         {
          fprintf(stderr,"\nSegment too long (%d->%d) = %.1f km\n",node1->id,node2->id,distance_to_km(distance));
          distance=(distance_short_t)~0;
         }

       if(duration>(duration_short_t)~0)
         {
          fprintf(stderr,"\nSegment too long (%d->%d) = %.1f mins\n",node1->id,node2->id,duration_to_minutes(duration));
          duration=(duration_short_t)~0;
         }
      }

    segments->segments->segments[i].distance=distance;
    segments->segments->segments[i].duration=duration;
   }
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
