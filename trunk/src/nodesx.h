/***************************************
 $Header: /home/amb/CVS/routino/src/nodesx.h,v 1.4 2009-06-25 18:17:58 amb Exp $

 A header file for the extended nodes.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008,2009 Andrew M. Bishop

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


#ifndef NODESX_H
#define NODESX_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "typesx.h"
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

void IndexNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx);


#endif /* NODESX_H */
