/***************************************
 Data extractor and file dumper.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2016 Andrew M. Bishop

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
#include <math.h>

#include "types.h"
#include "functions.h"
#include "nodes.h"
#include "segments.h"
#include "ways.h"
#include "sorting.h"


/* Global variables (required to link sorting.c) */

/*+ The command line '--tmpdir' option or its default value. +*/
char *option_tmpdirname=NULL;

/*+ The amount of RAM to use for filesorting. +*/
size_t option_filesort_ramsize=0;

/*+ The number of threads to use for filesorting. +*/
int option_filesort_threads=1;

/* Local types */

typedef struct _crossing
{
 double   f;
 uint32_t x;
 uint32_t y;
}
 crossing;

/* Local functions */

static void ll_xy_rad(int zoom,double longitude,double latitude,double *x,double *y);

static int compare_crossing(crossing *a,crossing *b);


/*++++++++++++++++++++++++++++++++++++++
  The main function.
  ++++++++++++++++++++++++++++++++++++++*/

int main(int argc,char** argv)
{
 Nodes     *OSMNodes;
 Segments  *OSMSegments;
 Ways      *OSMWays;
 int        arg;
 char      *dirname=NULL,*prefix=NULL;
 char      *nodes_filename,*segments_filename,*ways_filename;
 index_t    item;
 crossing  *crossings=(crossing*)malloc(128*sizeof(crossing));
 crossing **crossingsp=(crossing**)malloc(128*sizeof(crossing*));
 float   ***highways,***transports,***properties,***speeds;
 double     lat,lon,x,y;
 uint32_t   xmin,ymin,xmax,ymax,xt,yt;
 int        zoom=13;
 int        speed_count=0;
 double     *speed_selection=NULL;

 /* Parse the command line arguments */

 for(arg=1;arg<argc;arg++)
   {
    if(!strcmp(argv[arg],"--help"))
       goto usage;
    else if(!strncmp(argv[arg],"--dir=",6))
       dirname=&argv[arg][6];
    else if(!strncmp(argv[arg],"--prefix=",9))
       prefix=&argv[arg][9];
    else if(!strncmp(argv[arg],"--speed=",8))
      {
       int i;

       speed_count=1;

       for(i=8;argv[arg][i];i++)
          if(argv[arg][i]==',')
             speed_count++;

       speed_selection=calloc(speed_count,sizeof(double));

       speed_count=0;

       for(i=7;argv[arg][i];i++)
          if(argv[arg][i]==',' || argv[arg][i]=='=')
             speed_selection[speed_count++]=atof(&argv[arg][i+1]);
      }
    else if(!strncmp(argv[arg],"--zoom=",7))
      {
       zoom=atoi(&argv[arg][7]);

       if(zoom<10 || zoom>16)
          goto usage;
      }
    else
      {
      usage:

       fprintf(stderr,"Usage: dumper\n"
                      "              [--help]\n"
                      "              [--dir=<name>] [--prefix=<name>]\n"
                      "              [--speed=<speed1>,<speed2>,...,<speedN>]"
                      "              [--zoom=<10-16>]\n");

       return(1);
      }
   }

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

 /* Allocate the arrays */

 lat=latlong_to_radians(bin_to_latlong(OSMNodes->file.latzero));
 lon=latlong_to_radians(bin_to_latlong(OSMNodes->file.lonzero));

 ll_xy_rad(zoom,lon,lat,&x,&y);

 xmin=(uint32_t)floor(x);
 ymax=(uint32_t)floor(y);

 lat=latlong_to_radians(bin_to_latlong(OSMNodes->file.latzero+OSMNodes->file.latbins));
 lon=latlong_to_radians(bin_to_latlong(OSMNodes->file.lonzero+OSMNodes->file.lonbins));

 ll_xy_rad(zoom,lon,lat,&x,&y);

 xmax=(uint32_t)floor(x);
 ymin=(uint32_t)floor(y);

 highways  =malloc(sizeof(highways[0])  *(xmax-xmin+1));
 transports=malloc(sizeof(transports[0])*(xmax-xmin+1));
 properties=malloc(sizeof(properties[0])*(xmax-xmin+1));
 speeds    =malloc(sizeof(speeds[0])    *(xmax-xmin+1));

 for(xt=xmin;xt<=xmax;xt++)
   {
    highways[xt-xmin]  =malloc(sizeof(highways[0][0])  *(ymax-ymin+1));
    transports[xt-xmin]=malloc(sizeof(transports[0][0])*(ymax-ymin+1));
    properties[xt-xmin]=malloc(sizeof(properties[0][0])*(ymax-ymin+1));
    speeds[xt-xmin]    =malloc(sizeof(speeds[0][0])    *(ymax-ymin+1));

    for(yt=ymin;yt<=ymax;yt++)
      {
       highways[xt-xmin][yt-ymin]  =calloc((Highway_Count-1)  ,sizeof(highways[0][0][0])  );
       transports[xt-xmin][yt-ymin]=calloc((Transport_Count-1),sizeof(transports[0][0][0]));
       properties[xt-xmin][yt-ymin]=calloc((Property_Count+1) ,sizeof(properties[0][0][0]));
       speeds[xt-xmin][yt-ymin]    =calloc((speed_count)      ,sizeof(speeds[0][0][0])    );
      }
   }

 /* Dump the segments out with lat/long in tile units. */

 for(item=0;item<OSMSegments->file.number;item++)
   {
    Segment *segment=LookupSegment(OSMSegments,item,1);

    if(IsNormalSegment(segment))
      {
       double latitude1,longitude1,latitude2,longitude2;
       double x1,y1,x2,y2;
       uint32_t xi1,yi1,xi2,yi2;
       Way *way;
       double distance;

       /* Get the coordinates */

       distance=1000*distance_to_km(DISTANCE(segment->distance));

       GetLatLong(OSMNodes,segment->node1,NULL,&latitude1,&longitude1);

       GetLatLong(OSMNodes,segment->node2,NULL,&latitude2,&longitude2);

       way=LookupWay(OSMWays,segment->way,1);

       ll_xy_rad(zoom,longitude1,latitude1,&x1,&y1);
       ll_xy_rad(zoom,longitude2,latitude2,&x2,&y2);

       /* Map to tiles and find tile crossings */

       xi1=(uint32_t)floor(x1);
       yi1=(uint32_t)floor(y1);

       xi2=(uint32_t)floor(x2);
       yi2=(uint32_t)floor(y2);

       if(xi1==xi2 && yi1==yi2)
         {
          int j;

          for(j=1;j<Highway_Count;j++)
             if(HIGHWAY(way->type) == j)
                highways[xi1-xmin][yi1-ymin][j-1]+=distance;

          for(j=1;j<Transport_Count;j++)
             if(way->allow & TRANSPORTS(j))
                transports[xi1-xmin][yi1-ymin][j-1]+=distance;

          for(j=1;j<Property_Count;j++)
             if(way->props & PROPERTIES(j))
                properties[xi1-xmin][yi1-ymin][j-1]+=distance;

          if(way->type & Highway_CycleBothWays)
             properties[xi1-xmin][yi1-ymin][Property_Count-1]+=distance;

          if(way->type & Highway_OneWay)
             properties[xi1-xmin][yi1-ymin][Property_Count]+=distance;

          for(j=0;j<speed_count;j++)
             if(speed_to_kph(way->speed) == speed_selection[j])
                speeds[xi1-xmin][yi1-ymin][j]+=distance;
         }
       else
         {
          int crossing_count=2,i;
          double lastf;
          uint32_t lastx,lasty;

          crossings[0].f=0.0;
          crossings[0].x=xi1;
          crossings[0].y=yi1;

          crossings[1].f=1.0;
          crossings[1].x=xi2;
          crossings[1].y=yi2;

          /* Find X boundaries */

          if(xi1!=xi2)
            {
             uint32_t x,minx,maxx;

             if(xi1<xi2)
               {
                minx=xi1+1;
                maxx=xi2;
               }
             else
               {
                minx=xi2+1;
                maxx=xi1;
               }

             for(x=minx;x<=maxx;x++)
               {
                double fraction=(double)(x-x1)/(double)(x2-x1);
                uint32_t y=(uint32_t)(y1+fraction*(y2-y1));

                crossings[crossing_count].f=fraction;
                crossings[crossing_count].x=x;
                crossings[crossing_count].y=y;
                crossing_count++;
               }
            }

          /* Find Y boundaries */

          if(yi1!=yi2)
            {
             uint32_t y,miny,maxy;

             if(yi1<yi2)
               {
                miny=yi1+1;
                maxy=yi2;
               }
             else
               {
                miny=yi2+1;
                maxy=yi1;
               }

             for(y=miny;y<=maxy;y++)
               {
                double fraction=(double)(y-y1)/(double)(y2-y1);
                uint32_t x=(uint32_t)(x1+fraction*(x2-x1));

                crossings[crossing_count].f=fraction;
                crossings[crossing_count].x=x;
                crossings[crossing_count].y=y;
                crossing_count++;
               }
            }

          /* Sort the boundaries */

          for(i=0;i<crossing_count;i++)
             crossingsp[i]=&crossings[i];

          filesort_heapsort((void**)crossingsp,crossing_count,(int (*)(const void*,const void*))compare_crossing);

          /* Partition the data among the tiles */

          lastf=crossingsp[0]->f;
          lastx=crossingsp[0]->x;
          lasty=crossingsp[0]->y;

          for(i=1;i<crossing_count;i++)
            {
             double f=crossingsp[i]->f;
             uint32_t xi,x=crossingsp[i]->x;
             uint32_t yi,y=crossingsp[i]->y;
             double d;
             int j;

             xi=x;
             if(lastx<x) xi=lastx;

             yi=y;
             if(lasty<y) yi=lasty;

             d=distance*(f-lastf);

             for(j=1;j<Highway_Count;j++)
                if(HIGHWAY(way->type) == j)
                   highways[xi-xmin][yi-ymin][j-1]+=d;

             for(j=1;j<Transport_Count;j++)
                if(way->allow & TRANSPORTS(j))
                   transports[xi-xmin][yi-ymin][j-1]+=d;

             for(j=1;j<Property_Count;j++)
                if(way->props & PROPERTIES(j))
                   properties[xi-xmin][yi-ymin][j-1]+=d;

             if(way->type & Highway_CycleBothWays)
                properties[xi1-xmin][yi1-ymin][Property_Count-1]+=distance;

             if(way->type & Highway_OneWay)
                properties[xi1-xmin][yi1-ymin][Property_Count]+=distance;

             for(j=0;j<speed_count;j++)
                if(speed_to_kph(way->speed) == speed_selection[j])
                   speeds[xi1-xmin][yi1-ymin][j]+=distance;

             lastf=f;
             lastx=x;
             lasty=y;
            }
         }
      }
   }

 /* Print the results */

 for(xt=xmin;xt<=xmax;xt++)
   {
    for(yt=ymin;yt<=ymax;yt++)
      {
       int do_highways=0,do_transports=0,do_properties=0;
       int j;

       for(j=1;j<Highway_Count;j++)
          if(highways[xt-xmin][yt-ymin][j-1] > 0)
             do_highways++;

       for(j=1;j<Transport_Count;j++)
          if(transports[xt-xmin][yt-ymin][j-1] > 0)
             do_transports++;

       for(j=1;j<Property_Count;j++)
          if(properties[xt-xmin][yt-ymin][j-1] > 0)
             do_properties++;

       if(do_highways || do_transports || do_properties)
         {
          printf("%d %d",xt,yt);

          for(j=1;j<Highway_Count;j++)
             printf(" %.2f",highways[xt-xmin][yt-ymin][j-1]);

          for(j=1;j<Transport_Count;j++)
             printf(" %.2f",transports[xt-xmin][yt-ymin][j-1]);

          for(j=1;j<=Property_Count+1;j++)
             printf(" %.2f",properties[xt-xmin][yt-ymin][j-1]);

          for(j=0;j<speed_count;j++)
             printf(" %.2f",speeds[xt-xmin][yt-ymin][j]);

          printf("\n");
         }

       free(highways[xt-xmin][yt-ymin]);
       free(transports[xt-xmin][yt-ymin]);
       free(properties[xt-xmin][yt-ymin]);
       free(speeds[xt-xmin][yt-ymin]);
      }

    free(highways[xt-xmin]);
    free(transports[xt-xmin]);
    free(properties[xt-xmin]);
    free(speeds[xt-xmin]);
   }

 free(highways);
 free(transports);
 free(properties);
 free(speeds);

 free(speed_selection);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the tile edge crossings into order.

  int compare_crossing Returns the comparison of the fraction (f) field.

  crossing *a The first boundary crossing.

  crossing *b The second boundary crossing.
  ++++++++++++++++++++++++++++++++++++++*/

static int compare_crossing(crossing *a,crossing *b)
{
 double ad=a->f;
 double bd=b->f;

 if(ad<bd)
    return(-1);
 else if(ad>bd)
    return(1);
 else
    return(-FILESORT_PRESERVE_ORDER(a,b)); /* latest version first */
}


/*++++++++++++++++++++++++++++++++++++++
  Convert latitude and longitude into tile coordinates.

  int zoom The zoom level.

  double longitude The latitude of the point.

  double latitude The longitude of the point.

  double *x The tile X number (including fractional part).

  double *y The tile Y number (including fractional part).
  ++++++++++++++++++++++++++++++++++++++*/

static void ll_xy_rad(int zoom,double longitude,double latitude,double *x,double *y)
{
 *x=longitude/M_PI+1;
 *y=1-log(tan(latitude/2+(M_PI/4)))/M_PI;

 *x/=pow(2,1-zoom);
 *y/=pow(2,1-zoom);
}
