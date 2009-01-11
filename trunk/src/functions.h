/***************************************
 $Header: /home/amb/CVS/routino/src/functions.h,v 1.9 2009-01-11 09:42:26 amb Exp $

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

Results *FindRoute(Nodes *nodes,Segments *segments,node_t start,node_t finish);

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,node_t start,node_t finish);

Results *FindRoutes(Nodes *nodes,Segments *segments,node_t start,Nodes *finish);

void PrintRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes,Segments *supersegments,node_t start,node_t finish);


#endif /* FUNCTIONS_H */
