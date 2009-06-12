# $Header: /home/amb/CVS/routino/src/Makefile,v 1.21 2009-06-12 18:36:23 amb Exp $
#
# Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2008,2009 Andrew M. Bishop
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

# Program options

CFLAGS=-O3 -Wall -Wmissing-prototypes
#CFLAGS=-O3 -g -Wall -Wmissing-prototypes
#CFLAGS=-pg -g -O0 -Wall -Wmissing-prototypes
#CFLAGS=-pg --coverage -g -O0 -Wall -Wmissing-prototypes
LDFLAGS=-lm
#LDFLAGS=-pg -static -lm -lc
#LDFLAGS=-pg --coverage -static -lm -lc

# Compilation targets

C=$(wildcard *.c)
D=$(foreach f,$(C),$(addprefix .deps/,$(addsuffix .d,$(basename $f))))

########

EXE=planetsplitter filedumper router

all : $(EXE)
	[ -d ../web/bin ] && cp $(EXE) ../web/bin

########

PLANETSPLITTER_OBJ=planetsplitter.o \
	           nodesx.o segmentsx.o waysx.o superx.o \
	           ways.o \
	           files.o profiles.o results.o osmparser.o

planetsplitter : $(PLANETSPLITTER_OBJ)
	$(LD) $(PLANETSPLITTER_OBJ) -o $@ $(LDFLAGS)

########

FILEDUMPER_OBJ=filedumper.o \
	       nodes.o segments.o ways.o results.o \
	       files.o \
               visualiser.o

filedumper : $(FILEDUMPER_OBJ)
	$(LD) $(FILEDUMPER_OBJ) -o $@ $(LDFLAGS)

########

ROUTER_OBJ=router.o \
	   nodes.o segments.o ways.o \
	   files.o profiles.o optimiser.o output.o results.o

router : $(ROUTER_OBJ)
	$(LD) $(ROUTER_OBJ) -o $@ $(LDFLAGS)

########

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $<)))

########

clean:
	rm -f *.o
	rm -f *~

########

distclean: clean
	-rm -f $(EXE)
	-rm -f $(D)
	-rm -fr .deps

########

.deps : .FORCE
	@[ -d .deps ] || mkdir $@

$(D) : .FORCE .deps
	@touch $@

include $(D)

########

.FORCE :
