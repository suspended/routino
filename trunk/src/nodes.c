/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.c,v 1.4 2009-01-07 19:21:06 amb Exp $

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


/* Functions */

static int sort_by_id(Node *a,Node *b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new node list.

  NodesMem *NewNodeList Returns the node list.
  ++++++++++++++++++++++++++++++++++++++*/

NodesMem *NewNodeList(void)
{
 NodesMem *nodes;
 int i;

 nodes=(NodesMem*)calloc(sizeof(NodesMem),1);

 nodes->alloced=INCREMENT_NODES;

 for(i=0;i<NBINS_NODES;i++)
    nodes->bins[i]=(Node*)malloc(nodes->alloced*sizeof(Node));

 return(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Load in a node list from a file.

  NodesFile* LoadNodeList Returns the node list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

NodesFile *LoadNodeList(const char *filename)
{
 return((NodesFile*)MapFile(filename));
}


/*++++++++++++++++++++++++++++++++++++++
  Save the node list to a file.

  NodesFile* SaveNodeList Returns the node list that has just been saved.

  NodesMem* nodesm The set of nodes to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

NodesFile *SaveNodeList(NodesMem* nodesm,const char *filename)
{
 NodesFile nodesf;
 int i;

 assert(nodesm->sorted);        /* Must be sorted */

 nodesf.offset[0]=0;
 for(i=1;i<=NBINS_NODES;i++)
    nodesf.offset[i]=nodesf.offset[i-1]+nodesm->number[i-1];

 if(WriteFile(filename,(void*)&nodesf,sizeof(nodesf.offset),0))
    assert(0);

 for(i=0;i<NBINS_NODES;i++)
    if(WriteFile(filename,(void*)nodesm->bins[i],nodesm->number[i]*sizeof(Node),1))
       assert(0);

 for(i=0;i<NBINS_NODES;i++)
    free(nodesm->bins[i]);

 free(nodesm);

 return(LoadNodeList(filename));
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node.

  Node *FindNode Returns a pointer to the node with the specified id.

  NodesFile* nodes The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

Node *FindNode(NodesFile* nodes,node_t id)
{
 int bin=id%NBINS_NODES;
 int start=nodes->offset[bin];
 int end=nodes->offset[bin+1]-1;
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

 if(end<start)                      /* There are no nodes */
    return(NULL);
 else if(id<nodes->nodes[start].id) /* Check key is not before start */
    return(NULL);
 else if(id>nodes->nodes[end].id)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;               /* Choose mid point */

       if(nodes->nodes[mid].id<id)      /* Mid point is too low */
          start=mid+1;
       else if(nodes->nodes[mid].id>id) /* Mid point is too high */
          end=mid-1;
       else                             /* Mid point is correct */
          return(&nodes->nodes[mid]);
      }
    while((end-start)>1);

    if(nodes->nodes[start].id==id)      /* Start is correct */
       return(&nodes->nodes[start]);

    if(nodes->nodes[end].id==id)        /* End is correct */
       return(&nodes->nodes[end]);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a node to a newly created node list (unsorted).

  NodesMem* nodes The set of nodes to process.

  node_t id The node identification.

  latlong_t latitude The latitude of the node.

  latlong_t longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendNode(NodesMem* nodes,node_t id,latlong_t latitude,latlong_t longitude)
{
 int i;
 int bin=id%NBINS_NODES;

 /* Check that the arrays have enough space. */

 if(nodes->number[bin]==nodes->alloced)
   {
    nodes->alloced+=INCREMENT_NODES;

    for(i=0;i<NBINS_NODES;i++)
       nodes->bins[i]=(Node*)realloc((void*)nodes->bins[i],nodes->alloced*sizeof(Node));
   }

 /* Insert the node */

 nodes->bins[bin][nodes->number[bin]].id=id;
 nodes->bins[bin][nodes->number[bin]].latitude=latitude;
 nodes->bins[bin][nodes->number[bin]].longitude=longitude;

 nodes->number[bin]++;

 nodes->sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.

  NodesMem* nodes The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesMem* nodes)
{
 int i;

 for(i=0;i<NBINS_NODES;i++)
    qsort(nodes->bins[i],nodes->number[i],sizeof(Node),(int (*)(const void*,const void*))sort_by_id);

 nodes->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into id order.

  int sort_by_id Returns the comparison of the id fields.

  Node *a The first Node.

  Node *b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(Node *a,Node *b)
{
 node_t a_id=a->id;
 node_t b_id=b->id;

 return(a_id-b_id);
}
