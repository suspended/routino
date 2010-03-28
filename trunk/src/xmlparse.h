/***************************************
 $Header: /home/amb/CVS/routino/src/xmlparse.h,v 1.1 2010-03-28 15:56:49 amb Exp $

 A simple XML parser

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010 Andrew M. Bishop

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


#ifndef XMLPARSE_H
#define XMLPARSE_H    /*+ To stop multiple inclusions. +*/

#include <stdio.h>


/*+ The maximum number of attributes per tag. +*/
#define XMLPARSE_MAX_ATTRS   16

/*+ The maximum number of subtags per tag. +*/
#define XMLPARSE_MAX_SUBTAGS 16


/*+ A forward definition of the xmltag +*/
typedef struct _xmltag xmltag;


/*+ A structure to hold the definition of a tag. +*/
struct _xmltag
{
 char *name;                            /*+ The name of the tag. +*/

 char *attributes[XMLPARSE_MAX_ATTRS];  /*+ The valid attributes for the tag (null terminated). +*/

 void (*callback)();                    /*+ The callback function when the tag is seen. +*/

 xmltag *subtags[XMLPARSE_MAX_SUBTAGS]; /*+ The list of valid tags contained within this one (null terminated). +*/
};


/* XML parser function */

void ParseXML(FILE *file,xmltag **tags);


#endif /* XMLPARSE_H */
