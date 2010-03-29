# $Header: /home/amb/CVS/routino/src/Makefile,v 1.28 2010-03-29 18:18:54 amb Exp $
#
# Source code Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2010 Andrew M. Bishop
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Programs

CC=gcc
LD=gcc

LEX=flex

# Program options

CFLAGS=-Wall -Wmissing-prototypes
#CFLAGS+= -Wextra -pedantic -std=c99
LDFLAGS=-lm -lc

CFLAGS+= -O3
#CFLAGS+= -O0 -g
#CFLAGS+= -pg
#CFLAGS+= --coverage

LDFLAGS+=
#LDFLAGS+= -pg -static
#LDFLAGS+= --coverage

LEXFLAGS=
FLEXFLAGS=-p -B -F -8 -s

# Compilation targets

C=$(wildcard *.c)
D=$(foreach f,$(C),$(addprefix .deps/,$(addsuffix .d,$(basename $f))))

EXE=planetsplitter filedumper router

########

all : $(EXE)
	-[ -d ../web/bin ] && cp $(EXE) ../web/bin
	cd xml && $(MAKE) CC="$(CC)" LD="$(LD)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"

########

PLANETSPLITTER_OBJ=planetsplitter.o \
	           nodesx.o segmentsx.o waysx.o superx.o \
	           ways.o \
	           files.o profiles.o \
	           results.o queue.o sorting.o \
	           osmparser.o

planetsplitter : $(PLANETSPLITTER_OBJ)
	$(LD) $(PLANETSPLITTER_OBJ) -o $@ $(LDFLAGS)

########

FILEDUMPER_OBJ=filedumper.o \
	       nodes.o segments.o ways.o \
	       files.o \
               visualiser.o

filedumper : $(FILEDUMPER_OBJ)
	$(LD) $(FILEDUMPER_OBJ) -o $@ $(LDFLAGS)

########

ROUTER_OBJ=router.o \
	   nodes.o segments.o ways.o \
	   files.o profiles.o xmlparse.o \
	   optimiser.o output.o results.o queue.o

router : $(ROUTER_OBJ)
	$(LD) $(ROUTER_OBJ) -o $@ $(LDFLAGS)

########

xmlparse.c : xmlparse.l
	[ ! "$(LEX)" = "flex" ] || $(LEX) $(FLEXFLAGS) $<
	[   "$(LEX)" = "flex" ] || $(LEX) $(LEXFLAGS)  $<
	-@mv lex.yy.c xmlparse.c
	@echo Created xmlparse.c

########

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $<)))

########

clean:
	rm -f *.o
	rm -f *~
	cd xml && $(MAKE) clean

########

distclean: clean
	-[ -d ../web/bin ] && cd ../web/bin/ && rm -f $(EXE)
	-rm -f $(EXE)
	-rm -f $(D)
	-rm -fr .deps
	cd xml && $(MAKE) distclean

########

.deps : .FORCE
	@[ -d .deps ] || mkdir $@

$(D) : .FORCE .deps
	@touch $@

include $(D)

########

.FORCE :
