/***************************************
 Load the tagging rules from a file and the functions for handling them.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010-2015 Andrew M. Bishop

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
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>

#include "files.h"
#include "tagging.h"
#include "xmlparse.h"
#include "logging.h"


/* Constants */

#define TAGACTION_IF       1
#define TAGACTION_IFNOT    2

#define TAGACTION_INHERIT  3    /* Not a real action, just a marker */

#define TAGACTION_SET      4
#define TAGACTION_UNSET    5
#define TAGACTION_OUTPUT   6
#define TAGACTION_LOGERROR 7

static const char* const default_logerror_message="ignoring it";


/* Local variable (intialised before each use) */

static int64_t current_id;

/* Local parsing variables (re-initialised by DeleteXMLTaggingRules() function) */

static TaggingRuleList NodeRules={NULL,0};
static TaggingRuleList WayRules={NULL,0};
static TaggingRuleList RelationRules={NULL,0};

static int current_list_stack_depth=0;
static TaggingRuleList **current_list_stack=NULL;
static TaggingRuleList *current_list=NULL;

/* Local parsing functions */

static TaggingRuleList *AppendTaggingRule(TaggingRuleList *rules,const char *k,const char *v,int action);
static void AppendTaggingAction(TaggingRuleList *rules,const char *k,const char *v,int action,const char *message);
static void DeleteTaggingRuleList(TaggingRuleList *rules);

static void ApplyRules(TaggingRuleList *rules,TagList *input,TagList *output,const char *match_k,const char *match_v);


/* The XML tag processing function prototypes */

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding);
//static int RoutinoTaggingType_function(const char *_tag_,int _type_);
static int NodeType_function(const char *_tag_,int _type_);
static int WayType_function(const char *_tag_,int _type_);
static int RelationType_function(const char *_tag_,int _type_);
static int IfType_function(const char *_tag_,int _type_,const char *k,const char *v);
static int IfNotType_function(const char *_tag_,int _type_,const char *k,const char *v);
static int SetType_function(const char *_tag_,int _type_,const char *k,const char *v);
static int UnsetType_function(const char *_tag_,int _type_,const char *k);
static int OutputType_function(const char *_tag_,int _type_,const char *k,const char *v);
static int LogErrorType_function(const char *_tag_,int _type_,const char *k,const char *v,const char *message);


/* The XML tag definitions (forward declarations) */

static const xmltag xmlDeclaration_tag;
static const xmltag RoutinoTaggingType_tag;
static const xmltag NodeType_tag;
static const xmltag WayType_tag;
static const xmltag RelationType_tag;
static const xmltag IfType_tag;
static const xmltag IfNotType_tag;
static const xmltag SetType_tag;
static const xmltag UnsetType_tag;
static const xmltag OutputType_tag;
static const xmltag LogErrorType_tag;


/* The XML tag definition values */

/*+ The complete set of tags at the top level. +*/
static const xmltag * const xml_toplevel_tags[]={&xmlDeclaration_tag,&RoutinoTaggingType_tag,NULL};

/*+ The xmlDeclaration type tag. +*/
static const xmltag xmlDeclaration_tag=
              {"xml",
               2, {"version","encoding"},
               NULL,
               {NULL}};

/*+ The RoutinoTaggingType type tag. +*/
static const xmltag RoutinoTaggingType_tag=
              {"routino-tagging",
               0, {NULL},
               NULL,
               {&NodeType_tag,&WayType_tag,&RelationType_tag,NULL}};

/*+ The NodeType type tag. +*/
static const xmltag NodeType_tag=
              {"node",
               0, {NULL},
               NodeType_function,
               {&IfType_tag,&IfNotType_tag,NULL}};

/*+ The WayType type tag. +*/
static const xmltag WayType_tag=
              {"way",
               0, {NULL},
               WayType_function,
               {&IfType_tag,&IfNotType_tag,NULL}};

/*+ The RelationType type tag. +*/
static const xmltag RelationType_tag=
              {"relation",
               0, {NULL},
               RelationType_function,
               {&IfType_tag,&IfNotType_tag,NULL}};

/*+ The IfType type tag. +*/
static const xmltag IfType_tag=
              {"if",
               2, {"k","v"},
               IfType_function,
               {&IfType_tag,&IfNotType_tag,&SetType_tag,&UnsetType_tag,&OutputType_tag,&LogErrorType_tag,NULL}};

/*+ The IfNotType type tag. +*/
static const xmltag IfNotType_tag=
              {"ifnot",
               2, {"k","v"},
               IfNotType_function,
               {&IfType_tag,&IfNotType_tag,&SetType_tag,&UnsetType_tag,&OutputType_tag,&LogErrorType_tag,NULL}};

/*+ The SetType type tag. +*/
static const xmltag SetType_tag=
              {"set",
               2, {"k","v"},
               SetType_function,
               {NULL}};

/*+ The UnsetType type tag. +*/
static const xmltag UnsetType_tag=
              {"unset",
               1, {"k"},
               UnsetType_function,
               {NULL}};

/*+ The OutputType type tag. +*/
static const xmltag OutputType_tag=
              {"output",
               2, {"k","v"},
               OutputType_function,
               {NULL}};

/*+ The LogErrorType type tag. +*/
static const xmltag LogErrorType_tag=
              {"logerror",
               3, {"k","v","message"},
               LogErrorType_function,
               {NULL}};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the XML declaration is seen

  int xmlDeclaration_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *version The contents of the 'version' attribute (or NULL if not defined).

  const char *encoding The contents of the 'encoding' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the RoutinoTaggingType XSD type is seen

  int RoutinoTaggingType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int RoutinoTaggingType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the NodeType XSD type is seen

  int NodeType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

static int NodeType_function(const char *_tag_,int _type_)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    current_list_stack_depth=0;
    current_list=&NodeRules;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the WayType XSD type is seen

  int WayType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

static int WayType_function(const char *_tag_,int _type_)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    current_list_stack_depth=0;
    current_list=&WayRules;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the RelationType XSD type is seen

  int RelationType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

static int RelationType_function(const char *_tag_,int _type_)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    current_list_stack_depth=0;
    current_list=&RelationRules;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the IfType XSD type is seen

  int IfType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int IfType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    if(!current_list_stack || (current_list_stack_depth%8)==7)
       current_list_stack=(TaggingRuleList**)realloc((void*)current_list_stack,(current_list_stack_depth+8)*sizeof(TaggingRuleList*));

    current_list_stack[current_list_stack_depth++]=current_list;

    current_list=AppendTaggingRule(current_list,k,v,TAGACTION_IF);
   }

 if(_type_&XMLPARSE_TAG_END)
    current_list=current_list_stack[--current_list_stack_depth];

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the IfNotType XSD type is seen

  int IfNotType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int IfNotType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    if(!current_list_stack || (current_list_stack_depth%8)==7)
       current_list_stack=(TaggingRuleList**)realloc((void*)current_list_stack,(current_list_stack_depth+8)*sizeof(TaggingRuleList*));

    current_list_stack[current_list_stack_depth++]=current_list;

    current_list=AppendTaggingRule(current_list,k,v,TAGACTION_IFNOT);
   }

 if(_type_&XMLPARSE_TAG_END)
    current_list=current_list_stack[--current_list_stack_depth];

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the SetType XSD type is seen

  int SetType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int SetType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START)
    AppendTaggingAction(current_list,k,v,TAGACTION_SET,NULL);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the UnsetType XSD type is seen

  int UnsetType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int UnsetType_function(const char *_tag_,int _type_,const char *k)
{
 if(_type_&XMLPARSE_TAG_START)
    AppendTaggingAction(current_list,k,NULL,TAGACTION_UNSET,NULL);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the OutputType XSD type is seen

  int OutputType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int OutputType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START)
    AppendTaggingAction(current_list,k,v,TAGACTION_OUTPUT,NULL);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the LogErrorType XSD type is seen

  int LogErrorType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).

  const char *message The contents of the 'message' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int LogErrorType_function(const char *_tag_,int _type_,const char *k,const char *v,const char *message)
{
 if(_type_&XMLPARSE_TAG_START)
    AppendTaggingAction(current_list,k,v,TAGACTION_LOGERROR,message);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The XML tagging rules parser.

  int ParseXMLTaggingRules Returns 0 if OK or something else in case of an error.

  const char *filename The name of the file to read.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXMLTaggingRules(const char *filename)
{
 int fd;
 int retval;

 if(!ExistsFile(filename))
   {
    fprintf(stderr,"Error: Specified tagging rules file '%s' does not exist.\n",filename);
    return(1);
   }

 fd=OpenFile(filename);

 /* Initialise variables used for parsing */

 retval=ParseXML(fd,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_ERRNONAME);

 CloseFile(fd);

 if(current_list_stack)
    free(current_list_stack);

 if(retval)
    return(1);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Delete the tagging rules loaded from the XML file.
  ++++++++++++++++++++++++++++++++++++++*/

void DeleteXMLTaggingRules(void)
{
 current_list_stack_depth=0;
 current_list_stack=NULL;
 current_list=NULL;

 DeleteTaggingRuleList(&NodeRules);
 DeleteTaggingRuleList(&WayRules);
 DeleteTaggingRuleList(&RelationRules);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a tagging rule to the list of rules.

  TaggingRuleList *AppendTaggingRule Returns the new TaggingRuleList inside the new TaggingRule.

  TaggingRuleList *rules The list of rules to add to.

  const char *k The tag key.

  const char *v The tag value.

  int action Set to the type of action.
  ++++++++++++++++++++++++++++++++++++++*/

TaggingRuleList *AppendTaggingRule(TaggingRuleList *rules,const char *k,const char *v,int action)
{
 if((rules->nrules%16)==0)
    rules->rules=(TaggingRule*)realloc((void*)rules->rules,(rules->nrules+16)*sizeof(TaggingRule));

 rules->nrules++;

 rules->rules[rules->nrules-1].action=action;

 if(k)
    rules->rules[rules->nrules-1].k=strcpy(malloc(strlen(k)+1),k);
 else
    rules->rules[rules->nrules-1].k=NULL;

 if(v)
    rules->rules[rules->nrules-1].v=strcpy(malloc(strlen(v)+1),v);
 else
    rules->rules[rules->nrules-1].v=NULL;

 rules->rules[rules->nrules-1].message=NULL;

 rules->rules[rules->nrules-1].rulelist=(TaggingRuleList*)calloc(sizeof(TaggingRuleList),1);

 return(rules->rules[rules->nrules-1].rulelist);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a tagging action to the list of rules.

  TaggingRuleList *rules The list of rules to add to.

  const char *k The tag key.

  const char *v The tag value.

  int action Set to the type of action.

  const char *message The message to use for the logerror action.
  ++++++++++++++++++++++++++++++++++++++*/

static void AppendTaggingAction(TaggingRuleList *rules,const char *k,const char *v,int action,const char *message)
{
 if((rules->nrules%16)==0)
    rules->rules=(TaggingRule*)realloc((void*)rules->rules,(rules->nrules+16)*sizeof(TaggingRule));

 rules->nrules++;

 rules->rules[rules->nrules-1].action=action;

 if(k)
    rules->rules[rules->nrules-1].k=strcpy(malloc(strlen(k)+1),k);
 else
    rules->rules[rules->nrules-1].k=NULL;

 if(v)
    rules->rules[rules->nrules-1].v=strcpy(malloc(strlen(v)+1),v);
 else
    rules->rules[rules->nrules-1].v=NULL;

 if(message)
    rules->rules[rules->nrules-1].message=strcpy(malloc(strlen(message)+1),message);
 else
    rules->rules[rules->nrules-1].message=(char*)default_logerror_message;

 rules->rules[rules->nrules-1].rulelist=NULL;
}


/*++++++++++++++++++++++++++++++++++++++
  Delete a tagging rule.

  TaggingRuleList *rules The list of rules to be deleted.
  ++++++++++++++++++++++++++++++++++++++*/

void DeleteTaggingRuleList(TaggingRuleList *rules)
{
 int i;

 for(i=0;i<rules->nrules;i++)
   {
    if(rules->rules[i].k)
       free(rules->rules[i].k);
    if(rules->rules[i].v)
       free(rules->rules[i].v);
    if(rules->rules[i].message && rules->rules[i].message!=default_logerror_message)
       free(rules->rules[i].message);

    if(rules->rules[i].rulelist)
      {
       DeleteTaggingRuleList(rules->rules[i].rulelist);
       free(rules->rules[i].rulelist);
      }
   }

 if(rules->rules)
    free(rules->rules);

 rules->rules=NULL;
 rules->nrules=0;
}


/*++++++++++++++++++++++++++++++++++++++
  Create a new TagList structure.

  TagList *NewTagList Returns the new allocated TagList.
  ++++++++++++++++++++++++++++++++++++++*/

TagList *NewTagList(void)
{
 return((TagList*)calloc(sizeof(TagList),1));
}


/*++++++++++++++++++++++++++++++++++++++
  Delete a tag list and the contents.

  TagList *tags The list of tags to delete.
  ++++++++++++++++++++++++++++++++++++++*/

void DeleteTagList(TagList *tags)
{
 int i;

 for(i=0;i<tags->ntags;i++)
   {
    if(tags->k[i]) free(tags->k[i]);
    if(tags->v[i]) free(tags->v[i]);
   }

 if(tags->k) free(tags->k);
 if(tags->v) free(tags->v);

 free(tags);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a tag to the list of tags.

  TagList *tags The list of tags to add to.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendTag(TagList *tags,const char *k,const char *v)
{
 if((tags->ntags%8)==0)
   {
    int i;

    tags->k=(char**)realloc((void*)tags->k,(tags->ntags+8)*sizeof(char*));
    tags->v=(char**)realloc((void*)tags->v,(tags->ntags+8)*sizeof(char*));

    for(i=tags->ntags;i<(tags->ntags+8);i++)
       tags->k[i]=tags->v[i]=NULL;
   }

 tags->k[tags->ntags]=strcpy(realloc(tags->k[tags->ntags],strlen(k)+1),k);
 tags->v[tags->ntags]=strcpy(realloc(tags->v[tags->ntags],strlen(v)+1),v);

 tags->ntags++;
}


/*++++++++++++++++++++++++++++++++++++++
  Modify an existing tag or append a new tag to the list of tags.

  TagList *tags The list of tags to modify.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

void ModifyTag(TagList *tags,const char *k,const char *v)
{
 int i;

 for(i=0;i<tags->ntags;i++)
    if(!strcmp(tags->k[i],k))
      {
       tags->v[i]=strcpy(realloc(tags->v[i],strlen(v)+1),v);
       return;
      }

 AppendTag(tags,k,v);
}


/*++++++++++++++++++++++++++++++++++++++
  Delete an existing tag from the list of tags.

  TagList *tags The list of tags to modify.

  const char *k The tag key.
  ++++++++++++++++++++++++++++++++++++++*/

void DeleteTag(TagList *tags,const char *k)
{
 int i,j;

 for(i=0;i<tags->ntags;i++)
    if(!strcmp(tags->k[i],k))
      {
       if(tags->k[i]) free(tags->k[i]);
       if(tags->v[i]) free(tags->v[i]);

       for(j=i+1;j<tags->ntags;j++)
         {
          tags->k[j-1]=tags->k[j];
          tags->v[j-1]=tags->v[j];
         }

       tags->ntags--;

       tags->k[tags->ntags]=NULL;
       tags->v[tags->ntags]=NULL;

       return;
      }
}


/*++++++++++++++++++++++++++++++++++++++
  Create a string containing all of the tags formatted as if HTML.

  char *StringifyTag Returns a static pointer to the created string.

  TagList *tags The list of tags to convert.
  ++++++++++++++++++++++++++++++++++++++*/

char *StringifyTag(TagList *tags)
{
 static char *string=NULL; /* static allocation of return value */
 int i,length=0,used=0;

 for(i=0;i<tags->ntags;i++)
   {
    length+=strlen(tags->k[i]);
    length+=strlen(tags->v[i]);

    length+=16;
   }

 string=realloc((char*)string,length);

 for(i=0;i<tags->ntags;i++)
    used+=sprintf(string+used,"<tag k='%s' v='%s'>",tags->k[i],tags->v[i]);

 return(string);
}


/*++++++++++++++++++++++++++++++++++++++
  Apply a set of tagging rules to a set of node tags.

  TagList *ApplyNodeTaggingRules Returns the list of output tags after modification.

  TagList *tags The tags to be modified.

  int64_t id The ID of the node.
  ++++++++++++++++++++++++++++++++++++++*/

TagList *ApplyNodeTaggingRules(TagList *tags,int64_t id)
{
 TagList *result=NewTagList();

 current_id=id;
 current_list=&NodeRules;

 ApplyRules(current_list,tags,result,NULL,NULL);

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Apply a set of tagging rules to a set of way tags.

  TagList *ApplyWayTaggingRules Returns the list of output tags after modification.

  TagList *tags The tags to be modified.

  int64_t id The ID of the way.
  ++++++++++++++++++++++++++++++++++++++*/

TagList *ApplyWayTaggingRules(TagList *tags,int64_t id)
{
 TagList *result=NewTagList();

 current_id=id;
 current_list=&WayRules;

 ApplyRules(current_list,tags,result,NULL,NULL);

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Apply a set of tagging rules to a set of relation tags.

  TagList *ApplyRelationTaggingRules Returns the list of output tags after modification.

  TagList *tags The tags to be modified.

  int64_t id The ID of the relation.
  ++++++++++++++++++++++++++++++++++++++*/

TagList *ApplyRelationTaggingRules(TagList *tags,int64_t id)
{
 TagList *result=NewTagList();

 current_id=id;
 current_list=&RelationRules;

 ApplyRules(current_list,tags,result,NULL,NULL);

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Apply a set of rules to a matching tag.

  TaggingRuleList *rules The rules that are to be matched.

  TagList *input The input tags.

  TagList *output The output tags.

  const char *match_k The key matched at the higher level rule.

  const char *match_v The value matched at the higher level rule.
  ++++++++++++++++++++++++++++++++++++++*/

static void ApplyRules(TaggingRuleList *rules,TagList *input,TagList *output,const char *match_k,const char *match_v)
{
 int i,j;
 char *match_k_copy=NULL,*match_v_copy=NULL;
 
 if(match_k)
    match_k_copy=strcpy(malloc(strlen(match_k)+1),match_k);

 if(match_v)
    match_v_copy=strcpy(malloc(strlen(match_v)+1),match_v);

 for(i=0;i<rules->nrules;i++)
   {
    const char *k,*v;

    k=rules->rules[i].k;

    if(!k && rules->rules[i].action >= TAGACTION_INHERIT)
       k=match_k_copy;

    v=rules->rules[i].v;

    if(!v && rules->rules[i].action >= TAGACTION_INHERIT)
       v=match_v_copy;

    switch(rules->rules[i].action)
      {
      case TAGACTION_IF:
       if(k && v)
         {
          for(j=0;j<input->ntags;j++)
             if(!strcmp(input->k[j],k) && !strcmp(input->v[j],v))
                ApplyRules(rules->rules[i].rulelist,input,output,input->k[j],input->v[j]);
         }
       else if(k && !v)
         {
          for(j=0;j<input->ntags;j++)
             if(!strcmp(input->k[j],k))
                ApplyRules(rules->rules[i].rulelist,input,output,input->k[j],input->v[j]);
         }
       else if(!k && v)
         {
          for(j=0;j<input->ntags;j++)
             if(!strcmp(input->v[j],v))
                ApplyRules(rules->rules[i].rulelist,input,output,input->k[j],input->v[j]);
         }
       else /* if(!k && !v) */
         {
          if(!input->ntags)
             ApplyRules(rules->rules[i].rulelist,input,output,"","");
          else
             for(j=0;j<input->ntags;j++)
                ApplyRules(rules->rules[i].rulelist,input,output,input->k[j],input->v[j]);
         }
       break;

      case TAGACTION_IFNOT:
       if(k && v)
         {
          for(j=0;j<input->ntags;j++)
             if(!strcmp(input->k[j],k) && !strcmp(input->v[j],v))
                break;

          if(j!=input->ntags)
             break;
         }
       else if(k && !v)
         {
          for(j=0;j<input->ntags;j++)
             if(!strcmp(input->k[j],k))
                break;

          if(j!=input->ntags)
             break;
         }
       else if(!k && v)
         {
          for(j=0;j<input->ntags;j++)
             if(!strcmp(input->v[j],v))
                break;

          if(j!=input->ntags)
             break;
         }
       else /* if(!k && !v) */
         {
          break;
         }

       ApplyRules(rules->rules[i].rulelist,input,output,k,v);
       break;

      case TAGACTION_SET:
       ModifyTag(input,k,v);
       break;

      case TAGACTION_UNSET:
       DeleteTag(input,k);
       break;

      case TAGACTION_OUTPUT:
       ModifyTag(output,k,v);
       break;

      case TAGACTION_LOGERROR:
       if(rules->rules[i].k && !rules->rules[i].v)
          for(j=0;j<input->ntags;j++)
             if(!strcmp(input->k[j],rules->rules[i].k))
               {
                v=input->v[j];
                break;
               }

       if(current_list==&NodeRules)
          logerror("Node %"Pnode_t" has an unrecognised tag '%s' = '%s' (in tagging rules); %s.\n",logerror_node(current_id),k,v,rules->rules[i].message);
       if(current_list==&WayRules)
          logerror("Way %"Pway_t" has an unrecognised tag '%s' = '%s' (in tagging rules); %s.\n",logerror_way(current_id),k,v,rules->rules[i].message);
       if(current_list==&RelationRules)
          logerror("Relation %"Prelation_t" has an unrecognised tag '%s' = '%s' (in tagging rules); %s.\n",logerror_relation(current_id),k,v,rules->rules[i].message);
      }
   }

 if(match_k_copy) free(match_k_copy);
 if(match_v_copy) free(match_v_copy);
}
