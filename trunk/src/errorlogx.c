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

 printf_first("Re-indexing the Nodes, Ways and Relations: Nodes=0 Ways=0 Relations=0");

 reindex_nodes(nodesx);

 printf_middle("Re-indexing the Nodes, Ways and Relations: Nodes=%"Pindex_t" Ways=0 Relations=0",nodesx->number);

 reindex_ways(waysx);

 printf_middle("Re-indexing the Nodes, Ways and Relations: Nodes=%"Pindex_t" Ways=%"Pindex_t" Relations=0",nodesx->number,waysx->number);

 reindex_relations(relationsx);

 printf_last("Re-indexed the Nodes, Ways and Relations: Nodes=%"Pindex_t" Ways=%"Pindex_t" Relations=%"Pindex_t,nodesx->number,waysx->number,relationsx->trnumber);


 /* Print the start message */

 printf_first("Calculating Coordinates: Errors=0");

 /* Map into memory / open the files */

#if !SLIM
 nodesx->data=MapFile(nodesx->filename);
 waysx->data=MapFile(waysx->filename);
#else
 nodesx->fd=ReOpenFile(nodesx->filename);
 waysx->fd=ReOpenFile(waysx->filename);

 InvalidateNodeXCache(nodesx->cache);
 InvalidateWayXCache(waysx->cache);
#endif

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
       int i;
       latlong_t latitude=NO_LATLONG,longitude=NO_LATLONG;

       /* Calculate suitable coordinates */

       for(i=0;i<nerrorlogobjects;i++)
         {
          if(((errorlogobjects[i].type_id>>56)&0xff)=='N')
            {
             index_t node=IndexNodeX(nodesx,errorlogobjects[i].type_id&(uint64_t)0x00ffffffffffffff);

             if(node!=NO_NODE)
               {
                NodeX *nodex=LookupNodeX(nodesx,node,1);

                latitude =nodex->latitude;
                longitude=nodex->longitude;
               }
            }
         }

       /* Write to file */

       errorlogx.offset=offset;
       errorlogx.length=errorlogobject.offset-offset;

       errorlogx.latitude =latitude;
       errorlogx.longitude=longitude;

       //       printf("errorlogx.lat=%ld (%f) .lon=%ld (%f)\n",errorlogx.latitude,radians_to_degrees(latlong_to_radians(errorlogx.latitude)),errorlogx.longitude,radians_to_degrees(latlong_to_radians(errorlogx.longitude)));

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
 waysx->data=UnmapFile(waysx->data);
#else
 nodesx->fd=CloseFile(nodesx->fd);
 waysx->fd=CloseFile(waysx->fd);
#endif

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
 WayX wayx;

 waysx->number=waysx->knumber;

 waysx->idata=(way_t*)malloc(waysx->number*sizeof(way_t));
 waysx->odata=(off_t*)malloc(waysx->number*sizeof(off_t));

 /* Get the way id and the offset for each way in the file */

 size=SizeFile(waysx->filename);

 fd=ReOpenFile(waysx->filename);

 while(position<size)
   {
    FILESORT_VARINT waysize;

    SeekReadFile(fd,&waysize,FILESORT_VARSIZE,position);
    ReadFile(fd,&wayx,sizeof(WayX));

    waysx->idata[index]=wayx.id;
    waysx->odata[index]=position;

    index++;

    position+=waysize+FILESORT_VARSIZE;
   }

 CloseFile(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Re-index the relations that were kept.

  RelationsX *relationsx The set of relations to process (contains the filename and number of relations).
  ++++++++++++++++++++++++++++++++++++++*/

static void reindex_relations(RelationsX *relationsx)
{
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the error logs geographically.
  ++++++++++++++++++++++++++++++++++++++*/

void SortErrorLogsGeographically(void)
{
 int oldfd,newfd;

 /* Print the start message */

 printf_first("Sorting Errors Geographically");

 /* Re-open the file read-only and a new file writeable */

 oldfd=ReOpenFile(errorbinfilename);

 DeleteFile(errorbinfilename);

 newfd=OpenFileNew(errorbinfilename);

 /* Sort errors geographically */

 filesort_fixed(oldfd,newfd,sizeof(ErrorLogX),NULL,
                                              (int (*)(const void*,const void*))sort_by_lat_long,
                                              NULL);

 /* Close the files */

 CloseFile(oldfd);
 CloseFile(newfd);

 /* Print the final message */

 printf_last("Sorted Errors Geographically");
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
