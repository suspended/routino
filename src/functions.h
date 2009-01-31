/***************************************
 $Header: /home/amb/CVS/routino/src/functions.h,v 1.24 2009-01-31 14:53:28 amb Exp $

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
#include "segments.h"
#include "ways.h"
#include "profiles.h"
#include "results.h"


/* In osmparser.c */

int ParseXML(FILE *file,NodesMem *OSMNodes,SegmentsMem *OSMSegments,WaysMem *OSMWays,Profile *profile);


/* In files.c */

void *MapFile(const char *filename);

void UnMapFile(void *address);

int WriteFile(const char *filename,void *address,size_t length);

int OpenFile(const char *filename);


/* In optimiser.c */

Results *FindRoute(Nodes *nodes,Segments *segments,Ways *ways,uint32_t start,uint32_t finish,Profile *profile,int all);
Results *FindRoute3(Nodes *supernodes,Segments *supersegments,Ways *superways,uint32_t start,uint32_t finish,Results *begin,Results *end,Profile *profile);

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,uint32_t start,uint32_t finish,Profile *profile);

Results *FindRoutes(Nodes *nodes,Segments *segments,Ways *ways,uint32_t start,Profile *profile);
Results *FindReverseRoutes(Nodes *nodes,Segments *segments,Ways *ways,uint32_t finish,Profile *profile);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,uint32_t start,uint32_t finish,Profile *profile);

Results *FindRoutesWay(NodesMem *nodesmem,SegmentsMem *segmentsmem,WaysMem *waysmem,node_t start,WayEx *match,int iteration);


/* Functions in supersegments.c */

void ChooseSuperNodes(NodesMem *nodesmem,SegmentsMem *segmentsmem,WaysMem *waysmem,int iteration);

SegmentsMem *CreateSuperSegments(NodesMem *nodesmem,SegmentsMem *segmentsmem,WaysMem *waysmem,int iteration);


#endif /* FUNCTIONS_H */
