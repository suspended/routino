# Source code Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2015 Andrew M. Bishop
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

include ../Makefile.conf

# Sub-directories and sub-makefiles

SUBDIRS=xml test

# Compilation targets

C=$(wildcard *.c)
D=$(wildcard .deps/*.d)

EXE=planetsplitter$(.EXE) planetsplitter-slim$(.EXE) router$(.EXE) router-slim$(.EXE) \
    filedumperx$(.EXE) filedumper$(.EXE) filedumper-slim$(.EXE) \
    router+lib$(.EXE) router+lib-slim$(.EXE)

ifneq ($(HOST),MINGW)
LIB=libroutino.so libroutino-slim.so
else
LIB =routino.dll routino-slim.dll
LIB+=routino.def routino-slim.def
LIB+=routino.lib routino-slim.lib
endif

INC=routino.h

########

all: all-local
	for dir in $(SUBDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

all-local: $(EXE) $(LIB)

########

PLANETSPLITTER_OBJ=planetsplitter.o \
	           nodesx.o segmentsx.o waysx.o relationsx.o superx.o prunex.o \
	           ways.o types.o \
	           files.o logging.o logerror.o errorlogx.o \
	           results.o queue.o sorting.o \
	           xmlparse.o tagging.o \
	           uncompress.o osmxmlparse.o osmpbfparse.o osmo5mparse.o osmparser.o

ifeq ($(HOST),MINGW)
PLANETSPLITTER_OBJ+=mman-win32.o
endif

planetsplitter$(.EXE) : $(PLANETSPLITTER_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

PLANETSPLITTER_SLIM_OBJ=planetsplitter-slim.o \
	                nodesx-slim.o segmentsx-slim.o waysx-slim.o relationsx-slim.o superx-slim.o prunex-slim.o \
	                ways.o types.o \
	                files.o logging.o logerror-slim.o errorlogx-slim.o \
	                results.o queue.o sorting.o \
	                xmlparse.o tagging.o \
	                uncompress.o osmxmlparse.o osmpbfparse.o osmo5mparse.o osmparser.o

ifeq ($(HOST),MINGW)
PLANETSPLITTER_SLIM_OBJ+=mman-win32.o
endif

planetsplitter-slim$(.EXE) : $(PLANETSPLITTER_SLIM_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

ROUTER_OBJ=router.o \
	   nodes.o segments.o ways.o relations.o types.o fakes.o \
	   optimiser.o output.o \
	   files.o logging.o profiles.o xmlparse.o \
	   results.o queue.o translations.o

ifeq ($(HOST),MINGW)
ROUTER_OBJ+=mman-win32.o
endif

router$(.EXE) : $(ROUTER_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

ROUTER_SLIM_OBJ=router-slim.o \
	        nodes-slim.o segments-slim.o ways-slim.o relations-slim.o types.o fakes-slim.o \
	        optimiser-slim.o output-slim.o \
	        files.o logging.o profiles.o xmlparse.o \
	        results.o queue.o translations.o

ifeq ($(HOST),MINGW)
ROUTER_SLIM_OBJ+=mman-win32.o
endif

router-slim$(.EXE) : $(ROUTER_SLIM_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

FILEDUMPERX_OBJ=filedumperx.o \
	        files.o logging.o

ifeq ($(HOST),MINGW)
FILEDUMPERX_OBJ+=mman-win32.o
endif

filedumperx$(.EXE) : $(FILEDUMPERX_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

FILEDUMPER_OBJ=filedumper.o \
	       nodes.o segments.o ways.o relations.o types.o fakes.o errorlog.o \
               visualiser.o \
	       files.o logging.o xmlparse.o

ifeq ($(HOST),MINGW)
FILEDUMPER_OBJ+=mman-win32.o
endif

filedumper$(.EXE) : $(FILEDUMPER_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

FILEDUMPER_SLIM_OBJ=filedumper-slim.o \
	       nodes-slim.o segments-slim.o ways-slim.o relations-slim.o types.o fakes-slim.o errorlog-slim.o \
	       visualiser-slim.o \
	       files.o logging.o xmlparse.o

ifeq ($(HOST),MINGW)
FILEDUMPER_SLIM_OBJ+=mman-win32.o
endif

filedumper-slim$(.EXE) : $(FILEDUMPER_SLIM_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS)

########

%.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) -DSLIM=0 -DROUTINO_DATADIR=\"$(datadir)\" $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

%-slim.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) -DSLIM=1 -DROUTINO_DATADIR=\"$(datadir)\" $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

########

ROUTER_LIB_OBJ=router+lib.o

router+lib$(.EXE) : $(ROUTER_LIB_OBJ) libroutino.so
	$(LD) $^ -o $@ $(LDFLAGS) $(LDFLAGS_LDSO)

router+lib-slim$(.EXE) : $(ROUTER_LIB_OBJ) libroutino-slim.so
	$(LD) $^ -o $@ $(LDFLAGS) $(LDFLAGS_LDSO)

########

LIBROUTINO_OBJ=routino-lib.o \
	        nodes-lib.o segments-lib.o ways-lib.o relations-lib.o types-lib.o fakes-lib.o \
	        optimiser-lib.o output-lib.o \
	        files-lib.o profiles-lib.o xmlparse-lib.o \
	        results-lib.o queue-lib.o translations-lib.o

ifeq ($(HOST),MINGW)
LIBROUTINO_OBJ+=mman-win32.o
endif

libroutino.so : $(LIBROUTINO_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS) $(LDFLAGS_LIB)

libroutino.dll : $(LIBROUTINO_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS) $(LDFLAGS_LIB)

routino.def : routino-lib.o
	dlltool -v --output-def $@ $<

routino.lib : routino.dll routino.def
	dlltool -v --dllname routino.dll --def routino.def --output-lib $@

########

LIBROUTINO_SLIM_OBJ=routino-slim-lib.o \
	        nodes-slim-lib.o segments-slim-lib.o ways-slim-lib.o relations-slim-lib.o types-lib.o fakes-slim-lib.o \
	        optimiser-slim-lib.o output-slim-lib.o \
	        files-lib.o profiles-lib.o xmlparse-lib.o \
	        results-lib.o queue-lib.o translations-lib.o

ifeq ($(HOST),MINGW)
LIBROUTINO_SLIM_OBJ+=mman-win32.o
endif

libroutino-slim.so : $(LIBROUTINO_SLIM_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS) $(LDFLAGS_LIB)

libroutino-slim.dll : $(LIBROUTINO_SLIM_OBJ)
	$(LD) $^ -o $@ $(LDFLAGS) $(LDFLAGS_LIB)

routino-slim.def : routino-slim-lib.o
	dlltool -v --output-def $@ $<

routino-slim.lib : routino-slim.dll routino-slim.def
	dlltool -v --dllname routino-slim.dll --def routino-slim.def --output-lib $@

########

%-lib.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) $(CFLAGS_LIB) -DSLIM=0 -DLIBROUTINO $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

%-slim-lib.o : %.c
	@[ -d .deps ] || mkdir .deps
	$(CC) -c $(CFLAGS) $(CFLAGS_LIB) -DSLIM=1 -DLIBROUTINO $< -o $@ -MMD -MP -MF $(addprefix .deps/,$(addsuffix .d,$(basename $@)))

########

test: test-local
	for dir in $(SUBDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

test-local:

########

install: install-local
	for dir in $(SUBDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

install-local: all-local
	@[ -d $(DESTDIR)$(bindir) ] || mkdir -p $(DESTDIR)$(bindir)
	@for file in $(EXE); do \
	    if [ -f $$file ]; then \
	       echo cp $$file $(DESTDIR)$(bindir) ;\
	       cp -f $$file $(DESTDIR)$(bindir) ;\
	    fi ;\
	 done
	@[ -d $(DESTDIR)$(incdir) ] || mkdir -p $(DESTDIR)$(incdir)
	@for file in $(INC); do \
	    if [ -f $$file ]; then \
	       echo cp $$file $(DESTDIR)$(incdir) ;\
	       cp -f $$file $(DESTDIR)$(incdir) ;\
	    fi ;\
	 done
	@[ -d $(DESTDIR)$(libdir) ] || mkdir -p $(DESTDIR)$(libdir)
	@for file in $(LIB); do \
	    if [ -f $$file ]; then \
	       echo cp $$file $(DESTDIR)$(libdir) ;\
	       cp -f $$file $(DESTDIR)$(libdir) ;\
	    fi ;\
	 done

########

clean: clean-local
	for dir in $(SUBDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

clean-local:
	rm -f *~
	rm -f *.o
	rm -f $(EXE)
	rm -f $(LIB)
	rm -f $(D)
	rm -fr .deps
	rm -f core
	rm -f *.gcda *.gcno *.gcov gmon.out

########

distclean: distclean-local
	for dir in $(SUBDIRS); do \
	   ( cd $$dir && $(MAKE) $@ ); \
	done

distclean-local: clean-local

########

include $(D)

########

.PHONY:: all test install clean distclean

.PHONY:: all-local test-local install-local clean-local distclean-local
