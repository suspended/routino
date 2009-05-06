/***************************************
 $Header: /home/amb/CVS/routino/src/optimiser.c,v 1.66 2009-05-06 18:26:41 amb Exp $

 Routing optimiser.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008,2009 Andrew M. Bishop

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
#include "functions.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "results.h"


/*+ The option not to print any progress information. +*/
extern int option_quiet;

/*+ The option to calculate the quickest route insted of the shortest. +*/
extern int option_quickest;


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes.

  Results *FindRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  index_t start The start node.

  index_t finish The finish node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.

  int all A flag to indicate that a big results structure is required.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute(Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile,int all)
{
 Results *results;
 index_t node1,node2;
 distance_t finish_distance;
 duration_t finish_duration;
 score_t finish_score;
 float finish_lat,finish_lon;
 speed_t max_speed=0;
 score_t max_pref=0;
 Result *result1,*result2;
 Segment *segment;
 Way *way;
 int i;

 /* Set up the finish conditions */

 finish_distance=~0;
 finish_duration=~0;
 finish_score   =~(distance_t)0;

 GetLatLong(nodes,LookupNode(nodes,finish),&finish_lat,&finish_lon);

 for(i=0;i<sizeof(profile->speed)/sizeof(profile->speed[0]);i++)
    if(profile->speed[i]>max_speed)
       max_speed=profile->speed[i];

 for(i=0;i<sizeof(profile->highway)/sizeof(profile->highway[0]);i++)
    if(profile->highway[i]>max_pref)
       max_pref=profile->highway[i];

 /* Create the list of results and insert the first node into the queue */

 if(all)
    results=NewResultsList(65536);
 else
    results=NewResultsList(8);

 results->start=start;
 results->finish=finish;

 result1=InsertResult(results,start);

 ZeroResult(result1);

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    if(result1->sortby>finish_score)
       continue;

    node1=result1->node;

    segment=FirstSegment(segments,LookupNode(nodes,node1));

    while(segment)
      {
       distance_t segment_distance,cumulative_distance;
       duration_t segment_duration,cumulative_duration;
       score_t    segment_score,   cumulative_score;

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->prev==node2)
          goto endloop;

       way=LookupWay(ways,segment->way);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highway[HIGHWAY(way->type)])
          goto endloop;

       if(way->weight<profile->weight)
          goto endloop;

       if(way->height<profile->height || way->width<profile->width || way->length<profile->length)
          goto endloop;

       segment_distance=DISTANCE(segment->distance);
       segment_duration=Duration(segment,way,profile);
       if(option_quickest==0)
          segment_score=(score_t)segment_distance/profile->highway[HIGHWAY(way->type)];
       else
          segment_score=(score_t)segment_duration/profile->highway[HIGHWAY(way->type)];

       cumulative_distance=result1->distance+segment_distance;
       cumulative_duration=result1->duration+segment_duration;
       cumulative_score   =result1->score   +segment_score;

       if(cumulative_score>finish_score)
          goto endloop;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev=node1;
          result2->next=0;
          result2->distance=cumulative_distance;
          result2->duration=cumulative_duration;
          result2->score   =cumulative_score;

          if(node2==finish)
            {
             finish_distance=cumulative_distance;
             finish_duration=cumulative_duration;
             finish_score   =cumulative_score;
            }
          else
            {
             float lat,lon;
             distance_t direct;

             GetLatLong(nodes,LookupNode(nodes,node2),&lat,&lon);
             direct=Distance(lat,lon,finish_lat,finish_lon);

             if(option_quickest==0)
                result2->sortby=result2->score+(score_t)direct/max_pref;
             else
                result2->sortby=result2->score+(score_t)distance_speed_to_duration(direct,max_speed)/max_pref;

             insert_in_queue(result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->prev=node1;
          result2->distance=cumulative_distance;
          result2->duration=cumulative_duration;
          result2->score   =cumulative_score;

          if(node2==finish)
            {
             finish_score=cumulative_score;
            }
          else if(!all)
            {
             insert_in_queue(result2);
            }
          else
            {
             float lat,lon;
             distance_t direct;

             GetLatLong(nodes,LookupNode(nodes,node2),&lat,&lon);
             direct=Distance(lat,lon,finish_lat,finish_lon);

             if(option_quickest==0)
                result2->sortby=result2->score+(score_t)direct/max_pref;
             else
                result2->sortby=result2->score+(score_t)distance_speed_to_duration(direct,max_speed)/max_pref;

             insert_in_queue(result2);
            }
         }

      endloop:

       if(!option_quiet && !(results->number%10000))
         {
          printf("\rRouting: End Nodes=%d %.1fkm %.0fmin  ",results->number,
                 distance_to_km(result1->distance),duration_to_minutes(result1->duration));
          fflush(stdout);
         }

       segment=NextSegment(segments,segment,node1);
      }
   }

 if(!option_quiet)
   {
    printf("\rRouted: End Nodes=%d %.1fkm %.0fmin \n",results->number,
           distance_to_km(finish_distance),duration_to_minutes(finish_duration));
    fflush(stdout);
   }

 /* Check it worked */

 result2=FindResult(results,finish);

 if(!result2)
   {
    FreeResultsList(results);
    return(NULL);
   }

 /* Reverse the results */

 result2=FindResult(results,finish);

 do
   {
    if(result2->prev)
      {
       index_t node1=result2->prev;

       result1=FindResult(results,node1);

       result1->next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the optimum route between two nodes.

  Results *FindRoute3 Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Results *begin The initial portion of the route.

  Results *end The final portion of the route.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute3(Nodes *nodes,Segments *segments,Ways *ways,Results *begin,Results *end,Profile *profile)
{
 Results *results;
 index_t node1,node2;
 distance_t finish_distance;
 duration_t finish_duration;
 score_t finish_score;
 float finish_lat,finish_lon;
 speed_t max_speed=0;
 score_t max_pref=0;
 Result *result1,*result2,*result3;
 Segment *segment;
 Way *way;
 int i;

 /* Set up the finish conditions */

 finish_distance=~0;
 finish_duration=~0;
 finish_score   =~(distance_t)0;

 GetLatLong(nodes,LookupNode(nodes,end->finish),&finish_lat,&finish_lon);

 for(i=0;i<sizeof(profile->speed)/sizeof(profile->speed[0]);i++)
    if(profile->speed[i]>max_speed)
       max_speed=profile->speed[i];

 for(i=0;i<sizeof(profile->highway)/sizeof(profile->highway[0]);i++)
    if(profile->highway[i]>max_pref)
       max_pref=profile->highway[i];

 /* Create the list of results and insert the first node into the queue */

 results=NewResultsList(65536);

 results->start=begin->start;
 results->finish=end->finish;

 result1=InsertResult(results,begin->start);
 result3=FindResult(begin,begin->start);

 *result1=*result3;

 /* Insert the finish points of the beginning part of the path into the queue */

 result3=FirstResult(begin);

 while(result3)
   {
    if(IsSuperNode(LookupNode(nodes,result3->node)))
      {
       if(!(result2=FindResult(results,result3->node)))
         {
          result2=InsertResult(results,result3->node);

          *result2=*result3;

          result2->prev=begin->start;

          result2->sortby=result2->score;
         }

       insert_in_queue(result2);
      }

    result3=NextResult(begin,result3);
   }

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    if(result1->sortby>finish_score)
       continue;

    node1=result1->node;

    segment=FirstSegment(segments,LookupNode(nodes,node1));

    while(segment)
      {
       distance_t segment_distance,cumulative_distance;
       duration_t segment_duration,cumulative_duration;
       score_t    segment_score,   cumulative_score;

       if(!IsSuperSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->prev==node2)
          goto endloop;

       way=LookupWay(ways,segment->way);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highway[HIGHWAY(way->type)])
          goto endloop;

       if(way->weight<profile->weight)
          goto endloop;

       if(way->height<profile->height || way->width<profile->width || way->length<profile->length)
          goto endloop;

       segment_distance=DISTANCE(segment->distance);
       segment_duration=Duration(segment,way,profile);
       if(option_quickest==0)
          segment_score=(score_t)segment_distance/profile->highway[HIGHWAY(way->type)];
       else
          segment_score=(score_t)segment_duration/profile->highway[HIGHWAY(way->type)];

       cumulative_distance=result1->distance+segment_distance;
       cumulative_duration=result1->duration+segment_duration;
       cumulative_score   =result1->score   +segment_score;

       if(cumulative_score>finish_score)
          goto endloop;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev=node1;
          result2->next=0;
          result2->distance=cumulative_distance;
          result2->duration=cumulative_duration;
          result2->score   =cumulative_score;

          if((result3=FindResult(end,node2)))
            {
             finish_distance=result2->distance+result3->distance;
             finish_duration=result2->duration+result3->duration;
             finish_score   =result2->score   +result3->score;
            }
          else
            {
             float lat,lon;
             distance_t direct;

             GetLatLong(nodes,LookupNode(nodes,node2),&lat,&lon);
             direct=Distance(lat,lon,finish_lat,finish_lon);

             if(option_quickest==0)
                result2->sortby=result2->score+(score_t)direct/max_pref;
             else
                result2->sortby=result2->score+(score_t)distance_speed_to_duration(direct,max_speed)/max_pref;

             insert_in_queue(result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->prev=node1;
          result2->distance=cumulative_distance;
          result2->duration=cumulative_duration;
          result2->score   =cumulative_score;

          if((result3=FindResult(end,node2)))
            {
             if((result2->score+result3->score)<finish_score)
               {
                finish_score=result2->score+result3->score;
               }
            }
          else
            {
             float lat,lon;
             distance_t direct;

             GetLatLong(nodes,LookupNode(nodes,node2),&lat,&lon);
             direct=Distance(lat,lon,finish_lat,finish_lon);

             if(option_quickest==0)
                result2->sortby=result2->score+(score_t)direct/max_pref;
             else
                result2->sortby=result2->score+(score_t)distance_speed_to_duration(direct,max_speed)/max_pref;

             insert_in_queue(result2);
            }
         }

      endloop:

       if(!option_quiet && !(results->number%10000))
         {
          printf("\rRouting: End Nodes=%d %.1fkm %.0fmin  ",results->number,
                 distance_to_km(result1->distance),duration_to_minutes(result1->duration));
          fflush(stdout);
         }

       segment=NextSegment(segments,segment,node1);
      }
   }

 if(!option_quiet)
   {
    printf("\rRouted: End Super-Nodes=%d %.1fkm %.0fmin \n",results->number,
           distance_to_km(finish_distance),duration_to_minutes(finish_duration));
    fflush(stdout);
   }

 /* Finish off the end part of the route. */

 if(!FindResult(results,end->finish))
   {
    result2=InsertResult(results,end->finish);
    result3=FindResult(end,end->finish);

    *result2=*result3;

    result2->distance=~0;
    result2->duration=~0;

    result3=FirstResult(end);

    while(result3)
      {
       if(IsSuperNode(LookupNode(nodes,result3->node)))
          if((result1=FindResult(results,result3->node)))
            {
             if(option_quickest==0) /* shortest */
               {
                if((result1->distance+result3->distance)<result2->distance ||
                   ((result1->distance+result3->distance)==result2->distance &&
                    (result1->duration+result3->duration)<result2->duration))
                  {
                   result2->distance=result1->distance+result3->distance;
                   result2->duration=result1->duration+result3->duration;
                   result2->prev=result1->node;
                  }
               }
             else
               {
                if((result1->duration+result3->duration)<result2->duration ||
                   ((result1->duration+result3->duration)==result2->duration &&
                    (result1->distance+result3->distance)<result2->distance))
                  {
                   result2->distance=result1->distance+result3->distance;
                   result2->duration=result1->duration+result3->duration;
                   result2->prev=result1->node;
                  }
               }
            }

       result3=NextResult(end,result3);
      }
   }

 /* Check it worked */

 if(finish_distance == ~0)
   {
    FreeResultsList(results);
    return(NULL);
   }

 /* Reverse the results */

 result2=FindResult(results,end->finish);

 do
   {
    if(result2->prev)
      {
       index_t node1=result2->prev;

       result1=FindResult(results,node1);

       result1->next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any super-node.

  Results *FindStartRoutes Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  index_t start The start node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t start,Profile *profile)
{
 Results *results;
 index_t node1,node2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 results->start=start;

 result1=InsertResult(results,start);

 ZeroResult(result1);

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    node1=result1->node;

    segment=FirstSegment(segments,LookupNode(nodes,node1));

    while(segment)
      {
       distance_t segment_distance,cumulative_distance;
       duration_t segment_duration,cumulative_duration;
       score_t    segment_score,   cumulative_score;

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->prev==node2)
          goto endloop;

       way=LookupWay(ways,segment->way);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highway[HIGHWAY(way->type)])
          goto endloop;

       if(way->weight<profile->weight)
          goto endloop;

       if(way->height<profile->height || way->width<profile->width || way->length<profile->length)
          goto endloop;

       segment_distance=DISTANCE(segment->distance);
       segment_duration=Duration(segment,way,profile);
       if(option_quickest==0)
          segment_score=(score_t)segment_distance/profile->highway[HIGHWAY(way->type)];
       else
          segment_score=(score_t)segment_duration/profile->highway[HIGHWAY(way->type)];

       cumulative_distance=result1->distance+segment_distance;
       cumulative_duration=result1->duration+segment_duration;
       cumulative_score   =result1->score   +segment_score;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev=node1;
          result2->next=0;
          result2->distance=cumulative_distance;
          result2->duration=cumulative_duration;
          result2->score   =cumulative_score;

          if(!IsSuperNode(LookupNode(nodes,node2)))
            {
             result2->sortby=result2->score;
             insert_in_queue(result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->prev=node1;
          result2->distance=cumulative_distance;
          result2->duration=cumulative_duration;
          result2->score   =cumulative_score;

          if(!IsSuperNode(LookupNode(nodes,node2)))
            {
             result2->sortby=result2->score;
             insert_in_queue(result2);
            }
         }

      endloop:

       segment=NextSegment(segments,segment,node1);
      }
   }

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

  index_t finish The finishing node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t finish,Profile *profile)
{
 Results *results;
 index_t node1,node2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 results->finish=finish;

 result1=InsertResult(results,finish);

 ZeroResult(result1);

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    node1=result1->node;

    segment=FirstSegment(segments,LookupNode(nodes,node1));

    while(segment)
      {
       distance_t segment_distance,cumulative_distance;
       duration_t segment_duration,cumulative_duration;
       score_t    segment_score,   cumulative_score;

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayFrom(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->next==node2)
          goto endloop;

       way=LookupWay(ways,segment->way);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highway[HIGHWAY(way->type)])
          goto endloop;

       if(way->weight<profile->weight)
          goto endloop;

       if(way->height<profile->height || way->width<profile->width || way->length<profile->length)
          goto endloop;

       segment_distance=DISTANCE(segment->distance);
       segment_duration=Duration(segment,way,profile);
       if(option_quickest==0)
          segment_score=(score_t)segment_distance/profile->highway[HIGHWAY(way->type)];
       else
          segment_score=(score_t)segment_duration/profile->highway[HIGHWAY(way->type)];

       cumulative_distance=result1->distance+segment_distance;
       cumulative_duration=result1->duration+segment_duration;
       cumulative_score   =result1->score   +segment_score;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->prev=0;
          result2->next=node1;
          result2->distance=cumulative_distance;
          result2->duration=cumulative_duration;
          result2->score   =cumulative_score;

          if(!IsSuperNode(LookupNode(nodes,node2)))
            {
             result2->sortby=result2->score;
             insert_in_queue(result2);
            }
         }
       else if(cumulative_score<result2->score) /* New end node is better */
         {
          result2->next=node1;
          result2->distance=cumulative_distance;
          result2->duration=cumulative_duration;
          result2->score   =cumulative_score;

          if(!IsSuperNode(LookupNode(nodes,node2)))
            {
             result2->sortby=result2->score;
             insert_in_queue(result2);
            }
         }

      endloop:

       segment=NextSegment(segments,segment,node1);
      }
   }

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

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile)
{
 Result *result1,*result2,*result3,*result4;
 Results *combined;
 int quiet=option_quiet;

 combined=NewResultsList(64);

 combined->start=results->start;
 combined->finish=results->finish;

 option_quiet=1;

 /* Sort out the combined route */

 result1=FindResult(results,results->start);

 result3=InsertResult(combined,results->start);

 ZeroResult(result3);

 do
   {
    if(result1->next)
      {
       Results *results2=FindRoute(nodes,segments,ways,result1->node,result1->next,profile,0);

       result2=FindResult(results2,result1->node);

       result3->next=result2->next;

       result2=FindResult(results2,result2->next);

       do
         {
          result4=InsertResult(combined,result2->node);

          *result4=*result2;
          result4->distance+=result3->distance;
          result4->duration+=result3->duration;
          result4->score   +=result3->score;

          if(result2->next)
             result2=FindResult(results2,result2->next);
          else
             result2=NULL;
         }
       while(result2);

       FreeResultsList(results2);

       result1=FindResult(results,result1->next);

       result3=result4;
      }
    else
       result1=NULL;
   }
 while(result1);

 /* Now print out the result normally */

 option_quiet=quiet;

 return(combined);
}
