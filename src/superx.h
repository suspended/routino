/***************************************
 $Header: /home/amb/CVS/routino/src/superx.h,v 1.1 2009-02-07 15:57:42 amb Exp $

 Header for super-node and super-segment functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef SUPERX_H
#define SUPERX_H    /*+ To stop multiple inclusions. +*/

#include "types.h"
#include "results.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"


void ChooseSuperNodes(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,int iteration);

SegmentsX *CreateSuperSegments(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,int iteration);

void MergeSuperSegments(SegmentsX* segmentsx,SegmentsX* supersegmentsx);

Results *FindRoutesWay(NodesX *nodesx,SegmentsX *segmentsx,WaysX *waysx,node_t start,WayX *match,int iteration);


#endif /* SUPERX_H */
