/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.54 2009-09-06 15:51:09 amb Exp $

 OSM planet file splitter.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008,2009 Andrew M. Bishop

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typesx.h"
#include "types.h"
#include "functionsx.h"
#include "functions.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "superx.h"
#include "profiles.h"
#include "ways.h"


/*+ The option to use a slim mode with file-backed read-only intermediate storage. +*/
int option_slim=0;

/*+ The name of the temporary directory. +*/
char *tmpdirname=NULL;

int main(int argc,char** argv)
{
 NodesX *Nodes;
 SegmentsX *Segments,*SuperSegments=NULL,*MergedSegments=NULL;
 WaysX *Ways;
 int iteration=0,quit=0;
 int max_iterations=10;
 char *dirname=NULL,*prefix=NULL;
 Profile profile={0};
 int i;

 /* Fill in the default profile. */

 profile.transport=Transport_None; /* Not used by planetsplitter */

 profile.allow=Allow_ALL;

 for(i=1;i<Way_Unknown;i++)
    profile.highway[i]=1;

 profile.oneway=1; /* Not used by planetsplitter */

 /* Parse the command line arguments */

 while(--argc>=1)
   {
    if(!strcmp(argv[argc],"--help"))
       goto usage;
    else if(!strcmp(argv[argc],"--slim"))
       option_slim=1;
    else if(!strncmp(argv[argc],"--dir=",6))
       dirname=&argv[argc][6];
    else if(!strncmp(argv[argc],"--tmpdir=",9))
       tmpdirname=&argv[argc][9];
    else if(!strncmp(argv[argc],"--prefix=",9))
       prefix=&argv[argc][9];
    else if(!strncmp(argv[argc],"--max-iterations=",17))
       max_iterations=atoi(&argv[argc][17]);
    else if(!strncmp(argv[argc],"--transport=",12))
      {
       profile.transport=TransportType(&argv[argc][12]);
       profile.allow=1<<(profile.transport-1);
      }
    else if(!strncmp(argv[argc],"--not-highway=",14))
      {
       Highway highway=HighwayType(&argv[argc][14]);
       profile.highway[highway]=0;
      }
    else
      {
      usage:

       fprintf(stderr,"Usage: planetsplitter\n"
                      "                      [--help]\n"
                      "                      [--slim] [--tmpdir=<name>]\n"
                      "                      [--dir=<name>] [--prefix=<name>]\n"
                      "                      [--max-iterations=<number>]\n"
                      "                      [--transport=<transport>]\n"
                      "                      [--not-highway=<highway> ...]\n"
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

 if(!tmpdirname)
   {
    if(!dirname)
       tmpdirname=".";
    else
       tmpdirname=dirname;
   }

 /* Create new variables */

 Nodes=NewNodeList();
 Segments=NewSegmentList();
 Ways=NewWayList();

 /* Parse the file */

 printf("\nParse OSM Data\n==============\n\n");
 fflush(stdout);

 ParseXML(stdin,Nodes,Segments,Ways,&profile);

 /* Process the data */

 printf("\nProcess OSM Data\n================\n\n");
 fflush(stdout);

 /* Sort the ways */

 SortWayList(Ways);

 /* Sort the segments (first time) */

 InitialSortSegmentList(Segments);

 /* Sort the nodes (first time) */

 InitialSortNodeList(Nodes);

 /* Compact the ways */

 CompactWays(Ways);

 /* Remove bad segments (must be after sorting the nodes) */

 RemoveBadSegments(Nodes,Segments);

 /* Sort the segments (final time) */

 FinalSortSegmentList(Segments);

 /* Remove non-highway nodes (must be after removing the bad segments) */

 RemoveNonHighwayNodes(Nodes,Segments);

 /* Sort the nodes (final time) */

 FinalSortNodeList(Nodes);

 /* Measure the segments (must be after final sorting of the nodes) */

 MeasureSegments(Segments,Nodes);


 /* Repeated iteration on Super-Nodes, Super-Segments and Super-Ways */

 do
   {
    printf("\nProcess Super-Data (iteration %d)\n================================%s\n\n",iteration,iteration>9?"=":"");
    fflush(stdout);

    if(iteration==0)
      {
       /* Select the super-nodes */

       ChooseSuperNodes(Nodes,Segments,Ways);

       /* Select the super-segments */

       SuperSegments=CreateSuperSegments(Nodes,Segments,Ways,iteration);
      }
    else
      {
       SegmentsX *SuperSegments2;

       /* Select the super-nodes */

       ChooseSuperNodes(Nodes,SuperSegments,Ways);

       /* Select the super-segments */

       SuperSegments2=CreateSuperSegments(Nodes,SuperSegments,Ways,iteration);

       if(SuperSegments->number==SuperSegments2->number)
          quit=1;

       FreeSegmentList(SuperSegments);

       SuperSegments=SuperSegments2;
      }

    /* Sort the super-segments (first time) */

    InitialSortSegmentList(SuperSegments);

    /* Remove duplicated super-segments */

    DeduplicateSegments(SuperSegments,Ways);

    /* Sort the super-segments (final time) */

    FinalSortSegmentList(SuperSegments);

    iteration++;

    if(iteration>max_iterations)
       quit=1;
   }
 while(!quit);

 /* Combine the super-segments */

 printf("\nCombine Segments and Super-Segments\n===================================\n\n");
 fflush(stdout);

 /* Merge the super-segments */

 MergedSegments=MergeSuperSegments(Segments,SuperSegments);

 FreeSegmentList(Segments);

 FreeSegmentList(SuperSegments);

 Segments=MergedSegments;

 /* Sort the merged segments (thoroughly) */

 InitialSortSegmentList(Segments);
 FinalSortSegmentList(Segments);

 /* Cross reference the nodes and segments */

 printf("\nCross-Reference Nodes and Segments\n==================================\n\n");
 fflush(stdout);

 /* Sort the node list geographically */

 SortNodeListGeographically(Nodes);

 /* Create the real segments and nodes */

 CreateRealNodes(Nodes,iteration);

 CreateRealSegments(Segments,Ways);

 /* Fix the segment and node indexes */

 IndexNodes(Nodes,Segments);

 IndexSegments(Segments,Nodes);

 /* Output the results */

 printf("\nWrite Out Database Files\n========================\n\n");
 fflush(stdout);

 /* Write out the nodes */

 SaveNodeList(Nodes,FileName(dirname,prefix,"nodes.mem"));

 FreeNodeList(Nodes);

 /* Write out the segments */

 SaveSegmentList(Segments,FileName(dirname,prefix,"segments.mem"));

 FreeSegmentList(Segments);

 /* Write out the ways */

 SaveWayList(Ways,FileName(dirname,prefix,"ways.mem"));

 FreeWayList(Ways);

 return(0);
}
