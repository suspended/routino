# $Header: /home/amb/CVS/routino/xml/Makefile,v 1.1 2010-04-07 17:22:45 amb Exp $
#
# XML directory Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2010 Andrew M. Bishop
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

XML=routino-profiles.xml

########

all : $(XML)
	-@[ -d ../web/data ] && \
	  for file in $(XML); do \
	     if [ $$file -nt ../web/data/$file ]; then \
	        echo $$file ../web/data ;\
	        cp $$file ../web/data ;\
	     fi ;\
	  done

########

clean:
	rm -f *~

########

distclean: clean
