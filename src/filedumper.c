/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.18 2009-02-07 15:56:07 amb Exp $

 Memory file dumper.
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

#include "types.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"


int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Segments *OSMSegments;
 Ways     *OSMWays;
 char *dirname=NULL,*prefix=NULL,*filename;

 /* Parse the command line arguments */

 while(--argc>=1)
   {
    if(!strcmp(argv[argc],"--help"))
       goto usage;
    else if(!strncmp(argv[argc],"--dir=",6))
       dirname=&argv[argc][6];
    else if(!strncmp(argv[argc],"--prefix=",9))
       prefix=&argv[argc][9];
    else
      {
      usage:

       fprintf(stderr,"Usage: filedumper\n"
                      "                  [--help]\n"
                      "                  [--dir=<name>] [--prefix=<name>]\n");

       return(1);
      }
   }

 filename=(char*)malloc((dirname?strlen(dirname):0)+(prefix?strlen(prefix):0)+16);

 /* Examine the nodes */

 sprintf(filename,"%s%s%s%snodes.mem",dirname?dirname:"",dirname?"/":"",prefix?prefix:"",prefix?"-":"");
 OSMNodes=LoadNodeList(filename);

 printf("Nodes\n");
 printf("-----\n");

 printf("sizeof(Node)=%9d Bytes\n",sizeof(Node));
 printf("number      =%9d\n",OSMNodes->number);

 printf("Lat bins=%3d\n",OSMNodes->latbins);
 printf("Lon bins=%3d\n",OSMNodes->lonbins);

 printf("Lat zero=%4.6f\n",OSMNodes->latzero);
 printf("Lon zero=%4.6f\n",OSMNodes->lonzero);

 /* Examine the segments */

 sprintf(filename,"%s%s%s%ssegments.mem",dirname?dirname:"",dirname?"/":"",prefix?prefix:"",prefix?"-":"");
 OSMSegments=LoadSegmentList(filename);

 printf("\n");
 printf("Segments\n");
 printf("--------\n");

 printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
 printf("number         =%9d\n",OSMSegments->number);

 /* Examine the ways */

 sprintf(filename,"%s%s%s%sways.mem",dirname?dirname:"",dirname?"/":"",prefix?prefix:"",prefix?"-":"");
 OSMWays=LoadWayList(filename);

 printf("\n");
 printf("Ways\n");
 printf("----\n");

 printf("sizeof(Way) =%9d Bytes\n",sizeof(Way));
 printf("number      =%9d\n",OSMWays->number);

 return(0);
}
