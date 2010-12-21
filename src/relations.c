/***************************************
 $Header: /home/amb/CVS/routino/src/relations.c,v 1.2 2010-12-21 17:01:46 amb Exp $

 Relation data type functions.

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


#include <sys/types.h>
#include <stdlib.h>

#include "relations.h"

#include "files.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a relation list from a file.

  Relations* LoadRelationList Returns the relation list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Relations *LoadRelationList(const char *filename)
{
 Relations *relations;

 relations=(Relations*)malloc(sizeof(Relations));

#if !SLIM

 relations->data=MapFile(filename);

 /* Copy the RelationsFile header structure from the loaded data */

 relations->file=*((RelationsFile*)relations->data);

 /* Set the pointers in the Relations structure. */

 relations->turnrelations=(TurnRelation*)(relations->data+sizeof(RelationsFile));

#else

 relations->fd=ReOpenFile(filename);

 /* Copy the RelationsFile header structure from the loaded data */

 ReadFile(relations->fd,&relations->file,sizeof(RelationsFile));

 relations->troffset=sizeof(RelationsFile);

 relations->incache[0]=NO_RELATION;
 relations->incache[1]=NO_RELATION;
 relations->incache[2]=NO_RELATION;

#endif

 return(relations);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular turn relation in the file.

  index_t FindFirstTurnRelation1 Returns the index of the first turn relation matching via.

  Relations *relations The set of relations to process.

  index_t via The node that the route is going via.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindFirstTurnRelation1(Relations *relations,index_t via)
{
 TurnRelation *relation;
 int start=0;
 int end=relations->file.trnumber-1;
 int mid;
 int match=-1;

 /* Binary search - search key first match is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 if(end<start)                      /* There are no relations */
    return(NO_RELATION);

 relation=LookupTurnRelation(relations,start,1);

 if(via<relation->via)              /* Check key is not before start */
    return(NO_RELATION);

 relation=LookupTurnRelation(relations,end,2);

 if(via>relation->via)              /* Check key is not after end */
    return(NO_RELATION);

 do
   {
    mid=(start+end)/2;              /* Choose mid point */

    relation=LookupTurnRelation(relations,mid,3);

    if(relation->via<via)           /* Mid point is too low for 'via' */
       start=mid+1;
    else if(relation->via>via)      /* Mid point is too high for 'via' */
       end=mid-1;
    else                            /* Mid point is correct for 'from' */
      {
       match=mid;
       break;
      }
   }
 while((end-start)>1);

 if(match==-1)                      /* Check if start matches */
   {
    relation=LookupTurnRelation(relations,start,3);

    if(relation->via==via)
       match=start;
   }

 if(match==-1)                      /* Check if end matches */
   {
    relation=LookupTurnRelation(relations,end,3);

    if(relation->via==via)
       match=end;
   }

 if(match==-1)
    return(NO_RELATION);

 while(match>0)                     /* Search backwards for the first match */
   {
    relation=LookupTurnRelation(relations,match-1,3);

    if(relation->via==via)
       match--;
    else
       break;
   }

 return(match);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next matching turn relation in the file.

  index_t FindNextTurnRelation1 Returns the index of the next turn relation matching via.

  Relations *relations The set of relations to process.

  index_t current The current index of a relation that matches.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindNextTurnRelation1(Relations *relations,index_t current)
{
 TurnRelation *relation;
 index_t via;

 relation=LookupTurnRelation(relations,current,3);

 via=relation->via;

 current++;

 relation=LookupTurnRelation(relations,current,3);

 if(relation->via==via)
    return(current);
 else
    return(NO_RELATION);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular turn relation in the file.

  index_t FindFirstTurnRelation2 Returns the index of the first turn relation matching via and from.

  Relations *relations The set of relations to process.

  index_t via The node that the route is going via.

  index_t from The node that the route is coming from.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindFirstTurnRelation2(Relations *relations,index_t via,index_t from)
{
 TurnRelation *relation;
 int start=0;
 int end=relations->file.trnumber-1;
 int mid;
 int match=-1;

 /* Binary search - search key first match is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 if(end<start)                                /* There are no relations */
    return(NO_RELATION);

 relation=LookupTurnRelation(relations,start,1);

 if(via<relation->via || from<relation->from) /* Check keys are not before start */
    return(NO_RELATION);

 relation=LookupTurnRelation(relations,end,2);

 if(via>relation->via || from>relation->from) /* Check key is not after end */
    return(NO_RELATION);

 do
   {
    mid=(start+end)/2;                        /* Choose mid point */

    relation=LookupTurnRelation(relations,mid,3);

    if(relation->via<via)                     /* Mid point is too low for 'via' */
       start=mid+1;
    else if(relation->via>via)                /* Mid point is too high for 'via' */
       end=mid-1;
    else                                      /* Mid point is correct for 'via' */
      {
       if(relation->from<from)                /* Mid point is too low for 'from' */
          start=mid+1;
       else if(relation->from>from)           /* Mid point is too high for 'from' */
          end=mid-1;
       else                                   /* Mid point is correct for 'from' */
         {
          match=mid;
          break;
         }
      }
   }
 while((end-start)>1);

 if(match==-1)                                /* Check if start matches */
   {
    relation=LookupTurnRelation(relations,start,3);

    if(relation->via==via && relation->from==from)
       match=start;
   }

 if(match==-1)                                /* Check if end matches */
   {
    relation=LookupTurnRelation(relations,end,3);

    if(relation->via==via && relation->from==from)
       match=end;
   }

 if(match==-1)
    return(NO_RELATION);

 while(match>0)                               /* Search backwards for the first match */
   {
    relation=LookupTurnRelation(relations,match-1,3);

    if(relation->via==via && relation->from==from)
       match--;
    else
       break;
   }

 return(match);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next matching turn relation in the file.

  index_t FindNextTurnRelation2 Returns the index of the next turn relation matching via and from.

  Relations *relations The set of relations to process.

  index_t current The current index of a relation that matches.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindNextTurnRelation2(Relations *relations,index_t current)
{
 TurnRelation *relation;
 index_t via,from;

 relation=LookupTurnRelation(relations,current,3);

 via=relation->via;
 from=relation->from;

 current++;

 relation=LookupTurnRelation(relations,current,3);

 if(relation->via==via && relation->from==from)
    return(current);
 else
    return(NO_RELATION);
}
