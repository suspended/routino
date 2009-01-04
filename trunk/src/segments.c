/***************************************
 $Header: /home/amb/CVS/routino/src/segments.c,v 1.4 2009-01-04 17:51:24 amb Exp $

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

#include "functions.h"
#include "types.h"

#define INCREMENT 64*1024

/*+ The list of segments +*/
Segments *OSMSegments=NULL;

/*+ Is the data sorted and therefore searchable? +*/
static int sorted=0;

/*+ Is the data loaded from a file and therefore read-only? +*/
static int loaded=0;

/*+ How many entries are allocated? +*/
static size_t alloced=0;

/* Functions */

static int sort_by_id(Segment *a,Segment *b);


/*++++++++++++++++++++++++++++++++++++++
  Load in a segment list from a file.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

void LoadSegmentList(const char *filename)
{
 assert(!OSMSegments);          /* Must be NULL */

 OSMSegments=(Segments*)MapFile(filename);

 assert(OSMSegments);           /* Must be non-NULL */

 sorted=1;
 loaded=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Save the segment list to a file.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveSegmentList(const char *filename)
{
 int retval;

 assert(!loaded);               /* Must not be loaded */

 assert(sorted);                /* Must be sorted */

 retval=WriteFile(filename,OSMSegments,sizeof(Segments)-sizeof(OSMSegments->segments)+OSMSegments->number*sizeof(Segment));

 assert(!retval);               /* Must be zero */
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first segment with a particular starting node.

  Segment *FindFirstSegment Returns a pointer to the first segment with the specified id.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *FindFirstSegment(node_t node)
{
 int start=0;
 int end=OSMSegments->number-1;
 int mid;
 int found;

 assert(sorted);                /* Must be sorted */

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

 if(OSMSegments->number==0)                       /* There are no segments */
    return(NULL);
 else if(node<OSMSegments->segments[start].node1) /* Check key is not before start */
    return(NULL);
 else if(node>OSMSegments->segments[end].node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                             /* Choose mid point */

       if(OSMSegments->segments[mid].node1<node)      /* Mid point is too low */
          start=mid;
       else if(OSMSegments->segments[mid].node1>node) /* Mid point is too high */
          end=mid;
       else                                           /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(OSMSegments->segments[start].node1==node)      /* Start is correct */
         {found=start; goto found;}

    if(OSMSegments->segments[end].node1==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && OSMSegments->segments[found-1].node1==node)
    found--;

 return(&OSMSegments->segments[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node.

  Segment *FindSegment Returns a pointer to the first segment with the specified id.

  Segment *segment The current segment.
  ++++++++++++++++++++++++++++++++++++++*/

Segment *FindNextSegment(Segment *segment)
{
 Segment *next=segment+1;

 if((next-OSMSegments->segments)==OSMSegments->number)
    return(NULL);

 if(next->node1==segment->node1)
    return(next);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a segment to a newly created segment list (unsorted).

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.

  way_t way The way that the pair of segments are connected by.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendSegment(node_t node1,node_t node2,way_t way)
{
 assert(!loaded);               /* Must not be loaded */

 /* Check that the whole thing is allocated. */

 if(!OSMSegments)
   {
    alloced=INCREMENT;
    OSMSegments=(Segments*)malloc(sizeof(Segments)-sizeof(OSMSegments->segments)+alloced*sizeof(Segment));

    OSMSegments->number=0;
   }

 /* Check that the arrays have enough space. */

 if(OSMSegments->number==alloced)
   {
    alloced+=INCREMENT;
    OSMSegments=(Segments*)realloc((void*)OSMSegments,sizeof(Segments)-sizeof(OSMSegments->segments)+alloced*sizeof(Segment));
   }

 /* Insert the segment */

 OSMSegments->segments[OSMSegments->number].node1=node1;
 OSMSegments->segments[OSMSegments->number].node2=node2;
 OSMSegments->segments[OSMSegments->number].way=way;
 OSMSegments->segments[OSMSegments->number].distance=0;
 OSMSegments->segments[OSMSegments->number].duration=0;

 OSMSegments->number++;

 sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(void)
{
 assert(!loaded);               /* Must not be loaded */

 qsort(OSMSegments->segments,OSMSegments->number,sizeof(Segment),(int (*)(const void*,const void*))sort_by_id);

 sorted=1;
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

 if(a_id1==b_id1)
    return(a_id2-b_id2);
 else
    return(a_id1-b_id1);
}


/*++++++++++++++++++++++++++++++++++++++
  Assign the lengths and durations to all of the segments.
  ++++++++++++++++++++++++++++++++++++++*/

void FixupSegmentLengths(void)
{
 int i;

 assert(!loaded);               /* Must not be loaded */

 assert(sorted);                /* Must be sorted */

 for(i=0;i<OSMSegments->number;i++)
   {
    Node *node1=FindNode(OSMSegments->segments[i].node1);
    Node *node2=FindNode(OSMSegments->segments[i].node2);
    Way  *way=FindWay(OSMSegments->segments[i].way);

    speed_t    speed=way->speed;
    distance_t distance=Distance(node1,node2);
    duration_t duration=hours_to_duration(distance_to_km(distance)/speed);

    OSMSegments->segments[i].distance=distance;
    OSMSegments->segments[i].duration=duration;
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
