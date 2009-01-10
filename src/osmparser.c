/***************************************
 $Header: /home/amb/CVS/routino/src/osmparser.c,v 1.6 2009-01-10 15:59:58 amb Exp $

 OSM XML file parser (either JOSM or planet)
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
#include <ctype.h>

#include "nodes.h"
#include "ways.h"
#include "segments.h"
#include "functions.h"


#define BUFFSIZE 64

static char *fgets_realloc(char *buffer,FILE *file);


/*++++++++++++++++++++++++++++++++++++++
  Parse an OSM XML file (from JOSM or planet download).

  int ParseXML Returns 0 if OK or something else in case of an error.

  FILE *file The file to read from.

  NodesMem *OSMNodes The array of nodes to fill in.

  SegmentsMem *OSMSegments The array of segments to fill in.

  WaysMem *OSMWays The arrray of ways to fill in.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML(FILE *file,NodesMem *OSMNodes,SegmentsMem *OSMSegments,WaysMem *OSMWays)
{
 char *line=NULL;
 long nlines=0;
 long nnodes=0,nways=0,nrelations=0;
 int isnode=0,isway=0,isrelation=0;
 way_t way_id=0;
 int way_oneway=0,way_roundabout=0;
 float way_maxspeed=0;
 char *way_highway=NULL,*way_name=NULL,*way_ref=NULL,*way_access=NULL,*way_car=NULL;
 node_t *way_nodes=NULL;
 int way_nnodes=0,way_nalloc=0;

 /* Parse the file */

 while((line=fgets_realloc(line,file)))
   {
    char *l=line,*m;

    nlines++;

    while(isspace(*l))
       l++;

    if(!strncmp(l,"<node",5)) /* The start of a node */
      {
       node_t id;
       latlong_t latitude,longitude;

       nnodes++;

       isnode=1; isway=0; isrelation=0;

       m=strstr(l,"id=");  m+=4; if(*m=='"' || *m=='\'') m++; id=atoll(m);
       m=strstr(l,"lat="); m+=5; if(*m=='"' || *m=='\'') m++; latitude=atof(m);
       m=strstr(l,"lon="); m+=4; if(*m=='"' || *m=='\'') m++; longitude=atof(m);

       AppendNode(OSMNodes,id,latitude,longitude);
      }
    else if(!strncmp(l,"</node",6)) /* The end of a node */
      {
       isnode=0; isway=0; isrelation=0;
      }
    else if(!strncmp(l,"<way",4)) /* The start of a way */
      {
       nways++;

       isnode=0; isway=1; isrelation=0;

       m=strstr(l,"id="); m+=3; if(*m=='"' || *m=='\'') m++; way_id=atol(m);

       way_oneway=0;
       way_roundabout=0;
       way_maxspeed=0;
       way_name=NULL; way_ref=NULL;
       way_nnodes=0;
      }
    else if(!strncmp(l,"</way",5)) /* The end of a way */
      {
       isnode=0; isway=0; isrelation=0;

       if(way_highway)
         {
          Way *way;
          char *refname;
          int i;

          if(way_ref && way_name)
            {
             refname=(char*)malloc(strlen(way_ref)+strlen(way_name)+4);
             sprintf(refname,"%s (%s)",way_name,way_ref);
            }
          else if(way_ref && !way_name && way_roundabout)
            {
             refname=(char*)malloc(strlen(way_ref)+14);
             sprintf(refname,"%s (roundabout)",way_ref);
            }
          else if(way_ref && !way_name)
             refname=way_ref;
          else if(!way_ref && way_name)
             refname=way_name;
          else if(way_roundabout)
            {
             refname=(char*)malloc(strlen(way_highway)+14);
             sprintf(refname,"%s (roundabout)",way_highway);
            }
          else /* if(!way_ref && !way_name && !way_roundabout) */
             refname=way_highway;

          for(i=1;i<way_nnodes;i++)
            {
             node_t from=way_nodes[i-1];
             node_t to  =way_nodes[i];

             AppendSegment(OSMSegments,from,to,way_id);

             if(!way_oneway)
                AppendSegment(OSMSegments,to,from,way_id);
            }

          way=AppendWay(OSMWays,way_id,refname);

          way->limit=way_maxspeed;
          way->type=TypeOfWay(way_highway);

          switch(way->type)
            {
            case Way_Motorway:
             way->speed=1.6*80; break;
            case Way_Trunk:
             way->speed=1.6*((way_oneway&&!way_roundabout)?75:65); break;
            case Way_Primary:
             way->speed=1.6*((way_oneway&&!way_roundabout)?70:60); break;
            case Way_Secondary:
             way->speed=1.6*55; break;
            case Way_Tertiary:
             way->speed=1.6*50; break;
            case Way_Unclassfied:
             way->speed=1.6*40; break;
            case Way_Residential:
             way->speed=1.6*30; break;
            case Way_Service:
             way->speed=1.6*20; break;
            case Way_Track:
             way->speed=1.6*10; break;
            case Way_Bridleway:
            case Way_Cycleway:
            case Way_Footway:
            case Way_Unknown:
             way->speed=0;
            }

          if(way_oneway)
             way->type|=Way_ONEWAY;

          if(way_roundabout)
             way->type|=Way_ROUNDABOUT;

          if(way_access && (!strcmp(way_access,"private") || !strcmp(way_access,"no")))
             way->type|=Way_NOTROUTABLE;

          if(way_car && !strcmp(way_car,"no"))
             way->type|=Way_NOTROUTABLE;

          if(refname!=way_ref && refname!=way_name && refname!=way_highway)
             free(refname);
         }

       if(way_highway) {free(way_highway); way_highway=NULL;}
       if(way_name)    {free(way_name);    way_name=NULL;}
       if(way_ref)     {free(way_ref);     way_ref=NULL;}
       if(way_access)  {free(way_access);  way_access=NULL;}
       if(way_car)     {free(way_car);     way_car=NULL;}
      }
    else if(!strncmp(l,"<relation",9)) /* The start of a relation */
      {
       nrelations++;

       isnode=0; isway=0; isrelation=1;
      }
    else if(!strncmp(l,"</relation",10)) /* The end of a relation */
      {
       isnode=0; isway=0; isrelation=0;
      }
    else if(isnode) /* The middle of a node */
      {
      }
    else if(isway) /* The middle of a way */
      {
       node_t id;

       if(!strncmp(l,"<nd",3)) /* The start of a node specifier */
         {
          m=strstr(l,"ref="); m+=4; if(*m=='"' || *m=='\'') m++; id=atoll(m);

          if(way_nnodes<=way_nalloc)
             way_nodes=(node_t*)realloc((void*)way_nodes,(way_nalloc+=16)*sizeof(node_t));

          way_nodes[way_nnodes++]=id;
         }

       if(!strncmp(l,"<tag",4)) /* The start of a tag specifier */
         {
          char delimiter,*k="",*v="";

          m=strstr(l,"k="); m+=2; delimiter=*m; m++; k=m;
          while(*m!=delimiter) m++; *m=0; l=m+1;

          m=strstr(l,"v="); m+=2; delimiter=*m; m++; v=m;
          while(*m!=delimiter) m++; *m=0;

          if(!strcmp(k,"oneway") && (!strcmp(v,"true") || !strcmp(v,"yes"))) way_oneway=1;
          if(!strcmp(k,"junction") && !strcmp(v,"roundabout")) {way_oneway=1; way_roundabout=1;}
          if(!strcmp(k,"highway") && !strncmp(v,"motorway",8)) way_oneway=1;

          if(!strcmp(k,"highway"))
             way_highway=strcpy((char*)malloc(strlen(v)+1),v);

          if(!strcmp(k,"name"))
             way_name=strcpy((char*)malloc(strlen(v)+1),v);

          if(!strcmp(k,"ref"))
             way_ref=strcpy((char*)malloc(strlen(v)+1),v);

          if(!strcmp(k,"access"))
             way_access=strcpy((char*)malloc(strlen(v)+1),v);

          if(!strcmp(k,"car"))
             way_car=strcpy((char*)malloc(strlen(v)+1),v);

          if(!strcmp(k,"maxspeed"))
            {
             way_maxspeed=atof(v);
             if(strstr(v,"mph"))
                way_maxspeed*=1.6;
            }
         }
      }
    else if(isrelation) /* The middle of a relation */
      {
      }

    if(!(nlines%10000))
      {
       printf("\rReading: Lines=%ld Nodes=%ld Ways=%ld Relations=%ld",nlines,nnodes,nways,nrelations);
       fflush(stdout);
      }
   }

 printf("\rRead   : Lines=%ld Nodes=%ld Ways=%ld Relations=%ld\n",nlines,nnodes,nways,nrelations);
 fflush(stdout);

 if(way_nalloc)
    free(way_nodes);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Call fgets and realloc the buffer as needed to get a whole line.

  char *fgets_realloc Returns the modified buffer (NULL at the end of the file).

  char *buffer The current buffer.

  FILE *file The file to read from.
  ++++++++++++++++++++++++++++++++++++++*/

static char *fgets_realloc(char *buffer,FILE *file)
{
 int n=0;
 char *buf;

 if(!buffer)
    buffer=(char*)malloc(BUFFSIZE+1);

 while((buf=fgets(&buffer[n],BUFFSIZE,file)))
   {
    int s=strlen(buf);
    n+=s;

    if(buffer[n-1]=='\n')
       break;
    else
       buffer=(char*)realloc(buffer,n+BUFFSIZE+1);
   }

 if(!buf)
   {free(buffer);buffer=NULL;}

 return(buffer);
}
