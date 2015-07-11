/***************************************
 A header file for the relations.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2015 Andrew M. Bishop

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


#ifndef RELATIONS_H
#define RELATIONS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>
#include <sys/types.h>

#include "types.h"

#include "cache.h"
#include "files.h"
#include "profiles.h"


/* Data structures */


/*+ A structure containing a single relation. +*/
struct _TurnRelation
{
 index_t      from;             /*+ The segment that the path comes from. +*/
 index_t      via;              /*+ The node that the path goes via. +*/
 index_t      to;               /*+ The segment that the path goes to. +*/

 transports_t except;           /*+ The types of transports that that this relation does not apply to. +*/
};


/*+ A structure containing the header from the file. +*/
typedef struct _RelationsFile
{
 index_t       trnumber;        /*+ The number of turn relations in total. +*/
}
 RelationsFile;


/*+ A structure containing a set of relations (and pointers to mmap file). +*/
struct _Relations
{
 RelationsFile file;            /*+ The header data from the file. +*/

#if !SLIM

 char         *data;            /*+ The memory mapped data. +*/

 TurnRelation *turnrelations;   /*+ An array of nodes. +*/

#else

 int           fd;              /*+ The file descriptor for the file. +*/

 offset_t      troffset;        /*+ The offset of the turn relations in the file. +*/

 TurnRelation  cached[2];       /*+ Two cached relations read from the file in slim mode. +*/

 TurnRelationCache *cache;      /*+ A RAM cache of turn relations read from the file. +*/

#endif

 index_t       via_start;       /*+ The first via node in the file. +*/
 index_t       via_end;         /*+ The last via node in the file. +*/
};


/* Functions in relations.c */

Relations *LoadRelationList(const char *filename);

void DestroyRelationList(Relations *relations);

index_t FindFirstTurnRelation1(Relations *relations,index_t via);
index_t FindNextTurnRelation1(Relations *relations,index_t current);

index_t FindFirstTurnRelation2(Relations *relations,index_t via,index_t from);
index_t FindNextTurnRelation2(Relations *relations,index_t current);

int IsTurnAllowed(Relations *relations,index_t index,index_t via,index_t from,index_t to,transports_t transport);


/* Macros and inline functions */

#if !SLIM

/*+ Return a Relation pointer given a set of relations and an index. +*/
#define LookupTurnRelation(xxx,yyy,ppp)   (&(xxx)->turnrelations[yyy])

#else

/* Prototypes */

static inline TurnRelation *LookupTurnRelation(Relations *relations,index_t index,int position);

CACHE_NEWCACHE_PROTO(TurnRelation)
CACHE_DELETECACHE_PROTO(TurnRelation)
CACHE_FETCHCACHE_PROTO(TurnRelation)
CACHE_INVALIDATECACHE_PROTO(TurnRelation)

/* Data type */

CACHE_STRUCTURE(TurnRelation)

/* Inline functions */

CACHE_NEWCACHE(TurnRelation)
CACHE_DELETECACHE(TurnRelation)
CACHE_FETCHCACHE(TurnRelation)
CACHE_INVALIDATECACHE(TurnRelation)


/*++++++++++++++++++++++++++++++++++++++
  Find the Relation information for a particular relation.

  TurnRelation *LookupTurnRelation Returns a pointer to the cached relation information.

  Relations *relations The set of relations to use.

  index_t index The index of the relation.

  int position The position in the cache to store this result.
  ++++++++++++++++++++++++++++++++++++++*/

static inline TurnRelation *LookupTurnRelation(Relations *relations,index_t index,int position)
{
 relations->cached[position-1]=*FetchCachedTurnRelation(relations->cache,index,relations->fd,relations->troffset);

 return(&relations->cached[position-1]);
}

#endif


#endif /* RELATIONS_H */
