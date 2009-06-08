/***************************************
 $Header: /home/amb/CVS/routino/src/visualiser.c,v 1.2 2009-06-08 18:21:10 amb Exp $

 Extract data from Routino.

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
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "visualiser.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"


#define SPEED_LIMIT  1
#define WEIGHT_LIMIT 2
#define HEIGHT_LIMIT 3
#define WIDTH_LIMIT  4
#define LENGTH_LIMIT 5

/* Local types */

typedef void (*callback_t)(index_t node,float latitude,float longitude);

/* Local variables */

static Nodes    *OSMNodes;
static Segments *OSMSegments;
static Ways     *OSMWays;

static int limit_type=0;

/* Local functions */

static void find_all_nodes(Nodes *nodes,float latmin,float latmax,float lonmin,float lonmax,callback_t callback);
static void output_junctions(index_t node,float latitude,float longitude);
static void output_super(index_t node,float latitude,float longitude);
static void output_oneway(index_t node,float latitude,float longitude);
static void output_limits(index_t node,float latitude,float longitude);


/*++++++++++++++++++++++++++++++++++++++
  Output the data for junctions.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputJunctions(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,latmin,latmax,lonmin,lonmax,(callback_t)output_junctions);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node (called as a callback).

  index_t node The node to output.

  float latitude The latitude of the node.

  float longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_junctions(index_t node,float latitude,float longitude)
{
 Segment *segment;
 Way *firstway;
 int count=0,difference=0;

 segment=FirstSegment(OSMSegments,OSMNodes,node);
 firstway=LookupWay(OSMWays,segment->way);

 do
   {
    Way *way=LookupWay(OSMWays,segment->way);

    if(IsNormalSegment(segment))
       count++;

    if(!WaysSame(firstway,way))
       difference=1;

    segment=NextSegment(OSMSegments,segment,node);
   }
 while(segment);

 if(count!=2 || difference)
    printf("%.6f %.6f %d\n",(180.0/M_PI)*latitude,(180.0/M_PI)*longitude,count);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for super-nodes and super-segments.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputSuper(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,latmin,latmax,lonmin,lonmax,(callback_t)output_super);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node (called as a callback).

  index_t node The node to output.

  float latitude The latitude of the node.

  float longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_super(index_t node,float latitude,float longitude)
{
 Segment *segment;

 if(!IsSuperNode(OSMNodes,node))
    return;

 printf("%.6f %.6f n\n",(180.0/M_PI)*latitude,(180.0/M_PI)*longitude);

 segment=FirstSegment(OSMSegments,OSMNodes,node);

 do
   {
    if(IsSuperSegment(segment))
      {
       index_t othernode=OtherNode(segment,node);

       if(node>othernode)
         {
          float lat,lon;

          GetLatLong(OSMNodes,othernode,&lat,&lon);

          printf("%.6f %.6f s\n",(180.0/M_PI)*lat,(180.0/M_PI)*lon);
         }
      }

    segment=NextSegment(OSMSegments,segment,node);
   }
 while(segment);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for one-way segments.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputOneway(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;

 /* Iterate through the nodes and process them */

 find_all_nodes(nodes,latmin,latmax,lonmin,lonmax,(callback_t)output_oneway);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node (called as a callback).

  index_t node The node to output.

  float latitude The latitude of the node.

  float longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_oneway(index_t node,float latitude,float longitude)
{
 Segment *segment;

 segment=FirstSegment(OSMSegments,OSMNodes,node);

 do
   {
    if(IsNormalSegment(segment))
      {
       index_t othernode=OtherNode(segment,node);

       if(node>othernode)
         {
          float lat,lon;

          GetLatLong(OSMNodes,othernode,&lat,&lon);

          if(IsOnewayFrom(segment,node))
             printf("%.6f %.6f %.6f %.6f\n",(180.0/M_PI)*latitude,(180.0/M_PI)*longitude,(180.0/M_PI)*lat,(180.0/M_PI)*lon);
          else if(IsOnewayFrom(segment,othernode))
             printf("%.6f %.6f %.6f %.6f\n",(180.0/M_PI)*lat,(180.0/M_PI)*lon,(180.0/M_PI)*latitude,(180.0/M_PI)*longitude);
         }
      }

    segment=NextSegment(OSMSegments,segment,node);
   }
 while(segment);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for speed limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputSpeedLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;

 /* Iterate through the nodes and process them */

 limit_type=SPEED_LIMIT;

 find_all_nodes(nodes,latmin,latmax,lonmin,lonmax,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for weight limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputWeightLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;

 /* Iterate through the nodes and process them */

 limit_type=WEIGHT_LIMIT;

 find_all_nodes(nodes,latmin,latmax,lonmin,lonmax,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for height limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputHeightLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;

 /* Iterate through the nodes and process them */

 limit_type=HEIGHT_LIMIT;

 find_all_nodes(nodes,latmin,latmax,lonmin,lonmax,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for width limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputWidthLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;

 /* Iterate through the nodes and process them */

 limit_type=WIDTH_LIMIT;

 find_all_nodes(nodes,latmin,latmax,lonmin,lonmax,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Output the data for length limits.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.
  ++++++++++++++++++++++++++++++++++++++*/

void OutputLengthLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax)
{
 /* Use local variables so that the callback doesn't need to pass them backwards and forwards */

 OSMNodes=nodes;
 OSMSegments=segments;
 OSMWays=ways;

 /* Iterate through the nodes and process them */

 limit_type=LENGTH_LIMIT;

 find_all_nodes(nodes,latmin,latmax,lonmin,lonmax,(callback_t)output_limits);
}


/*++++++++++++++++++++++++++++++++++++++
  Process a single node (called as a callback).

  index_t node The node to output.

  float latitude The latitude of the node.

  float longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void output_limits(index_t node,float latitude,float longitude)
{
 Segment *segment,*segments[16];
 Way *ways[16];
 int limits[16];
 int count=0;
 int i,j,same=0;

 segment=FirstSegment(OSMSegments,OSMNodes,node);

 do
   {
    if(IsNormalSegment(segment) && count<16)
      {
       ways    [count]=LookupWay(OSMWays,segment->way);
       segments[count]=segment;

       switch(limit_type)
         {
         case SPEED_LIMIT:  limits[count]=ways[count]->speed;  break;
         case WEIGHT_LIMIT: limits[count]=ways[count]->weight; break;
         case HEIGHT_LIMIT: limits[count]=ways[count]->height; break;
         case WIDTH_LIMIT:  limits[count]=ways[count]->width;  break;
         case LENGTH_LIMIT: limits[count]=ways[count]->length; break;
         }

       count++;
      }

    segment=NextSegment(OSMSegments,segment,node);
   }
 while(segment);

 /* Nodes with only one limit are not interesting */

 if(count==1)
    return;

 /* Nodes with all segments the same limit is not interesting */

 same=0;
 for(j=0;j<count;j++)
    if(limits[0]==limits[j])
       same++;

 if(same==count)
    return;

 /* Display the interesting speed limits */

 printf("%.6f %.6f\n",(180.0/M_PI)*latitude,(180.0/M_PI)*longitude);

 for(i=0;i<count;i++)
   {
    same=0;
    for(j=0;j<count;j++)
       if(limits[i]==limits[j])
          same++;

    if(count==2 || same!=(count-1))
      {
       float lat,lon;

       GetLatLong(OSMNodes,OtherNode(segments[i],node),&lat,&lon);

       switch(limit_type)
         {
         case SPEED_LIMIT:
          printf("%.6f %.6f %.0f\n",(180.0/M_PI)*lat,(180.0/M_PI)*lon,(float)speed_to_kph(limits[i]));
          break;
         case WEIGHT_LIMIT:
          printf("%.6f %.6f %.1f\n",(180.0/M_PI)*lat,(180.0/M_PI)*lon,weight_to_tonnes(limits[i]));
          break;
         case HEIGHT_LIMIT:
          printf("%.6f %.6f %.1f\n",(180.0/M_PI)*lat,(180.0/M_PI)*lon,height_to_metres(limits[i]));
          break;
         case WIDTH_LIMIT:
          printf("%.6f %.6f %.1f\n",(180.0/M_PI)*lat,(180.0/M_PI)*lon,width_to_metres(limits[i]));
          break;
         case LENGTH_LIMIT:
          printf("%.6f %.6f %.1f\n",(180.0/M_PI)*lat,(180.0/M_PI)*lon,length_to_metres(limits[i]));
          break;
         }
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  A function to iterate through all nodes and call a callback function for each one.

  Nodes *nodes The list of nodes to process.

  float latmin The minimum latitude.

  float latmax The maximum latitude.

  float lonmin The minimum longitude.

  float lonmax The maximum longitude.

  callback_t callback The callback function for each node.
  ++++++++++++++++++++++++++++++++++++++*/

static void find_all_nodes(Nodes *nodes,float latmin,float latmax,float lonmin,float lonmax,callback_t callback)
{
 int32_t latminbin=lat_long_to_bin(latmin)-nodes->latzero;
 int32_t latmaxbin=lat_long_to_bin(latmax)-nodes->latzero;
 int32_t lonminbin=lat_long_to_bin(lonmin)-nodes->lonzero;
 int32_t lonmaxbin=lat_long_to_bin(lonmax)-nodes->lonzero;
 int latb,lonb,llbin;
 index_t node;

 /* Loop through all of the nodes. */

 for(latb=latminbin;latb<=latmaxbin;latb++)
    for(lonb=lonminbin;lonb<=lonmaxbin;lonb++)
      {
       llbin=lonb*nodes->latbins+latb;

       if(llbin<0 || llbin>(nodes->latbins*nodes->lonbins))
          continue;

       for(node=nodes->offsets[llbin];node<nodes->offsets[llbin+1];node++)
         {
          float lat=(float)((nodes->latzero+latb)*LAT_LONG_BIN+nodes->nodes[node].latoffset)/LAT_LONG_SCALE;
          float lon=(float)((nodes->lonzero+lonb)*LAT_LONG_BIN+nodes->nodes[node].lonoffset)/LAT_LONG_SCALE;

          if(lat>latmin && lat<latmax && lon>lonmin && lon<lonmax)
             (*callback)(node,lat,lon);
         }
      }
}
