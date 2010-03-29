# $Header: /home/amb/CVS/routino/src/xml/Makefile,v 1.2 2010-03-29 18:16:35 amb Exp $
#
# XML test programs Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2010 Andrew M. Bishop
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

CFLAGS=-Wall -Wmissing-prototypes -O3
LDFLAGS=-lm -lc

# Compilation targets

XMLDIR=../../xml

X=$(notdir $(wildcard $(XMLDIR)/*.xsd))
C=$(foreach f,$(X),$(addsuffix -skeleton.c,$(basename $f)))
D=$(foreach f,$(C),$(addprefix .deps/,$(addsuffix .d,$(basename $f))))
O=$(foreach f,$(C),$(addsuffix .o,$(basename $f)))
E=$(foreach f,$(C),$(basename $f))

EXE=xsd-to-xmlparser

########

all : $(EXE) $(C) $(E)

########

xsd-to-xmlparser : xsd-to-xmlparser.o ../xmlparse.o
	$(LD) xsd-to-xmlparser.o ../xmlparse.o -o $@ $(LDFLAGS)

########

%-skeleton.c : $(XMLDIR)/%.xsd xsd-to-xmlparser
	./xsd-to-xmlparser < $< > $@

%-skeleton : %-skeleton.o ../xmlparse.o
	$(LD) $< ../xmlparse.o -o $@ $(LDFLAGS)

.SECONDARY : $(O)

########

../xmlparse.o : ../xmlparse.c ../xmlparse.h
	cd .. && $(MAKE) xmlparse.o

../xmlparse.c : ../xmlparse.l
	cd .. && $(MAKE) xmlparse.o

########

%.o : %.c
	$(CC) -c $(CFLAGS) -I.. $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $<)))

########

clean:
	rm -f *.o
	rm -f *~

########

distclean: clean
	-rm -f $(EXE)
	-rm -f $(E)
	-rm -f $(D)
	-rm -f $(C)
	-rm -fr .deps

########

.deps : .FORCE
	@[ -d .deps ] || mkdir $@

$(D) : .FORCE .deps
	@touch $@

include $(D)

########

.FORCE :
