/***************************************
 $Header: /home/amb/CVS/routino/src/sorting.c,v 1.2 2009-10-04 15:53:31 amb Exp $

 Merge sort functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2009 Andrew M. Bishop

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
#include <assert.h>

#include "functions.h"


/* Variables */

extern char *tmpdirname;


/*++++++++++++++++++++++++++++++++++++++
  A function to sort the contents of a file using a limited amount of RAM.

  The data is sorted using a "Merge sort" http://en.wikipedia.org/wiki/Merge_sort
  and in particular an "external sort" http://en.wikipedia.org/wiki/External_sorting.
  The individual sort steps and the merge step both use a "Heap sort"
  http://en.wikipedia.org/wiki/Heapsort.  The combination of the two should work well
  if the data is already partially sorted.

  int fd_in The file descriptor of the input file (opened for reading and at the beginning).

  int fd_out The file descriptor of the output file (opened for writing and empty).

  size_t itemsize The size of each item in the file that needs sorting.

  size_t ramsize The maximum in-core buffer size to use when sorting.

  int (*compare)(const void*, const void*) The comparison function (identical to qsort if the
                                           data to be sorted is an array of things not pointers).

  int (*buildindex)(void *,index_t) If non-NULL then this function is called for each item, if it
                                    returns 1 then it is written to the output file.
  ++++++++++++++++++++++++++++++++++++++*/

void filesort(int fd_in,int fd_out,size_t itemsize,size_t ramsize,int (*compare)(const void*,const void*),
                                                                  int (*buildindex)(void*,index_t))
{
 int *fds=NULL,*heap=NULL;
 int nfiles=0,ndata=0;
 index_t count=0,total=0;
 size_t nitems=ramsize/(itemsize+sizeof(void*));
 void *data,**datap;
 char *filename;
 int i,more=1;

 /* Allocate the RAM buffer and other bits */

 data=malloc(nitems*itemsize);
 datap=malloc(nitems*sizeof(void*));

 filename=(char*)malloc(strlen(tmpdirname)+24);

 /* Loop around, fill the buffer, sort the data and write a temporary file */

 do
   {
    int fd,n=0;

    /* Read in the data and create pointers */

    for(i=0;i<nitems;i++)
      {
       datap[i]=data+i*itemsize;

       if(ReadFile(fd_in,datap[i],itemsize))
         {
          more=0;
          break;
         }

       total++;
      }

    n=i;

    if(n==0)
       break;

    /* Sort the data pointers using a heap sort */

    heapsort(datap,n,compare);

    /* Shortcut if all read in and sorted at once */

    if(nfiles==0 && n<nitems)
      {
       for(i=0;i<n;i++)
         {
          if(!buildindex || buildindex(datap[i],count))
            {
             WriteFile(fd_out,datap[i],itemsize);
             count++;
            }
         }

       return;
      }

    /* Create a temporary file and write the result */

    sprintf(filename,"%s/filesort.%d.tmp",tmpdirname,nfiles);

    fd=OpenFile(filename);

    for(i=0;i<n;i++)
       WriteFile(fd,datap[i],itemsize);

    CloseFile(fd);

    nfiles++;
   }
 while(more);

 /* Shortcut if only one file (unlucky for us there must have been exactly
    nitems, lucky for us we still have the data in RAM) */

 if(nfiles==1)
   {
    for(i=0;i<nitems;i++)
      {
       if(!buildindex || buildindex(datap[i],count))
         {
          WriteFile(fd_out,datap[i],itemsize);
          count++;
         }
      }

    DeleteFile(filename);

    return;
   }

 /* Check that number of files is less than file size */

 assert(nfiles<nitems);

 /* Open all of the temporary files */

 fds=(int*)malloc(nfiles*sizeof(int));

 for(i=0;i<nfiles;i++)
   {
    sprintf(filename,"%s/filesort.%d.tmp",tmpdirname,i);

    fds[i]=ReOpenFile(filename);

    DeleteFile(filename);
   }

 /* Perform an n-way merge using a binary heap */

 heap=(int*)malloc(nfiles*sizeof(int));

 /* Fill the heap to start with */

 for(i=0;i<nfiles;i++)
   {
    int index;

    datap[i]=data+i*itemsize;

    ReadFile(fds[i],datap[i],itemsize);

    heap[i]=i;

    index=i;

    /* Bubble up the new value */

    while(index>0 &&
          compare(datap[heap[index]],datap[heap[(index-1)/2]])<0)
      {
       int newindex;
       int temp;

       newindex=(index-1)/2;

       temp=heap[index];
       heap[index]=heap[newindex];
       heap[newindex]=temp;

       index=newindex;
      }
   }

 /* Repeatedly pull out the root of the heap and refill from the same file */

 ndata=nfiles;

 do
   {
    int index=0;

    if(!buildindex || buildindex(datap[heap[0]],count))
      {
       WriteFile(fd_out,datap[heap[0]],itemsize);
       count++;
      }

    if(ReadFile(fds[heap[0]],datap[heap[0]],itemsize))
      {
       ndata--;
       heap[0]=heap[ndata];
      }

    /* Bubble down the new value */

    while((2*index+2)<ndata &&
          (compare(datap[heap[index]],datap[heap[2*index+1]])>0 ||
           compare(datap[heap[index]],datap[heap[2*index+2]])>0))
      {
       int newindex;
       int temp;

       if(compare(datap[heap[2*index+1]],datap[heap[2*index+2]])<0)
          newindex=2*index+1;
       else
          newindex=2*index+2;

       temp=heap[newindex];
       heap[newindex]=heap[index];
       heap[index]=temp;

       index=newindex;
      }

    if((2*index+2)==ndata &&
       compare(datap[heap[index]],datap[heap[2*index+1]])>0)
      {
       int newindex;
       int temp;

       newindex=2*index+1;

       temp=heap[newindex];
       heap[newindex]=heap[index];
       heap[index]=temp;
      }
   }
 while(ndata>0);

 /* Tidy up */

 for(i=0;i<nfiles;i++)
    CloseFile(fds[i]);

 free(heap);
 free(data);
 free(datap);
 free(filename);
}


/*++++++++++++++++++++++++++++++++++++++
  A function to sort an array of pointers efficiently.

  The data is sorted using a "Heap sort" http://en.wikipedia.org/wiki/Heapsort,
  in particular an this good because it can operate in-place and doesn't
  allocate more memory like using qsort() does.

  void **datap A pointer to the array of pointers to sort.

  size_t nitems The number of items of data to sort.

  int(*compare)(const void *, const void *) The comparison function (identical to qsort if the
                                            data to be sorted was an array of things not pointers).
  ++++++++++++++++++++++++++++++++++++++*/

void heapsort(void **datap,size_t nitems,int(*compare)(const void*, const void*))
{
 int i;

 /* Fill the heap by pretending to insert the data that is already there */

 for(i=1;i<nitems;i++)
   {
    int index=i;

    /* Bubble up the new value (upside-down, put largest at top) */

    while(index>0 &&
          compare(datap[index],datap[(index-1)/2])>0) /* reversed compared to filesort() above */
      {
       int newindex;
       void *temp;

       newindex=(index-1)/2;

       temp=datap[index];
       datap[index]=datap[newindex];
       datap[newindex]=temp;

       index=newindex;
      }
   }

 /* Repeatedly pull out the root of the heap and swap with the bottom item */

 for(i=nitems-1;i>0;i--)
   {
    int index=0;
    void *temp;

    temp=datap[index];
    datap[index]=datap[i];
    datap[i]=temp;

    /* Bubble down the new value (upside-down, put largest at top) */

    while((2*index+2)<i &&
          (compare(datap[index],datap[2*index+1])<0 || /* reversed compared to filesort() above */
           compare(datap[index],datap[2*index+2])<0))  /* reversed compared to filesort() above */
      {
       int newindex;
       void *temp;

       if(compare(datap[2*index+1],datap[2*index+2])>0) /* reversed compared to filesort() above */
          newindex=2*index+1;
       else
          newindex=2*index+2;

       temp=datap[newindex];
       datap[newindex]=datap[index];
       datap[index]=temp;

       index=newindex;
      }

    if((2*index+2)==i &&
       compare(datap[index],datap[2*index+1])<0) /* reversed compared to filesort() above */
      {
       int newindex;
       void *temp;

       newindex=2*index+1;

       temp=datap[newindex];
       datap[newindex]=datap[index];
       datap[index]=temp;
      }
   }
}
