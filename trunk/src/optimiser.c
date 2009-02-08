/***************************************
 $Header: /home/amb/CVS/routino/src/optimiser.c,v 1.49 2009-02-08 12:03:50 amb Exp $

 Routing optimiser.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <string.h>
#include <stdio.h>

#include "types.h"
#include "functions.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "results.h"


/*+ Print the progress? +*/
extern int option_quiet;


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
 HalfResult shortestfinish,quickestfinish;
 Result *result1,*result2;
 Segment *segment;
 Way *way;

 /* Set up the finish conditions */

 shortestfinish.distance=~0;
 shortestfinish.duration=~0;
 quickestfinish.distance=~0;
 quickestfinish.duration=~0;

 /* Insert the first node into the queue */

 if(all)
    results=NewResultsList(65536);
 else
    results=NewResultsList(8);

 result1=InsertResult(results,start);

 result1->node=start;
 result1->shortest.prev=0;
 result1->shortest.next=0;
 result1->shortest.distance=0;
 result1->shortest.duration=0;
 result1->quickest.prev=0;
 result1->quickest.next=0;
 result1->quickest.distance=0;
 result1->quickest.duration=0;

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    node1=result1->node;

    segment=FirstSegment(segments,LookupNode(nodes,node1));

    while(segment)
      {
       HalfResult shortest2,quickest2;
       duration_t segment_duration;

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->shortest.prev==node2 && result1->quickest.prev==node2)
          goto endloop;

       way=LookupWay(ways,segment->way);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highways[HIGHWAY(way->type)])
          goto endloop;

       segment_duration=Duration(segment,way,profile);

       shortest2.distance=result1->shortest.distance+DISTANCE(segment->distance);
       shortest2.duration=result1->shortest.duration+segment_duration;
       quickest2.distance=result1->quickest.distance+DISTANCE(segment->distance);
       quickest2.duration=result1->quickest.duration+segment_duration;

       if(shortest2.distance>shortestfinish.distance && quickest2.duration>quickestfinish.duration)
          goto endloop;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=node1;
          result2->shortest.next=0;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=node1;
          result2->quickest.next=0;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(node2==finish)
            {
             shortestfinish.distance=shortest2.distance;
             shortestfinish.duration=shortest2.duration;
             quickestfinish.distance=quickest2.distance;
             quickestfinish.duration=quickest2.duration;
            }
          else
             insert_in_queue(result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.prev=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(node2==finish)
               {
                shortestfinish.distance=shortest2.distance;
                shortestfinish.duration=shortest2.duration;
               }
             else
                insert_in_queue(result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.prev=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(node2==finish)
               {
                quickestfinish.distance=quickest2.distance;
                quickestfinish.duration=quickest2.duration;
               }
             else
                insert_in_queue(result2);
            }
         }

      endloop:

       if(!option_quiet && !(results->number%10000))
         {
          printf("\rRouting: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin  ",results->number,
                 distance_to_km(shortest2.distance),duration_to_minutes(shortest2.duration),
                 distance_to_km(quickest2.distance),duration_to_minutes(quickest2.duration));
          fflush(stdout);
         }

       segment=NextSegment(segments,segment,node1);
      }
   }

 if(!option_quiet)
   {
    printf("\rRouted: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin\n",results->number,
           distance_to_km(shortestfinish.distance),duration_to_minutes(shortestfinish.duration),
           distance_to_km(quickestfinish.distance),duration_to_minutes(quickestfinish.duration));
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
    if(result2->shortest.prev)
      {
       index_t node1=result2->shortest.prev;

       result1=FindResult(results,node1);

       result1->shortest.next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 result2=FindResult(results,finish);

 do
   {
    if(result2->quickest.prev)
      {
       index_t node1=result2->quickest.prev;

       result1=FindResult(results,node1);

       result1->quickest.next=result2->node;

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

  index_t start The start node.

  index_t finish The finish node.

  Results *begin The initial portion of the route.

  Results *end The final portion of the route.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoute3(Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Results *begin,Results *end,Profile *profile)
{
 Results *results;
 index_t node1,node2;
 HalfResult shortestfinish,quickestfinish;
 Result *result1,*result2,*result3;
 Segment *segment;
 Way *way;

 /* Set up the finish conditions */

 shortestfinish.distance=~0;
 shortestfinish.duration=~0;
 quickestfinish.distance=~0;
 quickestfinish.duration=~0;

 /* Insert the start node */

 results=NewResultsList(65536);

 result1=InsertResult(results,start);
 result3=FindResult(begin,start);

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

          result2->shortest.prev=start;
          result2->quickest.prev=start;
         }

       insert_in_queue(result2);
      }

    result3=NextResult(begin,result3);
   }

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    node1=result1->node;

    segment=FirstSegment(segments,LookupNode(nodes,node1));

    while(segment)
      {
       HalfResult shortest2,quickest2;
       duration_t segment_duration;

       if(!IsSuperSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->shortest.prev==node2 && result1->quickest.prev==node2)
          goto endloop;

       way=LookupWay(ways,segment->way);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highways[HIGHWAY(way->type)])
          goto endloop;

       segment_duration=Duration(segment,way,profile);

       shortest2.distance=result1->shortest.distance+DISTANCE(segment->distance);
       shortest2.duration=result1->shortest.duration+segment_duration;
       quickest2.distance=result1->quickest.distance+DISTANCE(segment->distance);
       quickest2.duration=result1->quickest.duration+segment_duration;

       if(shortest2.distance>shortestfinish.distance && quickest2.duration>quickestfinish.duration)
          goto endloop;

       result2=FindResult(results,node2);
       result3=NULL;

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=node1;
          result2->shortest.next=0;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=node1;
          result2->quickest.next=0;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(!(result3=FindResult(end,node2)))
             insert_in_queue(result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.prev=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(!(result3=FindResult(end,node2)))
                insert_in_queue(result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.prev=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!(result3=FindResult(end,node2)))
                insert_in_queue(result2);
            }
         }

       if(result3)
         {
          if((shortest2.distance+result3->shortest.distance)<shortestfinish.distance ||
             ((shortest2.distance+result3->shortest.distance)==shortestfinish.distance &&
              (shortest2.duration+result3->shortest.duration)<shortestfinish.duration))
            {
             shortestfinish.distance=shortest2.distance+result3->shortest.distance;
             shortestfinish.duration=shortest2.duration+result3->shortest.duration;
            }
          if((quickest2.duration+result3->quickest.duration)<quickestfinish.duration ||
             ((quickest2.duration+result3->quickest.duration)==quickestfinish.duration &&
              (quickest2.distance+result3->quickest.distance)<quickestfinish.distance))
            {
             quickestfinish.distance=quickest2.distance+result3->quickest.distance;
             quickestfinish.duration=quickest2.duration+result3->quickest.duration;
            }
         }

      endloop:

       if(!option_quiet && !(results->number%10000))
         {
          printf("\rRouting: End Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin  ",results->number,
                 distance_to_km(shortest2.distance),duration_to_minutes(shortest2.duration),
                 distance_to_km(quickest2.distance),duration_to_minutes(quickest2.duration));
          fflush(stdout);
         }

       segment=NextSegment(segments,segment,node1);
      }
   }

 if(!option_quiet)
   {
    printf("\rRouted: End Super-Nodes=%d Shortest=%.1fkm,%.0fmin Quickest=%.1fkm,%.0fmin\n",results->number,
           distance_to_km(shortestfinish.distance),duration_to_minutes(shortestfinish.duration),
           distance_to_km(quickestfinish.distance),duration_to_minutes(quickestfinish.duration));
    fflush(stdout);
   }

 /* Finish off the end part of the route. */

 if(!FindResult(results,finish))
   {
    result2=InsertResult(results,finish);
    result3=FindResult(end,finish);

    *result2=*result3;

    result2->shortest.distance=~0;
    result2->shortest.duration=~0;
    result2->quickest.distance=~0;
    result2->quickest.duration=~0;

    result3=FirstResult(end);

    while(result3)
      {
       if(IsSuperNode(LookupNode(nodes,result3->node)))
          if((result1=FindResult(results,result3->node)))
            {
             if((result1->shortest.distance+result3->shortest.distance)<result2->shortest.distance ||
                ((result1->shortest.distance+result3->shortest.distance)==result2->shortest.distance &&
                 (result1->shortest.duration+result3->shortest.duration)<result2->shortest.duration))
               {
                result2->shortest.distance=result1->shortest.distance+result3->shortest.distance;
                result2->shortest.duration=result1->shortest.duration+result3->shortest.duration;
                result2->shortest.prev=result1->node;
               }
             if((result1->quickest.duration+result3->quickest.duration)<result2->quickest.duration ||
                ((result1->quickest.duration+result3->quickest.duration)==result2->quickest.duration &&
                 (result1->quickest.distance+result3->quickest.distance)<result2->quickest.distance))
               {
                result2->quickest.distance=result1->quickest.distance+result3->quickest.distance;
                result2->quickest.duration=result1->quickest.duration+result3->quickest.duration;
                result2->quickest.prev=result1->node;
               }
            }

       result3=NextResult(end,result3);
      }
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
    if(result2->shortest.prev)
      {
       index_t node1=result2->shortest.prev;

       result1=FindResult(results,node1);

       result1->shortest.next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 result2=FindResult(results,finish);

 do
   {
    if(result2->quickest.prev)
      {
       index_t node1=result2->quickest.prev;

       result1=FindResult(results,node1);

       result1->quickest.next=result2->node;

       result2=result1;
      }
    else
       result2=NULL;
   }
 while(result2);

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the optimum route between two nodes.

  Results *Results The set of results to print.

  Nodes *nodes The list of nodes.

  Segments *segments The set of segments to use.

  Ways *ways The list of ways.

  index_t start The start node.

  index_t finish The finish node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile)
{
 FILE *textfile,*gpxfile;
 Result *result;

 /* Print the result for the shortest route */

 textfile=fopen("shortest.txt","w");
 gpxfile=fopen("shortest.gpx","w");

 fprintf(gpxfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 fprintf(gpxfile,"<gpx version=\"1.0\" creator=\"Router\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/0\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n");
 fprintf(gpxfile,"<trk>\n");
 fprintf(gpxfile,"<trkseg>\n");

 result=FindResult(results,start);

 do
   {
    float latitude,longitude;
    Node *node=LookupNode(nodes,result->node);

    GetLatLong(nodes,node,&latitude,&longitude);

    fprintf(gpxfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"></trkpt>\n",latitude,longitude);

    if(result->shortest.prev)
      {
       Segment *segment;
       Way *way;

       segment=FirstSegment(segments,LookupNode(nodes,result->shortest.prev));
       while(NODE(segment->node2)!=result->node && NODE(segment->node1)!=result->node)
          segment=NextSegment(segments,segment,result->shortest.prev);

       way=LookupWay(ways,segment->way);

       fprintf(textfile,"%8.4f %9.4f %8d%c %5.3f %5.2f %7.2f %5.1f %3d %s\n",latitude,longitude,
               result->node,IsSuperNode(node)?'*':' ',
               distance_to_km(DISTANCE(segment->distance)),duration_to_minutes(Duration(segment,way,profile)),
               distance_to_km(result->shortest.distance),duration_to_minutes(result->shortest.duration),
               profile->speed[HIGHWAY(way->type)],WayName(ways,way));
      }
    else
       fprintf(textfile,"%8.4f %9.4f %8d%c %5.3f %5.2f %7.2f %5.1f\n",latitude,longitude,
               result->node,IsSuperNode(node)?'*':' ',
               0.0,0.0,0.0,0.0);

    if(result->shortest.next)
       result=FindResult(results,result->shortest.next);
    else
       result=NULL;
   }
 while(result);

 fprintf(gpxfile,"</trkseg>\n");
 fprintf(gpxfile,"</trk>\n");
 fprintf(gpxfile,"</gpx>\n");

 fclose(textfile);
 fclose(gpxfile);

 /* Print the result for the quickest route */

 textfile=fopen("quickest.txt","w");
 gpxfile=fopen("quickest.gpx","w");

 fprintf(gpxfile,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 fprintf(gpxfile,"<gpx version=\"1.0\" creator=\"Router\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/0\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n");
 fprintf(gpxfile,"<trk>\n");
 fprintf(gpxfile,"<trkseg>\n");

 result=FindResult(results,start);

 do
   {
    float latitude,longitude;
    Node *node=LookupNode(nodes,result->node);

    GetLatLong(nodes,node,&latitude,&longitude);

    fprintf(gpxfile,"<trkpt lat=\"%.6f\" lon=\"%.6f\"></trkpt>\n",latitude,longitude);

    if(result->quickest.prev)
      {
       Segment *segment;
       Way *way;

       segment=FirstSegment(segments,LookupNode(nodes,result->quickest.prev));
       while(NODE(segment->node2)!=result->node && NODE(segment->node1)!=result->node)
          segment=NextSegment(segments,segment,result->quickest.prev);

       way=LookupWay(ways,segment->way);

       fprintf(textfile,"%8.4f %9.4f %8d%c %5.3f %5.2f %7.2f %5.1f %3d %s\n",latitude,longitude,
               result->node,IsSuperNode(node)?'*':' ',
               distance_to_km(DISTANCE(segment->distance)),duration_to_minutes(Duration(segment,way,profile)),
               distance_to_km(result->quickest.distance),duration_to_minutes(result->quickest.duration),
               profile->speed[HIGHWAY(way->type)],WayName(ways,way));
      }
    else
       fprintf(textfile,"%8.4f %9.4f %8d%c %5.3f %5.2f %7.2f %5.1f\n",latitude,longitude,
               result->node,IsSuperNode(node)?'*':' ',
               0.0,0.0,0.0,0.0);

    if(result->quickest.next)
       result=FindResult(results,result->quickest.next);
    else
       result=NULL;
   }
 while(result);

 fprintf(gpxfile,"</trkseg>\n");
 fprintf(gpxfile,"</trk>\n");
 fprintf(gpxfile,"</gpx>\n");

 fclose(textfile);
 fclose(gpxfile);
}


/*++++++++++++++++++++++++++++++++++++++
  Find all routes from a specified node to any super-node.

  Results *FindRoutes Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  index_t start The start node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t start,Profile *profile)
{
 Results *results;
 index_t node1,node2;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 result1=InsertResult(results,start);

 result1->node=start;
 result1->shortest.prev=0;
 result1->shortest.next=0;
 result1->shortest.distance=0;
 result1->shortest.duration=0;
 result1->quickest.prev=0;
 result1->quickest.next=0;
 result1->quickest.distance=0;
 result1->quickest.duration=0;

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    node1=result1->node;

    segment=FirstSegment(segments,LookupNode(nodes,node1));

    while(segment)
      {
       duration_t segment_duration;

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayTo(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->shortest.prev==node2 && result1->quickest.prev==node2)
          goto endloop;

       way=LookupWay(ways,segment->way);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highways[HIGHWAY(way->type)])
          goto endloop;

       segment_duration=Duration(segment,way,profile);

       shortest2.distance=result1->shortest.distance+DISTANCE(segment->distance);
       shortest2.duration=result1->shortest.duration+segment_duration;
       quickest2.distance=result1->quickest.distance+DISTANCE(segment->distance);
       quickest2.duration=result1->quickest.duration+segment_duration;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=node1;
          result2->shortest.next=0;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=node1;
          result2->quickest.next=0;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(!IsSuperNode(LookupNode(nodes,node2)))
             insert_in_queue(result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.prev=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(!IsSuperNode(LookupNode(nodes,node2)))
                insert_in_queue(result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.prev=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!IsSuperNode(LookupNode(nodes,node2)))
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

  Results *FindReverseRoute Returns a set of results.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  index_t finish The finishing node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *FindReverseRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t finish,Profile *profile)
{
 Results *results;
 index_t node1,node2;
 HalfResult shortest2,quickest2;
 Result *result1,*result2;
 Segment *segment;
 Way *way;

 /* Insert the first node into the queue */

 results=NewResultsList(8);

 result1=InsertResult(results,finish);

 result1->node=finish;
 result1->shortest.prev=0;
 result1->shortest.next=0;
 result1->shortest.distance=0;
 result1->shortest.duration=0;
 result1->quickest.prev=0;
 result1->quickest.next=0;
 result1->quickest.distance=0;
 result1->quickest.duration=0;

 insert_in_queue(result1);

 /* Loop across all nodes in the queue */

 while((result1=pop_from_queue()))
   {
    node1=result1->node;

    segment=FirstSegment(segments,LookupNode(nodes,node1));

    while(segment)
      {
       duration_t segment_duration;

       if(!IsNormalSegment(segment))
          goto endloop;

       if(profile->oneway && IsOnewayFrom(segment,node1))
          goto endloop;

       node2=OtherNode(segment,node1);

       if(result1->shortest.next==node2 && result1->quickest.next==node2)
          goto endloop;

       way=LookupWay(ways,segment->way);

       if(!(way->allow&profile->allow))
          goto endloop;

       if(!profile->highways[HIGHWAY(way->type)])
          goto endloop;

       segment_duration=Duration(segment,way,profile);

       shortest2.distance=result1->shortest.distance+DISTANCE(segment->distance);
       shortest2.duration=result1->shortest.duration+segment_duration;
       quickest2.distance=result1->quickest.distance+DISTANCE(segment->distance);
       quickest2.duration=result1->quickest.duration+segment_duration;

       result2=FindResult(results,node2);

       if(!result2)                         /* New end node */
         {
          result2=InsertResult(results,node2);
          result2->node=node2;
          result2->shortest.prev=0;
          result2->shortest.next=node1;
          result2->shortest.distance=shortest2.distance;
          result2->shortest.duration=shortest2.duration;
          result2->quickest.prev=0;
          result2->quickest.next=node1;
          result2->quickest.distance=quickest2.distance;
          result2->quickest.duration=quickest2.duration;

          if(!IsSuperNode(LookupNode(nodes,node2)))
             insert_in_queue(result2);
         }
       else
         {
          if(shortest2.distance<result2->shortest.distance ||
             (shortest2.distance==result2->shortest.distance &&
              shortest2.duration<result2->shortest.duration)) /* New end node is shorter */
            {
             result2->shortest.next=node1;
             result2->shortest.distance=shortest2.distance;
             result2->shortest.duration=shortest2.duration;

             if(!IsSuperNode(LookupNode(nodes,node2)))
                insert_in_queue(result2);
            }

          if(quickest2.duration<result2->quickest.duration ||
             (quickest2.duration==result2->quickest.duration &&
              quickest2.distance<result2->quickest.distance)) /* New end node is quicker */
            {
             result2->quickest.next=node1;
             result2->quickest.distance=quickest2.distance;
             result2->quickest.duration=quickest2.duration;

             if(!IsSuperNode(LookupNode(nodes,node2)))
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

  index_t start The start node.

  index_t finish The finish node.

  Profile *profile The profile containing the transport type, speeds and allowed highways.
  ++++++++++++++++++++++++++++++++++++++*/

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile)
{
 Result *result1,*result2,*result3,*result4;
 Results *combined;
 int quiet=option_quiet;

 combined=NewResultsList(64);

 option_quiet=1;

 /* Sort out the shortest */

 result1=FindResult(results,start);

 result3=InsertResult(combined,start);

 result3->node=result1->node;

 result3->shortest.distance=0;
 result3->shortest.duration=0;
 result3->shortest.next=0;
 result3->shortest.prev=0;

 do
   {
    if(result1->shortest.next)
      {
       Results *results2=FindRoute(nodes,segments,ways,result1->node,result1->shortest.next,profile,0);

       result2=FindResult(results2,result1->node);

       result3->shortest.next=result2->shortest.next;

       result2=FindResult(results2,result2->shortest.next);

       do
         {
          result4=InsertResult(combined,result2->node);

          result4->node=result2->node;

          result4->shortest=result2->shortest;
          result4->shortest.distance+=result3->shortest.distance;
          result4->shortest.duration+=result3->shortest.duration;

          if(result2->shortest.next)
             result2=FindResult(results2,result2->shortest.next);
          else
             result2=NULL;
         }
       while(result2);

       FreeResultsList(results2);

       result1=FindResult(results,result1->shortest.next);

       result3=result4;
      }
    else
       result1=NULL;
   }
 while(result1);

 /* Sort out the quickest */

 result1=FindResult(results,start);

 result3=FindResult(combined,start);

 result3->quickest.distance=0;
 result3->quickest.duration=0;
 result3->quickest.next=0;
 result3->quickest.prev=0;

 do
   {
    if(result1->quickest.next)
      {
       Results *results2=FindRoute(nodes,segments,ways,result1->node,result1->quickest.next,profile,0);

       result2=FindResult(results2,result1->node);

       result3->quickest.next=result2->quickest.next;

       result2=FindResult(results2,result2->quickest.next);

       do
         {
          result4=FindResult(combined,result2->node);

          if(!result4)
            {
             result4=InsertResult(combined,result2->node);

             result4->node=result2->node;
            }

          result4->quickest=result2->quickest;
          result4->quickest.distance+=result3->quickest.distance;
          result4->quickest.duration+=result3->quickest.duration;

          if(result2->quickest.next)
             result2=FindResult(results2,result2->quickest.next);
          else
             result2=NULL;
         }
       while(result2);

       FreeResultsList(results2);

       result1=FindResult(results,result1->quickest.next);

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
