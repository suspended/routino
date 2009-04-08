/***************************************
 $Header: /home/amb/CVS/routino/src/functions.h,v 1.30 2009-04-08 16:54:34 amb Exp $

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

void *MapFile(const char *filename);

int OpenFile(const char *filename);
int WriteFile(int fd,void *address,size_t length);
void CloseFile(int fd);


/* In osmparser.c */

int ParseXML(FILE *file,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,Profile *profile);


/* In optimiser.c */

Results *FindRoute(Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile,int all);
Results *FindRoute3(Nodes *supernodes,Segments *supersegments,Ways *superways,index_t start,index_t finish,Results *begin,Results *end,Profile *profile);

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile);

Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t start,Profile *profile);
Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t finish,Profile *profile);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile);


#endif /* FUNCTIONS_H */
