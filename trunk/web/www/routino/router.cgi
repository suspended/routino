#!/usr/bin/perl
#
# Routino interactive router CGI
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

# Use the generic router script
require "router.pl";

# Use the perl CGI module
use CGI ':cgi';

# Create the query and get the parameters

$query=new CGI;

@rawparams=$query->param;

# Legal CGI parameters with regexp validity check

%legalparams=(
              "lon[1-9]"        => "[-0-9.]+",
              "lat[1-9]"        => "[-0-9.]+",
              "transport"       => "[a-z]+",
              "highway-[a-z]+"  => "[0-9.]+",
              "speed-[a-z]+"    => "[0-9.]+",
              "property-[a-z]+" => "[0-9.]+",
              "oneway"          => "(1|0|true|false|on|off)",
              "weight"          => "[0-9.]+",
              "height"          => "[0-9.]+",
              "width"           => "[0-9.]+",
              "length"          => "[0-9.]+",
              "length"          => "[0-9.]+",
              "type"            => "(shortest|quickest)"
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

# Get the important parameters

$type=$cgiparams{type};
delete $cgiparams{type};

# Fill in the default parameters using the ones in router.pl (don't use compiled in defaults)

$cgiparams{transport}='motorcar' if(!defined $cgiparams{transport});

$transport=$cgiparams{transport};

foreach $highway (@router_highways)
  {
   $key="highway-$highway";
   $value=$router_profile_highway{$highway}->{$transport};
   $cgiparams{$key}=$value if(!defined $cgiparams{$key});

   $key="speed-$highway";
   $value=$router_profile_speed{$highway}->{$transport};
   $cgiparams{$key}=$value if(!defined $cgiparams{$key});
  }

$cgiparams{oneway} =~ s/(true|on)/1/;
$cgiparams{oneway} =~ s/(false|off)/0/;

foreach $restriction (@router_restrictions)
  {
   $key="$restriction";
   $value=$router_profile_restrictions{$restriction}->{$transport};
   $cgiparams{$key}=$value if(!defined $cgiparams{$key});
  }

# Run the router

($router_uuid,$router_time,$router_result,$router_message)=&RunRouter($type,%cgiparams);

# Return the output

print header('text/plain');

print "$router_uuid\n";
print "$router_time\n";
print "$router_result\n";
print "$router_message\n";
