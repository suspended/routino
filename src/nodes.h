/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.h,v 1.18 2009-02-06 20:23:32 amb Exp $

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
typedef struct _NodeX
{
 node_t   id;                   /*+ The node identifier. +*/
 float    latitude;             /*+ The node latitude. +*/
 float    longitude;            /*+ The node longitude. +*/
 int      super;                /*+ A marker for super nodes. +*/

 Node     node;                 /*+ The real node data. +*/
}
 NodeX;

/*+ A structure containing a set of nodes (mmap format). +*/
typedef struct _Nodes
{
 uint32_t number;               /*+ How many entries are used in total? +*/

 uint32_t latbins;              /*+ The number of bins containing latitude. +*/
 uint32_t lonbins;              /*+ The number of bins containing longitude. +*/

 float    latzero;              /*+ The latitude of the SW corner of the first bin. +*/
 float    lonzero;              /*+ The longitude of the SW corner of the first bin. +*/

 index_t *offsets;              /*+ The offset of the first node in each bin. +*/

 Node    *nodes;                /*+ An array of nodes. +*/

 void    *data;                 /*+ The memory mapped data. +*/
}
 Nodes;

/*+ A structure containing a set of nodes (memory format). +*/
typedef struct _NodesX
{
 uint32_t sorted;               /*+ Is the data sorted and therefore searchable? +*/
 uint32_t alloced;              /*+ How many entries are allocated? +*/
 uint32_t xnumber;              /*+ How many entries are used from those allocated? +*/
 uint32_t number;               /*+ How many entries are still useful? +*/

 float    lat_min;              /*+ The minimum latitude of the set of nodes. +*/
 float    lat_max;              /*+ The maximum latitude of the set of nodes. +*/
 float    lon_min;              /*+ The minimum longitude of the set of nodes. +*/
 float    lon_max;              /*+ The maximum longitude of the set of nodes. +*/

 NodeX   *xdata;                /*+ The extended node data (unsorted). +*/
 NodeX  **gdata;                /*+ The extended node data (sorted geographically). +*/
 NodeX  **idata;                /*+ The extended node data (sorted by ID). +*/
}
 NodesX;


/* Functions */

#include "segments.h"


Nodes *LoadNodeList(const char *filename);

Node *FindNode(Nodes* nodes,float latitude,float longitude);

void GetLatLong(Nodes *nodes,Node *node,float *latitude,float *longitude);

NodesX *NewNodeList(void);

void SaveNodeList(NodesX *nodesx,const char *filename);

NodeX *FindNodeX(NodesX* nodesx,node_t id);

Node *AppendNode(NodesX* nodesx,node_t id,float latitude,float longitude);

void SortNodeList(NodesX *nodesx);

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx);

void MarkSuperNodes(NodesX *nodesx,int iteration);

void IndexNodes(NodesX *nodesx,SegmentsX* segmentsx);


#endif /* NODES_H */
