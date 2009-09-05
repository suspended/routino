/***************************************
 $Header: /home/amb/CVS/routino/src/nodesx.c,v 1.34 2009-09-05 10:35:47 amb Exp $

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
#include <string.h>

#include "types.h"
#include "functions.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "segments.h"
#include "nodes.h"


/* Variables */

extern int option_slim;

/* Functions */

static int sort_by_id(node_t *a,node_t *b);
static int sort_by_lat_long(node_t *a,node_t *b);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new node list.

  NodesX *NewNodeList Returns the node list.

  const char *dirname The name of the directory to save the temporary file into.
  ++++++++++++++++++++++++++++++++++++++*/

NodesX *NewNodeList(const char *dirname)
{
 NodesX *nodesx;

 nodesx=(NodesX*)calloc(1,sizeof(NodesX));

 assert(nodesx); /* Check calloc() worked */

 nodesx->filename=(char*)malloc(strlen(dirname)+24);
 sprintf(nodesx->filename,"nodes.%p.tmp",nodesx);

 nodesx->fd=OpenFile(nodesx->filename);

 return(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a node list.

  NodesX *nodesx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeNodeList(NodesX *nodesx)
{
 DeleteFile(nodesx->filename);

 if(nodesx->xdata)
    free(nodesx->xdata);

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
 Nodes *nodes;
 index_t *offsets;
 ll_bin_t lat_min_bin,lat_max_bin,lon_min_bin,lon_max_bin;
 latlong_t lat_min,lat_max,lon_min,lon_max;
 int latbins,lonbins,latlonbin;
 int super_number=0;

 assert(nodesx->gdata);         /* Must have gdata filled in => sorted geographically */
 assert(nodesx->ndata);         /* Must have ndata filled in => real nodes exist */
 assert(nodesx->idata);         /* Must have idata filled in => sorted by id */

 printf("Writing Nodes: Nodes=0");
 fflush(stdout);

 /* Work out the range of data */

 lat_min=radians_to_latlong( 2);
 lat_max=radians_to_latlong(-2);
 lon_min=radians_to_latlong( 4);
 lon_max=radians_to_latlong(-4);

 for(i=0;i<nodesx->number;i++)
   {
    NodeX *nodex=FindNodeX(nodesx,nodesx->idata[i]);

    if(nodex->latitude<lat_min)
       lat_min=nodex->latitude;
    if(nodex->latitude>lat_max)
       lat_max=nodex->latitude;
    if(nodex->longitude<lon_min)
       lon_min=nodex->longitude;
    if(nodex->longitude>lon_max)
       lon_max=nodex->longitude;

    if(nodesx->ndata[i].firstseg&NODE_SUPER)
       super_number++;
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
    NodeX *nodex=FindNodeX(nodesx,nodesx->gdata[i]);

    ll_bin_t latbin=latlong_to_bin(nodex->latitude )-lat_min_bin;
    ll_bin_t lonbin=latlong_to_bin(nodex->longitude)-lon_min_bin;
    int llbin=lonbin*latbins+latbin;

    for(;latlonbin<=llbin;latlonbin++)
       offsets[latlonbin]=i;
   }

 for(;latlonbin<=(latbins*lonbins);latlonbin++)
    offsets[latlonbin]=nodesx->number;

 /* Fill in a Nodes structure with the offset of the real data in the file after
    the Node structure itself. */

 nodes=calloc(1,sizeof(Nodes));

 assert(nodes); /* Check calloc() worked */

 nodes->number=nodesx->number;
 nodes->snumber=super_number;

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
    Node *node=&nodesx->ndata[IndexNodeX(nodesx,nodesx->gdata[i])];

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
  Find a particular node index.

  index_t IndexNodeX Returns the index of the extended node with the specified id.

  NodesX* nodesx The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexNodeX(NodesX* nodesx,node_t id)
{
 int start=0;
 int end=nodesx->number-1;
 int mid;

 assert(nodesx->idata);         /* Must have idata filled in => sorted by id */

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

 if(end<start)                    /* There are no nodes */
    return(NO_NODE);
 else if(id<nodesx->idata[start]) /* Check key is not before start */
    return(NO_NODE);
 else if(id>nodesx->idata[end])   /* Check key is not after end */
    return(NO_NODE);
 else
   {
    do
      {
       mid=(start+end)/2;             /* Choose mid point */

       if(nodesx->idata[mid]<id)      /* Mid point is too low */
          start=mid+1;
       else if(nodesx->idata[mid]>id) /* Mid point is too high */
          end=mid-1;
       else                           /* Mid point is correct */
          return(mid);
      }
    while((end-start)>1);

    if(nodesx->idata[start]==id)      /* Start is correct */
       return(start);

    if(nodesx->idata[end]==id)        /* End is correct */
       return(end);
   }

 return(NO_NODE);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node.

  NodeX *FindNodeX Returns a pointer to the extended node with the specified id.

  NodesX* nodesx The set of nodes to process.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

NodeX *FindNodeX(NodesX* nodesx,node_t id)
{
 index_t index=IndexNodeX(nodesx,id);

 if(option_slim)
   {
    static NodeX nodex[2];
    static int count=0;

    SeekFile(nodesx->fd,index*sizeof(NodeX));

    count=(count+1)%2;

    ReadFile(nodesx->fd,&nodex[count],sizeof(NodeX));

    return(&nodex[count]);
   }
 else
   {
    return(&nodesx->xdata[index]);
   }
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
 NodeX nodex;

 assert(!nodesx->idata);        /* Must not have idata filled in => unsorted */

 nodex.id=id;
 nodex.latitude =radians_to_latlong(latitude);
 nodex.longitude=radians_to_latlong(longitude);

 WriteFile(nodesx->fd,&nodex,sizeof(NodeX));

 nodesx->xnumber++;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list for the first time (i.e. create the sortable indexes).

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void InitialSortNodeList(NodesX* nodesx)
{
 NodeX nodex;

 assert(!nodesx->idata);        /* Must not have idata filled in => unsorted */

 printf("Creating sortable index");
 fflush(stdout);

 /* Allocate the array of indexes */

 nodesx->idata=(node_t*)malloc(nodesx->xnumber*sizeof(node_t));

 assert(nodesx->idata); /* Check malloc() worked */

 nodesx->number=0;

 CloseFile(nodesx->fd);

 nodesx->fd=ReOpenFile(nodesx->filename);

 while(!ReadFile(nodesx->fd,&nodex,sizeof(NodeX)))
   {
    nodesx->idata[nodesx->number]=nodex.id;
    nodesx->number++;
   }

 printf("\rCreated sortable index \n");

 ReSortNodeList(nodesx);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list again.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void ReSortNodeList(NodesX* nodesx)
{
 index_t i;
 int duplicate;

 assert(nodesx->idata);         /* Must have idata filled in => initially sorted */
 assert(!nodesx->super);        /* Must not have super filled in => not finally sorted */

 printf("Sorting Nodes");
 fflush(stdout);

 /* Sort the node indexes */

 do
   {
    qsort(nodesx->idata,nodesx->number,sizeof(node_t),(int (*)(const void*,const void*))sort_by_id);

    duplicate=0;

    while(nodesx->idata[nodesx->number-1]==NO_NODE)
       nodesx->number--;

    for(i=1;i<nodesx->number;i++)
       if(nodesx->idata[i]==nodesx->idata[i-1])
         {
          nodesx->idata[i-1]=NO_NODE;
          duplicate++;
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
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list for the final time.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void FinalSortNodeList(NodesX* nodesx)
{
 NodeX nodex;

 assert(nodesx->idata);         /* Must have idata filled in => initially sorted */
 assert(!nodesx->super);        /* Must not have super filled in => not finally sorted */

 ReSortNodeList(nodesx);

 /* Sort the on-disk image */

 if(option_slim)
   {
    int fd;

    printf("Sorting disk file");
    fflush(stdout);

    DeleteFile(nodesx->filename);

    fd=OpenFile(nodesx->filename);
    SeekFile(nodesx->fd,0);

    while(!ReadFile(nodesx->fd,&nodex,sizeof(NodeX)))
      {
       index_t index=IndexNodeX(nodesx,nodex.id);

       if(index!=NO_NODE)
         {
          SeekFile(fd,index*sizeof(NodeX));
          WriteFile(fd,&nodex,sizeof(NodeX));
         }
      }

    CloseFile(nodesx->fd);
    CloseFile(fd);

    nodesx->fd=ReOpenFile(nodesx->filename);

    printf("\rSorted disk file \n");
    fflush(stdout);
   }
 else
   {
    nodesx->xdata=(NodeX*)malloc(nodesx->number*sizeof(NodeX));

    assert(nodesx->xdata);      /* Check malloc() worked */

    SeekFile(nodesx->fd,0);

    while(!ReadFile(nodesx->fd,&nodex,sizeof(NodeX)))
      {
       index_t index=IndexNodeX(nodesx,nodex.id);

       if(index!=NO_NODE)
          nodesx->xdata[index]=nodex;
      }

    CloseFile(nodesx->fd);
   }

 /* Allocate and clear the super-node markers */

 if(!nodesx->super)
   {
    nodesx->super=(uint8_t*)malloc(nodesx->number*sizeof(uint8_t));
    memset(nodesx->super,0,nodesx->number*sizeof(uint8_t));
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into id order.

  int sort_by_id Returns the comparison of the id fields.

  node_t *a The first node id.

  node_t *b The second node id.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(node_t *a,node_t *b)
{
 node_t a_id=*a;
 node_t b_id=*b;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*+ A temporary file-local variable for use by the sort function. +*/
static NodesX *sortnodesx;


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list geographically.

  NodesX* nodesx The set of nodes to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeListGeographically(NodesX* nodesx)
{
 index_t i;

 assert(nodesx->idata);         /* Must have idata filled in => sorted by id */
 assert(!nodesx->gdata);        /* Must not have gdata filled in => unsorted geographically */

 printf("Sorting Nodes Geographically");
 fflush(stdout);

 /* Allocate the array of pointers and sort them */

 nodesx->gdata=(node_t*)malloc(nodesx->number*sizeof(node_t));

 assert(nodesx->gdata); /* Check malloc() worked */

 for(i=0;i<nodesx->number;i++)
    nodesx->gdata[i]=nodesx->idata[i];

 sortnodesx=nodesx;

 qsort(nodesx->gdata,nodesx->number,sizeof(node_t),(int (*)(const void*,const void*))sort_by_lat_long);

 printf("\rSorted Nodes Geographically \n");
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into latitude and longitude order.

  int sort_by_lat_long Returns the comparison of the latitude and longitude fields.

  node_t *a The first node id.

  node_t *b The second node id.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_lat_long(node_t *a,node_t *b)
{
 NodeX *nodex_a=FindNodeX(sortnodesx,*a);
 NodeX *nodex_b=FindNodeX(sortnodesx,*b);

 ll_bin_t a_lon=latlong_to_bin(nodex_a->longitude);
 ll_bin_t b_lon=latlong_to_bin(nodex_b->longitude);

 if(a_lon<b_lon)
    return(-1);
 else if(a_lon>b_lon)
    return(1);
 else
   {
    ll_bin_t a_lat=latlong_to_bin(nodex_a->latitude);
    ll_bin_t b_lat=latlong_to_bin(nodex_b->latitude);

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

 assert(nodesx->idata);         /* Must have idata filled in => data sorted */

 printf("Checking: Nodes=0");
 fflush(stdout);

 for(i=0;i<nodesx->number;i++)
   {
    if(FindFirstSegmentX(segmentsx,nodesx->idata[i]))
       highway++;
    else
      {
       nodesx->idata[i]=NO_NODE;
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
}


/*++++++++++++++++++++++++++++++++++++++
  Create the real node data.

  NodesX *nodesx The set of nodes to use.

  int iteration The final super-node iteration.
  ++++++++++++++++++++++++++++++++++++++*/

void CreateRealNodes(NodesX *nodesx,int iteration)
{
 index_t i;

 assert(nodesx->idata);         /* Must have idata filled in => sorted by id */
 assert(!nodesx->ndata);        /* Must not have ndata filled in => no real nodes */

 printf("Creating Real Nodes: Nodes=0");
 fflush(stdout);

 /* Allocate the memory */

 nodesx->ndata=(Node*)malloc(nodesx->number*sizeof(Node));

 assert(nodesx->ndata); /* Check malloc() worked */

 /* Loop through and allocate. */

 for(i=0;i<nodesx->number;i++)
   {
    NodeX *nodex=FindNodeX(nodesx,nodesx->idata[i]);

    nodesx->ndata[i].latoffset=latlong_to_off(nodex->latitude);
    nodesx->ndata[i].lonoffset=latlong_to_off(nodex->longitude);

    nodesx->ndata[i].firstseg=SEGMENT(NO_SEGMENT);

    if(nodesx->super[i]==iteration)
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

 assert(nodesx->idata);         /* Must have idata filled in => sorted */
 assert(nodesx->ndata);         /* Must have ndata filled in => real nodes exist */
 assert(segmentsx->n1data);     /* Must have n1data filled in => sorted */
 assert(segmentsx->sdata);      /* Must have sdata filled in => real segments exist */

 printf("Indexing Segments: Segments=0");
 fflush(stdout);

 /* Index the nodes */

 for(i=0;i<segmentsx->number;i++)
   {
    Node *node1=&nodesx->ndata[IndexNodeX(nodesx,segmentsx->n1data[i]->node1)];
    Node *node2=&nodesx->ndata[IndexNodeX(nodesx,segmentsx->n1data[i]->node2)];

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
          if(segmentsx->n1data[index]->node1==segmentsx->n1data[i]->node1)
            {
             index++;

             if(index>=segmentsx->number || segmentsx->n1data[index]->node1!=segmentsx->n1data[i]->node1)
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
          if(segmentsx->n1data[index]->node1==segmentsx->n1data[i]->node2)
            {
             index++;

             if(index>=segmentsx->number || segmentsx->n1data[index]->node1!=segmentsx->n1data[i]->node2)
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
