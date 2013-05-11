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


/* Data structures */

/*+ A structure containing a single object as written by the logerror_*() functions. +*/
typedef struct _ErrorLogObject
{
 uint64_t  type_id;          /*+ The type and id of the object. +*/

 uint32_t  offset;           /*+ The offset of the error message from the beginning of the text file. +*/
}
 ErrorLogObject;


/* Parsing/processing error logging functions in logerror.c */

void open_errorlog(const char *filename,int append,int bin);
void close_errorlog(void);

#ifdef __GNUC__

void logerror(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#else

void logerror(const char *format, ...);

#endif

node_t     logerror_node    (node_t     id);
way_t      logerror_way     (way_t      id);
relation_t logerror_relation(relation_t id);


#endif /* LOGERROR_H */
