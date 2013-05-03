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


#include <stdlib.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "relations.h"

#include "cache.h"


#if SLIM


#define CACHEWIDTH 2048
#define CACHEDEPTH   16


/*+ A cache of Nodes. +*/
struct _NodeCache
{
 int     first  [CACHEWIDTH];             /*+ The first entry to fill +*/

 Node    data   [CACHEWIDTH][CACHEDEPTH]; /*+ The array of nodes. +*/
 index_t indices[CACHEWIDTH][CACHEDEPTH]; /*+ The array of indexes. +*/

 index_t accesses;
 index_t cached;
};

/*+ A cache of Segments. +*/
struct _SegmentCache
{
 int     first  [CACHEWIDTH];             /*+ The first entry to fill +*/

 Segment data   [CACHEWIDTH][CACHEDEPTH]; /*+ The array of segments. +*/
 index_t indices[CACHEWIDTH][CACHEDEPTH]; /*+ The array of indexes. +*/

 index_t accesses;
 index_t cached;
};

/*+ A cache of Ways. +*/
struct _WayCache
{
 int     first  [CACHEWIDTH];             /*+ The first entry to fill +*/

 Way     data   [CACHEWIDTH][CACHEDEPTH]; /*+ The array of ways. +*/
 index_t indices[CACHEWIDTH][CACHEDEPTH]; /*+ The array of indexes. +*/

 index_t accesses;
 index_t cached;
};

/*+ A cache of Turn Relations. +*/
struct _TurnRelationCache
{
 int     first  [CACHEWIDTH];             /*+ The first entry to fill +*/

 TurnRelation data   [CACHEWIDTH][CACHEDEPTH]; /*+ The array of turn relations. +*/
 index_t      indices[CACHEWIDTH][CACHEDEPTH]; /*+ The array of indexes. +*/

 index_t accesses;
 index_t cached;
};



NodeCache *NewNodeCache(void)
{
 NodeCache *cache;

 cache=(NodeCache*)malloc(sizeof(NodeCache));

 InvalidateNodeCache(cache);

 return(cache);
}


void DeleteNodeCache(NodeCache *cache)
{
 printf("DeleteNodeCache access=%ld cached=%ld hit-rate=%.1f%%\n",cache->accesses,cache->cached,100*(double)cache->cached/(double)cache->accesses);

 free(cache);
}


Node *FetchCachedNode(Nodes *nodes,index_t index)
{
 int row=index%CACHEWIDTH;
 int col;

 nodes->cache->accesses++;

 for(col=0;col<CACHEDEPTH;col++)
    if(nodes->cache->indices[row][col]==index)
      {
       nodes->cache->cached++;
       return(&nodes->cache->data[row][col]);
      }

 col=nodes->cache->first[row];

 nodes->cache->first[row]=(nodes->cache->first[row]+1)%CACHEDEPTH;

 SeekReadFile(nodes->fd,&nodes->cache->data[row][col],sizeof(Node),nodes->nodesoffset+(off_t)index*sizeof(Node));

 nodes->cache->indices[row][col]=index;

 return(&nodes->cache->data[row][col]);
}


void InvalidateNodeCache(NodeCache *cache)
{
 int row,col;

 for(row=0;row<CACHEWIDTH;row++)
   {
    cache->first[row]=0;

    for(col=0;col<CACHEDEPTH;col++)
       cache->indices[row][col]=NO_NODE;
   }
}


SegmentCache *NewSegmentCache(void)
{
 SegmentCache *cache;

 cache=(SegmentCache*)malloc(sizeof(SegmentCache));

 InvalidateSegmentCache(cache);

 return(cache);
}


void DeleteSegmentCache(SegmentCache *cache)
{
 printf("DeleteSegmentCache access=%ld cached=%ld hit-rate=%.1f%%\n",cache->accesses,cache->cached,100*(double)cache->cached/(double)cache->accesses);

 free(cache);
}


Segment *FetchCachedSegment(Segments *segments,index_t index)
{
 int row=index%CACHEWIDTH;
 int col;

 segments->cache->accesses++;

 for(col=0;col<CACHEDEPTH;col++)
    if(segments->cache->indices[row][col]==index)
      {
       segments->cache->cached++;
       return(&segments->cache->data[row][col]);
      }

 col=segments->cache->first[row];

 segments->cache->first[row]=(segments->cache->first[row]+1)%CACHEDEPTH;

 SeekReadFile(segments->fd,&segments->cache->data[row][col],sizeof(Segment),sizeof(SegmentsFile)+(off_t)index*sizeof(Segment));

 segments->cache->indices[row][col]=index;

 return(&segments->cache->data[row][col]);
}


void InvalidateSegmentCache(SegmentCache *cache)
{
 int row,col;

 for(row=0;row<CACHEWIDTH;row++)
   {
    cache->first[row]=0;

    for(col=0;col<CACHEDEPTH;col++)
       cache->indices[row][col]=NO_SEGMENT;
   }
}


WayCache *NewWayCache(void)
{
 WayCache *cache;

 cache=(WayCache*)malloc(sizeof(WayCache));

 InvalidateWayCache(cache);

 return(cache);
}


void DeleteWayCache(WayCache *cache)
{
 printf("DeleteWayCache access=%ld cached=%ld hit-rate=%.1f%%\n",cache->accesses,cache->cached,100*(double)cache->cached/(double)cache->accesses);

 free(cache);
}


Way *FetchCachedWay(Ways *ways,index_t index)
{
 int row=index%CACHEWIDTH;
 int col;

 ways->cache->accesses++;

 for(col=0;col<CACHEDEPTH;col++)
    if(ways->cache->indices[row][col]==index)
      {
       ways->cache->cached++;
       return(&ways->cache->data[row][col]);
      }

 col=ways->cache->first[row];

 ways->cache->first[row]=(ways->cache->first[row]+1)%CACHEDEPTH;

 SeekReadFile(ways->fd,&ways->cache->data[row][col],sizeof(Way),sizeof(WaysFile)+(off_t)index*sizeof(Way));

 ways->cache->indices[row][col]=index;

 return(&ways->cache->data[row][col]);
}


void InvalidateWayCache(WayCache *cache)
{
 int row,col;

 for(row=0;row<CACHEWIDTH;row++)
   {
    cache->first[row]=0;

    for(col=0;col<CACHEDEPTH;col++)
       cache->indices[row][col]=NO_WAY;
   }
}


TurnRelationCache *NewTurnRelationCache(void)
{
 TurnRelationCache *cache;

 cache=(TurnRelationCache*)malloc(sizeof(TurnRelationCache));

 InvalidateTurnRelationCache(cache);

 return(cache);
}


void DeleteTurnRelationCache(TurnRelationCache *cache)
{
 printf("DeleteTurnRelationCache access=%ld cached=%ld hit-rate=%.1f%%\n",cache->accesses,cache->cached,100*(double)cache->cached/(double)cache->accesses);

 free(cache);
}


TurnRelation *FetchCachedTurnRelation(Relations *relations,index_t index)
{
 int row=index%CACHEWIDTH;
 int col;

 relations->cache->accesses++;

 for(col=0;col<CACHEDEPTH;col++)
    if(relations->cache->indices[row][col]==index)
      {
       relations->cache->cached++;
       return(&relations->cache->data[row][col]);
      }

 col=relations->cache->first[row];

 relations->cache->first[row]=(relations->cache->first[row]+1)%CACHEDEPTH;

 SeekReadFile(relations->fd,&relations->cache->data[row][col],sizeof(TurnRelation),relations->troffset+(off_t)index*sizeof(TurnRelation));

 relations->cache->indices[row][col]=index;

 return(&relations->cache->data[row][col]);
}


void InvalidateTurnRelationCache(TurnRelationCache *cache)
{
 int row,col;

 for(row=0;row<CACHEWIDTH;row++)
   {
    cache->first[row]=0;

    for(col=0;col<CACHEDEPTH;col++)
       cache->indices[row][col]=NO_RELATION;
   }
}

#endif
