/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.4 2009-01-04 17:51:23 amb Exp $

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

#include "functions.h"
#include "types.h"


int main(int argc,char** argv)
{
 /* Parse the file */

 ParseXML(stdin);

 /* Sort the variables */

 printf("Sorting Nodes"); fflush(stdout);
 SortNodeList();
 printf("\rSorted Nodes \n"); fflush(stdout);

 printf("Sorting Ways"); fflush(stdout);
 SortWayList();
 printf("\rSorted Ways \n"); fflush(stdout);

 printf("Sorting Segments"); fflush(stdout);
 SortSegmentList();
 printf("\rSorted Segments \n"); fflush(stdout);

 /* Fix the segment lengths */

 printf("Measuring Segments"); fflush(stdout);
 FixupSegmentLengths();
 printf("\rMeasured Segments \n"); fflush(stdout);

 /* Write out the variables */

 printf("Saving Nodes"); fflush(stdout);
 SaveNodeList("data/nodes.mem");
 printf("\rSaved Nodes \n"); fflush(stdout);

 printf("Saving Ways"); fflush(stdout);
 SaveWayList("data/ways.mem");
 printf("\rSaved Ways \n"); fflush(stdout);

 printf("Saving Segments"); fflush(stdout);
 SaveSegmentList("data/segments.mem");
 printf("\rSaved Segments \n"); fflush(stdout);

 return(0);
}
