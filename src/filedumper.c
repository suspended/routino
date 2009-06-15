/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.24 2009-06-15 18:52:54 amb Exp $

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
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "types.h"
#include "functions.h"
#include "visualiser.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"

/* Local functions */

static char *RFC822Date(time_t t);


int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Segments *OSMSegments;
 Ways     *OSMWays;
 char *dirname=NULL,*prefix=NULL;
 char *nodes_filename,*segments_filename,*ways_filename;
 int   option_statistics=0;
 int   option_visualiser=0,coordcount=0;
 float latmin=0,latmax=0,lonmin=0,lonmax=0;
 char *option_data=NULL;

 /* Parse the command line arguments */

 while(--argc>=1)
   {
    if(!strcmp(argv[argc],"--help"))
       goto usage;
    else if(!strncmp(argv[argc],"--dir=",6))
       dirname=&argv[argc][6];
    else if(!strncmp(argv[argc],"--prefix=",9))
       prefix=&argv[argc][9];
    else if(!strncmp(argv[argc],"--statistics",12))
       option_statistics=1;
    else if(!strncmp(argv[argc],"--visualiser",12))
       option_visualiser=1;
    else if(!strncmp(argv[argc],"--latmin",8) && argv[argc][8]=='=')
      {latmin=degrees_to_radians(atof(&argv[argc][9]));coordcount++;}
    else if(!strncmp(argv[argc],"--latmax",8) && argv[argc][8]=='=')
      {latmax=degrees_to_radians(atof(&argv[argc][9]));coordcount++;}
    else if(!strncmp(argv[argc],"--lonmin",8) && argv[argc][8]=='=')
      {lonmin=degrees_to_radians(atof(&argv[argc][9]));coordcount++;}
    else if(!strncmp(argv[argc],"--lonmax",8) && argv[argc][8]=='=')
      {lonmax=degrees_to_radians(atof(&argv[argc][9]));coordcount++;}
    else if(!strncmp(argv[argc],"--data",6) && argv[argc][6]=='=')
       option_data=&argv[argc][7];
    else
      {
      usage:

       fprintf(stderr,"Usage: filedumper\n"
                      "                  [--help]\n"
                      "                  [--dir=<name>] [--prefix=<name>]\n"
                      "                  [--statistics]\n"
                      "                  [--visualiser --latmin=<latmin> --latmax=<latmax>\n"
                      "                                --lonmin=<lonmin> --lonmax=<lonmax>\n"
                      "                                --data=<data-type>]\n"
                      "\n"
                      "<data-type> can be selected from:\n"
                      "junctions = segment count at each junction.\n"
                      "super     = super-node and super-segments.\n"
                      "oneway    = oneway segments.\n"
                      "speed     = speed limits.\n"
                      "weight    = weight limits.\n"
                      "height    = height limits.\n"
                      "width     = width limits.\n"
                      "length    = length limits.\n");

       return(1);
      }
   }

 if(!option_statistics && !option_visualiser)
    goto usage;

 /* Load in the data */

 OSMNodes=LoadNodeList(nodes_filename=FileName(dirname,prefix,"nodes.mem"));

 if(!OSMNodes)
   {
    fprintf(stderr,"Cannot open nodes file '%s'.\n",nodes_filename);
    return(1);
   }

 OSMSegments=LoadSegmentList(segments_filename=FileName(dirname,prefix,"segments.mem"));

 if(!OSMSegments)
   {
    fprintf(stderr,"Cannot open segments file '%s'.\n",segments_filename);
    return(1);
   }

 OSMWays=LoadWayList(ways_filename=FileName(dirname,prefix,"ways.mem"));

 if(!OSMWays)
   {
    fprintf(stderr,"Cannot open ways file '%s'.\n",ways_filename);
    return(1);
   }

 /* Write out the visualiser data */

 if(option_visualiser)
   {
    if(coordcount!=4)
      {
       fprintf(stderr,"The --visualiser option must have --latmin, --latmax, --lonmin, --lonmax.\n");
       exit(1);
      }

    if(!option_data)
      {
       fprintf(stderr,"The --visualiser option must have --data.\n");
       exit(1);
      }

    if(!strcmp(option_data,"junctions"))
       OutputJunctions(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"super"))
       OutputSuper(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"oneway"))
       OutputOneway(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"speed"))
       OutputSpeedLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"weight"))
       OutputWeightLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"height"))
       OutputHeightLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"width"))
       OutputWidthLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else if(!strcmp(option_data,"length"))
       OutputLengthLimits(OSMNodes,OSMSegments,OSMWays,latmin,latmax,lonmin,lonmax);
    else
      {
       fprintf(stderr,"Unrecognised data option '%s' with --visualiser.\n",option_data);
       exit(1);
      }
   }

 /* Print out statistics */

 if(option_statistics)
   {
    struct stat buf;

    /* Examine the files */

    printf("Files\n");
    printf("-----\n");
    printf("\n");

    stat(nodes_filename,&buf);

    printf("'%s%snodes.mem'    - %ld Bytes\n",prefix?prefix:"",prefix?"-":"",buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    stat(segments_filename,&buf);

    printf("'%s%ssegments.mem' - %ld Bytes\n",prefix?prefix:"",prefix?"-":"",buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    stat(ways_filename,&buf);

    printf("'%s%sways.mem'     - %ld Bytes\n",prefix?prefix:"",prefix?"-":"",buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    /* Examine the nodes */

    printf("Nodes\n");
    printf("-----\n");
    printf("\n");

    printf("sizeof(Node)=%9d Bytes\n",sizeof(Node));
    printf("number      =%9d\n",OSMNodes->number);
    printf("\n");

    printf("Lat bins= %4d\n",OSMNodes->latbins);
    printf("Lon bins= %4d\n",OSMNodes->lonbins);
    printf("\n");

    printf("Lat zero=%5d (%8.4f deg)\n",OSMNodes->latzero,radians_to_degrees(bin_to_lat_long(OSMNodes->latzero)));
    printf("Lon zero=%5d (%8.4f deg)\n",OSMNodes->lonzero,radians_to_degrees(bin_to_lat_long(OSMNodes->lonzero)));

    /* Examine the segments */

    printf("\n");
    printf("Segments\n");
    printf("--------\n");
    printf("\n");

    printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
    printf("number         =%9d\n",OSMSegments->number);

    /* Examine the ways */

    printf("\n");
    printf("Ways\n");
    printf("----\n");
    printf("\n");

    printf("sizeof(Way) =%9d Bytes\n",sizeof(Way));
    printf("number      =%9d\n",OSMWays->number);
   }

 return(0);
}


/*+ Conversion from time_t to date string and back (day of week). +*/
static const char* const weekdays[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

/*+ Conversion from time_t to date string and back (month of year). +*/
static const char* const months[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};


/*++++++++++++++++++++++++++++++++++++++
  Convert the time into an RFC 822 compliant date.

  char *RFC822Date Returns a pointer to a fixed string containing the date.

  time_t t The time.
  ++++++++++++++++++++++++++++++++++++++*/

static char *RFC822Date(time_t t)
{
 static char value[32];
 char weekday[4];
 char month[4];
 struct tm *tim;

 tim=gmtime(&t);

 strcpy(weekday,weekdays[tim->tm_wday]);
 strcpy(month,months[tim->tm_mon]);

 /* Sun, 06 Nov 1994 08:49:37 GMT    ; RFC 822, updated by RFC 1123 */

 sprintf(value,"%3s, %02d %3s %4d %02d:%02d:%02d %s",
         weekday,
         tim->tm_mday,
         month,
         tim->tm_year+1900,
         tim->tm_hour,
         tim->tm_min,
         tim->tm_sec,
         "GMT"
         );

 return(value);
}
