# $Header: /home/amb/CVS/routino/src/Makefile,v 1.3 2009-01-03 12:25:23 amb Exp $
#
# Makefile
#
# Written by Andrew M. Bishop
#
# This file Copyright 2008,2009 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.

# Programs

CC=gcc
LD=gcc

# Program options

CFLAGS=-pg -g -O0 -Wall -Wmissing-prototypes
#LDFLAGS=-lm
LDFLAGS=-static -lm -lc -pg

# Compilation targets

C=$(wildcard *.c)
D=$(foreach f,$(C),$(addsuffix .d,$(basename $f)))

########

EXE=planetsplitter filedumper router

all : $(EXE)

########

PLANETSPLITTER_OBJ=planetsplitter.o files.o nodes.o ways.o segments.o osmparser.o

planetsplitter : $(PLANETSPLITTER_OBJ)
	$(LD) $(PLANETSPLITTER_OBJ) -o $@ $(LDFLAGS)

########

FILEDUMPER_OBJ=filedumper.o files.o nodes.o ways.o segments.o osmparser.o

filedumper : $(FILEDUMPER_OBJ)
	$(LD) $(FILEDUMPER_OBJ) -o $@ $(LDFLAGS)

########

ROUTER_OBJ=router.o files.o nodes.o ways.o segments.o osmparser.o optimiser.o

router : $(ROUTER_OBJ)
	$(LD) $(ROUTER_OBJ) -o $@ $(LDFLAGS)

########

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ -MMD -MP -MF $(addsuffix .d,$(basename $<))

########

clean:
	rm -f *.o
	rm -f *~

########

distclean: clean
	-rm -f $(EXE)
	-rm -f $(D)

########

$(D) : .FORCE
	@touch $@

include $(D)

########

.FORCE :
