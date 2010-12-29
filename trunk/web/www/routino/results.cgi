#!/usr/bin/perl
#
# Routino router results retrieval CGI
#
# Part of the Routino routing software.
#
# This file Copyright 2008,2009 Andrew M. Bishop
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

# Use the directory paths script
require "paths.pl";

# Use the perl CGI module
use CGI ':cgi';

# Create the query and get the parameters

$query=new CGI;

@rawparams=$query->param;

# Legal CGI parameters with regexp validity check

%legalparams=(
              "type"   => "(shortest|quickest)",
              "uuid"   => "[0-9a-f]{32}",
              "format" => "(gpx-route|gpx-track|txt|txt-all)"
             );

# Validate the CGI parameters, ignore invalid ones

foreach $key (@rawparams)
  {
   foreach $test (keys (%legalparams))
     {
      if($key =~ m%^$test$%)
        {
         $value=$query->param($key);

         if($value =~ m%^$legalparams{$test}$%)
           {
            $cgiparams{$key}=$value;
            last;
           }
        }
     }
  }

# Possible file formats

%formats=(
          "gpx-route" => "-route.gpx",
          "gpx-track" => "-track.gpx",
          "txt"       => ".txt",
          "txt-all"   => "-all.txt"
         );

# Possible MIME types

%mimetypes=(
            "gpx-route" => "text/xml",
            "gpx-track" => "text/xml",
            "txt"       => "text/plain",
            "txt-all"   => "text/plain"
           );

# Parse the parameters

$type  =$cgiparams{"type"};
$uuid  =$cgiparams{"uuid"};
$format=$formats{$cgiparams{"format"}};
$mime  =$mimetypes{$cgiparams{"format"}};

$file="$results_dir/$uuid/$type$format";

if(!$type || !$uuid || !$format || ! -f $file)
  {
   print header('text/plain','404 Not found');
   print "Not Found!\n";

   exit;
  }

# Return the output

print header($mime);

system "cat $file";
