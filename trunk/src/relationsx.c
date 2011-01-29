/***************************************
 Extended Relation data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010-2011 Andrew M. Bishop

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

#include "types.h"
#include "relations.h"

#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "relationsx.h"

#include "files.h"
#include "logging.h"
#include "sorting.h"


/* Functions */

static int sort_by_id(TurnRestrictRelX *a,TurnRestrictRelX *b);
static int deduplicate_by_id(TurnRestrictRelX *relationx,index_t index);

static int sort_by_via(TurnRestrictRelX *a,TurnRestrictRelX *b);


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


 /* Route Relations */

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


 /* Turn Restriction Relations */

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
 /* Close the files (finished appending) */

 relationsx->rfd=CloseFile(relationsx->rfd);

 relationsx->trfd=CloseFile(relationsx->trfd);


 /* Route Relations */


 /* Turn Restriction Relations. */

 if(relationsx->trxnumber)
   {
    int trfd;

    /* Print the start message */

    printf_first("Sorting Turn Restriction Relations");

    /* Re-open the file read-only and a new file writeable */

    relationsx->trfd=ReOpenFile(relationsx->trfilename);

    DeleteFile(relationsx->trfilename);

    trfd=OpenFileNew(relationsx->trfilename);

    /* Sort the relations */

    sortrelationsx=relationsx;

    filesort_fixed(relationsx->trfd,trfd,sizeof(TurnRestrictRelX),(int (*)(const void*,const void*))sort_by_id,(int (*)(void*,index_t))deduplicate_by_id);

    /* Close the files */

    relationsx->trfd=CloseFile(relationsx->trfd);
    CloseFile(trfd);

    /* Print the final message */

    printf_last("Sorted Relations: Relations=%d Duplicates=%d",relationsx->trxnumber,relationsx->trxnumber-relationsx->trnumber);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the turn restriction relations into id order.

  int sort_by_id Returns the comparison of the id fields.

  TurnRestrictRelX *a The first extended relation.

  TurnRestrictRelX *b The second extended relation.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(TurnRestrictRelX *a,TurnRestrictRelX *b)
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
  Sort the list of turn relations.

  RelationsX* relationsx The set of relations to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortTurnRelationList(RelationsX* relationsx)
{
 int trfd;

 if(relationsx->trnumber==0)
    return;

 /* Print the start message */

 printf_first("Sorting Turn Restriction Relations");

 /* Re-open the file read-only and a new file writeable */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 DeleteFile(relationsx->trfilename);

 trfd=OpenFileNew(relationsx->trfilename);

 /* Sort the relations */

 filesort_fixed(relationsx->trfd,trfd,sizeof(TurnRestrictRelX),(int (*)(const void*,const void*))sort_by_via,NULL);

 /* Close the files */

 relationsx->trfd=CloseFile(relationsx->trfd);
 CloseFile(trfd);

 /* Print the final message */

 printf_last("Sorted Relations: Relations=%d",relationsx->trnumber);
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
    node_t a_id=a->from;
    node_t b_id=b->from;

    if(a_id<b_id)
       return(-1);
    else if(a_id>b_id)
       return(1);
    else
      {
       node_t a_id=a->to;
       node_t b_id=b->to;

       if(a_id<b_id)
          return(-1);
       else if(a_id>b_id)
          return(1);
       else
          return(0);
      }
   }
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

 /* Map into memory / open the files */

#if !SLIM
 waysx->xdata=MapFileWriteable(waysx->filename);
#else
 waysx->fd=ReOpenFileWriteable(waysx->filename);
#endif

 /* Re-open the file read-only */

 relationsx->rfd=ReOpenFile(relationsx->rfilename);

 /* Read through the file. */

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

                PutBackWayX(waysx,way,1);

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

 /* Close the file */

 relationsx->rfd=CloseFile(relationsx->rfd);

 /* Unmap from memory / close the files */

#if !SLIM
 waysx->xdata=UnmapFile(waysx->filename);
#else
 waysx->fd=CloseFile(waysx->fd);
#endif
}


/*++++++++++++++++++++++++++++++++++++++
  Process the turn relations (first part) to update them with the node information.

  RelationsX *relationsx The set of relations to process.

  NodesX *nodesx The set of nodes to process.

  SegmentsX *segmentsx The set of segments to process.

  WaysX *waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessTurnRelations1(RelationsX *relationsx,NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx)
{
 TurnRestrictRelX relationx;
 int trfd;
 int total=0;

 if(nodesx->number==0 || segmentsx->number==0)
    return;

 /* Print the start message */

 printf_first("Processing Turn Restriction Relations (1): Turn Relations=0");

 /* Map into memory / open the files */

#if !SLIM
 nodesx->xdata=MapFileWriteable(nodesx->filename);
 segmentsx->xdata=MapFile(segmentsx->filename);
#else
 nodesx->fd=ReOpenFileWriteable(nodesx->filename);
 segmentsx->fd=ReOpenFile(segmentsx->filename);
#endif

 /* Re-open the file read-only and a new file writeable */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 DeleteFile(relationsx->trfilename);

 trfd=OpenFileNew(relationsx->trfilename);

 /* Process all of the relations */

 while(!ReadFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX)))
   {
    if(relationx.restrict==TurnRestrict_no_right_turn ||
       relationx.restrict==TurnRestrict_no_left_turn ||
       relationx.restrict==TurnRestrict_no_u_turn ||
       relationx.restrict==TurnRestrict_no_straight_on)
      {
       NodeX *nodex;
       index_t seg;
       node_t node_from=NO_NODE,node_to=NO_NODE;

       /* Find the segments that join the node 'via' */

       seg=IndexFirstSegmentX1(segmentsx,relationx.via);

       do
         {
          SegmentX *segx=LookupSegmentX(segmentsx,seg,1);

          if(segx->way==relationx.from)
            {
             if(node_from!=NO_NODE) /* Only one segment can be on the 'from' way */
                goto endloop;

             node_from=segx->node2;
            }

          if(segx->way==relationx.to)
            {
             if(node_to!=NO_NODE) /* Only one segment can be on the 'to' way */
                goto endloop;

             node_to=segx->node2;
            }

          seg=IndexNextSegmentX1(segmentsx,seg,relationx.via);
         }
       while(seg!=NO_SEGMENT);

       if(node_to==NO_NODE || node_from==NO_NODE) /* Not enough segments for the selected ways */
          goto endloop;

       /* Write the results */

       relationx.from=IndexNodeX(nodesx,node_from);
       relationx.to  =IndexNodeX(nodesx,node_to);
       relationx.via =IndexNodeX(nodesx,relationx.via);

       WriteFile(trfd,&relationx,sizeof(TurnRestrictRelX));

       nodex=LookupNodeX(nodesx,relationx.from,1);
       nodex->flags|=NODE_TURNRSTRCT2;
       PutBackNodeX(nodesx,relationx.from,1);

       nodex=LookupNodeX(nodesx,relationx.via,1);
       nodex->flags|=NODE_TURNRSTRCT;
       PutBackNodeX(nodesx,relationx.via,1);

       nodex=LookupNodeX(nodesx,relationx.to,1);
       nodex->flags|=NODE_TURNRSTRCT2;
       PutBackNodeX(nodesx,relationx.to,1);

       total++;

       if(!(total%10000))
          printf_middle("Processing Turn Restriction Relations (1): Turn Relations=%d New=%d",total,total-relationsx->trnumber);
      }
    else
      {
       NodeX *nodex;
       index_t seg;
       node_t node_from=NO_NODE,node_to[8];
       int nnodes_to=0,i;

       /* Find the segments that join the node 'via' */

       seg=IndexFirstSegmentX1(segmentsx,relationx.via);

       do
         {
          SegmentX *segx=LookupSegmentX(segmentsx,seg,1);

          if(segx->way==relationx.from)
            {
             if(node_from!=NO_NODE) /* Only one segment can be on the 'from' way */
                goto endloop;

             node_from=segx->node2;
            }

          if(segx->way!=relationx.to)
            {
             if(nnodes_to==8)   /* Too many segments (arbitrary choice) */
                goto endloop;

             node_to[nnodes_to++]=segx->node2;
            }

          seg=IndexNextSegmentX1(segmentsx,seg,relationx.via);
         }
       while(seg!=NO_SEGMENT);

       if(nnodes_to==0 || node_from==NO_NODE) /* Not enough segments for the selected ways */
          goto endloop;

       /* Write the results */

       relationx.via=IndexNodeX(nodesx,relationx.via);

       for(i=0;i<nnodes_to;i++)
         {
          if(node_to[i]==node_from)
             continue;

          relationx.from=IndexNodeX(nodesx,node_from);
          relationx.to  =IndexNodeX(nodesx,node_to[i]);

          WriteFile(trfd,&relationx,sizeof(TurnRestrictRelX));

          nodex=LookupNodeX(nodesx,relationx.to,1);
          nodex->flags|=NODE_TURNRSTRCT2;
          PutBackNodeX(nodesx,relationx.to,1);

          total++;

          if(!(total%10000))
             printf_middle("Processing Turn Restriction Relations (1): Turn Relations=%d New=%d",total,total-relationsx->trnumber);
         }

       nodex=LookupNodeX(nodesx,relationx.from,1);
       nodex->flags|=NODE_TURNRSTRCT2;
       PutBackNodeX(nodesx,relationx.from,1);

       nodex=LookupNodeX(nodesx,relationx.via,1);
       nodex->flags|=NODE_TURNRSTRCT;
       PutBackNodeX(nodesx,relationx.via,1);
      }

   endloop: ;
   }

 /* Close the files */

 relationsx->trfd=CloseFile(relationsx->trfd);
 CloseFile(trfd);

 /* Unmap from memory / close the files */

#if !SLIM
 nodesx->xdata=UnmapFile(nodesx->filename);
 segmentsx->xdata=UnmapFile(segmentsx->filename);
#else
 nodesx->fd=CloseFile(nodesx->fd);
 segmentsx->fd=CloseFile(segmentsx->fd);
#endif

 /* Print the final message */

 printf_last("Processing Turn Restriction Relations (1): Turn Relations=%d New=%d",total,total-relationsx->trnumber);

 relationsx->trnumber=total;
}


/*++++++++++++++++++++++++++++++++++++++
  Process the turn relations (second part) to update them with the re-ordered node information.

  RelationsX *relationsx The set of relations to process.

  NodesX *nodesx The set of nodes to process.

  SegmentsX *segmentsx The set of segments to process.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessTurnRelations2(RelationsX *relationsx,NodesX *nodesx,SegmentsX *segmentsx)
{
 int trfd;
 int i;

 /* Print the start message */

 printf_first("Processing Turn Restriction Relations (2): Turn Relations=0");

 /* Map into memory / open the files */

#if !SLIM
 nodesx->xdata=MapFile(nodesx->filename);
 segmentsx->xdata=MapFile(segmentsx->filename);
 segmentsx->sdata=MapFile(segmentsx->sfilename);
#else
 nodesx->fd=ReOpenFile(nodesx->filename);
 segmentsx->fd=ReOpenFile(segmentsx->filename);
 segmentsx->sfd=ReOpenFile(segmentsx->sfilename);
#endif

 /* Re-open the file read-only and a new file writeable */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 DeleteFile(relationsx->trfilename);

 trfd=OpenFileNew(relationsx->trfilename);

 /* Process all of the relations */

 for(i=0;i<relationsx->trnumber;i++)
   {
    NodeX *nodex;
    TurnRestrictRelX relationx;
    index_t from_node,via_node,to_node;
    index_t seg;

    ReadFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX));

    from_node=relationx.from;
    via_node=relationx.via;
    to_node=relationx.to;

    relationx.via=nodesx->gdata[via_node];

    nodex=LookupNodeX(nodesx,relationx.via,1);
    seg=nodex->id;

    do
      {
       SegmentX *segmentx=LookupSegmentX(segmentsx,seg,1);

       if((segmentx->node1==from_node && segmentx->node2==via_node) ||
          (segmentx->node2==from_node && segmentx->node1==via_node))
          relationx.from=seg;

       if((segmentx->node1==to_node && segmentx->node2==via_node) ||
          (segmentx->node2==to_node && segmentx->node1==via_node))
          relationx.to=seg;

       if(segmentx->node1==via_node)
          seg++;
       else if(segmentx->node2==via_node)
         {
          Segment *segment=LookupSegmentXSegment(segmentsx,seg,1);

          seg=segment->next2;
         }
       else
          seg=NO_SEGMENT;
      }
    while(seg!=NO_SEGMENT);

    WriteFile(trfd,&relationx,sizeof(TurnRestrictRelX));

    if(!(relationsx->trnumber%10000))
       printf_middle("Processing Turn Restriction Relations (2): Turn Relations=%d",relationsx->trnumber);
   }

 /* Close the files */

 relationsx->trfd=CloseFile(relationsx->trfd);
 CloseFile(trfd);

 /* Unmap from memory / close the files */

#if !SLIM
 nodesx->xdata=UnmapFile(nodesx->filename);
 segmentsx->xdata=UnmapFile(segmentsx->filename);
 segmentsx->sdata=UnmapFile(segmentsx->sfilename);
#else
 nodesx->fd=CloseFile(nodesx->fd);
 segmentsx->fd=CloseFile(segmentsx->fd);
 segmentsx->sfd=CloseFile(segmentsx->sfd);
#endif

 /* Print the final message */

 printf_last("Processing Turn Restriction Relations (2): Turn Relations=%d",relationsx->trnumber);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the relation list to a file.

  RelationsX* relationsx The set of relations to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveRelationList(RelationsX* relationsx,const char *filename)
{
 index_t i;
 int fd;
 RelationsFile relationsfile={0};

 /* Print the start message */

 printf_first("Writing Relations: Turn Relations=0");

 /* Re-open the file read-only */

 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 /* Write out the relations data */

 fd=OpenFileNew(filename);

 SeekFile(fd,sizeof(RelationsFile));

 for(i=0;i<relationsx->trnumber;i++)
   {
    TurnRestrictRelX relationx;
    TurnRelation relation;

    ReadFile(relationsx->trfd,&relationx,sizeof(TurnRestrictRelX));

    relation.from=relationx.from;
    relation.via=relationx.via;
    relation.to=relationx.to;
    relation.except=relationx.except;

    WriteFile(fd,&relation,sizeof(TurnRelation));

    if(!((i+1)%10000))
       printf_middle("Writing Relations: Turn Relations=%d",i+1);
   }

 /* Write out the header structure */

 relationsfile.trnumber=relationsx->trnumber;

 SeekFile(fd,0);
 WriteFile(fd,&relationsfile,sizeof(RelationsFile));

 CloseFile(fd);

 /* Close the file */

 relationsx->trfd=CloseFile(relationsx->trfd);

 /* Print the final message */

 printf_last("Wrote Relations: Turn Relations=%d",relationsx->trnumber);
}
