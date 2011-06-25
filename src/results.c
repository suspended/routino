/***************************************
 Result data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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

/*+ The size of the increment for the Results data structure. +*/
#define RESULTS_INCREMENT 64


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new results list.

  Results *NewResultsList Returns the results list.

  int nbins The number of bins in the results array.
  ++++++++++++++++++++++++++++++++++++++*/

Results *NewResultsList(int nbins)
{
 Results *results;
 uint32_t i;

 results=(Results*)malloc(sizeof(Results));

 results->nbins=1;
 results->mask=~0;

 while(nbins>>=1)
   {
    results->mask<<=1;
    results->nbins<<=1;
   }

 results->mask=~results->mask;

 results->alloced=RESULTS_INCREMENT;
 results->number=0;

 results->count=(uint32_t*)malloc(results->nbins*sizeof(uint32_t));
 results->point=(Result***)malloc(results->nbins*sizeof(Result**));

 for(i=0;i<results->nbins;i++)
   {
    results->count[i]=0;

    results->point[i]=(Result**)malloc(results->alloced*sizeof(Result*));
   }

 results->data=(Result**)malloc(1*sizeof(Result*));
 results->data[0]=(Result*)malloc(results->nbins*RESULTS_INCREMENT*sizeof(Result));

 results->start_node=NO_NODE;
 results->prev_segment=NO_SEGMENT;

 results->finish_node=NO_NODE;
 results->last_segment=NO_SEGMENT;

 return(results);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a results list.

  Results *results The results list to be destroyed.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeResultsList(Results *results)
{
 int i,c=(results->number-1)/(results->nbins*RESULTS_INCREMENT);

 for(i=c;i>=0;i--)
    free(results->data[i]);

 free(results->data);

 for(i=0;i<results->nbins;i++)
    free(results->point[i]);

 free(results->point);

 free(results->count);

 free(results);
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
 int bin=node&results->mask;
 uint32_t i;

 /* Check that the arrays have enough space or allocate more. */

 if(results->count[bin]==results->alloced)
   {
    results->alloced+=RESULTS_INCREMENT;

    for(i=0;i<results->nbins;i++)
       results->point[i]=(Result**)realloc((void*)results->point[i],results->alloced*sizeof(Result*));
   }

 if(results->number && (results->number%RESULTS_INCREMENT)==0 && (results->number%(RESULTS_INCREMENT*results->nbins))==0)
   {
    int c=results->number/(results->nbins*RESULTS_INCREMENT);

    results->data=(Result**)realloc((void*)results->data,(c+1)*sizeof(Result*));
    results->data[c]=(Result*)malloc(results->nbins*RESULTS_INCREMENT*sizeof(Result));
   }

 /* Insert the new entry */

 results->point[bin][results->count[bin]]=&results->data[results->number/(results->nbins*RESULTS_INCREMENT)][results->number%(results->nbins*RESULTS_INCREMENT)];

 results->number++;

 results->count[bin]++;

 /* Initialise the result */

 result=results->point[bin][results->count[bin]-1];

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
  Find a result; search by node only (don't care about the segment but find the shortest).

  Result *FindResult1 Returns the result that has been found.

  Results *results The results structure to search.

  index_t node The node that is to be found.
  ++++++++++++++++++++++++++++++++++++++*/

Result *FindResult1(Results *results,index_t node)
{
 int bin=node&results->mask;
 score_t best_score=INF_SCORE;
 Result *best_result=NULL;
 int i;

 for(i=results->count[bin]-1;i>=0;i--)
    if(results->point[bin][i]->node==node && results->point[bin][i]->score<best_score)
      {
       best_score=results->point[bin][i]->score;
       best_result=results->point[bin][i];
      }

 return(best_result);
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
 int bin=node&results->mask;
 int i;

 for(i=results->count[bin]-1;i>=0;i--)
    if(results->point[bin][i]->node==node && results->point[bin][i]->segment==segment)
       return(results->point[bin][i]);

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
 int i,j=0,c=(results->number-1)/(results->nbins*RESULTS_INCREMENT);

 for(i=0;i<=c;i++)
   {
    j=result-results->data[i];

    if(j>=0 && j<(results->nbins*RESULTS_INCREMENT))
       break;
   }

 if(++j>=(results->nbins*RESULTS_INCREMENT))
   {i++;j=0;}

 if((i*(results->nbins*RESULTS_INCREMENT)+j)>=results->number)
    return(NULL);

 return(&results->data[i][j]);
}
