/***************************************
 $Header: /home/amb/CVS/routino/src/nodesx.c,v 1.25 2009-07-12 08:38:11 amb Exp $

 Extented Node data type functions.

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


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "functions.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"


/* Constants */

/*+ The array size increment for NodesX (UK is ~10.3M raw nodes, this is ~78 increments). +*/
#define INCREMENT_NODESX (128*1024)


/* Functions */

static int sort_by_id(NodeX **a,NodeX **b);
static int sort_by_lat_long(NodeX **a,NodeX **b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new node list.

  NodesX *NewNodeList Returns the node list.
  ++++++++++++++++++++++++++++++++++++++*/

NodesX *NewNodeList(void)
{
 NodesX *nodesx;

 nodesx=(NodesX*)calloc(1,sizeof(NodesX));

 nodesx->row=-1;

 return(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a node list.

  NodesX *nodesx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeNodeList(NodesX *nodesx)
{
 if(nodesx->xdata)
   {
    int i;
    for(i=0;i<=nodesx->row;i++)
       free(nodesx->xdata[i]);
    free(nodesx->xdata);
   }

 if(nodesx->gdata)
    free(nodesx->gdata);
 if(nodesx->idata)
    free(nodesx->idata);

 if(nodesx->ndata)
    free(nodesx->ndata);

 free(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the node list to a file.

  NodesX* nodesx The set of nodes to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveNodeList(NodesX* nodesx,const char *filename)
{
 index_t i;
 int fd;
 Nodes *nodes=calloc(1,sizeof(Nodes));
 index_t *offsets;
 ll_bin_t lat_min_bin,lat_max_bin,lon_min_bin,lon_max_bin;
 latlong_t lat_min,lat_max,lon_min,lon_max;
 int latbins,lonbins,latlonbin;

 assert(nodesx->sorted);        /* Must be sorted */
 assert(nodesx->gdata);         /* Must have gdata filled in */
 assert(nodesx->ndata);         /* Must have ndata filled in */

 printf("Writing Nodes: Nodes=0");
 fflush(stdout);

 /* Work out the range of data */

 lat_min=radians_to_latlong( 2);
 lat_max=radians_to_latlong(-2);
 lon_min=radians_to_latlong( 4);
 lon_max=radians_to_latlong(-4);

 for(i=0;i<nodesx->number;i++)
   {
    if(nodesx->idata[i]->latitude<lat_min)
       lat_min=nodesx->idata[i]->latitude;
    if(nodesx->idata[i]->latitude>lat_max)
       lat_max=nodesx->idata[i]->latitude;
    if(nodesx->idata[i]->longitude<lon_min)
       lon_min=nodesx->idata[i]->longitude;
    if(nodesx->idata[i]->longitude>lon_max)
       lon_max=nodesx->idata[i]->longitude;
   }

 /* Work out the offsets */

 lat_min_bin=latlong_to_bin(lat_min);
 lon_min_bin=latlong_to_bin(lon_min);
 lat_max_bin=latlong_to_bin(lat_max);
 lon_max_bin=latlong_to_bin(lon_max);

 latbins=(lat_max_bin-lat_min_bin)+1;
 lonbins=(lon_max_bin-lon_min_bin)+1;

 offsets=(index_t*)malloc((latbins*lonbins+1)*sizeof(index_t));

 latlonbin=0;

 for(i=0;i<nodesx->number;i++)
   {
    ll_bin_t latbin=latlong_to_bin(nodesx->gdata[i]->latitude )-lat_min_bin;
    ll_bin_t lonbin=latlong_to_bin(nodesx->gdata[i]->longitude)-lon_min_bin;
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

 nodes->latzero=lat_min_bin;
 nodes->lonzero=lon_min_bin;

 nodes->data=NULL;
 nodes->offsets=(void*)sizeof(Nodes);
 nodes->nodes=(void*)(sizeof(Nodes)+(latbins*lonbins+1)*sizeof(index_t));

 /* Write out the Nodes structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,nodes,sizeof(Nodes));

 WriteFile(fd,offsets,(latbins*lonbins+1)*sizeof(index_t));

 for(i=0;i<nodes->number;i++)
   {
    NodeX **nodex=FindNodeX(nodesx,nodesx->gdata[i]->id);
    Node   *node =&nodesx->ndata[nodex-nodesx->idata];

    WriteFile(fd,node,sizeof(Node));

    if(!((i+1)%10000))
      {
       printf("\rWriting Nodes: Nodes=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rWrote Nodes: Nodes=%d  \n",nodes->number);
 fflush(stdout);

 CloseFile(fd);

 /* Free the fake Nodes */

 free(nodes);
 free(offsets);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node.

  NodeX **FindNodeX Returns a pointer to the extended node with the specified id.

  NodesX* nodesx The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

NodeX **FindNodeX(NodesX* nodesx,node_t id)
{
 int start=0;
 int end=nodesx->number-1;
 int mid;

 assert(nodesx->sorted);        /* Must be sorted */
 assert(nodesx->idata);         /* Must have idata filled in */

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
          return(&nodesx->idata[mid]);
      }
    while((end-start)>1);

    if(nodesx->idata[start]->id==id)      /* Start is correct */
       return(&nodesx->idata[start]);

    if(nodesx->idata[end]->id==id)        /* End is correct */
       return(&nodesx->idata[end]);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a node to a newly created node list (unsorted).

  NodesX* nodesx The set of nodes to process.

  node_t id The node identification.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendNode(NodesX* nodesx,node_t id,double latitude,double longitude)
{
 /* Check that the array has enough space. */

 if(nodesx->row==-1 || nodesx->col==INCREMENT_NODESX)
   {
    nodesx->row++;
    nodesx->col=0;

    if((nodesx->row%16)==0)
       nodesx->xdata=(NodeX**)realloc((void*)nodesx->xdata,(nodesx->row+16)*sizeof(NodeX*));

    nodesx->xdata[nodesx->row]=(NodeX*)malloc(INCREMENT_NODESX*sizeof(NodeX));
   }

 /* Insert the node */

 nodesx->xdata[nodesx->row][nodesx->col].id=id;
 nodesx->xdata[nodesx->row][nodesx->col].super=0;
 nodesx->xdata[nodesx->row][nodesx->col].latitude =radians_to_latlong(latitude);
 nodesx->xdata[nodesx->row][nodesx->col].longitude=radians_to_latlong(longitude);

 nodesx->col++;

 nodesx->sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(NodesX* nodesx)
{
 index_t i;
 int duplicate;

 assert(nodesx->xdata);         /* Must have xdata filled in */

 printf("Sorting Nodes");
 fflush(stdout);

 /* Allocate the array of pointers and sort them */

 nodesx->idata=(NodeX**)realloc(nodesx->idata,(nodesx->row*INCREMENT_NODESX+nodesx->col)*sizeof(NodeX*));

 do
   {
    nodesx->number=0;

    for(i=0;i<(nodesx->row*INCREMENT_NODESX+nodesx->col);i++)
       if(nodesx->xdata[i/INCREMENT_NODESX][i%INCREMENT_NODESX].id!=NO_NODE)
         {
          nodesx->idata[nodesx->number]=&nodesx->xdata[i/INCREMENT_NODESX][i%INCREMENT_NODESX];
          nodesx->number++;
         }

    qsort(nodesx->idata,nodesx->number,sizeof(NodeX*),(int (*)(const void*,const void*))sort_by_id);

    duplicate=0;

    for(i=1;i<nodesx->number;i++)
      {
       if(nodesx->idata[i]->id==nodesx->idata[i-1]->id &&
          nodesx->idata[i]->id!=NO_NODE)
         {
          nodesx->idata[i-1]->id=NO_NODE;
          duplicate++;
         }
      }

    if(duplicate)
      {
       printf(" - %d duplicates found; trying again.\nSorting Nodes",duplicate);
       fflush(stdout);
      }
   }
 while(duplicate);

 printf("\rSorted Nodes \n");
 fflush(stdout);

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
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list geographically.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeListGeographically(NodesX* nodesx)
{
 index_t i;

 assert(nodesx->idata);         /* Must have idata filled in */

 printf("Sorting Nodes Geographically");
 fflush(stdout);

 /* Allocate the array of pointers and sort them */

 nodesx->gdata=(NodeX**)malloc(nodesx->number*sizeof(NodeX*));

 for(i=0;i<nodesx->number;i++)
    nodesx->gdata[i]=nodesx->idata[i];

 qsort(nodesx->gdata,nodesx->number,sizeof(NodeX*),(int (*)(const void*,const void*))sort_by_lat_long);

 printf("\rSorted Nodes Geographically \n");
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into latitude and longitude order.

  int sort_by_lat_long Returns the comparison of the latitude and longitude fields.

  NodeX **a The first Node.

  NodeX **b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_lat_long(NodeX **a,NodeX **b)
{
 ll_bin_t a_lon=latlong_to_bin((*a)->longitude);
 ll_bin_t b_lon=latlong_to_bin((*b)->longitude);

 if(a_lon<b_lon)
    return(-1);
 else if(a_lon>b_lon)
    return(1);
 else
   {
    ll_bin_t a_lat=latlong_to_bin((*a)->latitude);
    ll_bin_t b_lat=latlong_to_bin((*b)->latitude);

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
 index_t i;
 int highway=0,nothighway=0;

 assert(nodesx->xdata);         /* Must have xdata filled in */

 printf("Checking: Nodes=0");
 fflush(stdout);

 for(i=0;i<(nodesx->row*INCREMENT_NODESX+nodesx->col);i++)
   {
    if(FindFirstSegmentX(segmentsx,nodesx->xdata[i/INCREMENT_NODESX][i%INCREMENT_NODESX].id))
       highway++;
    else
      {
       nodesx->xdata[i/INCREMENT_NODESX][i%INCREMENT_NODESX].id=NO_NODE;
       nothighway++;
      }

    if(!((i+1)%10000))
      {
       printf("\rChecking: Nodes=%d Highway=%d not-Highway=%d",i+1,highway,nothighway);
       fflush(stdout);
      }
   }

 printf("\rChecked: Nodes=%d Highway=%d not-Highway=%d  \n",i,highway,nothighway);
 fflush(stdout);

 nodesx->sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Create the real node data.

  NodesX *nodesx The set of nodes to use.

  int iteration The final super-node iteration.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateRealNodes(NodesX *nodesx,int iteration)
{
 index_t i;

 assert(nodesx->gdata);         /* Must have gdata filled in */

 printf("Creating Real Nodes: Nodes=0");
 fflush(stdout);

 /* Allocate the memory */

 nodesx->ndata=(Node*)malloc(nodesx->number*sizeof(Node));

 /* Loop through and allocate. */

 for(i=0;i<nodesx->number;i++)
   {
    nodesx->ndata[i].latoffset=latlong_to_off(nodesx->idata[i]->latitude);
    nodesx->ndata[i].lonoffset=latlong_to_off(nodesx->idata[i]->longitude);

    nodesx->ndata[i].firstseg=SEGMENT(NO_SEGMENT);

    if(nodesx->idata[i]->super==iteration)
       nodesx->ndata[i].firstseg|=NODE_SUPER;

    if(!((i+1)%10000))
      {
       printf("\rCreating Real Nodes: Nodes=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rCreating Real Nodes: Nodes=%d \n",nodesx->number);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Assign the segment indexes to the nodes.

  NodesX *nodesx The list of nodes to process.

  SegmentsX* segmentsx The set of segments to use.
  ++++++++++++++++++++++++++++++++++++++*/

void IndexNodes(NodesX *nodesx,SegmentsX *segmentsx)
{
 index_t i;

 assert(nodesx->sorted);        /* Must be sorted */
 assert(nodesx->idata);         /* Must have idata filled in */
 assert(nodesx->ndata);         /* Must have ndata filled in */
 assert(segmentsx->sorted);     /* Must be sorted */
 assert(segmentsx->ndata);      /* Must have ndata filled in */
 assert(segmentsx->sdata);      /* Must have sdata filled in */

 printf("Indexing Segments: Segments=0");
 fflush(stdout);

 /* Index the nodes */

 for(i=0;i<segmentsx->number;i++)
   {
    NodeX **nodex1=FindNodeX(nodesx,segmentsx->ndata[i]->node1);
    NodeX **nodex2=FindNodeX(nodesx,segmentsx->ndata[i]->node2);
    Node  *node1=&nodesx->ndata[nodex1-nodesx->idata];
    Node  *node2=&nodesx->ndata[nodex2-nodesx->idata];

    /* Check node1 */

    if(SEGMENT(node1->firstseg)==SEGMENT(NO_SEGMENT))
      {
       node1->firstseg^=SEGMENT(NO_SEGMENT);
       node1->firstseg|=i;
      }
    else
      {
       index_t index=SEGMENT(node1->firstseg);

       do
         {
          if(segmentsx->ndata[index]->node1==segmentsx->ndata[i]->node1)
            {
             index++;

             if(index>=segmentsx->number || segmentsx->ndata[index]->node1!=segmentsx->ndata[i]->node1)
                break;
            }
          else
            {
             if(segmentsx->sdata[index].next2==NO_NODE)
               {
                segmentsx->sdata[index].next2=i;
                break;
               }
             else
                index=segmentsx->sdata[index].next2;
            }
         }
       while(1);
      }

    /* Check node2 */

    if(SEGMENT(node2->firstseg)==SEGMENT(NO_SEGMENT))
      {
       node2->firstseg^=SEGMENT(NO_SEGMENT);
       node2->firstseg|=i;
      }
    else
      {
       index_t index=SEGMENT(node2->firstseg);

       do
         {
          if(segmentsx->ndata[index]->node1==segmentsx->ndata[i]->node2)
            {
             index++;

             if(index>=segmentsx->number || segmentsx->ndata[index]->node1!=segmentsx->ndata[i]->node2)
                break;
            }
          else
            {
             if(segmentsx->sdata[index].next2==NO_NODE)
               {
                segmentsx->sdata[index].next2=i;
                break;
               }
             else
                index=segmentsx->sdata[index].next2;
            }
         }
       while(1);
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
