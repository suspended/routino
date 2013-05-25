/***************************************
 Error log processing functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2013 Andrew M. Bishop

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


#include "typesx.h"
#include "nodesx.h"
#include "waysx.h"
#include "relationsx.h"

#include "errorlogx.h"
#include "errorlog.h"

#include "files.h"
#include "sorting.h"


/* Local variables */

/*+ The name of the error log file. +*/
extern char *errorlogfilename;

/*+ The name of the binary error log file. +*/
extern char *errorbinfilename;


/* Local functions */

static void reindex_nodes(NodesX *nodesx);
static void reindex_ways(WaysX *waysx);
static void reindex_relations(RelationsX *relationsx);

static int lookup_lat_long_node(NodesX *nodesx,node_t node,latlong_t *latitude,latlong_t *longitude);
static int lookup_lat_long_way(WaysX *waysx,NodesX *nodesx,way_t way,latlong_t *latitude,latlong_t *longitude,index_t error);
static int lookup_lat_long_relation(RelationsX *relationsx,WaysX *waysx,NodesX *nodesx,relation_t relation,latlong_t *latitude,latlong_t *longitude,index_t error);

static int sort_by_lat_long(ErrorLogX *a,ErrorLogX *b);


/*++++++++++++++++++++++++++++++++++++++
  Process the binary error log.

  NodesX *nodesx The set of nodes.

  WaysX *waysx The set of ways.

  RelationsX *relationsx The set of relations.
  ++++++++++++++++++++++++++++++++++++++*/

void ProcessErrorLogs(NodesX *nodesx,WaysX *waysx,RelationsX *relationsx)
{
 int oldfd,newfd;
 uint32_t offset=0;
 int nerrorlogobjects=0;
 int finished;
 ErrorLogObject errorlogobjects[8];
 index_t number;

 /* Re-index the nodes, ways and relations */

 printf_first("Re-indexing the Data: Nodes=0 Ways=0 Route-Relations=0 Turn-Relations=0");

 reindex_nodes(nodesx);

 printf_middle("Re-indexing the Data: Nodes=%"Pindex_t" Ways=0 Route-Relations=0 Turn-Relations=0",nodesx->number);

 reindex_ways(waysx);

 printf_middle("Re-indexing the Data: Nodes=%"Pindex_t" Ways=%"Pindex_t" Route-Relations=0 Turn-Relations=0",nodesx->number,waysx->number);

 reindex_relations(relationsx);

 printf_last("Re-indexed the Data: Nodes=%"Pindex_t" Ways=%"Pindex_t" Route-Relations=%"Pindex_t" Turn-Relations=%"Pindex_t,nodesx->number,waysx->number,relationsx->rrnumber,relationsx->trnumber);


 /* Print the start message */

 printf_first("Calculating Coordinates: Errors=0");

 /* Map into memory / open the files */

#if !SLIM
 nodesx->data=MapFile(nodesx->filename);
#else
 nodesx->fd=ReOpenFile(nodesx->filename);

 InvalidateNodeXCache(nodesx->cache);
#endif

 waysx->fd=ReOpenFile(waysx->filename);
 relationsx->rrfd=ReOpenFile(relationsx->rrfilename);
 relationsx->trfd=ReOpenFile(relationsx->trfilename);

 /* Open the binary log file read-only and a new file writeable */

 oldfd=ReOpenFile(errorbinfilename);

 DeleteFile(errorbinfilename);

 newfd=OpenFileNew(errorbinfilename);

 /* Loop through the file and merge the raw data into coordinates */

 number=0;

 do
   {
    ErrorLogObject errorlogobject;

    finished=ReadFile(oldfd,&errorlogobject,sizeof(ErrorLogObject));

    if(finished)
       errorlogobject.offset=SizeFile(errorlogfilename);

    if(offset!=errorlogobject.offset)
      {
       ErrorLogX errorlogx;
       latlong_t errorlat=NO_LATLONG,errorlon=NO_LATLONG;

       /* Calculate suitable coordinates */

       if(nerrorlogobjects==1)
         {
          if(((errorlogobjects[0].type_id>>56)&0xff)=='N')
            {
             node_t node=(node_t)(errorlogobjects[0].type_id&(uint64_t)0x00ffffffffffffff);

             lookup_lat_long_node(nodesx,node,&errorlat,&errorlon);
            }
          else if(((errorlogobjects[0].type_id>>56)&0xff)=='W')
            {
             way_t way=(way_t)(errorlogobjects[0].type_id&(uint64_t)0x00ffffffffffffff);

             lookup_lat_long_way(waysx,nodesx,way,&errorlat,&errorlon,number);
            }
          else if(((errorlogobjects[0].type_id>>56)&0xff)=='R')
            {
             relation_t relation=(relation_t)(errorlogobjects[0].type_id&(uint64_t)0x00ffffffffffffff);

             lookup_lat_long_relation(relationsx,waysx,nodesx,relation,&errorlat,&errorlon,number);
            }
         }
       else
         {
          latlong_t latitude[8],longitude[8];
          int i;
          int ncoords=0,nnodes=0,nways=0,nrelations=0;

          for(i=0;i<nerrorlogobjects;i++)
            {
             if(((errorlogobjects[i].type_id>>56)&0xff)=='N')
               {
                node_t node=(node_t)(errorlogobjects[i].type_id&(uint64_t)0x00ffffffffffffff);

                if(lookup_lat_long_node(nodesx,node,&latitude[ncoords],&longitude[ncoords]))
                   ncoords++;

                nnodes++;
               }
             else if(((errorlogobjects[i].type_id>>56)&0xff)=='W') nways++;
             else if(((errorlogobjects[i].type_id>>56)&0xff)=='R') nrelations++;
            }

          if(nways==0 && nrelations==0) /* only nodes */
             ;
          else if(ncoords)      /* some good nodes, possibly ways and/or relations */
             ;
          else if(nways)        /* no good nodes, possibly some good ways */
            {
             for(i=0;i<nerrorlogobjects;i++)
                if(((errorlogobjects[i].type_id>>56)&0xff)=='W')
                  {
                   way_t way=(way_t)(errorlogobjects[i].type_id&(uint64_t)0x00ffffffffffffff);

                   if(lookup_lat_long_way(waysx,nodesx,way,&latitude[ncoords],&longitude[ncoords],number))
                      ncoords++;
                  }
            }

          if(nrelations==0) /* only nodes and/or ways */
             ;
          else if(ncoords)  /* some good nodes and/or ways, possibly relations */
             ;
          else /* if(nrelations) */
            {
             for(i=0;i<nerrorlogobjects;i++)
                if(((errorlogobjects[i].type_id>>56)&0xff)=='R')
                  {
                   relation_t relation=(relation_t)(errorlogobjects[i].type_id&(uint64_t)0x00ffffffffffffff);

                   if(lookup_lat_long_relation(relationsx,waysx,nodesx,relation,&latitude[ncoords],&longitude[ncoords],number))
                      ncoords++;
                  }
            }

          if(ncoords)
            {
             errorlat=0;
             errorlon=0;

             for(i=0;i<ncoords;i++)
               {
                errorlat+=latitude[i];
                errorlon+=longitude[i];
               }

             errorlat/=ncoords;
             errorlon/=ncoords;
            }
          else
            {
             errorlat=NO_LATLONG;
             errorlon=NO_LATLONG;
            }
         }

       /* Write to file */

       errorlogx.offset=offset;
       errorlogx.length=errorlogobject.offset-offset;

       errorlogx.latitude =errorlat;
       errorlogx.longitude=errorlon;

       WriteFile(newfd,&errorlogx,sizeof(ErrorLogX));

       number++;

       offset=errorlogobject.offset;
       nerrorlogobjects=0;

       if(!(number%10000))
          printf_middle("Calculating Coordinates: Errors=%"Pindex_t,number);
      }

    /* Store for later */

    logassert(nerrorlogobjects<8,"Too many error log objects for one error message."); /* Only a limited amount of information stored. */

    errorlogobjects[nerrorlogobjects]=errorlogobject;

    nerrorlogobjects++;
   }
 while(!finished);

 /* Unmap from memory / close the files */

#if !SLIM
 nodesx->data=UnmapFile(nodesx->data);
#else
 nodesx->fd=CloseFile(nodesx->fd);
#endif

 waysx->fd=CloseFile(waysx->fd);
 relationsx->rrfd=CloseFile(relationsx->rrfd);
 relationsx->trfd=CloseFile(relationsx->trfd);

 CloseFile(oldfd);
 CloseFile(newfd);

 /* Print the final message */

 printf_last("Calculated Coordinates: Errors=%"Pindex_t,number);
}


/*++++++++++++++++++++++++++++++++++++++
  Re-index the nodes that were kept.

  NodesX *nodesx The set of nodes to process (contains the filename and number of nodes).
  ++++++++++++++++++++++++++++++++++++++*/

static void reindex_nodes(NodesX *nodesx)
{
 int fd;
 index_t index=0;
 NodeX nodex;

 nodesx->number=nodesx->knumber;

 nodesx->idata=(node_t*)malloc(nodesx->number*sizeof(node_t));

 /* Get the node id for each node in the file. */

 fd=ReOpenFile(nodesx->filename);

 while(!ReadFile(fd,&nodex,sizeof(NodeX)))
   {
    nodesx->idata[index]=nodex.id;

    index++;
   }

 CloseFile(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Re-index the ways that were kept.

  WaysX *waysx The set of ways to process (contains the filename and number of ways).
  ++++++++++++++++++++++++++++++++++++++*/

static void reindex_ways(WaysX *waysx)
{
 int fd;
 off_t size,position=0;
 index_t index=0;

 waysx->number=waysx->knumber;

 waysx->idata=(way_t*)malloc(waysx->number*sizeof(way_t));
 waysx->odata=(off_t*)malloc(waysx->number*sizeof(off_t));

 /* Get the way id and the offset for each way in the file */

 size=SizeFile(waysx->filename);

 fd=ReOpenFile(waysx->filename);

 while(position<size)
   {
    WayX wayx;
    FILESORT_VARINT waysize;

    SeekReadFile(fd,&waysize,FILESORT_VARSIZE,position);
    SeekReadFile(fd,&wayx,sizeof(WayX),position+FILESORT_VARSIZE);

    waysx->idata[index]=wayx.id;
    waysx->odata[index]=position+FILESORT_VARSIZE+sizeof(WayX);

    index++;

    position+=waysize+FILESORT_VARSIZE;
   }

 CloseFile(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Re-index the relations that were kept.

  RelationsX *relationsx The set of relations to process (contains the filenames and numbers of relations).
  ++++++++++++++++++++++++++++++++++++++*/

static void reindex_relations(RelationsX *relationsx)
{
 int fd;
 off_t size,position=0;
 index_t index;
 TurnRelX turnrelx;

 /* Route relations */

 relationsx->rrnumber=relationsx->rrknumber;

 relationsx->rridata=(relation_t*)malloc(relationsx->rrnumber*sizeof(relation_t));
 relationsx->rrodata=(off_t*)malloc(relationsx->rrnumber*sizeof(off_t));

 /* Get the relation id and the offset for each relation in the file */

 size=SizeFile(relationsx->rrfilename);

 fd=ReOpenFile(relationsx->rrfilename);

 index=0;

 while(position<size)
   {
    FILESORT_VARINT relationsize;
    RouteRelX routerelx;

    SeekReadFile(fd,&relationsize,FILESORT_VARSIZE,position);
    SeekReadFile(fd,&routerelx,sizeof(RouteRelX),position+FILESORT_VARSIZE);

    relationsx->rridata[index]=routerelx.id;
    relationsx->rrodata[index]=position+FILESORT_VARSIZE+sizeof(RouteRelX);

    index++;

    position+=relationsize+FILESORT_VARSIZE;
   }

 CloseFile(fd);


 /* Turn relations */

 relationsx->trnumber=relationsx->trknumber;

 relationsx->tridata=(relation_t*)malloc(relationsx->trnumber*sizeof(relation_t));

 /* Get the relation id and the offset for each relation in the file */

 fd=ReOpenFile(relationsx->trfilename);

 index=0;

 while(!ReadFile(fd,&turnrelx,sizeof(TurnRelX)))
   {
    relationsx->tridata[index]=turnrelx.id;

    index++;
   }

 CloseFile(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup a node's latitude and longitude.

  int lookup_lat_long_node Returns 1 if a node was found.

  NodesX *nodesx The set of nodes to use.

  node_t node The node number.

  latlong_t *latitude Returns the latitude.

  latlong_t *longitude Returns the longitude.
  ++++++++++++++++++++++++++++++++++++++*/

static int lookup_lat_long_node(NodesX *nodesx,node_t node,latlong_t *latitude,latlong_t *longitude)
{
 index_t index=IndexNodeX(nodesx,node);

 if(index==NO_NODE)
    return 0;
 else
   {
    NodeX *nodex=LookupNodeX(nodesx,index,1);

    *latitude =nodex->latitude;
    *longitude=nodex->longitude;

    return 1;
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup a way's latitude and longitude.

  int lookup_lat_long_way Returns 1 if a way was found.

  WaysX *waysx The set of ways to use.

  NodesX *nodesx The set of nodes to use.

  way_t way The way number.

  latlong_t *latitude Returns the latitude.

  latlong_t *longitude Returns the longitude.

  index_t error The index of the error in the complete set of errors.
  ++++++++++++++++++++++++++++++++++++++*/

static int lookup_lat_long_way(WaysX *waysx,NodesX *nodesx,way_t way,latlong_t *latitude,latlong_t *longitude,index_t error)
{
 index_t index=IndexWayX(waysx,way);

 if(index==NO_WAY)
    return 0;
 else
   {
    int count=1;
    off_t offset=waysx->odata[index];
    node_t node1,node2,prevnode,node;
    latlong_t latitude1,longitude1,latitude2,longitude2;

    SeekFile(waysx->fd,offset);

    /* Choose a random pair of adjacent nodes */

    if(ReadFile(waysx->fd,&node1,sizeof(node_t)) || node1==NO_NODE_ID)
       return 0;

    if(ReadFile(waysx->fd,&node2,sizeof(node_t)) || node2==NO_NODE_ID)
       return lookup_lat_long_node(nodesx,node1,latitude,longitude);

    prevnode=node2;

    while(!ReadFile(waysx->fd,&node,sizeof(node_t)) && node!=NO_NODE_ID)
      {
       count++;

       if((error%count)==0)     /* A 1/count chance */
         {
          node1=prevnode;
          node2=node;
         }

       prevnode=node;
      }

    if(!lookup_lat_long_node(nodesx,node1,&latitude1,&longitude1))
       return lookup_lat_long_node(nodesx,node2,latitude,longitude);

    if(!lookup_lat_long_node(nodesx,node2,&latitude2,&longitude2))
       return lookup_lat_long_node(nodesx,node1,latitude,longitude);

    *latitude =(latitude1 +latitude2 )/2;
    *longitude=(longitude1+longitude2)/2;

    return 1;
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Lookup a relation's latitude and longitude.

  int lookup_lat_long_relation Returns 1 if a relation was found.

  RelationsX *relationsx The set of relations to use.

  WaysX *waysx The set of ways to use.

  NodesX *nodesx The set of nodes to use.

  relation_t relation The relation number.

  latlong_t *latitude Returns the latitude.

  latlong_t *longitude Returns the longitude.

  index_t error The index of the error in the complete set of errors.
  ++++++++++++++++++++++++++++++++++++++*/

static int lookup_lat_long_relation(RelationsX *relationsx,WaysX *waysx,NodesX *nodesx,relation_t relation,latlong_t *latitude,latlong_t *longitude,index_t error)
{
 index_t index=IndexRouteRelX(relationsx,relation);

 if(index==NO_RELATION)
   {
    index=IndexTurnRelX(relationsx,relation);

    if(index==NO_RELATION)
       return 0;
    else
      {
       TurnRelX turnrelx;

       SeekReadFile(relationsx->trfd,&turnrelx,sizeof(TurnRelX),index*sizeof(TurnRelX));

       if(lookup_lat_long_node(nodesx,turnrelx.via,latitude,longitude))
          return 1;

       if(lookup_lat_long_way(waysx,nodesx,turnrelx.from,latitude,longitude,error))
          return 1;

       if(lookup_lat_long_way(waysx,nodesx,turnrelx.to,latitude,longitude,error))
          return 1;

       return 0;
      }
   }
 else
   {
    int count=0;
    off_t offset=relationsx->rrodata[index];
    way_t way=NO_WAY_ID,tempway;
    relation_t relation=NO_RELATION_ID,temprelation;

    SeekFile(relationsx->rrfd,offset);

    /* Choose a random way */

    while(!ReadFile(relationsx->rrfd,&tempway,sizeof(way_t)) && tempway!=NO_WAY_ID)
      {
       count++;

       if((error%count)==0)     /* A 1/count chance */
          way=tempway;
      }

    if(lookup_lat_long_way(waysx,nodesx,way,latitude,longitude,error))
       return 1;

    /* Choose a random relation */

    while(!ReadFile(relationsx->rrfd,&temprelation,sizeof(relation_t)) && temprelation!=NO_RELATION_ID)
      {
       count++;

       if((error%count)==0)     /* A 1/count chance */
          relation=temprelation;
      }

    return lookup_lat_long_relation(relationsx,waysx,nodesx,relation,latitude,longitude,error);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the error logs geographically.
  ++++++++++++++++++++++++++++++++++++++*/

void SortErrorLogsGeographically(void)
{
 int oldfd,newfd;
 index_t number;

 /* Print the start message */

 printf_first("Sorting Errors Geographically");

 /* Re-open the file read-only and a new file writeable */

 oldfd=ReOpenFile(errorbinfilename);

 DeleteFile(errorbinfilename);

 newfd=OpenFileNew(errorbinfilename);

 /* Sort errors geographically */

 number=filesort_fixed(oldfd,newfd,sizeof(ErrorLogX),NULL,
                                                     (int (*)(const void*,const void*))sort_by_lat_long,
                                                     NULL);

 /* Close the files */

 CloseFile(oldfd);
 CloseFile(newfd);

 /* Print the final message */

 printf_last("Sorted Errors Geographically: Error=%"Pindex_t,number);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the errors into latitude and longitude order (first by longitude bin
  number, then by latitude bin number and then by exact longitude and then by
  exact latitude).

  int sort_by_lat_long Returns the comparison of the latitude and longitude fields.

  ErrorLogX *a The first error location.

  ErrorLogX *b The second error location.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_lat_long(ErrorLogX *a,ErrorLogX *b)
{
 ll_bin_t a_lon=latlong_to_bin(a->longitude);
 ll_bin_t b_lon=latlong_to_bin(b->longitude);

 if(a_lon<b_lon)
    return(-1);
 else if(a_lon>b_lon)
    return(1);
 else
   {
    ll_bin_t a_lat=latlong_to_bin(a->latitude);
    ll_bin_t b_lat=latlong_to_bin(b->latitude);

    if(a_lat<b_lat)
       return(-1);
    else if(a_lat>b_lat)
       return(1);
    else
      {
       if(a->longitude<b->longitude)
          return(-1);
       else if(a->longitude>b->longitude)
          return(1);
       else
         {
          if(a->latitude<b->latitude)
             return(-1);
          else if(a->latitude>b->latitude)
             return(1);
         }

       return(FILESORT_PRESERVE_ORDER(a,b));
      }
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Save the binary error log.

  NodesX *nodesx The set of nodes to copy some data from.

  char *filename The name of the final file to write.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveErrorLogs(NodesX *nodesx,char *filename)
{
 ErrorLogsFile errorlogsfile;
 ErrorLogX errorlogx;
 int oldfd,newfd;
 ll_bin2_t latlonbin=0,maxlatlonbins;
 index_t *offsets;
 index_t number=0,number_geo=0,number_nongeo=0;
 off_t size;

 /* Print the start message */

 printf_first("Writing Errors: Geographical=0 Non-geographical=0");

 /* Allocate the memory for the geographical offsets array */

 offsets=(index_t*)malloc((nodesx->latbins*nodesx->lonbins+1)*sizeof(index_t));

 logassert(offsets,"Failed to allocate memory (try using slim mode?)"); /* Check malloc() worked */

 latlonbin=0;

 /* Re-open the file */

 oldfd=ReOpenFile(errorbinfilename);

 newfd=OpenFileNew(filename);

 /* Write out the geographical errors */

 SeekFile(newfd,sizeof(ErrorLogsFile)+(nodesx->latbins*nodesx->lonbins+1)*sizeof(index_t));

 while(!ReadFile(oldfd,&errorlogx,sizeof(ErrorLogX)))
   {
    ErrorLog errorlog={0};
    ll_bin_t latbin,lonbin;
    ll_bin2_t llbin;

    if(errorlogx.latitude==NO_LATLONG)
       continue;

    /* Create the ErrorLog */

    errorlog.latoffset=latlong_to_off(errorlogx.latitude);
    errorlog.lonoffset=latlong_to_off(errorlogx.longitude);

    errorlog.offset=errorlogx.offset;
    errorlog.length=errorlogx.length;

    /* Work out the offsets */

    latbin=latlong_to_bin(errorlogx.latitude )-nodesx->latzero;
    lonbin=latlong_to_bin(errorlogx.longitude)-nodesx->lonzero;
    llbin=lonbin*nodesx->latbins+latbin;

    for(;latlonbin<=llbin;latlonbin++)
       offsets[latlonbin]=number_geo;

    /* Write the data */

    WriteFile(newfd,&errorlog,sizeof(ErrorLog));

    number_geo++;
    number++;

    if(!(number%10000))
       printf_middle("Writing Errors: Geographical=%"Pindex_t" Non-geographical=%"Pindex_t,number_geo,number_nongeo);
   }

 /* Write out the non-geographical errors */

 SeekFile(oldfd,0);

 while(!ReadFile(oldfd,&errorlogx,sizeof(ErrorLogX)))
   {
    ErrorLog errorlog={0};

    if(errorlogx.latitude!=NO_LATLONG)
       continue;

    /* Create the ErrorLog */

    errorlog.latoffset=0;
    errorlog.lonoffset=0;

    errorlog.offset=errorlogx.offset;
    errorlog.length=errorlogx.length;

    /* Write the data */

    WriteFile(newfd,&errorlog,sizeof(ErrorLog));

    number_nongeo++;
    number++;

    if(!(number%10000))
       printf_middle("Writing Errors: Geographical=%"Pindex_t" Non-geographical=%"Pindex_t,number_geo,number_nongeo);
   }

 /* Close the input file */

 CloseFile(oldfd);

 DeleteFile(errorbinfilename);

 /* Append the text from the log file */

 size=SizeFile(errorlogfilename);

 oldfd=ReOpenFile(errorlogfilename);

 while(size)
   {
    int i;
    char buffer[4096];
    off_t chunksize=(size>sizeof(buffer)?sizeof(buffer):size);

    ReadFile(oldfd,buffer,chunksize);

    for(i=0;i<chunksize;i++)
       if(buffer[i]=='\n')
          buffer[i]=0;

    WriteFile(newfd,buffer,chunksize);

    size-=chunksize;
   }

 CloseFile(oldfd);

 /* Finish off the offset indexing and write them out */

 maxlatlonbins=nodesx->latbins*nodesx->lonbins;

 for(;latlonbin<=maxlatlonbins;latlonbin++)
    offsets[latlonbin]=number_geo;

 SeekFile(newfd,sizeof(ErrorLogsFile));
 WriteFile(newfd,offsets,(nodesx->latbins*nodesx->lonbins+1)*sizeof(index_t));

 free(offsets);

 /* Write out the header structure */

 errorlogsfile.number       =number;
 errorlogsfile.number_geo   =number_geo;
 errorlogsfile.number_nongeo=number_nongeo;

 errorlogsfile.latbins=nodesx->latbins;
 errorlogsfile.lonbins=nodesx->lonbins;

 errorlogsfile.latzero=nodesx->latzero;
 errorlogsfile.lonzero=nodesx->lonzero;

 SeekFile(newfd,0);
 WriteFile(newfd,&errorlogsfile,sizeof(ErrorLogsFile));

 CloseFile(newfd);

 /* Print the final message */

 printf_last("Wrote Errors: Geographical=%"Pindex_t" Non-geographical=%"Pindex_t,number_geo,number_nongeo);
}
