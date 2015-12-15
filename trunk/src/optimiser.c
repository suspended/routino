/***************************************
 Routing optimiser.

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


#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "relations.h"

#include "logging.h"
#include "functions.h"
#include "fakes.h"
#include "results.h"

#ifdef LIBROUTINO

#include "routino.h"

/*+ The function to be called to report on the routing progress. +*/
extern Routino_ProgressFunc progress_func;

/*+ The current state of the routing progress. +*/
extern double progress_value;

/*+ Set when the progress callback returns false in the routing function. +*/
extern int progress_abort;

#endif


/*+ To help when debugging +*/
#define DEBUG 0


/* Global variables */

/*+ The option not to print any progress information. +*/
extern int option_quiet;

/*+ The option to calculate the quickest route insted of the shortest. +*/
extern int option_quickest;


/* Local functions */

static Results *FindNormalRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t start_node,index_t prev_segment,index_t finish_node);
static Results *FindMiddleRoute(Nodes *supernodes,Segments *supersegments,Ways *superways,Relations *relations,Profile *profile,Results *begin,Results *end);
static index_t  FindSuperSegment(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t finish_node,index_t finish_segment);
static Results *FindSuperRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t start_node,index_t finish_node);
static Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t start_node,index_t prev_segment,index_t finish_node);
static Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t finish_node);
static Results *CombineRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,Results *begin,Results *middle,Results *end);

static void     FixForwardRoute(Results *results,Result *finish_result);

#if DEBUG
static void print_debug_route(Nodes *nodes,Segments *segments,Results *results,Result *first,int indent,int direction);
#endif


/*++++++++++++++++++++++++++++++++++++++
  Find a complete route from a specified node to another node.

  Results *CalculateRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  index_t start_node The start node.

  index_t prev_segment The previous segment before the start node.

  index_t finish_node The finish node.

  int start_waypoint The starting waypoint.

  int finish_waypoint The finish waypoint.
  ++++++++++++++++++++++++++++++++++++++*/

Results *CalculateRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,
                        index_t start_node,index_t prev_segment,index_t finish_node,
                        int start_waypoint,int finish_waypoint)
{
 Results *complete=NULL;

 /* A special case if the first and last nodes are the same */

 if(start_node==finish_node)
   {
    index_t fake_segment;
    Result *result1,*result2;

    complete=NewResultsList(8);

    if(prev_segment==NO_SEGMENT)
      {
       double lat,lon;
       distance_t distmin,dist1,dist2;
       index_t node1,node2;

       GetLatLong(nodes,start_node,NULL,&lat,&lon);

       prev_segment=FindClosestSegment(nodes,segments,ways,lat,lon,1,profile,&distmin,&node1,&node2,&dist1,&dist2);
      }

    fake_segment=CreateFakeNullSegment(segments,start_node,prev_segment,finish_waypoint);

    result1=InsertResult(complete,start_node,prev_segment);
    result2=InsertResult(complete,finish_node,fake_segment);

    result1->next=result2;

    complete->start_node=start_node;
    complete->prev_segment=prev_segment;

    complete->finish_node=finish_node;
    complete->last_segment=fake_segment;
   }
 else
   {
    Results *begin;

    /* Calculate the beginning of the route */

    begin=FindStartRoutes(nodes,segments,ways,relations,profile,start_node,prev_segment,finish_node);

    if(begin)
      {
       /* Check if the end of the route was reached */

       if(begin->finish_node!=NO_NODE)
          complete=begin;
      }
    else
      {
       if(prev_segment!=NO_SEGMENT)
         {
          /* Try again but allow a U-turn at the start waypoint -
             this solves the problem of facing a dead-end that contains no super-nodes. */

          prev_segment=NO_SEGMENT;

          begin=FindStartRoutes(nodes,segments,ways,relations,profile,start_node,prev_segment,finish_node);
         }

       if(begin)
         {
          /* Check if the end of the route was reached */

          if(begin->finish_node!=NO_NODE)
             complete=begin;
         }
       else
         {
#ifndef LIBROUTINO
          fprintf(stderr,"Error: Cannot find initial section of route compatible with profile.\n");
#endif
          return(NULL);
         }
      }

    /* Calculate the rest of the route */

    if(!complete)
      {
       Results *middle,*end;

       /* Calculate the end of the route */

       end=FindFinishRoutes(nodes,segments,ways,relations,profile,finish_node);

       if(!end)
         {
#ifndef LIBROUTINO
          fprintf(stderr,"Error: Cannot find final section of route compatible with profile.\n");
#endif
          return(NULL);
         }

       /* Calculate the middle of the route */

       middle=FindMiddleRoute(nodes,segments,ways,relations,profile,begin,end);

       if(!middle && prev_segment!=NO_SEGMENT)
         {
          /* Try again but allow a U-turn at the start waypoint -
             this solves the problem of facing a dead-end that contains some super-nodes. */

          FreeResultsList(begin);

          begin=FindStartRoutes(nodes,segments,ways,relations,profile,start_node,NO_SEGMENT,finish_node);

          if(begin)
             middle=FindMiddleRoute(nodes,segments,ways,relations,profile,begin,end);
         }

       if(!middle)
         {
#ifndef LIBROUTINO
          fprintf(stderr,"Error: Cannot find super-route compatible with profile.\n");
#endif
          return(NULL);
         }

       complete=CombineRoutes(nodes,segments,ways,relations,profile,begin,middle,end);

       if(!complete)
         {
#ifndef LIBROUTINO
          fprintf(stderr,"Error: Cannot create combined route following super-route.\n");
#endif
          return(NULL);
         }

       FreeResultsList(begin);
       FreeResultsList(middle);
       FreeResultsList(end);
      }
   }

 complete->start_waypoint=start_waypoint;
 complete->finish_waypoint=finish_waypoint;

#if DEBUG
 printf("The final route is:\n");

 print_debug_route(nodes,segments,complete,NULL,2,+1);
#endif

 return(complete);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes not passing through a super-node.

  Results *FindNormalRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  index_t start_node The start node.

  index_t prev_segment The previous segment before the start node.

  index_t finish_node The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *FindNormalRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t start_node,index_t prev_segment,index_t finish_node)
{
 Results *results;
 Queue   *queue;
 score_t total_score;
 double  finish_lat,finish_lon;
 Result  *start_result,*finish_result;
 Result  *result1,*result2;
 int     force_uturn=0;

#if DEBUG
 printf("    FindNormalRoute(...,start_node=%"Pindex_t" prev_segment=%"Pindex_t" finish_node=%"Pindex_t")\n",start_node,prev_segment,finish_node);
#endif

 /* Set up the finish conditions */

 total_score=INF_SCORE;
 finish_result=NULL;

 if(IsFakeNode(finish_node))
    GetFakeLatLong(finish_node,&finish_lat,&finish_lon);
 else
    GetLatLong(nodes,finish_node,NULL,&finish_lat,&finish_lon);

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(8);
 queue=NewQueueList(8);

 start_result=InsertResult(results,start_node,prev_segment);

 InsertInQueue(queue,start_result,0);

 /* Check for barrier at start waypoint - must perform U-turn */

 if(prev_segment!=NO_SEGMENT && !IsFakeNode(start_node))
   {
    Node *startp=LookupNode(nodes,start_node,1);

    if(!(startp->allow&profile->allow))
       force_uturn=1;
   }

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    Node *node1p=NULL;
    Segment *segment2p;
    index_t node1,seg1,seg1r;
    index_t turnrelation=NO_RELATION;

    /* score must be better than current best score */
    if(result1->score>=total_score)
       continue;

    node1=result1->node;
    seg1=result1->segment;

    if(IsFakeSegment(seg1))
       seg1r=IndexRealSegment(seg1);
    else
       seg1r=seg1;

    if(!IsFakeNode(node1))
       node1p=LookupNode(nodes,node1,1);

    /* lookup if a turn restriction applies */
    if(profile->turns && node1p && IsTurnRestrictedNode(node1p))
       turnrelation=FindFirstTurnRelation2(relations,node1,seg1r);

    /* Loop across all segments */

    if(IsFakeNode(node1))
       segment2p=FirstFakeSegment(node1);
    else
       segment2p=FirstSegment(segments,node1p,1);

    while(segment2p)
      {
       Node *node2p=NULL;
       Way *way2p;
       index_t node2,seg2,seg2r;
       score_t segment_pref,segment_score,cumulative_score;
       int i;

       node2=OtherNode(segment2p,node1); /* need this here because we use node2 at the end of the loop */

       /* must be a normal segment */
       if(!IsNormalSegment(segment2p))
          goto endloop;

       /* must obey one-way restrictions (unless profile allows) */
       if(profile->oneway && IsOnewayTo(segment2p,node1))
         {
          if(profile->allow!=Transports_Bicycle)
             goto endloop;

          way2p=LookupWay(ways,segment2p->way,1);

          if(!(way2p->type&Highway_CycleBothWays))
             goto endloop;
         }

       if(IsFakeNode(node1) || IsFakeNode(node2))
         {
          seg2 =IndexFakeSegment(segment2p);
          seg2r=IndexRealSegment(seg2);
         }
       else
         {
          seg2 =IndexSegment(segments,segment2p);
          seg2r=seg2;
         }

       /* must perform U-turn in special cases */
       if(force_uturn && node1==start_node)
         {
          if(seg2r!=result1->segment)
             goto endloop;
         }
       else
          /* must not perform U-turn (unless profile allows) */
          if(profile->turns && (seg1==seg2 || seg1==seg2r || seg1r==seg2 || (seg1r==seg2r && IsFakeUTurn(seg1,seg2))))
             goto endloop;

       /* must obey turn relations */
       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,seg1r,seg2r,profile->allow))
          goto endloop;

       if(!IsFakeNode(node2))
          node2p=LookupNode(nodes,node2,2);

       /* must not pass over super-node */
       if(node2!=finish_node && node2p && IsSuperNode(node2p))
          goto endloop;

       way2p=LookupWay(ways,segment2p->way,1);

       /* mode of transport must be allowed on the highway */
       if(!(way2p->allow&profile->allow))
          goto endloop;

       /* must obey weight restriction (if exists) */
       if(way2p->weight && way2p->weight<profile->weight)
          goto endloop;

       /* must obey height/width/length restriction (if exist) */
       if((way2p->height && way2p->height<profile->height) ||
          (way2p->width  && way2p->width <profile->width ) ||
          (way2p->length && way2p->length<profile->length))
          goto endloop;

       segment_pref=profile->highway[HIGHWAY(way2p->type)];

       /* highway preferences must allow this highway */
       if(segment_pref==0)
          goto endloop;

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way2p->props & PROPERTIES(i))
                segment_pref*=profile->props_yes[i];
             else
                segment_pref*=profile->props_no[i];
            }

       /* profile preferences must allow this highway */
       if(segment_pref==0)
          goto endloop;

       /* mode of transport must be allowed through node2 unless it is the final node */
       if(node2p && node2!=finish_node && !(node2p->allow&profile->allow))
          goto endloop;

       /* calculate the score for the segment and cumulative */
       if(option_quickest==0)
          segment_score=(score_t)DISTANCE(segment2p->distance)/segment_pref;
       else
          segment_score=(score_t)Duration(segment2p,way2p,profile)/segment_pref;

       cumulative_score=result1->score+segment_score;

       /* score must be better than current best score */
       if(cumulative_score>=total_score)
          goto endloop;

       /* find whether the node/segment combination already exists */
       result2=FindResult(results,node2,seg2);

       if(!result2) /* New end node/segment combination */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_score;
         }
       else if(cumulative_score<result2->score) /* New score for end node/segment combination is better */
         {
          result2->prev=result1;
          result2->score=cumulative_score;
          result2->segment=seg2;
         }
       else
          goto endloop;

       if(node2==finish_node)
         {
          total_score=cumulative_score;
          finish_result=result2;
         }
       else
          InsertInQueue(queue,result2,result2->score);

      endloop:

       if(IsFakeNode(node1))
          segment2p=NextFakeSegment(segment2p,node1);
       else if(IsFakeNode(node2))
          segment2p=NULL; /* cannot call NextSegment() with a fake segment */
       else
         {
          segment2p=NextSegment(segments,segment2p,node1);

          if(!segment2p && IsFakeNode(finish_node))
             segment2p=ExtraFakeSegment(node1,finish_node);
         }
      }
   }

 FreeQueueList(queue);

 /* Check it worked */

 if(!finish_result)
   {
#if DEBUG
    printf("      Failed\n");
#endif

    FreeResultsList(results);
    return(NULL);
   }

 /* Turn the route round and fill in the start and finish information */

 FixForwardRoute(results,finish_result);

 results->start_node  =start_result->node;
 results->prev_segment=start_result->segment;

 results->finish_node =finish_result->node;
 results->last_segment=finish_result->segment;

#if DEBUG
 printf("      -------- normal route (between super-nodes)\n");

 print_debug_route(nodes,segments,results,NULL,6,+1);
#endif

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes where the start and end are a set of pre/post-routed super-nodes.

  Results *FindMiddleRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  Results *begin The initial portion of the route.

  Results *end The final portion of the route.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *FindMiddleRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,Results *begin,Results *end)
{
 Results *results;
 Queue   *queue;
 Result  *begin_result,*end_result;
 Result  *start_result,*finish_result;
 score_t total_score;
 double  finish_lat,finish_lon;
 Result  *result1,*result2,*result3;
 int     force_uturn=0;
#ifdef LIBROUTINO
 int     loopcount=0;
#endif

#if DEBUG
 printf("  FindMiddleRoute(...,[begin has %d nodes],[end has %d nodes])\n",begin->number,end->number);
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_first("Finding Middle Route: Super-Nodes checked = 0");
#endif

 /* Set up the finish conditions */

 total_score=INF_SCORE;
 finish_result=NULL;

 if(IsFakeNode(end->finish_node))
    GetFakeLatLong(end->finish_node,&finish_lat,&finish_lon);
 else
    GetLatLong(nodes,end->finish_node,NULL,&finish_lat,&finish_lon);

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(20);
 queue=NewQueueList(12);

 if(begin->number==1 && begin->prev_segment!=NO_SEGMENT)
   {
    index_t superseg=FindSuperSegment(nodes,segments,ways,relations,profile,begin->start_node,begin->prev_segment);

    start_result=InsertResult(results,begin->start_node,superseg);
   }
 else
    start_result=InsertResult(results,begin->start_node,begin->prev_segment);

 /* Insert the finish points of the beginning part of the path into the queue,
    translating the segments into super-segments. */

 begin_result=FirstResult(begin);

 while(begin_result)
   {
    if((start_result->node!=begin_result->node || start_result->segment!=begin_result->segment) &&
       !IsFakeNode(begin_result->node) && IsSuperNode(LookupNode(nodes,begin_result->node,3)))
      {
       index_t superseg=FindSuperSegment(nodes,segments,ways,relations,profile,begin_result->node,begin_result->segment);

       result1=start_result;

       if(superseg!=begin_result->segment)
         {
          result1=InsertResult(results,begin_result->node,begin_result->segment);

          result1->score=begin_result->score;

          result1->prev=start_result;
         }

       if(!FindResult(results,begin_result->node,superseg))
         {
          result2=InsertResult(results,begin_result->node,superseg);

          result2->prev=result1;

          result2->score=begin_result->score;

          InsertInQueue(queue,result2,begin_result->score);

          if((end_result=FindResult(end,result2->node,result2->segment)))
            {
             if((result2->score+end_result->score)<total_score)
               {
                total_score=result2->score+end_result->score;
                finish_result=result2;
               }
            }
         }
      }

    begin_result=NextResult(begin,begin_result);
   }

 if(begin->number==1)
    InsertInQueue(queue,start_result,0);

 /* Check for barrier at start waypoint - must perform U-turn */

 if(begin->number==1 && start_result->segment!=NO_SEGMENT)
   {
    Node *startp=LookupNode(nodes,start_result->node,1);

    if(!(startp->allow&profile->allow))
       force_uturn=1;
   }

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    Node *node1p;
    Segment *segment2p;
    index_t node1,seg1;
    index_t turnrelation=NO_RELATION;

    /* score must be better than current best score */
    if(result1->score>=total_score)
       continue;

    node1=result1->node;
    seg1=result1->segment;

    node1p=LookupNode(nodes,node1,1); /* node1 cannot be a fake node (must be a super-node) */

    /* lookup if a turn restriction applies */
    if(profile->turns && IsTurnRestrictedNode(node1p)) /* node1 cannot be a fake node (must be a super-node) */
       turnrelation=FindFirstTurnRelation2(relations,node1,seg1);

    /* Loop across all segments */

    segment2p=FirstSegment(segments,node1p,1); /* node1 cannot be a fake node (must be a super-node) */

    while(segment2p)
      {
       Node *node2p;
       Way *way2p;
       index_t node2,seg2;
       score_t segment_pref,segment_score,cumulative_score;
       int i;

       /* must be a super segment */
       if(!IsSuperSegment(segment2p))
          goto endloop;

       /* must obey one-way restrictions (unless profile allows) */
       if(profile->oneway && IsOnewayTo(segment2p,node1))
         {
          if(profile->allow!=Transports_Bicycle)
             goto endloop;

          way2p=LookupWay(ways,segment2p->way,1);

          if(!(way2p->type&Highway_CycleBothWays))
             goto endloop;
         }

       seg2=IndexSegment(segments,segment2p); /* segment cannot be a fake segment (must be a super-segment) */

       /* must perform U-turn in special cases */
       if(force_uturn && node1==begin->start_node)
         {
          if(seg2!=result1->segment)
             goto endloop;
         }
       else
          /* must not perform U-turn */
          if(seg1==seg2) /* No fake segments, applies to all profiles */
             goto endloop;

       /* must obey turn relations */
       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,seg1,seg2,profile->allow))
          goto endloop;

       way2p=LookupWay(ways,segment2p->way,1);

       /* mode of transport must be allowed on the highway */
       if(!(way2p->allow&profile->allow))
          goto endloop;

       /* must obey weight restriction (if exists) */
       if(way2p->weight && way2p->weight<profile->weight)
          goto endloop;

       /* must obey height/width/length restriction (if exist) */
       if((way2p->height && way2p->height<profile->height) ||
          (way2p->width  && way2p->width <profile->width ) ||
          (way2p->length && way2p->length<profile->length))
          goto endloop;

       segment_pref=profile->highway[HIGHWAY(way2p->type)];

       /* highway preferences must allow this highway */
       if(segment_pref==0)
          goto endloop;

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way2p->props & PROPERTIES(i))
                segment_pref*=profile->props_yes[i];
             else
                segment_pref*=profile->props_no[i];
            }

       /* profile preferences must allow this highway */
       if(segment_pref==0)
          goto endloop;

       node2=OtherNode(segment2p,node1);

       node2p=LookupNode(nodes,node2,2); /* node2 cannot be a fake node (must be a super-node) */

       /* mode of transport must be allowed through node2 unless it is the final node */
       if(node2!=end->finish_node && !(node2p->allow&profile->allow))
          goto endloop;

       /* calculate the score for the segment and cumulative */
       if(option_quickest==0)
          segment_score=(score_t)DISTANCE(segment2p->distance)/segment_pref;
       else
          segment_score=(score_t)Duration(segment2p,way2p,profile)/segment_pref;

       cumulative_score=result1->score+segment_score;

       /* score must be better than current best score */
       if(cumulative_score>=total_score)
          goto endloop;

       /* find whether the node/segment combination already exists */
       result2=FindResult(results,node2,seg2);

       if(!result2) /* New end node/segment pair */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_score;
         }
       else if(cumulative_score<result2->score) /* New end node/segment pair is better */
         {
          result2->prev=result1;
          result2->score=cumulative_score;
         }
       else
          goto endloop;

       if((result3=FindResult(end,node2,seg2)))
         {
          if((result2->score+result3->score)<total_score)
            {
             total_score=result2->score+result3->score;
             finish_result=result2;
            }
         }
       else
         {
          double lat,lon;
          distance_t direct;
          score_t potential_score;

          GetLatLong(nodes,node2,node2p,&lat,&lon); /* node2 cannot be a fake node (must be a super-node) */

          direct=Distance(lat,lon,finish_lat,finish_lon);

          if(option_quickest==0)
             potential_score=result2->score+(score_t)direct/profile->max_pref;
          else
             potential_score=result2->score+(score_t)distance_speed_to_duration(direct,profile->max_speed)/profile->max_pref;

          if(potential_score<total_score)
             InsertInQueue(queue,result2,potential_score);
         }

      endloop:

       segment2p=NextSegment(segments,segment2p,node1); /* node1 cannot be a fake node (must be a super-node) */
      }

#ifdef LIBROUTINO
    if(!(++loopcount%100000))
       if(progress_func && !progress_func(progress_value))
         {
          progress_abort=1;
          break;
         }
#endif
   }

 FreeQueueList(queue);

 /* Check it worked */

 if(!finish_result)
   {
#if DEBUG
    printf("    Failed\n");
#endif

#if !DEBUG && !defined(LIBROUTINO)
    if(!option_quiet)
       printf_last("Found Middle Route: Super-Nodes checked = %d - Fail",results->number);
#endif

    FreeResultsList(results);
    return(NULL);
   }

 /* Turn the route round and fill in the start and finish information */

 FixForwardRoute(results,finish_result);

 results->start_node=start_result->node;
 results->prev_segment=start_result->segment;

 results->finish_node=finish_result->node;
 results->last_segment=finish_result->segment;

#if DEBUG
 printf("    -------- middle route (start route then via super-nodes/segments)\n");

 print_debug_route(nodes,segments,results,NULL,4,+1);
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_last("Found Middle Route: Super-Nodes checked = %d",results->number);
#endif

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the super-segment that represents the route that contains a particular segment.

  index_t FindSuperSegment Returns the index of the super-segment.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  index_t finish_node The super-node that the route ends at.

  index_t finish_segment The segment that the route ends with.
  ++++++++++++++++++++++++++++++++++++++*/

static index_t FindSuperSegment(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t finish_node,index_t finish_segment)
{
 Node *supernodep;
 Segment *supersegmentp;

#if DEBUG
 printf("    FindSuperSegment(...,finish_node=%"Pindex_t",finish_segment=%"Pindex_t")\n",finish_node,finish_segment);
#endif

 if(IsFakeSegment(finish_segment))
    finish_segment=IndexRealSegment(finish_segment);

 supernodep=LookupNode(nodes,finish_node,3); /* finish_node cannot be a fake node (must be a super-node) */
 supersegmentp=LookupSegment(segments,finish_segment,3); /* finish_segment cannot be a fake segment. */

 if(IsSuperSegment(supersegmentp))
   {
#if DEBUG
    printf("      -- already super-segment = %"Pindex_t"\n",finish_segment);
#endif

    return(finish_segment);
   }

 /* Loop across all segments */

 supersegmentp=FirstSegment(segments,supernodep,3); /* supernode cannot be a fake node (must be a super-node) */

 while(supersegmentp)
   {
    if(IsSuperSegment(supersegmentp))
      {
       Results *results;
       Result *result;
       index_t start_node;

       start_node=OtherNode(supersegmentp,finish_node);

       results=FindSuperRoute(nodes,segments,ways,relations,profile,start_node,finish_node);

       if(!results)
          continue;

       result=FindResult(results,finish_node,finish_segment);

       if(result && (distance_t)result->score==DISTANCE(supersegmentp->distance))
         {
          FreeResultsList(results);

#if DEBUG
          printf("      -- found super-segment = %"Pindex_t"\n",IndexSegment(segments,supersegmentp));
#endif

          return(IndexSegment(segments,supersegmentp));
         }

       if(results)
          FreeResultsList(results);
      }

    supersegmentp=NextSegment(segments,supersegmentp,finish_node); /* finish_node cannot be a fake node (must be a super-node) */
   }

#if DEBUG
    printf("      -- no super-segment = %"Pindex_t"\n",finish_segment);
#endif

 return(finish_segment);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the shortest route between two super-nodes using only normal nodes.
  This is effectively the same function as is used in superx.c when finding super-segments initially.

  Results *FindSuperRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  index_t start_node The start node.

  index_t finish_node The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *FindSuperRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t start_node,index_t finish_node)
{
 Results *results;
 Queue   *queue;
 Result  *result1,*result2;

#if DEBUG
 printf("      FindSuperRoute(...,start_node=%"Pindex_t" finish_node=%"Pindex_t")\n",start_node,finish_node);
#endif

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(8);
 queue=NewQueueList(8);

 result1=InsertResult(results,start_node,NO_SEGMENT);

 InsertInQueue(queue,result1,0);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    Node *node1p=NULL;
    Segment *segment2p;
    index_t node1,seg1;

    node1=result1->node;
    seg1=result1->segment;

    node1p=LookupNode(nodes,node1,4); /* node1 cannot be a fake node */

    /* Loop across all segments */

    segment2p=FirstSegment(segments,node1p,4); /* node1 cannot be a fake node */

    while(segment2p)
      {
       Node *node2p=NULL;
       index_t node2,seg2;
       score_t cumulative_score;

       /* must be a normal segment */
       if(!IsNormalSegment(segment2p))
          goto endloop;

       /* must obey one-way restrictions */
       if(IsOnewayTo(segment2p,node1))
         {
          Way *way2p;

          if(profile->allow!=Transports_Bicycle)
             goto endloop;

          way2p=LookupWay(ways,segment2p->way,2);

          if(!(way2p->type&Highway_CycleBothWays))
             goto endloop;
         }

       seg2=IndexSegment(segments,segment2p);

       /* must not perform U-turn */
       if(seg1==seg2)
          goto endloop;

       node2=OtherNode(segment2p,node1);

       node2p=LookupNode(nodes,node2,4); /* node2 cannot be a fake node */

       /* must not pass over super-node */
       if(node2!=finish_node && IsSuperNode(node2p))
          goto endloop;

       /* Specifically looking for the shortest route to emulate superx.c */
       cumulative_score=result1->score+(score_t)DISTANCE(segment2p->distance);

       result2=FindResult(results,node2,seg2);

       if(!result2) /* New end node/segment combination */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_score;
         }
       else if(cumulative_score<result2->score) /* New score for end node/segment combination is better */
         {
          result2->prev=result1;
          result2->segment=seg2;
          result2->score=cumulative_score;
         }
       else goto endloop;

       /* don't route beyond a super-node. */
       if(!IsSuperNode(node2p))
          InsertInQueue(queue,result2,result2->score);

      endloop:

       segment2p=NextSegment(segments,segment2p,node1);
      }
   }

 FreeQueueList(queue);

#if DEBUG
 Result *s=FirstResult(results);

 while(s)
   {
    if(s->node==finish_node)
      {
       printf("        -------- super-route\n");

       print_debug_route(nodes,segments,results,FindResult(results,s->node,s->segment),8,-1);
      }

    s=NextResult(results,s);
   }
#endif

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any super-node.

  Results *FindStartRoutes Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  index_t start_node The start node.

  index_t prev_segment The previous segment before the start node.

  index_t finish_node The finish node.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t start_node,index_t prev_segment,index_t finish_node)
{
 Results *results;
 Queue   *queue,*superqueue;
 Result  *result1,*result2;
 Result  *start_result,*finish_result=NULL;
 score_t total_score=INF_SCORE;
 int     nsuper=0,force_uturn=0;

#if DEBUG
 printf("  FindStartRoutes(...,start_node=%"Pindex_t" prev_segment=%"Pindex_t" finish_node=%"Pindex_t")\n",start_node,prev_segment,finish_node);
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_first("Finding Start Route: Nodes checked = 0");
#endif

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(8);
 queue=NewQueueList(8);
 superqueue=NewQueueList(8);

 start_result=InsertResult(results,start_node,prev_segment);

 InsertInQueue(queue,start_result,0);

 /* Check for barrier at start waypoint - must perform U-turn */

 if(prev_segment!=NO_SEGMENT && !IsFakeNode(start_node))
   {
    Node *startp=LookupNode(nodes,start_node,1);

    if(!(startp->allow&profile->allow))
       force_uturn=1;
   }

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    Node *node1p=NULL;
    Segment *segment2p;
    index_t node1,seg1,seg1r;
    index_t turnrelation=NO_RELATION;

    /* score must be better than current best score */
    if(result1->score>=total_score)
       continue;

    node1=result1->node;
    seg1=result1->segment;

    if(IsFakeSegment(seg1))
       seg1r=IndexRealSegment(seg1);
    else
       seg1r=seg1;

    if(!IsFakeNode(node1))
       node1p=LookupNode(nodes,node1,1);

    /* lookup if a turn restriction applies */
    if(profile->turns && node1p && IsTurnRestrictedNode(node1p))
       turnrelation=FindFirstTurnRelation2(relations,node1,seg1r);

    /* Loop across all segments */

    if(IsFakeNode(node1))
       segment2p=FirstFakeSegment(node1);
    else
       segment2p=FirstSegment(segments,node1p,1);

    while(segment2p)
      {
       Node *node2p=NULL;
       Way *way2p;
       index_t node2,seg2,seg2r;
       score_t segment_pref,segment_score,cumulative_score;
       int i;

       node2=OtherNode(segment2p,node1); /* need this here because we use node2 at the end of the loop */

       /* must be a normal segment */
       if(!IsNormalSegment(segment2p))
          goto endloop;

       /* must obey one-way restrictions (unless profile allows) */
       if(profile->oneway && IsOnewayTo(segment2p,node1))
         {
          if(profile->allow!=Transports_Bicycle)
             goto endloop;

          way2p=LookupWay(ways,segment2p->way,1);

          if(!(way2p->type&Highway_CycleBothWays))
             goto endloop;
         }

       if(IsFakeNode(node1) || IsFakeNode(node2))
         {
          seg2 =IndexFakeSegment(segment2p);
          seg2r=IndexRealSegment(seg2);
         }
       else
         {
          seg2 =IndexSegment(segments,segment2p);
          seg2r=seg2;
         }

       /* must perform U-turn in special cases */
       if(node1==start_node && force_uturn)
         {
          if(seg2r!=result1->segment)
             goto endloop;
         }
       else
          /* must not perform U-turn (unless profile allows) */
          if(profile->turns && (seg1==seg2 || seg1==seg2r || seg1r==seg2 || (seg1r==seg2r && IsFakeUTurn(seg1,seg2))))
             goto endloop;

       /* must obey turn relations */
       if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,seg1r,seg2r,profile->allow))
          goto endloop;

       way2p=LookupWay(ways,segment2p->way,1);

       /* mode of transport must be allowed on the highway */
       if(!(way2p->allow&profile->allow))
          goto endloop;

       /* must obey weight restriction (if exists) */
       if(way2p->weight && way2p->weight<profile->weight)
          goto endloop;

       /* must obey height/width/length restriction (if exists) */
       if((way2p->height && way2p->height<profile->height) ||
          (way2p->width  && way2p->width <profile->width ) ||
          (way2p->length && way2p->length<profile->length))
          goto endloop;

       segment_pref=profile->highway[HIGHWAY(way2p->type)];

       /* highway preferences must allow this highway */
       if(segment_pref==0)
          goto endloop;

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way2p->props & PROPERTIES(i))
                segment_pref*=profile->props_yes[i];
             else
                segment_pref*=profile->props_no[i];
            }

       /* profile preferences must allow this highway */
       if(segment_pref==0)
          goto endloop;

       if(!IsFakeNode(node2))
          node2p=LookupNode(nodes,node2,2);

       /* mode of transport must be allowed through node2 unless it is the final node */
       if(node2p && node2!=finish_node && !(node2p->allow&profile->allow))
          goto endloop;

       /* calculate the score for the segment and cumulative */
       if(option_quickest==0)
          segment_score=(score_t)DISTANCE(segment2p->distance)/segment_pref;
       else
          segment_score=(score_t)Duration(segment2p,way2p,profile)/segment_pref;

       /* prefer not to follow two fake segments when one would do (special case) */
       if(IsFakeSegment(seg2))
          segment_score*=1.01f;

       cumulative_score=result1->score+segment_score;

       /* score must be better than current best score (if finish node already found) */
       if(cumulative_score>=total_score)
          goto endloop;

       /* find whether the node/segment combination already exists */
       result2=FindResult(results,node2,seg2);

       if(!result2) /* New end node/segment combination */
         {
          result2=InsertResult(results,node2,seg2);
          result2->prev=result1;
          result2->score=cumulative_score;

          if(node2p && IsSuperNode(node2p))
             nsuper++;
         }
       else if(cumulative_score<result2->score) /* New score for end node/segment combination is better */
         {
          result2->prev=result1;
          result2->score=cumulative_score;
         }
       else
          goto endloop;

       if(node2==finish_node)
         {
          if(!finish_result)
            {
             Result *result3;

             while((result3=PopFromQueue(superqueue)))
                InsertInQueue(queue,result3,result3->score);
            }

          if(cumulative_score<total_score)
            {
             total_score=cumulative_score;
             finish_result=result2;
            }
         }

       if(finish_result || (node2p && !IsSuperNode(node2p)))
          InsertInQueue(queue,result2,result2->score);
       else if(node2p && IsSuperNode(node2p))
          InsertInQueue(superqueue,result2,result2->score);

      endloop:

       if(IsFakeNode(node1))
          segment2p=NextFakeSegment(segment2p,node1);
       else if(IsFakeNode(node2))
          segment2p=NULL; /* cannot call NextSegment() with a fake segment */
       else
         {
          segment2p=NextSegment(segments,segment2p,node1);

          if(!segment2p && IsFakeNode(finish_node))
             segment2p=ExtraFakeSegment(node1,finish_node);
         }
      }
   }

 FreeQueueList(queue);
 FreeQueueList(superqueue);

 /* Check it worked */

 if(results->number==1 || (nsuper==0 && !finish_result))
   {
#if DEBUG
    printf("    Failed (%d results, %d super)\n",results->number,nsuper);
#endif

#if !DEBUG && !defined(LIBROUTINO)
    if(!option_quiet)
       printf_last("Found Start Route: Nodes checked = %d - Fail",results->number);
#endif

    FreeResultsList(results);
    return(NULL);
   }

 /* Turn the route round and fill in the start and finish information */

 results->start_node  =start_result->node;
 results->prev_segment=start_result->segment;

 if(finish_result)
   {
    FixForwardRoute(results,finish_result);

    results->finish_node =finish_result->node;
    results->last_segment=finish_result->segment;
   }

#if DEBUG
 Result *s=FirstResult(results);

 while(s)
   {
    if(s->node==finish_node || (!IsFakeNode(s->node) && IsSuperNode(LookupNode(nodes,s->node,1))))
      {
       printf("    -------- possible start route\n");

       print_debug_route(nodes,segments,results,FindResult(results,s->node,s->segment),4,-1);
      }

    s=NextResult(results,s);
   }
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_last("Found Start Route: Nodes checked = %d",results->number);
#endif

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from any super-node to a specific node (by working backwards from the specific node to all super-nodes).

  Results *FindFinishRoutes Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  index_t finish_node The finishing node.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,index_t finish_node)
{
 Results *results,*finish_results;
 Queue   *queue;
 Result  *result1,*result2;
 Result  *finish_result;

#if DEBUG
 printf("  FindFinishRoutes(...,finish_node=%"Pindex_t")\n",finish_node);
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_first("Finding Finish Route: Nodes checked = 0");
#endif

 /* Create the results and insert the finish node into the queue */

 finish_results=NewResultsList(2);

 results=NewResultsList(8);
 queue=NewQueueList(8);

 finish_result=InsertResult(finish_results,finish_node,NO_SEGMENT);

 InsertInQueue(queue,finish_result,0);

 /* Loop across all nodes in the queue */

 while((result1=PopFromQueue(queue)))
   {
    Node *node1p=NULL;
    Segment *segment1p,*segment2p;
    Way *way1p;
    index_t real_node1,node1,seg1,seg1r;
    index_t turnrelation=NO_RELATION;
    score_t segment1_pref,segment1_score=0;
    int i;

    real_node1=result1->node;
    seg1=result1->segment;

    if(seg1!=NO_SEGMENT && IsFakeSegment(seg1))
       seg1r=IndexRealSegment(seg1);
    else
       seg1r=seg1;

    if(seg1!=NO_SEGMENT)
      {
       if(IsFakeSegment(seg1))
          segment1p=LookupFakeSegment(seg1);
       else
          segment1p=LookupSegment(segments,seg1,1);
      }

    if(seg1==NO_SEGMENT)
       node1=real_node1;
    else
       node1=OtherNode(segment1p,real_node1);

    if(!IsFakeNode(node1))
       node1p=LookupNode(nodes,node1,1);

    /* mode of transport must be allowed through node1 */
    if(seg1!=NO_SEGMENT)
       if(node1p && !(node1p->allow&profile->allow))
          continue;

    if(seg1!=NO_SEGMENT)
      {
       way1p=LookupWay(ways,segment1p->way,1);

       segment1_pref=profile->highway[HIGHWAY(way1p->type)];

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way1p->props & PROPERTIES(i))
                segment1_pref*=profile->props_yes[i];
             else
                segment1_pref*=profile->props_no[i];
            }

       /* calculate the score for the segment */
       if(option_quickest==0)
          segment1_score=(score_t)DISTANCE(segment1p->distance)/segment1_pref;
       else
          segment1_score=(score_t)Duration(segment1p,way1p,profile)/segment1_pref;

       /* prefer not to follow two fake segments when one would do (special case) */
       if(IsFakeSegment(seg1))
          segment1_score*=1.01f;
      }

    /* Loop across all segments */

    if(IsFakeNode(node1))
       segment2p=FirstFakeSegment(node1);
    else
       segment2p=FirstSegment(segments,node1p,1);

    while(segment2p)
      {
       Node *node2p=NULL;
       Way *way2p;
       index_t node2,seg2,seg2r;
       score_t segment_pref,cumulative_score;

       /* must be a normal segment unless node1 is a super-node (see below). */
       if((IsFakeNode(node1) || !IsSuperNode(node1p)) && !IsNormalSegment(segment2p))
          goto endloop;

       /* must be a super segment if node1 is a super-node to give starting super-segment for finding middle route. */
       if((!IsFakeNode(node1) && IsSuperNode(node1p)) && !IsSuperSegment(segment2p))
          goto endloop;

       /* must obey one-way restrictions (unless profile allows) */
       if(profile->oneway && IsOnewayFrom(segment2p,node1)) /* working backwards => disallow oneway *from* node1 */
         {
          if(profile->allow!=Transports_Bicycle)
             goto endloop;

          way2p=LookupWay(ways,segment2p->way,1);

          if(!(way2p->type&Highway_CycleBothWays))
             goto endloop;
         }

       node2=OtherNode(segment2p,node1);

       if(IsFakeNode(node1) || IsFakeNode(node2))
         {
          seg2 =IndexFakeSegment(segment2p);
          seg2r=IndexRealSegment(seg2);
         }
       else
         {
          seg2 =IndexSegment(segments,segment2p);
          seg2r=seg2;
         }

       if(seg1!=NO_SEGMENT)
         {
          /* must not perform U-turn (unless profile allows) */
          if(profile->turns)
            {
             if(IsFakeNode(node1) || !IsSuperNode(node1p))
               {
                if(seg1==seg2 || seg1==seg2r || seg1r==seg2 || (seg1r==seg2r && IsFakeUTurn(seg1,seg2)))
                   goto endloop;
               }
             else
               {
                index_t superseg=FindSuperSegment(nodes,segments,ways,relations,profile,node1,seg1);

                if(seg2==superseg)
                   goto endloop;
               }
            }

          /* lookup if a turn restriction applies */
          if(profile->turns && node1p && IsTurnRestrictedNode(node1p))
             turnrelation=FindFirstTurnRelation2(relations,node1,seg2r);

          /* must obey turn relations */
          if(turnrelation!=NO_RELATION && !IsTurnAllowed(relations,turnrelation,node1,seg2r,seg1r,profile->allow))
             goto endloop;
         }

       way2p=LookupWay(ways,segment2p->way,1);

       /* mode of transport must be allowed on the highway */
       if(!(way2p->allow&profile->allow))
          goto endloop;

       /* must obey weight restriction (if exists) */
       if(way2p->weight && way2p->weight<profile->weight)
          goto endloop;

       /* must obey height/width/length restriction (if exist) */
       if((way2p->height && way2p->height<profile->height) ||
          (way2p->width  && way2p->width <profile->width ) ||
          (way2p->length && way2p->length<profile->length))
          goto endloop;

       segment_pref=profile->highway[HIGHWAY(way2p->type)];

       /* highway preferences must allow this highway */
       if(segment_pref==0)
          goto endloop;

       for(i=1;i<Property_Count;i++)
          if(ways->file.props & PROPERTIES(i))
            {
             if(way2p->props & PROPERTIES(i))
                segment_pref*=profile->props_yes[i];
             else
                segment_pref*=profile->props_no[i];
            }

       /* profile preferences must allow this highway */
       if(segment_pref==0)
          goto endloop;

       if(!IsFakeNode(node2))
          node2p=LookupNode(nodes,node2,2);

       /* mode of transport must be allowed through node2 */
       if(node2p && !(node2p->allow&profile->allow))
          goto endloop;

       cumulative_score=result1->score+segment1_score;

       /* find whether the node/segment combination already exists */
       result2=FindResult(results,node1,seg2); /* adding in reverse => node1,seg2 */

       if(!result2) /* New end node */
         {
          result2=InsertResult(results,node1,seg2); /* adding in reverse => node1,seg2 */
          if(result1!=finish_result)
             result2->next=result1;   /* working backwards */
          result2->score=cumulative_score;
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          if(result1!=finish_result)
             result2->next=result1; /* working backwards */
          result2->score=cumulative_score;
         }
       else
          goto endloop;

       if(IsFakeNode(node1) || !IsSuperNode(node1p))
          InsertInQueue(queue,result2,result2->score);

      endloop:

       if(IsFakeNode(node1))
          segment2p=NextFakeSegment(segment2p,node1);
       else
          segment2p=NextSegment(segments,segment2p,node1);
      }
   }

 FreeQueueList(queue);

 FreeResultsList(finish_results);

 /* Check it worked */

 if(results->number==0)
   {
#if DEBUG
    printf("    Failed\n");
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_last("Found Finish Route: Nodes checked = %d - Fail",results->number);
#endif

    FreeResultsList(results);
    return(NULL);
   }

 /* Update the results */

 results->finish_node=finish_node;

#if DEBUG
 Result *s=FirstResult(results);

 while(s)
   {
    if(!IsFakeNode(s->node) && IsSuperNode(LookupNode(nodes,s->node,1)))
      {
       printf("    -------- possible finish route\n");

       print_debug_route(nodes,segments,results,FindResult(results,s->node,s->segment),4,+1);
      }

    s=NextResult(results,s);
   }
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_last("Found Finish Route: Nodes checked = %d",results->number);
#endif

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Create an optimum route given the set of super-nodes to follow.

  Results *CombineRoutes Returns the results from joining the super-nodes.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  Results *begin The set of results for the start of the route.

  Results *middle The set of results from the super-node route.

  Results *end The set of results for the end of the route.
  ++++++++++++++++++++++++++++++++++++++*/

static Results *CombineRoutes(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,Results *begin,Results *middle,Results *end)
{
 Result *midres,*comres;
 Results *combined;

#if DEBUG
 printf("  CombineRoutes(...,[begin has %d nodes],[middle has %d nodes],[end has %d nodes])\n",begin->number,middle->number,end->number);
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_first("Finding Combined Route: Nodes = 0");
#endif

 combined=NewResultsList(10);

 /* Insert the start point */

 midres=FindResult(middle,middle->start_node,middle->prev_segment);

 comres=InsertResult(combined,begin->start_node,begin->prev_segment);

 /* Insert the start of the route */

 if(begin->number>1 && midres->next)
   {
    Result *begres;

    midres=FindResult(middle,midres->next->node,midres->next->segment);

    begres=FindResult(begin,midres->node,midres->segment);

    FixForwardRoute(begin,begres);

    if(midres->next && midres->next->node==midres->node)
       midres=midres->next;

    begres=FindResult(begin,begin->start_node,begin->prev_segment);

    begres=begres->next;

    do
      {
       Result *comres2;

       comres2=InsertResult(combined,begres->node,begres->segment);

       comres2->score=begres->score;
       comres2->prev=comres;

       begres=begres->next;

       comres=comres2;
      }
    while(begres);
   }

 /* Sort out the combined route */

 while(midres->next)
   {
    Results *results=FindNormalRoute(nodes,segments,ways,relations,profile,comres->node,comres->segment,midres->next->node);
    Result *result;

    if(!results)
      {
#if !DEBUG && !defined(LIBROUTINO)
       if(!option_quiet)
          printf_last("Found Combined Route: Nodes = %d - Fail",combined->number);
#endif

       FreeResultsList(combined);
       return(NULL);
      }

    result=FindResult(results,midres->node,comres->segment);

    result=result->next;

    /*
     *      midres                          midres->next
     *         =                                  =
     *      ---*----------------------------------*  = middle
     *
     *      ---*----.----.----.----.----.----.----*  = results
     *              =
     *             result
     *
     *      ---*----.----.----.----.----.----.----*  = combined
     *         =    =
     *     comres  comres2
     */

    do
      {
       Result *comres2;

       comres2=InsertResult(combined,result->node,result->segment);

       comres2->score=midres->score+result->score;
       comres2->prev=comres;

       result=result->next;

       comres=comres2;
      }
    while(result);

    FreeResultsList(results);

    midres=midres->next;
   }

 /* Insert the end of the route */

 if(end->number>0)
   {
    Result *endres=FindResult(end,midres->node,midres->segment);

    while(endres->next)
      {
       Result *comres2;

       comres2=InsertResult(combined,endres->next->node,endres->next->segment);

       comres2->score=comres->score+(endres->score-endres->next->score);
       comres2->prev=comres;

       endres=endres->next;

       comres=comres2;
      }
   }

 /* Turn the route round and fill in the start and finish information */

 FixForwardRoute(combined,comres);

 combined->start_node=begin->start_node;
 combined->prev_segment=begin->prev_segment;

 combined->finish_node=comres->node;
 combined->last_segment=comres->segment;

#if DEBUG
 printf("    -------- combined route (end-to-end)\n");

 print_debug_route(nodes,segments,combined,NULL,4,+1);
#endif

#if !DEBUG && !defined(LIBROUTINO)
 if(!option_quiet)
    printf_last("Found Combined Route: Nodes = %d",combined->number);
#endif

 return(combined);
}


/*++++++++++++++++++++++++++++++++++++++
  Fix the forward route (i.e. setup next pointers for forward path from prev nodes on reverse path).

  Results *results The set of results to update.

  Result *finish_result The result for the finish point.
  ++++++++++++++++++++++++++++++++++++++*/

static void FixForwardRoute(Results *results,Result *finish_result)
{
 Result *current_result=finish_result;

 do
   {
    Result *result;

    if(current_result->prev)
      {
       index_t node1=current_result->prev->node;
       index_t seg1=current_result->prev->segment;

       result=FindResult(results,node1,seg1);

#ifndef LIBROUTINO
       logassert(!result->next,"Unable to reverse route through results (report a bug)"); /* Bugs elsewhere can lead to infinite loop here. */
#endif

       result->next=current_result;

       current_result=result;
      }
    else
       current_result=NULL;
   }
 while(current_result);
}


#if DEBUG

/*++++++++++++++++++++++++++++++++++++++
  Print a debug message about a route.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Results *results The set of results to print.

  Result *first The result to start with or NULL for the first result.

  int indent The number of spaces of indentation at the beginning.

  int direction The direction of travel, -1 = backwards (prev) or +1 = forwards (next).
  ++++++++++++++++++++++++++++++++++++++*/

static void print_debug_route(Nodes *nodes,Segments *segments,Results *results,Result *first,int indent,int direction)
{
 Result *r;
 char *spaces="        ";

 if(first)
    r=first;
 else
    r=FindResult(results,results->start_node,results->prev_segment);

 while(r)
   {
    int is_fake_node=IsFakeNode(r->node);
    int is_super_node=is_fake_node?0:IsSuperNode(LookupNode(nodes,r->node,4));
    int is_no_segment=(r->segment==NO_SEGMENT);
    int is_fake_segment=is_no_segment?0:IsFakeSegment(r->segment);
    int is_super_segment=is_no_segment||is_fake_segment?0:IsSuperSegment(LookupSegment(segments,r->segment,4));
    int is_normal_segment=is_no_segment||is_fake_segment?0:IsNormalSegment(LookupSegment(segments,r->segment,4));
    int is_start=r->node==results->start_node&&r->segment==results->prev_segment;
    int is_finish=r->node==results->finish_node;

    printf("%s %s node=%10"Pindex_t" segment=%10"Pindex_t" score=%8.3f (%s-node,%s-segment)%s%s\n",
           &spaces[8-indent],
           (is_start||is_finish?"*":(direction==-1?"^":"v")),
           r->node,r->segment,r->score,
           (is_fake_node?"  fake":(is_super_node?" super":"normal")),
           (is_no_segment?"    no":(is_fake_segment?"  fake":(is_super_segment&&is_normal_segment?"  both":(is_super_segment?" super":"normal")))),
           (is_start?" [start]":""),
           (is_finish?" [finish]":""));

    if(direction==-1)
       r=r->prev;
    else
       r=r->next;
   }
}

#endif
