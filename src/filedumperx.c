/***************************************
 Memory file dumper for the intermediate files containing parsed data.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2015 Andrew M. Bishop

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

#include "version.h"

#include "typesx.h"
#include "nodesx.h"
#include "waysx.h"
#include "relationsx.h"

#include "files.h"
#include "sorting.h"


/* Local functions */

static void print_nodes(const char *filename);
static void print_ways(const char *filename);
static void print_route_relations(const char *filename);
static void print_turn_relations(const char *filename);

static void print_usage(int detail,const char *argerr,const char *err);


/*++++++++++++++++++++++++++++++++++++++
  The main program for the file dumper.
  ++++++++++++++++++++++++++++++++++++++*/

int main(int argc,char** argv)
{
 int   arg;
 char *dirname=NULL,*prefix=NULL;
 char *nodes_filename,*ways_filename,*route_relations_filename,*turn_relations_filename;
 int   option_dump;

 /* Parse the command line arguments */

 for(arg=1;arg<argc;arg++)
   {
    if(!strcmp(argv[arg],"--version"))
       print_usage(-1,NULL,NULL);
    else if(!strcmp(argv[arg],"--help"))
       print_usage(1,NULL,NULL);
    else if(!strncmp(argv[arg],"--dir=",6))
       dirname=&argv[arg][6];
    else if(!strncmp(argv[arg],"--prefix=",9))
       prefix=&argv[arg][9];
    else if(!strcmp(argv[arg],"--dump"))
       option_dump=1;
    else if(!strcmp(argv[arg],"--nodes"))
       ;
    else if(!strcmp(argv[arg],"--ways"))
       ;
    else if(!strcmp(argv[arg],"--route-relations"))
       ;
    else if(!strcmp(argv[arg],"--turn-relations"))
       ;
    else
       print_usage(0,argv[arg],NULL);
   }

 if((option_dump)!=1)
    print_usage(0,NULL,"Must choose --dump.");

 /* Get the filenames. */

 nodes_filename=FileName(dirname,prefix,"nodesx.parsed.mem");

 ways_filename=FileName(dirname,prefix,"waysx.parsed.mem");

 route_relations_filename=FileName(dirname,prefix,"relationsx.route.parsed.mem");

 turn_relations_filename=FileName(dirname,prefix,"relationsx.turn.parsed.mem");

 /* Print out internal data (in plain text format) */

 if(option_dump)
   {
    for(arg=1;arg<argc;arg++)
       if(!strcmp(argv[arg],"--nodes"))
         {
          print_nodes(nodes_filename);
         }
       else if(!strcmp(argv[arg],"--ways"))
         {
          print_ways(ways_filename);
         }
       else if(!strcmp(argv[arg],"--route-relations"))
         {
          print_route_relations(route_relations_filename);
         }
       else if(!strcmp(argv[arg],"--turn-relations"))
         {
          print_turn_relations(turn_relations_filename);
         }
   }

 exit(EXIT_SUCCESS);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out all of the nodes.

  const char *filename The name of the file containing the data.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_nodes(const char *filename)
{
 int fd;
 NodeX nodex;

 fd=ReOpenFileBuffered(filename);

 while(!ReadFileBuffered(fd,&nodex,sizeof(NodeX)))
   {
    printf("Node %"Pnode_t"\n",nodex.id);
    printf("  lat=%d lon=%d\n",nodex.latitude,nodex.longitude);
    printf("  allow=%02x\n",nodex.allow);
    printf("  flags=%02x\n",nodex.flags);
   }

 CloseFileBuffered(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out all of the ways.

  const char *filename The name of the file containing the data.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_ways(const char *filename)
{
 FILESORT_VARINT waysize;
 int fd;

 fd=ReOpenFileBuffered(filename);

 while(!ReadFileBuffered(fd,&waysize,FILESORT_VARSIZE))
   {
    WayX wayx;
    node_t node;
    char *name=NULL;
    size_t malloced=0;
    int first=1;

    ReadFileBuffered(fd,&wayx,sizeof(WayX));

    printf("Way %"Pway_t"\n",wayx.id);

    while(!ReadFileBuffered(fd,&node,sizeof(node_t)) && node!=NO_NODE_ID)
      {
       if(first)
          printf("  nodes=%"Pnode_t,node);
       else
          printf(",%"Pnode_t,node);

       waysize-=sizeof(node_t);

       first=0;
      }

    printf("\n");

    waysize-=sizeof(node_t)+sizeof(WayX);

    if(malloced<waysize)
      {
       malloced=waysize;
       name=(char*)realloc((void*)name,malloced);
      }

    ReadFileBuffered(fd,name,waysize);

    if(*name)
       printf("  name=%s\n",name);
    printf("  type=%02x\n",wayx.way.type);
    printf("  allow=%02x\n",wayx.way.allow);
    if(wayx.way.props)
       printf("  props=%02x\n",wayx.way.props);
    if(wayx.way.speed)
       printf("  speed=%d\n",wayx.way.speed);
    if(wayx.way.weight)
       printf("  weight=%d\n",wayx.way.weight);
    if(wayx.way.height)
       printf("  height=%d\n",wayx.way.height);
    if(wayx.way.width)
       printf("  width=%d\n",wayx.way.width);
    if(wayx.way.length)
       printf("  length=%d\n",wayx.way.length);
   }

 CloseFileBuffered(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out all of the route relations.

  const char *filename The name of the file containing the data.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_route_relations(const char *filename)
{
 FILESORT_VARINT relationsize;
 int fd;

 fd=ReOpenFileBuffered(filename);

 while(!ReadFileBuffered(fd,&relationsize,FILESORT_VARSIZE))
   {
    RouteRelX relationx;
    node_t nodeid;
    way_t wayid;
    relation_t relationid;

    ReadFileBuffered(fd,&relationx,sizeof(RouteRelX));

    printf("Relation %"Prelation_t"\n",relationx.id);
    printf("  routes=%02x\n",relationx.routes);

    while(!ReadFileBuffered(fd,&nodeid,sizeof(node_t)) && nodeid!=NO_NODE_ID)
       printf("  node=%"Pnode_t"\n",nodeid);

    while(!ReadFileBuffered(fd,&wayid,sizeof(way_t)) && wayid!=NO_WAY_ID);
       printf("  way=%"Pway_t"\n",wayid);

    while(!ReadFileBuffered(fd,&relationid,sizeof(relation_t)) && relationid!=NO_RELATION_ID);
       printf("  relation=%"Prelation_t"\n",relationid);
   }

 CloseFileBuffered(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out all of the turn relations.

  const char *filename The name of the file containing the data.
  ++++++++++++++++++++++++++++++++++++++*/

static void print_turn_relations(const char *filename)
{
 int fd;
 TurnRelX relationx;

 fd=ReOpenFileBuffered(filename);

 while(!ReadFileBuffered(fd,&relationx,sizeof(TurnRelX)))
   {
    printf("Relation %"Prelation_t"\n",relationx.id);
    printf("  from=%"Pway_t"\n",relationx.from);
    printf("  via=%"Pnode_t"\n",relationx.via);
    printf("  to=%"Pway_t"\n",relationx.to);
    printf("  type=%d\n",relationx.restriction);
    if(relationx.except)
       printf("  except=%02x\n",relationx.except);
   }

 CloseFileBuffered(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Print out the usage information.

  int detail The level of detail to use: -1 = just version number, 0 = low detail, 1 = full details.

  const char *argerr The argument that gave the error (if there is one).

  const char *err Other error message (if there is one).
  ++++++++++++++++++++++++++++++++++++++*/

static void print_usage(int detail,const char *argerr,const char *err)
{
 if(detail<0)
   {
    fprintf(stderr,
            "Routino version " ROUTINO_VERSION " " ROUTINO_URL ".\n"
            );
   }

 if(detail>=0)
   {
    fprintf(stderr,
            "Usage: filedumperx [--version]\n"
            "                   [--help]\n"
            "                   [--dir=<dirname>] [--prefix=<name>]\n"
            "                   [--dump [--nodes]\n"
            "                           [--ways]\n"
            "                           [--route-relations]\n"
            "                           [--turn-relations]]\n");

    if(argerr)
       fprintf(stderr,
               "\n"
               "Error with command line parameter: %s\n",argerr);

    if(err)
       fprintf(stderr,
               "\n"
               "Error: %s\n",err);
   }

 if(detail==1)
    fprintf(stderr,
            "\n"
            "--version                 Print the version of Routino.\n"
            "\n"
            "--help                    Prints this information.\n"
            "\n"
            "--dir=<dirname>           The directory containing the routing database.\n"
            "--prefix=<name>           The filename prefix for the routing database.\n"
            "\n"
            "--dump                    Dump the intermediate files after parsing.\n"
            "  --nodes                 * all of the nodes.\n"
            "  --ways                  * all of the ways.\n"
            "  --route-relations       * all of the route relations.\n"
            "  --turn-relations        * all of the turn relations.\n");

 exit(!detail);
}
