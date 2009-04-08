/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.h,v 1.22 2009-04-08 16:54:34 amb Exp $

 A header file for the nodes.

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


#ifndef NODES_H
#define NODES_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"


/* Data structures */


/*+ A structure containing a single node. +*/
struct _Node
{
 index_t    firstseg;           /*+ The index of the first segment. +*/

 ll_off_t   latoffset;          /*+ The node latitude offset within its bin. +*/
 ll_off_t   lonoffset;          /*+ The node longitude offset within its bin. +*/
};


/*+ A structure containing a set of nodes (mmap format). +*/
struct _Nodes
{
 uint32_t number;               /*+ How many entries are used in total? +*/

 uint32_t latbins;              /*+ The number of bins containing latitude. +*/
 uint32_t lonbins;              /*+ The number of bins containing longitude. +*/

 int32_t  latzero;              /*+ The latitude of the SW corner of the first bin. +*/
 int32_t  lonzero;              /*+ The longitude of the SW corner of the first bin. +*/

 index_t *offsets;              /*+ The offset of the first node in each bin. +*/

 Node    *nodes;                /*+ An array of nodes. +*/

 void    *data;                 /*+ The memory mapped data. +*/
};


/* Macros */


/*+ Return a Node pointer given a set of nodes and an index. +*/
#define LookupNode(xxx,yyy)    (&(xxx)->nodes[yyy])

/*+ Return an index for a Node pointer given a set of nodes. +*/
#define IndexNode(xxx,yyy)     ((yyy)-&(xxx)->nodes[0])

/*+ Return a Segment points given a Node pointer and a set of segments. +*/
#define FirstSegment(xxx,yyy)  LookupSegment((xxx),SEGMENT((yyy)->firstseg))

/*+ Return true if this is a super-node. +*/
#define IsSuperNode(xxx)       (((xxx)->firstseg)&SUPER_FLAG)


/* Functions */


Nodes *LoadNodeList(const char *filename);

Node *FindNode(Nodes* nodes,float latitude,float longitude,distance_t *distance);

void GetLatLong(Nodes *nodes,Node *node,float *latitude,float *longitude);


#endif /* NODES_H */
