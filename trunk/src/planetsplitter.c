/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.14 2009-01-23 17:09:41 amb Exp $

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
#include <string.h>

#include "nodes.h"
#include "ways.h"
#include "segments.h"
#include "functions.h"


int main(int argc,char** argv)
{
 NodesMem *OSMNodesMem;
 SegmentsMem *OSMSegmentsMem;
 WaysMem *OSMWaysMem;
 NodesMem *SuperNodesMem,*SuperNodesMem2;
 SegmentsMem *SuperSegmentsMem,*SuperSegmentsMem2;
 WaysMem *SuperWaysMem,*SuperWaysMem2;
 Nodes *OSMNodes;
 Segments *OSMSegments;
 Ways *OSMWays;
 Nodes *SuperNodes,*SuperNodes2;
 Segments *SuperSegments,*SuperSegments2;
 Ways *SuperWays,*SuperWays2;
 int iteration=0,quit=0;
 int skip_parsing=0,max_iterations=5;
 Transport transport;
 int highways[Way_Unknown+1];
 int i;

 /* Parse the command line arguments */

 for(i=0;i<Way_Unknown;i++)
    highways[i]=1;

 highways[Way_Unknown]=0;

 while(--argc>=1)
   {
    if(!strcmp(argv[argc],"-help"))
       goto usage;
    else if(!strcmp(argv[argc],"-skip-parsing"))
       skip_parsing=1;
    else if(!strncmp(argv[argc],"-max-iterations=",16))
       max_iterations=atoi(&argv[argc][16]);
    else if(!strncmp(argv[argc],"-transport=",11))
       transport=TransportType(&argv[argc][11]);
    else if(!strncmp(argv[argc],"-not-highway=",13))
      {
       Highway highway=HighwayType(&argv[argc][13]);
       highways[highway]=0;
      }
    else
      {
      usage:

       fprintf(stderr,"Usage: planetsplitter\n"
                      "                      [-help]\n"
                      "                      [-skip-parsing]\n"
                      "                      [-max-iterations=<number>]\n"
                      "                      [-transport=<transport>]\n"
                      "                      [-not-highway=<highway> ...]\n"
                      "\n"
                      "<transport> can be:\n"
                      "%s"
                      "\n"
                      "<highway> can be:\n"
                      "%s",
                      TransportList(),HighwayList());

       return(1);
      }
   }

 if(!skip_parsing)
   {
    /* Create new variables */

    OSMNodesMem=NewNodeList();
    OSMSegmentsMem=NewSegmentList();
    OSMWaysMem=NewWayList();

    /* Parse the file */

    ParseXML(stdin,OSMNodesMem,OSMSegmentsMem,OSMWaysMem,transport,highways);

    /* Sort the variables */

    printf("Sorting All Nodes"); fflush(stdout);
    SortNodeList(OSMNodesMem);
    printf("\rSorted All Nodes \n"); fflush(stdout);

    printf("Sorting Segments"); fflush(stdout);
    SortSegmentList(OSMSegmentsMem);
    printf("\rSorted Segments \n"); fflush(stdout);

    printf("Sorting Ways"); fflush(stdout);
    SortWayList(OSMWaysMem);
    printf("\rSorted Ways \n"); fflush(stdout);

    /* Write out all the nodes */

    printf("Saving All Nodes"); fflush(stdout);
    OSMNodes=SaveNodeList(OSMNodesMem,"data/all-nodes.mem");
    printf("\rSaved All Nodes: %d\n",OSMNodes->number); fflush(stdout);

    /* Remove bad segments */

    RemoveBadSegments(OSMSegmentsMem);

    printf("Sorting Segments"); fflush(stdout);
    SortSegmentList(OSMSegmentsMem);
    printf("\rSorted Segments \n"); fflush(stdout);

    /* Fix the segment lengths */

    FixupSegmentLengths(OSMSegmentsMem,OSMNodes,OSMWays);

    /* Write out the segments */

    printf("Saving Segments"); fflush(stdout);
    OSMSegments=SaveSegmentList(OSMSegmentsMem,"data/segments.mem");
    printf("\rSaved Segments: %d\n",OSMSegments->number); fflush(stdout);

    /* Write out the ways */

    printf("Saving Ways"); fflush(stdout);
    OSMWays=SaveWayList(OSMWaysMem,"data/ways.mem");
    printf("\rSaved Ways: %d\n",OSMWays->number); fflush(stdout);

    /* Remove non-way nodes */

    OSMNodesMem=OnlyHighwayNodes(OSMNodes,OSMSegments);

    UnMapFile(OSMNodes);

    /* Sort the nodes */

    printf("Sorting Way Nodes"); fflush(stdout);
    SortNodeList(OSMNodesMem);
    printf("\rSorted Way Nodes \n"); fflush(stdout);

    /* Write out the nodes */

    printf("Saving Way Nodes"); fflush(stdout);
    OSMNodes=SaveNodeList(OSMNodesMem,"data/nodes.mem");
    printf("\rSaved Way Nodes: %d\n",OSMNodes->number); fflush(stdout);
   }
else
  {
   /* Load in the data */

   OSMNodes=LoadNodeList("data/nodes.mem");
   OSMSegments=LoadSegmentList("data/segments.mem");
   OSMWays=LoadWayList("data/ways.mem");
  }

 /* Select the super-nodes */

 SuperNodesMem=ChooseSuperNodes(OSMNodes,OSMSegments,OSMWays);

 /* Sort the super-nodes */

 printf("Sorting Super-Nodes [iteration 0]"); fflush(stdout);
 SortNodeList(SuperNodesMem);
 printf("\rSorted Super-Nodes [iteration 0] \n"); fflush(stdout);

 /* Write out the super-nodes */

 printf("Saving Super-Nodes [iteration 0]"); fflush(stdout);
 SuperNodes=SaveNodeList(SuperNodesMem,"data/super-nodes.mem");
 printf("\rSaved Super-Nodes [iteration 0]: %d\n",SuperNodes->number); fflush(stdout);

 /* Select the super-segments */

 SuperSegmentsMem=CreateSuperSegments(OSMNodes,OSMSegments,OSMWays,SuperNodes,iteration);

 /* Sort the super-segments */

 printf("Sorting Super-Segments [iteration 0]"); fflush(stdout);
 SortSegmentList(SuperSegmentsMem);
 printf("\rSorted Super-Segments [iteration 0] \n"); fflush(stdout);

 /* Select the super-ways */

 SuperWaysMem=CreateSuperWays(OSMWays,SuperSegmentsMem);

 /* Write out the super-segments */

 printf("Saving Super-Segments [iteration 0]"); fflush(stdout);
 SuperSegments=SaveSegmentList(SuperSegmentsMem,"data/super-segments.mem");
 printf("\rSaved Super-Segments [iteration 0]: %d\n",SuperSegments->number); fflush(stdout);

 /* Sort the super-ways */

 printf("Sorting Super-Ways [iteration 0]"); fflush(stdout);
 SortWayList(SuperWaysMem);
 printf("\rSorted Super-Ways [iteration 0] \n"); fflush(stdout);

 /* Write out the super-ways */

 printf("Saving Super-Ways [iteration 0]"); fflush(stdout);
 SuperWays=SaveWayList(SuperWaysMem,"data/super-ways.mem");
 printf("\rSaved Super-Ways [iteration 0]: %d\n",SuperWays->number); fflush(stdout);

 /* Repeated iteration on Super-Nodes, Super-Segments and Super-Ways */

 UnMapFile(SuperNodes);
 UnMapFile(SuperSegments);
 UnMapFile(SuperWays);

 do
   {
    iteration++;

    if(iteration>max_iterations)
       break;

    /* Load the previous iteration */

    SuperNodes=LoadNodeList("data/super-nodes.mem");
    SuperWays=LoadWayList("data/super-ways.mem");
    SuperSegments=LoadSegmentList("data/super-segments.mem");

    /* Select the super-nodes */

    SuperNodesMem2=ChooseSuperNodes(SuperNodes,SuperSegments,SuperWays);

    /* Sort the super-nodes */

    printf("Sorting Super-Nodes [iteration %d]",iteration); fflush(stdout);
    SortNodeList(SuperNodesMem2);
    printf("\rSorted Super-Nodes [iteration %d] \n",iteration); fflush(stdout);

    /* Write out the super-nodes */

    printf("Saving Super-Nodes [iteration %d]",iteration); fflush(stdout);
    SuperNodes2=SaveNodeList(SuperNodesMem2,"data/super-nodes2.mem");
    printf("\rSaved Super-Nodes [iteration %d]: %d\n",iteration,SuperNodes2->number); fflush(stdout);

    /* Select the super-segments */

    SuperSegmentsMem2=CreateSuperSegments(SuperNodes,SuperSegments,SuperWays,SuperNodes2,iteration);

    /* Sort the super-segments */

    printf("Sorting Super-Segments [iteration %d]",iteration); fflush(stdout);
    SortSegmentList(SuperSegmentsMem2);
    printf("\rSorted Super-Segments [iteration %d] \n",iteration); fflush(stdout);

    /* Select the super-ways */

    SuperWaysMem2=CreateSuperWays(SuperWays,SuperSegmentsMem2);

    /* Write out the super-segments */

    printf("Saving Super-Segments [iteration %d]",iteration); fflush(stdout);
    SuperSegments2=SaveSegmentList(SuperSegmentsMem2,"data/super-segments2.mem");
    printf("\rSaved Super-Segments [iteration %d]: %d\n",iteration,SuperSegments2->number); fflush(stdout);

    /* Sort the super-ways */

    printf("Sorting Super-Ways [iteration %d]",iteration); fflush(stdout);
    SortWayList(SuperWaysMem2);
    printf("\rSorted Super-Ways [iteration %d] \n",iteration); fflush(stdout);

    /* Write out the super-ways */

    printf("Saving Super-Ways [iteration %d]",iteration); fflush(stdout);
    SuperWays2=SaveWayList(SuperWaysMem2,"data/super-ways2.mem");
    printf("\rSaved Super-Ways [iteration %d]: %d\n",iteration,SuperWays2->number); fflush(stdout);

    /* Decide when to quit */

    quit=0;
    if(SuperNodes2->number>(0.95*SuperNodes->number))       quit=1;
    if(SuperSegments2->number>(0.95*SuperSegments->number)) quit=1;

    /* Unmap the data and Rename the files */

    UnMapFile(SuperNodes);
    UnMapFile(SuperSegments);
    UnMapFile(SuperWays);

    UnMapFile(SuperNodes2);
    UnMapFile(SuperSegments2);
    UnMapFile(SuperWays2);

    rename("data/super-nodes2.mem","data/super-nodes.mem");
    rename("data/super-segments2.mem","data/super-segments.mem");
    rename("data/super-ways2.mem","data/super-ways.mem");
   }
 while(!quit);

 UnMapFile(OSMNodes);
 UnMapFile(OSMSegments);
 UnMapFile(OSMWays);

 return(0);
}
