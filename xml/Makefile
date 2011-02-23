# XML directory Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2010-2011 Andrew M. Bishop
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

# Web file paths

WEBDATADIR=../web/data
WEBWWWDIR=../web/www/routino

# Files to install

FILES=profiles.xml \
      translations.xml \
      tagging.xml

########

all :
	-@[ -d $(WEBDATADIR) ] && \
	  for file in $(FILES); do \
	     if [ ! -f $(WEBDATADIR)/$$file ] || [ routino-$$file -nt $(WEBDATADIR)/$$file ]; then \
	        echo cp routino-$$file $(WEBDATADIR)/$$file ;\
	        cp -f routino-$$file $(WEBDATADIR)/$$file ;\
	     fi ;\
	  done ;\
	  if [ ! -f $(WEBWWWDIR)/profiles.js ] || [ ! -f $(WEBWWWDIR)/profiles.pl ] || \
	     [ $(WEBDATADIR)/profiles.xml -nt $(WEBWWWDIR)/profiles.pl ] || \
	     [ $(WEBDATADIR)/profiles.xml -nt $(WEBWWWDIR)/profiles.js ]; then \
	     echo perl update-profiles.pl ;\
	     ( cd $(WEBWWWDIR) ; perl update-profiles.pl ) ;\
	  fi

########

install: all
	-[ -d $(DESTDIR)$(datadir) ] || mkdir -p $(DESTDIR)$(datadir)
	@[ -d $(DESTDIR)$(datadir) ] && \
	  for file in $(FILES); do \
	     echo cp routino-$$file $(DESTDIR)$(datadir)/$$file ;\
	     cp -f routino-$$file $(DESTDIR)$(datadir)/$$file ;\
	  done

########

clean:
	rm -f *~

########

distclean: clean
	rm -f $(WEBDATADIR)/*.xml

########

top=-top
include ../Makefile
