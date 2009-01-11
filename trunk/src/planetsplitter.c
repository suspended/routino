/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.8 2009-01-11 09:42:26 amb Exp $

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


#define SKIP_PARSING 0


int main(int argc,char** argv)
{
#if !SKIP_PARSING
 NodesMem *OSMNodesMem;
 WaysMem *OSMWaysMem;
 SegmentsMem *OSMSegmentsMem;
#endif
 NodesMem *SuperNodesMem;
 SegmentsMem *SuperSegmentsMem;
 Nodes *OSMNodes;
 Ways *OSMWays;
 Segments *OSMSegments;
 Nodes *SuperNodes;
 Segments *SuperSegments;

#if !SKIP_PARSING

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

#else

 /* Load in the data */

 OSMNodes=LoadNodeList("data/nodes.mem");
 OSMWays=LoadWayList("data/ways.mem");
 OSMSegments=LoadSegmentList("data/segments.mem");

#endif

 /* Select the super-nodes */

 SuperNodesMem=ChooseSuperNodes(OSMNodes,OSMSegments,OSMWays);

 /* Sort the super-nodes */

 printf("Sorting Super-Nodes"); fflush(stdout);
 SortNodeList(SuperNodesMem);
 printf("\rSorted Super-Nodes \n"); fflush(stdout);

 /* Write out the variables */

 printf("Saving Super-Nodes"); fflush(stdout);
 SuperNodes=SaveNodeList(SuperNodesMem,"data/super-nodes.mem");
 printf("\rSaved Super-Nodes \n"); fflush(stdout);

 /* Select the super-segments */

 SuperSegmentsMem=CreateSuperSegments(OSMNodes,OSMSegments,OSMWays,SuperNodes);

 /* Sort the super-segments */

 printf("Sorting Super-Segments"); fflush(stdout);
 SortSegmentList(SuperSegmentsMem);
 printf("\rSorted Super-Segments \n"); fflush(stdout);

 /* Write out the variables */

 printf("Saving Super-Segments"); fflush(stdout);
 SuperSegments=SaveSegmentList(SuperSegmentsMem,"data/super-segments.mem");
 printf("\rSaved Super-Segments \n"); fflush(stdout);

 return(0);
}
