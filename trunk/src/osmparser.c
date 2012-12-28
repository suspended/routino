/***************************************
 OSM file parser (either JOSM or planet)

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2012 Andrew M. Bishop

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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "typesx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "relationsx.h"

#include "osmparser.h"
#include "tagging.h"
#include "logging.h"


/* Macros */

/*+ Checks if a value in the XML is one of the allowed values for true. +*/
#define ISTRUE(xx)  (!strcmp(xx,"true") || !strcmp(xx,"yes") || !strcmp(xx,"1"))

/*+ Checks if a value in the XML is one of the allowed values for false. +*/
#define ISFALSE(xx) (!strcmp(xx,"false") || !strcmp(xx,"no") || !strcmp(xx,"0"))

/* Global variables */

node_t *osmparser_way_nodes=NULL;
int     osmparser_way_nnodes=0;

node_t     *osmparser_relation_nodes=NULL;
int         osmparser_relation_nnodes=0;
way_t      *osmparser_relation_ways=NULL;
int         osmparser_relation_nways=0;
relation_t *osmparser_relation_relations=NULL;
int         osmparser_relation_nrelations=0;
way_t       osmparser_relation_from=NO_WAY_ID;
way_t       osmparser_relation_to=NO_WAY_ID;
node_t      osmparser_relation_via=NO_NODE_ID;

/* Local variables */

static NodesX     *nodes;
static SegmentsX  *segments;
static WaysX      *ways;
static RelationsX *relations;

/* Local functions */

static double parse_speed(way_t id,const char *k,const char *v);
static double parse_weight(way_t id,const char *k,const char *v);
static double parse_length(way_t id,const char *k,const char *v);


/*++++++++++++++++++++++++++++++++++++++
  Parse an OSM XML file (from JOSM or planet download).

  int ParseOSMFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor of the file to read from.

  NodesX *OSMNodes The data structure of nodes to fill in.

  SegmentsX *OSMSegments The data structure of segments to fill in.

  WaysX *OSMWays The data structure of ways to fill in.

  RelationsX *OSMRelations The data structure of relations to fill in.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseOSMFile(int fd,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,RelationsX *OSMRelations)
{
 int retval;

 /* Copy the function parameters and initialise the variables */

 nodes=OSMNodes;
 segments=OSMSegments;
 ways=OSMWays;
 relations=OSMRelations;

 osmparser_way_nodes=(node_t*)malloc(256*sizeof(node_t));

 osmparser_relation_nodes    =(node_t    *)malloc(256*sizeof(node_t));
 osmparser_relation_ways     =(way_t     *)malloc(256*sizeof(way_t));
 osmparser_relation_relations=(relation_t*)malloc(256*sizeof(relation_t));

 /* Parse the file */

 retval=ParseXML(fd,xml_osm_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_IGNORE);

 /* Free the variables */

 free(osmparser_way_nodes);

 free(osmparser_relation_nodes);
 free(osmparser_relation_ways);
 free(osmparser_relation_relations);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Parse an OSC XML file (from planet download).

  int ParseOSCFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor of the file to read from.

  NodesX *OSMNodes The data structure of nodes to fill in.

  SegmentsX *OSMSegments The data structure of segments to fill in.

  WaysX *OSMWays The data structure of ways to fill in.

  RelationsX *OSMRelations The data structure of relations to fill in.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseOSCFile(int fd,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,RelationsX *OSMRelations)
{
 int retval;

 /* Copy the function parameters and initialise the variables */

 nodes=OSMNodes;
 segments=OSMSegments;
 ways=OSMWays;
 relations=OSMRelations;

 osmparser_way_nodes=(node_t*)malloc(256*sizeof(node_t));

 osmparser_relation_nodes    =(node_t    *)malloc(256*sizeof(node_t));
 osmparser_relation_ways     =(way_t     *)malloc(256*sizeof(way_t));
 osmparser_relation_relations=(relation_t*)malloc(256*sizeof(relation_t));

 /* Parse the file */

 retval=ParseXML(fd,xml_osc_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_IGNORE);

 /* Free the variables */

 free(osmparser_way_nodes);

 free(osmparser_relation_nodes);
 free(osmparser_relation_ways);
 free(osmparser_relation_relations);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Parse a PBF format OSM file (from planet download).

  int ParsePBFFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor of the file to read from.

  NodesX *OSMNodes The data structure of nodes to fill in.

  SegmentsX *OSMSegments The data structure of segments to fill in.

  WaysX *OSMWays The data structure of ways to fill in.

  RelationsX *OSMRelations The data structure of relations to fill in.
  ++++++++++++++++++++++++++++++++++++++*/

int ParsePBFFile(int fd,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,RelationsX *OSMRelations)
{
 int retval;

 /* Copy the function parameters and initialise the variables */

 nodes=OSMNodes;
 segments=OSMSegments;
 ways=OSMWays;
 relations=OSMRelations;

 osmparser_way_nodes=(node_t*)malloc(256*sizeof(node_t));

 osmparser_relation_nodes    =(node_t    *)malloc(256*sizeof(node_t));
 osmparser_relation_ways     =(way_t     *)malloc(256*sizeof(way_t));
 osmparser_relation_relations=(relation_t*)malloc(256*sizeof(relation_t));

 /* Parse the file */

 retval=ParsePBF(fd);

 /* Free the variables */

 free(osmparser_way_nodes);

 free(osmparser_relation_nodes);
 free(osmparser_relation_ways);
 free(osmparser_relation_relations);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Parse an O5M format OSM file (from planet download).

  int ParseO5MFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor of the file to read from.

  NodesX *OSMNodes The data structure of nodes to fill in.

  SegmentsX *OSMSegments The data structure of segments to fill in.

  WaysX *OSMWays The data structure of ways to fill in.

  RelationsX *OSMRelations The data structure of relations to fill in.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseO5MFile(int fd,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,RelationsX *OSMRelations)
{
 int retval;

 /* Copy the function parameters and initialise the variables */

 nodes=OSMNodes;
 segments=OSMSegments;
 ways=OSMWays;
 relations=OSMRelations;

 osmparser_way_nodes=(node_t*)malloc(256*sizeof(node_t));

 osmparser_relation_nodes    =(node_t    *)malloc(256*sizeof(node_t));
 osmparser_relation_ways     =(way_t     *)malloc(256*sizeof(way_t));
 osmparser_relation_relations=(relation_t*)malloc(256*sizeof(relation_t));

 /* Parse the file */

 retval=ParseO5M(fd,0);

 /* Free the variables */

 free(osmparser_way_nodes);

 free(osmparser_relation_nodes);
 free(osmparser_relation_ways);
 free(osmparser_relation_relations);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Parse an O5C format OSM file (from planet download).

  int ParseO5CFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor of the file to read from.

  NodesX *OSMNodes The data structure of nodes to fill in.

  SegmentsX *OSMSegments The data structure of segments to fill in.

  WaysX *OSMWays The data structure of ways to fill in.

  RelationsX *OSMRelations The data structure of relations to fill in.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseO5CFile(int fd,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,RelationsX *OSMRelations)
{
 int retval;

 /* Copy the function parameters and initialise the variables */

 nodes=OSMNodes;
 segments=OSMSegments;
 ways=OSMWays;
 relations=OSMRelations;

 osmparser_way_nodes=(node_t*)malloc(256*sizeof(node_t));

 osmparser_relation_nodes    =(node_t    *)malloc(256*sizeof(node_t));
 osmparser_relation_ways     =(way_t     *)malloc(256*sizeof(way_t));
 osmparser_relation_relations=(relation_t*)malloc(256*sizeof(relation_t));

 /* Parse the file */

 retval=ParseO5M(fd,1);

 /* Free the variables */

 free(osmparser_way_nodes);

 free(osmparser_relation_nodes);
 free(osmparser_relation_ways);
 free(osmparser_relation_relations);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a node.

  TagList *tags The list of node tags.

  node_t id The id of the node.

  double latitude The latitude of the node.

  double longitude The longitude of the node.

  int mode The mode of operation to take (create, modify, delete).
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessNodeTags(TagList *tags,node_t id,double latitude,double longitude,int mode)
{
 transports_t allow=Transports_ALL;
 nodeflags_t flags=0;
 int i;

 /* Delete */

 if(mode==MODE_DELETE)
   {
    AppendNodeList(nodes,id,degrees_to_radians(latitude),degrees_to_radians(longitude),allow,NODE_DELETED);

    return;
   }

 /* Parse the tags */

 for(i=0;i<tags->ntags;i++)
   {
    int recognised=0;
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'b':
       if(!strcmp(k,"bicycle"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Bicycle;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'bicycle' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'f':
       if(!strcmp(k,"foot"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Foot;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'foot' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'g':
       if(!strcmp(k,"goods"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Goods;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'goods' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'h':
       if(!strcmp(k,"highway"))
          if(!strcmp(v,"mini_roundabout"))
            {
             flags|=NODE_MINIRNDBT;
             recognised=1; break;
            }

       if(!strcmp(k,"horse"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Horse;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'horse' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"hgv"))
         {
          if(ISFALSE(v))
             allow&=~Transports_HGV;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'hgv' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'm':
       if(!strcmp(k,"moped"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Moped;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'moped' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"motorbike"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Motorbike;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'motorbike' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"motorcar"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Motorcar;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'motorcar' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'p':
       if(!strcmp(k,"psv"))
         {
          if(ISFALSE(v))
             allow&=~Transports_PSV;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'psv' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'w':
       if(!strcmp(k,"wheelchair"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Wheelchair;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'wheelchair' = '%s' (after tagging rules); using 'yes'.\n",id,v);
          recognised=1; break;
         }

       break;

      default:
       break;
      }

    if(!recognised)
       logerror("Node %"Pnode_t" has an unrecognised tag '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 /* Create the node */

 AppendNodeList(nodes,id,degrees_to_radians(latitude),degrees_to_radians(longitude),allow,flags);
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a way.

  TagList *tags The list of way tags.

  way_t id The id of the way.

  int mode The mode of operation to take (create, modify, delete).
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessWayTags(TagList *tags,way_t id,int mode)
{
 Way way={0};
 distance_t oneway=0,area=0;
 int roundabout=0;
 char *name=NULL,*ref=NULL,*refname=NULL;
 int i,j;

 /* Delete */

 if(mode==MODE_DELETE || mode==MODE_MODIFY)
   {
    way.type=WAY_DELETED;

    AppendWayList(ways,id,&way,"");

    way.type=Highway_None;

    AppendSegmentList(segments,id,NO_NODE_ID,NO_NODE_ID,0);
   }

 if(mode==MODE_DELETE)
    return;

 /* Sanity check */

 if(osmparser_way_nnodes==0)
   {
    logerror("Way %"Pway_t" has no nodes.\n",id);
    return;
   }

 if(osmparser_way_nnodes==1)
   {
    logerror("Way %"Pway_t" has only one node.\n",id);
    return;
   }

 /* Parse the tags - just look for highway */

 for(i=0;i<tags->ntags;i++)
   {
    char *k=tags->k[i];
    char *v=tags->v[i];

    if(!strcmp(k,"highway"))
      {
       way.type=HighwayType(v);

       if(way.type==Highway_None)
          logerror("Way %"Pway_t" has an unrecognised highway type '%s' (after tagging rules); ignoring it.\n",id,v);

       break;
      }
   }

 /* Don't continue if this is not a highway (bypass error logging) */

 if(way.type==Highway_None)
    return;

 /* Parse the tags - look for the others */

 for(i=0;i<tags->ntags;i++)
   {
    int recognised=0;
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'a':
       if(!strcmp(k,"area"))
         {
          if(ISTRUE(v))
             area=SEGMENT_AREA;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'area' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'b':
       if(!strcmp(k,"bicycle"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Bicycle;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'bicycle' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"bicycleroute"))
         {
          if(ISTRUE(v))
             way.props|=Properties_BicycleRoute;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'bicycleroute' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"bridge"))
         {
          if(ISTRUE(v))
             way.props|=Properties_Bridge;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'bridge' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'f':
       if(!strcmp(k,"foot"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Foot;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'foot' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"footroute"))
         {
          if(ISTRUE(v))
             way.props|=Properties_FootRoute;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'footroute' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'g':
       if(!strcmp(k,"goods"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Goods;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'goods' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'h':
       if(!strcmp(k,"highway"))
         {recognised=1; break;}

       if(!strcmp(k,"horse"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Horse;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'horse' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"hgv"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_HGV;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'hgv' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'm':
       if(!strncmp(k,"max",3))
         {
          if(!strcmp(k+3,"speed"))
            {
             way.speed=kph_to_speed(parse_speed(id,k,v));
             recognised=1; break;
            }

          if(!strcmp(k+3,"weight"))
            {
             way.weight=tonnes_to_weight(parse_weight(id,k,v));
             recognised=1; break;
            }

          if(!strcmp(k+3,"height"))
            {
             way.height=metres_to_height(parse_length(id,k,v));
             recognised=1; break;
            }

          if(!strcmp(k+3,"width"))
            {
             way.width=metres_to_height(parse_length(id,k,v));
             recognised=1; break;
            }

          if(!strcmp(k+3,"length"))
            {
             way.length=metres_to_height(parse_length(id,k,v));
             recognised=1; break;
            }
         }

       if(!strcmp(k,"moped"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Moped;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'moped' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"motorbike"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Motorbike;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'motorbike' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"motorcar"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Motorcar;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'motorcar' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"multilane"))
         {
          if(ISTRUE(v))
             way.props|=Properties_Multilane;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'multilane' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'n':
       if(!strcmp(k,"name"))
         {
          name=v;
          recognised=1; break;
         }

       break;

      case 'o':
       if(!strcmp(k,"oneway"))
         {
          if(ISTRUE(v))
             oneway=ONEWAY_1TO2;
          else if(!strcmp(v,"-1"))
             oneway=ONEWAY_2TO1;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'oneway' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'p':
       if(!strcmp(k,"paved"))
         {
          if(ISTRUE(v))
             way.props|=Properties_Paved;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'paved' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       if(!strcmp(k,"psv"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_PSV;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'psv' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'r':
       if(!strcmp(k,"ref"))
         {
          ref=v;
          recognised=1; break;
         }

       if(!strcmp(k,"roundabout"))
         {
          if(ISTRUE(v))
             roundabout=1;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'roundabout' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 't':
       if(!strcmp(k,"tunnel"))
         {
          if(ISTRUE(v))
             way.props|=Properties_Tunnel;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'tunnel' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'w':
       if(!strcmp(k,"wheelchair"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Wheelchair;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'wheelchair' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      default:
       break;
      }

    if(!recognised)
       logerror("Way %"Pway_t" has an unrecognised tag '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 /* Create the way */

 if(!way.allow)
    return;

 if(oneway)
    way.type|=Highway_OneWay;

 if(roundabout)
    way.type|=Highway_Roundabout;

 if(ref && name)
   {
    refname=(char*)malloc(strlen(ref)+strlen(name)+4);
    sprintf(refname,"%s (%s)",name,ref);
   }
 else if(ref && !name)
    refname=ref;
 else if(!ref && name)
    refname=name;
 else /* if(!ref && !name) */
    refname="";

 AppendWayList(ways,id,&way,refname);

 if(ref && name)
    free(refname);

 if(area && osmparser_way_nodes[0]!=osmparser_way_nodes[osmparser_way_nnodes-1])
    logerror("Way %"Pway_t" is an area but not closed.\n",id);

 for(i=1;i<osmparser_way_nnodes;i++)
   {
    node_t from=osmparser_way_nodes[i-1];
    node_t to  =osmparser_way_nodes[i];

    if(from==to)
       logerror("Node %"Pnode_t" in way %"Pway_t" is connected to itself.\n",from,id);
    else
      {
       int nto=1,duplicated=0;

       for(j=1;j<i;j++)
         {
          node_t n1=osmparser_way_nodes[j-1];
          node_t n2=osmparser_way_nodes[j];

          if(n1==to && (i!=osmparser_way_nnodes-1 || j!=1))
             nto++;

          if((n1==from && n2==to) || (n2==from && n1==to))
            {
             duplicated=1;
             logerror("Segment connecting nodes %"Pnode_t" and %"Pnode_t" in way %"Pway_t" is duplicated.\n",n1,n2,id);
            }
         }

       if(nto>=2 && !duplicated)
          logerror("Node %"Pnode_t" in way %"Pway_t" appears more than once.\n",to,id);

       if(!duplicated)
          AppendSegmentList(segments,id,from,to,area+oneway);
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a relation.

  TagList *tags The list of relation tags.

  relation_t id The id of the relation.

  int mode The mode of operation to take (create, modify, delete).
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessRelationTags(TagList *tags,relation_t id,int mode)
{
 transports_t routes=Transports_None;
 transports_t except=Transports_None;
 int relation_turn_restriction=0;
 TurnRestriction restriction=TurnRestrict_None;
 int i;

 /* Delete */

 if(mode==MODE_DELETE || mode==MODE_MODIFY)
   {
    AppendRouteRelationList(relations,id,RELATION_DELETED,
                            osmparser_relation_ways,osmparser_relation_nways,
                            osmparser_relation_relations,osmparser_relation_nrelations);

    AppendTurnRelationList(relations,id,
                           osmparser_relation_from,osmparser_relation_to,osmparser_relation_via,
                           restriction,RELATION_DELETED);
   }

 if(mode==MODE_DELETE)
    return;

 /* Sanity check */

 if(osmparser_relation_nnodes==0 && osmparser_relation_nways==0 && osmparser_relation_nrelations==0)
   {
    logerror("Relation %"Prelation_t" has no nodes, ways or relations.\n",id);
    return;
   }

 /* Parse the tags */

 for(i=0;i<tags->ntags;i++)
   {
    int recognised=0;
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'b':
       if(!strcmp(k,"bicycleroute"))
         {
          if(ISTRUE(v))
             routes|=Transports_Bicycle;
          else if(!ISFALSE(v))
             logerror("Relation %"Prelation_t" has an unrecognised tag value 'bicycleroute' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'e':
       if(!strcmp(k,"except"))
         {
          for(i=1;i<Transport_Count;i++)
             if(strstr(v,TransportName(i)))
                except|=TRANSPORTS(i);

          if(except==Transports_None)
             logerror("Relation %"Prelation_t" has an unrecognised tag value 'except' = '%s' (after tagging rules); ignoring it.\n",id,v);

          recognised=1; break;
         }

       break;

      case 'f':
       if(!strcmp(k,"footroute"))
         {
          if(ISTRUE(v))
             routes|=Transports_Foot;
          else if(!ISFALSE(v))
             logerror("Relation %"Prelation_t" has an unrecognised tag value 'footroute' = '%s' (after tagging rules); using 'no'.\n",id,v);
          recognised=1; break;
         }

       break;

      case 'r':
       if(!strcmp(k,"restriction"))
         {
          if(!strcmp(v,"no_right_turn"   )) restriction=TurnRestrict_no_right_turn;
          if(!strcmp(v,"no_left_turn"    )) restriction=TurnRestrict_no_left_turn;
          if(!strcmp(v,"no_u_turn"       )) restriction=TurnRestrict_no_u_turn;
          if(!strcmp(v,"no_straight_on"  )) restriction=TurnRestrict_no_straight_on;
          if(!strcmp(v,"only_right_turn" )) restriction=TurnRestrict_only_right_turn;
          if(!strcmp(v,"only_left_turn"  )) restriction=TurnRestrict_only_left_turn;
          if(!strcmp(v,"only_straight_on")) restriction=TurnRestrict_only_straight_on;

          if(restriction==TurnRestrict_None)
             logerror("Relation %"Prelation_t" has an unrecognised tag value 'restriction' = '%s' (after tagging rules); ignoring it.\n",id,v);

          recognised=1; break;
         }

       break;

      case 't':
       if(!strcmp(k,"type"))
         {
          if(!strcmp(v,"restriction"))
             relation_turn_restriction=1;

          /* Don't log an error for relations of types that we don't handle - there are so many */
          recognised=1; break;
         }

       break;

      default:
       break;
      }

    if(!recognised)
       logerror("Relation %"Prelation_t" has an unrecognised tag '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 /* Create the route relation (must store all relations that have ways or
    relations even if they are not routes because they might be referenced by
    other relations that are routes) */

 if((osmparser_relation_nways || osmparser_relation_nrelations) && !relation_turn_restriction)
    AppendRouteRelationList(relations,id,routes,
                            osmparser_relation_ways,osmparser_relation_nways,
                            osmparser_relation_relations,osmparser_relation_nrelations);

 /* Create the turn restriction relation. */

 if(relation_turn_restriction && restriction!=TurnRestrict_None)
   {
    if(osmparser_relation_from==NO_WAY_ID)
       logerror("Relation %"Prelation_t" is a turn restriction but has no 'from' way.\n",id);
    else if(osmparser_relation_to==NO_WAY_ID)
       logerror("Relation %"Prelation_t" is a turn restriction but has no 'to' way.\n",id);
    else if(osmparser_relation_via==NO_NODE_ID)
       logerror("Relation %"Prelation_t" is a turn restriction but has no 'via' node.\n",id);
    else
       AppendTurnRelationList(relations,id,
                              osmparser_relation_from,osmparser_relation_to,osmparser_relation_via,
                              restriction,except);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string containing a speed into a double precision.

  double parse_speed Returns the speed in km/h if it can be parsed.

  way_t id The way being processed.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

static double parse_speed(way_t id,const char *k,const char *v)
{
 char *ev;
 double value=strtod(v,&ev);

 if(v==ev)
    logerror("Way %"Pway_t" has an unrecognised tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
 else
   {
    while(isspace(*ev)) ev++;

    if(!strcmp(ev,"mph"))
       return(1.609*value);

    if(*ev==0 || !strcmp(ev,"kph"))
       return(value);

    logerror("Way %"Pway_t" has an un-parseable tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string containing a weight into a double precision.

  double parse_weight Returns the weight in tonnes if it can be parsed.

  way_t id The way being processed.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

static double parse_weight(way_t id,const char *k,const char *v)
{
 char *ev;
 double value=strtod(v,&ev);

 if(v==ev)
    logerror("Way %"Pway_t" has an unrecognised tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
 else
   {
    while(isspace(*ev)) ev++;

    if(!strcmp(ev,"kg"))
       return(value/1000.0);

    if(*ev==0 || !strcmp(ev,"T") || !strcmp(ev,"t")
              || !strcmp(ev,"ton") || !strcmp(ev,"tons")
              || !strcmp(ev,"tonne") || !strcmp(ev,"tonnes"))
       return(value);

    logerror("Way %"Pway_t" has an un-parseable tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string containing a length into a double precision.

  double parse_length Returns the length in metres if it can be parsed.

  way_t id The way being processed.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

static double parse_length(way_t id,const char *k,const char *v)
{
 char *ev;
 double value=strtod(v,&ev);

 if(v==ev)
    logerror("Way %"Pway_t" has an unrecognised tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
 else
   {
    int en=0;
    int feet=0,inches=0;

    if(sscanf(v,"%d' %d\"%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(sscanf(v,"%d'%d\"%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(sscanf(v,"%d'-%d\"%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(sscanf(v,"%d - %d%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(sscanf(v,"%d ft %d in%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(sscanf(v,"%d feet %d inches%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(!strcmp(ev,"'"))
       return(feet*0.254);

    while(isspace(*ev)) ev++;

    if(!strcmp(ev,"ft") || !strcmp(ev,"feet"))
       return(value*0.254);

    if(*ev==0 || !strcmp(ev,"m") || !strcmp(ev,"metre") || !strcmp(ev,"metres"))
       return(value);

    logerror("Way %"Pway_t" has an un-parseable tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 return(0);
}
