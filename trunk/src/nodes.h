/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.h,v 1.14 2009-01-31 15:32:41 amb Exp $

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

#include "types.h"


/* Simple Types */


/*+ A node identifier. +*/
typedef uint32_t node_t;


/* Data structures */


/*+ An extended structure used for processing. +*/
typedef struct _NodeEx
{
 node_t    id;                  /*+ The node identifier. +*/
 int       super;               /*+ A marker for super nodes. +*/

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
void SaveNodeList(NodesMem *nodesmem,const char *filename);

NodeEx *FindNode(NodesMem* nodesmem,node_t id);

NodeEx *AppendNode(NodesMem *nodesmem,node_t id,latlong_t latitude,latlong_t longitude);

void SortNodeList(NodesMem *nodesmem);

void RemoveNonHighwayNodes(NodesMem *nodes,SegmentsMem *segments);

void FixupNodes(NodesMem *nodesmem,SegmentsMem* segmentsmem,int iteration);

#define LookupNodeEx(xxx,yyy) (&(xxx)->xdata[NODE(yyy)])

#define IndexNodeEx(xxx,yyy)  ((yyy)-&(xxx)->xdata[0])


#endif /* NODES_H */
