# Documentation directory Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2010-2014 Andrew M. Bishop
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

# Files to install

HTML_FILES=$(notdir $(wildcard html/*.html)) $(notdir $(wildcard html/*.css))
TXT_FILES=*.txt
TOP_FILES=../agpl-3.0.txt

########

all:

########

test:

########

install: install-txt install-html

install-txt:
	@[ -d $(DESTDIR)$(docdir) ] || mkdir -p $(DESTDIR)$(docdir)
	@for file in $(TOP_FILES); do \
	    echo cp $$file $(DESTDIR)$(docdir) ;\
	    cp -f $$file $(DESTDIR)$(docdir) ;\
	 done
	@for file in $(TXT_FILES); do \
	    echo cp $$file $(DESTDIR)$(docdir) ;\
	    cp -f $$file $(DESTDIR)$(docdir) ;\
	 done

install-html:
	@[ -d $(DESTDIR)$(docdir)/html ] || mkdir -p $(DESTDIR)$(docdir)/html
	@for file in $(HTML_FILES); do \
	    echo cp html/$$file $(DESTDIR)$(docdir)/html ;\
	    cp -f html/$$file $(DESTDIR)$(docdir)/html ;\
	 done

########

clean:
	rm -f *~
	rm -f html/*~

########

distclean: clean

########

.PHONY:: all test install clean distclean

.PHONY:: install-txt install-html
