/***************************************
 $Header: /home/amb/CVS/routino/src/segments.c,v 1.2 2009-01-01 20:01:14 amb Exp $

 Segment data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <math.h>
#include <stdlib.h>

#include "functions.h"
#include "types.h"

#define INCREMENT 64*1024

/*+ The list of segments +*/
Segments *OSMSegments=NULL;

/*+ Is the data sorted and therefore searchable? +*/
static int sorted=0;

/* Functions */

static void sort_segment_list(void);
static int sort_by_id(Segment *a,Segment *b);


/*++++++++++++++++++++++++++++++++++++++
  Load in a segment list from a file.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

int LoadSegmentList(const char *filename)
{
 OSMSegments=(Segments*)MapFile(filename);

 if(OSMSegments)
    sorted=1;

 return(!OSMSegments);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the segment list to a file.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

int SaveSegmentList(const char *filename)
{
 int retval;
 size_t alloced;

 if(!sorted)
    sort_segment_list();

 alloced=OSMSegments->alloced;
 OSMSegments->alloced=OSMSegments->number;

 retval=WriteFile(filename,OSMSegments,sizeof(Segments)-sizeof(OSMSegments->segments)+OSMSegments->number*sizeof(Segment));

 OSMSegments->alloced=alloced;

 return(retval);
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

 if(!sorted)
    sort_segment_list();

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

  way_t way THe way that the pair of segments are connected by.

  distance_t distance The distane between the two nodes.

  duration_t duration The distane between the two nodes.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendSegment(node_t node1,node_t node2,way_t way,distance_t distance,duration_t duration)
{
 /* Check that the whole thing is allocated. */

 if(!OSMSegments)
   {
    OSMSegments=(Segments*)malloc(sizeof(Segments));

    OSMSegments->alloced=sizeof(OSMSegments->segments)/sizeof(OSMSegments->segments[0]);
    OSMSegments->number=0;
   }

 /* Check that the arrays have enough space. */

 if(OSMSegments->number==OSMSegments->alloced)
   {
    OSMSegments=(Segments*)realloc((void*)OSMSegments,sizeof(Segments)-sizeof(OSMSegments->segments)+(OSMSegments->alloced+INCREMENT)*sizeof(Segment));

    OSMSegments->alloced+=INCREMENT;
   }

 /* Insert the segment */

 OSMSegments->segments[OSMSegments->number].node1=node1;
 OSMSegments->segments[OSMSegments->number].node2=node2;
 OSMSegments->segments[OSMSegments->number].way=way;
 OSMSegments->segments[OSMSegments->number].distance=distance;
 OSMSegments->segments[OSMSegments->number].duration=duration;

 OSMSegments->number++;

 sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.
  ++++++++++++++++++++++++++++++++++++++*/

static void sort_segment_list(void)
{
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
  Calculate the length of a segment.

  distance_t SegmentLength Returns the length of the segment.

  Node *node1 The starting node.

  Node *node2 The end node.
  ++++++++++++++++++++++++++++++++++++++*/

distance_t SegmentLength(Node *node1,Node *node2)
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
 d = 6378137 * c;

 return d;
}
