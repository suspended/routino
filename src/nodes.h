/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.h,v 1.10 2009-01-27 18:22:37 amb Exp $

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


/*+ The array size increment for nodes - expect ~8,000,000 nodes. +*/
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
 uint32_t   firstseg;           /*+ The index of the first segment. +*/
 latlong_t  latitude;           /*+ The node latitude. +*/
 latlong_t  longitude;          /*+ The node longitude. +*/
}
 Node;

/*+ An extended structure used for processing. +*/
typedef struct _NodeEx
{
 node_t    id;                  /*+ The node identifier. +*/

 Node      node;                /*+ The real node data. +*/
}
 NodeEx;

/*+ A structure containing a set of nodes (mmap format). +*/
typedef struct _Nodes
{
 uint32_t number;               /*+ How many entries are used in total? +*/

 Node    *nodes;                /*+ An array of nodes. +*/

 void    *data;                 /*+ The memory mapped data. +*/
}
 Nodes;

/*+ A structure containing a set of nodes (memory format). +*/
typedef struct _NodesMem
{
 uint32_t sorted;               /*+ Is the data sorted and therefore searchable? +*/
 uint32_t alloced;              /*+ How many entries are allocated? +*/
 uint32_t number;               /*+ How many entries are used? +*/

 NodeEx  *xdata;                /*+ The extended node data. +*/
}
 NodesMem;


/* Functions */

#include "segments.h"


NodesMem *NewNodeList(void);

Nodes *LoadNodeList(const char *filename);
Nodes *SaveNodeList(NodesMem *nodesmem,const char *filename);

void DropNodeList(Nodes *nodes);

uint32_t FindNode(NodesMem* nodesmem,node_t id);

Node *AppendNode(NodesMem *nodesmem,node_t id,latlong_t latitude,latlong_t longitude);

void SortNodeList(NodesMem *nodesmem);

void RemoveNonHighwayNodes(NodesMem *nodes,SegmentsMem *segments);

void FixupNodes(NodesMem *nodesmem,SegmentsMem* segmentsmem);

#define LookupNode(xxx,yyy)   (&(xxx)->nodes[yyy])

#define FirstSegment(xxx,yyy) ((xxx)->nodes[yyy].firstseg)

#endif /* NODES_H */
