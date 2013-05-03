/***************************************
 Functions to maintain an in-RAM cache of on-disk data for slim mode.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2013 Andrew M. Bishop

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


#ifndef CACHE_H
#define CACHE_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* Node cache */

typedef struct _NodeCache NodeCache;
typedef struct _SegmentCache SegmentCache;
typedef struct _WayCache WayCache;
typedef struct _TurnRelationCache TurnRelationCache;


NodeCache *NewNodeCache(void);

void DeleteNodeCache(NodeCache *cache);

Node *FetchCachedNode(Nodes *nodes,index_t id);

void InvalidateNodeCache(NodeCache *cache);


SegmentCache *NewSegmentCache(void);

void DeleteSegmentCache(SegmentCache *cache);

Segment *FetchCachedSegment(Segments *nodes,index_t id);

void InvalidateSegmentCache(SegmentCache *cache);


WayCache *NewWayCache(void);

void DeleteWayCache(WayCache *cache);

Way *FetchCachedWay(Ways *nodes,index_t id);

void InvalidateWayCache(WayCache *cache);


TurnRelationCache *NewTurnRelationCache(void);

void DeleteTurnRelationCache(TurnRelationCache *cache);

TurnRelation *FetchCachedTurnRelation(Relations *nodes,index_t id);

void InvalidateTurnRelationCache(TurnRelationCache *cache);


#endif /* CACHE_H */
