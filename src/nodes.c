/***************************************
 Node data type functions.

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


#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"

#include "files.h"
#include "profiles.h"


/* Local functions */

static int valid_segment_for_profile(Ways *ways,Segment *segmentp,Profile *profile);


/*++++++++++++++++++++++++++++++++++++++
  Load in a node list from a file.

  Nodes *LoadNodeList Returns the node list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Nodes *LoadNodeList(const char *filename)
{
 Nodes *nodes;
#if SLIM
 size_t sizeoffsets;
#endif

 nodes=(Nodes*)malloc(sizeof(Nodes));

#if !SLIM

 nodes->data=MapFile(filename);

 /* Copy the NodesFile header structure from the loaded data */

 nodes->file=*((NodesFile*)nodes->data);

 /* Set the pointers in the Nodes structure. */

 nodes->offsets=(index_t*)(nodes->data+sizeof(NodesFile));
 nodes->nodes  =(Node*   )(nodes->data+sizeof(NodesFile)+(nodes->file.latbins*nodes->file.lonbins+1)*sizeof(index_t));

#else

 nodes->fd=SlimMapFile(filename);

 /* Copy the NodesFile header structure from the loaded data */

 SlimFetch(nodes->fd,&nodes->file,sizeof(NodesFile),0);

 sizeoffsets=(nodes->file.latbins*nodes->file.lonbins+1)*sizeof(index_t);

 nodes->offsets=(index_t*)malloc(sizeoffsets);
#ifndef LIBROUTINO
 log_malloc(nodes->offsets,sizeoffsets);
#endif

 SlimFetch(nodes->fd,nodes->offsets,sizeoffsets,sizeof(NodesFile));

 nodes->nodesoffset=(offset_t)(sizeof(NodesFile)+sizeoffsets);

 nodes->cache=NewNodeCache();
#ifndef LIBROUTINO
 log_malloc(nodes->cache,sizeof(*nodes->cache));
#endif

#endif

 return(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy the node list.

  Nodes *nodes The node list to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

void DestroyNodeList(Nodes *nodes)
{
#if !SLIM

 nodes->data=UnmapFile(nodes->data);

#else

 nodes->fd=SlimUnmapFile(nodes->fd);

#ifndef LIBROUTINO
 log_free(nodes->offsets);
#endif
 free(nodes->offsets);

#ifndef LIBROUTINO
 log_free(nodes->cache);
#endif
 DeleteNodeCache(nodes->cache);

#endif

 free(nodes);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the closest node given its latitude, longitude and the profile of the
  mode of transport that must be able to move to/from this node.

  index_t FindClosestNode Returns the closest node.

  Nodes *nodes The set of nodes to search.

  Segments *segments The set of segments to use.

  Ways *ways The set of ways to use.

  double latitude The latitude to look for.

  double longitude The longitude to look for.

  distance_t distance The maximum distance to look from the specified coordinates.

  Profile *profile The profile of the mode of transport.

  distance_t *bestdist Returns the distance to the best node.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindClosestNode(Nodes *nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                        distance_t distance,Profile *profile,distance_t *bestdist)
{
 ll_bin_t   latbin=latlong_to_bin(radians_to_latlong(latitude ))-nodes->file.latzero;
 ll_bin_t   lonbin=latlong_to_bin(radians_to_latlong(longitude))-nodes->file.lonzero;
 int        delta=0,count;
 index_t    i,index1,index2;
 index_t    bestn=NO_NODE;
 distance_t bestd=INF_DISTANCE;

 /* Find the maximum distance to search */

 double dlat=DeltaLat(longitude,distance);
 double dlon=DeltaLon(latitude ,distance);

 double minlat=latitude -dlat;
 double maxlat=latitude +dlat;
 double minlon=longitude-dlon;
 double maxlon=longitude+dlon;

 ll_bin_t minlatbin=latlong_to_bin(radians_to_latlong(minlat))-nodes->file.latzero;
 ll_bin_t maxlatbin=latlong_to_bin(radians_to_latlong(maxlat))-nodes->file.latzero;
 ll_bin_t minlonbin=latlong_to_bin(radians_to_latlong(minlon))-nodes->file.lonzero;
 ll_bin_t maxlonbin=latlong_to_bin(radians_to_latlong(maxlon))-nodes->file.lonzero;

 ll_off_t minlatoff=latlong_to_off(radians_to_latlong(minlat));
 ll_off_t maxlatoff=latlong_to_off(radians_to_latlong(maxlat));
 ll_off_t minlonoff=latlong_to_off(radians_to_latlong(minlon));
 ll_off_t maxlonoff=latlong_to_off(radians_to_latlong(maxlon));

 /* Start with the bin containing the location, then spiral outwards. */

 do
   {
    ll_bin_t latb,lonb;
    ll_bin2_t llbin;

    count=0;

    for(latb=latbin-delta;latb<=latbin+delta;latb++)
      {
       if(latb<0 || latb>=nodes->file.latbins || latb<minlatbin || latb>maxlatbin)
          continue;

       for(lonb=lonbin-delta;lonb<=lonbin+delta;lonb++)
         {
          if(lonb<0 || lonb>=nodes->file.lonbins || lonb<minlonbin || lonb>maxlonbin)
             continue;

          if(abs(latb-latbin)<delta && abs(lonb-lonbin)<delta)
             continue;

          /* Check every node in this grid square. */

          llbin=lonb*nodes->file.latbins+latb;

          index1=LookupNodeOffset(nodes,llbin);
          index2=LookupNodeOffset(nodes,llbin+1);

          for(i=index1;i<index2;i++)
            {
             Node *nodep=LookupNode(nodes,i,3);
             double lat,lon;
             distance_t dist;

             if(latb==minlatbin && nodep->latoffset<minlatoff)
                continue;

             if(latb==maxlatbin && nodep->latoffset>maxlatoff)
                continue;

             if(lonb==minlonbin && nodep->lonoffset<minlonoff)
                continue;

             if(lonb==maxlonbin && nodep->lonoffset>maxlonoff)
                continue;

             lat=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb)+off_to_latlong(nodep->latoffset));
             lon=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb)+off_to_latlong(nodep->lonoffset));

             dist=Distance(lat,lon,latitude,longitude);

             if(dist<bestd)
               {
                Segment *segmentp;

                /* Check that at least one segment is valid for the profile */

                segmentp=FirstSegment(segments,nodep,1);

                do
                  {
                   if(IsNormalSegment(segmentp) && valid_segment_for_profile(ways,segmentp,profile))
                     {
                      bestn=i;
                      bestd=dist;

                      break;
                     }

                   segmentp=NextSegment(segments,segmentp,i);
                  }
                while(segmentp);
               }
            }

          count++;
         }
      }

    delta++;
   }
 while(count);

 *bestdist=bestd;

 return(bestn);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the closest point on the closest segment given its latitude, longitude
  and the profile of the mode of transport that must be able to move along this
  segment.

  index_t FindClosestSegment Returns the closest segment index.

  Nodes *nodes The set of nodes to use.

  Segments *segments The set of segments to search.

  Ways *ways The set of ways to use.

  double latitude The latitude to look for.

  double longitude The longitude to look for.

  distance_t distance The maximum distance to look from the specified coordinates.

  Profile *profile The profile of the mode of transport.

  distance_t *bestdist Returns the distance to the closest point on the best segment.

  index_t *bestnode1 Returns the index of the node at one end of the closest segment.

  index_t *bestnode2 Returns the index of the node at the other end of the closest segment.

  distance_t *bestdist1 Returns the distance along the segment to the node at one end.

  distance_t *bestdist2 Returns the distance along the segment to the node at the other end.
  ++++++++++++++++++++++++++++++++++++++*/

index_t FindClosestSegment(Nodes *nodes,Segments *segments,Ways *ways,double latitude,double longitude,
                           distance_t distance,Profile *profile, distance_t *bestdist,
                           index_t *bestnode1,index_t *bestnode2,distance_t *bestdist1,distance_t *bestdist2)
{
 ll_bin_t   latbin=latlong_to_bin(radians_to_latlong(latitude ))-nodes->file.latzero;
 ll_bin_t   lonbin=latlong_to_bin(radians_to_latlong(longitude))-nodes->file.lonzero;
 int        delta=0,count;
 index_t    i,index1,index2;
 index_t    bestn1=NO_NODE,bestn2=NO_NODE;
 distance_t bestd=INF_DISTANCE,bestd1=INF_DISTANCE,bestd2=INF_DISTANCE;
 index_t    bests=NO_SEGMENT;

 /* Find the maximum distance to search */

 double dlat=DeltaLat(longitude,distance);
 double dlon=DeltaLon(latitude ,distance);

 double minlat=latitude -dlat;
 double maxlat=latitude +dlat;
 double minlon=longitude-dlon;
 double maxlon=longitude+dlon;

 ll_bin_t minlatbin=latlong_to_bin(radians_to_latlong(minlat))-nodes->file.latzero;
 ll_bin_t maxlatbin=latlong_to_bin(radians_to_latlong(maxlat))-nodes->file.latzero;
 ll_bin_t minlonbin=latlong_to_bin(radians_to_latlong(minlon))-nodes->file.lonzero;
 ll_bin_t maxlonbin=latlong_to_bin(radians_to_latlong(maxlon))-nodes->file.lonzero;

 ll_off_t minlatoff=latlong_to_off(radians_to_latlong(minlat));
 ll_off_t maxlatoff=latlong_to_off(radians_to_latlong(maxlat));
 ll_off_t minlonoff=latlong_to_off(radians_to_latlong(minlon));
 ll_off_t maxlonoff=latlong_to_off(radians_to_latlong(maxlon));

 /* Start with the bin containing the location, then spiral outwards. */

 do
   {
    ll_bin_t latb,lonb;
    ll_bin2_t llbin;

    count=0;

    for(latb=latbin-delta;latb<=latbin+delta;latb++)
      {
       if(latb<0 || latb>=nodes->file.latbins || latb<minlatbin || latb>maxlatbin)
          continue;

       for(lonb=lonbin-delta;lonb<=lonbin+delta;lonb++)
         {
          if(lonb<0 || lonb>=nodes->file.lonbins || lonb<minlonbin || lonb>maxlonbin)
             continue;

          if(abs(latb-latbin)<delta && abs(lonb-lonbin)<delta)
             continue;

          /* Check every node in this grid square. */

          llbin=lonb*nodes->file.latbins+latb;

          index1=LookupNodeOffset(nodes,llbin);
          index2=LookupNodeOffset(nodes,llbin+1);

          for(i=index1;i<index2;i++)
            {
             Node *nodep=LookupNode(nodes,i,3);
             double lat1,lon1;
             distance_t dist1;

             if(latb==minlatbin && nodep->latoffset<minlatoff)
                continue;

             if(latb==maxlatbin && nodep->latoffset>maxlatoff)
                continue;

             if(lonb==minlonbin && nodep->lonoffset<minlonoff)
                continue;

             if(lonb==maxlonbin && nodep->lonoffset>maxlonoff)
                continue;

             lat1=latlong_to_radians(bin_to_latlong(nodes->file.latzero+latb)+off_to_latlong(nodep->latoffset));
             lon1=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonb)+off_to_latlong(nodep->lonoffset));

             dist1=Distance(lat1,lon1,latitude,longitude);

             if(dist1<distance)
               {
                Segment *segmentp;

                /* Check each segment for closeness and if valid for the profile */

                segmentp=FirstSegment(segments,nodep,1);

                do
                  {
                   if(IsNormalSegment(segmentp) && valid_segment_for_profile(ways,segmentp,profile))
                     {
                      distance_t dist2,dist3;
                      double lat2,lon2,dist3a,dist3b,distp;

                      GetLatLong(nodes,OtherNode(segmentp,i),NULL,&lat2,&lon2);

                      dist2=Distance(lat2,lon2,latitude,longitude);

                      dist3=Distance(lat1,lon1,lat2,lon2);

                      /* Use law of cosines (assume flat Earth) */

                      if(dist3==0)
                        {
                         distp=dist1;  /* == dist2 */
                         dist3a=dist1; /* == dist2 */
                         dist3b=dist2; /* == dist1 */
                        }
                      else if((dist1+dist2)<dist3)
                        {
                         distp=0;
                         dist3a=dist1;
                         dist3b=dist2;
                        }
                      else
                        {
                         dist3a=((double)dist1*(double)dist1-(double)dist2*(double)dist2+(double)dist3*(double)dist3)/(2.0*(double)dist3);
                         dist3b=(double)dist3-dist3a;

                         if(dist3a>=0 && dist3b>=0)
                            distp=sqrt((double)dist1*(double)dist1-dist3a*dist3a);
                         else if(dist3a>0)
                           {
                            distp=dist2;
                            dist3a=dist3;
                            dist3b=0;
                           }
                         else /* if(dist3b>0) */
                           {
                            distp=dist1;
                            dist3a=0;
                            dist3b=dist3;
                           }
                        }

                      if(distp<(double)bestd)
                        {
                         bests=IndexSegment(segments,segmentp);

                         if(segmentp->node1==i)
                           {
                            bestn1=i;
                            bestn2=OtherNode(segmentp,i);
                            bestd1=(distance_t)dist3a;
                            bestd2=(distance_t)dist3b;
                           }
                         else
                           {
                            bestn1=OtherNode(segmentp,i);
                            bestn2=i;
                            bestd1=(distance_t)dist3b;
                            bestd2=(distance_t)dist3a;
                           }

                         bestd=(distance_t)distp;
                        }
                     }

                   segmentp=NextSegment(segments,segmentp,i);
                  }
                while(segmentp);

               } /* dist1 < distance */
            }

          count++;
         }
      }

    delta++;
   }
 while(count);

 *bestdist=bestd;

 *bestnode1=bestn1;
 *bestnode2=bestn2;
 *bestdist1=bestd1;
 *bestdist2=bestd2;

 return(bests);
}


/*++++++++++++++++++++++++++++++++++++++
  Check if the transport defined by the profile is allowed on the segment.

  int valid_segment_for_profile Return 1 if it is or 0 if not.

  Ways *ways The set of ways to use.

  Segment *segmentp The segment to check.

  Profile *profile The profile to check.
  ++++++++++++++++++++++++++++++++++++++*/

static int valid_segment_for_profile(Ways *ways,Segment *segmentp,Profile *profile)
{
 Way *wayp=LookupWay(ways,segmentp->way,1);
 score_t segment_pref;
 int i;

 /* mode of transport must be allowed on the highway */
 if(!(wayp->allow&profile->allow))
    return(0);

 /* must obey weight restriction (if exists) */
 if(wayp->weight && wayp->weight<profile->weight)
    return(0);

 /* must obey height/width/length restriction (if exists) */
 if((wayp->height && wayp->height<profile->height) ||
    (wayp->width  && wayp->width <profile->width ) ||
    (wayp->length && wayp->length<profile->length))
    return(0);

 segment_pref=profile->highway[HIGHWAY(wayp->type)];

 for(i=1;i<Property_Count;i++)
    if(ways->file.props & PROPERTIES(i))
      {
       if(wayp->props & PROPERTIES(i))
          segment_pref*=profile->props_yes[i];
       else
          segment_pref*=profile->props_no[i];
      }

 /* profile preferences must allow this highway */
 if(segment_pref==0)
    return(0);

 /* Must be OK */
 return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Get the latitude and longitude associated with a node.

  Nodes *nodes The set of nodes to use.

  index_t index The node index.

  Node *nodep A pointer to the node if already available.

  double *latitude Returns the latitude.

  double *longitude Returns the logitude.
  ++++++++++++++++++++++++++++++++++++++*/

void GetLatLong(Nodes *nodes,index_t index,Node *nodep,double *latitude,double *longitude)
{
 ll_bin_t latbin,lonbin;
 ll_bin2_t bin=-1;
 ll_bin2_t start,end,mid;
 index_t offset;

 /* Binary search - search key nearest match below is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  A lower bound match is wanted we can set end=mid-1 or
  *  # <- mid    |  start=mid because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 /* Search for offset */

 start=0;
 end=nodes->file.lonbins*nodes->file.latbins;

 do
   {
    mid=(start+end)/2;                  /* Choose mid point */

    offset=LookupNodeOffset(nodes,mid);

    if(offset<index)                    /* Mid point is too low for an exact match but could be lower bound */
       start=mid;
    else if(offset>index)               /* Mid point is too high */
       end=mid?(mid-1):mid;
    else                                /* Mid point is correct */
      {bin=mid;break;}
   }
 while((end-start)>1);

 if(bin==-1)
   {
    offset=LookupNodeOffset(nodes,end);

    if(offset>index)
       bin=start;
    else
       bin=end;
   }

 while(bin<=(nodes->file.lonbins*nodes->file.latbins) && 
       LookupNodeOffset(nodes,bin)==LookupNodeOffset(nodes,bin+1))
    bin++;

 latbin=bin%nodes->file.latbins;
 lonbin=bin/nodes->file.latbins;

 /* Return the values */

 if(nodep==NULL)
    nodep=LookupNode(nodes,index,4);

 *latitude =latlong_to_radians(bin_to_latlong(nodes->file.latzero+latbin)+off_to_latlong(nodep->latoffset));
 *longitude=latlong_to_radians(bin_to_latlong(nodes->file.lonzero+lonbin)+off_to_latlong(nodep->lonoffset));
}
