/***************************************
 $Header: /home/amb/CVS/routino/src/functions.h,v 1.21 2009-01-27 18:22:37 amb Exp $

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

void PrintRoute(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes,uint32_t start,uint32_t finish,Profile *profile);

Results *FindRoutes(Nodes *nodes,Segments *segments,Ways *ways,uint32_t start,Nodes *finish,Profile *profile);
Results *FindReverseRoutes(Nodes *nodes,Segments *segments,Ways *ways,Nodes *start,uint32_t finish,Profile *profile);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,uint32_t start,uint32_t finish,Profile *profile);

Results *FindRoutesWay(Nodes *nodes,Segments *segments,Ways *ways,uint32_t start,Nodes *finish,Way *match);


/* Functions in supersegments.c */

NodesMem *ChooseSuperNodes(Nodes *nodes,Segments *segments,Ways *ways);

SegmentsMem *CreateSuperSegments(Nodes *nodes,Segments *segments,Ways *ways,Nodes *supernodes);

WaysMem *CreateSuperWays(Ways *ways,SegmentsMem *supersegments);


#endif /* FUNCTIONS_H */
