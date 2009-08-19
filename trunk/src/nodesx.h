/***************************************
 $Header: /home/amb/CVS/routino/src/nodesx.h,v 1.12 2009-08-19 18:02:08 amb Exp $

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


/* Data structures */


/*+ An extended structure used for processing. +*/
struct _NodeX
{
 node_t    id;                  /*+ The node identifier. +*/

 latlong_t latitude;            /*+ The node latitude. +*/
 latlong_t longitude;           /*+ The node longitude. +*/

 uint8_t   super;               /*+ A marker for super nodes. +*/
};

/*+ A structure containing a set of nodes (memory format). +*/
struct _NodesX
{
 int32_t   row;                 /*+ How many rows are allocated? +*/
 uint32_t  col;                 /*+ How many columns are used in the last row? +*/

 NodeX   **xdata;               /*+ The extended node data (unsorted). +*/

 uint32_t  number;              /*+ How many entries are still useful? +*/

 NodeX   **gdata;               /*+ The extended node data (sorted geographically). +*/
 NodeX   **idata;               /*+ The extended node data (sorted by ID). +*/

 Node     *ndata;               /*+ The actual nodes (same order as idata). +*/
};


/* Functions */

NodesX *NewNodeList(void);
void FreeNodeList(NodesX *nodesx);

void SaveNodeList(NodesX *nodesx,const char *filename);

NodeX **FindNodeX(NodesX* nodesx,node_t id);

void AppendNode(NodesX* nodesx,node_t id,double latitude,double longitude);

void SortNodeList(NodesX *nodesx);

void SortNodeListGeographically(NodesX* nodesx);

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx);

void CreateRealNodes(NodesX *nodesx,int iteration);

void IndexNodes(NodesX *nodesx,SegmentsX *segmentsx);


#endif /* NODESX_H */
