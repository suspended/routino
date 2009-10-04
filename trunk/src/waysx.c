/***************************************
 $Header: /home/amb/CVS/routino/src/waysx.c,v 1.22 2009-10-04 15:52:37 amb Exp $

 Extended Way data type functions.

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

#include "functions.h"
#include "waysx.h"
#include "ways.h"


/* Constants */

#define SORT_RAMSIZE (64*1024*1024)

/* Variables */

extern int option_slim;
extern char *tmpdirname;

/* Functions */

static int sort_by_id(WayX *a,WayX *b);
static int index_by_id(WayX *wayx,index_t index);

static int sort_by_name(char **a,char **b);
static index_t index_way_name(char** names,int number,char *name);
static int sort_by_name_and_properties(Way **a,Way **b);
static index_t index_way(Way** data,int number,Way *way);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new way list.

  WaysX *NewWayList Returns the way list.
  ++++++++++++++++++++++++++++++++++++++*/

WaysX *NewWayList(void)
{
 WaysX *waysx;

 waysx=(WaysX*)calloc(1,sizeof(WaysX));

 assert(waysx); /* Check calloc() worked */

 waysx->filename=(char*)malloc(strlen(tmpdirname)+24);
 sprintf(waysx->filename,"%s/ways.%p.tmp",tmpdirname,waysx);

 waysx->fd=OpenFile(waysx->filename);

 waysx->nfilename=(char*)malloc(strlen(tmpdirname)+24);
 sprintf(waysx->nfilename,"%s/waynames.%p.tmp",tmpdirname,waysx);

 waysx->nfd=OpenFile(waysx->nfilename);

 return(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a way list.

  WaysX *waysx The list to be freed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeWayList(WaysX *waysx)
{
 DeleteFile(waysx->filename);

 if(waysx->xdata)
    UnmapFile(waysx->filename);

 if(waysx->idata)
    free(waysx->idata);

 DeleteFile(waysx->nfilename);

 free(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the way list to a file.

  WaysX* waysx The set of ways to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveWayList(WaysX* waysx,const char *filename)
{
 index_t i;
 int fd;
 Ways *ways;

 printf("Writing Ways: Ways=0");
 fflush(stdout);

 if(option_slim)
    waysx->xdata=MapFile(waysx->filename);

 /* Fill in a Ways structure with the offset of the real data in the file after
    the Way structure itself. */

 ways=calloc(1,sizeof(Ways));

 assert(ways); /* Check calloc() worked */

 ways->number=waysx->cnumber;
 ways->onumber=waysx->number;

 ways->data=NULL;

 ways->ways=(void*)sizeof(Ways);
 ways->names=(void*)(sizeof(Ways)+ways->number*sizeof(Way));

 /* Write out the Ways structure and then the real data. */

 fd=OpenFile(filename);

 WriteFile(fd,ways,sizeof(Ways));

 for(i=0;i<waysx->number;i++)
   {
    SeekFile(fd,sizeof(Ways)+waysx->xdata[i].cid*sizeof(Way));
    WriteFile(fd,&waysx->xdata[i].way,sizeof(Way));

    if(!((i+1)%10000))
      {
       printf("\rWriting Ways: Ways=%d",i+1);
       fflush(stdout);
      }
   }

 if(option_slim)
    waysx->xdata=UnmapFile(waysx->filename);

 waysx->names=MapFile(waysx->nfilename);

 SeekFile(fd,sizeof(Ways)+waysx->cnumber*sizeof(Way));
 WriteFile(fd,waysx->names,waysx->nlength);

 waysx->names=UnmapFile(waysx->nfilename);

 CloseFile(fd);

 printf("\rWrote Ways: Ways=%d  \n",waysx->number);
 fflush(stdout);

 /* Free the fake Ways */

 free(ways);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular way index.

  index_t IndexWayX Returns the index of the extended way with the specified id.

  WaysX* waysx The set of ways to process.

  way_t id The way id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexWayX(WaysX* waysx,way_t id)
{
 int start=0;
 int end=waysx->number-1;
 int mid;

 assert(waysx->idata);         /* Must have idata filled in => sorted */

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

 if(end<start)                   /* There are no ways */
    return(NO_WAY);
 else if(id<waysx->idata[start]) /* Check key is not before start */
    return(NO_WAY);
 else if(id>waysx->idata[end])   /* Check key is not after end */
    return(NO_WAY);
 else
   {
    do
      {
       mid=(start+end)/2;            /* Choose mid point */

       if(waysx->idata[mid]<id)      /* Mid point is too low */
          start=mid+1;
       else if(waysx->idata[mid]>id) /* Mid point is too high */
          end=mid-1;
       else                          /* Mid point is correct */
          return(mid);
      }
    while((end-start)>1);

    if(waysx->idata[start]==id)      /* Start is correct */
       return(start);

    if(waysx->idata[end]==id)        /* End is correct */
       return(end);
   }

 return(NO_WAY);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup a particular way.

  WayX *LookupWayX Returns a pointer to the extended way with the specified id.

  WaysX* waysx The set of ways to process.

  index_t index The way index to look for.

  int position The position in the cache to use.
  ++++++++++++++++++++++++++++++++++++++*/

WayX *LookupWayX(WaysX* waysx,index_t index,int position)
{
 assert(index!=NO_WAY);     /* Must be a valid way */

 if(option_slim)
   {
    SeekFile(waysx->fd,index*sizeof(WayX));

    ReadFile(waysx->fd,&waysx->cached[position-1],sizeof(WayX));

    return(&waysx->cached[position-1]);
   }
 else
   {
    return(&waysx->xdata[index]);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Append a way to a way list.

  void AppendWay Returns the newly appended way.

  WaysX* waysx The set of ways to process.

  way_t id The ID of the way.

  Way *way The way data itself.

  const char *name The name or reference of the way.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendWay(WaysX* waysx,way_t id,Way *way,const char *name)
{
 WayX wayx;

 assert(!waysx->idata);       /* Must not have idata filled in => unsorted */

 wayx.id=id;
 wayx.way=*way;
 wayx.name=waysx->nlength;

 WriteFile(waysx->fd,&wayx,sizeof(WayX));

 waysx->xnumber++;

 WriteFile(waysx->nfd,name,strlen(name)+1);

 waysx->nlength+=strlen(name)+1;
}


/*+ A temporary file-local variable for use by the sort functions. +*/
static WaysX *sortwaysx;


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of ways.

  WaysX* waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortWayList(WaysX* waysx)
{
 int fd;

 /* Check the start conditions */

 assert(!waysx->idata);         /* Must not have idata filled in => unsorted */

 /* Print the start message */

 printf("Sorting Ways");
 fflush(stdout);

 /* Close the files and re-open them (finished appending) */

 CloseFile(waysx->fd);
 waysx->fd=ReOpenFile(waysx->filename);

 CloseFile(waysx->nfd);
 waysx->nfd=ReOpenFile(waysx->nfilename);

 DeleteFile(waysx->filename);

 fd=OpenFile(waysx->filename);

 /* Allocate the array of indexes */

 waysx->idata=(way_t*)malloc(waysx->xnumber*sizeof(way_t));

 assert(waysx->idata); /* Check malloc() worked */

 /* Sort the way indexes and remove duplicates */

 sortwaysx=waysx;

 filesort(waysx->fd,fd,sizeof(WayX),SORT_RAMSIZE,(int (*)(const void*,const void*))sort_by_id,(int (*)(void*,index_t))index_by_id);

 /* Close the files and re-open them */

 CloseFile(waysx->fd);
 CloseFile(fd);

 waysx->fd=ReOpenFile(waysx->filename);

 if(!option_slim)
    waysx->xdata=MapFile(waysx->filename);

 /* Print the final message */

 printf("\rSorted Ways: Ways=%d Duplicates=%d\n",waysx->xnumber,waysx->xnumber-waysx->number);
 fflush(stdout);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into id order.

  int sort_by_id Returns the comparison of the id fields.

  WayX *a The first extended way.

  WayX *b The second extended way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(WayX *a,WayX *b)
{
 way_t a_id=a->id;
 way_t b_id=b->id;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Index the ways after sorting.

  index_by_id Return 1 if the value is to be kept, otherwise zero.

  WayX *wayx The extended way.

  index_t index The index of this way in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int index_by_id(WayX *wayx,index_t index)
{
 if(index==0 || sortwaysx->idata[index-1]!=wayx->id)
   {
    sortwaysx->idata[index]=wayx->id;

    sortwaysx->number++;

    return(1);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Compact the list of way names.

  WaysX* waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void CompactWayNames(WaysX* waysx)
{
 index_t i,j,*offsets;
 char **cnames;
 WayX wayx;
 int duplicate=0;
 int fd,nfd;

 /* Print the start message for sorting names */

 printf("Sorting Way names");
 fflush(stdout);

 /* Get the uncompacted name data and create list for compacted */

 if(option_slim)
    waysx->xdata=MapFile(waysx->filename);

 waysx->names=MapFile(waysx->nfilename);

 waysx->nnumber=waysx->number;

 cnames=(char**)malloc(waysx->nnumber*sizeof(char*));

 assert(cnames); /* Check malloc() worked */

 /* Create the index of names, sort it and remove duplicates */

 for(i=0;i<waysx->nnumber;i++)
    cnames[i]=&waysx->names[waysx->xdata[i].name];

 qsort(cnames,waysx->nnumber,sizeof(char*),(int (*)(const void*,const void*))sort_by_name);

 j=0;
 for(i=1;i<waysx->nnumber;i++)
    if(strcmp(cnames[i],cnames[j]))
       cnames[++j]=cnames[i];
    else
       duplicate++;

 waysx->nnumber=++j;

 /* Sort the on-disk image */

 DeleteFile(waysx->nfilename);

 nfd=OpenFile(waysx->nfilename);

 offsets=(index_t*)malloc(waysx->nnumber*sizeof(index_t));

 assert(offsets); /* Check malloc() worked */

 waysx->nlength=0;

 for(i=0;i<waysx->nnumber;i++)
   {
    offsets[i]=waysx->nlength;
    waysx->nlength+=strlen(cnames[i])+1;
    WriteFile(nfd,cnames[i],strlen(cnames[i])+1);
   }

 CloseFile(waysx->nfd);
 CloseFile(nfd);

 waysx->nfd=ReOpenFile(waysx->nfilename);

 /* Print the final message for sorting names */

 printf("\rSorted Way names: Names=%d Duplicate=%d\n",waysx->number,duplicate);
 fflush(stdout);

 /* Print the start message for compacting names */

 printf("Compacting Way names");
 fflush(stdout);

 /* Update the on-disk image */

 DeleteFile(waysx->filename);

 fd=OpenFile(waysx->filename);
 SeekFile(waysx->fd,0);

 while(!ReadFile(waysx->fd,&wayx,sizeof(WayX)))
   {
    wayx.way.name=offsets[index_way_name(cnames,waysx->nnumber,&waysx->names[wayx.name])];

    WriteFile(fd,&wayx,sizeof(WayX));
   }

 CloseFile(waysx->fd);
 CloseFile(fd);

 waysx->xdata=UnmapFile(waysx->filename);
 waysx->names=UnmapFile(waysx->nfilename);

 waysx->fd=ReOpenFile(waysx->filename);

 if(!option_slim)
    waysx->xdata=MapFile(waysx->filename);

 /* Print the final message for compacting names */

 printf("\rCompacted Way names: Names=%d Unique=%d\n",waysx->number,waysx->nnumber);
 fflush(stdout);

 free(cnames);
 free(offsets);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the way names.

  int sort_by_name Returns the comparison of the name fields.

  char **a The first extended Way.

  char **b The second extended Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name(char **a,char **b)
{
 if(*a==NULL)
    return(1);
 else if(*b==NULL)
    return(-1);
 else
    return(strcmp(*a,*b));
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular name within the sorted list of names.

  index_t index_way_name Returns the index of the name within the list.

  char **names The set of names to search.

  int number The number of names.

  char *name The name to look for.
  ++++++++++++++++++++++++++++++++++++++*/

static index_t index_way_name(char** names,int number,char *name)
{
 int start=0;
 int end=number-1;
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

 do
   {
    mid=(start+end)/2;                 /* Choose mid point */

    if(strcmp(name,names[mid])>0)      /* Mid point is too low */
       start=mid+1;
    else if(strcmp(name,names[mid])<0) /* Mid point is too high */
       end=mid-1;
    else                               /* Mid point is correct */
       return(mid);
   }
 while((end-start)>1);

 if(strcmp(name,names[start])==0)      /* Start is correct */
    return(start);

 if(strcmp(name,names[end])==0)        /* End is correct */
    return(end);

 assert(0);

 return(NO_WAY);
}


/*++++++++++++++++++++++++++++++++++++++
  Compact the list of way properties.

  WaysX* waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void CompactWayProperties(WaysX* waysx)
{
 index_t i,j;
 WayX wayx;
 Way **cdata;
 int duplicate=0;
 int fd;

 /* Print the start message for sorting properties */

 printf("Sorting Ways by properties");
 fflush(stdout);

 /* Get the uncompacted data and create list for compacted */

 if(option_slim)
    waysx->xdata=MapFile(waysx->filename);

 waysx->cnumber=waysx->number;

 cdata=(Way**)malloc(waysx->cnumber*sizeof(Way*));

 assert(cdata); /* Check malloc() worked */

 for(i=0;i<waysx->cnumber;i++)
    cdata[i]=&waysx->xdata[i].way;

 /* Create the index of names, sort it and remove duplicates */

 qsort(cdata,waysx->cnumber,sizeof(Way*),(int (*)(const void*,const void*))sort_by_name_and_properties);

 j=0;
 for(i=1;i<waysx->cnumber;i++)
    if(cdata[i-1]->name!=cdata[i]->name || WaysCompare(cdata[i-1],cdata[i]))
       cdata[++j]=cdata[i];
    else
       duplicate++;

 waysx->cnumber=++j;

 /* Print the final message for sorting properties */

 printf("\rSorted Ways by properties: Properties=%d Duplicate=%d\n",waysx->number,duplicate);
 fflush(stdout);

 /* Print the start message for compacting properties */

 printf("Compacting Way properties");
 fflush(stdout);

 /* Update the on-disk image */

 DeleteFile(waysx->filename);

 fd=OpenFile(waysx->filename);
 SeekFile(waysx->fd,0);

 while(!ReadFile(waysx->fd,&wayx,sizeof(WayX)))
   {
    wayx.cid=index_way(cdata,waysx->cnumber,&wayx.way);

    WriteFile(fd,&wayx,sizeof(WayX));
   }

 CloseFile(waysx->fd);
 CloseFile(fd);

 waysx->fd=ReOpenFile(waysx->filename);

 waysx->xdata=UnmapFile(waysx->filename);

 if(!option_slim)
    waysx->xdata=MapFile(waysx->filename);

 /* Print the final message for compacting properties */

 printf("\rCompacted Way properties: Properties=%d Unique=%d\n",waysx->number,waysx->cnumber);
 fflush(stdout);

 free(cdata);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into name and properties order.

  int sort_by_name_and_properties Returns the comparison of the name and properties fields.

  Way **a The first Way.

  Way **b The second Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name_and_properties(Way **a,Way **b)
{
 if(*a==NULL)
    return(1);
 else if(*b==NULL)
    return(-1);
 else
   {
    index_t a_name=(*a)->name;
    index_t b_name=(*b)->name;

    if(a_name<b_name)
       return(-1);
    else if(a_name>b_name)
       return(1);
    else
      {
       Way *a_way=*a;
       Way *b_way=*b;

       return(WaysCompare(a_way,b_way));
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular Way within the sorted list of ways.

  index_t index_way Returns the index of the Way within the list.

  Way **data The set of Ways to search.

  int number The number of Ways.

  Way *way The name to look for.
  ++++++++++++++++++++++++++++++++++++++*/

static index_t index_way(Way** data,int number,Way *way)
{
 int start=0;
 int end=number-1;
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

 do
   {
    mid=(start+end)/2;                                                 /* Choose mid point */

    if(way->name>data[mid]->name)      /* Mid point is too low */
       start=mid+1;
    else if(way->name<data[mid]->name) /* Mid point is too high */
       end=mid-1;
    else if(WaysCompare(way,data[mid])>0) /* Mid point is too low */
       start=mid+1;
    else if(WaysCompare(way,data[mid])<0) /* Mid point is too high */
       end=mid-1;
    else                                                               /* Mid point is correct */
       return(mid);
   }
 while((end-start)>1);

 if(way->name==data[start]->name && WaysCompare(way,data[start])==0)   /* Start is correct */
    return(start);

 if(way->name==data[end]->name && WaysCompare(way,data[end])==0)       /* End is correct */
    return(end);

 assert(0);

 return(NO_WAY);
}
