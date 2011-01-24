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

  index_t start_node The start node.

  index_t prev_segment The previous segment before the start node.

  index_t finish_node The finish node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindNormalRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,index_t start_node,index_t prev_segment,index_t finish_node,Profile *profile)
{
 Results *results;
 Queue   *queue;
 score_t finish_score;
 double  finish_lat,finish_lon;
 Result  *finish_result;
 Result  *result1,*result2;

// printf("FindNormalRoute(start_node=%ld prev_segment=%ld finish_node=%ld)\n",start_node,prev_segment,finish_node);

 /* Set up the finish conditions */

 finish_score=INF_SCORE;
 finish_result=NULL;

 if(IsFakeNode(finish_node))
    GetFakeLatLong(finish_node,&finish_lat,&finish_lon);
 else
    GetLatLong(nodes,finish_node,&finish_lat,&finish_lon);

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(8);

 results->start_node=start_node;
 results->finish_node=finish_node;

 result1=InsertResult(results,start_node,prev_segment);

 queue=NewQueueList();

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    Segment *segment;
    index_t node1,seg1;
//    index_t turnrelation=NO_RELATION;

    if(result1->score>finish_score)
       continue;

    node1=result1->node;
    seg1=result1->segment;

//    if(IsTurnRestrictedNode(LookupNode(nodes,node1,1)))
//       turnrelation=FindFirstTurnRelation2(relations,node1,seg1);

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

       if(result1->prev && result1->prev->node==node2)
          goto endloop;

       if(IsFakeNode(node1) || IsFakeNode(node2))
          seg2=IndexFakeSegment(segment);
       else
          seg2=IndexSegment(segments,segment);

//       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,seg1,seg2,profile->allow))
//          goto endloop;

       if(node2!=finish_node && !IsFakeNode(node2) && IsSuperNode(LookupNode(nodes,node2,2)))
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

       result2=FindResult(results,node2,seg2);

       if(!result2)                         /* New end node/segment combination */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_score;

          if(node2==finish_node)
            {
             if(cumulative_score<finish_score)
               {
                finish_score=cumulative_score;
                finish_result=result2;
               }
            }
          else
            {
             result2->sortby=result2->score;

             InsertInQueue(queue,result2);
            }
         }
       else if(cumulative_score<result2->score) /* New score for end node/segment combination is better */
         {
          result2->prev=result1;
          result2->score=cumulative_score;
          result2->segment=seg2;

          if(node2==finish_node)
            {
             if(cumulative_score<finish_score)
               {
                finish_score=cumulative_score;
                finish_result=result2;
               }
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

          if(!segment && IsFakeNode(finish_node))
             segment=ExtraFakeSegment(node1,finish_node);
         }
      }
   }

 FreeQueueList(queue);

 /* Check it worked */

 if(!finish_result)
   {
    FreeResultsList(results);
    return(NULL);
   }

 FixForwardRoute(results,finish_result);

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
 Result  *finish_result;
 score_t finish_score;
 double  finish_lat,finish_lon;
 Result  *result1,*result2,*result3;

 if(!option_quiet)
    printf_first("Routing: Super-Nodes checked = 0");

 /* Set up the finish conditions */

 finish_score=INF_DISTANCE;
 finish_result=NULL;

 if(IsFakeNode(end->finish_node))
    GetFakeLatLong(end->finish_node,&finish_lat,&finish_lon);
 else
    GetLatLong(nodes,end->finish_node,&finish_lat,&finish_lon);

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(2048);

 results->start_node=begin->start_node;
 results->prev_segment=begin->prev_segment;

 results->finish_node=end->finish_node;

 result1=InsertResult(results,begin->start_node,begin->prev_segment);

 queue=NewQueueList();

 /* Insert the finish points of the beginning part of the path into the queue */

 result3=FirstResult(begin);

 while(result3)
   {
    if(result3->node!=begin->start_node && !IsFakeNode(result3->node) && IsSuperNode(LookupNode(nodes,result3->node,1)))
      {
       result2=InsertResult(results,result3->node,result3->segment);

       result2->prev=result1;

       result2->score=result3->score;
       result2->sortby=result3->score;

       InsertInQueue(queue,result2);
      }

    result3=NextResult(begin,result3);
   }

 if(begin->number==1)
    InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    index_t node1,seg1;
    Segment *segment;
    index_t turnrelation=NO_RELATION;

    if(result1->score>finish_score)
       continue;

    node1=result1->node;
    seg1=result1->segment;

    if(IsTurnRestrictedNode(LookupNode(nodes,node1,1)))
       turnrelation=FindFirstTurnRelation2(relations,node1,seg1);

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

       if(result1->prev && result1->prev->node==node2)
          goto endloop;

       seg2=IndexSegment(segments,segment);

       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,seg1,seg2,profile->allow))
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

       result2=FindResult(results,node2,seg2);

       if(!result2)                         /* New end node/segment pair */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_score;

          if((result3=FindResult(end,node2,seg2)))
            {
             if((result2->score+result3->score)<finish_score)
               {
                finish_score=result2->score+result3->score;
                finish_result=result2;
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
       else if(cumulative_score<result2->score) /* New end node/segment pair is better */
         {
          result2->prev=result1;
          result2->score=cumulative_score;

          if((result3=FindResult(end,node2,seg2)))
            {
             if((result2->score+result3->score)<finish_score)
               {
                finish_score=result2->score+result3->score;
                finish_result=result2;
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

 FreeQueueList(queue);

 /* Check it worked */

 if(!finish_result)
   {
    FreeResultsList(results);
    return(NULL);
   }

 /* Finish off the end part of the route. */

 result3=InsertResult(results,end->finish_node,NO_SEGMENT);

 result3->prev=finish_result;
 result3->score=finish_score;

 FixForwardRoute(results,result3);

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any super-node.

  Results *FindStartRoutes Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  index_t start_node The start node.

  index_t prev_segment The previous segment before the start node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,index_t start_node,index_t prev_segment,Profile *profile)
{
 Results *results;
 Queue   *queue;
 Result  *result1,*result2;

 /* Create the results and insert the start node */

 results=NewResultsList(8);

 results->start_node=start_node;
 results->prev_segment=prev_segment;

 result1=InsertResult(results,start_node,prev_segment);

 /* Take a shortcut if the first node is a super-node. */

 if(!IsFakeNode(start_node) && IsSuperNode(LookupNode(nodes,start_node,1)))
    return(results);

 /* Insert the first node into the queue */

 queue=NewQueueList();

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    index_t node1,seg1;
    Segment *segment;
//    index_t turnrelation=NO_RELATION;

    node1=result1->node;
    seg1=result1->segment;

//    if(IsTurnRestrictedNode(LookupNode(nodes,node1,1)))
//       turnrelation=FindFirstTurnRelation2(relations,node1,seg1);

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

       if(result1->prev && result1->prev->node==node2)
          goto endloop;

       if(IsFakeNode(node1) || IsFakeNode(node2))
          seg2=IndexFakeSegment(segment);
       else
          seg2=IndexSegment(segments,segment);

//       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,seg1,seg2,profile->allow))
//          goto endloop;

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

       result2=FindResult(results,node2,seg2);

       if(!result2)                         /* New end node/segment combination */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_score;

          if(!IsFakeNode(node2) && !IsSuperNode(LookupNode(nodes,node2,2)))
            {
             result2->sortby=result2->score;
             InsertInQueue(queue,result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node/segment combination is better */
         {
          result2->prev=result1;
          result2->score=cumulative_score;

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

  index_t finish_node The finishing node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,index_t finish_node,Profile *profile)
{
 Results *results,*results2;
 Queue   *queue;
 Result  *result1,*result2,*result3;

 /* Create the results and insert the finish node */

 results=NewResultsList(8);

 results->finish_node=finish_node;

 result1=InsertResult(results,finish_node,NO_SEGMENT);

 /* Take a shortcut if the first node is a super-node. */

 if(!IsFakeNode(finish_node) && IsSuperNode(LookupNode(nodes,finish_node,1)))
    return(results);

 /* Insert the first node into the queue */

 queue=NewQueueList();

 InsertInQueue(queue,result1);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    index_t node1,seg1;
    Segment *segment;
    index_t turnrelation=NO_RELATION;

    node1=result1->node;
    seg1=result1->segment;

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

       if(result1->next && result1->next->node==node2)
          goto endloop;

       if(IsFakeNode(node1) || IsFakeNode(node2))
          seg2=IndexFakeSegment(segment);
       else
          seg2=IndexSegment(segments,segment);

       if(turnrelation!=NO_RELATION)
         {
          index_t turnrelation2=FindFirstTurnRelation2(relations,node1,node2); /* node2 -> node1 -> result1->next->node */

          if(turnrelation2!=NO_RELATION && !IsTurnAllowed(relations,turnrelation2,node1,seg2,seg1,profile->allow))
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

       result2=FindResult(results,node2,seg2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2,seg2);
          result2->next=result1;   /* working backwards */
          result2->score=cumulative_score;

          if(!IsFakeNode(node1) && !IsSuperNode(LookupNode(nodes,node1,1))) /* Overshoot by one segment */
            {
             result2->sortby=result2->score;
             InsertInQueue(queue,result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->next=result1; /* working backwards */
          result2->score=cumulative_score;

          if(!IsFakeNode(node1) && !IsSuperNode(LookupNode(nodes,node1,1))) /* Overshoot by one segment */
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

 /* Create a results structure with the node at the end of the segment opposite the start */

 results2=NewResultsList(8);

 results2->finish_node=results->finish_node;

 result3=FirstResult(results);

 while(result3)
   {
    if(result3->next)
      {
       result2=InsertResult(results2,result3->next->node,result3->segment);

       result2->score=result3->score;
      }

    result3=NextResult(results,result3);
   }

 /* Fix up the result->next pointers */

 result3=FirstResult(results);

 while(result3)
   {
    if(result3->next && result3->next->next)
      {
       result1=FindResult(results2,result3->next->node,result3->segment);
       result2=FindResult(results2,result3->next->next->node,result3->next->segment);

       result1->next=result2;
      }

    result3=NextResult(results,result3);
   }

 FreeResultsList(results);

 return(results2);
}


/*++++++++++++++++++++++++++++++++++++++
  Create an optimum route given the set of super-nodes to follow.

  Results *CombineRoutes Returns the results from joining the super-nodes.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  Relations *relations The set of relations to use.

  Results *results The set of results from the super-nodes.

  index_t prev_segment The previous segment before the start node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *CombineRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Results *results,index_t prev_segment,Profile *profile)
{
 Result *result1,*result3;
 Results *combined;

 combined=NewResultsList(64);

 combined->start_node=results->start_node;
 combined->finish_node=results->finish_node;

 combined->last_segment=results->last_segment;

 /* Sort out the combined route */

 result1=FindResult(results,results->start_node,prev_segment);

 result3=InsertResult(combined,results->start_node,prev_segment);

 do
   {
    Result *result2,*result4;

    if(result1->next)
      {
       Results *results2=FindNormalRoute(nodes,segments,ways,relations,result1->node,result3->segment,result1->next->node,profile);

       result2=FindResult(results2,result1->node,result3->segment);

//       printf("CombineRoutes(result2->node=%ld result2->segment=%ld)\n",result2->node,result2->segment);

       result2=result2->next;

       /*
        *      result1                          result1->next
        *         =                                  =
        *      ---*----------------------------------*  = results
        *
        *      ---*----.----.----.----.----.----.----*  = results2
        *              =
        *              result2
        *
        *      ---*----.----.----.----.----.----.----*  = combined
        *         =    =
        *   result3    result4
        */

       do
         {
//          printf("CombineRoutes(result2->node=%ld result2->segment=%ld)\n",result2->node,result2->segment);

          result4=InsertResult(combined,result2->node,result2->segment);

          result4->score=result2->score+result3->score;
          result4->prev=result3;

          result2=result2->next;

          result3=result4;
         }
       while(result2);

       FreeResultsList(results2);
      }

    result1=result1->next;
   }
 while(result1);

 FixForwardRoute(combined,result3);

 return(combined);
}


/*++++++++++++++++++++++++++++++++++++++
  Fix the forward route (i.e. setup next pointers for forward path from prev nodes on reverse path).

  Results *results The set of results to update.

  Result *finish_result The finish result.
  ++++++++++++++++++++++++++++++++++++++*/

void FixForwardRoute(Results *results,Result *finish_result)
{
 Result *result2=finish_result;

 /* Create the forward links for the optimum path */

 do
   {
    Result *result1;

    if(result2->prev)
      {
       index_t node1=result2->prev->node;
       index_t seg1=result2->prev->segment;

       result1=FindResult(results,node1,seg1);

       result1->next=result2;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 results->finish_node=finish_result->node;
 results->last_segment=finish_result->segment;
}
