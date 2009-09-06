/***************************************
 $Header: /home/amb/CVS/routino/src/filedumper.c,v 1.33 2009-09-06 15:48:42 amb Exp $

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

static void print_node(Nodes* nodes,index_t item);
static void print_segment(Segments *segments,index_t item);
static void print_way(Ways *ways,index_t item);

int main(int argc,char** argv)
{
 Nodes    *OSMNodes;
 Segments *OSMSegments;
 Ways     *OSMWays;
 int    arg;
 char  *dirname=NULL,*prefix=NULL;
 char  *nodes_filename,*segments_filename,*ways_filename;
 int    option_statistics=0;
 int    option_visualiser=0,coordcount=0;
 double latmin=0,latmax=0,lonmin=0,lonmax=0;
 char  *option_data=NULL;
 int    option_dump=0;

 /* Parse the command line arguments */

 for(arg=1;arg<argc;arg++)
   {
    if(!strcmp(argv[arg],"--help"))
       goto usage;
    else if(!strncmp(argv[arg],"--dir=",6))
       dirname=&argv[arg][6];
    else if(!strncmp(argv[arg],"--prefix=",9))
       prefix=&argv[arg][9];
    else if(!strncmp(argv[arg],"--statistics",12))
       option_statistics=1;
    else if(!strncmp(argv[arg],"--visualiser",12))
       option_visualiser=1;
    else if(!strncmp(argv[arg],"--dump",6))
       option_dump=1;
    else if(!strncmp(argv[arg],"--latmin",8) && argv[arg][8]=='=')
      {latmin=degrees_to_radians(atof(&argv[arg][9]));coordcount++;}
    else if(!strncmp(argv[arg],"--latmax",8) && argv[arg][8]=='=')
      {latmax=degrees_to_radians(atof(&argv[arg][9]));coordcount++;}
    else if(!strncmp(argv[arg],"--lonmin",8) && argv[arg][8]=='=')
      {lonmin=degrees_to_radians(atof(&argv[arg][9]));coordcount++;}
    else if(!strncmp(argv[arg],"--lonmax",8) && argv[arg][8]=='=')
      {lonmax=degrees_to_radians(atof(&argv[arg][9]));coordcount++;}
    else if(!strncmp(argv[arg],"--data",6) && argv[arg][6]=='=')
       option_data=&argv[arg][7];
    else if(!strncmp(argv[arg],"--node=",7))
       ;
    else if(!strncmp(argv[arg],"--segment=",10))
       ;
    else if(!strncmp(argv[arg],"--way=",6))
       ;
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
                      "                  [--dump --node=<node> ...\n"
                      "                          --segment=<segment> ...\n"
                      "                          --way=<way> ...]\n"
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

 if(!option_statistics && !option_visualiser && !option_dump)
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

    printf("'%s%snodes.mem'    - %9ld Bytes\n",prefix?prefix:"",prefix?"-":"",buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    stat(segments_filename,&buf);

    printf("'%s%ssegments.mem' - %9ld Bytes\n",prefix?prefix:"",prefix?"-":"",buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    stat(ways_filename,&buf);

    printf("'%s%sways.mem'     - %9ld Bytes\n",prefix?prefix:"",prefix?"-":"",buf.st_size);
    printf("%s\n",RFC822Date(buf.st_mtime));
    printf("\n");

    /* Examine the nodes */

    printf("Nodes\n");
    printf("-----\n");
    printf("\n");

    printf("sizeof(Node) =%9d Bytes\n",sizeof(Node));
    printf("Number       =%9d\n",OSMNodes->number);
    printf("Number(super)=%9d\n",OSMNodes->snumber);
    printf("\n");

    printf("Lat bins= %4d\n",OSMNodes->latbins);
    printf("Lon bins= %4d\n",OSMNodes->lonbins);
    printf("\n");

    printf("Lat zero=%5d (%8.4f deg)\n",OSMNodes->latzero,radians_to_degrees(latlong_to_radians(bin_to_latlong(OSMNodes->latzero))));
    printf("Lon zero=%5d (%8.4f deg)\n",OSMNodes->lonzero,radians_to_degrees(latlong_to_radians(bin_to_latlong(OSMNodes->lonzero))));

    /* Examine the segments */

    printf("\n");
    printf("Segments\n");
    printf("--------\n");
    printf("\n");

    printf("sizeof(Segment)=%9d Bytes\n",sizeof(Segment));
    printf("Number(total)  =%9d\n",OSMSegments->number);
    printf("Number(super)  =%9d\n",OSMSegments->snumber);
    printf("Number(normal) =%9d\n",OSMSegments->nnumber);

    /* Examine the ways */

    printf("\n");
    printf("Ways\n");
    printf("----\n");
    printf("\n");

    printf("sizeof(Way)      =%9d Bytes\n",sizeof(Way));
    printf("Number(compacted)=%9d\n",OSMWays->number);
    printf("Number(original) =%9d\n",OSMWays->onumber);
    printf("\n");

    printf("Total names =%9ld Bytes\n",buf.st_size-sizeof(Ways)-OSMWays->number*sizeof(Way));
   }

 /* Print out internal data */

 if(option_dump)
   {
    index_t item;

    for(arg=1;arg<argc;arg++)
       if(!strcmp(argv[arg],"--node=all"))
         {
          for(item=0;item<OSMNodes->number;item++)
             print_node(OSMNodes,item);
         }
       else if(!strncmp(argv[arg],"--node=",7))
         {
          item=atoi(&argv[arg][7]);

          print_node(OSMNodes,item);
         }
       else if(!strcmp(argv[arg],"--segment=all"))
         {
          for(item=0;item<OSMSegments->number;item++)
             print_segment(OSMSegments,item);
         }
       else if(!strncmp(argv[arg],"--segment=",10))
         {
          item=atoi(&argv[arg][10]);

          print_segment(OSMSegments,item);
         }
       else if(!strcmp(argv[arg],"--way=all"))
         {
          for(item=0;item<OSMWays->number;item++)
             print_way(OSMWays,item);
         }
       else if(!strncmp(argv[arg],"--way=",6))
         {
          item=atoi(&argv[arg][6]);

          print_way(OSMWays,item);
         }
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the contents of a node from the routing database.

  Nodes *nodes The set of nodes to use.

  index_t item The node index to print.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_node(Nodes* nodes,index_t item)
{
 Node *node=LookupNode(nodes,item);
 double latitude,longitude;

 GetLatLong(nodes,item,&latitude,&longitude);

 printf("Node %d\n",item);
 printf("  firstseg=%d\n",SEGMENT(node->firstseg));
 printf("  latoffset=%d lonoffset=%d (latitude=%.6f longitude=%.6f)\n",node->latoffset,node->lonoffset,radians_to_degrees(latitude),radians_to_degrees(longitude));
 if(IsSuperNode(nodes,item))
    printf("  Super-Node\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the contents of a segment from the routing database.

  Segments *segments The set of segments to use.

  index_t item The segment index to print.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_segment(Segments *segments,index_t item)
{
 Segment *segment=LookupSegment(segments,item);

 printf("Segment %d\n",item);
 printf("  node1=%d node2=%d\n",segment->node1,segment->node2);
 printf("  next2=%d\n",segment->next2);
 printf("  way=%d\n",segment->way);
 printf("  distance=%d (%.3f km)\n",DISTANCE(segment->distance),distance_to_km(DISTANCE(segment->distance)));
 if(IsSuperSegment(segment) && IsNormalSegment(segment))
    printf("  Super-Segment AND normal Segment\n");
 else if(IsSuperSegment(segment) && !IsNormalSegment(segment))
    printf("  Super-Segment\n");
 if(IsOnewayTo(segment,segment->node1))
    printf("  One-Way from node2 to node1\n");
 if(IsOnewayTo(segment,segment->node2))
    printf("  One-Way from node1 to node2\n");
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the contents of a way from the routing database.

  Ways *ways The set of ways to use.

  index_t item The way index to print.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_way(Ways *ways,index_t item)
{
 Way *way=LookupWay(ways,item);

 printf("Way %d\n",item);
 printf("  name=%s\n",WayName(ways,way));
 printf("  type=%02x (%s%s%s)\n",way->type,HighwayName(HIGHWAY(way->type)),way->type&Way_OneWay?",One-Way":"",way->type&Way_Roundabout?",Roundabout":"");
 printf("  allow=%02x\n",way->allow);
 if(way->speed)
    printf("  speed=%d (%d km/hr)\n",way->speed,speed_to_kph(way->speed));
 if(way->weight)
    printf("  weight=%d (%.1f tonnes)\n",way->weight,weight_to_tonnes(way->weight));
 if(way->height)
    printf("  height=%d (%.1f m)\n",way->height,height_to_metres(way->height));
 if(way->width)
    printf("  width=%d (%.1f m)\n",way->width,width_to_metres(way->width));
 if(way->length)
    printf("  length=%d (%.1f m)\n",way->length,length_to_metres(way->length));
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
