/***************************************
 Functions to handle logging functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2014 Andrew M. Bishop

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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "logging.h"


/* Global variables */

/*+ The option to print the output in a way that allows logging to a file. +*/
int option_loggable=0;

/*+ The option to print timestamps with the output. +*/
int option_logtime=0;

/*+ The option to print timestamps with the output. +*/
int option_logmemory=0;


/*+ A structure to contain the list of allocated memory. +*/
struct mallocinfo
{
 void  *address;            /*+ The address of the allocated memory. +*/
 size_t size;               /*+ The size of the allocated memory. +*/
};

/*+ The list of allocated memory. +*/
static struct mallocinfo *mallocedmem;

/*+ The number of allocated memory blocks. +*/
static int nmallocedmem=0;


/* Local functions */

static void vfprintf_first(FILE *file,const char *format,va_list ap);
static void vfprintf_middle(FILE *file,const char *format,va_list ap);
static void vfprintf_last(FILE *file,const char *format,va_list ap);

static void fprintf_elapsed_time(FILE *file,struct timeval *start);
static void fprintf_max_memory(FILE *file,size_t max_alloc,size_t max_mmap);


/* Local variables */

/*+ The time that program_start() was called. +*/
static struct timeval program_start_time;

/*+ The time that printf_first() was called. +*/
static struct timeval function_start_time;

/*+ The length of the string printed out last time. +*/
static int printed_length=0;

/*+ The maximum amount of memory allocated and memory mapped since starting the program. +*/
static size_t program_max_alloc=0,program_max_mmap=0;

/*+ The maximum amount of memory allocated and memory mapped since starting the function. +*/
static size_t function_max_alloc=0,function_max_mmap=0;

/*+ The current amount of memory allocated and memory mapped. +*/
static size_t current_alloc=0,current_mmap=0;


/*++++++++++++++++++++++++++++++++++++++
  Record the time that the program started.
  ++++++++++++++++++++++++++++++++++++++*/

void printf_program_start(void)
{
 gettimeofday(&program_start_time,NULL);

 program_max_alloc=program_max_mmap=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Record the time that the program started.
  ++++++++++++++++++++++++++++++++++++++*/

void printf_program_end(void)
{
 if(option_logtime || option_logmemory)
   {
    printf("\n");

    if(option_logtime)
       fprintf_elapsed_time(stdout,&program_start_time);

    if(option_logmemory)
       fprintf_max_memory(stdout,program_max_alloc,program_max_mmap);

    printf("Complete\n");
    fflush(stdout);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Print the first message in an overwriting sequence (to stdout).

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void printf_first(const char *format, ...)
{
 va_list ap;

 if(option_logtime)
    gettimeofday(&function_start_time,NULL);

 if(option_logmemory)
   {
    function_max_alloc=current_alloc;
    function_max_mmap=current_mmap;
   }

 if(option_loggable)
    return;

 va_start(ap,format);

 vfprintf_first(stdout,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the middle message in an overwriting sequence (to stdout).

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void printf_middle(const char *format, ...)
{
 va_list ap;

 if(option_loggable)
    return;

 va_start(ap,format);

 vfprintf_middle(stdout,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the last message in an overwriting sequence (to stdout).

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void printf_last(const char *format, ...)
{
 va_list ap;

 va_start(ap,format);

 vfprintf_last(stdout,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the first message in an overwriting sequence to a specified file.

  FILE *file The file to write to.

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void fprintf_first(FILE *file,const char *format, ...)
{
 va_list ap;

 if(option_logtime)
    gettimeofday(&function_start_time,NULL);

 if(option_logmemory)
    function_max_alloc=function_max_mmap=0;

 if(option_loggable)
    return;

 va_start(ap,format);

 vfprintf_first(file,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the middle message in an overwriting sequence to a specified file.

  FILE *file The file to write to.

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void fprintf_middle(FILE *file,const char *format, ...)
{
 va_list ap;

 if(option_loggable)
    return;

 va_start(ap,format);

 vfprintf_middle(file,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the last message in an overwriting sequence to a specified file.

  FILE *file The file to write to.

  const char *format The format string.

  ... The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

void fprintf_last(FILE *file,const char *format, ...)
{
 va_list ap;

 va_start(ap,format);

 vfprintf_last(file,format,ap);

 va_end(ap);
}


/*++++++++++++++++++++++++++++++++++++++
  Record the memory allocations (record the amount in use).

  void *address The address that has been allocated.

  size_t size The size of the memory that has been allocated.
  ++++++++++++++++++++++++++++++++++++++*/

void log_malloc(void *address,size_t size)
{
 int i;

 if(!option_logmemory)
    return;

 /* Store the information about the allocated memory */

 for(i=0;i<nmallocedmem;i++)
    if(mallocedmem[i].address==address)
      {
       size=size-mallocedmem[i].size;
       mallocedmem[i].size+=size;
       break;
      }

 if(i==nmallocedmem)
   {
    mallocedmem=(struct mallocinfo*)realloc((void*)mallocedmem,(nmallocedmem+1)*sizeof(struct mallocinfo));

    mallocedmem[nmallocedmem].address=address;
    mallocedmem[nmallocedmem].size=size;

    nmallocedmem++;
   }

 /* Increase the sum of allocated memory */

 current_alloc+=size;

 if(current_alloc>function_max_alloc)
    function_max_alloc=current_alloc;

 if(current_alloc>program_max_alloc)
    program_max_alloc=current_alloc;
}


/*++++++++++++++++++++++++++++++++++++++
  Record the memory de-allocations.

  void *address The address that has been freed.
  ++++++++++++++++++++++++++++++++++++++*/

void log_free(void *address)
{
 size_t size=0;
 int i;

 if(!option_logmemory)
    return;

 /* Remove the information about the allocated memory */

 for(i=0;i<nmallocedmem;i++)
    if(mallocedmem[i].address==address)
      {
       size=mallocedmem[i].size;
       break;
      }

 logassert(i!=nmallocedmem,"Memory freed with log_free() was not allocated with log_[cm]alloc()");

 nmallocedmem--;

 if(nmallocedmem>i)
    memmove(&mallocedmem[i],&mallocedmem[i+1],(nmallocedmem-i)*sizeof(struct mallocinfo));

 /* Reduce the sum of allocated memory */

 current_alloc-=size;
}


/*++++++++++++++++++++++++++++++++++++++
  Record the amount of memory that has been mapped into files.

  size_t size The size of the file that has been mapped.
  ++++++++++++++++++++++++++++++++++++++*/

void log_mmap(size_t size)
{
 if(!option_logmemory)
    return;

 current_mmap+=size;

 if(current_mmap>function_max_mmap)
    function_max_mmap=current_mmap;

 if(current_mmap>program_max_mmap)
    program_max_mmap=current_mmap;
}


/*++++++++++++++++++++++++++++++++++++++
  Record the amount of memory that has been unmapped from files.

  size_t size The size of the file that has been unmapped.
  ++++++++++++++++++++++++++++++++++++++*/

void log_munmap(size_t size)
{
 if(!option_logmemory)
    return;

 current_mmap-=size;
}


/*++++++++++++++++++++++++++++++++++++++
  Do the work to print the first message in an overwriting sequence.

  FILE *file The file to write to.

  const char *format The format string.

  va_list ap The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static void vfprintf_first(FILE *file,const char *format,va_list ap)
{
 int retval;

 if(option_logtime)
    fprintf_elapsed_time(file,&function_start_time);

 if(option_logmemory)
    fprintf_max_memory(file,function_max_alloc,function_max_mmap);

 retval=vfprintf(file,format,ap);
 fflush(file);

 if(retval>0)
    printed_length=retval;
}


/*++++++++++++++++++++++++++++++++++++++
  Do the work to print the middle message in an overwriting sequence.

  FILE *file The file to write to.

  const char *format The format string.

  va_list ap The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static void vfprintf_middle(FILE *file,const char *format,va_list ap)
{
 int retval;

 fputc('\r',file);

 if(option_logtime)
    fprintf_elapsed_time(file,&function_start_time);

 if(option_logmemory)
    fprintf_max_memory(file,function_max_alloc,function_max_mmap);

 retval=vfprintf(file,format,ap);
 fflush(file);

 if(retval>0)
   {
    int new_printed_length=retval;

    while(retval++<printed_length)
       fputc(' ',file);

    printed_length=new_printed_length;
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Do the work to print the last message in an overwriting sequence.

  FILE *file The file to write to.

  const char *format The format string.

  va_list ap The other arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static void vfprintf_last(FILE *file,const char *format,va_list ap)
{
 int retval;

 if(!option_loggable)
    fputc('\r',file);

 if(option_logtime)
    fprintf_elapsed_time(file,&function_start_time);

 if(option_logmemory)
    fprintf_max_memory(file,function_max_alloc,function_max_mmap);

 retval=vfprintf(file,format,ap);

 if(retval>0)
    while(retval++<printed_length)
       fputc(' ',file);

 fputc('\n',file);
 fflush(file);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the elapsed time without a following newline.

  FILE *file The file to print to.

  struct timeval *start The start time from which the elapsed time is to be printed.
  ++++++++++++++++++++++++++++++++++++++*/

static void fprintf_elapsed_time(FILE *file,struct timeval *start)
{
 struct timeval finish,elapsed;

 gettimeofday(&finish,NULL);

 elapsed.tv_sec =finish.tv_sec -start->tv_sec;
 elapsed.tv_usec=finish.tv_usec-start->tv_usec;
 if(elapsed.tv_usec<0)
   {
    elapsed.tv_sec -=1;
    elapsed.tv_usec+=1000000;
   }

 fprintf(file,"[%2ld:%02ld.%03ld] ",elapsed.tv_sec/60,elapsed.tv_sec%60,elapsed.tv_usec/1000);
}


/*++++++++++++++++++++++++++++++++++++++
  Print the maximum used memory without a following newline.

  FILE *file The file to print to.

  size_t max_alloc The maximum amount of allocated memory.

  size_t max_mmap The maximum amount of memory mapped memory.
  ++++++++++++++++++++++++++++++++++++++*/

static void fprintf_max_memory(FILE *file,size_t max_alloc,size_t max_mmap)
{
 fprintf(file,"[%3d, %3d MB] ",max_alloc/(1024*1024),max_mmap/(1024*1024));
}


/*++++++++++++++++++++++++++++++++++++++
  Log a fatal error and exit

  const char *message The error message.

  const char *file The file in which the error occured.

  int line The line number in the file at which the error occured.
  ++++++++++++++++++++++++++++++++++++++*/

void _logassert(const char *message,const char *file,int line)
{
 fprintf(stderr,"Routino Fatal Error (%s:%d): %s\n",file,line,message);

 exit(EXIT_FAILURE);
}
