/***************************************
 $Header: /home/amb/CVS/routino/src/osmparser.c,v 1.1 2008-12-31 12:20:45 amb Exp $

 OSM XML file parser (either JOSM or planet)
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2008 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "types.h"
#include "functions.h"

#define BUFFSIZE 64

static char *fgets_realloc(char *buffer,FILE *file);


/*++++++++++++++++++++++++++++++++++++++
  Parse an OSM XML file (from JOSM or planet download).

  int ParseXML Returns 0 if OK or something else in case of an error.

  FILE *file The file to read from.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML(FILE *file)
{
 char *line=NULL;
 long nlines=0;
 long nnodes=0,nways=0,nrelations=0;
 int isnode=0,isway=0,isrelation=0;
 way_t way_id=0;
 int way_oneway=0;
 float way_maxspeed=0;
 char *way_highway=NULL,*way_name=NULL,*way_ref=NULL,*way_access=NULL;
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

       AppendNode(id,latitude,longitude);
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
       way_maxspeed=0;
       way_name=NULL; way_ref=NULL;
       way_nnodes=0;
      }
    else if(!strncmp(l,"</way",5)) /* The end of a way */
      {
       double speed=0;
       int i;

       isnode=0; isway=0; isrelation=0;

       if(!way_highway) speed=0;
       else if(way_access && !strcmp(way_access,"private")) speed=0;
       else if(way_maxspeed) speed=way_maxspeed;
       else if(!strncmp(way_highway,"motorway",8)) speed=80*1.6;
       else if(!strncmp(way_highway,"trunk",5) && way_oneway) speed=75*1.6;
       else if(!strncmp(way_highway,"primary",7) && way_oneway) speed=70*1.6;
       else if(!strncmp(way_highway,"trunk",5)) speed=65*1.6;
       else if(!strncmp(way_highway,"primary",7)) speed=60*1.6;
       else if(!strncmp(way_highway,"secondary",9)) speed=55*1.6;
       else if(!strcmp(way_highway,"tertiary")) speed=50*1.6;
       else if(!strcmp(way_highway,"unclassified") || !strcmp(way_highway,"road") || !strcmp(way_highway,"minor")) speed=40*1.6;
       else if(!strcmp(way_highway,"residential")) speed=30*1.6;
       else if(!strcmp(way_highway,"service")) speed=20*1.6;
//      else if(!strcmp(way_highway =~ m%(track|byway|unsurfaced|unpaved)%) { $speed = 10*1.6; }
//      else if(!strcmp(way_highway =~ m%(steps|path|walkway|footway|pedestrian|bridleway|cycleway|living_street)%) { $speed = 5*1.6; }
//      else if(!strcmp(way_highway =~ m%(raceway)%) { next; }
//      else { print STDERR "\nhighway=$highway\n"; next; }

       if(speed)
         {
//       if($ref && $name)
//         {$refname="$name ($ref)";}
//       if($ref && !$name)
//         {$refname=$ref;}
//       if(!$ref && $name)
//         {$refname=$name;}
//       if(!$ref && !$name)
//         {$refname="unamed $highway road";}
//
//       print WAYS "$id\t$refname\n";

          for(i=1;i<way_nnodes;i++)
            {
             node_t from=way_nodes[i-1];
             node_t to  =way_nodes[i];

             distance_t distance=SegmentLength(FindNode(from),FindNode(to));
             duration_t duration=(3600000.0/1000.0)*((double)distance/speed);

             AppendSegment(from,to,way_id,distance,duration);

             if(!way_oneway)
                AppendSegment(to,from,way_id,distance,duration);

//          $junctions{$from}++;
//          $junctions{$to}++;
            }
         }

       if(way_highway) {free(way_highway); way_highway=NULL;}
       if(way_name)    {free(way_name);    way_name=NULL;}
       if(way_ref)     {free(way_ref);     way_ref=NULL;}
       if(way_access)  {free(way_access);  way_access=NULL;}
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
          if(!strcmp(k,"junction") && !strcmp(v,"roundabout")) way_oneway=1;
          if(!strcmp(k,"highway") && !strncmp(v,"motorway",8)) way_oneway=1;

          if(!strcmp(k,"highway"))
             way_highway=strcpy((char*)malloc(strlen(v)+1),v);

          if(!strcmp(k,"name"))
             way_name=strcpy((char*)malloc(strlen(v)+1),v);

          if(!strcmp(k,"ref"))
             way_ref=strcpy((char*)malloc(strlen(v)+1),v);

          if(!strcmp(k,"access"))
             way_access=strcpy((char*)malloc(strlen(v)+1),v);

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
