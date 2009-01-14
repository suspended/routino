/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.h,v 1.7 2009-01-14 19:30:50 amb Exp $

 A header file for the nodes.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef NODES_H
#define NODES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>


/* Constants */


#if 1 /* set to 0 to use a flat array, 1 for indexed. */

/*+ The number of bins for nodes. +*/
#define NBINS_NODES 2048

#else

#undef NBINS_NODES

#endif

/*+ The array size increment for nodes. +*/
#define INCREMENT_NODES 1024*1024


/* Simple Types */


/*+ A node identifier. +*/
typedef uint32_t node_t;

/*+ A node latitude or longitude. +*/
typedef float latlong_t;


/* Data structures */


/*+ A structure containing a single node. +*/
typedef struct _Node
{
 node_t    id;                  /*+ The node identifier. +*/
 latlong_t latitude;            /*+ The node latitude. +*/
 latlong_t longitude;           /*+ The node longitude. +*/
}
 Node;

/*+ A structure containing a set of nodes (mmap format). +*/
typedef struct _Nodes
{
 uint32_t number;               /*+ How many entries are used? +*/
#ifdef NBINS_NODES
 uint32_t offset[NBINS_NODES+1];/*+ An offset to the first entry in each bin. +*/
#endif
 Node     nodes[1];             /*+ An array of nodes whose size is not limited to 1
                                    (i.e. may overflow the end of this structure). +*/
}
 Nodes;

/*+ A structure containing a set of nodes (memory format). +*/
typedef struct _NodesMem
{
 uint32_t alloced;              /*+ How many entries are allocated? +*/
 uint32_t number;               /*+ How many entries are used? +*/
 uint32_t sorted;               /*+ Is the data sorted and therefore searchable? +*/

 Nodes   *nodes;                /*+ The real data +*/
}
 NodesMem;


/* Functions */

#include "segments.h"


NodesMem *NewNodeList(void);

Nodes *LoadNodeList(const char *filename);
Nodes *SaveNodeList(NodesMem *nodes,const char *filename);

Node *FindNode(Nodes *nodes,node_t id);

Node *AppendNode(NodesMem *nodes,node_t id,latlong_t latitude,latlong_t longitude);

void SortNodeList(NodesMem *nodes);

void RemoveNonWayNodes(NodesMem *nodesmem,Nodes *nodes,Segments *segments);

#define LookupNode(xxx,yyy) (&xxx->nodes[yyy])


#endif /* NODES_H */
