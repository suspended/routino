/***************************************
 $Header: /home/amb/CVS/routino/src/files.c,v 1.4 2009-04-08 16:54:34 amb Exp $

 Functions to map a file into memory.

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
