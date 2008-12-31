/***************************************
 $Header: /home/amb/CVS/routino/src/planetsplitter.c,v 1.1 2008-12-31 12:20:55 amb Exp $

 OSM planet file splitter.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008 Andrew M. Bishop
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
 /* Create the variables */

 NewNodeList();
 NewSegmentList();

 /* Parse the file */

 ParseXML(stdin);

 /* Write out the variables */

 printf("Saving Nodes"); fflush(stdout);
 SaveNodeList("data/nodes.mem");
 printf("\rSaved Nodes \n"); fflush(stdout);

 printf("Saving Segments"); fflush(stdout);
 SaveSegmentList("data/segments.mem");
 printf("\rSaved Segments \n"); fflush(stdout);

 return(0);
}
