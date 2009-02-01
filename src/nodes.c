/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.c,v 1.15 2009-02-01 17:11:07 amb Exp $

 Node data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <stdlib.h>

#include "functions.h"
#include "nodes.h"


/* Constants */

/*+ The array size increment for nodes - expect ~8,000,000 nodes. +*/
#define INCREMENT_NODES 1024*1024


/* Functions */

static int sort_by_id(NodeX *a,NodeX *b);


/*++++++++++++++++++++++++++++++++++++++
  Load in a node list from a file.

  Nodes* LoadNodeList Returns the node list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Nodes *LoadNodeList(const char *filename)
{
 void *data;
 Nodes *nodes;

 nodes=(Nodes*)malloc(sizeof(Nodes));

 data=MapFile(filename);

 /* Copy the Nodes structure from the loaded data */

 *nodes=*((Nodes*)data);

 /* Adjust the pointers in the Nodes structure. */

 nodes->data =data;
 nodes->nodes=(Node*)(data+(off_t)nodes->nodes);

 return(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new node list.

  NodesX *NewNodeList Returns the node list.
  ++++++++++++++++++++++++++++++++++++++*/

NodesX *NewNodeList(void)
{
 NodesX *nodesx;

 nodesx=(NodesX*)malloc(sizeof(NodesX));

 nodesx->alloced=INCREMENT_NODES;
 nodesx->number=0;
 nodesx->sorted=0;

 nodesx->xdata=(NodeX*)malloc(nodesx->alloced*sizeof(NodeX));

 return(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the node list to a file.

  NodesX* nodesx The set of nodes to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveNodeList(NodesX* nodesx,const char *filename)
{
 int i;
 int fd;
 Nodes *nodes=calloc(1,sizeof(Nodes));

 assert(nodesx->sorted);      /* Must be sorted */

 /* Fill in a Nodes structure with the offset of the real data in the file after
    the Node structure itself. */

 nodes->number=nodesx->number;
 nodes->data=NULL;
 nodes->nodes=(void*)sizeof(Nodes);

 /* Write out the Nodes structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,nodes,sizeof(Nodes));

 for(i=0;i<nodesx->number;i++)
   {
    if(nodesx->xdata[i].id==10319449)
       printf("\n10319449 -> %d\n",i);

    if(nodesx->xdata[i].id==30810456)
       printf("\n30810456 -> %d\n",i);

    if(nodesx->xdata[i].id==21734658)
       printf("\n21734658 -> %d\n",i);

    if(nodesx->xdata[i].id==206231001)
       printf("\n206231001 -> %d\n",i);

    WriteFile(fd,&nodesx->xdata[i].node,sizeof(Node));
   }

 CloseFile(fd);

 /* Free the fake Nodes */

 free(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node.

  NodeX *FindNode Returns the extended node with the specified id.

  NodesX* nodesx The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

NodeX *FindNode(NodesX* nodesx,node_t id)
{
 int start=0;
 int end=nodesx->number-1;
 int mid;

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

 if(end<start)                       /* There are no nodes */
    return(NULL);
 else if(id<nodesx->xdata[start].id) /* Check key is not before start */
    return(NULL);
 else if(id>nodesx->xdata[end].id)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                /* Choose mid point */

       if(nodesx->xdata[mid].id<id)      /* Mid point is too low */
          start=mid+1;
       else if(nodesx->xdata[mid].id>id) /* Mid point is too high */
          end=mid-1;
       else                              /* Mid point is correct */
          return(LookupNodeX(nodesx,mid));
      }
    while((end-start)>1);

    if(nodesx->xdata[start].id==id)      /* Start is correct */
       return(LookupNodeX(nodesx,start));

    if(nodesx->xdata[end].id==id)        /* End is correct */
       return(LookupNodeX(nodesx,end));
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a node to a newly created node list (unsorted).

  NodeX *AppendNode Return a pointer to the new extended node.

  NodesX* nodesx The set of nodes to process.

  node_t id The node identification.

  latlong_t latitude The latitude of the node.

  latlong_t longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

NodeX *AppendNode(NodesX* nodesx,node_t id,latlong_t latitude,latlong_t longitude)
{
 /* Check that the array has enough space. */

 if(nodesx->number==nodesx->alloced)
   {
    nodesx->alloced+=INCREMENT_NODES;

    nodesx->xdata=(NodeX*)realloc((void*)nodesx->xdata,nodesx->alloced*sizeof(NodeX));
   }

 /* Insert the node */

 nodesx->xdata[nodesx->number].id=id;
 nodesx->xdata[nodesx->number].super=0;
 nodesx->xdata[nodesx->number].node.latitude=latitude;
 nodesx->xdata[nodesx->number].node.longitude=longitude;

 nodesx->number++;

 nodesx->sorted=0;

 return(&nodesx->xdata[nodesx->number-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesX* nodesx)
{
 qsort(nodesx->xdata,nodesx->number,sizeof(NodeX),(int (*)(const void*,const void*))sort_by_id);

 while(nodesx->xdata[nodesx->number-1].id==~0)
    nodesx->number--;

 nodesx->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into id order.

  int sort_by_id Returns the comparison of the id fields.

  NodeX *a The first Node.

  NodeX *b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(NodeX *a,NodeX *b)
{
 node_t a_id=a->id;
 node_t b_id=b->id;

 if(a_id<b_id)
    return(-1);
 else
    return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Remove any nodes that are not part of a highway.

  NodesX *nodesx The complete node list.

  SegmentsX *segmentsx The list of segments.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx)
{
 int i;
 int highway=0,nothighway=0;

 for(i=0;i<nodesx->number;i++)
   {
    if(FindFirstSegment(segmentsx,nodesx->xdata[i].id))
       highway++;
    else
      {
       nodesx->xdata[i].id=~0;
       nothighway++;
      }

    if(!((i+1)%10000))
      {
       printf("\rChecking: Nodes=%d Highway=%d not-Highway=%d",i+1,highway,nothighway);
       fflush(stdout);
      }
   }

 printf("\rChecked: Nodes=%d Highway=%d not-Highway=%d  \n",nodesx->number,highway,nothighway);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Fix the node indexes to the segments.

  NodesX* nodesx The set of nodes to process.

  SegmentsX *segmentsx The list of segments to use.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

void FixupNodes(NodesX *nodesx,SegmentsX* segmentsx,int iteration)
{
 int i;

 assert(nodesx->sorted);      /* Must be sorted */

 for(i=0;i<nodesx->number;i++)
   {
    SegmentX *firstseg=FindFirstSegment(segmentsx,nodesx->xdata[i].id);

    nodesx->xdata[i].node.firstseg=IndexSegmentX(segmentsx,firstseg);

    if(nodesx->xdata[i].super==iteration)
       nodesx->xdata[i].node.firstseg|=SUPER_FLAG;

    if(!((i+1)%10000))
      {
       printf("\rFixing Nodes: Nodes=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rFixed Nodes: Nodes=%d \n",nodesx->number);
 fflush(stdout);
}
