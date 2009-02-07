/***************************************
 $Header: /home/amb/CVS/routino/src/nodesx.h,v 1.1 2009-02-07 15:56:38 amb Exp $

 A header file for the extended nodes.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef NODESX_H
#define NODESX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "nodes.h"


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _NodeX
{
 node_t   id;                   /*+ The node identifier. +*/
 float    latitude;             /*+ The node latitude. +*/
 float    longitude;            /*+ The node longitude. +*/
 int      super;                /*+ A marker for super nodes. +*/

 Node     node;                 /*+ The real node data. +*/
};

/*+ A structure containing a set of nodes (memory format). +*/
struct _NodesX
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
};


/* Functions */

NodesX *NewNodeList(void);

void SaveNodeList(NodesX *nodesx,const char *filename);

NodeX *FindNodeX(NodesX* nodesx,node_t id);

Node *AppendNode(NodesX* nodesx,node_t id,float latitude,float longitude);

void SortNodeList(NodesX *nodesx);

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx);

void MarkSuperNodes(NodesX *nodesx,int iteration);

void IndexNodes(NodesX *nodesx,SegmentsX* segmentsx);


#endif /* NODESX_H */
