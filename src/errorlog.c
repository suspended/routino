/***************************************
 Error log data type functions.

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


#include <stdlib.h>
#include <math.h>

#include "types.h"
#include "errorlog.h"

#include "files.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in an error log list from a file.

  ErrorLogs *LoadErrorLogs Returns the error log list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

ErrorLogs *LoadErrorLogs(const char *filename)
{
 ErrorLogs *errorlogs;

 errorlogs=(ErrorLogs*)malloc(sizeof(ErrorLogs));

#if !SLIM

 errorlogs->data=MapFile(filename);

 /* Copy the ErrorLogsFile header structure from the loaded data */

 errorlogs->file=*((ErrorLogsFile*)errorlogs->data);

 /* Set the pointers in the ErrorLogs structure. */

 errorlogs->offsets         =(index_t* )(errorlogs->data+sizeof(ErrorLogsFile));
 errorlogs->errorlogs_geo   =(ErrorLog*)(errorlogs->data+sizeof(ErrorLogsFile)+(errorlogs->file.latbins*errorlogs->file.lonbins+1)*sizeof(index_t));
 errorlogs->errorlogs_nongeo=(ErrorLog*)(errorlogs->data+sizeof(ErrorLogsFile)+(errorlogs->file.latbins*errorlogs->file.lonbins+1)*sizeof(index_t)+errorlogs->file.number_geo*sizeof(ErrorLog));
 errorlogs->strings         =(char*    )(errorlogs->data+sizeof(ErrorLogsFile)+(errorlogs->file.latbins*errorlogs->file.lonbins+1)*sizeof(index_t)+errorlogs->file.number*sizeof(ErrorLog));

#else

 errorlogs->fd=ReOpenFile(filename);

 /* Copy the ErrorLogsFile header structure from the loaded data */

 ReadFile(errorlogs->fd,&errorlogs->file,sizeof(ErrorLogsFile));

 errorlogs->offsetsoffset         =sizeof(ErrorLogsFile);
 errorlogs->errorlogsoffset_geo   =sizeof(ErrorLogsFile)+(errorlogs->file.latbins*errorlogs->file.lonbins+1)*sizeof(index_t);
 errorlogs->errorlogsoffset_nongeo=sizeof(ErrorLogsFile)+(errorlogs->file.latbins*errorlogs->file.lonbins+1)*sizeof(index_t)+errorlogs->file.number_geo*sizeof(ErrorLog);
 errorlogs->stringsoffset         =sizeof(ErrorLogsFile)+(errorlogs->file.latbins*errorlogs->file.lonbins+1)*sizeof(index_t)+errorlogs->file.number*sizeof(ErrorLog);

#endif

 return(errorlogs);
}


/*++++++++++++++++++++++++++++++++++++++
  Destroy the node list.

  ErrorLogs *errorlogs The node list to destroy.
  ++++++++++++++++++++++++++++++++++++++*/

void DestroyErrorLogs(ErrorLogs *errorlogs)
{
#if !SLIM

 errorlogs->data=UnmapFile(errorlogs->data);

#else

 errorlogs->fd=CloseFile(errorlogs->fd);

#endif

 free(errorlogs);
}
