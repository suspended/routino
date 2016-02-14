#!/bin/sh
#
# OSM data statistics shell script
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2016 Andrew M. Bishop
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

set -e

trap "rm binned.dat" 0

# Database location and zoom - PARAMETERS THAT CAN BE EDITED

database_dir="../../web/data"
database_dir="/home/amb/www/local/cgi-temp/routino-data"
zoom=13

# Run the dumper

./dumper --speed=32,48,64,80,96,112 --dir=$database_dir --zoom=$zoom > binned.dat

# Generate the base map

./create-basemap.pl

# Generate the highway outputs

./create-image.pl highway motorway      0 < binned.dat
./create-image.pl highway trunk         1 < binned.dat
./create-image.pl highway primary       2 < binned.dat
./create-image.pl highway secondary     3 < binned.dat
./create-image.pl highway tertiary      4 < binned.dat
./create-image.pl highway unclassified  5 < binned.dat
./create-image.pl highway residential   6 < binned.dat
./create-image.pl highway service       7 < binned.dat
./create-image.pl highway track         8 < binned.dat
./create-image.pl highway cycleway      9 < binned.dat
./create-image.pl highway path         10 < binned.dat
./create-image.pl highway steps        11 < binned.dat
./create-image.pl highway ferry        12 < binned.dat

# Generate the transport outputs

./create-image.pl transport foot        13 < binned.dat
./create-image.pl transport horse       14 < binned.dat
./create-image.pl transport bicycle     15 < binned.dat
./create-image.pl transport wheelchair  16 < binned.dat
./create-image.pl transport moped       17 < binned.dat
./create-image.pl transport motorcycle  18 < binned.dat
./create-image.pl transport motorcar    19 < binned.dat
./create-image.pl transport goods       20 < binned.dat
./create-image.pl transport HGV         21 < binned.dat
./create-image.pl transport PSV         22 < binned.dat

# Create the property outputs

./create-image.pl property paved           23 < binned.dat
./create-image.pl property multilane       24 < binned.dat
./create-image.pl property bridge          25 < binned.dat
./create-image.pl property tunnel          26 < binned.dat
./create-image.pl property foot-route      27 < binned.dat
./create-image.pl property bicycle-route   28 < binned.dat
./create-image.pl property cycle-both-ways 29 < binned.dat
./create-image.pl property oneway          30 < binned.dat

# Generate the speed limit outputs

./create-image.pl speed 20mph 31 < binned.dat
./create-image.pl speed 30mph 32 < binned.dat
./create-image.pl speed 40mph 33 < binned.dat
./create-image.pl speed 50mph 34 < binned.dat
./create-image.pl speed 60mph 35 < binned.dat
./create-image.pl speed 70mph 36 < binned.dat

# Tidy up and exit

# This is handled by the trap at the top
#
#rm binned.dat
