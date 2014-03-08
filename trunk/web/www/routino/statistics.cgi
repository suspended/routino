#!/usr/bin/perl
#
# Routino data statistics
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2014 Andrew M. Bishop
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

# Use the directory paths script
require "paths.pl";

# Use the perl CGI module
use CGI ':cgi';


# Print the output

print header('text/plain');

# Run the filedumper

my $params="";

$params.=" --dir=$main::data_dir" if($main::data_dir);
$params.=" --prefix=$main::data_prefix" if($main::data_prefix);
$params.=" --statistics";

system "$main::bin_dir/$main::filedumper_exe $params 2>&1";
