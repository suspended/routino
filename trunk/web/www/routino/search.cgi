#!/usr/bin/perl
#
# Routino search results retrieval CGI
#
# Part of the Routino routing software.
#
# This file Copyright 2012 Andrew M. Bishop
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

# Use the generic search script
require "search.pl";

# Use the perl CGI module
use CGI ':cgi';


# Create the query and get the parameters

$query=new CGI;

@rawparams=$query->param;

# Convert the CGI parameters

foreach my $key (@rawparams)
  {
   my $value=$query->param($key);

   $cgiparams{$key}=$value;
  }

# Parse the parameters

$marker=$cgiparams{"marker"};
$search=$cgiparams{"search"};

# Run the search

($search_time,$search_lat,$search_lon,$search_message)=RunSearch($search);

# Return the output

print header('text/plain');

print "$marker\n";
print "$search_time\n";
print "$search_lat $search_lon\n";
print "$search_message\n";
