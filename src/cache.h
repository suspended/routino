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


#if SLIM

#ifndef CACHE_H
#define CACHE_H    /*+ To stop multiple inclusions. +*/

#include <unistd.h>

#include "types.h"


#define CACHE_STRUCTURE_DEF(type) typedef struct _##type##Cache type##Cache;

#define CACHE_NEWCACHE_PROTO(type) type##Cache *New##type##Cache(void);

#define CACHE_DELETECACHE_PROTO(type) void Delete##type##Cache(type##Cache *cache);

#define CACHE_FETCHCACHE_PROTO(type) type *FetchCached##type(type##Cache *cache,index_t index,int fd,off_t offset);

#define CACHE_INVALIDATECACHE_PROTO(type) void Invalidate##type##Cache(type##Cache *cache);



/*+ Data structure definitions to hold caches. +*/
CACHE_STRUCTURE_DEF(Node)
CACHE_STRUCTURE_DEF(Segment)
CACHE_STRUCTURE_DEF(Way)
CACHE_STRUCTURE_DEF(TurnRelation)


/*+ Function prototypes to create a new cache data structure. +*/
CACHE_NEWCACHE_PROTO(Node)
CACHE_NEWCACHE_PROTO(Segment)
CACHE_NEWCACHE_PROTO(Way)
CACHE_NEWCACHE_PROTO(TurnRelation)


/*+ Function prototypes to delete a cache data structure. +*/
CACHE_DELETECACHE_PROTO(Node)
CACHE_DELETECACHE_PROTO(Segment)
CACHE_DELETECACHE_PROTO(Way)
CACHE_DELETECACHE_PROTO(TurnRelation)


/*+ Function prototypes to fetch an item from a cache data structure. +*/
CACHE_FETCHCACHE_PROTO(Node)
CACHE_FETCHCACHE_PROTO(Segment)
CACHE_FETCHCACHE_PROTO(Way)
CACHE_FETCHCACHE_PROTO(TurnRelation)


/*+ Function prototypes to invalidate the contents of a cache data structure. +*/
CACHE_INVALIDATECACHE_PROTO(Node)
CACHE_INVALIDATECACHE_PROTO(Segment)
CACHE_INVALIDATECACHE_PROTO(Way)
CACHE_INVALIDATECACHE_PROTO(TurnRelation)


#endif /* CACHE_H */

#endif  /* SLIM */
