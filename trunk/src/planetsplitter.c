/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.10 2009-01-18 16:03:45 amb Exp $

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
 WaysMem *SuperWaysMem;
 Nodes *OSMNodes;
 Ways *OSMWays;
 Segments *OSMSegments;
 Nodes *SuperNodes;
 Segments *SuperSegments;
 Ways *SuperWays;

#if !SKIP_PARSING

 /* Create new variables */

 OSMNodesMem=NewNodeList();
 OSMWaysMem=NewWayList();
 OSMSegmentsMem=NewSegmentList();

 /* Parse the file */

 ParseXML(stdin,OSMNodesMem,OSMSegmentsMem,OSMWaysMem);

 /* Sort the variables */

 printf("Sorting All Nodes"); fflush(stdout);
 SortNodeList(OSMNodesMem);
 printf("\rSorted All Nodes \n"); fflush(stdout);

 printf("Sorting Ways"); fflush(stdout);
 SortWayList(OSMWaysMem);
 printf("\rSorted Ways \n"); fflush(stdout);

 printf("Sorting Segments"); fflush(stdout);
 SortSegmentList(OSMSegmentsMem);
 printf("\rSorted Segments \n"); fflush(stdout);

 /* Write out the variables */

 printf("Saving All Nodes"); fflush(stdout);
 OSMNodes=SaveNodeList(OSMNodesMem,"data/all-nodes.mem");
 printf("\rSaved All Nodes: %d\n",OSMNodes->number); fflush(stdout);

 printf("Saving Ways"); fflush(stdout);
 OSMWays=SaveWayList(OSMWaysMem,"data/ways.mem");
 printf("\rSaved Ways: %d\n",OSMWays->number); fflush(stdout);

 /* Remove bad segments */

 RemoveBadSegments(OSMSegmentsMem);

 printf("Sorting Segments"); fflush(stdout);
 SortSegmentList(OSMSegmentsMem);
 printf("\rSorted Segments \n"); fflush(stdout);

 /* Remove non-way nodes */

 OSMNodesMem=NewNodeList();

 RemoveNonWayNodes(OSMNodesMem,OSMNodes,OSMSegmentsMem->segments);

 UnMapFile(OSMNodes);

 printf("Sorting Way Nodes"); fflush(stdout);
 SortNodeList(OSMNodesMem);
 printf("\rSorted Way Nodes \n"); fflush(stdout);

 printf("Saving Way Nodes"); fflush(stdout);
 OSMNodes=SaveNodeList(OSMNodesMem,"data/nodes.mem");
 printf("\rSaved Way Nodes: %d\n",OSMNodes->number); fflush(stdout);

 /* Fix the segment lengths */

 FixupSegmentLengths(OSMSegmentsMem,OSMNodes,OSMWays);

 /* Write out the variables */

 printf("Saving Segments"); fflush(stdout);
 OSMSegments=SaveSegmentList(OSMSegmentsMem,"data/segments.mem");
 printf("\rSaved Segments: %d\n",OSMSegments->number); fflush(stdout);

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
 printf("\rSaved Super-Nodes: %d\n",SuperNodes->number); fflush(stdout);

 /* Select the super-segments */

 SuperSegmentsMem=CreateSuperSegments(OSMNodes,OSMSegments,OSMWays,SuperNodes);

 /* Sort the variables */

 printf("Sorting Super-Segments"); fflush(stdout);
 SortSegmentList(SuperSegmentsMem);
 printf("\rSorted Super-Segments \n"); fflush(stdout);

 /* Select the super-ways */

 SuperWaysMem=CreateSuperWays(OSMWays,SuperSegmentsMem);

 /* Sort the variables */

 printf("Sorting Super-Ways"); fflush(stdout);
 SortWayList(SuperWaysMem);
 printf("\rSorted Super-Ways \n"); fflush(stdout);

 /* Write out the variables */

 printf("Saving Super-Segments"); fflush(stdout);
 SuperSegments=SaveSegmentList(SuperSegmentsMem,"data/super-segments.mem");
 printf("\rSaved Super-Segments: %d\n",SuperSegments->number); fflush(stdout);

 printf("Saving Super-Ways"); fflush(stdout);
 SuperWays=SaveWayList(SuperWaysMem,"data/super-ways.mem");
 printf("\rSaved Super-Ways: %d\n",SuperWays->number); fflush(stdout);

 return(0);
}
