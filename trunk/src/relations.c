/***************************************
 $Header: /home/amb/CVS/routino/src/relations.c,v 1.1 2010-12-21 14:54:56 amb Exp $

 Relation data type functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2010 Andrew M. Bishop

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


#include <sys/types.h>
#include <stdlib.h>

#include "relations.h"

#include "files.h"


/*++++++++++++++++++++++++++++++++++++++
  Load in a relation list from a file.

  Relations* LoadRelationList Returns the relation list.

  const char *filename The name of the file to load.
  ++++++++++++++++++++++++++++++++++++++*/

Relations *LoadRelationList(const char *filename)
{
 Relations *relations;

 relations=(Relations*)malloc(sizeof(Relations));

#if !SLIM

 relations->data=MapFile(filename);

 /* Copy the RelationsFile header structure from the loaded data */

 relations->file=*((RelationsFile*)relations->data);

 /* Set the pointers in the Relations structure. */

 relations->turnrelations=(TurnRelation*)(relations->data+sizeof(RelationsFile));

#else

 relations->fd=ReOpenFile(filename);

 /* Copy the RelationsFile header structure from the loaded data */

 ReadFile(relations->fd,&relations->file,sizeof(RelationsFile));

 relations->troffset=sizeof(RelationsFile);

#endif

 return(relations);
}
