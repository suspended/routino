#
# Routino generic Search Perl script
#
# Part of the Routino routing software.
#
# This file Copyright 2012-2014 Andrew M. Bishop
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

# Use the perl URI module
use URI::Escape;

# Use the perl LWP module
use LWP::UserAgent;

# Use the perl JSON module
use JSON::PP;

# Use the perl Time::HiRes module
use Time::HiRes qw(gettimeofday tv_interval);

my $t0 = [gettimeofday];


#
# Run the search
#

sub RunSearch
  {
   my($search,$lonmin,$lonmax,$latmax,$latmin)=@_;

   # Perform the search based on the type

   my $message="";
   my @places=[];

   if($main::search_type eq "nominatim")
     {
      ($message,@places)=DoNominatimSearch($search,$lonmin,$lonmax,$latmax,$latmin);
     }
   else
     {
      $message="Unknown search type '$main::search_type'";
     }

   my(undef,undef,$cuser,$csystem) = times;
   my $time=sprintf "time: %.3f CPU / %.3f elapsed",$cuser+$csystem,tv_interval($t0);

   # Return the results

   return($time,$message,@places);
  }


#
# Fetch the search URL from Nominatim
#

sub DoNominatimSearch
  {
   my($search,$lonmin,$lonmax,$latmax,$latmin)=@_;

   $search = uri_escape($search);

   my $url;

   if($lonmin && $lonmax && $latmax && $latmin)
     {
      $url="$main::search_baseurl?format=json&viewbox=$lonmin,$latmax,$lonmax,$latmin&q=$search";
     }
   else
     {
      $url="$main::search_baseurl?format=json&q=$search";
     }

   my $ua=LWP::UserAgent->new;

   my $res=$ua->get($url);

   if(!$res->is_success)
     {
      return($res->status_line);
     }

   my $result=decode_json($res->content);

   my @places=();

   foreach my $place (@$result)
     {
      my $lat=$place->{"lat"};
      my $lon=$place->{"lon"};
      my $name=$place->{"display_name"};

      push(@places,"$lat $lon $name");
     }

   return("",@places);
  }


1;
