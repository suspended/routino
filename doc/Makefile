# $Header: /home/amb/CVS/routino/doc/Makefile,v 1.1 2010-06-27 07:43:42 amb Exp $
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

WEBDIR=../web/www/documentation

FILES=html/*

########

all :
	-@[ -d $(WEBDIR) ] && \
	  for file in $(FILES); do \
	     if [ ! -f $(WEBDIR)/$$file ] || [ $$file -nt $(WEBDIR)/$$file ]; then \
	        echo cp $$file $(WEBDIR) ;\
	        cp $$file $(WEBDIR) ;\
	     fi ;\
	  done

########

clean:
	rm -f *~

########

distclean: clean
