%{
/***************************************
 $Header: /home/amb/CVS/routino/src/xmlparse.l,v 1.1 2010-03-28 15:23:47 amb Exp $

 A simple generic XML parser where the structure comes from the function parameters.

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
#include <stdlib.h>
#include <string.h>

/* Parser outputs */

#define LEX_TAG_BEGIN     1
#define LEX_TAG_POP       2
#define LEX_TAG_PUSH      3
#define LEX_TAG_FINISH    4
#define LEX_ATTR_KEY      5
#define LEX_ATTR_VAL      6


/* Lexer definitions */

#define YY_SKIP_YYWRAP 1 /* Remove error with prototype of ..._yywrap */
#ifndef yywrap
/*+ Needed in lex but does nothing. +*/
#define yywrap() 1
#endif

/*+ Reset the current string. +*/
#define reset_string \
 if(string) *string=0; \
 stringused=0;

/*+ append information to the current string. +*/
#define append_string(xx) \
 newlen=strlen(xx); \
 if((stringused+newlen)>=stringlen) \
    string=(char*)realloc((void*)string,stringlen=(stringused+newlen+16)); \
 strcpy(string+stringused,xx); \
 stringused+=newlen;

#define YY_NO_INPUT


/* Lexer functions and variables */

extern int yylex(void);

static char *yylval=NULL;
static int lineno=0;

%}

W               [ \t]

nonascii        [\200-\377]
ascii           [ -~]
alphanum        [a-zA-Z0-9]
punct           [][!\"#$%&\'()*+,-./:;<=>?@\\^_`{|}~]
safepunct       [][!\#$%\()*+,-./:;=?@\\^_`{|}~]

tag             ({alphanum}|[-:])+
key             ({alphanum}|[-:])+
val             ({alphanum}|{nonascii}|{safepunct})+

%x COMMENT
%x TAG_START TAG TAG_ATTR_KEY TAG_ATTR_VAL
%x DQUOTED SQUOTED

%%
 /* Must use static variables since the parser returns often. */
 static char *string=NULL;
 static int stringlen=0,stringused=0;
 int newlen;

 /* Handle comments and other tags */

"<!--"                      { BEGIN(COMMENT); }
"<"                         { BEGIN(TAG_START); }
\n                          { lineno++; }
[^<\n]+                     { }

 /* Comments - not strictly correct. */

<COMMENT>"--"{W}*">"        { BEGIN(INITIAL); }
<COMMENT>">"                { }
<COMMENT>"-"                { }
<COMMENT>\n                 { lineno++; }
<COMMENT>[^->\n]+           { }

 /* Tags */

<TAG_START>{W}+             { }
<TAG_START>"?xml"           { BEGIN(TAG); yylval=yytext; return(LEX_TAG_BEGIN); }
<TAG_START>{tag}            { BEGIN(TAG); yylval=yytext; return(LEX_TAG_BEGIN); }

<TAG_START>"/"{tag}">"      { BEGIN(INITIAL); return(LEX_TAG_POP); }

<TAG_START>\n               { BEGIN(INITIAL); lineno++; }
<TAG_START>.                { BEGIN(INITIAL); }

<TAG>{W}+                   { }
<TAG>"/>"                   { BEGIN(INITIAL); return(LEX_TAG_FINISH); }
<TAG>"?>"                   { BEGIN(INITIAL); return(LEX_TAG_FINISH); }
<TAG>">"                    { BEGIN(INITIAL); return(LEX_TAG_PUSH); }
<TAG>{key}                  { BEGIN(TAG_ATTR_KEY); yylval=yytext; return(LEX_ATTR_KEY); }
<TAG>\n                     { lineno++; }
<TAG>.                      { }

<TAG_ATTR_KEY>{W}*=         { BEGIN(TAG_ATTR_VAL); }
<TAG_ATTR_KEY>\n            { lineno++; }
<TAG_ATTR_KEY>.             { BEGIN(TAG); unput(yytext[0]); yylval=NULL; return(LEX_ATTR_VAL); }

<TAG_ATTR_VAL>{W}+          { }
<TAG_ATTR_VAL>\"            { BEGIN(DQUOTED); reset_string; }
<TAG_ATTR_VAL>\'            { BEGIN(SQUOTED); reset_string; }
<TAG_ATTR_VAL>{val}         { BEGIN(TAG);                   yylval=yytext; return(LEX_ATTR_VAL); }
<TAG_ATTR_VAL>\n            { lineno++; }
<TAG_ATTR_VAL>.             { BEGIN(TAG); unput(yytext[0]); yylval=NULL;   return(LEX_ATTR_VAL); }

 /* Quoted strings */

<DQUOTED>\\\\               { append_string(yytext); }
<DQUOTED>\\\"               { append_string(yytext); }
<DQUOTED>\\                 { append_string(yytext); }
<DQUOTED>\"                 { BEGIN(TAG); yylval=string; return(LEX_ATTR_VAL); }
<DQUOTED>\n                 { lineno++; }
<DQUOTED>[^\\\"\n]+         { append_string(yytext); }

<SQUOTED>\\\\               { append_string(yytext); }
<SQUOTED>\\\'               { append_string(yytext); }
<SQUOTED>\\                 { append_string(yytext); }
<SQUOTED>\'                 { BEGIN(TAG); yylval=string; return(LEX_ATTR_VAL); }
<SQUOTED>\n                 { lineno++; }
<SQUOTED>[^\\\'\n]+         { append_string(yytext); }

 /* End of file */

<<EOF>>                     { free(string); string=NULL; BEGIN(INITIAL); return(0); }

%%

#include "xmlparse.h"


/*++++++++++++++++++++++++++++++++++++++
  Parse the XML and call the functions for each tag as seen.

  FILE *file The file to parse.

  xmltag **tags The array of pointers to tags for the top level.
  ++++++++++++++++++++++++++++++++++++++*/

void ParseXML(FILE *file,xmltag **tags)
{
 int yychar,i;

 int nattributes=0;
 char *attributes[XMLPARSE_MAX_ATTRS];
 int attribute=0;

 int stackdepth=0,stackused=0;
 xmltag ***tagstack=NULL;
 xmltag *tag=NULL;

 static int first=1;

 /* Parser (re)-initialisation */

 yyin=file;

 if(!first)
    yyrestart(NULL);

 first=0;

 lineno=1;

 /* The actual parser. */

 while((yychar=yylex()))
    switch(yychar)
      {
       /* The start of a tag for an element */

      case LEX_TAG_BEGIN:

       tag=NULL;

       for(i=0;tags[i];i++)
          if(!strcasecmp(yylval,tags[i]->name))
            {
             tag=tags[i];

             for(i=0;i<nattributes;i++)
                free(attributes[i]);

             for(i=0;i<XMLPARSE_MAX_ATTRS;i++)
                if(!tag->attributes[i])
                   break;

             nattributes=i;

             for(i=0;i<nattributes;i++)
                attributes[i]=NULL;

             break;
            }

       if(tag==NULL)
         {
          fprintf(stderr,"XML Parser: Error unexpected tag '%s' on line %d.\n",yylval,lineno);
          exit(1);
         }
       break;

       /* The end of the start-tag for an element */

      case LEX_TAG_PUSH:

       if(stackused==stackdepth)
          tagstack=(xmltag***)realloc((void*)tagstack,(stackdepth+=8)*sizeof(xmltag**));

       tagstack[stackused++]=tags;
       tags=tag->subtags;

       /* The end of the empty-element-tag for an element */

      case LEX_TAG_FINISH:

       if(tag->callback)
          switch(nattributes)
            {
            case  0: (*tag->callback)(); break;
            case  1: (*tag->callback)(attributes[0]); break;
            case  2: (*tag->callback)(attributes[0],attributes[1]); break;
            case  3: (*tag->callback)(attributes[0],attributes[1],attributes[2]); break;
            case  4: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3]); break;
            case  5: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4]); break;
            case  6: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5]); break;
            case  7: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6]); break;
            case  8: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7]); break;
            case  9: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8]); break;
            case 10: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9]); break;
            case 11: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10]); break;
            case 12: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11]); break;
            case 13: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12]); break;
            case 14: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13]); break;
            case 15: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13],attributes[14]); break;
            case 16: (*tag->callback)(attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13],attributes[14],attributes[15]); break;

            default:
             fprintf(stderr,"XML Parser: Error too many attributes for tag '%s' on line %d.\n",tag->name,lineno);
             exit(1);
            }

       tag=NULL;

       break;

       /* The end of the end-tag for an element */

      case LEX_TAG_POP:

       tags=tagstack[--stackused];
       tag=NULL;

       break;

       /* An attribute key */

      case LEX_ATTR_KEY:

       attribute=-1;

       for(i=0;i<nattributes;i++)
          if(!strcasecmp(yylval,tag->attributes[i]))
            {
             attribute=i;

             break;
            }

       if(attribute==-1)
         {
          fprintf(stderr,"XML Parser: Error unexpected attribute '%s' for tag '%s' on line %d.\n",yylval,tag->name,lineno);
          exit(1);
         }
       break;

       /* An attribute value */

      case LEX_ATTR_VAL:

       if(yylval)
          attributes[attribute]=strcpy(malloc(strlen(yylval)+1),yylval);
      }

 /* Delete the tagdata */

 for(i=0;i<nattributes;i++)
    if(attributes[i])
       free(attributes[i]);

 if(stackdepth)
    free(tagstack);
}
