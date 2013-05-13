/***************************************
 Header file for error logging function prototypes

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


#ifndef LOGERROR_H
#define LOGERROR_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>

#include "types.h"
#include "typesx.h"


/*+ A structure containing information for an error message during processing. +*/
typedef struct _ErrorLogX
{
 latlong_t latitude;         /*+ The error message latitude. +*/
 latlong_t longitude;        /*+ The error message longitude. +*/

 uint32_t  offset;           /*+ The offset of the error message from the beginning of the text file. +*/
 uint32_t  length;           /*+ The length of the error message in the text file. +*/
}
 ErrorLogX;


/*+ A structure containing information for an error message in the file. +*/
typedef struct _ErrorLog
{
 ll_off_t  latoffset;        /*+ The error message latitude offset within its bin. +*/
 ll_off_t  lonoffset;        /*+ The error message longitude offset within its bin. +*/

 uint32_t  offset;           /*+ The offset of the error message from the beginning of the text section. +*/
 uint32_t  length;           /*+ The length of the error message in the text section. +*/
}
 ErrorLog;


/*+ A structure containing the header from the error log file. +*/
typedef struct _ErrorLogsFile
{
 index_t  number;               /*+ The total number of error messages. +*/
 index_t  number_geo;           /*+ The number of error messages with a geographical location. +*/
 index_t  number_nongeo;        /*+ The number of error messages without a geographical location. +*/

 ll_bin_t latbins;              /*+ The number of bins containing latitude. +*/
 ll_bin_t lonbins;              /*+ The number of bins containing longitude. +*/

 ll_bin_t latzero;              /*+ The bin number of the furthest south bin. +*/
 ll_bin_t lonzero;              /*+ The bin number of the furthest west bin. +*/
}
 ErrorLogsFile;


/*+ A structure containing a set of error log messages read from the file. +*/
struct _ErrorLogs
{
 ErrorLogsFile file;            /*+ The header data from the file. +*/

#if !SLIM

 void     *data;                /*+ The memory mapped data in the file. +*/

 index_t  *offsets;             /*+ A pointer to the array of offsets in the file. +*/

 ErrorLog *errorlogs_geo;       /*+ A pointer to the array of geographical error logs in the file. +*/
 ErrorLog *errorlogs_nongeo;    /*+ A pointer to the array of non-geographical error logs in the file. +*/

 char     *strings;             /*+ A pointer to the array of error strings in the file. +*/

#else

 int       fd;                  /*+ The file descriptor for the file. +*/

 index_t  *offsets;             /*+ An allocated array with a copy of the file offsets. +*/

 off_t     errorlogsoffset_geo;    /*+ The offset of the geographical error logs within the file. +*/
 off_t     errorlogsoffset_nongeo; /*+ The offset of the non-geographical error logs within the file. +*/

 off_t     stringsoffset;       /*+ The offset of the error strings within the file. +*/

#endif
}
 ErrorLogs;


/* Error log processing functions in logerrorx.c */

void ProcessErrorLogs(NodesX *nodesx,WaysX *waysx,RelationsX *relationsx);
void SortErrorLogsGeographically(void);
void SaveErrorLogs(NodesX *nodesx,char *filename);


#endif /* LOGERROR_H */
