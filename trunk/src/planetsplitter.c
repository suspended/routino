/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.6 2009-01-10 11:53:48 amb Exp $

 OSM planet file splitter.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008,2009 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdio.h>
#include <stdlib.h>

#include "nodes.h"
#include "ways.h"
#include "segments.h"
#include "functions.h"


int main(int argc,char** argv)
{
 NodesMem *OSMNodesMem;
 Nodes *OSMNodes;
 WaysMem *OSMWaysMem;
 Ways *OSMWays;
 SegmentsMem *OSMSegmentsMem;
 Segments *OSMSegments;
 SegmentsMem *SuperSegmentsMem;
 Segments *SuperSegments;

 /* Create new variables */

 OSMNodesMem=NewNodeList();
 OSMWaysMem=NewWayList();
 OSMSegmentsMem=NewSegmentList();

 /* Parse the file */

 ParseXML(stdin,OSMNodesMem,OSMSegmentsMem,OSMWaysMem);

 /* Sort the variables */

 printf("Sorting Nodes"); fflush(stdout);
 SortNodeList(OSMNodesMem);
 printf("\rSorted Nodes \n"); fflush(stdout);

 printf("Sorting Ways"); fflush(stdout);
 SortWayList(OSMWaysMem);
 printf("\rSorted Ways \n"); fflush(stdout);

 printf("Sorting Segments"); fflush(stdout);
 SortSegmentList(OSMSegmentsMem);
 printf("\rSorted Segments \n"); fflush(stdout);

 /* Write out the variables */

 printf("Saving Nodes"); fflush(stdout);
 OSMNodes=SaveNodeList(OSMNodesMem,"data/nodes.mem");
 printf("\rSaved Nodes \n"); fflush(stdout);

 printf("Saving Ways"); fflush(stdout);
 OSMWays=SaveWayList(OSMWaysMem,"data/ways.mem");
 printf("\rSaved Ways \n"); fflush(stdout);

 /* Fix the segment lengths */

 printf("Measuring Segments"); fflush(stdout);
 FixupSegmentLengths(OSMSegmentsMem,OSMNodes,OSMWays);
 printf("\rMeasured Segments \n"); fflush(stdout);

 /* Write out the variables */

 printf("Saving Segments"); fflush(stdout);
 OSMSegments=SaveSegmentList(OSMSegmentsMem,"data/segments.mem");
 printf("\rSaved Segments \n"); fflush(stdout);

 /* Create new variables */

 SuperSegmentsMem=NewSegmentList();

 /* Select the super-segments */

 printf("Selecting Super-Segments"); fflush(stdout);
 ChooseSuperSegments(SuperSegmentsMem,OSMNodes,OSMSegments,OSMWays);
 printf("\rSelected Super-Segments \n"); fflush(stdout);

 /* Sort the super-segments */

 printf("Sorting SuperSegments"); fflush(stdout);
 SortSegmentList(SuperSegmentsMem);
 printf("\rSorted SuperSegments \n"); fflush(stdout);

 /* Write out the variables */

 printf("Saving Super Segments"); fflush(stdout);
 SuperSegments=SaveSegmentList(SuperSegmentsMem,"data/super-segments.mem");
 printf("\rSaved Super Segments \n"); fflush(stdout);

 return(0);
}
