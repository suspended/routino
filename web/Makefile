# web directory Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2010-2015 Andrew M. Bishop
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

# Web file paths and other paths

WEBBINDIR=bin
WEBDATADIR=data
WEBTRANSDIR=translations
WEBWWWDIR=www/routino
WEBICONDIR=www/routino/icons
WEBDOCDIR=www/routino/documentation

XMLDIR=../xml
DOCDIR=../doc
SRCDIR=../src

# Files to install

STANDARD_XML_FILES=profiles.xml \
	           translations.xml \
	           tagging.xml

SPECIAL_XML_FILES=tagging-drive.xml \
	          tagging-ride.xml \
	          tagging-walk.xml

PROFILE_FILES=profiles.pl \
	      profiles.js

TRANS_FILES=$(wildcard $(WEBTRANSDIR)/translation.*.txt)

DOC_FILES=$(notdir $(wildcard $(DOCDIR)/html/*.html)) $(notdir $(wildcard $(DOCDIR)/html/*.css))

EXE_FILES=planetsplitter planetsplitter-slim \
	  router         router-slim \
	  filedumper     filedumper-slim \
	  filedumperx

########

all: all-bin all-data all-doc all-profiles all-icons all-translations

all-bin: all-exe
	@[ -d $(WEBBINDIR) ] || mkdir -p $(WEBBINDIR)
	@for file in $(EXE_FILES); do \
	    if [ ! -f $(WEBBINDIR)/$$file ] || [ $(SRCDIR)/$$file -nt $(WEBBINDIR)/$$file ]; then \
	       echo cp $(SRCDIR)/$$file $(WEBBINDIR) ;\
	       cp -f $(SRCDIR)/$$file $(WEBBINDIR) ;\
	    fi ;\
	 done

all-data: all-xml
	@[ -d $(WEBDATADIR) ] || mkdir -p $(WEBDATADIR)
	@for file in $(STANDARD_XML_FILES); do \
	    if [ ! -f $(WEBDATADIR)/$$file ] || [ $(XMLDIR)/routino-$$file -nt $(WEBDATADIR)/$$file ]; then \
	       echo cp $(XMLDIR)/routino-$$file $(WEBDATADIR)/$$file ;\
	       cp -f $(XMLDIR)/routino-$$file $(WEBDATADIR)/$$file ;\
	    fi ;\
	 done
	@for file in $(SPECIAL_XML_FILES); do \
	    if [ ! -f $(WEBDATADIR)/$$file ] || [ $(XMLDIR)/$$file -nt $(WEBDATADIR)/$$file ]; then \
	       echo cp $(XMLDIR)/$$file $(WEBDATADIR)/$$file ;\
	       cp -f $(XMLDIR)/$$file $(WEBDATADIR)/$$file ;\
	    fi ;\
	 done

all-doc:
	@[ -d $(WEBDOCDIR) ] || mkdir -p $(WEBDOCDIR)
	@for file in $(DOC_FILES); do \
	    if [ ! -f $(WEBDOCDIR)/$$file ] || [ $(DOCDIR)/html/$$file -nt $(WEBDOCDIR)/$$file ]; then \
	       echo cp $(DOCDIR)/html/$$file $(WEBDOCDIR) ;\
	       cp -f $(DOCDIR)/html/$$file $(WEBDOCDIR) ;\
	    fi ;\
	 done

all-profiles: all-exe all-data
	@if [ ! -f $(WEBWWWDIR)/profiles.js ] || [ ! -f $(WEBWWWDIR)/profiles.pl ] || \
	     [ $(WEBDATADIR)/profiles.xml -nt $(WEBWWWDIR)/profiles.pl ] || \
	     [ $(WEBDATADIR)/profiles.xml -nt $(WEBWWWDIR)/profiles.js ]; then \
	    echo update-profiles.pl ;\
	    ( cd $(WEBWWWDIR) ; perl update-profiles.pl ) ;\
	 fi

all-icons: $(WEBICONDIR)/ball-0.png

$(WEBICONDIR)/ball-0.png: $(WEBICONDIR)/create-icons.pl
	@echo create-icons.pl
	@cd $(WEBICONDIR) && perl create-icons.pl

all-translations: $(WEBWWWDIR)/router.html.en $(WEBWWWDIR)/visualiser.html.en $(XMLDIR)/routino-translations.xml

$(WEBWWWDIR)/router.html.en: $(WEBTRANSDIR)/router.html $(TRANS_FILES) $(WEBTRANSDIR)/translate.pl
	@echo translate.pl
	@cd $(WEBTRANSDIR) && perl translate.pl

$(WEBWWWDIR)/visualiser.html.en: $(WEBTRANSDIR)/visualiser.html $(TRANS_FILES) $(WEBTRANSDIR)/translate.pl
	@echo translate.pl
	@cd $(WEBTRANSDIR) && perl translate.pl

$(XMLDIR)/routino-translations.xml: $(WEBTRANSDIR)/translations-head.xml $(WEBTRANSDIR)/translations-body.xml $(WEBTRANSDIR)/translations-tail.xml $(TRANS_FILES) $(WEBTRANSDIR)/translate.pl
	@echo translate.pl
	@cd $(WEBTRANSDIR) && perl translate.pl

all-exe:
	cd $(SRCDIR) && $(MAKE) $(EXE_FILES)

all-xml: $(XMLDIR)/routino-translations.xml
	cd $(XMLDIR) && $(MAKE) all

########

test:

########

install: all
	@echo "******************************************************"
	@echo "* Note: web directory is not installed automatically *"
	@echo "******************************************************"

########

clean:
	rm -f *~

########

distclean: clean
	-cd $(WEBBINDIR)  && rm -f $(EXE_FILES)
	-cd $(WEBDATADIR) && rm -f $(STANDARD_XML_FILES) $(SPECIAL_XML_FILES)
	-cd $(WEBDOCDIR)  && rm -f $(DOC_FILES)
	-cd $(WEBWWWDIR)  && rm -f $(PROFILE_FILES)

########

.PHONY:: all test install clean distclean

.PHONY:: all-bin all-data all-doc all-profiles all-icons all-translations all-exe all-xml
