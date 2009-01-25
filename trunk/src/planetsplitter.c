/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.18 2009-01-25 10:58:51 amb Exp $

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
#include "segments.h"
#include "ways.h"
#include "profiles.h"
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
 int help_profile=0,skip_parsing=0,max_iterations=5;
 Profile profile;
 int i;

 /* Fill in the default profile. */

 profile.transport=Transport_None; /* Not used by planetsplitter */

 profile.allow=Allow_ALL;

 for(i=1;i<Way_Unknown;i++)
    profile.highways[i]=1;

 for(i=1;i<Way_Unknown;i++)
    profile.speed[i]=0; /* Not used by planetsplitter */

 profile.oneway=1; /* Not used by planetsplitter */

 /* Parse the command line arguments */

 while(--argc>=1)
   {
    if(!strcmp(argv[argc],"-help"))
       goto usage;
    if(!strcmp(argv[argc],"-help-profile"))
       help_profile=1;
    else if(!strcmp(argv[argc],"-skip-parsing"))
       skip_parsing=1;
    else if(!strncmp(argv[argc],"-max-iterations=",16))
       max_iterations=atoi(&argv[argc][16]);
    else if(!strncmp(argv[argc],"-transport=",11))
      {
       profile.transport=TransportType(&argv[argc][11]);
       profile.allow=1<<(profile.transport-1);
      }
    else if(!strncmp(argv[argc],"-not-highway=",13))
      {
       Highway highway=HighwayType(&argv[argc][13]);
       profile.highways[highway]=0;
      }
    else
      {
      usage:

       fprintf(stderr,"Usage: planetsplitter\n"
                      "                      [-help] [-help-profile]\n"
                      "                      [-skip-parsing]\n"
                      "                      [-max-iterations=<number>]\n"
                      "                      [-transport=<transport>]\n"
                      "                      [-not-highway=<highway> ...]\n"
                      "\n"
                      "<transport> defaults to all but can be set to:\n"
                      "%s"
                      "\n"
                      "<highway> can be selected from:\n"
                      "%s",
                      TransportList(),HighwayList());

       return(1);
      }
   }

 if(help_profile)
   {
    PrintProfile(&profile);

    return(0);
   }

 /* Parse the file (or not) */

 if(!skip_parsing)
   {
    /* Create new variables */

    OSMNodesMem=NewNodeList();
    OSMSegmentsMem=NewSegmentList();
    OSMWaysMem=NewWayList();

    /* Parse the file */

    printf("\nParsing OSM Data\n================\n\n"); fflush(stdout);

    ParseXML(stdin,OSMNodesMem,OSMSegmentsMem,OSMWaysMem,&profile);

    printf("\nProcessing OSM Data\n===================\n\n"); fflush(stdout);

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

 printf("\nProcessing Super-Data\n=====================\n\n"); fflush(stdout);

 /* Select the super-nodes */

 SuperNodesMem=ChooseSuperNodes(OSMNodes,OSMSegments,OSMWays);

 /* Sort the super-nodes */

 printf("Sorting Super-Nodes"); fflush(stdout);
 SortNodeList(SuperNodesMem);
 printf("\rSorted Super-Nodes \n"); fflush(stdout);

 /* Write out the super-nodes */

 printf("Saving Super-Nodes"); fflush(stdout);
 SuperNodes=SaveNodeList(SuperNodesMem,"data/super-nodes.mem");
 printf("\rSaved Super-Nodes: %d\n",SuperNodes->number); fflush(stdout);

 /* Select the super-segments */

 SuperSegmentsMem=CreateSuperSegments(OSMNodes,OSMSegments,OSMWays,SuperNodes);

 /* Sort the super-segments */

 printf("Sorting Super-Segments"); fflush(stdout);
 SortSegmentList(SuperSegmentsMem);
 printf("\rSorted Super-Segments \n"); fflush(stdout);

 /* Select the super-ways */

 SuperWaysMem=CreateSuperWays(OSMWays,SuperSegmentsMem);

 /* Write out the super-segments */

 printf("Saving Super-Segments"); fflush(stdout);
 SuperSegments=SaveSegmentList(SuperSegmentsMem,"data/super-segments.mem");
 printf("\rSaved Super-Segments: %d\n",SuperSegments->number); fflush(stdout);

 /* Sort the super-ways */

 printf("Sorting Super-Ways"); fflush(stdout);
 SortWayList(SuperWaysMem);
 printf("\rSorted Super-Ways \n"); fflush(stdout);

 /* Write out the super-ways */

 printf("Saving Super-Ways"); fflush(stdout);
 SuperWays=SaveWayList(SuperWaysMem,"data/super-ways.mem");
 printf("\rSaved Super-Ways: %d\n",SuperWays->number); fflush(stdout);

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

    printf("\nProcessing Super-Data (iteration %d)\n===================================%s\n\n",iteration,iteration>10?"=":""); fflush(stdout);

    /* Select the super-nodes */

    SuperNodesMem2=ChooseSuperNodes(SuperNodes,SuperSegments,SuperWays);

    /* Sort the super-nodes */

    printf("Sorting Super-Nodes"); fflush(stdout);
    SortNodeList(SuperNodesMem2);
    printf("\rSorted Super-Nodes \n"); fflush(stdout);

    /* Write out the super-nodes */

    printf("Saving Super-Nodes"); fflush(stdout);
    SuperNodes2=SaveNodeList(SuperNodesMem2,"data/super-nodes2.mem");
    printf("\rSaved Super-Nodes: %d\n",SuperNodes2->number); fflush(stdout);

    /* Select the super-segments */

    SuperSegmentsMem2=CreateSuperSegments(SuperNodes,SuperSegments,SuperWays,SuperNodes2);

    /* Sort the super-segments */

    printf("Sorting Super-Segments"); fflush(stdout);
    SortSegmentList(SuperSegmentsMem2);
    printf("\rSorted Super-Segments \n"); fflush(stdout);

    /* Select the super-ways */

    SuperWaysMem2=CreateSuperWays(SuperWays,SuperSegmentsMem2);

    /* Write out the super-segments */

    printf("Saving Super-Segments"); fflush(stdout);
    SuperSegments2=SaveSegmentList(SuperSegmentsMem2,"data/super-segments2.mem");
    printf("\rSaved Super-Segments: %d\n",SuperSegments2->number); fflush(stdout);

    /* Sort the super-ways */

    printf("Sorting Super-Ways"); fflush(stdout);
    SortWayList(SuperWaysMem2);
    printf("\rSorted Super-Ways \n"); fflush(stdout);

    /* Write out the super-ways */

    printf("Saving Super-Ways"); fflush(stdout);
    SuperWays2=SaveWayList(SuperWaysMem2,"data/super-ways2.mem");
    printf("\rSaved Super-Ways: %d\n",SuperWays2->number); fflush(stdout);

    /* Decide when to quit */

    quit=0;
    if(SuperNodes2->number>(0.999*SuperNodes->number))       quit=1;
    if(SuperSegments2->number>(0.999*SuperSegments->number)) quit=1;

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
