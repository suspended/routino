/***************************************
 $Header: /home/amb/CVS/routino/src/supersegments.c,v 1.23 2009-01-31 15:32:42 amb Exp $

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

  NodesMem *nodesmem The nodes.

  SegmentsMem *segmentsmem The segments.

  WaysMem *waysmem The ways.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

void ChooseSuperNodes(NodesMem *nodesmem,SegmentsMem *segmentsmem,WaysMem *waysmem,int iteration)
{
 int i;
 int        segcount=0,difference=0,nnodes=0;
 node_t     node=0;
 speed_t    limit=0;
 waytype_t  type=0;
 wayallow_t allow=0;

 /* Find super-nodes */

 node=segmentsmem->xdata[0].node1;

 for(i=0;i<segmentsmem->number;i++)
   {
    SegmentEx *segmentex=LookupSegmentEx(segmentsmem,i);
    WayEx *wayex=LookupWayEx(waysmem,segmentex->segment.way);

    if(segmentex->node1!=node)
      {
       /* Store the node if there is a difference in the ways that could affect routing.
          Store the node if it is not a dead-end and if it isn't just the middle of a way. */

       if(difference || segcount>2)
         {
          NodeEx *nodeex=FindNode(nodesmem,node);

          nodeex->super++;

          nnodes++;
         }

       segcount=1;
       difference=0;

       node=segmentex->node1;
       type=wayex->way.type;
       limit=wayex->way.limit;
       allow=wayex->way.allow;
      }
    else                        /* Same starting node */
      {
       if(wayex->way.type!=type)
          difference=1;

       if(wayex->way.limit!=limit)
          difference=1;

       if(wayex->way.allow!=allow)
          difference=1;

       segcount+=1;
      }

    if(!((i+1)%10000))
      {
       printf("\rFinding Super-Nodes: Segments=%d Super-Nodes=%d",i+1,nnodes);
       fflush(stdout);
      }
   }

 printf("\rFound Super-Nodes: Segments=%d Super-Nodes=%d  \n",segmentsmem->number,nnodes);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Create the super-segments.

  SegmentsMem *CreateSuperSegments Creates the super segments.

  NodesMem *nodesmem The nodes.

  SegmentsMem *segmentsmem The segments.

  WaysMem *waysmem The ways.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

SegmentsMem *CreateSuperSegments(NodesMem *nodesmem,SegmentsMem *segmentsmem,WaysMem *waysmem,int iteration)
{
 int i;
 SegmentsMem *supersegmentsmem;

 supersegmentsmem=NewSegmentList();

 /* Create super-segments for each super-node. */

 for(i=0;i<nodesmem->number;i++)
   {
    if(nodesmem->xdata[i].super>iteration)
      {
       SegmentEx *segmentex,*first;

       segmentex=first=FindFirstSegment(segmentsmem,nodesmem->xdata[i].id);

       while(segmentex)
         {
          WayEx *wayex=LookupWayEx(waysmem,segmentex->segment.way);

          /* Check that this type of way hasn't already been routed */

          if(segmentex!=first)
            {
             SegmentEx *othersegmentex=first;

             while(othersegmentex && othersegmentex!=segmentex)
               {
                WayEx *otherwayex=LookupWayEx(waysmem,othersegmentex->segment.way);

                if(otherwayex->way.type ==wayex->way.type  &&
                   otherwayex->way.allow==wayex->way.allow &&
                   otherwayex->way.limit==wayex->way.limit)
                  {
                   wayex=NULL;
                   break;
                  }

                othersegmentex=FindNextSegment(segmentsmem,othersegmentex);
               }
            }

          /* Route the way and store the super-segments. */

          if(wayex)
            {
             Results *results=FindRoutesWay(nodesmem,segmentsmem,waysmem,nodesmem->xdata[i].id,wayex,iteration);
             Result *result=FirstResult(results);

             while(result)
               {
                NodeEx *nodeex=FindNode(nodesmem,result->node);

                if(result->node!=nodesmem->xdata[i].id && nodeex->super>iteration)
                  {
                   SegmentEx *supersegmentex=AppendSegment(supersegmentsmem,nodesmem->xdata[i].id,result->node,IndexWayEx(waysmem,wayex));

                   supersegmentex->segment.distance=result->shortest.distance;

                   if(wayex->way.type&Way_OneWay)
                     {
                      supersegmentex=AppendSegment(supersegmentsmem,result->node,nodesmem->xdata[i].id,IndexWayEx(waysmem,wayex));

                      supersegmentex->segment.distance=ONEWAY_OPPOSITE|result->shortest.distance;
                     }
                  }

                result=NextResult(results,result);
               }

             FreeResultsList(results);
            }

          segmentex=FindNextSegment(segmentsmem,segmentex);
         }
      }

    if(!((i+1)%10000))
      {
       printf("\rCreating Super-Segments: Nodes=%d Super-Segments=%d",i+1,supersegmentsmem->number);
       fflush(stdout);
      }
   }

 printf("\rCreated Super-Segments: Nodes=%d Super-Segments=%d \n",nodesmem->number,supersegmentsmem->number);
 fflush(stdout);

 /* Append the new supersegments onto the segments. */

 return(supersegmentsmem);
}
