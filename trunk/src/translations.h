/***************************************
 Load the translations from a file and the functions for handling them.

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


#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* Type declarations */

typedef struct _Translation
{
 char *language;

 char *raw_copyright_creator[2];
 char *raw_copyright_source[2];
 char *raw_copyright_license[2];

 char *xml_copyright_creator[2];
 char *xml_copyright_source[2];
 char *xml_copyright_license[2];

 char *xml_heading[9];
 char *xml_turn[9];
 char *xml_ordinal[10];

 char *notxml_heading[9];
 char *notxml_turn[9];
 char *notxml_ordinal[10];

 char *raw_highway[Highway_Count];

 char *xml_route_shortest;
 char *xml_route_quickest;

 char *html_waypoint;
 char *html_junction;
 char *html_roundabout;

 char *html_title;
 char *html_start;
 char *html_segment;
 char *html_node;
 char *html_rbnode;
 char *html_stop;
 char *html_total;
 char *html_subtotal;

 char *nothtml_waypoint;
 char *nothtml_junction;
 char *nothtml_roundabout;

 char *nothtml_title;
 char *nothtml_start;
 char *nothtml_segment;
 char *nothtml_node;
 char *nothtml_rbnode;
 char *nothtml_stop;
 char *nothtml_total;
 char *nothtml_subtotal;

 char *gpx_desc;
 char *gpx_name;
 char *gpx_step;
 char *gpx_final;

 char *gpx_start;
 char *gpx_inter;
 char *gpx_trip;
 char *gpx_finish;
}
 Translation;


/* Functions in translations.c */

int ParseXMLTranslations(const char *filename,const char *language,int all);

char **GetTranslationLanguages(void);

Translation *GetTranslation(const char *language);

void FreeXMLTranslations(void);


#endif /* TRANSLATIONS_H */
