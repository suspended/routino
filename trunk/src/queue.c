/***************************************
 $Header: /home/amb/CVS/routino/src/queue.c,v 1.1 2009-07-23 17:34:59 amb Exp $

 Queue data type functions.

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


#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "results.h"

/*+ The size of the increment for the Queue data structure. +*/
#define QUEUE_INCREMENT   10240


/*+ A queue of results. +*/
typedef struct _Queue
{
 uint32_t  alloced;             /*+ The amount of space allocated for results in the array. +*/
 uint32_t  number;              /*+ The number of occupied results in the array. +*/

 Result  **xqueue;              /*+ An array of pointers to parts of the results structure. +*/
}
 Queue;

/*+ The queue of nodes. +*/
static Queue queue={0,0,NULL};


/*++++++++++++++++++++++++++++++++++++++
  Insert an item into the queue in the right order.

  Result *result The result to insert into the queue.
  ++++++++++++++++++++++++++++++++++++++*/

void InsertInQueue(Result *result)
{
 int start=0;
 int end=queue.number-1;
 int mid;
 int insert=-1;

 /* Check that the array is allocated. */

 if(!queue.xqueue)
   {
    queue.alloced=QUEUE_INCREMENT;
    queue.number=0;
    queue.xqueue=(Result**)malloc(queue.alloced*sizeof(Result*));
   }

 /* Check that the arrays have enough space. */

 if(queue.number==queue.alloced)
   {
    queue.alloced+=QUEUE_INCREMENT;
    queue.xqueue=(Result**)realloc((void*)queue.xqueue,queue.alloced*sizeof(Result*));
   }

 /* Binary search - search key may not match, new insertion point required
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since there may not be an exact match we must set end=mid
  *  # <- mid    |  or start=mid because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually end=start+1 and the insertion point is before
  *  # <- end    |  end (since it cannot be before the initial start or end).
  */

 if(queue.number==0)                                 /* There is nothing in the queue */
    insert=0;
 else if(result->sortby>queue.xqueue[start]->sortby) /* Check key is not before start */
    insert=start;
 else if(result->sortby<queue.xqueue[end]->sortby)   /* Check key is not after end */
    insert=end+1;
 else if(queue.number==2)                            /* Must be between them */
    insert=1;
 else
   {
    do
      {
       mid=(start+end)/2;                                /* Choose mid point */

       if(queue.xqueue[mid]->sortby>result->sortby)      /* Mid point is too low */
          start=mid;
       else if(queue.xqueue[mid]->sortby<result->sortby) /* Mid point is too high */
          end=mid;
       else                                              /* Mid point is correct */
         {
          if(queue.xqueue[mid]==result)
             return;

          insert=mid;
          break;
         }
      }
    while((end-start)>1);

    if(insert==-1)
       insert=end;
   }

 /* Shuffle the array up */

 if(insert!=queue.number)
    memmove(&queue.xqueue[insert+1],&queue.xqueue[insert],(queue.number-insert)*sizeof(Result*));

 /* Insert the new entry */

 queue.xqueue[insert]=result;

 queue.number++;
}


/*++++++++++++++++++++++++++++++++++++++
  Pop an item from the end of the queue.

  Result *PopFromQueue Returns the top item.

  Results *results The set of results that the queue is processing.
  ++++++++++++++++++++++++++++++++++++++*/

Result *PopFromQueue()
{
 if(queue.number)
    return(queue.xqueue[--queue.number]);
 else
    return(NULL);
}
