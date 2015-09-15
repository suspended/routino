# tagmodifier Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2013-2015 Andrew M. Bishop
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

EXE=tagmodifier$(.EXE)

########

all: $(EXE)

########

TAGMODIFIER_OBJ=tagmodifier.o \
	        $(ROUTINO_SRC)/files.o $(ROUTINO_SRC)/logging.o $(ROUTINO_SRC)/logerror.o \
	        $(ROUTINO_SRC)/uncompress.o $(ROUTINO_SRC)/xmlparse.o $(ROUTINO_SRC)/tagging.o

ifeq ($(HOST),MINGW)
TAGMODIFIER_OBJ+=$(ROUTINO_SRC)/mman-win32.o
endif

tagmodifier$(.EXE) : $(TAGMODIFIER_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

$(ROUTINO_SRC)/%.o :
	cd $(ROUTINO_SRC) && $(MAKE) $(notdir $@)

$(DEPDIR) :
	@[ -d $(DEPDIR) ] || mkdir $(DEPDIR)

%.o : %.c $(DEPDIR)
	$(CC) -c $(CFLAGS) -DSLIM=0 -I$(ROUTINO_SRC) $< -o $@ -MMD -MP -MF $(addprefix $(DEPDIR)/,$(addsuffix .d,$(basename $@)))

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

########

distclean: clean

########

include $(D)

########

.PHONY:: all test install clean distclean
