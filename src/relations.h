/***************************************
 $Header: /home/amb/CVS/routino/src/relations.h,v 1.1 2010-12-21 14:55:18 amb Exp $

 A header file for the relations.

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


#ifndef RELATIONS_H
#define RELATIONS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"

#include "files.h"
#include "profiles.h"


/* Data structures */


/*+ A structure containing a single relation. +*/
struct _TurnRelation
{
 index_t      from;             /*+ The node that the path comes from. +*/
 index_t      via;              /*+ The node that the path goes via. +*/
 index_t      to;               /*+ The node that the path goes to. +*/

 transports_t except;           /*+ The types of transports that that this relation does not apply to. +*/
};


/*+ A structure containing the header from the file. +*/
typedef struct _RelationsFile
{
 index_t       trnumber;        /*+ How many turn relations in total? +*/
}
 RelationsFile;


/*+ A structure containing a set of relations (and pointers to mmap file). +*/
struct _Relations
{
 RelationsFile file;            /*+ The header data from the file. +*/

#if !SLIM

 void         *data;            /*+ The memory mapped data. +*/

 TurnRelation *turnrelations;   /*+ An array of nodes. +*/

#else

 int           fd;              /*+ The file descriptor for the file. +*/

 off_t         troffset;        /*+ The offset of the turn relations in the file. +*/

 TurnRelation  cached;          /*+ The cached relations. +*/

#endif
};


/* Functions */

Relations *LoadRelationList(const char *filename);


/* Macros and inline functions */

#if !SLIM

/*+ Return a Relation pointer given a set of relations and an index. +*/
#define LookupTurnRelation(xxx,yyy)   (&(xxx)->turnrelations[yyy])

#else

static TurnRelation *LookupTurnRelation(Relations *relations,index_t index);


/*++++++++++++++++++++++++++++++++++++++
  Find the Relation information for a particular relation.

  TurnRelation *LookupTurnRelation Returns a pointer to the cached relation information.

  Relations *relations The relations structure to use.

  index_t index The index of the relation.
  ++++++++++++++++++++++++++++++++++++++*/

static inline TurnRelation *LookupTurnRelation(Relations *relations,index_t index)
{
 SeekFile(relations->fd,relations->troffset+(off_t)index*sizeof(TurnRelation));

 ReadFile(relations->fd,&relations->cached,sizeof(TurnRelation));

 return(&relations->cached);
}

#endif


#endif /* RELATIONS_H */
