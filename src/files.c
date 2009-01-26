/***************************************
 $Header: /home/amb/CVS/routino/src/files.c,v 1.2 2009-01-26 18:47:22 amb Exp $

 Functions to map a file into memory.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "functions.h"

/*+ A structure to contain the file mapping between file descriptor and address. +*/
struct filemapping
{
 int fd;                        /*+ The file descriptor. +*/
 void *address;                 /*+ The address the file is mapped to. +*/
 size_t length;                 /*+ The length of the mapped file. +*/
};

/*+ The current set of file mappings. +*/
static struct filemapping *mappings=NULL;

/*+ The number of file mappings +*/
static int nmappings=0;


/*++++++++++++++++++++++++++++++++++++++
  Open a file and map it into memory.

  void *MapFile Returns the address of the file.

  const char *filename The name of the file to open.
  ++++++++++++++++++++++++++++++++++++++*/

void *MapFile(const char *filename)
{
 int fd;
 struct stat buf;
 void *address;

 /* Open the file */

 fd=open(filename,O_RDONLY);

 if(fd==-1)
    return(NULL);

 /* Get the length of the file */

 if(fstat(fd,&buf))
   {
    close(fd);
    return(NULL);
   }

 /* Map the file */

 address=mmap(NULL,buf.st_size,PROT_READ,MAP_PRIVATE,fd,0);

 if(!address)
   {
    close(fd);
    return(NULL);
   }

 /* Store the file mapping */

 nmappings++;
 mappings=(struct filemapping*)realloc((void*)mappings,nmappings*sizeof(struct filemapping));

 mappings[nmappings-1].fd=fd;
 mappings[nmappings-1].address=address;
 mappings[nmappings-1].length=buf.st_size;

 return(address);
}


/*++++++++++++++++++++++++++++++++++++++
  Unmap a file and close it.

  void *address The address of the mapped file.
  ++++++++++++++++++++++++++++++++++++++*/

void UnMapFile(void *address)
{
 int i;

 /* Search for the file mapping and unmap it and close it. */

 for(i=0;i<nmappings;i++)
   {
    if(address==mappings[i].address)
      {
       munmap(mappings[i].address,mappings[i].length);

       close(mappings[i].fd);

       mappings[i].fd=-1;
       mappings[i].address=NULL;
       mappings[i].length=0;

       break;
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Write a new file to disk.

  int WriteFile Returns 0 if OK or something else in case of an error.

  const char *filename The name of the file to create.

  void *address The address of the data to be written.

  size_t length The length of data to write.
  ++++++++++++++++++++++++++++++++++++++*/

int WriteFile(const char *filename,void *address,size_t length)
{
 int fd;

 /* Open the file */

 fd=open(filename,O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU|S_IRGRP|S_IROTH);

 if(fd==-1)
    return(-1);

 /* Write the data */

 if(write(fd,address,length)!=length)
    return(-1);

 /* Close the file */

 close(fd);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Open a new file on disk.

  int OpenFile Returns the file descriptor if OK or something negative else in case of an error.

  const char *filename The name of the file to create.
  ++++++++++++++++++++++++++++++++++++++*/

int OpenFile(const char *filename)
{
 int fd;

 /* Open the file */

 fd=open(filename,O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU|S_IRGRP|S_IROTH);

 if(fd<0)
    assert(0);

 return(fd);
}
