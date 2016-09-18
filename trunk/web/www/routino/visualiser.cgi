#!/usr/bin/perl
#
# Routino data visualiser CGI
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2014, 2016 Andrew M. Bishop
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
require "./paths.pl";

# Use the perl CGI module
use CGI ':cgi';


# Create the query and get the parameters

my $query=new CGI;

my @rawparams=$query->param;

# Legal CGI parameters with regexp validity check

my %legalparams=(
                 "latmin" => "[-0-9.]+",
                 "latmax" => "[-0-9.]+",
                 "lonmin" => "[-0-9.]+",
                 "lonmax" => "[-0-9.]+",
                 "data"   => "(junctions|super|waytype-.*|highway-.*|transport-.*|barrier-.*|turns|speed|weight|height|width|length|property-.*|errorlogs)",
                 "dump"   => "(node|segment|turn-relation|errorlog)[0-9]+"
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

# Data or dump?

my $params="";

my $data=$cgiparams{"data"};
my $dump=$cgiparams{"dump"};

if(!defined $data && !defined $dump)
  {
   print header(-status => '500 Invalid CGI parameters');
   exit;
  }

if(defined $data)
  {
   # Parameters to limit range selected

   my %limits=(
               "junctions" => 0.2,
               "super"     => 0.2,
               "waytype"   => 0.2,
               "highway"   => 0.2,
               "transport" => 0.2,
               "barrier"   => 0.3,
               "turns"     => 0.3,
               "speed"     => 0.3,
               "weight"    => 0.3,
               "height"    => 0.3,
               "width"     => 0.3,
               "length"    => 0.3,
               "property"  => 0.3,
               "errorlogs" => 0.5
              );

   # Check the parameters

   my $latmin=$cgiparams{"latmin"};
   my $latmax=$cgiparams{"latmax"};
   my $lonmin=$cgiparams{"lonmin"};
   my $lonmax=$cgiparams{"lonmax"};

   if($latmin eq "" || $latmax eq "" || $lonmin eq "" || $lonmax eq "" || $data eq "")
     {
      print header(-status => '500 Invalid CGI parameters');
      exit;
     }

   my $subdata=$data;
   $subdata="waytype"   if($data =~ m%waytype-%);
   $subdata="highway"   if($data =~ m%highway-%);
   $subdata="transport" if($data =~ m%transport-%);
   $subdata="barrier"   if($data =~ m%barrier-%);
   $subdata="property"  if($data =~ m%property-%);

   if(($latmax-$latmin)>$limits{$subdata} || ($lonmax-$lonmin)>$limits{$subdata})
     {
      print header(-status => '500 Selected area too large');
      exit;
     }

   # Print the output

   print header('text/plain');

   print "$latmin $lonmin $latmax $lonmax\n";

   # Set the parameters

   $params.=" --visualiser --data=$data";
   $params.=" --latmin=$latmin --latmax=$latmax --lonmin=$lonmin --lonmax=$lonmax";
  }
else
  {
   # Print the output

   print header('text/plain');

   # Set the parameters

   $params.=" --dump-visualiser --data=$dump";
  }

# Run the filedumper

$params.=" --dir=$main::data_dir" if($main::data_dir);
$params.=" --prefix=$main::data_prefix" if($main::data_prefix);

system "$main::bin_dir/$main::filedumper_exe $params 2>&1";
