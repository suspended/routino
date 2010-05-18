# $Header: /home/amb/CVS/routino/xml/Makefile,v 1.3 2010-05-18 18:35:48 amb Exp $
#
# XML directory Makefile
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

XML=profiles.xml \
    translations.xml \
    tagging.xml

########

all :
	-@[ -d ../web/data ] && \
	  for file in $(XML); do \
	     if [ ! -f ../web/data/$$file -o routino-$$file -nt ../web/data/$$file ]; then \
	        echo cp routino-$$file ../web/data/$$file ;\
	        cp routino-$$file ../web/data/$$file ;\
	     fi ;\
	  done

########

clean:
	rm -f *~

########

distclean: clean
