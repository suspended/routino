/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.20 2009-04-08 16:54:34 amb Exp $

 Memory file dumper.

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

 printf("Lat bins= %4d\n",OSMNodes->latbins);
 printf("Lon bins= %4d\n",OSMNodes->lonbins);

 printf("Lat zero=%5d (%8.4f)\n",OSMNodes->latzero,(double)bin_to_lat_long(OSMNodes->latzero));
 printf("Lon zero=%5d (%8.4f)\n",OSMNodes->lonzero,(double)bin_to_lat_long(OSMNodes->lonzero));

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
