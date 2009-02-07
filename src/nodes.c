/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.c,v 1.20 2009-02-07 11:50:48 amb Exp $

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
#include <string.h>

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
static int sort_by_lat_long(NodeX **a,NodeX **b);


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
 int i,best=~0;
 float distance=1000000;

 for(i=nodes->offsets[llbin];i<nodes->offsets[llbin+1];i++)
   {
    float lat=nodes->latzero+(double)latbin/(double)LAT_LONG_DEGBIN+(double)nodes->nodes[i].latoffset/(double)LAT_LONG_SCALE;
    float lon=nodes->lonzero+(double)lonbin/(double)LAT_LONG_DEGBIN+(double)nodes->nodes[i].lonoffset/(double)LAT_LONG_SCALE;

    float dist=Distance(lat,lon,latitude,longitude);

    if(dist<distance)
      {best=i; distance=dist;}
   }

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
 nodesx->xnumber=0;
 nodesx->sorted=0;

 nodesx->xdata=(NodeX*)malloc(nodesx->alloced*sizeof(NodeX));
 nodesx->gdata=NULL;
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

 assert(nodesx->sorted);        /* Must be sorted */

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
    int32_t latbin=(int32_t)((nodesx->gdata[i]->latitude-lat_min)*LAT_LONG_DEGBIN);
    int32_t lonbin=(int32_t)((nodesx->gdata[i]->longitude-lon_min)*LAT_LONG_DEGBIN);
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
    WriteFile(fd,&nodesx->gdata[i]->node,sizeof(Node));

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

 assert(nodesx->sorted);        /* Must be sorted */

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

 if(nodesx->xnumber==nodesx->alloced)
   {
    nodesx->alloced+=INCREMENT_NODES;

    nodesx->xdata=(NodeX*)realloc((void*)nodesx->xdata,nodesx->alloced*sizeof(NodeX));
   }

 /* Insert the node */

 nodesx->xdata[nodesx->xnumber].id=id;
 nodesx->xdata[nodesx->xnumber].super=0;
 nodesx->xdata[nodesx->xnumber].latitude=latitude;
 nodesx->xdata[nodesx->xnumber].longitude=longitude;

 memset(&nodesx->xdata[nodesx->xnumber].node,0,sizeof(Node));

 nodesx->xnumber++;

 nodesx->sorted=0;

 return(&nodesx->xdata[nodesx->xnumber-1].node);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesX* nodesx)
{
 int i;

 /* Allocate the arrays of pointers */

 if(nodesx->sorted)
   {
    nodesx->gdata=realloc(nodesx->gdata,nodesx->xnumber*sizeof(NodeX*));
    nodesx->idata=realloc(nodesx->idata,nodesx->xnumber*sizeof(NodeX*));
   }
 else
   {
    nodesx->gdata=malloc(nodesx->xnumber*sizeof(NodeX*));
    nodesx->idata=malloc(nodesx->xnumber*sizeof(NodeX*));
   }

 nodesx->number=0;

 for(i=0;i<nodesx->xnumber;i++)
    if(nodesx->xdata[i].id!=~0)
      {
       nodesx->gdata[nodesx->number]=&nodesx->xdata[i];
       nodesx->idata[nodesx->number]=&nodesx->xdata[i];
       nodesx->number++;
      }

 /* Sort geographically */

 qsort(nodesx->gdata,nodesx->number,sizeof(NodeX*),(int (*)(const void*,const void*))sort_by_lat_long);

 nodesx->lat_min=90;
 nodesx->lat_max=-90;
 nodesx->lon_min=180;
 nodesx->lon_max=-180;

 for(i=0;i<nodesx->number;i++)
   {
    int32_t lat=(int32_t)(nodesx->gdata[i]->latitude*LAT_LONG_SCALE);
    int32_t lon=(int32_t)(nodesx->gdata[i]->longitude*LAT_LONG_SCALE);

    nodesx->gdata[i]->node.latoffset=lat%LAT_LONG_BIN;
    nodesx->gdata[i]->node.lonoffset=lon%LAT_LONG_BIN;

    if(nodesx->gdata[i]->latitude<nodesx->lat_min)
       nodesx->lat_min=nodesx->gdata[i]->latitude;
    if(nodesx->gdata[i]->latitude>nodesx->lat_max)
       nodesx->lat_max=nodesx->gdata[i]->latitude;
    if(nodesx->gdata[i]->longitude<nodesx->lon_min)
       nodesx->lon_min=nodesx->gdata[i]->longitude;
    if(nodesx->gdata[i]->longitude>nodesx->lon_max)
       nodesx->lon_max=nodesx->gdata[i]->longitude;
   }

 /* Sort by id */

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

  NodeX **a The first Node.

  NodeX **b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_lat_long(NodeX **a,NodeX **b)
{
 int32_t a_lon=(int32_t)((*a)->longitude*LAT_LONG_DEGBIN);
 int32_t b_lon=(int32_t)((*b)->longitude*LAT_LONG_DEGBIN);

 if(a_lon<b_lon)
    return(-1);
 else if(a_lon>b_lon)
    return(1);
 else
   {
    int32_t a_lat=(int32_t)((*a)->latitude*LAT_LONG_DEGBIN);
    int32_t b_lat=(int32_t)((*b)->latitude*LAT_LONG_DEGBIN);

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

 assert(!nodesx->sorted);     /* Must not be sorted */

 for(i=0;i<nodesx->xnumber;i++)
   {
    if(FindFirstSegmentX(segmentsx,nodesx->xdata[i].id))
       highway++;
    else
      {
       nodesx->xdata[i].id=~0;
       nothighway++;
      }

    if(!((i+1)%10000))
      {
       printf("\rChecking: Nodes=%d Highway=%d not-Highway=%d",i+1,highway,nothighway);
       fflush(stdout);
      }
   }

 printf("\rChecked: Nodes=%d Highway=%d not-Highway=%d  \n",nodesx->xnumber,highway,nothighway);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Mark super nodes.

  NodesX* nodesx The set of nodes to process.

  int iteration The final super-node / super-segment iteration number.
  ++++++++++++++++++++++++++++++++++++++*/

void MarkSuperNodes(NodesX *nodesx,int iteration)
{
 int i,nnodes=0;;

 assert(nodesx->sorted);      /* Must be sorted */

 for(i=0;i<nodesx->number;i++)
   {
    if(nodesx->gdata[i]->super==iteration)
      {
       nodesx->gdata[i]->node.firstseg=SEGMENT(~0)|SUPER_FLAG;
       nnodes++;
      }
    else
       nodesx->gdata[i]->node.firstseg=SEGMENT(~0);

    if(!((i+1)%10000))
      {
       printf("\rMarking Super-Nodes: Nodes=%d Super-Nodes=%d",i+1,nnodes);
       fflush(stdout);
      }
   }

 printf("\rMarked Super-Nodes: Nodes=%d Super-Nodes=%d \n",nodesx->number,nnodes);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Assign the segment indexes to the nodes.

  NodesX *nodesx The list of nodes to process.

  SegmentsX* segmentsx The set of segments to use.
  ++++++++++++++++++++++++++++++++++++++*/

void IndexNodes(NodesX *nodesx,SegmentsX* segmentsx)
{
 int i;

 assert(nodesx->sorted);        /* Must be sorted */
 assert(segmentsx->sorted);     /* Must be sorted */

 /* Index the nodes */

 for(i=0;i<segmentsx->number;i++)
   {
    NodeX *node1=FindNodeX(nodesx,segmentsx->sdata[i]->node1);
    NodeX *node2=FindNodeX(nodesx,segmentsx->sdata[i]->node2);

    /* Check node1 */

    if(SEGMENT(node1->node.firstseg)==SEGMENT(~0))
      {
       node1->node.firstseg^=SEGMENT(~0);
       node1->node.firstseg|=i;
      }
    else
      {
       SegmentX *segmentx=LookupSegmentX(segmentsx,SEGMENT(node1->node.firstseg));

       do
         {
          if(segmentx->node1==segmentsx->sdata[i]->node1)
            {
             if(SEGMENT(segmentx->segment.next1)==SEGMENT(~0))
               {
                segmentx->segment.next1=i;
                segmentx=NULL;
               }
             else
                segmentx=LookupSegmentX(segmentsx,SEGMENT(segmentx->segment.next1));
            }
          else
            {
             if(SEGMENT(segmentx->segment.next2)==SEGMENT(~0))
               {
                segmentx->segment.next2=i;
                segmentx=NULL;
               }
             else
                segmentx=LookupSegmentX(segmentsx,SEGMENT(segmentx->segment.next2));
            }
         }
       while(segmentx);
      }

    /* Check node2 */

    if(SEGMENT(node2->node.firstseg)==SEGMENT(~0))
      {
       node2->node.firstseg^=SEGMENT(~0);
       node2->node.firstseg|=i;
      }
    else
      {
       SegmentX *segmentx=LookupSegmentX(segmentsx,SEGMENT(node2->node.firstseg));

       do
         {
          if(segmentx->node1==segmentsx->sdata[i]->node2)
            {
             if(SEGMENT(segmentx->segment.next1)==SEGMENT(~0))
               {
                segmentx->segment.next1=i;
                segmentx=NULL;
               }
             else
                segmentx=LookupSegmentX(segmentsx,SEGMENT(segmentx->segment.next1));
            }
          else
            {
             if(SEGMENT(segmentx->segment.next2)==SEGMENT(~0))
               {
                segmentx->segment.next2=i;
                segmentx=NULL;
               }
             else
                segmentx=LookupSegmentX(segmentsx,SEGMENT(segmentx->segment.next2));
            }
         }
       while(segmentx);
      }

    if(!((i+1)%10000))
      {
       printf("\rIndexing Segments: Segments=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rIndexed Segments: Segments=%d \n",segmentsx->number);
 fflush(stdout);
}
