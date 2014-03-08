#!/usr/bin/perl
#
# Routino interactive router CGI
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

# Use the generic router script
require "router.pl";

# Use the perl CGI module
use CGI ':cgi';


# Create the query and get the parameters

my $query=new CGI;

my @rawparams=$query->param;

# Legal CGI parameters with regexp validity check

my %legalparams=(
              "lon[1-9][0-9]*"  => "[-0-9.]+",
              "lat[1-9][0-9]*"  => "[-0-9.]+",
              "heading"         => "[-0-9.]+",
              "transport"       => "[a-z]+",
              "highway-[a-z]+"  => "[0-9.]+",
              "speed-[a-z]+"    => "[0-9.]+",
              "property-[a-z]+" => "[0-9.]+",
              "oneway"          => "(1|0|true|false|on|off)",
              "turns"           => "(1|0|true|false|on|off)",
              "weight"          => "[0-9.]+",
              "height"          => "[0-9.]+",
              "width"           => "[0-9.]+",
              "length"          => "[0-9.]+",
              "length"          => "[0-9.]+",

              "language"        => "[-a-zA-Z]+",
              "type"            => "(shortest|quickest)",
              "format"          => "(html|gpx-route|gpx-track|text|text-all)",

              "reverse"         => "(1|0|true|false|on|off)",
              "loop"            => "(1|0|true|false|on|off)"
             );

# Validate the CGI parameters, ignore invalid ones

my %cgiparams=();

foreach my $key (@rawparams)
  {
   foreach my $test (keys (%legalparams))
     {
      if($key =~ m%^$test$%)
        {
         my $value=$query->param($key);

         if($value =~ m%^$legalparams{$test}$%)
           {
            $cgiparams{$key}=$value;
            last;
           }
        }
     }
  }

# Get the important parameters

my $type;
my $format;

$type=$cgiparams{type};
delete $cgiparams{type};

$type="shortest" if(!$type);

$format=$cgiparams{format};
delete $cgiparams{format};

# Fill in the default parameters

my %fullparams=FillInDefaults(%cgiparams);

# Run the router

my($router_uuid,$router_success)=RunRouter($type,%fullparams);

# Return the output

if($format)
  {
   ReturnOutput($router_uuid,$type,$format);
  }
else
  {
   print header('text/plain');

   print "$router_uuid\n";
   print "$router_success\n";
  }
