/***************************************
 $Header: /home/amb/CVS/routino/src/functions.h,v 1.38 2009-09-03 18:35:47 amb Exp $

 Header file for function prototypes

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


#ifndef FUNCTIONS_H
#define FUNCTIONS_H    /*+ To stop multiple inclusions. +*/

#include <stdio.h>

#include "types.h"
#include "profiles.h"
#include "results.h"


/* In files.c */

char *FileName(const char *dirname,const char *prefix, const char *name);

void *MapFile(const char *filename);
void UnmapFile(const char *filename,int delete);

int OpenFile(const char *filename);
int WriteFile(int fd,void *address,size_t length);
void CloseFile(int fd);


/* In optimiser.c */

Results *FindNormalRoute(Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile);
Results *FindMiddleRoute(Nodes *supernodes,Segments *supersegments,Ways *superways,Results *begin,Results *end,Profile *profile);

Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t start,Profile *profile);
Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t finish,Profile *profile);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile);


/* In output.c */

void PrintRouteHead(const char *copyright);
void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile);
void PrintRouteTail(void);


#endif /* FUNCTIONS_H */
