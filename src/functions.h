/***************************************
 $Header: /home/amb/CVS/routino/src/functions.h,v 1.26 2009-02-01 17:11:07 amb Exp $

 Header file for function prototypes
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef FUNCTIONS_H
#define FUNCTIONS_H    /*+ To stop multiple inclusions. +*/

#include <stdio.h>

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "profiles.h"
#include "results.h"


/* In osmparser.c */

int ParseXML(FILE *file,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,Profile *profile);


/* In files.c */

void *MapFile(const char *filename);

int OpenFile(const char *filename);
int WriteFile(int fd,void *address,size_t length);
void CloseFile(int fd);


/* In optimiser.c */

Results *FindRoute(Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile,int all);
Results *FindRoute3(Nodes *supernodes,Segments *supersegments,Ways *superways,index_t start,index_t finish,Results *begin,Results *end,Profile *profile);

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile);

Results *FindRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t start,Profile *profile);
Results *FindReverseRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t finish,Profile *profile);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile);


/* Functions in supersegments.c */

void ChooseSuperNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,int iteration);

SegmentsX *CreateSuperSegments(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,int iteration);

Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,WayX *match,int iteration);


#endif /* FUNCTIONS_H */
