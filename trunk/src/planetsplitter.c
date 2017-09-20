/***************************************
 OSM planet file splitter.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2017 Andrew M. Bishop

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
#include <errno.h>

#include "version.h"

#include "types.h"
#include "ways.h"

#include "typesx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "relationsx.h"
#include "superx.h"
#include "prunex.h"

#include "files.h"
#include "logging.h"
#include "errorlogx.h"
#include "functions.h"
#include "osmparser.h"
#include "tagging.h"
#include "uncompress.h"


/* Global variables */

/*+ The name of the temporary directory. +*/
char *option_tmpdirname=NULL;

/*+ The amount of RAM to use for filesorting. +*/
ssize_t option_filesort_ramsize=0;

/*+ The number of threads to use for filesorting. +*/
int option_filesort_threads=1;


/* Local functions */

static void print_usage(int detail,const char *argerr,const char *err);


/*++++++++++++++++++++++++++++++++++++++
  The main program for the planetsplitter.
  ++++++++++++++++++++++++++++++++++++++*/

int main(int argc,char** argv)
{
 NodesX     *OSMNodes;
 SegmentsX  *OSMSegments,*SuperSegments=NULL,*MergedSegments=NULL;
 WaysX      *OSMWays;
 RelationsX *OSMRelations;
 int         iteration=0,quit=0;
 int         max_iterations=5;
 char       *dirname=NULL,*prefix=NULL,*tagging=NULL,*errorlog=NULL;
 int         option_parse_only=0,option_process_only=0;
 int         option_append=0,option_keep=0,option_changes=0;
 int         option_filenames=0;
 int         option_prune_isolated=500,option_prune_short=5,option_prune_straight=3;
 int         arg;

 printf_program_start();

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
    else if(!strncmp(argv[arg],"--sort-ram-size=",16))
       option_filesort_ramsize=atoi(&argv[arg][16]);
#if defined(USE_PTHREADS) && USE_PTHREADS
    else if(!strncmp(argv[arg],"--sort-threads=",15))
       option_filesort_threads=atoi(&argv[arg][15]);
#endif
    else if(!strncmp(argv[arg],"--tmpdir=",9))
       option_tmpdirname=&argv[arg][9];
    else if(!strncmp(argv[arg],"--tagging=",10))
       tagging=&argv[arg][10];
    else if(!strcmp(argv[arg],"--loggable"))
       option_loggable=1;
    else if(!strcmp(argv[arg],"--logtime"))
       option_logtime=1;
    else if(!strcmp(argv[arg],"--logmemory"))
       option_logmemory=1;
    else if(!strcmp(argv[arg],"--errorlog"))
       errorlog="error.log";
    else if(!strncmp(argv[arg],"--errorlog=",11))
       errorlog=&argv[arg][11];
    else if(!strcmp(argv[arg],"--parse-only"))
       option_parse_only=1;
    else if(!strcmp(argv[arg],"--process-only"))
       option_process_only=1;
    else if(!strcmp(argv[arg],"--append"))
       option_append=1;
    else if(!strcmp(argv[arg],"--keep"))
       option_keep=1;
    else if(!strcmp(argv[arg],"--changes"))
       option_changes=1;
    else if(!strncmp(argv[arg],"--max-iterations=",17))
       max_iterations=atoi(&argv[arg][17]);
    else if(!strncmp(argv[arg],"--prune",7))
      {
       if(!strcmp(&argv[arg][7],"-none"))
          option_prune_isolated=option_prune_short=option_prune_straight=0;
       else if(!strncmp(&argv[arg][7],"-isolated=",10))
          option_prune_isolated=atoi(&argv[arg][17]);
       else if(!strncmp(&argv[arg][7],"-short=",7))
          option_prune_short=atoi(&argv[arg][14]);
       else if(!strncmp(&argv[arg][7],"-straight=",10))
          option_prune_straight=atoi(&argv[arg][17]);
       else
          print_usage(0,argv[arg],NULL);
      }
    else if(argv[arg][0]=='-' && argv[arg][1]=='-')
       print_usage(0,argv[arg],NULL);
    else
       option_filenames++;
   }

 /* Check the specified command line options */

 if(option_parse_only && option_process_only)
    print_usage(0,NULL,"Cannot use '--parse-only' and '--process-only' at the same time.");

 if(option_append && option_process_only)
    print_usage(0,NULL,"Cannot use '--append' and '--process-only' at the same time.");

 if(option_filenames && option_process_only)
    print_usage(0,NULL,"Cannot use '--process-only' and filenames at the same time.");

 if(!option_filenames && !option_process_only)
    print_usage(0,NULL,"File names must be specified unless using '--process-only'.");

 if(option_filesort_ramsize<0 || option_filesort_ramsize>1024*1024)
    print_usage(0,NULL,"Sorting RAM size '--sort-ram-size=...' must be positive and in MB.");
 else if(option_filesort_ramsize==0)
   {
#if SLIM
    option_filesort_ramsize=64*1024*1024;
#else
    option_filesort_ramsize=256*1024*1024;
#endif
   }
 else
    option_filesort_ramsize*=1024*1024;

#if defined(USE_PTHREADS) && USE_PTHREADS
 if(option_filesort_threads<1 || option_filesort_threads>32)
    print_usage(0,NULL,"Sorting threads '--sort-threads=...' must be small positive integer.");
#endif

 if(!option_tmpdirname)
   {
    if(!dirname)
       option_tmpdirname=".";
    else
       option_tmpdirname=dirname;
   }

 if(!option_process_only)
   {
    if(tagging)
      {
       if(!ExistsFile(tagging))
         {
          fprintf(stderr,"Error: The '--tagging' option specifies a file '%s' that does not exist.\n",tagging);
          exit(EXIT_FAILURE);
         }
      }
    else
      {
       tagging=FileName(dirname,prefix,"tagging.xml");

       if(!ExistsFile(tagging))
         {
          char *defaulttagging=FileName(ROUTINO_DATADIR,NULL,"tagging.xml");

          if(!ExistsFile(defaulttagging))
            {
             fprintf(stderr,"Error: The '--tagging' option was not used and the files '%s' and '%s' do not exist.\n",tagging,defaulttagging);
             exit(EXIT_FAILURE);
            }

          free(tagging);
          tagging=defaulttagging;
         }
      }

    if(ParseXMLTaggingRules(tagging))
      {
       fprintf(stderr,"Error: Cannot read the tagging rules in the file '%s'.\n",tagging);
       exit(EXIT_FAILURE);
      }
   }

 /* Create new node, segment, way and relation variables */

 OSMNodes=NewNodeList(option_append||option_changes,option_process_only);

 OSMWays=NewWayList(option_append||option_changes,option_process_only);

 OSMRelations=NewRelationList(option_append||option_changes,option_process_only);

 /* Create the error log file */

 if(errorlog)
    open_errorlog(FileName(dirname,prefix,errorlog),option_append||option_changes||option_process_only,option_keep);

 /* Parse the file */

if(!option_process_only)
  {
   for(arg=1;arg<argc;arg++)
     {
      int fd;
      char *filename,*p;

      if(argv[arg][0]=='-' && argv[arg][1]=='-')
         continue;

      filename=strcpy(malloc(strlen(argv[arg])+1),argv[arg]);

      fd=OpenFile(filename);

      if((p=strstr(filename,".bz2")) && !strcmp(p,".bz2"))
        {
         fd=Uncompress_Bzip2(fd);
         *p=0;
        }

      if((p=strstr(filename,".gz")) && !strcmp(p,".gz"))
        {
         fd=Uncompress_Gzip(fd);
         *p=0;
        }

      if((p=strstr(filename,".xz")) && !strcmp(p,".xz"))
        {
         fd=Uncompress_Xz(fd);
         *p=0;
        }

      if(option_changes)
        {
         printf("\nParse OSC Data [%s]\n==============\n\n",filename);
         fflush(stdout);

         if((p=strstr(filename,".pbf")) && !strcmp(p,".pbf"))
           {
            logassert(0,"Unable to read a PBF file to apply changes (format does not permit this)");
           }
         else if((p=strstr(filename,".o5c")) && !strcmp(p,".o5c"))
           {
            if(ParseO5CFile(fd,OSMNodes,OSMWays,OSMRelations))
               exit(EXIT_FAILURE);
           }
         else
           {
            if(ParseOSCFile(fd,OSMNodes,OSMWays,OSMRelations))
               exit(EXIT_FAILURE);
           }
        }
      else
        {
         printf("\nParse OSM Data [%s]\n==============\n\n",filename);
         fflush(stdout);

         if((p=strstr(filename,".pbf")) && !strcmp(p,".pbf"))
           {
            if(ParsePBFFile(fd,OSMNodes,OSMWays,OSMRelations))
               exit(EXIT_FAILURE);
           }
         else if((p=strstr(filename,".o5m")) && !strcmp(p,".o5m"))
           {
            if(ParseO5MFile(fd,OSMNodes,OSMWays,OSMRelations))
               exit(EXIT_FAILURE);
           }
         else
           {
            if(ParseOSMFile(fd,OSMNodes,OSMWays,OSMRelations))
               exit(EXIT_FAILURE);
           }
        }

      CloseFile(fd);

      free(filename);
     }

   DeleteXMLTaggingRules();
  }

 FinishNodeList(OSMNodes);
 FinishWayList(OSMWays);
 FinishRelationList(OSMRelations);

 if(option_parse_only)
   {
    FreeNodeList(OSMNodes,1);
    FreeWayList(OSMWays,1);
    FreeRelationList(OSMRelations,1);

    exit(EXIT_SUCCESS);
   }


 /* Sort the data */

 printf("\nSort OSM Data\n=============\n\n");
 fflush(stdout);

 /* Sort the nodes, ways and relations */

 SortNodeList(OSMNodes);

 SortWayList(OSMWays);

 SortRelationList(OSMRelations);

 /* Process the data */

 printf("\nProcess OSM Data\n================\n\n");
 fflush(stdout);

 /* Remove non-highway nodes by looking through the ways (must be before splitting the ways) */

 RemoveNonHighwayNodes(OSMNodes,OSMWays,option_keep||option_changes);

 /* Separate the segments and way names and sort them (must be before processing the segments) */

 OSMSegments=SplitWays(OSMWays,OSMNodes,option_keep||option_changes);

 SortWayNames(OSMWays);

 SortSegmentList(OSMSegments);

 /* Process the segments and index them (must be before processing relations) */

 ProcessSegments(OSMSegments,OSMNodes,OSMWays);

 IndexSegments(OSMSegments,OSMNodes,OSMWays);

 /* Process the route relations and turn relations (must be before compacting the ways) */

 ProcessRouteRelations(OSMRelations,OSMWays,option_keep||option_changes);

 ProcessTurnRelations(OSMRelations,OSMNodes,OSMSegments,OSMWays,option_keep||option_changes);

 /* Compact the ways */

 CompactWayList(OSMWays,OSMSegments);

 /* Sort the nodes and segments geographically */

 SortNodeListGeographically(OSMNodes);

 SortSegmentListGeographically(OSMSegments,OSMNodes);

 /* Re-index the segments */

 IndexSegments(OSMSegments,OSMNodes,OSMWays);

 /* Sort the turn relations geographically */

 SortTurnRelationListGeographically(OSMRelations,OSMNodes,OSMSegments,0);

 /* Prune unwanted nodes/segments */

 if(option_prune_straight || option_prune_isolated || option_prune_short)
   {
    printf("\nPrune Unneeded Data\n===================\n\n");
    fflush(stdout);

    if(option_prune_straight)
      {
       StartPruning(OSMNodes,OSMSegments,OSMWays);

       PruneStraightHighwayNodes(OSMNodes,OSMSegments,OSMWays,option_prune_straight);

       FinishPruning(OSMNodes,OSMSegments,OSMWays);

       RemovePrunedNodes(OSMNodes,OSMSegments);
       RemovePrunedSegments(OSMSegments,OSMWays);
       CompactWayList(OSMWays,OSMSegments);
       RemovePrunedTurnRelations(OSMRelations,OSMNodes);

       IndexSegments(OSMSegments,OSMNodes,OSMWays);
      }

    if(option_prune_isolated)
      {
       StartPruning(OSMNodes,OSMSegments,OSMWays);

       PruneIsolatedRegions(OSMNodes,OSMSegments,OSMWays,option_prune_isolated);

       FinishPruning(OSMNodes,OSMSegments,OSMWays);

       RemovePrunedNodes(OSMNodes,OSMSegments);
       RemovePrunedSegments(OSMSegments,OSMWays);
       CompactWayList(OSMWays,OSMSegments);
       RemovePrunedTurnRelations(OSMRelations,OSMNodes);

       IndexSegments(OSMSegments,OSMNodes,OSMWays);
      }

    if(option_prune_short)
      {
       StartPruning(OSMNodes,OSMSegments,OSMWays);

       PruneShortSegments(OSMNodes,OSMSegments,OSMWays,option_prune_short);

       FinishPruning(OSMNodes,OSMSegments,OSMWays);

       RemovePrunedNodes(OSMNodes,OSMSegments);
       RemovePrunedSegments(OSMSegments,OSMWays);
       CompactWayList(OSMWays,OSMSegments);
       RemovePrunedTurnRelations(OSMRelations,OSMNodes);

       IndexSegments(OSMSegments,OSMNodes,OSMWays);
      }
   }

 /* Repeated iteration on Super-Nodes and Super-Segments */

 do
   {
    index_t nsuper;

    printf("\nProcess Super-Data (iteration %d)\n================================%s\n\n",iteration,iteration>9?"=":"");
    fflush(stdout);

    if(iteration==0)
      {
       /* Select the super-nodes */

       ChooseSuperNodes(OSMNodes,OSMSegments,OSMWays);

       /* Select the super-segments */

       SuperSegments=CreateSuperSegments(OSMNodes,OSMSegments,OSMWays);

       nsuper=OSMSegments->number;
      }
    else
      {
       SegmentsX *SuperSegments2;

       /* Index the super-segments */

       IndexSegments(SuperSegments,OSMNodes,OSMWays);

       /* Select the super-nodes */

       ChooseSuperNodes(OSMNodes,SuperSegments,OSMWays);

       /* Select the super-segments */

       SuperSegments2=CreateSuperSegments(OSMNodes,SuperSegments,OSMWays);

       nsuper=SuperSegments->number;

       FreeSegmentList(SuperSegments);

       SuperSegments=SuperSegments2;
      }

    /* Sort the super-segments and remove duplicates */

    DeduplicateSuperSegments(SuperSegments,OSMWays);

    /* Check for end condition */

    if(SuperSegments->number==nsuper)
       quit=1;

    iteration++;

    if(iteration>max_iterations)
       quit=1;
   }
 while(!quit);

 /* Combine the super-segments */

 printf("\nCombine Segments and Super-Segments\n===================================\n\n");
 fflush(stdout);

 /* Merge the super-segments */

 MergedSegments=MergeSuperSegments(OSMSegments,SuperSegments);

 FreeSegmentList(OSMSegments);

 FreeSegmentList(SuperSegments);

 OSMSegments=MergedSegments;

 /* Cross reference the nodes and segments */

 printf("\nCross-Reference Nodes and Segments\n==================================\n\n");
 fflush(stdout);

 /* Sort the nodes and segments geographically */

 SortNodeListGeographically(OSMNodes);

 SortSegmentListGeographically(OSMSegments,OSMNodes);

 /* Re-index the segments */

 IndexSegments(OSMSegments,OSMNodes,OSMWays);

 /* Sort the turn relations geographically */

 SortTurnRelationListGeographically(OSMRelations,OSMNodes,OSMSegments,1);

 /* Output the results */

 printf("\nWrite Out Database Files\n========================\n\n");
 fflush(stdout);

 /* Write out the nodes */

 SaveNodeList(OSMNodes,FileName(dirname,prefix,"nodes.mem"),OSMSegments);

 /* Write out the segments */

 SaveSegmentList(OSMSegments,FileName(dirname,prefix,"segments.mem"));

 /* Write out the ways */

 SaveWayList(OSMWays,FileName(dirname,prefix,"ways.mem"));

 /* Write out the relations */

 SaveRelationList(OSMRelations,FileName(dirname,prefix,"relations.mem"));

 /* Free the memory (delete the temporary files) */

 FreeSegmentList(OSMSegments);

 /* Close the error log file and process the data */

 if(errorlog)
   {
    close_errorlog();

    if(option_keep)
      {
       ErrorLogsX *OSMErrorLogs;

       printf("\nCreate Error Log\n================\n\n");
       fflush(stdout);

       OSMErrorLogs=NewErrorLogList();

       ProcessErrorLogs(OSMErrorLogs,OSMNodes,OSMWays,OSMRelations);

       SortErrorLogsGeographically(OSMErrorLogs);

       SaveErrorLogs(OSMErrorLogs,FileName(dirname,prefix,"errorlogs.mem"));

       FreeErrorLogList(OSMErrorLogs);
      }
   }

 /* Free the memory (delete the temporary files) */

 FreeNodeList(OSMNodes,0);
 FreeWayList(OSMWays,0);
 FreeRelationList(OSMRelations,0);

 printf("\n");
 fflush(stdout);

 printf_program_end();

 exit(EXIT_SUCCESS);
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
            "Usage: planetsplitter [--version]\n"
            "                      [--help]\n"
            "                      [--dir=<dirname>] [--prefix=<name>]\n"
#if defined(USE_PTHREADS) && USE_PTHREADS
            "                      [--sort-ram-size=<size>] [--sort-threads=<number>]\n"
#else
            "                      [--sort-ram-size=<size>]\n"
#endif
            "                      [--tmpdir=<dirname>]\n"
            "                      [--tagging=<filename>]\n"
            "                      [--loggable] [--logtime] [--logmemory]\n"
            "                      [--errorlog[=<name>]]\n"
            "                      [--parse-only | --process-only]\n"
            "                      [--append] [--keep] [--changes]\n"
            "                      [--max-iterations=<number>]\n"
            "                      [--prune-none]\n"
            "                      [--prune-isolated=<len>]\n"
            "                      [--prune-short=<len>]\n"
            "                      [--prune-straight=<len>]\n"
            "                      [<filename.osm> ... | <filename.osc> ...\n"
            "                       | <filename.pbf> ...\n"
            "                       | <filename.o5m> ... | <filename.o5c> ..."
#if defined(USE_BZIP2) && USE_BZIP2
            "\n                       | <filename.(osm|osc|o5m|o5c).bz2> ..."
#endif
#if defined(USE_GZIP) && USE_GZIP
            "\n                       | <filename.(osm|osc|o5m|o5c).gz> ..."
#endif
#if defined(USE_XZ) && USE_XZ
            "\n                       | <filename.(osm|osc|o5m|o5c).xz> ..."
#endif
            "]\n");

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
            "--sort-ram-size=<size>    The amount of RAM (in MB) to use for data sorting\n"
#if SLIM
            "                          (defaults to 64MB otherwise.)\n"
#else
            "                          (defaults to 256MB otherwise.)\n"
#endif
#if defined(USE_PTHREADS) && USE_PTHREADS
            "--sort-threads=<number>   The number of threads to use for data sorting.\n"
#endif
            "\n"
            "--tmpdir=<dirname>        The directory name for temporary files.\n"
            "                          (defaults to the '--dir' option directory.)\n"
            "\n"
            "--tagging=<filename>      The name of the XML file containing the tagging rules\n"
            "                          (defaults to 'tagging.xml' with '--dir' and\n"
            "                           '--prefix' options or the file installed in\n"
            "                           '" ROUTINO_DATADIR "').\n"
            "\n"
            "--loggable                Print progress messages suitable for logging to file.\n"
            "--logtime                 Print the elapsed time for each processing step.\n"
            "--logmemory               Print the max allocated/mapped memory for each step.\n"
            "--errorlog[=<name>]       Log parsing errors to 'error.log' or the given name\n"
            "                          (the '--dir' and '--prefix' options are applied).\n"
            "\n"
            "--parse-only              Parse the OSM/OSC file(s) and store the results.\n"
            "--process-only            Process the stored results from previous option.\n"
            "--append                  Parse the OSM file(s) and append to existing results.\n"
            "--keep                    Keep the intermediate files after parsing & sorting.\n"
            "--changes                 Parse the data as an OSC file and apply the changes.\n"
            "\n"
            "--max-iterations=<number> The number of iterations for finding super-nodes\n"
            "                          (defaults to 5).\n"
            "\n"
            "--prune-none              Disable the prune options below, they are re-enabled\n"
            "                          by adding them to the command line after this option.\n"
            "--prune-isolated=<len>    Remove access from small disconnected segment groups\n"
            "                          (defaults to removing groups under 500m).\n"
            "--prune-short=<len>       Remove short segments (defaults to removing segments\n"
            "                          up to a maximum length of 5m).\n"
            "--prune-straight=<len>    Remove nodes in almost straight highways (defaults to\n"
            "                          removing nodes up to 3m offset from a straight line).\n"
            "\n"
            "<filename.osm>, <filename.osc>, <filename.pbf>, <filename.o5m>, <filename.o5c>\n"
            "                          The name(s) of the file(s) to read and parse.\n"
            "                          Filenames ending '.pbf' read as PBF, filenames ending\n"
            "                          '.o5m' or '.o5c' read as O5M/O5C, others as XML.\n"
#if defined(USE_BZIP2) && USE_BZIP2
            "                          Filenames ending '.bz2' will be bzip2 uncompressed.\n"
#endif
#if defined(USE_GZIP) && USE_GZIP
            "                          Filenames ending '.gz' will be gzip uncompressed.\n"
#endif
#if defined(USE_XZ) && USE_XZ
            "                          Filenames ending '.xz' will be xz uncompressed.\n"
#endif
            "\n"
            "<transport> defaults to all but can be set to:\n"
            "%s"
            "\n"
            "<highway> can be selected from:\n"
            "%s"
            "\n"
            "<property> can be selected from:\n"
            "%s",
            TransportList(),HighwayList(),PropertyList());

 exit(!detail);
}
