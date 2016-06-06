/***************************************
 Header file for miscellaneous function prototypes

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2016 Andrew M. Bishop

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


#ifndef FUNCTIONS_H
#define FUNCTIONS_H    /*+ To stop multiple inclusions. +*/

#include "types.h"

#include "profiles.h"
#include "translations.h"
#include "results.h"

#include "routino.h"


/* Functions in optimiser.c */

Results *CalculateRoute(Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,
                        index_t start_node,index_t prev_segment,index_t finish_node,
                        int start_waypoint,int finish_waypoint);


/* Functions in output.c */

Routino_Output *PrintRoute(Results **results,int nresults,Nodes *nodes,Segments *segments,Ways *ways,Relations *relations,Profile *profile,Translation *translation);


#endif /* FUNCTIONS_H */
