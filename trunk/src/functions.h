/***************************************
 $Header: /home/amb/CVS/routino/src/functions.h,v 1.13 2009-01-19 19:51:42 amb Exp $

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

#include "nodes.h"
#include "ways.h"
#include "segments.h"
#include "results.h"


/* In osmparser.c */

int ParseXML(FILE *file,NodesMem *OSMNodes,SegmentsMem *OSMSegments,WaysMem *OSMWays);


/* In files.c */

void *MapFile(const char *filename);

void UnMapFile(void *address);

int WriteFile(const char *filename,void *address,size_t length);


/* In optimiser.c */

Results *FindRoute(Nodes *nodes,Segments *segments,Ways *ways,node_t start,node_t finish,wayallow_t transport);
Results *FindRoute3(Nodes *supernodes,Segments *supersegments,Ways *superways,node_t start,node_t finish,Results *begin,Results *end,wayallow_t transport);

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes,node_t start,node_t finish);

Results *FindRoutes(Nodes *nodes,Segments *segments,Ways *ways,node_t start,Nodes *finish,wayallow_t transport);
Results *FindReverseRoutes(Nodes *nodes,Segments *segments,Ways *ways,Nodes *start,node_t finish,wayallow_t transport);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,node_t start,node_t finish,wayallow_t transport);

Results *FindRoutesWay(Nodes *nodes,Segments *segments,Ways *ways,node_t start,Nodes *finish,Way *match,int iteration);


/* Functions in supersegments.c */

NodesMem *ChooseSuperNodes(Nodes *nodes,Segments *segments,Ways *ways);

SegmentsMem *CreateSuperSegments(Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes,int iteration);

WaysMem *CreateSuperWays(Ways *ways,SegmentsMem *supersegments);


#endif /* FUNCTIONS_H */
