/***************************************
 $Header: /home/amb/CVS/routino/src/relationsx.c,v 1.2 2010-09-19 14:06:54 amb Exp $

 Extended Relation data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010 Andrew M. Bishop

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


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "waysx.h"
#include "relationsx.h"

#include "files.h"
#include "functions.h"


/* Variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new relation list (create a new file or open an existing one).

  RelationsX *NewRelationList Returns the relation list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

RelationsX *NewRelationList(int append)
{
 RelationsX *relationsx;

 relationsx=(RelationsX*)calloc(1,sizeof(RelationsX));

 assert(relationsx); /* Check calloc() worked */

 relationsx->rfilename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(relationsx->rfilename,"%s/relationsx.route.input.tmp",option_tmpdirname);
 else
    sprintf(relationsx->rfilename,"%s/relationsx.route.%p.tmp",option_tmpdirname,relationsx);

 if(append)
   {
    off_t size,position=0;

    relationsx->rfd=AppendFile(relationsx->rfilename);

    size=SizeFile(relationsx->rfilename);

    while(position<size)
      {
       FILESORT_VARINT relationsize;

       SeekFile(relationsx->rfd,position);
       ReadFile(relationsx->rfd,&relationsize,FILESORT_VARSIZE);

       relationsx->rxnumber++;
       position+=relationsize+FILESORT_VARSIZE;
      }

    SeekFile(relationsx->rfd,size);
   }
 else
    relationsx->rfd=OpenFile(relationsx->rfilename);

 return(relationsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a relation list.

  RelationsX *relationsx The list to be freed.

  int keep Set to 1 if the file is to be kept.
  ++++++++++++++++++++++++++++++++++++++*/

void FreeRelationList(RelationsX *relationsx,int keep)
{
 if(!keep)
    DeleteFile(relationsx->rfilename);

 free(relationsx->rfilename);

 free(relationsx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single relation to an unsorted route relation list.

  RelationsX* relationsx The set of relations to process.

  relation_t id The ID of the relation.

  allow_t routes The types of routes that this relation is for.

  way_t *ways The array of ways that are members of the relation.

  int nways The number of ways that are members of the relation.

  relation_t *relations The array of relations that are members of the relation.

  int nrelations The number of relations that are members of the relation.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendRouteRelation(RelationsX* relationsx,relation_t id,allow_t routes,
                         way_t *ways,int nways,
                         relation_t *relations,int nrelations)
{
 RouteRelX relationx;
 FILESORT_VARINT size,padsize;
 way_t zeroway=0;
 relation_t zerorelation=0;
 void *zeropointer=NULL;

 relationx.id=id;
 relationx.routes=routes;

 size=sizeof(RouteRelX)+(nways+1)*sizeof(way_t)+(nrelations+1)*sizeof(relation_t);
 padsize=sizeof(RouteRelX*)*((size+sizeof(RouteRelX*)-1)/sizeof(RouteRelX*));

 WriteFile(relationsx->rfd,&padsize,FILESORT_VARSIZE);
 WriteFile(relationsx->rfd,&relationx,sizeof(RouteRelX));

 WriteFile(relationsx->rfd,ways    ,nways*sizeof(way_t));
 WriteFile(relationsx->rfd,&zeroway,      sizeof(way_t));

 WriteFile(relationsx->rfd,relations    ,nrelations*sizeof(relation_t));
 WriteFile(relationsx->rfd,&zerorelation,           sizeof(relation_t));

 WriteFile(relationsx->rfd,&zeropointer,padsize-size);

 relationsx->rxnumber++;

 assert(!(relationsx->rxnumber==0)); /* Zero marks the high-water mark for relations. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of relations.

  RelationsX* relationsx The set of relations to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortRelationList(RelationsX* relationsx)
{
 /* Don't need to sort route relations */
}


/*++++++++++++++++++++++++++++++++++++++
  Process the route relations and apply the information to the ways.

  RelationsX *relationsx The set of relations to process.

  WaysX *waysx The set of ways to update.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessRouteRelations(RelationsX *relationsx,WaysX *waysx)
{
 int i;

 /* Print the start message */

 printf("Processing Route Relations: Relations=0");
 fflush(stdout);

 /* Open the file and read through it */

 relationsx->rfd=ReOpenFile(relationsx->rfilename);

 for(i=0;i<relationsx->rnumber;i++)
   {
    FILESORT_VARINT padsize,size;
    RouteRelX relationx;
    way_t way;
    relation_t relation;
    void *pointer;

    ReadFile(relationsx->rfd,&padsize,FILESORT_VARSIZE);
    ReadFile(relationsx->rfd,&relationx,sizeof(RouteRelX));

    size=FILESORT_VARSIZE+sizeof(RouteRelX);

    do
      {
       ReadFile(relationsx->rfd,&way,sizeof(way_t));

//       if(way)
//          printf("Relation %d includes way %d\n",relationx.id,way);

       size+=sizeof(way_t);
      }
    while(way);

    do
      {
       ReadFile(relationsx->rfd,&relation,sizeof(relation_t));

//       if(relation)
//          printf("Relation %d includes relation %d\n",relationx.id,relation);

       size+=sizeof(relation_t);
      }
    while(relation);

    ReadFile(relationsx->rfd,&pointer,padsize-size);
   }

 CloseFile(relationsx->rfd);

 /* Print the final message */

 printf("\rProcessed Route Relations: Relations=%d  \n",relationsx->rnumber);
 fflush(stdout);
}
