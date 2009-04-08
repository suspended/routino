# $Header: /home/amb/CVS/routino/Makefile,v 1.1 2009-04-08 18:58:33 amb Exp $
#
# Makefile
#
# Part of the Routino routing software.
#
# This file Copyright 2009 Andrew M. Bishop
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

MAKEFILES=$(wildcard */Makefile)
DIRS=$(foreach f,$(MAKEFILES),$(dir $f))

########

all:
	for dir in $(DIRS); do \
	   make -C $$dir $@; \
	done

########

clean:
	for dir in $(DIRS); do \
	   make -C $$dir $@; \
	done

########

distclean: clean
	for dir in $(DIRS); do \
	   make -C $$dir $@; \
	done
