/***************************************
 Header file for file function prototypes

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2015 Andrew M. Bishop

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


#ifndef FILES_H
#define FILES_H    /*+ To stop multiple inclusions. +*/

/* If your system does not have the pread() and pwrite() system calls then you
 * will need to change this line to the value 0 so that seek() and
 * read()/write() are used instead of pread()/pwrite(). */

#if defined(_MSC_VER) || defined(__MINGW32__)
#define HAVE_PREAD_PWRITE 0
#else
#define HAVE_PREAD_PWRITE 1
#endif

#if defined(_MSC_VER)
#include <io.h>
#include <basetsd.h>
#define read(fd,address,length)  _read(fd,address,(unsigned int)(length))
#define write(fd,address,length) _write(fd,address,(unsigned int)(length))
#define ssize_t SSIZE_T
#else
#include <unistd.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#undef lseek
#define lseek _lseeki64
#endif

#include <sys/types.h>
#include <inttypes.h>

#include "logging.h"


/* Types */

/*+ A 64-bit file offset since a 32-bit off_t (which is signed) is smaller than a
    32-bit size_t (which is unsigned) that can be writtento or read from a file. +*/
typedef int64_t offset_t;


/* Functions in files.c */

char *FileName(const char *dirname,const char *prefix, const char *name);

void *MapFile(const char *filename);
void *MapFileWriteable(const char *filename);

void *UnmapFile(const void *address);

int SlimMapFile(const char *filename);
int SlimMapFileWriteable(const char *filename);

int SlimUnmapFile(int fd);

int OpenFileBufferedNew(const char *filename);
int OpenFileBufferedAppend(const char *filename);

int ReOpenFileBuffered(const char *filename);

int ReplaceFileBuffered(const char *filename,int *oldfd);

int WriteFileBuffered(int fd,const void *address,size_t length);
int ReadFileBuffered(int fd,void *address,size_t length);

int SeekFileBuffered(int fd,offset_t position);
int SkipFileBuffered(int fd,offset_t skip);

int CloseFileBuffered(int fd);

int OpenFile(const char *filename);

void CloseFile(int fd);

offset_t SizeFile(const char *filename);
offset_t SizeFileFD(int fd);
int ExistsFile(const char *filename);

int DeleteFile(const char *filename);

int RenameFile(const char *oldfilename,const char *newfilename);

/* Functions in files.h */

static inline int SlimReplace(int fd,const void *address,size_t length,offset_t position);
static inline int SlimFetch(int fd,void *address,size_t length,offset_t position);


/* Inline the frequently called functions */

/*++++++++++++++++++++++++++++++++++++++
  Write data to a file that has been opened for slim mode access.

  int SlimReplace Returns 0 if OK or something else in case of an error.

  int fd The file descriptor to write to.

  const void *address The address of the data to be written.

  size_t length The length of data to write.

  offset_t position The position in the file to seek to.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int SlimReplace(int fd,const void *address,size_t length,offset_t position)
{
 /* Seek and write the data */

#if HAVE_PREAD_PWRITE

 if(pwrite(fd,address,length,position)!=(ssize_t)length)
    return(-1);

#else

 if(lseek(fd,position,SEEK_SET)!=position)
    return(-1);

 if(write(fd,address,length)!=(ssize_t)length)
    return(-1);

#endif

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Read data from a file that has been opened for slim mode access.

  int SlimFetch Returns 0 if OK or something else in case of an error.

  int fd The file descriptor to read from.

  void *address The address the data is to be read into.

  size_t length The length of data to read.

  offset_t position The position in the file to seek to.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int SlimFetch(int fd,void *address,size_t length,offset_t position)
{
 /* Seek and read the data */

#if HAVE_PREAD_PWRITE

 if(pread(fd,address,length,position)!=(ssize_t)length)
    return(-1);

#else

 if(lseek(fd,position,SEEK_SET)!=position)
    return(-1);

 if(read(fd,address,length)!=(ssize_t)length)
    return(-1);

#endif

 return(0);
}


#endif /* FILES_H */
