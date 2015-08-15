# find-fixme Makefile
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

# Web file paths

WEBBINDIR=web/bin
WEBDATADIR=web/data
WEBWWWDIR=web/www

# Compilation targets

C=$(wildcard *.c)
D=$(wildcard .deps/*.d)

ROUTINO_SRC=../../src
ROUTINO_WEBWWWDIR=../../web/www/routino
ROUTINO_DOCDIR=../../doc/html

EXE=fixme-finder$(.EXE) fixme-finder-slim$(.EXE) fixme-dumper$(.EXE) fixme-dumper-slim$(.EXE)
DATA=fixme.xml
WWW_COPY=page-elements.css page-elements.js maplayout.css mapprops.js maploader.js
DOC_COPY=style.css

########

all: all-bin all-data all-www

all-bin: $(EXE)
	@[ -d $(WEBBINDIR) ] || mkdir -p $(WEBBINDIR)
	@for file in $(EXE); do \
	    if [ ! -f $(WEBBINDIR)/$$file ] || [ $$file -nt $(WEBBINDIR)/$$file ]; then \
	       echo cp $$file $(WEBBINDIR) ;\
	       cp -f $$file $(WEBBINDIR) ;\
	    fi ;\
	 done

all-data:
	@[ -d $(WEBDATADIR) ] || mkdir -p $(WEBDATADIR)
	@for file in $(DATA); do \
	    if [ ! -f $(WEBDATADIR)/$$file ] || [ $$file -nt $(WEBDATADIR)/$$file ]; then \
	       echo cp $$file $(WEBDATADIR) ;\
	       cp -f $$file $(WEBDATADIR) ;\
	    fi ;\
	 done

all-www:
	@for file in $(WWW_COPY); do \
	    if [ ! -f $(WEBWWWDIR)/$$file ] || [ $(ROUTINO_WEBWWWDIR)/$$file -nt $(WEBWWWDIR)/$$file ]; then \
	       echo cp $(ROUTINO_WEBWWWDIR)/$$file $(WEBWWWDIR) ;\
	       cp -f $(ROUTINO_WEBWWWDIR)/$$file $(WEBWWWDIR) ;\
	    fi ;\
	 done
	@for file in $(DOC_COPY); do \
	    if [ ! -f $(WEBWWWDIR)/$$file ] || [ $(ROUTINO_DOCDIR)/$$file -nt $(WEBWWWDIR)/$$file ]; then \
	       echo cp $(ROUTINO_DOCDIR)/$$file $(WEBWWWDIR) ;\
	       cp -f $(ROUTINO_DOCDIR)/$$file $(WEBWWWDIR) ;\
	    fi ;\
	 done

########

FIXME_FINDER_OBJ=fixme-finder.o osmparser.o \
	       	 $(ROUTINO_SRC)/nodesx.o $(ROUTINO_SRC)/segmentsx.o $(ROUTINO_SRC)/waysx.o $(ROUTINO_SRC)/relationsx.o \
	       	 $(ROUTINO_SRC)/ways.o $(ROUTINO_SRC)/types.o \
	       	 $(ROUTINO_SRC)/files.o $(ROUTINO_SRC)/logging.o $(ROUTINO_SRC)/logerror.o $(ROUTINO_SRC)/errorlogx.o \
	       	 $(ROUTINO_SRC)/sorting.o \
	       	 $(ROUTINO_SRC)/xmlparse.o $(ROUTINO_SRC)/tagging.o \
	       	 $(ROUTINO_SRC)/uncompress.o $(ROUTINO_SRC)/osmxmlparse.o $(ROUTINO_SRC)/osmpbfparse.o $(ROUTINO_SRC)/osmo5mparse.o

ifeq ($(HOST),MINGW)
FIXME_FINDER_OBJ+=$(ROUTINO_SRC)/mman-win32.o
endif

fixme-finder$(.EXE) : $(FIXME_FINDER_OBJ)
	$(LD) $(FIXME_FINDER_OBJ) -o $@ $(LDFLAGS)

########

FIXME_FINDER_SLIM_OBJ=fixme-finder-slim.o osmparser.o \
	       	      $(ROUTINO_SRC)/nodesx-slim.o $(ROUTINO_SRC)/segmentsx-slim.o $(ROUTINO_SRC)/waysx-slim.o $(ROUTINO_SRC)/relationsx-slim.o \
	              $(ROUTINO_SRC)/ways.o $(ROUTINO_SRC)/types.o \
	       	      $(ROUTINO_SRC)/files.o $(ROUTINO_SRC)/logging.o $(ROUTINO_SRC)/logerror-slim.o $(ROUTINO_SRC)/errorlogx-slim.o \
	              $(ROUTINO_SRC)/sorting.o \
	       	      $(ROUTINO_SRC)/xmlparse.o $(ROUTINO_SRC)/tagging.o \
	       	      $(ROUTINO_SRC)/uncompress.o $(ROUTINO_SRC)/osmxmlparse.o $(ROUTINO_SRC)/osmpbfparse.o $(ROUTINO_SRC)/osmo5mparse.o

ifeq ($(HOST),MINGW)
FIXME_FINDER_SLIM_OBJ+=$(ROUTINO_SRC)/mman-win32.o
endif

fixme-finder-slim$(.EXE) : $(FIXME_FINDER_SLIM_OBJ)
	$(LD) $(FIXME_FINDER_SLIM_OBJ) -o $@ $(LDFLAGS)

########

FIXME_DUMPER_OBJ=fixme-dumper.o \
	         $(ROUTINO_SRC)/errorlog.o \
	         $(ROUTINO_SRC)/files.o $(ROUTINO_SRC)/logging.o $(ROUTINO_SRC)/xmlparse.o

ifeq ($(HOST),MINGW)
FIXME_DUMPER_OBJ+=$(ROUTINO_SRC)/mman-win32.o
endif

fixme-dumper$(.EXE) : $(FIXME_DUMPER_OBJ)
	$(LD) $(FIXME_DUMPER_OBJ) -o $@ $(LDFLAGS)

########

FIXME_DUMPER_SLIM_OBJ=fixme-dumper-slim.o \
	              $(ROUTINO_SRC)/errorlog-slim.o \
	              $(ROUTINO_SRC)/files.o $(ROUTINO_SRC)/logging.o $(ROUTINO_SRC)/xmlparse.o

ifeq ($(HOST),MINGW)
FIXME_DUMPER_SLIM_OBJ+=$(ROUTINO_SRC)/mman-win32.o
endif

fixme-dumper-slim$(.EXE) : $(FIXME_DUMPER_SLIM_OBJ)
	$(LD) $(FIXME_DUMPER_SLIM_OBJ) -o $@ $(LDFLAGS)

########

$(ROUTINO_SRC)/%.o :
	cd $(ROUTINO_SRC) && $(MAKE) $(notdir $@)

$(ROUTINO_SRC)/%-slim.o :
	cd $(ROUTINO_SRC) && $(MAKE) $(notdir $@)

%.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) -DSLIM=0 -I$(ROUTINO_SRC) $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

%-slim.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) -DSLIM=1 -I$(ROUTINO_SRC) $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

########

test:

########

install:

########

clean:
	rm -f *~
	rm -f *.o
	cd $(WEBBINDIR) && rm -f $(EXE)
	cd $(WEBDATADIR) && rm -f $(DATA)
	cd $(WEBWWWDIR) && rm -f $(WWW_COPY)
	cd $(WEBWWWDIR) && rm -f $(DOC_COPY)
	rm -f $(EXE)
	rm -f $(D)
	rm -fr .deps
	rm -f core

########

distclean: clean

########

include $(D)

########

.PHONY:: all test install clean distclean

.PHONY:: all-bin all-data all-www
