/***************************************
 $Header: /home/amb/CVS/routino/src/xml/xsd-to-xmlparser.c,v 1.4 2010-04-04 14:30:44 amb Exp $

 An XML parser for simplified XML Schema Definitions to create XML parser skeletons.

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


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "xmlparse.h"


/* The local variables and functions */

int ntags=0;
xmltag **tags=NULL;
char **types=NULL;
char *currenttype=NULL;

static char *safe(const char *name);


/* The XML tag processing function prototypes */

static void xml_function(int _type_,char *version,char *encoding);
static void xsd_schema_function(int _type_,char *elementFormDefault,char *xmlns_xsd);
static void xsd_complexType_function(int _type_,char *name);
static void xsd_attribute_function(int _type_,char *name,char *type);
static void xsd_sequence_function(int _type_);
static void xsd_element_function(int _type_,char *name,char *type,char *minOccurs,char *maxOccurs);


/* The XML tag definitions */

/*+ The elementType type tag. +*/
static xmltag xsd_element_tag=
              {"xsd:element",
               {"name","type","minOccurs","maxOccurs",NULL},
               xsd_element_function,
               {NULL}};

/*+ The sequenceType type tag. +*/
static xmltag xsd_sequence_tag=
              {"xsd:sequence",
               {NULL},
               xsd_sequence_function,
               {&xsd_element_tag,NULL}};

/*+ The attributeType type tag. +*/
static xmltag xsd_attribute_tag=
              {"xsd:attribute",
               {"name","type",NULL},
               xsd_attribute_function,
               {NULL}};

/*+ The complexType type tag. +*/
static xmltag xsd_complexType_tag=
              {"xsd:complexType",
               {"name",NULL},
               xsd_complexType_function,
               {&xsd_sequence_tag,&xsd_attribute_tag,NULL}};

/*+ The schemaType type tag. +*/
static xmltag xsd_schema_tag=
              {"xsd:schema",
               {"elementFormDefault","xmlns:xsd",NULL},
               xsd_schema_function,
               {&xsd_element_tag,&xsd_complexType_tag,NULL}};

/*+ The xmlType type tag. +*/
static xmltag xml_tag=
              {"xml",
               {"version","encoding",NULL},
               xml_function,
               {NULL}};


/*+ The complete set of tags at the top level. +*/
static xmltag *xml_toplevel_tags[]={&xml_tag,&xsd_schema_tag,NULL};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the elementType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *name The contents of the 'name' attribute (or NULL if not defined).

  char *type The contents of the 'type' attribute (or NULL if not defined).

  char *minOccurs The contents of the 'minOccurs' attribute (or NULL if not defined).

  char *maxOccurs The contents of the 'maxOccurs' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void xsd_element_function(int _type_,char *name,char *type,char *minOccurs,char *maxOccurs)
{
 xmltag *tag=NULL;
 int i,j;

 if(_type_==XMLPARSE_TAG_END)
    return;

 for(i=0;i<ntags;i++)
    if(!strcmp(type,types[i]) && !strcmp(name,tags[i]->name))
       tag=tags[i];

 if(!tag)
   {
    ntags++;
    tags=(xmltag**)realloc((void*)tags,ntags*sizeof(xmltag*));
    types=(char**)realloc((void*)types,ntags*sizeof(char*));

    tags[ntags-1]=(xmltag*)calloc(1,sizeof(xmltag));
    tags[ntags-1]->name=strcpy(malloc(strlen(name)+1),name);
    types[ntags-1]=strcpy(malloc(strlen(type)+1),type);

    tag=tags[ntags-1];
   }

 if(!currenttype)
    return;

 for(i=0;i<ntags;i++)
    if(!strcmp(types[i],currenttype))
      {
       for(j=0;j<XMLPARSE_MAX_SUBTAGS;j++)
          if(!tags[i]->subtags[j])
            {
             tags[i]->subtags[j]=tag;
             break;
            }

       if(j==XMLPARSE_MAX_SUBTAGS)
         {fprintf(stderr,"Too many subtags seen for type '%s'.\n",currenttype); exit(1);}
      }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the sequenceType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

static void xsd_sequence_function(int _type_)
{
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the attributeType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *name The contents of the 'name' attribute (or NULL if not defined).

  char *type The contents of the 'type' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void xsd_attribute_function(int _type_,char *name,char *type)
{
 int i,j;

 if(_type_==XMLPARSE_TAG_END)
    return;

 for(i=0;i<ntags;i++)
    if(!strcmp(types[i],currenttype))
      {
       for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
          if(!tags[i]->attributes[j])
            {
             tags[i]->attributes[j]=strcpy(malloc(strlen(name)+1),name);
             break;
            }

       if(j==XMLPARSE_MAX_ATTRS)
         {fprintf(stderr,"Too many attributes seen for type '%s'.\n",currenttype); exit(1);}
      }
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the complexType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *name The contents of the 'name' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void xsd_complexType_function(int _type_,char *name)
{
 if(_type_==XMLPARSE_TAG_END)
    return;

 currenttype=strcpy(realloc(currenttype,strlen(name)+1),name);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the schemaType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *elementFormDefault The contents of the 'elementFormDefault' attribute (or NULL if not defined).

  char *xmlns_xsd The contents of the 'xmlns:xsd' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void xsd_schema_function(int _type_,char *elementFormDefault,char *xmlns_xsd)
{
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the xmlType XSD type is seen

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  char *version The contents of the 'version' attribute (or NULL if not defined).

  char *encoding The contents of the 'encoding' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static void xml_function(int _type_,char *version,char *encoding)
{
}


/*++++++++++++++++++++++++++++++++++++++
  The XML Schema Definition XML parser and C program generator.
  ++++++++++++++++++++++++++++++++++++++*/

int main(int argc,char **argv)
{
 int i,j,k;

 if(ParseXML(stdin,xml_toplevel_tags,0))
   {
    fprintf(stderr,"Cannot parse XML file - exiting.\n");
    exit(1);
   }

 /* Sort the tags */

 sorttags:

 for(i=0;i<ntags;i++)
   {
    for(j=0;j<XMLPARSE_MAX_SUBTAGS;j++)
       if(tags[i]->subtags[j])
         {
          for(k=0;k<XMLPARSE_MAX_SUBTAGS;k++)
             if(tags[i]->subtags[j]==tags[k])
                break;

          if(i<k)
            {
             xmltag *tempx=tags[i];
             char *tempc=types[i];

             tags[i]=tags[k];
             types[i]=types[k];

             tags[k]=tempx;
             types[k]=tempc;

             goto sorttags;
            }
         }
   }

 currenttype=NULL;
 xsd_element_function(XMLPARSE_TAG_START|XMLPARSE_TAG_END,"xml","xmlType",NULL,NULL);
 xsd_complexType_function(XMLPARSE_TAG_START,"xmlType");
 xsd_attribute_function(XMLPARSE_TAG_START|XMLPARSE_TAG_END,"version",NULL);
 xsd_attribute_function(XMLPARSE_TAG_START|XMLPARSE_TAG_END,"encoding",NULL);
 xsd_complexType_function(XMLPARSE_TAG_END,NULL);

 /* Print the header */

 printf("/***************************************\n");
 printf(" An automatically generated skeleton XML parser.\n");
 printf("\n");
 printf(" Automatically generated by xsd-to-xmlparser.\n");
 printf(" ***************************************/\n");
 printf("\n");
 printf("\n");
 printf("#include <stdio.h>\n");
 printf("\n");
 printf("#include \"xmlparse.h\"\n");

 /* Print the function prototypes */

 printf("\n");
 printf("\n");
 printf("/* The XML tag processing function prototypes */\n");
 printf("\n");

 for(i=ntags-1;i>=0;i--)
   {
    printf("static void %s_function(int _type_",safe(tags[i]->name));

    for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
       if(tags[i]->attributes[j])
          printf(",char *%s",safe(tags[i]->attributes[j]));

    printf(");\n");
   }

 /* Print the xmltag variables */

 printf("\n");
 printf("\n");
 printf("/* The XML tag definitions */\n");

 for(i=0;i<ntags;i++)
   {
    printf("\n");
    printf("/*+ The %s type tag. +*/\n",types[i]);
    printf("static xmltag %s_tag=\n",safe(tags[i]->name));
    printf("              {\"%s\",\n",tags[i]->name);

    printf("               {");
    for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
       if(tags[i]->attributes[j])
          printf("\"%s\",",tags[i]->attributes[j]);
    printf("NULL},\n");

    printf("               %s_function,\n",safe(tags[i]->name));

    printf("               {");
    for(j=0;j<XMLPARSE_MAX_SUBTAGS;j++)
       if(tags[i]->subtags[j])
          printf("&%s_tag,",safe(tags[i]->subtags[j]->name));
    printf("NULL}};\n");
   }

 printf("\n");
 printf("\n");
 printf("/*+ The complete set of tags at the top level. +*/\n");
 printf("static xmltag *xml_toplevel_tags[]={");
 printf("&%s_tag,",safe(tags[ntags-1]->name));
 printf("&%s_tag,",safe(tags[ntags-2]->name));
 printf("NULL};\n");

 /* Print the functions */

 printf("\n");
 printf("\n");
 printf("/* The XML tag processing functions */\n");

 for(i=0;i<ntags;i++)
   {
    printf("\n");
    printf("\n");
    printf("/*++++++++++++++++++++++++++++++++++++++\n");
    printf("  The function that is called when the %s XSD type is seen\n",types[i]);
    printf("\n");
    printf("  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.\n");
    for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
       if(tags[i]->attributes[j])
         {
          printf("\n");
          printf("  char *%s The contents of the '%s' attribute (or NULL if not defined).\n",safe(tags[i]->attributes[j]),tags[i]->attributes[j]);
         }
    printf("  ++++++++++++++++++++++++++++++++++++++*/\n");
    printf("\n");

    printf("static void %s_function(int _type_",safe(tags[i]->name));

    for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
       if(tags[i]->attributes[j])
          printf(",char *%s",safe(tags[i]->attributes[j]));

    printf(")\n");

    printf("{\n");

    if(i==(ntags-1))            /* XML tag */
      {
       printf(" printf(\"<?%s",tags[i]->name);
       for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
          if(tags[i]->attributes[j])
             printf(" %s=\\\"%%s\\\"",tags[i]->attributes[j]);
       printf("%s?>\\n\"",tags[i]->attributes[0]?" ":"");
       for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
          if(tags[i]->attributes[j])
             printf(",(%s?%s:\"\")",safe(tags[i]->attributes[j]),safe(tags[i]->attributes[j]));
       printf(");\n");
      }
    else
      {
       printf(" if(_type_&XMLPARSE_TAG_START)\n");
       printf("    printf(\"<%s",tags[i]->name);
       for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
          if(tags[i]->attributes[j])
             printf(" %s=\\\"%%s\\\"",tags[i]->attributes[j]);
       printf("%%s>\\n\"");
       for(j=0;j<XMLPARSE_MAX_ATTRS;j++)
          if(tags[i]->attributes[j])
             printf(",(%s?%s:\"\")",safe(tags[i]->attributes[j]),safe(tags[i]->attributes[j]));
       printf(",(_type_&XMLPARSE_TAG_END)?\" /\":\"\");\n");
       printf(" else\n");
       printf("    printf(\"</%s>\\n\");\n",tags[i]->name);
      }

    printf("}\n");
   }

 /* Print the main function */

 printf("\n");
 printf("\n");
 printf("/*++++++++++++++++++++++++++++++++++++++\n");
 printf("  A skeleton XML parser.\n");
 printf("  ++++++++++++++++++++++++++++++++++++++*/\n");
 printf("\n");
 printf("int main(int argc,char **argv)\n");
 printf("{\n");
 printf(" if(ParseXML(stdin,xml_toplevel_tags,1))\n");
 printf("    return(1);\n");
 printf(" else\n");
 printf("    return(0);\n");
 printf("}\n");

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  A function to return a safe C identifier from an XML tag or attribute name.

  char *safe Returns the safe name in a private string (only use once).

  const char *name The name to convert.
  ++++++++++++++++++++++++++++++++++++++*/

static char *safe(const char *name)
{
 static char *safe=NULL;
 int i;

 safe=realloc(safe,strlen(name)+1);

 for(i=0;name[i];i++)
    if(isalnum(name[i]))
       safe[i]=name[i];
    else
       safe[i]='_';

 safe[i]=0;

 return(safe);
}
