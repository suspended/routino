/***************************************
 $Header: /home/amb/CVS/routino/src/functions.h,v 1.59 2010-11-27 14:56:37 amb Exp $

 Header file for function prototypes

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


#ifndef FUNCTIONS_H
#define FUNCTIONS_H    /*+ To stop multiple inclusions. +*/

#include "types.h"

#include "profiles.h"
#include "results.h"


/* In optimiser.c */

Results *FindNormalRoute(Nodes *nodes,Segments *segments,Ways *ways,index_t start,index_t finish,Profile *profile);
Results *FindMiddleRoute(Nodes *supernodes,Segments *supersegments,Ways *superways,Results *begin,Results *end,Profile *profile);

Results *FindStartRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t start,Profile *profile);
Results *FindFinishRoutes(Nodes *nodes,Segments *segments,Ways *ways,index_t finish,Profile *profile);

Results *CombineRoutes(Results *results,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile);

void FixForwardRoute(Results *results,index_t finish);


/* In output.c */

void PrintRoute(Results **results,int nresults,Nodes *nodes,Segments *segments,Ways *ways,Profile *profile);


#endif /* FUNCTIONS_H */
