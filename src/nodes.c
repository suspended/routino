/***************************************
 $Header: /home/amb/CVS/routino/src/nodes.c,v 1.3 2009-01-04 17:51:23 amb Exp $

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
#include "types.h"

#define INCREMENT 256*1024

/*+ The list of nodes +*/
Nodes *OSMNodes=NULL;

/*+ Is the data sorted and therefore searchable? +*/
static int sorted=0;

/*+ Is the data loaded from a file and therefore read-only? +*/
static int loaded=0;

/*+ How many entries are allocated? +*/
static size_t alloced=0;

/* Functions */

static int sort_by_id(Node *a,Node *b);


/*++++++++++++++++++++++++++++++++++++++
  Load in a node list from a file.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

void LoadNodeList(const char *filename)
{
 assert(!OSMNodes);             /* Must be NULL */

 OSMNodes=(Nodes*)MapFile(filename);

 assert(OSMNodes);              /* Must be non-NULL */

 sorted=1;
 loaded=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Save the node list to a file.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveNodeList(const char *filename)
{
 int retval;

 assert(!loaded);               /* Must not be loaded */

 assert(sorted);                /* Must be sorted */

 retval=WriteFile(filename,OSMNodes,sizeof(Nodes)-sizeof(OSMNodes->nodes)+OSMNodes->number*sizeof(Node));

 assert(!retval);               /* Must be zero */
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular node.

  Node *FindNode Returns a pointer to the node with the specified id.

  node_t id The node id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

Node *FindNode(node_t id)
{
 int start=0;
 int end=OSMNodes->number-1;
 int mid;

 assert(sorted);                /* Must be sorted */

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

 if(OSMNodes->number==0)               /* There are no nodes */
    return(NULL);
 else if(id<OSMNodes->nodes[start].id) /* Check key is not before start */
    return(NULL);
 else if(id>OSMNodes->nodes[end].id)   /* Check key is not after end */
    return(NULL);
 else
   {
    do
      {
       mid=(start+end)/2;                  /* Choose mid point */

       if(OSMNodes->nodes[mid].id<id)      /* Mid point is too low */
          start=mid+1;
       else if(OSMNodes->nodes[mid].id>id) /* Mid point is too high */
          end=mid-1;
       else                                /* Mid point is correct */
          return(&OSMNodes->nodes[mid]);
      }
    while((end-start)>1);

    if(OSMNodes->nodes[start].id==id)      /* Start is correct */
       return(&OSMNodes->nodes[start]);

    if(OSMNodes->nodes[end].id==id)        /* End is correct */
       return(&OSMNodes->nodes[end]);
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a node to a newly created node list (unsorted).

  node_t id The node identification.

  latlong_t latitude The latitude of the node.

  latlong_t longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendNode(node_t id,latlong_t latitude,latlong_t longitude)
{
 assert(!loaded);               /* Must not be loaded */

 /* Check that the whole thing is allocated. */

 if(!OSMNodes)
   {
    alloced=INCREMENT;
    OSMNodes=(Nodes*)malloc(sizeof(Nodes)-sizeof(OSMNodes->nodes)+alloced*sizeof(Node));

    OSMNodes->number=0;
   }

 /* Check that the arrays have enough space. */

 if(OSMNodes->number==alloced)
   {
    alloced+=INCREMENT;
    OSMNodes=(Nodes*)realloc((void*)OSMNodes,sizeof(Nodes)-sizeof(OSMNodes->nodes)+alloced*sizeof(Node));
   }

 /* Insert the node */

 OSMNodes->nodes[OSMNodes->number].id=id;
 OSMNodes->nodes[OSMNodes->number].latitude=latitude;
 OSMNodes->nodes[OSMNodes->number].longitude=longitude;

 OSMNodes->number++;

 sorted=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the node list.
  ++++++++++++++++++++++++++++++++++++++*/

void SortNodeList(void)
{
 assert(!loaded);               /* Must not be loaded */

 qsort(OSMNodes->nodes,OSMNodes->number,sizeof(Node),(int (*)(const void*,const void*))sort_by_id);

 sorted=1;
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the nodes into id order.

  int sort_by_id Returns the comparison of the id fields.

  Node *a The first Node.

  Node *b The second Node.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(Node *a,Node *b)
{
 node_t a_id=a->id;
 node_t b_id=b->id;

 return(a_id-b_id);
}
