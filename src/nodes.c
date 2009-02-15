/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.c,v 1.25 2009-02-15 16:19:28 amb Exp $

 Node data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdlib.h>
#include <math.h>

#include "nodes.h"
#include "segments.h"
#include "functions.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a node list from a file.

  Nodes* LoadNodeList Returns the node list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Nodes *LoadNodeList(const char *filename)
{
 void *data;
 Nodes *nodes;

 nodes=(Nodes*)malloc(sizeof(Nodes));

 data=MapFile(filename);

 if(!data)
    return(NULL);

 /* Copy the Nodes structure from the loaded data */

 *nodes=*((Nodes*)data);

 /* Adjust the pointers in the Nodes structure. */

 nodes->data=data;
 nodes->offsets=(index_t*)(data+(off_t)nodes->offsets);
 nodes->nodes=(Node*)(data+(off_t)nodes->nodes);

 return(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a node given its latitude and longitude.

  Node *FindNode Returns the node.

  Nodes* nodes The set of nodes to search.

  float latitude The latitude to look for.

  float longitude The longitude to look for.

  distance_t distance The maximum distance to look, returns the final distance.
  ++++++++++++++++++++++++++++++++++++++*/

Node *FindNode(Nodes* nodes,float latitude,float longitude,distance_t *distance)
{
 int32_t latbin=lat_long_to_bin(latitude )-nodes->latzero;
 int32_t lonbin=lat_long_to_bin(longitude)-nodes->lonzero;
 int     delta=0,count;
 index_t i,best=~0;

 /* Start with the bin containing the location, then spiral outwards. */

 do
   {
    int latb,lonb,llbin;

    count=0;
   
    for(latb=latbin-delta;latb<=latbin+delta;latb++)
       for(lonb=lonbin-delta;lonb<=lonbin+delta;lonb++)
         {
          if(abs(latb-latbin)<delta && abs(lonb-lonbin)<delta)
             continue;

          llbin=lonb*nodes->latbins+latb;

          if(llbin<0 || llbin>nodes->number)
             continue;

          if(delta>0)
            {
             float lat1=(float)((nodes->latzero+latb)*LAT_LONG_BIN)/LAT_LONG_SCALE;
             float lon1=(float)((nodes->lonzero+lonb)*LAT_LONG_BIN)/LAT_LONG_SCALE;
             float lat2=(float)((nodes->latzero+latb+1)*LAT_LONG_BIN)/LAT_LONG_SCALE;
             float lon2=(float)((nodes->lonzero+lonb+1)*LAT_LONG_BIN)/LAT_LONG_SCALE;

             if(latb==latbin)
               {
                distance_t dist1=Distance(latitude,lon1,latitude,longitude);
                distance_t dist2=Distance(latitude,lon2,latitude,longitude);

                if(dist1>*distance && dist2>*distance)
                   continue;
               }
             else if(lonb==lonbin)
               {
                distance_t dist1=Distance(lat1,longitude,latitude,longitude);
                distance_t dist2=Distance(lat2,longitude,latitude,longitude);

                if(dist1>*distance && dist2>*distance)
                   continue;
               }
             else
               {
                distance_t dist1=Distance(lat1,lon1,latitude,longitude);
                distance_t dist2=Distance(lat2,lon1,latitude,longitude);
                distance_t dist3=Distance(lat2,lon2,latitude,longitude);
                distance_t dist4=Distance(lat1,lon2,latitude,longitude);

                if(dist1>*distance && dist2>*distance && dist3>*distance && dist4>*distance)
                   continue;
               }
            }

          for(i=nodes->offsets[llbin];i<nodes->offsets[llbin+1];i++)
            {
             float lat=(float)((nodes->latzero+latbin)*LAT_LONG_BIN+nodes->nodes[i].latoffset)/LAT_LONG_SCALE;
             float lon=(float)((nodes->lonzero+lonbin)*LAT_LONG_BIN+nodes->nodes[i].lonoffset)/LAT_LONG_SCALE;

             distance_t dist=Distance(lat,lon,latitude,longitude);

             if(dist<*distance)
               {best=i; *distance=dist;}
            }

          count++;
         }

    delta++;
   }
 while(count);

 if(best==~0)
    return(NULL);
 else
    return(&nodes->nodes[best]);
}


/*++++++++++++++++++++++++++++++++++++++
  Get the latitude and longitude associated with a node.

  Nodes *nodes The set of nodes.

  Node *node The node.

  float *latitude Returns the latitude.

  float *longitude Returns the logitude.
  ++++++++++++++++++++++++++++++++++++++*/

void GetLatLong(Nodes *nodes,Node *node,float *latitude,float *longitude)
{
 index_t index=IndexNode(nodes,node);
 int latbin=-1,lonbin=-1;
 int start,end,mid;

 /* Binary search - search key closest below is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an inexact match is wanted we must set end=mid-1
  *  # <- mid    |  or start=mid because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 /* Search for longitude */

 start=0;
 end=nodes->lonbins-1;

 do
   {
    mid=(start+end)/2;                                 /* Choose mid point */

    if(nodes->offsets[nodes->latbins*mid]<index)       /* Mid point is too low */
       start=mid;
    else if(nodes->offsets[nodes->latbins*mid]>index)  /* Mid point is too high */
       end=mid-1;
    else                                               /* Mid point is correct */
      {lonbin=mid;break;}
   }
 while((end-start)>1);

 if(lonbin==-1)
   {
    if(nodes->offsets[nodes->latbins*end]>index)
       lonbin=start;
    else
       lonbin=end;
   }

 while(lonbin<nodes->lonbins && nodes->offsets[lonbin*nodes->latbins]==nodes->offsets[(lonbin+1)*nodes->latbins])
    lonbin++;

 /* Search for latitude */

 start=0;
 end=nodes->latbins-1;

 do
   {
    mid=(start+end)/2;                                       /* Choose mid point */

    if(nodes->offsets[lonbin*nodes->latbins+mid]<index)      /* Mid point is too low */
       start=mid;
    else if(nodes->offsets[lonbin*nodes->latbins+mid]>index) /* Mid point is too high */
       end=mid-1;
    else                                                     /* Mid point is correct */
      {latbin=mid;break;}
   }
 while((end-start)>1);

 if(latbin==-1)
   {
    if(nodes->offsets[lonbin*nodes->latbins+end]>index)
       latbin=start;
    else
       latbin=end;
   }

 while(latbin<nodes->latbins && nodes->offsets[lonbin*nodes->latbins+latbin]==nodes->offsets[lonbin*nodes->latbins+latbin+1])
    latbin++;

 /* Return the values */

 *latitude =(float)((nodes->latzero+latbin)*LAT_LONG_BIN+node->latoffset)/LAT_LONG_SCALE;
 *longitude=(float)((nodes->lonzero+lonbin)*LAT_LONG_BIN+node->lonoffset)/LAT_LONG_SCALE;
}
