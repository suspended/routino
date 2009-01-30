/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.24 2009-01-30 19:56:59 amb Exp $

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
 SegmentsMem *OSMSegmentsMem,*SuperSegmentsMem=NULL;
 WaysMem *OSMWaysMem;
 int iteration=0,quit=0;
 int help_profile=0,max_iterations=5;
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

 /* Create new variables */

 OSMNodesMem=NewNodeList();
 OSMSegmentsMem=NewSegmentList();
 OSMWaysMem=NewWayList();

 /* Parse the file */

 printf("\nParsing OSM Data\n================\n\n"); fflush(stdout);

 ParseXML(stdin,OSMNodesMem,OSMSegmentsMem,OSMWaysMem,&profile);

 printf("\nProcessing OSM Data\n===================\n\n"); fflush(stdout);

 /* Sort the ways */

 printf("Sorting Ways"); fflush(stdout);
 SortWayList(OSMWaysMem);
 printf("\rSorted Ways \n"); fflush(stdout);

 /* Sort the segments */

 printf("Sorting Segments"); fflush(stdout);
 SortSegmentList(OSMSegmentsMem);
 printf("\rSorted Segments \n"); fflush(stdout);

 /* Remove bad segments */

 RemoveBadSegments(OSMSegmentsMem);

 printf("Sorting Segments"); fflush(stdout);
 SortSegmentList(OSMSegmentsMem);
 printf("\rSorted Segments \n"); fflush(stdout);

 /* Remove non-way nodes */

 RemoveNonHighwayNodes(OSMNodesMem,OSMSegmentsMem);

 printf("Sorting Nodes"); fflush(stdout);
 SortNodeList(OSMNodesMem);
 printf("\rSorted Nodes \n"); fflush(stdout);

 /* Measure the segments */

 MeasureSegments(OSMSegmentsMem,OSMNodesMem);


 /* Repeated iteration on Super-Nodes, Super-Segments and Super-Ways */

 do
   {
    printf("\nProcessing Super-Data (iteration %d)\n===================================%s\n\n",iteration,iteration>10?"=":""); fflush(stdout);

    if(iteration==0)
      {
       /* Select the super-nodes */

       ChooseSuperNodes(OSMNodesMem,OSMSegmentsMem,OSMWaysMem,iteration);

       /* Select the super-segments */

       SuperSegmentsMem=CreateSuperSegments(OSMNodesMem,OSMSegmentsMem,OSMWaysMem,iteration);
      }
    else
      {
       SegmentsMem *SuperSegmentsMem2;

       /* Select the super-nodes */

       ChooseSuperNodes(OSMNodesMem,SuperSegmentsMem,OSMWaysMem,iteration);

       /* Select the super-segments */

       SuperSegmentsMem2=CreateSuperSegments(OSMNodesMem,SuperSegmentsMem,OSMWaysMem,iteration);

       FreeSegmentList(SuperSegmentsMem);

       SuperSegmentsMem=SuperSegmentsMem2;
      }

    /* Sort the super-segments */

    printf("Sorting Super-Segments"); fflush(stdout);
    SortSegmentList(SuperSegmentsMem);
    printf("\rSorted Super-Segments \n"); fflush(stdout);

    iteration++;

    if(iteration>max_iterations)
       quit=1;
   }
 while(!quit);


 /* Fix the node indexes */

 FixupSegments(OSMSegmentsMem,OSMNodesMem,SuperSegmentsMem);

 FreeSegmentList(SuperSegmentsMem);

 /* Sort the segments */

 printf("Sorting Segments"); fflush(stdout);
 SortSegmentList(OSMSegmentsMem);
 printf("\rSorted Segments \n"); fflush(stdout);

 /* Fix the segment indexes */

 FixupNodes(OSMNodesMem,OSMSegmentsMem,iteration);

 /* Write out the nodes */

 printf("Saving Nodes"); fflush(stdout);
 SaveNodeList(OSMNodesMem,"data/nodes.mem");
 printf("\rSaved Nodes: %d\n",OSMNodesMem->number); fflush(stdout);

 /* Write out the segments */

 printf("Saving Segments"); fflush(stdout);
 SaveSegmentList(OSMSegmentsMem,"data/segments.mem");
 printf("\rSaved Segments: %d\n",OSMSegmentsMem->number); fflush(stdout);

 /* Write out the ways */

 printf("Saving Ways"); fflush(stdout);
 SaveWayList(OSMWaysMem,"data/ways.mem");
 printf("\rSaved Ways: %d\n",OSMWaysMem->number); fflush(stdout);

 return(0);
}
