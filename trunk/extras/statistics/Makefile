# database-stats Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2016 Andrew M. Bishop
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

# All configuration is in the top-level Makefile.conf

include ../../Makefile.conf

# Compilation targets

DEPDIR=.deps

C=$(wildcard *.c)
D=$(wildcard $(DEPDIR)/*.d)

ROUTINO_SRC=../../src

EXE=dumper$(.EXE) dumper-slim$(.EXE)

########

all : $(EXE)

########

DUMPER_OBJ=dumper.o \
	   $(ROUTINO_SRC)/nodes.o $(ROUTINO_SRC)/segments.o $(ROUTINO_SRC)/ways.o \
	   $(ROUTINO_SRC)/files.o $(ROUTINO_SRC)/fakes.o $(ROUTINO_SRC)/logging.o \
           $(ROUTINO_SRC)/sorting.o

ifeq ($(HOST),MINGW)
DUMPER_OBJ+=$(ROUTINO_SRC)/mman-win32.o
endif

dumper$(.EXE) : $(DUMPER_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

DUMPER_SLIM_OBJ=dumper-slim.o \
	   $(ROUTINO_SRC)/nodes-slim.o $(ROUTINO_SRC)/segments-slim.o $(ROUTINO_SRC)/ways-slim.o \
	   $(ROUTINO_SRC)/files.o $(ROUTINO_SRC)/fakes.o $(ROUTINO_SRC)/logging.o \
           $(ROUTINO_SRC)/sorting.o

ifeq ($(HOST),MINGW)
DUMPER_SLIM_OBJ+=$(ROUTINO_SRC)/mman-win32.o
endif

dumper-slim$(.EXE) : $(DUMPER_SLIM_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

$(ROUTINO_SRC)/%.o :
	cd $(ROUTINO_SRC) && $(MAKE) $(notdir $@)

$(ROUTINO_SRC)/%-slim.o :
	cd $(ROUTINO_SRC) && $(MAKE) $(notdir $@)

%.o : %.c
	-@[ -d $(DEPDIR) ] || mkdir $(DEPDIR)
	$(CC) -c $(CFLAGS) -DSLIM=0 -I$(ROUTINO_SRC) $< -o $@ -MMD -MP -MF $(addprefix $(DEPDIR)/,$(addsuffix .d,$(basename $@)))

%-slim.o : %.c
	-@[ -d $(DEPDIR) ] || mkdir $(DEPDIR)
	$(CC) -c $(CFLAGS) -DSLIM=1 -I$(ROUTINO_SRC) $< -o $@ -MMD -MP -MF $(addprefix $(DEPDIR)/,$(addsuffix .d,$(basename $@)))

########

test:

########

install:

########

clean:
	rm -f *~
	rm -f *.o
	rm -f $(EXE)
	rm -f $(D)
	rm -fr $(DEPDIR)
	rm -f core
	rm -f *.png

########

distclean: clean

########

include $(D)

########

.PHONY:: all test install clean distclean
