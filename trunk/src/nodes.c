/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.c,v 1.13 2009-01-29 19:31:52 amb Exp $

 Node data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#include "functions.h"
#include "nodes.h"


/* Functions */

static int sort_by_id(NodeEx *a,NodeEx *b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new node list.

  NodesMem *NewNodeList Returns the node list.
  ++++++++++++++++++++++++++++++++++++++*/

NodesMem *NewNodeList(void)
{
 NodesMem *nodesmem;

 nodesmem=(NodesMem*)malloc(sizeof(NodesMem));

 nodesmem->alloced=INCREMENT_NODES;
 nodesmem->number=0;
 nodesmem->sorted=0;

 nodesmem->xdata=(NodeEx*)malloc(nodesmem->alloced*sizeof(NodeEx));

 return(nodesmem);
}


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
  Save the node list to a file.

  NodesMem* nodesmem The set of nodes to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveNodeList(NodesMem* nodesmem,const char *filename)
{
 int i;
 int fd;
 Nodes *nodes=calloc(1,sizeof(Nodes));

 assert(nodesmem->sorted);      /* Must be sorted */

 /* Fill in a Nodes structure with the offset of the real data in the file after
    the Node structure itself. */

 nodes->number=nodesmem->number;
 nodes->data=NULL;
 nodes->nodes=(void*)sizeof(Nodes);

 /* Write out the Nodes structure and then the real data. */

 fd=OpenFile(filename);

 write(fd,nodes,sizeof(Nodes));

 for(i=0;i<nodesmem->number;i++)
   {
    if(nodesmem->xdata[i].id==10319449)
       printf("\n10319449 -> %d\n",i);

    if(nodesmem->xdata[i].id==30810456)
       printf("\n30810456 -> %d\n",i);

    if(nodesmem->xdata[i].id==21734658)
       printf("\n21734658 -> %d\n",i);

    if(nodesmem->xdata[i].id==206231001)
       printf("\n206231001 -> %d\n",i);

    write(fd,&nodesmem->xdata[i].node,sizeof(Node));
   }

 close(fd);

 /* Free the fake Nodes */

 free(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node.

  NodeEx *FindNode Returns the extended node with the specified id.

  NodesMem* nodesmem The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

NodeEx *FindNode(NodesMem* nodesmem,node_t id)
{
 int start=0;
 int end=nodesmem->number-1;
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

 if(end<start)                         /* There are no nodes */
    return(NULL);
 else if(id<nodesmem->xdata[start].id) /* Check key is not before start */
    return(NULL);
 else if(id>nodesmem->xdata[end].id)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                  /* Choose mid point */

       if(nodesmem->xdata[mid].id<id)      /* Mid point is too low */
          start=mid+1;
       else if(nodesmem->xdata[mid].id>id) /* Mid point is too high */
          end=mid-1;
       else                                /* Mid point is correct */
          return(LookupNodeEx(nodesmem,mid));
      }
    while((end-start)>1);

    if(nodesmem->xdata[start].id==id)      /* Start is correct */
       return(LookupNodeEx(nodesmem,start));

    if(nodesmem->xdata[end].id==id)        /* End is correct */
       return(LookupNodeEx(nodesmem,end));
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a node to a newly created node list (unsorted).

  NodeEx *AppendNode Return a pointer to the new extended node.

  NodesMem* nodesmem The set of nodes to process.

  node_t id The node identification.

  latlong_t latitude The latitude of the node.

  latlong_t longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

NodeEx *AppendNode(NodesMem* nodesmem,node_t id,latlong_t latitude,latlong_t longitude)
{
 /* Check that the array has enough space. */

 if(nodesmem->number==nodesmem->alloced)
   {
    nodesmem->alloced+=INCREMENT_NODES;

    nodesmem->xdata=(NodeEx*)realloc((void*)nodesmem->xdata,nodesmem->alloced*sizeof(NodeEx));
   }

 /* Insert the node */

 nodesmem->xdata[nodesmem->number].id=id;
 nodesmem->xdata[nodesmem->number].super=0;
 nodesmem->xdata[nodesmem->number].node.latitude=latitude;
 nodesmem->xdata[nodesmem->number].node.longitude=longitude;

 nodesmem->number++;

 nodesmem->sorted=0;

 return(&nodesmem->xdata[nodesmem->number-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.

  NodesMem* nodesmem The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesMem* nodesmem)
{
 qsort(nodesmem->xdata,nodesmem->number,sizeof(NodeEx),(int (*)(const void*,const void*))sort_by_id);

 while(nodesmem->xdata[nodesmem->number-1].id==~0)
    nodesmem->number--;

 nodesmem->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into id order.

  int sort_by_id Returns the comparison of the id fields.

  NodeEx *a The first Node.

  NodeEx *b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(NodeEx *a,NodeEx *b)
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

  NodesMem *nodesmem The complete node list.

  SegmentsMem *segmentsmem The list of segments.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveNonHighwayNodes(NodesMem *nodesmem,SegmentsMem *segmentsmem)
{
 int i;
 int highway=0,nothighway=0;

 for(i=0;i<nodesmem->number;i++)
   {
    if(FindFirstSegment(segmentsmem,nodesmem->xdata[i].id))
       highway++;
    else
      {
       nodesmem->xdata[i].id=~0;
       nothighway++;
      }

    if(!((i+1)%10000))
      {
       printf("\rChecking: Nodes=%d Highway=%d not-Highway=%d",i+1,highway,nothighway);
       fflush(stdout);
      }
   }

 printf("\rChecked: Nodes=%d Highway=%d not-Highway=%d  \n",nodesmem->number,highway,nothighway);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Fix the node indexes to the segments.

  NodesMem* nodesmem The set of nodes to process.

  SegmentsMem *segmentsmem The list of segments to use.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

void FixupNodes(NodesMem *nodesmem,SegmentsMem* segmentsmem,int iteration)
{
 int i;

 assert(nodesmem->sorted);      /* Must be sorted */

 for(i=0;i<nodesmem->number;i++)
   {
    SegmentEx *firstseg=FindFirstSegment(segmentsmem,nodesmem->xdata[i].id);

    nodesmem->xdata[i].node.firstseg=IndexSegmentEx(segmentsmem,firstseg);

    if(nodesmem->xdata[i].super==iteration)
       nodesmem->xdata[i].node.firstseg|=SUPER_NODE;

    if(!((i+1)%10000))
      {
       printf("\rFixing Nodes: Nodes=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rFixed Nodes: Nodes=%d \n",nodesmem->number);
 fflush(stdout);
}
