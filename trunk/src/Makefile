# $Header: /home/amb/CVS/routino/src/Makefile,v 1.8 2009-01-17 19:03:17 amb Exp $
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

#CFLAGS=-O3 -Wall -Wmissing-prototypes
CFLAGS=-pg -g -O0 -Wall -Wmissing-prototypes
#CFLAGS=-pg --coverage -g -O0 -Wall -Wmissing-prototypes
#LDFLAGS=-lm
LDFLAGS=-pg -static -lm -lc
#LDFLAGS=-pg --coverage -static -lm -lc

# Compilation targets

C=$(wildcard *.c)
D=$(foreach f,$(C),$(addsuffix .d,$(basename $f)))

########

EXE=planetsplitter filedumper router

all : $(EXE)

########

PLANETSPLITTER_OBJ=planetsplitter.o files.o nodes.o ways.o segments.o supersegments.o results.o osmparser.o optimiser.o

planetsplitter : $(PLANETSPLITTER_OBJ)
	$(LD) $(PLANETSPLITTER_OBJ) -o $@ $(LDFLAGS)

########

FILEDUMPER_OBJ=filedumper.o files.o nodes.o ways.o segments.o results.o

filedumper : $(FILEDUMPER_OBJ)
	$(LD) $(FILEDUMPER_OBJ) -o $@ $(LDFLAGS)

########

ROUTER_OBJ=router.o files.o nodes.o ways.o segments.o optimiser.o results.o

router : $(ROUTER_OBJ)
	$(LD) $(ROUTER_OBJ) -o $@ $(LDFLAGS)

########

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ -MMD -MP -MF $(addsuffix .d,$(basename $<))

%.s : %.c
	$(CC) -S $(CFLAGS) $< -o $@ -MMD -MP -MF $(addsuffix .d,$(basename $<))

########

clean:
	rm -f *.o
	rm -f *.s
	rm -f *~
	rm -f *.gcno *.gcda *.gcov

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
