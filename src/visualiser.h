/***************************************
 $Header: /home/amb/CVS/routino/src/visualiser.h,v 1.1 2009-06-07 15:32:54 amb Exp $

 Header file for visualiser functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008,2009 Andrew M. Bishop

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


#ifndef VISUALISER_H
#define VISUALISER_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* In visualiser.c */

void OutputJunctions(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax);

void OutputSuper(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax);

void OutputOneway(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax);

void OutputSpeedLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax);

void OutputWeightLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax);

void OutputHeightLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax);
void OutputWidthLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax);
void OutputLengthLimits(Nodes *nodes,Segments *segments,Ways *ways,float latmin,float latmax,float lonmin,float lonmax);


#endif /* VISUALISER_H */
