/***************************************
 Routing optimiser.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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


#include <stdio.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "relations.h"

#include "logging.h"
#include "functions.h"
#include "fakes.h"
#include "results.h"


/*+ The option not to print any progress information. +*/
extern int option_quiet;

/*+ The option to calculate the quickest route insted of the shortest. +*/
extern int option_quickest;


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes not passing through a super-node.

  Results *FindNormalRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  index_t start The start node.

  index_t finish The finish node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindNormalRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,index_t start,index_t finish,Profile *profile)
{
 Results *results;
 Queue   *queue;
 score_t finish_score;
 double  finish_lat,finish_lon;
 Result  *result1,*result2;

 /* Set up the finish conditions */

 finish_score=INF_SCORE;

 if(IsFakeNode(finish))
    GetFakeLatLong(finish,&finish_lat,&finish_lon);
 else
    GetLatLong(nodes,finish,&finish_lat,&finish_lon);

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(8);

 results->start=start;
 results->finish=finish;

 result1=InsertResult(results,start);

 ZeroResult(result1);

 queue=NewQueueList();

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    Segment *segment;
    index_t node1;
    index_t turnrelation=NO_RELATION;

    if(result1->score>finish_score)
       continue;

    node1=result1->node;

    if(IsTurnRestrictedNode(LookupNode(nodes,node1,1)))
       turnrelation=FindFirstTurnRelation2(relations,node1,result1->prev_seg);

    if(IsFakeNode(node1))
       segment=FirstFakeSegment(node1);
    else
       segment=FirstSegment(segments,nodes,node1);

    while(segment)
      {
       Way *way;
       index_t node2,seg2;
       score_t segment_pref,segment_score,cumulative_score;
       int i;

       node2=OtherNode(segment,node1);  /* need this here because we use node2 at the end of the loop */

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       if(result1->prev_node==node2)
          goto endloop;

       seg2=IndexSegment(segments,segment);

       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,result1->prev_seg,seg2,profile->allow))
          goto endloop;

       if(node2!=finish && !IsFakeNode(node2) && IsSuperNode(LookupNode(nodes,node2,2)))
          goto endloop;

       way=LookupWay(ways,segment->way,1);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highway[HIGHWAY(way->type)])
          goto endloop;

       if(way->weight && way->weight<profile->weight)
          goto endloop;

       if((way->height && way->height<profile->height) ||
          (way->width  && way->width <profile->width ) ||
          (way->length && way->length<profile->length))
          goto endloop;

       segment_pref=profile->highway[HIGHWAY(way->type)];

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way->props & PROPERTIES(i))
                segment_pref*=profile->props_yes[i];
             else
                segment_pref*=profile->props_no[i];
            }

       if(segment_pref==0)
          goto endloop;

       if(!IsFakeNode(node2))
         {
          Node *node=LookupNode(nodes,node2,2);

          if(!(node->allow&profile->allow))
             goto endloop;
         }

       if(option_quickest==0)
          segment_score=(score_t)DISTANCE(segment->distance)/segment_pref;
       else
          segment_score=(score_t)Duration(segment,way,profile)/segment_pref;

       cumulative_score=result1->score+segment_score;

       if(cumulative_score>finish_score)
          goto endloop;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev_node=node1;
          result2->next_node=NO_NODE;
          result2->score=cumulative_score;
          if(IsFakeNode(node1) || IsFakeNode(node2))
             result2->prev_seg=IndexFakeSegment(segment);
          else
             result2->prev_seg=IndexSegment(segments,segment);

          if(node2==finish)
            {
             finish_score=cumulative_score;
            }
          else
            {
             result2->sortby=result2->score;

             InsertInQueue(queue,result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->prev_node=node1;
          result2->score=cumulative_score;
          if(IsFakeNode(node1) || IsFakeNode(node2))
             result2->prev_seg=IndexFakeSegment(segment);
          else
             result2->prev_seg=IndexSegment(segments,segment);

          if(node2==finish)
            {
             finish_score=cumulative_score;
            }
          else
            {
             result2->sortby=result2->score;

             if(result2->score<finish_score)
                InsertInQueue(queue,result2);
            }
         }

      endloop:

       if(IsFakeNode(node1))
          segment=NextFakeSegment(segment,node1);
       else if(IsFakeNode(node2))
          segment=NULL; /* cannot call NextSegment() with a fake segment */
       else
         {
          segment=NextSegment(segments,segment,node1);

          if(!segment && IsFakeNode(finish))
             segment=ExtraFakeSegment(node1,finish);
         }
      }
   }

 FreeQueueList(queue);

 /* Check it worked */

 if(!FindResult(results,finish))
   {
    FreeResultsList(results);
    return(NULL);
   }

 FixForwardRoute(results,finish);

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes where the start and end are a set of pre-routed super-nodes.

  Results *FindMiddleRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Results *begin The initial portion of the route.

  Results *end The final portion of the route.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindMiddleRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Results *begin,Results *end,Profile *profile)
{
 Results *results;
 Queue   *queue;
 index_t end_prev;
 score_t finish_score;
 double  finish_lat,finish_lon;
 Result  *result1,*result2,*result3;

 if(!option_quiet)
    printf_first("Routing: Super-Nodes checked = 0");

 /* Set up the finish conditions */

 finish_score=INF_DISTANCE;
 end_prev=NO_NODE;

 if(IsFakeNode(end->finish))
    GetFakeLatLong(end->finish,&finish_lat,&finish_lon);
 else
    GetLatLong(nodes,end->finish,&finish_lat,&finish_lon);

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(2048);

 results->start=begin->start;
 results->finish=end->finish;

 result1=InsertResult(results,begin->start);
 result3=FindResult(begin,begin->start);

 *result1=*result3;

 queue=NewQueueList();

 /* Insert the finish points of the beginning part of the path into the queue */

 result3=FirstResult(begin);

 while(result3)
   {
    if(result3->node!=begin->start && !IsFakeNode(result3->node) && IsSuperNode(LookupNode(nodes,result3->node,1)))
      {
       result2=InsertResult(results,result3->node);

       *result2=*result3;

       result2->prev_node=begin->start;

       result2->sortby=result2->score;

       InsertInQueue(queue,result2);
      }

    result3=NextResult(begin,result3);
   }

 if(begin->number==1)
    InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    index_t node1;
    Segment *segment;
    index_t turnrelation=NO_RELATION;

    if(result1->score>finish_score)
       continue;

    node1=result1->node;

    if(IsTurnRestrictedNode(LookupNode(nodes,node1,1)))
       turnrelation=FindFirstTurnRelation2(relations,node1,result1->prev_seg);

    segment=FirstSegment(segments,nodes,node1);

    while(segment)
      {
       index_t node2,seg2;
       Node *node;
       Way *way;
       score_t segment_pref,segment_score,cumulative_score;
       int i;

       if(!IsSuperSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->prev_node==node2)
          goto endloop;

       seg2=IndexSegment(segments,segment);

       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,result1->prev_seg,seg2,profile->allow))
          goto endloop;

       way=LookupWay(ways,segment->way,1);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highway[HIGHWAY(way->type)])
          goto endloop;

       if(way->weight && way->weight<profile->weight)
          goto endloop;

       if((way->height && way->height<profile->height) ||
          (way->width  && way->width <profile->width ) ||
          (way->length && way->length<profile->length))
          goto endloop;

       segment_pref=profile->highway[HIGHWAY(way->type)];

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way->props & PROPERTIES(i))
                segment_pref*=profile->props_yes[i];
             else
                segment_pref*=profile->props_no[i];
            }

       if(segment_pref==0)
          goto endloop;

       node=LookupNode(nodes,node2,2);

       if(!(node->allow&profile->allow))
          goto endloop;

       if(option_quickest==0)
          segment_score=(score_t)DISTANCE(segment->distance)/segment_pref;
       else
          segment_score=(score_t)Duration(segment,way,profile)/segment_pref;

       cumulative_score=result1->score+segment_score;

       if(cumulative_score>finish_score)
          goto endloop;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev_node=node1;
          result2->next_node=NO_NODE;
          result2->score=cumulative_score;
          result2->prev_seg=IndexSegment(segments,segment);

          if((result3=FindResult(end,node2)))
            {
             if((result2->score+result3->score)<finish_score)
               {
                finish_score=result2->score+result3->score;
                end_prev=node2;
               }
            }
          else
            {
             double lat,lon;
             distance_t direct;

             GetLatLong(nodes,node2,&lat,&lon);
             direct=Distance(lat,lon,finish_lat,finish_lon);

             if(option_quickest==0)
                result2->sortby=result2->score+(score_t)direct/profile->max_pref;
             else
                result2->sortby=result2->score+(score_t)distance_speed_to_duration(direct,profile->max_speed)/profile->max_pref;

             InsertInQueue(queue,result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->prev_node=node1;
          result2->score=cumulative_score;
          result2->prev_seg=IndexSegment(segments,segment);

          if((result3=FindResult(end,node2)))
            {
             if((result2->score+result3->score)<finish_score)
               {
                finish_score=result2->score+result3->score;
                end_prev=node2;
               }
            }
          else if(result2->score<finish_score)
            {
             double lat,lon;
             distance_t direct;

             GetLatLong(nodes,node2,&lat,&lon);
             direct=Distance(lat,lon,finish_lat,finish_lon);

             if(option_quickest==0)
                result2->sortby=result2->score+(score_t)direct/profile->max_pref;
             else
                result2->sortby=result2->score+(score_t)distance_speed_to_duration(direct,profile->max_speed)/profile->max_pref;

             InsertInQueue(queue,result2);
            }
         }

      endloop:

       if(!option_quiet && !(results->number%10000))
          printf_middle("Routing: Super-Nodes checked = %d",results->number);

       segment=NextSegment(segments,segment,node1);
      }
   }

 if(!option_quiet)
    printf_last("Routing: Super-Nodes checked = %d",results->number);

 /* Finish off the end part of the route. */

 if(!FindResult(results,end->finish) && end_prev!=NO_NODE)
   {
    result2=InsertResult(results,end->finish);
    result3=FindResult(end,end->finish);

    *result2=*result3;

    result2->prev_node=end_prev;
    result2->score=finish_score;
   }

 FreeQueueList(queue);

 /* Check it worked */

 if(end_prev==NO_NODE)
   {
    FreeResultsList(results);
    return(NULL);
   }

 FixForwardRoute(results,end->finish);

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any super-node.

  Results *FindStartRoutes Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  index_t start The start node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,index_t start,Profile *profile)
{
 Results *results;
 Queue   *queue;
 Result  *result1,*result2;

 /* Create the results and insert the start node */

 results=NewResultsList(8);

 results->start=start;

 result1=InsertResult(results,start);

 ZeroResult(result1);

 /* Take a shortcut if the first node is a super-node. */

 if(!IsFakeNode(start) && IsSuperNode(LookupNode(nodes,start,1)))
    return(results);

 /* Insert the first node into the queue */

 queue=NewQueueList();

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    index_t node1;
    Segment *segment;
    index_t turnrelation=NO_RELATION;

    node1=result1->node;

    if(IsTurnRestrictedNode(LookupNode(nodes,node1,1)))
       turnrelation=FindFirstTurnRelation2(relations,node1,result1->prev_seg);

    if(IsFakeNode(node1))
       segment=FirstFakeSegment(node1);
    else
       segment=FirstSegment(segments,nodes,node1);

    while(segment)
      {
       index_t node2,seg2;
       Way *way;
       score_t segment_pref,segment_score,cumulative_score;
       int i;

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->prev_node==node2)
          goto endloop;

       seg2=IndexSegment(segments,segment);

       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,result1->prev_seg,seg2,profile->allow))
          goto endloop;

       way=LookupWay(ways,segment->way,1);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highway[HIGHWAY(way->type)])
          goto endloop;

       if(way->weight && way->weight<profile->weight)
          goto endloop;

       if((way->height && way->height<profile->height) ||
          (way->width  && way->width <profile->width ) ||
          (way->length && way->length<profile->length))
          goto endloop;

       segment_pref=profile->highway[HIGHWAY(way->type)];

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way->props & PROPERTIES(i))
                segment_pref*=profile->props_yes[i];
             else
                segment_pref*=profile->props_no[i];
            }

       if(segment_pref==0)
          goto endloop;

       if(!IsFakeNode(node2))
         {
          Node *node=LookupNode(nodes,node2,2);

          if(!(node->allow&profile->allow))
             goto endloop;
         }

       if(option_quickest==0)
          segment_score=(score_t)DISTANCE(segment->distance)/segment_pref;
       else
          segment_score=(score_t)Duration(segment,way,profile)/segment_pref;

       cumulative_score=result1->score+segment_score;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev_node=node1;
          result2->next_node=NO_NODE;
          result2->score=cumulative_score;
          if(IsFakeNode(node1) || IsFakeNode(node2))
             result2->prev_seg=IndexFakeSegment(segment);
          else
             result2->prev_seg=IndexSegment(segments,segment);

          if(!IsFakeNode(node2) && !IsSuperNode(LookupNode(nodes,node2,2)))
            {
             result2->sortby=result2->score;
             InsertInQueue(queue,result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->prev_node=node1;
          result2->score=cumulative_score;
          if(IsFakeNode(node1) || IsFakeNode(node2))
             result2->prev_seg=IndexFakeSegment(segment);
          else
             result2->prev_seg=IndexSegment(segments,segment);

          if(!IsFakeNode(node2) && !IsSuperNode(LookupNode(nodes,node2,2)))
            {
             result2->sortby=result2->score;
             InsertInQueue(queue,result2);
            }
         }

      endloop:

       if(IsFakeNode(node1))
          segment=NextFakeSegment(segment,node1);
       else
          segment=NextSegment(segments,segment,node1);
      }
   }

 FreeQueueList(queue);

 /* Check it worked */

 if(results->number==1)
   {
    FreeResultsList(results);
    return(NULL);
   }

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from any super-node to a specific node.

  Results *FindFinishRoutes Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  index_t finish The finishing node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,index_t finish,Profile *profile)
{
 Results *results;
 Queue   *queue;
 Result  *result1,*result2;

 /* Create the results and insert the finish node */

 results=NewResultsList(8);

 results->finish=finish;

 result1=InsertResult(results,finish);

 ZeroResult(result1);

 /* Take a shortcut if the first node is a super-node. */

 if(!IsFakeNode(finish) && IsSuperNode(LookupNode(nodes,finish,1)))
    return(results);

 /* Insert the first node into the queue */

 queue=NewQueueList();

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    index_t node1;
    Segment *segment;
    index_t turnrelation=NO_RELATION;

    node1=result1->node;

    if(IsTurnRestrictedNode(LookupNode(nodes,node1,1)))
       turnrelation=FindFirstTurnRelation1(relations,node1); /* working backwards => turn relation sort order doesn't help */

    if(IsFakeNode(node1))
       segment=FirstFakeSegment(node1);
    else
       segment=FirstSegment(segments,nodes,node1);

    while(segment)
      {
       index_t node2,seg2;
       Way *way;
       score_t segment_pref,segment_score,cumulative_score;
       int i;

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayFrom(segment,node1)) /* Disallow oneway from node2 *to* node1 */
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->next_node==node2)
          goto endloop;

       if(turnrelation!=NO_RELATION)
         {
          index_t turnrelation2=FindFirstTurnRelation2(relations,node1,node2); /* node2 -> node1 -> result1->next_node */

          seg2=IndexSegment(segments,segment);

          if(turnrelation2!=NO_RELATION && !IsTurnAllowed(relations,turnrelation2,node1,seg2,result1->next_seg,profile->allow))
             goto endloop;
         }

       way=LookupWay(ways,segment->way,1);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highway[HIGHWAY(way->type)])
          goto endloop;

       if(way->weight && way->weight<profile->weight)
          goto endloop;

       if((way->height && way->height<profile->height) ||
          (way->width  && way->width <profile->width ) ||
          (way->length && way->length<profile->length))
          goto endloop;

       segment_pref=profile->highway[HIGHWAY(way->type)];

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way->props & PROPERTIES(i))
                segment_pref*=profile->props_yes[i];
             else
                segment_pref*=profile->props_no[i];
            }

       if(segment_pref==0)
          goto endloop;

       if(!IsFakeNode(node2))
         {
          Node *node=LookupNode(nodes,node2,2);

          if(!(node->allow&profile->allow))
             goto endloop;
         }

       if(option_quickest==0)
          segment_score=(score_t)DISTANCE(segment->distance)/segment_pref;
       else
          segment_score=(score_t)Duration(segment,way,profile)/segment_pref;

       cumulative_score=result1->score+segment_score;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev_node=NO_NODE; /* working backwards */
          result2->next_node=node1;   /* working backwards */
          result2->score=cumulative_score;
          if(IsFakeNode(node1) || IsFakeNode(node2))
             result2->prev_seg=IndexFakeSegment(segment);
          else
             result2->prev_seg=IndexSegment(segments,segment);

          if(!IsFakeNode(node2) && !IsSuperNode(LookupNode(nodes,node2,2)))
            {
             result2->sortby=result2->score;
             InsertInQueue(queue,result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->next_node=node1; /* working backwards */
          result2->score=cumulative_score;
          if(IsFakeNode(node1) || IsFakeNode(node2))
             result2->prev_seg=IndexFakeSegment(segment);
          else
             result2->prev_seg=IndexSegment(segments,segment);

          if(!IsFakeNode(node2) && !IsSuperNode(LookupNode(nodes,node2,2)))
            {
             result2->sortby=result2->score;
             InsertInQueue(queue,result2);
            }
         }

      endloop:

       if(IsFakeNode(node1))
          segment=NextFakeSegment(segment,node1);
       else
          segment=NextSegment(segments,segment,node1);
      }
   }

 FreeQueueList(queue);

 /* Check it worked */

 if(results->number==1)
   {
    FreeResultsList(results);
    return(NULL);
   }

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Create an optimum route given the set of super-nodes to follow.

  Results *CombineRoutes Returns the results from joining the super-nodes.

  Results *results The set of results from the super-nodes.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile)
{
 Result *result1,*result3;
 Results *combined;

 combined=NewResultsList(64);

 combined->start=results->start;
 combined->finish=results->finish;

 /* Sort out the combined route */

 result1=FindResult(results,results->start);

 result3=InsertResult(combined,results->start);

 ZeroResult(result3);

 do
   {
    Result *result2,*result4;

    if(result1->next_node!=NO_NODE)
      {
       Results *results2=FindNormalRoute(nodes,segments,ways,relations,result1->node,result1->next_node,profile);

       result2=FindResult(results2,result1->node);

       result3->next_node=result2->next_node;

       result2=FindResult(results2,result2->next_node);

       do
         {
          result4=InsertResult(combined,result2->node);

          *result4=*result2;
          result4->score+=result3->score;

          if(result2->next_node!=NO_NODE)
             result2=FindResult(results2,result2->next_node);
          else
             result2=NULL;
         }
       while(result2);

       FreeResultsList(results2);

       result1=FindResult(results,result1->next_node);

       result3=result4;
      }
    else
       result1=NULL;
   }
 while(result1);

 return(combined);
}


/*++++++++++++++++++++++++++++++++++++++
  Fx the forward route (i.e. setup next nodes for forward path from prev nodes on reverse path).

  Results *results The set of results to update.

  index_t finish The finish point.
  ++++++++++++++++++++++++++++++++++++++*/

void FixForwardRoute(Results *results,index_t finish)
{
 Result *result2=FindResult(results,finish);

 /* Create the forward links for the optimum path */

 do
   {
    Result *result1;

    if(result2->prev_node!=NO_NODE)
      {
       index_t node1=result2->prev_node;

       result1=FindResult(results,node1);

       result1->next_node=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 results->finish=finish;
}
