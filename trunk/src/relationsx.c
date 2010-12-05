/***************************************
 $Header: /home/amb/CVS/routino/src/relationsx.c,v 1.13 2010-12-05 16:19:24 amb Exp $

 Extended Relation data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010 Andrew M. Bishop

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


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "waysx.h"
#include "relationsx.h"

#include "files.h"
#include "logging.h"
#include "sorting.h"


/* Functions */

static int sort_by_via(TurnRestrictRelX *a,TurnRestrictRelX *b);
static int deduplicate_by_id(TurnRestrictRelX *relationx,index_t index);


/* Variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;

static RelationsX* sortrelationsx;


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new relation list (create a new file or open an existing one).

  RelationsX *NewRelationList Returns the relation list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

RelationsX *NewRelationList(int append)
{
 RelationsX *relationsx;

 relationsx=(RelationsX*)calloc(1,sizeof(RelationsX));

 assert(relationsx); /* Check calloc() worked */

 /* Route relations */

 relationsx->rfilename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(relationsx->rfilename,"%s/relationsx.route.input.tmp",option_tmpdirname);
 else
    sprintf(relationsx->rfilename,"%s/relationsx.route.%p.tmp",option_tmpdirname,relationsx);

 if(append)
   {
    off_t size,position=0;

    relationsx->rfd=OpenFileAppend(relationsx->rfilename);

    size=SizeFile(relationsx->rfilename);

    while(position<size)
      {
       FILESORT_VARINT relationsize;

       SeekFile(relationsx->rfd,position);
       ReadFile(relationsx->rfd,&relationsize,FILESORT_VARSIZE);

       relationsx->rxnumber++;
       position+=relationsize+FILESORT_VARSIZE;
      }

    SeekFile(relationsx->rfd,size);
   }
 else
    relationsx->rfd=OpenFileNew(relationsx->rfilename);

 /* Turn Restriction relations */

 relationsx->trfilename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(relationsx->trfilename,"%s/relationsx.turn.input.tmp",option_tmpdirname);
 else
    sprintf(relationsx->trfilename,"%s/relationsx.turn.%p.tmp",option_tmpdirname,relationsx);

 if(append)
   {
    off_t size;

    relationsx->trfd=OpenFileAppend(relationsx->trfilename);

    size=SizeFile(relationsx->trfilename);

    relationsx->trxnumber=size/sizeof(TurnRestrictRelX);
   }
 else
    relationsx->trfd=OpenFileNew(relationsx->trfilename);

 return(relationsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a relation list.

  RelationsX *relationsx The list to be freed.

  int keep Set to 1 if the file is to be kept.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeRelationList(RelationsX *relationsx,int keep)
{
 /* Route relations */

 if(!keep)
    DeleteFile(relationsx->rfilename);

 free(relationsx->rfilename);

 /* Turn Restriction relations */

 if(!keep)
    DeleteFile(relationsx->trfilename);

 free(relationsx->trfilename);

 free(relationsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single relation to an unsorted route relation list.

  RelationsX* relationsx The set of relations to process.

  relation_t id The ID of the relation.

  transports_t routes The types of routes that this relation is for.

  way_t *ways The array of ways that are members of the relation.

  int nways The number of ways that are members of the relation.

  relation_t *relations The array of relations that are members of the relation.

  int nrelations The number of relations that are members of the relation.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendRouteRelation(RelationsX* relationsx,relation_t id,
                         transports_t routes,
                         way_t *ways,int nways,
                         relation_t *relations,int nrelations)
{
 RouteRelX relationx;
 FILESORT_VARINT size;
 way_t zeroway=0;
 relation_t zerorelation=0;

 relationx.id=id;
 relationx.routes=routes;

 size=sizeof(RouteRelX)+(nways+1)*sizeof(way_t)+(nrelations+1)*sizeof(relation_t);

 WriteFile(relationsx->rfd,&size,FILESORT_VARSIZE);
 WriteFile(relationsx->rfd,&relationx,sizeof(RouteRelX));

 WriteFile(relationsx->rfd,ways    ,nways*sizeof(way_t));
 WriteFile(relationsx->rfd,&zeroway,      sizeof(way_t));

 WriteFile(relationsx->rfd,relations    ,nrelations*sizeof(relation_t));
 WriteFile(relationsx->rfd,&zerorelation,           sizeof(relation_t));

 relationsx->rxnumber++;

 assert(!(relationsx->rxnumber==0)); /* Zero marks the high-water mark for relations. */
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single relation to an unsorted turn restriction relation list.

  RelationsX* relationsx The set of relations to process.

  relation_t id The ID of the relation.

  way_t from The way that the turn restriction starts from.

  way_t to The way that the restriction finished on.

  node_t via The node that the turn restriction passes through.

  TurnRestriction restriction The type of restriction.

  transports_t except The set of transports allowed to bypass the restriction.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendTurnRestrictRelation(RelationsX* relationsx,relation_t id,
                                way_t from,way_t to,node_t via,
                                TurnRestriction restriction,transports_t except)
{
 TurnRestrictRelX relationx;

 relationx.id=id;
 relationx.from=from;
 relationx.to=to;
 relationx.via=via;
 relationx.restrict=restriction;
 relationx.except=except;

 WriteFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX));

 relationsx->trxnumber++;

 assert(!(relationsx->trxnumber==0)); /* Zero marks the high-water mark for relations. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of relations.

  RelationsX* relationsx The set of relations to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortRelationList(RelationsX* relationsx)
{
 /* Don't need to sort route relations */

 /* Sort the turn restriction relations by node. */

 int trfd;

 /* Print the start message */

 printf_first("Sorting Turn Restriction Relations");

 /* Close the file and re-open it (finished appending) */

 CloseFile(relationsx->trfd);
 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 DeleteFile(relationsx->trfilename);

 trfd=OpenFileNew(relationsx->trfilename);

 /* Sort the relations */

 sortrelationsx=relationsx;

 filesort_fixed(relationsx->trfd,trfd,sizeof(TurnRestrictRelX),(int (*)(const void*,const void*))sort_by_via,(int (*)(void*,index_t))deduplicate_by_id);

 /* Close the files */

 CloseFile(relationsx->trfd);
 CloseFile(trfd);

 /* Print the final message */

 printf_last("Sorted Relations: Relations=%d Duplicates=%d",relationsx->trxnumber,relationsx->trxnumber-relationsx->trnumber);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the turn restriction relations into via node order.

  int sort_by_via Returns the comparison of the via fields.

  TurnRestrictRelX *a The first extended relation.

  TurnRestrictRelX *b The second extended relation.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_via(TurnRestrictRelX *a,TurnRestrictRelX *b)
{
 node_t a_id=a->via;
 node_t b_id=b->via;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
   {
    relation_t a_id=a->id;
    relation_t b_id=b->id;

    if(a_id<b_id)
       return(-1);
    else if(a_id>b_id)
       return(1);
    else
       return(0);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Deduplicate the extended relations using the id after sorting.

  int deduplicate_by_id Return 1 if the value is to be kept, otherwise zero.

  TurnRestrictRelX *relationx The extended relation.

  index_t index The index of this relation in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int deduplicate_by_id(TurnRestrictRelX *relationx,index_t index)
{
 static relation_t previd;

 if(index==0 || relationx->id!=previd)
   {
    previd=relationx->id;

    sortrelationsx->trnumber++;

    return(1);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Process the route relations and apply the information to the ways.

  RelationsX *relationsx The set of relations to process.

  WaysX *waysx The set of ways to update.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessRouteRelations(RelationsX *relationsx,WaysX *waysx)
{
 RouteRelX *unmatched=NULL,*lastunmatched=NULL;
 int nunmatched=0,lastnunmatched=0,iteration=0;

 if(waysx->number==0)
    return;

 /* Map into memory */

#if !SLIM
 waysx->xdata=MapFileWriteable(waysx->filename);
#endif

 /* Re-open the ways file read/write */

#if SLIM
 CloseFile(waysx->fd);
 waysx->fd=ReOpenFileWriteable(waysx->filename);
#endif

 /* Open the file and read through it */

 relationsx->rfd=ReOpenFile(relationsx->rfilename);

 do
   {
    int ways=0,relations=0;
    int i;

    SeekFile(relationsx->rfd,0);

    /* Print the start message */

    printf_first("Processing Route Relations: Iteration=%d Relations=0 Modified Ways=0",iteration);

    for(i=0;i<relationsx->rxnumber;i++)
      {
       FILESORT_VARINT size;
       RouteRelX relationx;
       way_t wayid;
       relation_t relationid;
       transports_t routes=Transports_None;

       /* Read each route relation */

       ReadFile(relationsx->rfd,&size,FILESORT_VARSIZE);
       ReadFile(relationsx->rfd,&relationx,sizeof(RouteRelX));

       /* Decide what type of route it is */

       if(iteration==0)
         {
          relations++;
          routes=relationx.routes;
         }
       else
         {
          int j;

          for(j=0;j<lastnunmatched;j++)
             if(lastunmatched[j].id==relationx.id)
               {
                relations++;

                if((lastunmatched[j].routes|relationx.routes)==relationx.routes)
                   routes=0; /* Nothing new to add */
                else
                   routes=lastunmatched[j].routes;
                break;
               }
         }

       /* Loop through the ways */

       do
         {
          ReadFile(relationsx->rfd,&wayid,sizeof(way_t));

          /* Update the ways that are listed for the relation */

          if(wayid && routes)
            {
             index_t way=IndexWayX(waysx,wayid);

             if(way!=NO_WAY)
               {
                WayX *wayx=LookupWayX(waysx,way,1);

                if(routes&Transports_Foot)
                   wayx->way.props|=Properties_FootRoute;

                if(routes&Transports_Bicycle)
                   wayx->way.props|=Properties_BicycleRoute;

#if SLIM
                PutBackWayX(waysx,way,1);
#endif

                ways++;
               }
            }
         }
       while(wayid);

       /* Loop through the relations */

       do
         {
          ReadFile(relationsx->rfd,&relationid,sizeof(relation_t));

          /* Add the relations that are listed for this relation to the list for next time */

          if(relationid && routes && relationid!=relationx.id)
            {
             if(nunmatched%256==0)
                unmatched=(RouteRelX*)realloc((void*)unmatched,(nunmatched+256)*sizeof(RouteRelX));

             unmatched[nunmatched].id=relationid;
             unmatched[nunmatched].routes=routes;

             nunmatched++;
            }
         }
       while(relationid);

       if(!((i+1)%10000))
          printf_middle("Processing Route Relations: Iteration=%d Relations=%d Modified Ways=%d",iteration,relations,ways);
      }

    if(lastunmatched)
       free(lastunmatched);

    lastunmatched=unmatched;
    lastnunmatched=nunmatched;

    unmatched=NULL;
    nunmatched=0;

    /* Print the final message */

    printf_last("Processed Route Relations: Iteration=%d Relations=%d Modified Ways=%d",iteration,relations,ways);
   }
 while(lastnunmatched && ++iteration<5);

 if(lastunmatched)
    free(lastunmatched);

 CloseFile(relationsx->rfd);

 /* Unmap from memory */

#if !SLIM
 waysx->xdata=UnmapFile(waysx->filename);
#endif

 /* Re-open the ways file read only */

#if SLIM
 CloseFile(waysx->fd);
 waysx->fd=ReOpenFile(waysx->filename);
#endif
}
