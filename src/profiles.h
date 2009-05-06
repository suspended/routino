/***************************************
 $Header: /home/amb/CVS/routino/src/profiles.h,v 1.8 2009-05-06 18:26:41 amb Exp $

 A header file for the profiles.

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


#ifndef PROFILES_H
#define PROFILES_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* Data structures */

/*+ A data structure to hold a transport type profile. +*/
typedef struct _Profile
{
 Transport transport;           /*+ The type of transport. +*/

 Allowed   allow;               /*+ The type of transport expressed as what must be allowed on a way. +*/

 score_t   highway[Way_Unknown];/*+ A floating point preference for travel on the highway. +*/

 speed_t   speed[Way_Unknown];  /*+ The maximum speed on each type of highway. +*/

 int       oneway;              /*+ A flag to indicate if one-way restrictions apply. +*/

 weight_t  weight;              /*+ The minimum weight of the route. +*/

 height_t  height;              /*+ The minimum height of vehicles on the route. +*/
 width_t   width;               /*+ The minimum width of vehicles on the route. +*/
 length_t  length;              /*+ The minimum length of vehicles on the route. +*/
}
 Profile;


/* Functions */

Profile *GetProfile(Transport transport);

void UpdateProfile(Profile *profile);

void PrintProfile(const Profile *profile);

void PrintProfilesJS(void);

void PrintProfilesPerl(void);

#endif /* PROFILES_H */
