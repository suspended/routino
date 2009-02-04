/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.c,v 1.18 2009-02-04 18:26:29 amb Exp $

 Node data type functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <assert.h>
#include <stdlib.h>

#include "functions.h"
#include "nodes.h"


/* Constants */

/*+ The array size increment for nodes - expect ~8,000,000 nodes. +*/
#define INCREMENT_NODES 1024*1024

/*+ The latitude and longitude conversion factor from float to integer. +*/
#define LAT_LONG_SCALE  (1024*1024)

/*+ The latitude and longitude integer range within each bin. +*/
#define LAT_LONG_BIN    65536

/*+ The latitude and longitude number of bins per degree. +*/
#define LAT_LONG_DEGBIN (LAT_LONG_SCALE/LAT_LONG_BIN)


/* Functions */

static int sort_by_id(NodeX **a,NodeX **b);
static int sort_by_lat_long(NodeX *a,NodeX *b);


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
  ++++++++++++++++++++++++++++++++++++++*/

Node *FindNode(Nodes* nodes,float latitude,float longitude)
{
 int32_t latbin=(int32_t)((latitude-nodes->latzero)*LAT_LONG_DEGBIN);
 int32_t lonbin=(int32_t)((longitude-nodes->lonzero)*LAT_LONG_DEGBIN);
 int llbin=lonbin*nodes->latbins+latbin;
 int i,best;
 float distance=1000000;

 for(i=nodes->offsets[llbin];i<nodes->offsets[llbin+1];i++)
   {
    float lat=nodes->latzero+(double)latbin/(double)LAT_LONG_DEGBIN+(double)nodes->nodes[i].latoffset/(double)LAT_LONG_SCALE;
    float lon=nodes->lonzero+(double)lonbin/(double)LAT_LONG_DEGBIN+(double)nodes->nodes[i].lonoffset/(double)LAT_LONG_SCALE;

    float dist=Distance(lat,lon,latitude,longitude);

    if(dist<distance)
      {best=i; distance=dist;}
   }

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

 *latitude =nodes->latzero+(double)latbin/(double)LAT_LONG_DEGBIN+(double)node->latoffset/(double)LAT_LONG_SCALE;
 *longitude=nodes->lonzero+(double)lonbin/(double)LAT_LONG_DEGBIN+(double)node->lonoffset/(double)LAT_LONG_SCALE;
}


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new node list.

  NodesX *NewNodeList Returns the node list.
  ++++++++++++++++++++++++++++++++++++++*/

NodesX *NewNodeList(void)
{
 NodesX *nodesx;

 nodesx=(NodesX*)malloc(sizeof(NodesX));

 nodesx->alloced=INCREMENT_NODES;
 nodesx->number=0;
 nodesx->sorted=0;

 nodesx->gdata=(NodeX*)malloc(nodesx->alloced*sizeof(NodeX));
 nodesx->idata=NULL;

 return(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the node list to a file.

  NodesX* nodesx The set of nodes to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveNodeList(NodesX* nodesx,const char *filename)
{
 int i;
 int fd;
 Nodes *nodes=calloc(1,sizeof(Nodes));
 index_t *offsets;
 float lat_min,lat_max,lon_min,lon_max;
 int latbins,lonbins,latlonbin;

 assert(nodesx->sorted);      /* Must be sorted */

 /* Work out the offsets (careful with the rounding) */

 lat_min=-180.0+(float)((int32_t)((180.0+nodesx->lat_min)*LAT_LONG_DEGBIN))/LAT_LONG_DEGBIN;
 lon_min=-180.0+(float)((int32_t)((180.0+nodesx->lon_min)*LAT_LONG_DEGBIN))/LAT_LONG_DEGBIN;
 lat_max=-180.0+(float)((int32_t)((180.0+nodesx->lat_max)*LAT_LONG_DEGBIN+1))/LAT_LONG_DEGBIN;
 lon_max=-180.0+(float)((int32_t)((180.0+nodesx->lon_max)*LAT_LONG_DEGBIN+1))/LAT_LONG_DEGBIN;

 latbins=(lat_max-lat_min)*LAT_LONG_DEGBIN;
 lonbins=(lon_max-lon_min)*LAT_LONG_DEGBIN;

 offsets=malloc((latbins*lonbins+1)*sizeof(index_t));

 latlonbin=0;

 for(i=0;i<nodesx->number;i++)
   {
    int32_t latbin=(int32_t)((nodesx->gdata[i].latitude-lat_min)*LAT_LONG_DEGBIN);
    int32_t lonbin=(int32_t)((nodesx->gdata[i].longitude-lon_min)*LAT_LONG_DEGBIN);
    int llbin=lonbin*latbins+latbin;

    for(;latlonbin<=llbin;latlonbin++)
       offsets[latlonbin]=i;
   }

 for(;latlonbin<=(latbins*lonbins);latlonbin++)
    offsets[latlonbin]=nodesx->number;

 /* Fill in a Nodes structure with the offset of the real data in the file after
    the Node structure itself. */

 nodes->number=nodesx->number;

 nodes->latbins=latbins;
 nodes->lonbins=lonbins;

 nodes->latzero=lat_min;
 nodes->lonzero=lon_min;

 nodes->data=NULL;
 nodes->offsets=(void*)sizeof(Nodes);
 nodes->nodes=(void*)sizeof(Nodes)+(latbins*lonbins+1)*sizeof(index_t);

 /* Write out the Nodes structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,nodes,sizeof(Nodes));

 WriteFile(fd,offsets,(latbins*lonbins+1)*sizeof(index_t));

 for(i=0;i<nodesx->number;i++)
    WriteFile(fd,&nodesx->gdata[i].node,sizeof(Node));

 CloseFile(fd);

 /* Free the fake Nodes */

 free(nodes);
 free(offsets);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node.

  NodeX *FindNodeX Returns the extended node with the specified id.

  NodesX* nodesx The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

NodeX *FindNodeX(NodesX* nodesx,node_t id)
{
 int start=0;
 int end=nodesx->number-1;
 int mid;

 /* Binary search - search key exact match only is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 if(end<start)                        /* There are no nodes */
    return(NULL);
 else if(id<nodesx->idata[start]->id) /* Check key is not before start */
    return(NULL);
 else if(id>nodesx->idata[end]->id)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                 /* Choose mid point */

       if(nodesx->idata[mid]->id<id)      /* Mid point is too low */
          start=mid+1;
       else if(nodesx->idata[mid]->id>id) /* Mid point is too high */
          end=mid-1;
       else                               /* Mid point is correct */
          return(nodesx->idata[mid]);
      }
    while((end-start)>1);

    if(nodesx->idata[start]->id==id)      /* Start is correct */
       return(nodesx->idata[start]);

    if(nodesx->idata[end]->id==id)        /* End is correct */
       return(nodesx->idata[end]);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a node to a newly created node list (unsorted).

  Node *AppendNode Return a pointer to the new node.

  NodesX* nodesx The set of nodes to process.

  node_t id The node identification.

  float latitude The latitude of the node.

  float longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

Node *AppendNode(NodesX* nodesx,node_t id,float latitude,float longitude)
{
 /* Check that the array has enough space. */

 if(nodesx->number==nodesx->alloced)
   {
    nodesx->alloced+=INCREMENT_NODES;

    nodesx->gdata=(NodeX*)realloc((void*)nodesx->gdata,nodesx->alloced*sizeof(NodeX));
   }

 /* Insert the node */

 nodesx->gdata[nodesx->number].id=id;
 nodesx->gdata[nodesx->number].super=0;
 nodesx->gdata[nodesx->number].latitude=latitude;
 nodesx->gdata[nodesx->number].longitude=longitude;

 nodesx->number++;

 nodesx->sorted=0;

 return(&nodesx->gdata[nodesx->number-1].node);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesX* nodesx)
{
 int i;

 qsort(nodesx->gdata,nodesx->number,sizeof(NodeX),(int (*)(const void*,const void*))sort_by_lat_long);

 while(nodesx->gdata[nodesx->number-1].id==~0)
    nodesx->number--;

 nodesx->idata=malloc(nodesx->number*sizeof(NodeX*));

 nodesx->lat_min=90;
 nodesx->lat_max=-90;
 nodesx->lon_min=180;
 nodesx->lon_max=-180;

 for(i=0;i<nodesx->number;i++)
   {
    int32_t lat=(int32_t)(nodesx->gdata[i].latitude*LAT_LONG_SCALE);
    int32_t lon=(int32_t)(nodesx->gdata[i].longitude*LAT_LONG_SCALE);

    nodesx->gdata[i].node.latoffset=lat%LAT_LONG_BIN;
    nodesx->gdata[i].node.lonoffset=lon%LAT_LONG_BIN;

    if(nodesx->gdata[i].latitude<nodesx->lat_min)
       nodesx->lat_min=nodesx->gdata[i].latitude;
    if(nodesx->gdata[i].latitude>nodesx->lat_max)
       nodesx->lat_max=nodesx->gdata[i].latitude;
    if(nodesx->gdata[i].longitude<nodesx->lon_min)
       nodesx->lon_min=nodesx->gdata[i].longitude;
    if(nodesx->gdata[i].longitude>nodesx->lon_max)
       nodesx->lon_max=nodesx->gdata[i].longitude;

    nodesx->idata[i]=&nodesx->gdata[i];
   }

 qsort(nodesx->idata,nodesx->number,sizeof(NodeX*),(int (*)(const void*,const void*))sort_by_id);

 nodesx->sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into id order.

  int sort_by_id Returns the comparison of the id fields.

  NodeX **a The first Node.

  NodeX **b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(NodeX **a,NodeX **b)
{
 node_t a_id=(*a)->id;
 node_t b_id=(*b)->id;

 if(a_id<b_id)
    return(-1);
 else
    return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into latitude and longitude order.

  int sort_by_lat_long Returns the comparison of the latitude and longitude fields.

  NodeX *a The first Node.

  NodeX *b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_lat_long(NodeX *a,NodeX *b)
{
 node_t a_id=a->id;
 node_t b_id=b->id;
 int32_t a_lat=(int32_t)(a->latitude*LAT_LONG_DEGBIN);
 int32_t a_lon=(int32_t)(a->longitude*LAT_LONG_DEGBIN);
 int32_t b_lat=(int32_t)(b->latitude*LAT_LONG_DEGBIN);
 int32_t b_lon=(int32_t)(b->longitude*LAT_LONG_DEGBIN);

 if(a_id==~0)
    return(1);
 if(b_id==~0)
    return(-1);

 if(a_lon<b_lon)
    return(-1);
 else if(a_lon>b_lon)
    return(1);
 else
   {
    if(a_lat<b_lat)
       return(-1);
    else if(a_lat>b_lat)
       return(1);
    else
       return(0);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Remove any nodes that are not part of a highway.

  NodesX *nodesx The complete node list.

  SegmentsX *segmentsx The list of segments.
  ++++++++++++++++++++++++++++++++++++++*/

void RemoveNonHighwayNodes(NodesX *nodesx,SegmentsX *segmentsx)
{
 int i;
 int highway=0,nothighway=0;

 for(i=0;i<nodesx->number;i++)
   {
    if(FindFirstSegmentX(segmentsx,nodesx->gdata[i].id))
       highway++;
    else
      {
       nodesx->gdata[i].id=~0;
       nothighway++;
      }

    if(!((i+1)%10000))
      {
       printf("\rChecking: Nodes=%d Highway=%d not-Highway=%d",i+1,highway,nothighway);
       fflush(stdout);
      }
   }

 printf("\rChecked: Nodes=%d Highway=%d not-Highway=%d  \n",nodesx->number,highway,nothighway);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Fix the node indexes to the segments.

  NodesX* nodesx The set of nodes to process.

  SegmentsX *segmentsx The list of segments to use.

  int iteration The current super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

void FixupNodes(NodesX *nodesx,SegmentsX* segmentsx,int iteration)
{
 int i;

 assert(nodesx->sorted);      /* Must be sorted */

 for(i=0;i<nodesx->number;i++)
   {
    SegmentX *firstseg=FindFirstSegmentX(segmentsx,nodesx->gdata[i].id);

    nodesx->gdata[i].node.firstseg=IndexSegmentX(segmentsx,firstseg);

    if(nodesx->gdata[i].super==iteration)
       nodesx->gdata[i].node.firstseg|=SUPER_FLAG;

    if(!((i+1)%10000))
      {
       printf("\rFixing Nodes: Nodes=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rFixed Nodes: Nodes=%d \n",nodesx->number);
 fflush(stdout);
}
