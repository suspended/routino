/***************************************
 Extended Segment data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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
#include <sys/stat.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"

#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"

#include "types.h"

#include "files.h"
#include "logging.h"
#include "sorting.h"


/* Variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;

/* Local Functions */

static int sort_by_id(SegmentX *a,SegmentX *b);

static distance_t DistanceX(NodeX *nodex1,NodeX *nodex2);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new segment list (create a new file or open an existing one).

  SegmentsX *NewSegmentList Returns the segment list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsX *NewSegmentList(int append)
{
 SegmentsX *segmentsx;

 segmentsx=(SegmentsX*)calloc(1,sizeof(SegmentsX));

 assert(segmentsx); /* Check calloc() worked */

 segmentsx->filename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(segmentsx->filename,"%s/segmentsx.input.tmp",option_tmpdirname);
 else
    sprintf(segmentsx->filename,"%s/segmentsx.%p.tmp",option_tmpdirname,segmentsx);

 if(append)
   {
    off_t size;

    segmentsx->fd=OpenFileAppend(segmentsx->filename);

    size=SizeFile(segmentsx->filename);

    segmentsx->xnumber=size/sizeof(SegmentX);
   }
 else
    segmentsx->fd=OpenFileNew(segmentsx->filename);

 return(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a segment list.

  SegmentsX *segmentsx The list to be freed.

  int keep Set to 1 if the file is to be kept.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeSegmentList(SegmentsX *segmentsx,int keep)
{
 if(!keep)
    DeleteFile(segmentsx->filename);

 free(segmentsx->filename);

 if(segmentsx->idata)
    free(segmentsx->idata);

 if(segmentsx->usednode)
    free(segmentsx->usednode);

 if(segmentsx->firstnode)
    free(segmentsx->firstnode);

 free(segmentsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single segment to an unsorted segment list.

  SegmentsX* segmentsx The set of segments to process.

  way_t way The way that the segment belongs to.

  node_t node1 The first node in the segment.

  node_t node2 The second node in the segment.

  distance_t distance The distance between the nodes (or just the flags).
  ++++++++++++++++++++++++++++++++++++++*/

void AppendSegment(SegmentsX* segmentsx,way_t way,node_t node1,node_t node2,distance_t distance)
{
 SegmentX segmentx;

 if(node1>node2)
   {
    node_t temp;

    temp=node1;
    node1=node2;
    node2=temp;

    if(distance&(ONEWAY_2TO1|ONEWAY_1TO2))
       distance^=ONEWAY_2TO1|ONEWAY_1TO2;
   }

 segmentx.node1=node1;
 segmentx.node2=node2;
 segmentx.next2=NO_SEGMENT;
 segmentx.way=way;
 segmentx.distance=distance;

 WriteFile(segmentsx->fd,&segmentx,sizeof(SegmentX));

 segmentsx->xnumber++;

 assert(segmentsx->xnumber<SEGMENT_FAKE); /* SEGMENT_FAKE marks the high-water mark for real segments. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segment list.

  SegmentsX* segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortSegmentList(SegmentsX* segmentsx)
{
 int fd;

 /* Print the start message */

 printf_first("Sorting Segments");

 /* Close the file (finished appending) */

 segmentsx->fd=CloseFile(segmentsx->fd);

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Sort by node indexes */

 filesort_fixed(segmentsx->fd,fd,sizeof(SegmentX),(int (*)(const void*,const void*))sort_by_id,NULL);

 segmentsx->number=segmentsx->xnumber;

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Sorted Segments: Segments=%d",segmentsx->xnumber);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the segments into id order (node1 then node2).

  int sort_by_id Returns the comparison of the node fields.

  SegmentX *a The first segment.

  SegmentX *b The second segment.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(SegmentX *a,SegmentX *b)
{
 node_t a_id1=a->node1;
 node_t b_id1=b->node1;

 if(a_id1<b_id1)
    return(-1);
 else if(a_id1>b_id1)
    return(1);
 else /* if(a_id1==b_id1) */
   {
    node_t a_id2=a->node2;
    node_t b_id2=b->node2;

    if(a_id2<b_id2)
       return(-1);
    else if(a_id2>b_id2)
       return(1);
    else
      {
       distance_t a_distance=a->distance;
       distance_t b_distance=b->distance;

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
  Find the first segment index with a particular starting node.
 
  index_t IndexFirstSegmentX1 Returns the index of the first extended segment with the specified id.

  SegmentsX* segmentsx The set of segments to process.

  node_t node The node to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexFirstSegmentX1(SegmentsX* segmentsx,node_t node)
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

 if(end<start)                         /* There are no nodes */
    return(NO_SEGMENT);
 else if(node<segmentsx->idata[start]) /* Check key is not before start */
    return(NO_SEGMENT);
 else if(node>segmentsx->idata[end])   /* Check key is not after end */
    return(NO_SEGMENT);
 else
   {
    do
      {
       mid=(start+end)/2;                  /* Choose mid point */

       if(segmentsx->idata[mid]<node)      /* Mid point is too low */
          start=mid;
       else if(segmentsx->idata[mid]>node) /* Mid point is too high */
          end=mid;
       else                                /* Mid point is correct */
         {found=mid; goto found;}
      }
    while((end-start)>1);

    if(segmentsx->idata[start]==node)      /* Start is correct */
      {found=start; goto found;}

    if(segmentsx->idata[end]==node)        /* End is correct */
      {found=end; goto found;}
   }

 return(NO_SEGMENT);

 found:

 while(found>0 && segmentsx->idata[found-1]==node)
    found--;

 return(found);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment index with a particular starting node.

  index_t IndexNextSegmentX1 Returns the index of the next segment with the same id.

  SegmentsX* segmentsx The set of segments to process.

  index_t segindex The current segment index.

  node_t node The node.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexNextSegmentX1(SegmentsX* segmentsx,index_t segindex,node_t node)
{
 segindex++;

 if(segindex==segmentsx->xnumber)
    return(NO_SEGMENT);
 else if(segmentsx->idata[segindex]==node)
    return(segindex);
 else
    return(NO_SEGMENT);
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Find the first extended segment with a particular starting node index.
 
  SegmentX *FirstSegmentX2 Returns a pointer to the first extended segment with the specified id.

  SegmentsX* segmentsx The set of extended segments to process.

  index_t nodeindex The node index to look for.

  int position A flag to pass through.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX *FirstSegmentX2(SegmentsX* segmentsx,index_t nodeindex,int position)
{
 index_t index=segmentsx->firstnode[nodeindex];
 SegmentX *segmentx;

 segmentx=LookupSegmentX(segmentsx,index,position);

 if(segmentx->node1!=nodeindex && segmentx->node2!=nodeindex)
    return(NULL);

 return(segmentx);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next segment with a particular starting node index.

  SegmentX *NextSegmentX2 Returns a pointer to the next segment with the same id.

  SegmentsX* segmentsx The set of segments to process.

  SegmentX *segmentx The current segment.

  index_t node The node index.

  int position A flag to pass through.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentX *NextSegmentX2(SegmentsX* segmentsx,SegmentX *segmentx,index_t node,int position)
{
 if(segmentx->node1==node)
   {
#if SLIM
    index_t index=IndexSegmentX(segmentsx,segmentx);
    index++;

    if(index>=segmentsx->number)
       return(NULL);
    segmentx=LookupSegmentX(segmentsx,index,position);
    if(segmentx->node1!=node)
       return(NULL);
    else
       return(segmentx);
#else
    segmentx++;
    if(IndexSegmentX(segmentsx,segmentx)>=segmentsx->number || segmentx->node1!=node)
       return(NULL);
    else
       return(segmentx);
#endif
   }
 else
   {
    if(segmentx->next2==NO_SEGMENT)
       return(NULL);
    else
       return(LookupSegmentX(segmentsx,segmentx->next2,position));
   }
}
 
 
/*++++++++++++++++++++++++++++++++++++++
  Remove bad segments (duplicated, zero length or missing nodes).

  NodesX *nodesx The nodes to check.

  SegmentsX *segmentsx The segments to modify.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveBadSegments(NodesX *nodesx,SegmentsX *segmentsx)
{
 int duplicate=0,loop=0,missing=0,good=0,total=0;
 SegmentX segmentx;
 int fd;
 node_t prevnode1=NO_NODE,prevnode2=NO_NODE;

 /* Print the start message */

 printf_first("Checking Segments: Segments=0 Duplicate=0 Loop=0 Missing-Node=0");

 /* Allocate the array of indexes */

 segmentsx->idata=(node_t*)malloc(segmentsx->xnumber*sizeof(node_t));

 assert(segmentsx->idata); /* Check malloc() worked */

 /* Allocate the array of node flags */

 segmentsx->usednode=(char*)calloc(nodesx->xnumber,sizeof(char));

 assert(segmentsx->usednode); /* Check malloc() worked */

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Modify the on-disk image */

 while(!ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX)))
   {
    index_t index1=IndexNodeX(nodesx,segmentx.node1);
    index_t index2=IndexNodeX(nodesx,segmentx.node2);

    if(prevnode1==segmentx.node1 && prevnode2==segmentx.node2)
       duplicate++;
    else if(segmentx.node1==segmentx.node2)
       loop++;
    else if(index1==NO_NODE || index2==NO_NODE)
       missing++;
    else
      {
       WriteFile(fd,&segmentx,sizeof(SegmentX));

       segmentsx->usednode[index1]=1;
       segmentsx->usednode[index2]=1;

       segmentsx->idata[good]=segmentx.node1;
       good++;

       prevnode1=segmentx.node1;
       prevnode2=segmentx.node2;
      }

    total++;

    if(!(total%10000))
       printf_middle("Checking Segments: Segments=%d Duplicate=%d Loop=%d Missing-Node=%d",total,duplicate,loop,missing);
   }

 segmentsx->number=good;

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Checked Segments: Segments=%d Duplicate=%d Loop=%d Missing-Node=%d",total,duplicate,loop,missing);
}


/*++++++++++++++++++++++++++++++++++++++
  Measure the segments and replace node/way ids with indexes.

  SegmentsX* segmentsx The set of segments to process.

  NodesX *nodesx The list of nodes to use.

  WaysX *waysx The list of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void MeasureSegments(SegmentsX* segmentsx,NodesX *nodesx,WaysX *waysx)
{
 index_t index=0;
 int fd;
 SegmentX segmentx;

 /* Print the start message */

 printf_first("Measuring Segments: Segments=0");

 /* Map into memory /  open the file */

#if !SLIM
 nodesx->xdata=MapFile(nodesx->filename);
#else
 nodesx->fd=ReOpenFile(nodesx->filename);
#endif

 /* Free the now-unneeded index */

 free(segmentsx->idata);
 segmentsx->idata=NULL;

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Modify the on-disk image */

 while(!ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX)))
   {
    index_t node1=IndexNodeX(nodesx,segmentx.node1);
    index_t node2=IndexNodeX(nodesx,segmentx.node2);
    index_t way  =IndexWayX (waysx ,segmentx.way);

    NodeX *nodex1=LookupNodeX(nodesx,node1,1);
    NodeX *nodex2=LookupNodeX(nodesx,node2,2);

    /* Replace the node and way ids with their indexes */

    segmentx.node1=node1;
    segmentx.node2=node2;
    segmentx.way  =way;

    /* Set the distance but preserve the ONEWAY_* flags */

    segmentx.distance|=DISTANCE(DistanceX(nodex1,nodex2));

    /* Write the modified segment */

    WriteFile(fd,&segmentx,sizeof(SegmentX));

    index++;

    if(!(index%10000))
       printf_middle("Measuring Segments: Segments=%d",index);
   }

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Free the other now-unneeded indexes */

 free(nodesx->idata);
 nodesx->idata=NULL;

 free(waysx->idata);
 waysx->idata=NULL;

 /* Unmap from memory / close the file */

#if !SLIM
 nodesx->xdata=UnmapFile(nodesx->filename);
#else
 nodesx->fd=CloseFile(nodesx->fd);
#endif

 /* Print the final message */

 printf_last("Measured Segments: Segments=%d",segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Remove the duplicate segments.

  SegmentsX* segmentsx The set of segments to process.

  NodesX *nodesx The list of nodes to use.

  WaysX *waysx The list of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void DeduplicateSegments(SegmentsX* segmentsx,NodesX *nodesx,WaysX *waysx)
{
 int duplicate=0,good=0;
 index_t firstindex=0,index=0;
 int fd;
 SegmentX prevsegmentx[16],segmentx;

 /* Print the start message */

 printf_first("Deduplicating Segments: Segments=0 Duplicate=0");

 /* Map into memory / open the file */

#if !SLIM
 waysx->xdata=MapFile(waysx->filename);
#else
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Modify the on-disk image */

 while(!ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX)))
   {
    int isduplicate=0;

    if(index && segmentx.node1==prevsegmentx[0].node1 &&
                segmentx.node2==prevsegmentx[0].node2)
      {
       index_t previndex=firstindex;

       while(previndex<index)
         {
          int offset=previndex-firstindex;

          if(DISTFLAG(segmentx.distance)==DISTFLAG(prevsegmentx[offset].distance))
            {
             WayX *wayx1=LookupWayX(waysx,prevsegmentx[offset].way,1);
             WayX *wayx2=LookupWayX(waysx,    segmentx        .way,2);

             if(!WaysCompare(&wayx1->way,&wayx2->way))
               {
                isduplicate=1;
                break;
               }
            }

          previndex++;
         }

       assert((index-firstindex)<(sizeof(prevsegmentx)/sizeof(prevsegmentx[0])));

       prevsegmentx[index-firstindex]=segmentx;
      }
    else
      {
       firstindex=index;
       prevsegmentx[0]=segmentx;
      }

    if(isduplicate)
       duplicate++;
    else
      {
       WriteFile(fd,&segmentx,sizeof(SegmentX));

       good++;
      }

    index++;

    if(!(index%10000))
       printf_middle("Deduplicating Segments: Segments=%d Duplicate=%d",index,duplicate);
   }

 segmentsx->number=good;

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Unmap from memory / close the file */

#if !SLIM
 waysx->xdata=UnmapFile(waysx->filename);
#else
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Print the final message */

 printf_last("Deduplicated Segments: Segments=%d Duplicate=%d Unique=%d",index,duplicate,good);
}


/*++++++++++++++++++++++++++++++++++++++
  Index the segments by creating the firstnode index and the segment next2 indexes.

  SegmentsX* segmentsx The set of segments to process.

  NodesX *nodesx The list of nodes to use.
  ++++++++++++++++++++++++++++++++++++++*/

void IndexSegments(SegmentsX* segmentsx,NodesX *nodesx)
{
 index_t index;
 int i;

 /* Print the start message */

 printf_first("Indexing Segments: Segments=0");

 /* Allocate the array of indexes */

 if(!segmentsx->firstnode)
   {
    segmentsx->firstnode=(index_t*)malloc((nodesx->number+1)*sizeof(index_t));

    assert(segmentsx->firstnode); /* Check malloc() worked */
   }

 for(i=0;i<nodesx->number;i++)
    segmentsx->firstnode[i]=NO_SEGMENT;

 /* Map into memory / open the files */

#if !SLIM
 segmentsx->xdata=MapFileWriteable(segmentsx->filename);
#else
 segmentsx->fd=ReOpenFileWriteable(segmentsx->filename);
#endif

 /* Read through the segments in reverse order */

 for(index=segmentsx->number-1;index!=NO_SEGMENT;index--)
   {
    SegmentX *segmentx=LookupSegmentX(segmentsx,index,1);

    if(segmentsx->firstnode[segmentx->node2]!=NO_SEGMENT)
      {
       segmentx->next2=segmentsx->firstnode[segmentx->node2];

       PutBackSegmentX(segmentsx,index,1);
      }

    segmentsx->firstnode[segmentx->node1]=index;
    segmentsx->firstnode[segmentx->node2]=index;

    if(!(index%10000))
       printf_middle("Indexing Segments: Segments=%d",segmentsx->number-index);
   }

 /* Unmap from memory / close the files */

#if !SLIM
 segmentsx->xdata=UnmapFile(segmentsx->filename);
#else
 segmentsx->fd=CloseFile(segmentsx->fd);
#endif

 /* Fix-up the firstnode index for the missing nodes */

 segmentsx->firstnode[nodesx->number]=segmentsx->number;

 for(i=nodesx->number-1;i>=0;i--)
    if(segmentsx->firstnode[i]==NO_SEGMENT)
       segmentsx->firstnode[i]=segmentsx->firstnode[i+1];

 /* Print the final message */

 printf_last("Indexed Segments: Segments=%d",segmentsx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Update the segment indexes after geographical sorting.

  SegmentsX *segmentsx The list of segments to update.

  NodesX *nodesx The set of nodes to use.

  WaysX* waysx The set of ways to use.
  ++++++++++++++++++++++++++++++++++++++*/

void UpdateSegments(SegmentsX *segmentsx,NodesX *nodesx,WaysX *waysx)
{
 index_t i;
 int fd;

 /* Print the start message */

 printf_first("Updating Super Segments: Segments=0");

 /* Map into memory / open the files */

#if !SLIM
 waysx->xdata=MapFile(waysx->filename);
#else
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Re-open the file read-only and a new file writeable */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 DeleteFile(segmentsx->filename);

 fd=OpenFileNew(segmentsx->filename);

 /* Modify the on-disk image */

 for(i=0;i<segmentsx->number;i++)
   {
    SegmentX segmentx;
    WayX *wayx;

    ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX));

    segmentx.node1=nodesx->gdata[segmentx.node1];
    segmentx.node2=nodesx->gdata[segmentx.node2];

    wayx=LookupWayX(waysx,segmentx.way,1);

    segmentx.way=wayx->prop;

    WriteFile(fd,&segmentx,sizeof(SegmentX));

    if(!((i+1)%10000))
       printf_middle("Updating Super Segments: Segments=%d",i+1);
   }

 /* Close the files */

 segmentsx->fd=CloseFile(segmentsx->fd);
 CloseFile(fd);

 /* Unmap from memory / close the files */

#if !SLIM
 waysx->xdata=UnmapFile(waysx->filename);
#else
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Print the final message */

 printf_last("Updated Super Segments: Segments=%d",segmentsx->number);
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
 SegmentsFile segmentsfile={0};
 int super_number=0,normal_number=0;

 /* Print the start message */

 printf_first("Writing Segments: Segments=0");

 /* Re-open the file */

 segmentsx->fd=ReOpenFile(segmentsx->filename);

 /* Write out the segments data */

 fd=OpenFileNew(filename);

 SeekFile(fd,sizeof(SegmentsFile));

 for(i=0;i<segmentsx->number;i++)
   {
    SegmentX segmentx;
    Segment  segment;

    ReadFile(segmentsx->fd,&segmentx,sizeof(SegmentX));

    segment.node1   =segmentx.node1;
    segment.node2   =segmentx.node2;
    segment.next2   =segmentx.next2;
    segment.way     =segmentx.way;
    segment.distance=segmentx.distance;

    if(IsSuperSegment(&segment))
       super_number++;
    if(IsNormalSegment(&segment))
       normal_number++;

    WriteFile(fd,&segment,sizeof(Segment));

    if(!((i+1)%10000))
       printf_middle("Writing Segments: Segments=%d",i+1);
   }

 /* Write out the header structure */

 segmentsfile.number=segmentsx->number;
 segmentsfile.snumber=super_number;
 segmentsfile.nnumber=normal_number;

 SeekFile(fd,0);
 WriteFile(fd,&segmentsfile,sizeof(SegmentsFile));

 CloseFile(fd);

 /* Close the file */

 segmentsx->fd=CloseFile(segmentsx->fd);

 /* Print the final message */

 printf_last("Wrote Segments: Segments=%d",segmentsx->number);
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
