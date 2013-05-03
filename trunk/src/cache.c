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

#include <unistd.h>
#include <stdlib.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "relations.h"

#include "cache.h"


#define CACHEWIDTH 2048         /*+ The width of the cache. +*/
#define CACHEDEPTH   16         /*+ The depth of the cache. +*/


/*+ A macro to create a cache structure. +*/
#define CACHE_STRUCTURE(type) \
                              \
struct _##type##Cache                                                     \
{                                                                         \
 int     first  [CACHEWIDTH];             /*+ The first entry to fill +*/ \
                                                                          \
 type    data   [CACHEWIDTH][CACHEDEPTH]; /*+ The array of type##s. +*/   \
 index_t indices[CACHEWIDTH][CACHEDEPTH]; /*+ The array of indexes. +*/   \
                                                                          \
 index_t accesses;                                                        \
 index_t cached;                                                          \
};


/*+ A macro to create a function that creates a new cache data structure. +*/
#define CACHE_NEWCACHE(type) \
                             \
type##Cache *New##type##Cache(void)                     \
{                                                       \
 type##Cache *cache;                                    \
                                                        \
 cache=(type##Cache*)malloc(sizeof(type##Cache));       \
                                                        \
 Invalidate##type##Cache(cache);                        \
                                                        \
 return(cache);                                         \
}


/*+ A macro to create a function that deletes a cache data structure. +*/
#define CACHE_DELETECACHE(type) \
                                \
void Delete##type##Cache(type##Cache *cache)      \
{                                                 \
 printf("Delete" #type "Cache access=%ld cached=%ld hit-rate=%.1f%%\n",cache->accesses,cache->cached,100*(double)cache->cached/(double)cache->accesses); \
                                                  \
 free(cache);                                     \
}


/*+ A macro to create a function that fetches an item from a cache data structure. +*/
#define CACHE_FETCHCACHE(type) \
                               \
type *FetchCached##type(type##Cache *cache,index_t index,int fd,off_t offset) \
{                                                                       \
 int row=index%CACHEWIDTH;                                              \
 int col;                                                               \
                                                                        \
 cache->accesses++;                                                     \
                                                                        \
 for(col=0;col<CACHEDEPTH;col++)                                        \
    if(cache->indices[row][col]==index)                                 \
      {                                                                 \
       cache->cached++;                                                 \
       return(&cache->data[row][col]);                                  \
      }                                                                 \
                                                                        \
 col=cache->first[row];                                                 \
                                                                        \
 cache->first[row]=(cache->first[row]+1)%CACHEDEPTH;                    \
                                                                        \
 SeekReadFile(fd,&cache->data[row][col],sizeof(type),offset+(off_t)index*sizeof(type)); \
                                                                        \
 cache->indices[row][col]=index;                                        \
                                                                        \
 return(&cache->data[row][col]);                                        \
}


/*+ A macro to create a function that invalidates the contents of a cache data structure. +*/
#define CACHE_INVALIDATECACHE(type) \
                                    \
void Invalidate##type##Cache(type##Cache *cache)       \
{                                                      \
 int row,col;                                          \
                                                       \
 for(row=0;row<CACHEWIDTH;row++)                       \
   {                                                   \
    cache->first[row]=0;                               \
                                                       \
    for(col=0;col<CACHEDEPTH;col++)                    \
       cache->indices[row][col]=NO_NODE;               \
   }                                                   \
}


/*+ Data structures to hold caches. +*/
CACHE_STRUCTURE(Node)
CACHE_STRUCTURE(Segment)
CACHE_STRUCTURE(Way)
CACHE_STRUCTURE(TurnRelation)


/*+ Functions to create a new cache data structure. +*/
CACHE_NEWCACHE(Node)
CACHE_NEWCACHE(Segment)
CACHE_NEWCACHE(Way)
CACHE_NEWCACHE(TurnRelation)


/*+ Functions to delete a cache data structure. +*/
CACHE_DELETECACHE(Node)
CACHE_DELETECACHE(Segment)
CACHE_DELETECACHE(Way)
CACHE_DELETECACHE(TurnRelation)


/*+ Functions to fetch an item from a cache data structure. +*/
CACHE_FETCHCACHE(Node)
CACHE_FETCHCACHE(Segment)
CACHE_FETCHCACHE(Way)
CACHE_FETCHCACHE(TurnRelation)


/*+ Functions to invalidate the contents of a cache data structure. +*/
CACHE_INVALIDATECACHE(Node)
CACHE_INVALIDATECACHE(Segment)
CACHE_INVALIDATECACHE(Way)
CACHE_INVALIDATECACHE(TurnRelation)


#endif
