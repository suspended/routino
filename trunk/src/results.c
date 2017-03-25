/***************************************
 Result data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2015, 2017 Andrew M. Bishop

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


#include <string.h>
#include <stdlib.h>

#include "results.h"

#include "logging.h"


#define HASH_NODE_SEGMENT(node,segment) ((node)^(segment<<4))


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *NewResultsList Returns the results list.

  uint8_t log2bins The base 2 logarithm of the initial number of bins in the results array.
  ++++++++++++++++++++++++++++++++++++++*/

Results *NewResultsList(uint8_t log2bins)
{
 Results *results;

 results=(Results*)malloc(sizeof(Results));

 results->nbins=1<<log2bins;
 results->mask=results->nbins-1;

 results->number=0;

 results->point=(Result**)calloc(results->nbins,sizeof(Result*));

#ifndef LIBROUTINO
 log_malloc(results->point,results->nbins*sizeof(Result*));
#endif

 results->ndata1=0;
 results->nallocdata1=0;
 results->ndata2=results->nbins>>2;

 results->data=NULL;

 results->start_node=NO_NODE;
 results->prev_segment=NO_SEGMENT;

 results->finish_node=NO_NODE;
 results->last_segment=NO_SEGMENT;

 results->start_waypoint=NO_WAYPOINT;
 results->finish_waypoint=NO_WAYPOINT;

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *results The results list to be reset.
  ++++++++++++++++++++++++++++++++++++++*/

void ResetResultsList(Results *results)
{
 uint32_t i;

 results->number=0;
 results->ndata1=0;

 for(i=0;i<results->nbins;i++)
    results->point[i]=NULL;

 results->start_node=NO_NODE;
 results->prev_segment=NO_SEGMENT;

 results->finish_node=NO_NODE;
 results->last_segment=NO_SEGMENT;
}


/*++++++++++++++++++++++++++++++++++++++
  Free a results list.

  Results *results The results list to be destroyed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeResultsList(Results *results)
{
 uint32_t i;

 for(i=0;i<results->nallocdata1;i++)
   {
#ifndef LIBROUTINO
    log_free(results->data[i]);
#endif
    free(results->data[i]);
   }

 free(results->data);

#ifndef LIBROUTINO
 log_free(results->point);
#endif
 free(results->point);

 free(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Insert a single entry into the hashed list.

  Results *results The results structure to insert into.

  Result *result The result to insert.

  index_t node The node that is to be inserted into the results.

  index_t segment The segment that is to be inserted into the results.
  ++++++++++++++++++++++++++++++++++++++*/

static inline void insert_result(Results *results,Result *result,index_t node,index_t segment)
{
 uint32_t bin=HASH_NODE_SEGMENT(node,segment)&results->mask;

 while(1)
   {
    Result *r=results->point[bin];

    if(!r)
       break;

    bin=(bin+1)%results->nbins;
   }

 results->point[bin]=result;
}


/*++++++++++++++++++++++++++++++++++++++
  Insert a new result into the results data structure in the right order.

  Result *InsertResult Returns the result that has been inserted.

  Results *results The results structure to insert into.

  index_t node The node that is to be inserted into the results.

  index_t segment The segment that is to be inserted into the results.
  ++++++++++++++++++++++++++++++++++++++*/

Result *InsertResult(Results *results,index_t node,index_t segment)
{
 Result *result;

 /* Check if we have hit the limit on the number of entries */

 if(results->number==(results->nbins/2))
   {
    uint32_t i,j;

#ifndef LIBROUTINO
    log_free(results->point);
#endif

    free(results->point);

    results->nbins<<=1;
    results->mask=results->nbins-1;

    results->point=(Result**)calloc(results->nbins,sizeof(Result*));

#ifndef LIBROUTINO
    log_malloc(results->point,results->nbins*sizeof(Result*));
#endif

    for(i=0;i<results->ndata1;i++)
       for(j=0;j<results->ndata2;j++)
         {
          if(i==(results->ndata1-1) && j==(results->number%results->ndata2))
             break;

          result=&results->data[i][j];

          insert_result(results,result,result->node,result->segment);
         }
   }

 /* Check if we need more data space allocated */

 if((results->number%results->ndata2)==0)
   {
    results->ndata1++;

    if(results->ndata1>=results->nallocdata1)
      {
       results->nallocdata1++;
       results->data=(Result**)realloc((void*)results->data,results->nallocdata1*sizeof(Result*));
       results->data[results->nallocdata1-1]=(Result*)malloc(results->ndata2*sizeof(Result));
#ifndef LIBROUTINO
       log_malloc(results->data[results->nallocdata1-1],results->ndata2*sizeof(Result));
#endif
      }
   }

 /* Insert the new entry */

 result=&results->data[results->ndata1-1][results->number%results->ndata2];

 insert_result(results,result,node,segment);

 results->number++;

 /* Initialise the result */

 result->node=node;
 result->segment=segment;

 result->prev=NULL;
 result->next=NULL;

 result->score=0;
 result->sortby=0;

 result->queued=NOT_QUEUED;

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Find a result; search by node and segment.

  Result *FindResult Returns the result that has been found.

  Results *results The results structure to search.

  index_t node The node that is to be found.

  index_t segment The segment that was used to reach this node.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FindResult(Results *results,index_t node,index_t segment)
{
 uint32_t bin=HASH_NODE_SEGMENT(node,segment)&results->mask;

 while(1)
   {
    Result *r=results->point[bin];

    if(!r)
       break;

    if(r->segment==segment && r->node==node)
       return(r);

    bin=(bin+1)%results->nbins;
   }

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the first result from a set of results.

  Result *FirstResult Returns the first result.

  Results *results The set of results.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FirstResult(Results *results)
{
 return(&results->data[0][0]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the next result from a set of results.

  Result *NextResult Returns the next result.

  Results *results The set of results.

  Result *result The previous result.
  ++++++++++++++++++++++++++++++++++++++*/

Result *NextResult(Results *results,Result *result)
{
 uint32_t i;
 size_t j=0;

 for(i=0;i<results->ndata1;i++)
    if(result>=results->data[i])
      {
       j=result-results->data[i];

       if(j<results->ndata2)
          break;
      }

 if(++j>=results->ndata2)
   {i++;j=0;}

 if((i*results->ndata2+j)>=results->number)
    return(NULL);

 return(&results->data[i][j]);
}
