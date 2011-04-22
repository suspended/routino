/***************************************
 $Header: /home/amb/CVS/routino/src/sorting.h,v 1.1 2010-11-27 14:56:44 amb Exp $

 Header file for sorting function prototypes

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


#ifndef SORTING_H
#define SORTING_H    /*+ To stop multiple inclusions. +*/

#include "types.h"


/* In sorting.c */

/*+ The type, size and alignment of variable to store the variable length +*/
#define FILESORT_VARINT   unsigned short
#define FILESORT_VARSIZE  sizeof(FILESORT_VARINT)
#define FILESORT_VARALIGN sizeof(void*)

void filesort_fixed(int fd_in,int fd_out,size_t itemsize,int (*compare)(const void*,const void*),int (*buildindex)(void*,index_t));

void filesort_vary(int fd_in,int fd_out,int (*compare)(const void*,const void*),int (*buildindex)(void*,index_t));

void filesort_heapsort(void **datap,size_t nitems,int(*compare)(const void*, const void*));


#endif /* SORTING_H */