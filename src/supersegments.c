/***************************************
 $Header: /home/amb/CVS/routino/src/supersegments.c,v 1.11 2009-01-20 17:37:20 amb Exp $

 Super-Segment data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "nodes.h"
#include "ways.h"
#include "segments.h"
#include "functions.h"


/*++++++++++++++++++++++++++++++++++++++
  Select the super-segments from the list of segments.

  NodesMem *ChooseSuperNodes Returns the list of super-nodes.

  Nodes *nodes The nodes.

  Segments *segments The existing segments.

  Ways *ways The ways.
  ++++++++++++++++++++++++++++++++++++++*/

NodesMem *ChooseSuperNodes(Nodes *nodes,Segments *segments,Ways *ways)
{
 int i;
 int        exitcount=0,difference=0;
 node_t     node=0;
 speed_t    limit=0;
 waytype_t  type=0;
 wayallow_t allow=0;
 NodesMem *supernodes;

 /* Create super-nodes */

 supernodes=NewNodeList();

 /* Find super-nodes */

 node=segments->segments[0].node1;

 for(i=0;i<segments->number;i++)
   {
    Segment *segment=&segments->segments[i];
    Way *way=FindWay(ways,segment->way);

    if(segment->node1!=node)
      {
       /* Store the node if there is a difference in the ways that could affect routing.
          Store the node if it is not a dead-end and if it isn't just the middle of a way. */

       if(difference || exitcount>2)
         {
          Node *oldnode=FindNode(nodes,node);

          AppendNode(supernodes,node,oldnode->latitude,oldnode->longitude);
         }

       exitcount=0;
       difference=0;

       node=segment->node1;
       type=Way_TYPE(way->type);
       limit=way->limit;
       allow=way->allow;
      }
    else                        /* Same starting node */
      {
       if(Way_TYPE(way->type)!=type)
          difference=1;

       if(way->limit!=limit)
          difference=1;

       if(way->allow!=allow)
          difference=1;
      }

    if(segment->distance!=INVALID_DISTANCE)
      {
       if(way->type&Way_OneWay)
          exitcount+=2;
       else
          exitcount+=1;
      }

    if(!((i+1)%10000))
      {
       printf("\rFinding Super-Nodes: Segments=%d Super-Nodes=%d",i+1,supernodes->number);
       fflush(stdout);
      }
   }

 printf("\rFound Super-Nodes: Segments=%d Super-Nodes=%d  \n",segments->number,supernodes->number);
 fflush(stdout);

 return(supernodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the super-segments.

  SegmentsMem *CreateSuperSegments Returns the set of super-segments.

  Nodes *nodes The list of nodes.

  Segments *segments The list of segments.

  Ways *ways The list of ways.

  Nodes *supernodes The list of super-nodes.

  int iteration The iteration number of super-segment generation.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsMem *CreateSuperSegments(Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes,int iteration)
{
 SegmentsMem *supersegments;
 int i,j;

 /* Create super-segments */

 supersegments=NewSegmentList();

 /* Create super-segments for each super-node. */

 for(i=0;i<supernodes->number;i++)
   {
    Segment *segment,*first;

    segment=first=FindFirstSegment(segments,supernodes->nodes[i].id);

    while(segment)
      {
       Way *way=FindWay(ways,segment->way);

       /* Check that this type of way hasn't already been routed */

       if(segment!=first)
         {
          Segment *othersegment=first;

          while(othersegment && othersegment!=segment)
            {
             Way *otherway=FindWay(ways,othersegment->way);

             if(Way_TYPE(otherway->type)==Way_TYPE(way->type) &&
                otherway->allow==way->allow &&
                otherway->limit==way->limit)
               {
                way=NULL;
                break;
               }

             othersegment=FindNextSegment(segments,othersegment);
            }
         }

       /* Route the way and store the super-segments. */

       if(way)
         {
          Results *results=FindRoutesWay(nodes,segments,ways,supernodes->nodes[i].id,supernodes,way,iteration);

          for(j=0;j<results->number;j++)
             if(results->results[j].node!=supernodes->nodes[i].id && FindNode(supernodes,results->results[j].node))
               {
                Segment *supersegment=AppendSegment(supersegments,supernodes->nodes[i].id,results->results[j].node,segment->way);

                supersegment->distance=results->results[j].shortest.distance;
               }

          FreeResultsList(results);
         }

       segment=FindNextSegment(segments,segment);
      }

    if(!((i+1)%1000))
      {
       printf("\rCreating Super-Segments: Super-Nodes=%d Super-Segments=%d",i+1,supersegments->number);
       fflush(stdout);
      }
   }

 printf("\rCreated Super-Segments: Super-Nodes=%d Super-Segments=%d \n",supernodes->number,supersegments->number);
 fflush(stdout);

 return(supersegments);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the Super-Ways from the Super-Segments.

  WaysMem *CreateSuperWays Returns the set of super-ways.

  Ways *ways The list of ways.

  SegmentsMem *supersegments The list of super-segments.
  ++++++++++++++++++++++++++++++++++++++*/

WaysMem *CreateSuperWays(Ways *ways,SegmentsMem *supersegments)
{
 WaysMem *superways;
 int i,j;

 /* Create super-ways */

 superways=NewWayList();

 /* Create a new super-way to replace each existing way. */

 for(i=0;i<supersegments->segments->number;i++)
   {
    Way *way=FindWay(ways,supersegments->segments->segments[i].way);

    supersegments->segments->segments[i].way=0;

    for(j=0;j<superways->number;j++)
       if(Way_TYPE(superways->ways->ways[j].type)==Way_TYPE(way->type) &&
          superways->ways->ways[j].allow==way->allow &&
          superways->ways->ways[j].limit==way->limit)
         {
          supersegments->segments->segments[i].way=superways->ways->ways[j].id;
          break;
         }

    if(!supersegments->segments->segments[i].way)
      {
       Way *newway=AppendWay(superways,superways->number+1,"Super-Way");

       newway->limit=way->limit;
       newway->type =Way_TYPE(way->type);
       newway->allow=way->allow;
       newway->speed=way->speed;

       supersegments->segments->segments[i].way=newway->id;
      }

    if(!((i+1)%10000))
      {
       printf("\rCreating Super-Ways: Super-Segments=%d Super-Ways=%d",i+1,superways->number);
       fflush(stdout);
      }
   }

 printf("\rCreated Super-Ways: Super-Segments=%d Super-Ways=%d \n",supersegments->number,superways->number);
 fflush(stdout);

 return(superways);
}
