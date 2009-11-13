/***************************************
 $Header: /home/amb/CVS/routino/src/osmparser.c,v 1.58 2009-11-13 19:24:11 amb Exp $

 OSM XML file parser (either JOSM or planet)

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
#include <ctype.h>

#include "typesx.h"
#include "functionsx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "ways.h"


/*+ The length of the buffer and the size increment for reading lines from the file. +*/
#define BUFFSIZE 256

#define ISTRUE(xx)    (!strcmp(xx,"true") || !strcmp(xx,"yes") || !strcmp(xx,"1"))

#define ISALLOWED(xx) (!strcmp(xx,"true") || !strcmp(xx,"yes") || !strcmp(xx,"1") || \
                       !strcmp(xx,"permissive") || !strcmp(xx,"designated") || !strcmp(xx,"destination"))

/* Local functions */

static char *fgets_realloc(char *buffer,FILE *file);


/*++++++++++++++++++++++++++++++++++++++
  Parse an OSM XML file (from JOSM or planet download).

  int ParseXML Returns 0 if OK or something else in case of an error.

  FILE *file The file to read from.

  NodesX *OSMNodes The array of nodes to fill in.

  SegmentsX *OSMSegments The array of segments to fill in.

  WaysX *OSMWays The arrray of ways to fill in.

  Profile *profile A profile of the allowed transport types and included/excluded highway types.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML(FILE *file,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,Profile *profile)
{
 char *line=NULL;
 long nlines=0;
 long nnodes=0,nways=0,nrelations=0;
 int isnode=0,isway=0,isrelation=0;
 way_t way_id=0;
 wayallow_t way_allow_no=0,way_allow_yes=0;
 int way_oneway=0,way_roundabout=0;
 int way_paved=0;
 speed_t way_maxspeed=0;
 weight_t way_maxweight=0;
 height_t way_maxheight=0;
 width_t way_maxwidth=0;
 length_t way_maxlength=0;
 char *way_highway=NULL,*way_name=NULL,*way_ref=NULL;
 node_t *way_nodes=NULL;
 int way_nnodes=0,way_nalloc=0;

 printf("\rReading: Lines=0 Nodes=0 Ways=0 Relations=0");
 fflush(stdout);

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
       double latitude,longitude;

       nnodes++;

       isnode=1; isway=0; isrelation=0;

       m=strstr(l,"id=");  m+=4; if(*m=='"' || *m=='\'') m++; id=atoll(m);
       m=strstr(l,"lat="); m+=5; if(*m=='"' || *m=='\'') m++; latitude=degrees_to_radians(atof(m));
       m=strstr(l,"lon="); m+=4; if(*m=='"' || *m=='\'') m++; longitude=degrees_to_radians(atof(m));

       AppendNode(OSMNodes,id,latitude,longitude);

       if(strstr(l,"/>")) /* The end of a node */
         {
          isnode=0; isway=0; isrelation=0;
         }
      }
    else if(!strncmp(l,"</node",6)) /* The end of a node */
      {
       isnode=0; isway=0; isrelation=0;
      }
    else if(!strncmp(l,"<way",4)) /* The start of a way */
      {
       nways++;

       isnode=0; isway=1; isrelation=0;

       m=strstr(l,"id=");  m+=4; if(*m=='"' || *m=='\'') m++; way_id=atoll(m);

       way_allow_no=0; way_allow_yes=0;
       way_oneway=0; way_roundabout=0;
       way_paved=0;
       way_maxspeed=0; way_maxweight=0; way_maxheight=0; way_maxwidth=0;
       way_maxlength=0;
       way_highway=NULL; way_name=NULL; way_ref=NULL;
       way_nnodes=0;
      }
    else if(!strncmp(l,"</way",5)) /* The end of a way */
      {
       isnode=0; isway=0; isrelation=0;

       if(way_highway)
         {
          Way way={0};

          way.type=HighwayType(way_highway);

          if(profile->highway[way.type])
            {
             switch(way.type)
               {
               case Way_Motorway:
                way.type|=Way_OneWay;
                way.allow=Allow_Motorbike|Allow_Motorcar|Allow_PSV|Allow_Goods|Allow_HGV;
                way.props=Properties_Paved;
                break;
               case Way_Trunk:
                way.allow=Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_PSV|Allow_Goods|Allow_HGV;
                way.props=Properties_Paved;
                break;
               case Way_Primary:
                way.allow=Allow_Foot|Allow_Horse|Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_PSV|Allow_Goods|Allow_HGV;
                way.props=Properties_Paved;
                break;
               case Way_Secondary:
                way.allow=Allow_Foot|Allow_Horse|Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_PSV|Allow_Goods|Allow_HGV;
                way.props=Properties_Paved;
                break;
               case Way_Tertiary:
                way.allow=Allow_Foot|Allow_Horse|Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_PSV|Allow_Goods|Allow_HGV;
                way.props=Properties_Paved;
                break;
               case Way_Unclassified:
                way.allow=Allow_Foot|Allow_Horse|Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_PSV|Allow_Goods|Allow_HGV;
                way.props=Properties_Paved;
                break;
               case Way_Residential:
                way.allow=Allow_Foot|Allow_Horse|Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_PSV|Allow_Goods|Allow_HGV;
                way.props=Properties_Paved;
                break;
               case Way_Service:
                way.allow=Allow_Foot|Allow_Horse|Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_PSV|Allow_Goods|Allow_HGV;
                way.props=Properties_Paved;
                break;
               case Way_Track:
                way.allow=Allow_Foot|Allow_Horse|Allow_Bicycle;
                way.props=0;
                break;
               case Way_Cycleway:
                way.allow=Allow_Foot|Allow_Bicycle;
                way.props=Properties_Paved;
                break;
               case Way_Path:
                if(!strcmp(way_highway,"bridleway"))
                   way.allow=Allow_Foot|Allow_Horse|Allow_Bicycle; /* Special case for "bridleway". */
                else
                   way.allow=Allow_Foot; /* Only allow bicycle and horse if so indicated. */
                way.props=0;
                break;
               default:
                way.allow=0;
                way.props=0;
                break;
               }

             if(way_allow_no)      /* Remove the ones explicitly denied (e.g. private) */
                way.allow&=~way_allow_no;

             if(way_allow_yes)     /* Add the ones explicitly allowed (e.g. footpath along private) */
                way.allow|=way_allow_yes;

             if(way.allow)
               {
                char *refname;
                int i;

                if(way_oneway)
                   way.type|=Way_OneWay;

                if(way_roundabout)
                   way.type|=Way_Roundabout;

                if(way_paved>0 && profile->props_yes[Property_Paved])
                   way.props|=Properties_Paved;
                if(way_paved<0)
                   way.props&=~Properties_Paved;

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

                way.speed =way_maxspeed;
                way.weight=way_maxweight;
                way.height=way_maxheight;
                way.width =way_maxwidth;
                way.length=way_maxlength;

                AppendWay(OSMWays,way_id,&way,refname);

                if(refname!=way_ref && refname!=way_name && refname!=way_highway)
                   free(refname);

                for(i=1;i<way_nnodes;i++)
                  {
                   node_t from=way_nodes[i-1];
                   node_t to  =way_nodes[i];

                   if(way_oneway>0)
                     {
                      AppendSegment(OSMSegments,way_id,from,to,ONEWAY_1TO2);
                      AppendSegment(OSMSegments,way_id,to,from,ONEWAY_2TO1);
                     }
                   else if(way_oneway<0)
                     {
                      AppendSegment(OSMSegments,way_id,from,to,ONEWAY_2TO1);
                      AppendSegment(OSMSegments,way_id,to,from,ONEWAY_1TO2);
                     }
                   else
                     {
                      AppendSegment(OSMSegments,way_id,from,to,0);
                      AppendSegment(OSMSegments,way_id,to,from,0);
                     }
                  }
               }
            }
         }

       if(way_highway) free(way_highway);
       if(way_name)    free(way_name);
       if(way_ref)     free(way_ref);
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

          if(way_nnodes==way_nalloc)
             way_nodes=(node_t*)realloc((void*)way_nodes,(way_nalloc+=256)*sizeof(node_t));

          way_nodes[way_nnodes++]=id;
         }

       if(!strncmp(l,"<tag",4)) /* The start of a tag specifier */
         {
          char delimiter,*k="",*v="";

          m=strstr(l,"k="); m+=2; delimiter=*m; m++; k=m;
          while(*m!=delimiter) m++; *m=0; l=m+1;

          m=strstr(l,"v="); m+=2; delimiter=*m; m++; v=m;
          while(*m!=delimiter) m++; *m=0;

          switch(*k)
            {
            case 'a':
             if(!strcmp(k,"access"))
               {
                if(ISALLOWED(v))
                   ;
                else
                   way_allow_no=Allow_ALL;
               }
             break;

            case 'b':
             if(!strcmp(k,"bicycle"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Bicycle;
                else
                   way_allow_no |=Allow_Bicycle;
               }
             break;

            case 'd':
             if(!strcmp(k,"designation"))
               {
                if(!strcmp(v,"bridleway"))
                   way_allow_yes|=Allow_Foot|Allow_Horse|Allow_Bicycle;
                else if(!strcmp(v,"byway"))
                   way_allow_yes|=Allow_Foot|Allow_Horse|Allow_Bicycle;
                else if(!strcmp(v,"footpath"))
                   way_allow_yes|=Allow_Foot;
               }
             break;

            case 'f':
             if(!strcmp(k,"foot"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Foot;
                else
                   way_allow_no |=Allow_Foot;
               }
             break;

            case 'g':
             if(!strcmp(k,"goods"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Goods;
                else
                   way_allow_no |=Allow_Goods;
               }
             break;

            case 'h':
             if(!strcmp(k,"highway"))
                way_highway=strcpy((char*)malloc(strlen(v)+1),v);

             if(!strcmp(k,"horse"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Horse;
                else
                   way_allow_no |=Allow_Horse;
               }

             if(!strcmp(k,"hgv"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_HGV;
                else
                   way_allow_no |=Allow_HGV;
               }
             break;

            case 'j':
             if(!strcmp(k,"junction"))
                if(!strcmp(v,"roundabout"))
                  {way_oneway=1; way_roundabout=1;}
             break;

            case 'm':
             if(!strcmp(k,"maxspeed"))
               {
                if(strstr(v,"mph"))
                   way_maxspeed=kph_to_speed(1.609*atof(v));
                else
                   way_maxspeed=kph_to_speed(atof(v));
               }

             if(!strcmp(k,"maxspeed:mph"))
                way_maxspeed=kph_to_speed(1.609*atof(v));

             if(!strcmp(k,"maxweight"))
               {
                if(strstr(v,"kg"))
                   way_maxweight=tonnes_to_weight(atof(v)/1000);
                else
                   way_maxweight=tonnes_to_weight(atof(v));
               }

             if(!strcmp(k,"maxheight"))
               {
                if(strchr(v,'\''))
                  {
                   int feet,inches;

                   if(sscanf(v,"%d'%d\"",&feet,&inches)==2)
                      way_maxheight=metres_to_height((feet+(double)inches/12.0)*0.254);
                   else if(sscanf(v,"%d'",&feet)==1)
                      way_maxheight=metres_to_height((feet+(double)inches/12.0)*0.254);
                  }
                else if(strstr(v,"ft") || strstr(v,"feet"))
                   way_maxheight=metres_to_height(atof(v)*0.254);
                else
                   way_maxheight=metres_to_height(atof(v));
               }

             if(!strcmp(k,"maxwidth"))
               {
                if(strchr(v,'\''))
                  {
                   int feet,inches;

                   if(sscanf(v,"%d'%d\"",&feet,&inches)==2)
                      way_maxwidth=metres_to_height((feet+(double)inches/12.0)*0.254);
                   else if(sscanf(v,"%d'",&feet)==1)
                      way_maxwidth=metres_to_height((feet+(double)inches/12.0)*0.254);
                  }
                else if(strstr(v,"ft") || strstr(v,"feet"))
                   way_maxwidth=metres_to_width(atof(v)*0.254);
                else
                   way_maxwidth=metres_to_width(atof(v));
               }

             if(!strcmp(k,"maxlength"))
               {
                if(strchr(v,'\''))
                  {
                   int feet,inches;

                   if(sscanf(v,"%d'%d\"",&feet,&inches)==2)
                      way_maxlength=metres_to_height((feet+(double)inches/12.0)*0.254);
                   else if(sscanf(v,"%d'",&feet)==1)
                      way_maxlength=metres_to_height((feet+(double)inches/12.0)*0.254);
                  }
                else if(strstr(v,"ft") || strstr(v,"feet"))
                   way_maxlength=metres_to_length(atof(v)*0.254);
                else
                   way_maxlength=metres_to_length(atof(v));
               }

             if(!strcmp(k,"moped"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Moped;
                else
                   way_allow_no |=Allow_Moped;
               }

             if(!strcmp(k,"motorbike"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Motorbike;
                else
                   way_allow_no |=Allow_Motorbike;
               }

             if(!strcmp(k,"motorcar"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Motorcar;
                else
                   way_allow_no |=Allow_Motorcar;
               }

             if(!strcmp(k,"motor_vehicle"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_Goods|Allow_HGV|Allow_PSV;
                else
                   way_allow_no |=Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_Goods|Allow_HGV|Allow_PSV;
               }
             break;

            case 'n':
             if(!strcmp(k,"name"))
                way_name=strcpy((char*)malloc(strlen(v)+1),v);
             break;

            case 'o':
             if(!strcmp(k,"oneway"))
               {
                if(ISTRUE(v))
                   way_oneway=1;
                else if(!strcmp(v,"-1"))
                   way_oneway=-1;
               }
             break;

            case 'p':
             if(!strcmp(k,"paved"))
               {
                if(ISTRUE(v))
                   way_paved=1;
                else
                   way_paved=-1;
               }

             if(!strcmp(k,"psv"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_PSV;
                else
                   way_allow_no |=Allow_PSV;
               }
             break;

            case 'r':
             if(!strcmp(k,"ref"))
                way_ref=strcpy((char*)malloc(strlen(v)+1),v);
             break;

            case 's':
             if(!strcmp(k,"surface"))
               {
                if(!strcmp(v,"paved") || !strcmp(v,"asphalt") || !strcmp(v,"concrete"))
                   way_paved=1;
                else
                   way_paved=-1;
               }
             break;

            case 'v':
             if(!strcmp(k,"vehicle"))
               {
                if(ISALLOWED(v))
                   way_allow_yes|=Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_Goods|Allow_HGV|Allow_PSV;
                else
                   way_allow_no |=Allow_Bicycle|Allow_Moped|Allow_Motorbike|Allow_Motorcar|Allow_Goods|Allow_HGV|Allow_PSV;
               }
             break;

            default:
             ;
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

 printf("\rRead: Lines=%ld Nodes=%ld Ways=%ld Relations=%ld   \n",nlines,nnodes,nways,nrelations);
 fflush(stdout);

 if(line)
    free(line);

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
