/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.c,v 1.7 2009-01-11 09:33:59 amb Exp $

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

 nodes=(NodesMem*)malloc(sizeof(NodesMem));

 nodes->alloced=INCREMENT_NODES;
 nodes->number=0;
 nodes->sorted=0;

 nodes->nodes=(Nodes*)malloc(sizeof(Nodes)+nodes->alloced*sizeof(Node));

 return(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Load in a node list from a file.

  Nodes* LoadNodeList Returns the node list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Nodes *LoadNodeList(const char *filename)
{
 return((Nodes*)MapFile(filename));
}


/*++++++++++++++++++++++++++++++++++++++
  Save the node list to a file.

  Nodes* SaveNodeList Returns the node list that has just been saved.

  NodesMem* nodes The set of nodes to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

Nodes *SaveNodeList(NodesMem* nodes,const char *filename)
{
#ifdef NBINS_NODES
 int i,bin=0;
#endif

 assert(nodes->sorted);        /* Must be sorted */

 nodes->nodes->number=nodes->number;

#ifdef NBINS_NODES
 for(i=0;i<nodes->number;i++)
    for(;bin<=(nodes->nodes->nodes[i].id%NBINS_NODES);bin++)
       nodes->nodes->offset[bin]=i;

 for(;bin<=NBINS_NODES;bin++)
    nodes->nodes->offset[bin]=nodes->number;
#endif

 if(WriteFile(filename,(void*)nodes->nodes,sizeof(Nodes)-sizeof(nodes->nodes->nodes)+nodes->number*sizeof(Node)))
    assert(0);

 free(nodes->nodes);
 free(nodes);

 return(LoadNodeList(filename));
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node.

  Node *FindNode Returns a pointer to the node with the specified id.

  Nodes* nodes The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

Node *FindNode(Nodes* nodes,node_t id)
{
#ifdef NBINS_NODES
 int bin=id%NBINS_NODES;
 int start=nodes->offset[bin];
 int end=nodes->offset[bin+1]-1;
#else
 int start=0;
 int end=nodes->number-1;
#endif
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

  Node *AppendNode Return a pointer to the new node.

  NodesMem* nodes The set of nodes to process.

  node_t id The node identification.

  latlong_t latitude The latitude of the node.

  latlong_t longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

Node *AppendNode(NodesMem* nodes,node_t id,latlong_t latitude,latlong_t longitude)
{
 /* Check that the array has enough space. */

 if(nodes->number==nodes->alloced)
   {
    nodes->alloced+=INCREMENT_NODES;

    nodes->nodes=(Nodes*)realloc((void*)nodes->nodes,sizeof(Nodes)+nodes->alloced*sizeof(Node));
   }

 /* Insert the node */

 nodes->nodes->nodes[nodes->number].id=id;
 nodes->nodes->nodes[nodes->number].latitude=latitude;
 nodes->nodes->nodes[nodes->number].longitude=longitude;

 nodes->number++;

 nodes->sorted=0;

 return(&nodes->nodes->nodes[nodes->number-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.

  NodesMem* nodes The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesMem* nodes)
{
 qsort(nodes->nodes->nodes,nodes->number,sizeof(Node),(int (*)(const void*,const void*))sort_by_id);

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

#ifdef NBINS_NODES
 int a_bin=a->id%NBINS_NODES;
 int b_bin=b->id%NBINS_NODES;

 if(a_bin!=b_bin)
    return(a_bin-b_bin);
#endif

 return(a_id-b_id);
}
