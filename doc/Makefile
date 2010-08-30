# $Header: /home/amb/CVS/routino/doc/Makefile,v 1.3 2010-08-30 13:37:39 amb Exp $
#
# Documentation directory Makefile
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

# Installation paths

prefix=/usr/local
docdir=$(DESTDIR)$(prefix)/doc/routino

# Web file paths

WEBDIR=../web/www/routino/documentation

# Files to install

HTML_FILES=html/*
TXT_FILES=*.txt

########

all :
	-@[ -d $(WEBDIR) ] && \
	  for file in $(HTML_FILES); do \
	     if [ ! -f $(WEBDIR)/`basename $$file` ] || [ $$file -nt $(WEBDIR)/`basename $$file` ]; then \
	        echo cp $$file $(WEBDIR) ;\
	        cp -f $$file $(WEBDIR) ;\
	     fi ;\
	  done

########

install: all
	-[ -d $(docdir) ] || mkdir -p $(docdir)
	@[ -d $(docdir) ] && \
	  for file in $(TXT_FILES); do \
	     echo cp $$file $(docdir) ;\
	     cp -f $$file $(docdir) ;\
	  done
	-[ -d $(docdir)/html ] || mkdir -p $(docdir)/html
	@[ -d $(docdir)/html ] && \
	  for file in $(HTML_FILES); do \
	     echo cp $$file $(docdir)/html ;\
	     cp -f $$file $(docdir)/html ;\
	  done

########

clean:
	rm -f *~
	rm -f html/*~
	rm -f $(WEBDIR)/*~

########

distclean: clean
	rm -f $(WEBDIR)/*
