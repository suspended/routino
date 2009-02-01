/***************************************
 $Header: /home/amb/CVS/routino/src/files.c,v 1.3 2009-02-01 17:11:07 amb Exp $

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

 return(address);
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


/*++++++++++++++++++++++++++++++++++++++
  Write data to a file on disk.

  int WriteFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor to write to.

  void *address The address of the data to be written.

  size_t length The length of data to write.
  ++++++++++++++++++++++++++++++++++++++*/

int WriteFile(int fd,void *address,size_t length)
{
 /* Write the data */

 if(write(fd,address,length)!=length)
    return(-1);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Close a file on disk.

  int fd The file descriptor to close.
  ++++++++++++++++++++++++++++++++++++++*/

void CloseFile(int fd)
{
 close(fd);
}
