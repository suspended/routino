/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.45 2009-07-09 18:56:50 amb Exp $

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


int main(int argc,char** argv)
{
 NodesX *OSMNodes;
 SegmentsX *OSMSegments,*SuperSegments=NULL;
 WaysX *OSMWays;
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
    else if(!strncmp(argv[argc],"--dir=",6))
       dirname=&argv[argc][6];
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

 /* Create new variables */

 OSMNodes=NewNodeList();
 OSMSegments=NewSegmentList();
 OSMWays=NewWayList();

 /* Parse the file */

 printf("\nParsing OSM Data\n================\n\n"); fflush(stdout);

 ParseXML(stdin,OSMNodes,OSMSegments,OSMWays,&profile);

 printf("\nProcessing OSM Data\n===================\n\n"); fflush(stdout);

 /* Sort the ways */

 SortWayList(OSMWays);

 /* Sort the segments */

 SortSegmentList(OSMSegments);

 /* Sort the nodes */

 SortNodeList(OSMNodes);

 /* Compact the ways */

 CompactWays(OSMWays);

 /* Remove bad segments */

 RemoveBadSegments(OSMNodes,OSMSegments);

 SortSegmentList(OSMSegments);

 /* Remove non-way nodes */

 RemoveNonHighwayNodes(OSMNodes,OSMSegments);

 SortNodeList(OSMNodes);

 /* Measure the segments */

 MeasureSegments(OSMSegments,OSMNodes);


 /* Repeated iteration on Super-Nodes, Super-Segments and Super-Ways */

 do
   {
    printf("\nProcessing Super-Data (iteration %d)\n===================================%s\n\n",iteration,iteration>10?"=":""); fflush(stdout);

    if(iteration==0)
      {
       /* Select the super-nodes */

       ChooseSuperNodes(OSMNodes,OSMSegments,OSMWays);

       /* Select the super-segments */

       SuperSegments=CreateSuperSegments(OSMNodes,OSMSegments,OSMWays,iteration);
      }
    else
      {
       SegmentsX *SuperSegments2;

       /* Select the super-nodes */

       ChooseSuperNodes(OSMNodes,SuperSegments,OSMWays);

       /* Select the super-segments */

       SuperSegments2=CreateSuperSegments(OSMNodes,SuperSegments,OSMWays,iteration);

       if(SuperSegments->number==SuperSegments2->number)
          quit=1;

       FreeSegmentList(SuperSegments);

       SuperSegments=SuperSegments2;
      }

    /* Sort the super-segments */

    SortSegmentList(SuperSegments);

    iteration++;

    if(iteration>max_iterations)
       quit=1;
   }
 while(!quit);

 printf("\n"); fflush(stdout);

 /* Merge the super-segments */

 MergeSuperSegments(OSMSegments,SuperSegments);

 FreeSegmentList(SuperSegments);

 SortSegmentList(OSMSegments);

 /* Rotate segments so that node1<node2 */

 RotateSegments(OSMSegments);

 SortSegmentList(OSMSegments);

 /* Remove duplicated segments */

 DeduplicateSegments(OSMSegments,OSMWays);

 SortSegmentList(OSMSegments);

 /* Sort the node list geographically */

 SortNodeListGeographically(OSMNodes);

 /* Create the real segments and nodes */

 CreateRealNodes(OSMNodes,iteration);

 CreateRealSegments(OSMSegments,OSMWays);

 /* Fix the segment and node indexes */

 IndexNodes(OSMNodes,OSMSegments);

 IndexSegments(OSMSegments,OSMNodes);

 /* Write out the nodes */

 SaveNodeList(OSMNodes,FileName(dirname,prefix,"nodes.mem"));

 /* Write out the segments */

 SaveSegmentList(OSMSegments,FileName(dirname,prefix,"segments.mem"));

 /* Write out the ways */

 SaveWayList(OSMWays,FileName(dirname,prefix,"ways.mem"));

 return(0);
}
