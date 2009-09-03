/***************************************
 $Header: /home/amb/CVS/routino/src/files.c,v 1.9 2009-09-03 17:49:49 amb Exp $

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
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "functions.h"


struct mmapinfo
{
 const char  *filename;
       int    fd;
       void  *address;
       size_t length;
};

struct mmapinfo *mappedfiles;
int nmappedfiles=0;


/*++++++++++++++++++++++++++++++++++++++
  Return a filename composed of the dirname, prefix and filename.

  char *FileName Returns an allocated filename.

  const char *dirname The directory name.

  const char *prefix The file prefix.

  const char *name The filename.
  ++++++++++++++++++++++++++++++++++++++*/

char *FileName(const char *dirname,const char *prefix, const char *name)
{
 char *filename=(char*)malloc((dirname?strlen(dirname):0)+1+(prefix?strlen(prefix):0)+1+strlen(name));

 sprintf(filename,"%s%s%s%s%s",dirname?dirname:"",dirname?"/":"",prefix?prefix:"",prefix?"-":"",name);

 return(filename);
}


/*++++++++++++++++++++++++++++++++++++++
  Open a file read-only and map it into memory.

  void *MapFile Returns the address of the file.

  const char *filename The name of the file to open.

  size_t *length Returns the length of the file (in bytes).
  ++++++++++++++++++++++++++++++++++++++*/

void *MapFile(const char *filename,size_t *length)
{
 int fd;
 struct stat buf;
 void *address;

 /* Open the file */

 fd=open(filename,O_RDONLY);

 if(fd<0)
   {
    fprintf(stderr,"Cannot open file '%s' to read.\n",filename);
    exit(EXIT_FAILURE);
   }

 /* Get the length of the file */

 if(fstat(fd,&buf))
   {
    close(fd);

    fprintf(stderr,"Cannot stat file '%s'.\n",filename);
    exit(EXIT_FAILURE);
   }

 /* Map the file */

 address=mmap(NULL,buf.st_size,PROT_READ,MAP_SHARED,fd,0);

 if(address==MAP_FAILED)
   {
    close(fd);

    fprintf(stderr,"Cannot mmap file '%s'.\n",filename);
    exit(EXIT_FAILURE);
   }

 if(length)
    *length=buf.st_size;

 mappedfiles=(struct mmapinfo*)realloc((void*)mappedfiles,(nmappedfiles+1)*sizeof(struct mmapinfo));

 mappedfiles[nmappedfiles].filename=filename;
 mappedfiles[nmappedfiles].fd=fd;
 mappedfiles[nmappedfiles].address=address;
 mappedfiles[nmappedfiles].length=buf.st_size;

 nmappedfiles++;

 return(address);
}


/*++++++++++++++++++++++++++++++++++++++
  Unmap a file and optionally delete it.

  const char *filename The name of the file when it was opened.

  int delete If set then delete the file.
  ++++++++++++++++++++++++++++++++++++++*/

void UnmapFile(const char *filename,int delete)
{
 int i;

 for(i=0;i<nmappedfiles;i++)
    if(!strcmp(mappedfiles[i].filename,filename))
       break;

 if(i==nmappedfiles)
   {
    fprintf(stderr,"Cannot find file '%s' to unmap.\n",filename);
    exit(EXIT_FAILURE);
   }

 /* Close the file */

 close(mappedfiles[i].fd);

 /* Unmap the file */

 munmap(mappedfiles[i].address,mappedfiles[i].length);

 /* Delete the file */

 if(delete)
    unlink(filename);

 /* Shuffle the list of files */

 nmappedfiles--;

 if(nmappedfiles>i)
    memmove(&mappedfiles[i],&mappedfiles[i+1],(nmappedfiles-i)*sizeof(struct mmapinfo));
}


/*++++++++++++++++++++++++++++++++++++++
  Open a new file on disk for writing to.

  int OpenFile Returns the file descriptor if OK or something negative in case of an error.

  const char *filename The name of the file to create.
  ++++++++++++++++++++++++++++++++++++++*/

int OpenFile(const char *filename)
{
 int fd;

 /* Open the file */

 fd=open(filename,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

 if(fd<0)
   {
    fprintf(stderr,"Cannot open file '%s' to write.\n",filename);
    exit(EXIT_FAILURE);
   }

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
