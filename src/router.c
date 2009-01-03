/***************************************
 $Header: /home/amb/CVS/routino/src/router.c,v 1.3 2009-01-03 12:25:24 amb Exp $

 OSM router.
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
 node_t start,finish;

 /* Parse the command line aarguments */

 if(argc!=3)
   {
    fprintf(stderr,"Usage: %s <start-node> <finish-node>\n",argv[0]);
    return(1);
   }

 start=atoll(argv[1]);
 finish=atoll(argv[2]);

 /* Load in the data */

 LoadNodeList("data/nodes.mem");
 LoadWayList("data/ways.mem");
 LoadSegmentList("data/segments.mem");

 /* Calculate the route */

 FindRoute(start,finish);

 /* Print the route */

 PrintRoute(start,finish);

 return(0);
}
