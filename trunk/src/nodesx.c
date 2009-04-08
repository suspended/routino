/***************************************
 $Header: /home/amb/CVS/routino/src/nodesx.c,v 1.8 2009-04-08 16:54:34 amb Exp $

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
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "types.h"
#include "functions.h"
#include "nodesx.h"
#include "segmentsx.h"


/* Constants */

/*+ The array size increment for nodes - expect ~8,000,000 nodes. +*/
#define INCREMENT_NODES 1024*1024


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
 int32_t lat_min,lat_max,lon_min,lon_max;
 int latbins,lonbins,latlonbin;

 assert(nodesx->sorted);        /* Must be sorted */

 /* Work out the offsets */

 lat_min=lat_long_to_bin(nodesx->lat_min);
 lon_min=lat_long_to_bin(nodesx->lon_min);
 lat_max=lat_long_to_bin(nodesx->lat_max);
 lon_max=lat_long_to_bin(nodesx->lon_max);

 latbins=(lat_max-lat_min)+1;
 lonbins=(lon_max-lon_min)+1;

 offsets=malloc((latbins*lonbins+1)*sizeof(index_t));

 latlonbin=0;

 for(i=0;i<nodesx->number;i++)
   {
    int32_t latbin=lat_long_to_bin(nodesx->gdata[i]->latitude )-lat_min;
    int32_t lonbin=lat_long_to_bin(nodesx->gdata[i]->longitude)-lon_min;
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
   {
    WriteFile(fd,&nodesx->gdata[i]->node,sizeof(Node));

    if(!((i+1)%10000))
      {
       printf("\rWriting Nodes: Nodes=%d",i+1);
       fflush(stdout);
      }
   }

 printf("\rWrote Nodes: Nodes=%d  \n",nodesx->number);
 fflush(stdout);

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
 nodesx->xdata[nodesx->xnumber].latitude =floorf(latitude *LAT_LONG_SCALE)/LAT_LONG_SCALE;
 nodesx->xdata[nodesx->xnumber].longitude=floorf(longitude*LAT_LONG_SCALE)/LAT_LONG_SCALE;

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
 int duplicate;

 printf("Sorting Nodes"); fflush(stdout);

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

 sort_again:

 nodesx->number=0;

 for(i=0;i<nodesx->xnumber;i++)
    if(nodesx->xdata[i].id!=~0)
      {
       nodesx->gdata[nodesx->number]=&nodesx->xdata[i];
       nodesx->idata[nodesx->number]=&nodesx->xdata[i];
       nodesx->number++;
      }

 nodesx->sorted=1;

 /* Sort by id */

 qsort(nodesx->idata,nodesx->number,sizeof(NodeX*),(int (*)(const void*,const void*))sort_by_id);

 duplicate=0;

 for(i=1;i<nodesx->number;i++)
   {
    if(nodesx->idata[i]->id==nodesx->idata[i-1]->id &&
       nodesx->idata[i]->id!=~0)
      {
       nodesx->idata[i-1]->id=~0;
       duplicate++;
      }
   }

 if(duplicate)
   {
    printf(" - %d duplicates found; trying again.\nSorting Nodes",duplicate); fflush(stdout);
    goto sort_again;
   }

 /* Sort geographically */

 qsort(nodesx->gdata,nodesx->number,sizeof(NodeX*),(int (*)(const void*,const void*))sort_by_lat_long);

 nodesx->lat_min=2;
 nodesx->lat_max=-2;
 nodesx->lon_min=4;
 nodesx->lon_max=-4;

 for(i=0;i<nodesx->number;i++)
   {
    int32_t lat=(int32_t)(nodesx->gdata[i]->latitude *LAT_LONG_SCALE);
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

 printf("\rSorted Nodes \n"); fflush(stdout);
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
  Sort the nodes into latitude and longitude order.

  int sort_by_lat_long Returns the comparison of the latitude and longitude fields.

  NodeX **a The first Node.

  NodeX **b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_lat_long(NodeX **a,NodeX **b)
{
 int32_t a_lon=lat_long_to_bin((*a)->longitude);
 int32_t b_lon=lat_long_to_bin((*b)->longitude);

 if(a_lon<b_lon)
    return(-1);
 else if(a_lon>b_lon)
    return(1);
 else
   {
    int32_t a_lat=lat_long_to_bin((*a)->latitude);
    int32_t b_lat=lat_long_to_bin((*b)->latitude);

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
       SegmentX **segmentx=LookupSegmentX(segmentsx,SEGMENT(node1->node.firstseg));

       do
         {
          if((*segmentx)->node1==segmentsx->sdata[i]->node1)
            {
             segmentx++;

             if((*segmentx)->node1!=segmentsx->sdata[i]->node1 || (segmentx-segmentsx->sdata)>=segmentsx->number)
                segmentx=NULL;
            }
          else
            {
             if((*segmentx)->segment.next2==~0)
               {
                (*segmentx)->segment.next2=i;
                segmentx=NULL;
               }
             else
                segmentx=LookupSegmentX(segmentsx,(*segmentx)->segment.next2);
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
       SegmentX **segmentx=LookupSegmentX(segmentsx,SEGMENT(node2->node.firstseg));

       do
         {
          if((*segmentx)->node1==segmentsx->sdata[i]->node2)
            {
             segmentx++;

             if((*segmentx)->node1!=segmentsx->sdata[i]->node2 || (segmentx-segmentsx->sdata)>=segmentsx->number)
                segmentx=NULL;
            }
          else
            {
             if((*segmentx)->segment.next2==~0)
               {
                (*segmentx)->segment.next2=i;
                segmentx=NULL;
               }
             else
                segmentx=LookupSegmentX(segmentsx,(*segmentx)->segment.next2);
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
