#!/usr/bin/perl
#
# Routing test case generator tool.
#
# Part of the Routino routing software.
#
# This file Copyright 2011-2014 Andrew M. Bishop
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

use strict;

# Command line

if($#ARGV<1 || $ARGV>2 || ! -f $ARGV[0])
  {
   die  "Usage: waypoints.pl <filename.osm> list\n".
        "       waypoints.pl <filename.osm> <name> <number>\n";
  }

# Parse the file

open(FILE,"<$ARGV[0]") || die "Cannot open '$ARGV[0]'\n";

my %waypoints=();
my @waypoints=();
my @waypoint_lat=();
my @waypoint_lon=();
my $innode=0;

while(<FILE>)
  {
   if($innode)
     {
      if(m%<tag k='name' v='([^']+)'%)
        {
         push(@waypoints,$1);
         $waypoints{$1}=$#waypoints;
        }
      $innode=0 if(m%</node>%);
     }
   elsif(m%<node .* lat='([-.0-9]+)' *lon='([-.0-9]+)' *>%)
     {
      $innode=1;
      push(@waypoint_lat,$1);
      push(@waypoint_lon,$2);
     }
  }

close(FILE);

# Perform the action

if($ARGV[1] eq "list")
  {
   print join(" ",sort @waypoints)."\n";
   exit 0;
  }

if($waypoints{$ARGV[1]} ne "")
  {
   print "--lat$ARGV[2]=$waypoint_lat[$waypoints{$ARGV[1]}] --lon$ARGV[2]=$waypoint_lon[$waypoints{$ARGV[1]}]\n";
   exit 0;
  }

exit 1;
