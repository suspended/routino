/***************************************
 $Header: /home/amb/CVS/routino/src/supersegments.c,v 1.1 2009-01-06 18:32:27 amb Exp $

 Super-Segment data type functions.
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

/*+ The list of super-segments +*/
SuperSegments *OSMSuperSegments=NULL;

/*+ Is the data sorted and therefore searchable? +*/
static int sorted=0;

/*+ Is the data loaded from a file and therefore read-only? +*/
static int loaded=0;

/*+ How many entries are allocated? +*/
static size_t alloced=0;

/* Functions */

static void append_super_segment(node_t node1,node_t node2);
static int sort_by_id(SuperSegment *a,SuperSegment *b);


/*++++++++++++++++++++++++++++++++++++++
  Load in a super-segment list from a file.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

void LoadSuperSegmentList(const char *filename)
{
 assert(!OSMSuperSegments);          /* Must be NULL */

 OSMSuperSegments=(SuperSegments*)MapFile(filename);

 assert(OSMSuperSegments);           /* Must be non-NULL */

 sorted=1;
 loaded=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Save the super-segment list to a file.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveSuperSegmentList(const char *filename)
{
 int retval;

 assert(!loaded);               /* Must not be loaded */

 assert(sorted);                /* Must be sorted */

 retval=WriteFile(filename,OSMSuperSegments,sizeof(SuperSegments)-sizeof(OSMSuperSegments->segments)+OSMSuperSegments->number*sizeof(SuperSegment));

 assert(!retval);               /* Must be zero */
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first super-segment with a particular starting node.

  SuperSegment *FindFirstSuperSegment Returns a pointer to the first super-segment with the specified id.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

SuperSegment *FindFirstSuperSegment(node_t node)
{
 int start=0;
 int end=OSMSuperSegments->number-1;
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

 if(OSMSuperSegments->number==0)                       /* There are no segments */
    return(NULL);
 else if(node<OSMSuperSegments->segments[start].node1) /* Check key is not before start */
    return(NULL);
 else if(node>OSMSuperSegments->segments[end].node1)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                                  /* Choose mid point */

       if(OSMSuperSegments->segments[mid].node1<node)      /* Mid point is too low */
          start=mid;
       else if(OSMSuperSegments->segments[mid].node1>node) /* Mid point is too high */
          end=mid;
       else                                                /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(OSMSuperSegments->segments[start].node1==node)      /* Start is correct */
         {found=start; goto found;}

    if(OSMSuperSegments->segments[end].node1==node)        /* End is correct */
         {found=end; goto found;}
   }

 return(NULL);

 found:

 while(found>0 && OSMSuperSegments->segments[found-1].node1==node)
    found--;

 return(&OSMSuperSegments->segments[found]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next super-segment with a particular starting node.

  SuperSegment *FindSuperSegment Returns a pointer to the first super-segment with the specified id.

  SuperSegment *supersegment The current super-segment.
  ++++++++++++++++++++++++++++++++++++++*/

SuperSegment *FindNextSuperSegment(SuperSegment *supersegment)
{
 SuperSegment *next=supersegment+1;

 if((next-OSMSuperSegments->segments)==OSMSuperSegments->number)
    return(NULL);

 if(next->node1==supersegment->node1)
    return(next);

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Select the super-segments from the list of segments.
  ++++++++++++++++++++++++++++++++++++++*/

void ChooseSuperSegments(void)
{
 assert(!loaded);               /* Must not be loaded */


 /* Check that the whole thing is allocated. */

 if(!OSMSuperSegments)
   {
    alloced=INCREMENT;
    OSMSuperSegments=(SuperSegments*)malloc(sizeof(SuperSegments)-sizeof(OSMSuperSegments->segments)+alloced*sizeof(SuperSegment));

    OSMSuperSegments->number=0;
   }

}


/*++++++++++++++++++++++++++++++++++++++
  Append a super-segment to a newly created segment list (unsorted).

  node_t node1 The first node in the super-segment.

  node_t node2 The second node in the super-segment.
  ++++++++++++++++++++++++++++++++++++++*/

static void append_super_segment(node_t node1,node_t node2)
{
 assert(!loaded);               /* Must not be loaded */

 /* Check that the whole thing is allocated. */

 if(!OSMSuperSegments)
   {
    alloced=INCREMENT;
    OSMSuperSegments=(SuperSegments*)malloc(sizeof(SuperSegments)-sizeof(OSMSuperSegments->segments)+alloced*sizeof(SuperSegment));

    OSMSuperSegments->number=0;
   }

 /* Check that the arrays have enough space. */

 if(OSMSuperSegments->number==alloced)
   {
    alloced+=INCREMENT;
    OSMSuperSegments=(SuperSegments*)realloc((void*)OSMSuperSegments,sizeof(SuperSegments)-sizeof(OSMSuperSegments->segments)+alloced*sizeof(SuperSegment));
   }

 /* Insert the super-segment */

 OSMSuperSegments->segments[OSMSuperSegments->number].node1=node1;
 OSMSuperSegments->segments[OSMSuperSegments->number].node2=node2;
 OSMSuperSegments->segments[OSMSuperSegments->number].distance=0;
 OSMSuperSegments->segments[OSMSuperSegments->number].duration=0;

 OSMSuperSegments->number++;

 sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the super-segment list.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSuperSegmentList(void)
{
 assert(!loaded);               /* Must not be loaded */

 qsort(OSMSuperSegments->segments,OSMSuperSegments->number,sizeof(SuperSegment),(int (*)(const void*,const void*))sort_by_id);

 sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the super-segments into id order.

  int sort_by_id Returns the comparison of the node fields.

  SuperSegment *a The first SuperSegment.

  SuperSegment *b The second SuperSegment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(SuperSegment *a,SuperSegment *b)
{
 node_t a_id1=a->node1,a_id2=a->node2;
 node_t b_id1=b->node1,b_id2=b->node2;

 if(a_id1==b_id1)
    return(a_id2-b_id2);
 else
    return(a_id1-b_id1);
}


/*++++++++++++++++++++++++++++++++++++++
  Assign the lengths and durations to all of the super-segments.
  ++++++++++++++++++++++++++++++++++++++*/

void FixupSuperSegmentLengths(void)
{
 int i;

 assert(!loaded);               /* Must not be loaded */

 assert(sorted);                /* Must be sorted */

 for(i=0;i<OSMSuperSegments->number;i++)
   {
//    Node *node1=FindNode(OSMSuperSegments->segments[i].node1);
//    Node *node2=FindNode(OSMSuperSegments->segments[i].node2);

//    distance_t distance=Distance(node1,node2);
//    duration_t duration=hours_to_duration(distance_to_km(distance)/speed);

//    OSMSuperSegments->segments[i].distance=distance;
//    OSMSuperSegments->segments[i].duration=duration;
   }
}
