/***************************************
 $Header: /home/amb/CVS/routino/src/fakes.h,v 1.1 2010-11-27 14:56:49 amb Exp $

 Header file for fake node and segment function prototypes

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2010 Andrew M. Bishop

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


#ifndef FAKES_H
#define FAKES_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* In fakes.c */

/*+ Return true if this is a fake node. +*/
#define IsFakeNode(xxx)    ((xxx)>=NODE_FAKE)

/*+ Return true if this is a fake segment. +*/
#define IsFakeSegment(xxx) ((xxx)>=SEGMENT_FAKE)

index_t CreateFakes(Nodes *nodes,int point,Segment *segment,index_t node1,index_t node2,distance_t dist1,distance_t dist2);

void GetFakeLatLong(index_t node, double *latitude,double *longitude);

Segment *FirstFakeSegment(index_t node);
Segment *NextFakeSegment(Segment *segment,index_t node);
Segment *ExtraFakeSegment(index_t node,index_t fakenode);

Segment *LookupFakeSegment(index_t index);
index_t IndexFakeSegment(Segment *segment);


#endif /* FAKES_H */
