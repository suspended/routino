/***************************************
 Extract data from Routino.

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "relations.h"
#include "errorlog.h"

#include "typesx.h"

#include "visualiser.h"


/* Limit types */

#define SPEED_LIMIT  1
#define WEIGHT_LIMIT 2
#define HEIGHT_LIMIT 3
#define WIDTH_LIMIT  4
#define LENGTH_LIMIT 5

/* Local types */

typedef void (*callback_t)(index_t node,double latitude,double longitude);

/* Local variables (intialised by entry-point function before later use) */

static Nodes     *OSMNodes;
static Segments  *OSMSegments;
static Ways      *OSMWays;
static Relations *OSMRelations;

static double LatMin;
static double LatMax;
static double LonMin;
static double LonMax;

static int limit_type=0;
static Highway highways=Highway_None;
static Transports transports=Transports_None;
static Properties properties=Properties_None;
static highway_t waytype=0;

/* Local functions */

static void find_all_nodes(Nodes *nodes,callback_t callback);

static void output_junctions(index_t node,double latitude,double longitude);
static void output_super(index_t node,double latitude,double longitude);
static void output_waytype(index_t node,double latitude,double longitude);
static void output_highway(index_t node,double latitude,double longitude);
static void output_transport(index_t node,double latitude,double longitude);
static void output_barrier(index_t node,double latitude,double longitude);
static void output_turnrestriction(index_t node,double latitude,double longitude);
static void output_limits(index_t node,double latitude,double longitude);
static void output_property(index_t node,double latitude,double longitude);


/*++++++++++++++++++++++++++++++++++++++
  Output the data for junctions (--data=junctions).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputJunctions(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,(callback_t)output_junctions);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output all those that are junctions (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_junctions(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segmentp;
 Way *firstwayp;
 int count=0,difference=0;

 segmentp=FirstSegment(OSMSegments,nodep,1);
 firstwayp=LookupWay(OSMWays,segmentp->way,1);

 do
   {
    Way *wayp=LookupWay(OSMWays,segmentp->way,2);

    if(IsNormalSegment(segmentp))
       count++;

    if(WaysCompare(firstwayp,wayp))
       difference=1;

    segmentp=NextSegment(OSMSegments,segmentp,node);
   }
 while(segmentp);

 if(count!=2 || difference)
    printf("node%"Pindex_t" %.6f %.6f %d\n",node,radians_to_degrees(latitude),radians_to_degrees(longitude),count);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for super-nodes and super-segments (--data=super).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputSuper(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,(callback_t)output_super);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output all that are super-nodes and all connected super-segments (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_super(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segmentp;

 if(!IsSuperNode(nodep))
    return;

 printf("node%"Pindex_t" %.6f %.6f\n",node,radians_to_degrees(latitude),radians_to_degrees(longitude));

 segmentp=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsSuperSegment(segmentp))
      {
       index_t othernode=OtherNode(segmentp,node);
       double lat,lon;

       GetLatLong(OSMNodes,othernode,NULL,&lat,&lon);

       if(node>othernode || (lat<LatMin || lat>LatMax || lon<LonMin || lon>LonMax))
          printf("segment%"Pindex_t" %.6f %.6f\n",IndexSegment(OSMSegments,segmentp),radians_to_degrees(lat),radians_to_degrees(lon));
      }

    segmentp=NextSegment(OSMSegments,segmentp,node);
   }
 while(segmentp);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for segments of special highway types (--data=waytype-oneway etc).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.

  highway_t mask A bit mask that must match the highway type.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputWaytype(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax,highway_t mask)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 waytype=mask;

 find_all_nodes(nodes,(callback_t)output_waytype);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output all connected segments of a particular special highway type (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_waytype(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segmentp;

 segmentp=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsNormalSegment(segmentp))
      {
       Way *wayp=LookupWay(OSMWays,segmentp->way,1);

       if(wayp->type&waytype)
         {
          index_t othernode=OtherNode(segmentp,node);
          double lat,lon;

          GetLatLong(OSMNodes,othernode,NULL,&lat,&lon);

          if(node>othernode || (lat<LatMin || lat>LatMax || lon<LonMin || lon>LonMax))
            {
             if(IsOnewayFrom(segmentp,node))
                printf("segment%"Pindex_t" %.6f %.6f %.6f %.6f\n",IndexSegment(OSMSegments,segmentp),radians_to_degrees(latitude),radians_to_degrees(longitude),radians_to_degrees(lat),radians_to_degrees(lon));
             else if(IsOnewayFrom(segmentp,othernode))
                printf("segment%"Pindex_t" %.6f %.6f %.6f %.6f\n",IndexSegment(OSMSegments,segmentp),radians_to_degrees(lat),radians_to_degrees(lon),radians_to_degrees(latitude),radians_to_degrees(longitude));
            }
         }
      }

    segmentp=NextSegment(OSMSegments,segmentp,node);
   }
 while(segmentp);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for segments of a particular highway type (--data=highway-primary etc).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.

  Highway highway The type of highway.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputHighway(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax,Highway highway)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 highways=highway;

 find_all_nodes(nodes,(callback_t)output_highway);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output all connected segments that are of a particular highway type (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_highway(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segmentp;

 segmentp=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsNormalSegment(segmentp))
      {
       Way *wayp=LookupWay(OSMWays,segmentp->way,1);

       if(HIGHWAY(wayp->type)==highways)
         {
          index_t othernode=OtherNode(segmentp,node);
          double lat,lon;

          GetLatLong(OSMNodes,othernode,NULL,&lat,&lon);

          if(node>othernode || (lat<LatMin || lat>LatMax || lon<LonMin || lon>LonMax))
             printf("segment%"Pindex_t" %.6f %.6f %.6f %.6f\n",IndexSegment(OSMSegments,segmentp),radians_to_degrees(latitude),radians_to_degrees(longitude),radians_to_degrees(lat),radians_to_degrees(lon));
         }
      }

    segmentp=NextSegment(OSMSegments,segmentp,node);
   }
 while(segmentp);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for segments allowed for a particular type of traffic (--data=transport-motorcar etc).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.

  Transport transport The type of transport.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputTransport(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax,Transport transport)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 transports=TRANSPORTS(transport);

 find_all_nodes(nodes,(callback_t)output_transport);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output all connected segments for a particular traffic type (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_transport(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segmentp;

 segmentp=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsNormalSegment(segmentp))
      {
       Way *wayp=LookupWay(OSMWays,segmentp->way,1);

       if(wayp->allow&transports)
         {
          index_t othernode=OtherNode(segmentp,node);
          double lat,lon;

          GetLatLong(OSMNodes,othernode,NULL,&lat,&lon);

          if(node>othernode || (lat<LatMin || lat>LatMax || lon<LonMin || lon>LonMax))
             printf("segment%"Pindex_t" %.6f %.6f %.6f %.6f\n",IndexSegment(OSMSegments,segmentp),radians_to_degrees(latitude),radians_to_degrees(longitude),radians_to_degrees(lat),radians_to_degrees(lon));
         }
      }

    segmentp=NextSegment(OSMSegments,segmentp,node);
   }
 while(segmentp);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for nodes disallowed for a particular type of traffic (--data=barrier).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.

  Transport transport The type of transport.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputBarrier(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax,Transport transport)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 transports=TRANSPORTS(transport);

 find_all_nodes(nodes,(callback_t)output_barrier);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output those that are barriers (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_barrier(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);

 if(!(nodep->allow&transports))
    printf("node%"Pindex_t" %.6f %.6f\n",node,radians_to_degrees(latitude),radians_to_degrees(longitude));
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for turn restrictions (--data=turns).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputTurnRestrictions(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,(callback_t)output_turnrestriction);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output those that are 'via' nodes for a turn restriction and the associated segments (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_turnrestriction(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 index_t turnrelation=NO_RELATION;

 if(!IsTurnRestrictedNode(nodep))
    return;

 turnrelation=FindFirstTurnRelation1(OSMRelations,node);

 do
   {
    TurnRelation *relation;
    Segment *from_segmentp,*to_segmentp;
    index_t from_node,to_node;
    double from_lat,from_lon,to_lat,to_lon;

    relation=LookupTurnRelation(OSMRelations,turnrelation,1);

    from_segmentp=LookupSegment(OSMSegments,relation->from,1);
    to_segmentp  =LookupSegment(OSMSegments,relation->to  ,2);

    from_node=OtherNode(from_segmentp,node);
    to_node  =OtherNode(to_segmentp  ,node);

    GetLatLong(OSMNodes,from_node,NULL,&from_lat,&from_lon);
    GetLatLong(OSMNodes,to_node  ,NULL,&to_lat  ,&to_lon);

    printf("turn-relation%"Pindex_t" %.6f %.6f %.6f %.6f %.6f %.6f\n",
           turnrelation,
           radians_to_degrees(from_lat),radians_to_degrees(from_lon),
           radians_to_degrees(latitude),radians_to_degrees(longitude),
           radians_to_degrees(to_lat),radians_to_degrees(to_lon));

    turnrelation=FindNextTurnRelation1(OSMRelations,turnrelation);
   }
 while(turnrelation!=NO_RELATION);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for speed limits (--data=speed).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputSpeedLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=SPEED_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for weight limits (--data=weight).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputWeightLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=WEIGHT_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for height limits (--data=height).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputHeightLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=HEIGHT_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for width limits (--data=width).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputWidthLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=WIDTH_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for length limits (--data=length).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputLengthLimits(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 limit_type=LENGTH_LIMIT;

 find_all_nodes(nodes,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output those and connected segments that have a speed, weight, height, width or length limit change (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_limits(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segmentp,segmentps[MAX_SEG_PER_NODE];
 index_t segments[MAX_SEG_PER_NODE];
 int limits[MAX_SEG_PER_NODE];
 int count=0;
 int i,j,same=0;

 segmentp=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsNormalSegment(segmentp) && count<MAX_SEG_PER_NODE)
      {
       Way *wayp=LookupWay(OSMWays,segmentp->way,1);

       segmentps[count]=*segmentp;
       segments [count]=IndexSegment(OSMSegments,segmentp);

       switch(limit_type)
         {
         case SPEED_LIMIT:  limits[count]=wayp->speed;  break;
         case WEIGHT_LIMIT: limits[count]=wayp->weight; break;
         case HEIGHT_LIMIT: limits[count]=wayp->height; break;
         case WIDTH_LIMIT:  limits[count]=wayp->width;  break;
         case LENGTH_LIMIT: limits[count]=wayp->length; break;
         }

       if(limits[count] || HIGHWAY(wayp->type)<Highway_Track)
          count++;
      }

    segmentp=NextSegment(OSMSegments,segmentp,node);
   }
 while(segmentp);

 /* Nodes with only one limit are not interesting */

 if(count==1)
    return;

 /* Nodes with all segments the same limit are not interesting */

 same=0;
 for(j=0;j<count;j++)
    if(limits[0]==limits[j])
       same++;

 if(same==count)
    return;

 /* Display the interesting limits */

 printf("node%"Pindex_t" %.6f %.6f\n",node,radians_to_degrees(latitude),radians_to_degrees(longitude));

 for(i=0;i<count;i++)
   {
    same=0;
    for(j=0;j<count;j++)
       if(limits[i]==limits[j])
          same++;

    if(count==2 || same!=(count-1))
      {
       double lat,lon;

       GetLatLong(OSMNodes,OtherNode(&segmentps[i],node),NULL,&lat,&lon);

       switch(limit_type)
         {
         case SPEED_LIMIT:
          printf("segment%"Pindex_t" %.6f %.6f %d\n",segments[i],radians_to_degrees(lat),radians_to_degrees(lon),speed_to_kph(limits[i]));
          break;
         case WEIGHT_LIMIT:
          printf("segment%"Pindex_t" %.6f %.6f %.1f\n",segments[i],radians_to_degrees(lat),radians_to_degrees(lon),weight_to_tonnes(limits[i]));
          break;
         case HEIGHT_LIMIT:
          printf("segment%"Pindex_t" %.6f %.6f %.1f\n",segments[i],radians_to_degrees(lat),radians_to_degrees(lon),height_to_metres(limits[i]));
          break;
         case WIDTH_LIMIT:
          printf("segment%"Pindex_t" %.6f %.6f %.1f\n",segments[i],radians_to_degrees(lat),radians_to_degrees(lon),width_to_metres(limits[i]));
          break;
         case LENGTH_LIMIT:
          printf("segment%"Pindex_t" %.6f %.6f %.1f\n",segments[i],radians_to_degrees(lat),radians_to_degrees(lon),length_to_metres(limits[i]));
          break;
         }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for segments that have a particular property (--data=property-paved etc).

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  Relations *relations The set of relations to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.

  Property property The type of property.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputProperty(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,double latmin,double latmax,double lonmin,double lonmax,Property property)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;
 OSMRelations=relations;

 LatMin=latmin;
 LatMax=latmax;
 LonMin=lonmin;
 LonMax=lonmax;

 /* Iterate through the nodes and process them */

 properties=PROPERTIES(property);

 find_all_nodes(nodes,(callback_t)output_property);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node and output all connected segments with a particular property (called as a callback).

  index_t node The node to output.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_property(index_t node,double latitude,double longitude)
{
 Node *nodep=LookupNode(OSMNodes,node,1);
 Segment *segmentp;

 segmentp=FirstSegment(OSMSegments,nodep,1);

 do
   {
    if(IsNormalSegment(segmentp))
      {
       Way *wayp=LookupWay(OSMWays,segmentp->way,1);

       if(wayp->props&properties)
         {
          index_t othernode=OtherNode(segmentp,node);
          double lat,lon;

          GetLatLong(OSMNodes,othernode,NULL,&lat,&lon);

          if(node>othernode || (lat<LatMin || lat>LatMax || lon<LonMin || lon>LonMax))
             printf("segment%"Pindex_t" %.6f %.6f %.6f %.6f\n",IndexSegment(OSMSegments,segmentp),radians_to_degrees(latitude),radians_to_degrees(longitude),radians_to_degrees(lat),radians_to_degrees(lon));
         }
      }

    segmentp=NextSegment(OSMSegments,segmentp,node);
   }
 while(segmentp);
}


/*++++++++++++++++++++++++++++++++++++++
  A function to iterate through all nodes and call a callback function for each one.

  Nodes *nodes The set of nodes to use.

  callback_t callback The callback function for each node.
  ++++++++++++++++++++++++++++++++++++++*/

static void find_all_nodes(Nodes *nodes,callback_t callback)
{
 ll_bin_t latminbin=latlong_to_bin(radians_to_latlong(LatMin))-nodes->file.latzero;
 ll_bin_t latmaxbin=latlong_to_bin(radians_to_latlong(LatMax))-nodes->file.latzero;
 ll_bin_t lonminbin=latlong_to_bin(radians_to_latlong(LonMin))-nodes->file.lonzero;
 ll_bin_t lonmaxbin=latlong_to_bin(radians_to_latlong(LonMax))-nodes->file.lonzero;
 ll_bin_t latb,lonb;
 index_t i,index1,index2;

 /* Loop through all of the nodes. */

 if(latminbin<0)                   latminbin=0;
 if(latmaxbin>nodes->file.latbins) latmaxbin=nodes->file.latbins-1;

 if(lonminbin<0)                   lonminbin=0;
 if(lonmaxbin>nodes->file.lonbins) lonmaxbin=nodes->file.lonbins-1;

 for(latb=latminbin;latb<=latmaxbin;latb++)
    for(lonb=lonminbin;lonb<=lonmaxbin;lonb++)
      {
       ll_bin2_t llbin=lonb*nodes->file.latbins+latb;

       if(llbin<0 || llbin>(nodes->file.latbins*nodes->file.lonbins))
          continue;

       index1=LookupNodeOffset(nodes,llbin);
       index2=LookupNodeOffset(nodes,llbin+1);

       for(i=index1;i<index2;i++)
         {
          Node *nodep=LookupNode(nodes,i,1);

          double lat=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb)+off_to_latlong(nodep->latoffset));
          double lon=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb)+off_to_latlong(nodep->lonoffset));

          if(lat>LatMin && lat<LatMax && lon>LonMin && lon<LonMax)
             (*callback)(i,lat,lon);
         }
      }
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for error logs within the region (--data=errorlogs).

  ErrorLogs *errorlogs The set of error logs to use.

  double latmin The minimum latitude.

  double latmax The maximum latitude.

  double lonmin The minimum longitude.

  double lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputErrorLog(ErrorLogs *errorlogs,double latmin,double latmax,double lonmin,double lonmax)
{
 ll_bin_t latminbin=latlong_to_bin(radians_to_latlong(latmin))-errorlogs->file.latzero;
 ll_bin_t latmaxbin=latlong_to_bin(radians_to_latlong(latmax))-errorlogs->file.latzero;
 ll_bin_t lonminbin=latlong_to_bin(radians_to_latlong(lonmin))-errorlogs->file.lonzero;
 ll_bin_t lonmaxbin=latlong_to_bin(radians_to_latlong(lonmax))-errorlogs->file.lonzero;
 ll_bin_t latb,lonb;
 index_t i,index1,index2;

 /* Loop through all of the error logs. */

 for(latb=latminbin;latb<=latmaxbin;latb++)
    for(lonb=lonminbin;lonb<=lonmaxbin;lonb++)
      {
       ll_bin2_t llbin=lonb*errorlogs->file.latbins+latb;

       if(llbin<0 || llbin>(errorlogs->file.latbins*errorlogs->file.lonbins))
          continue;

       index1=LookupErrorLogOffset(errorlogs,llbin);
       index2=LookupErrorLogOffset(errorlogs,llbin+1);

       if(index2>errorlogs->file.number_geo)
          index2=errorlogs->file.number_geo;

       for(i=index1;i<index2;i++)
         {
          ErrorLog *errorlogp=LookupErrorLog(errorlogs,i,1);

          double lat=latlong_to_radians(bin_to_latlong(errorlogs->file.latzero+latb)+off_to_latlong(errorlogp->latoffset));
          double lon=latlong_to_radians(bin_to_latlong(errorlogs->file.lonzero+lonb)+off_to_latlong(errorlogp->lonoffset));

          if(lat>latmin && lat<latmax && lon>lonmin && lon<lonmax)
             printf("errorlog%"Pindex_t" %.6f %.6f\n",i,radians_to_degrees(lat),radians_to_degrees(lon));
         }
      }
}
