#
# Routino generic Search Perl script
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

# Use the directory paths script
require "paths.pl";

# Use the perl LWP module
use LWP::UserAgent;

# Use the perl Time::HiRes module
use Time::HiRes qw(gettimeofday tv_interval);

$t0 = [gettimeofday];


#
# Run the search
#

sub RunSearch
  {
   my($search)=@_;

   # Perform the search based on the type

   my($lat,$lon,$message);

   if($search_type eq "nominatim")
     {
      ($lat,$lon,$message)=DoNominatimSearch($search);
     }
   else
     {
      $lat="";
      $lon="";
      $message="Unknown search type '$search_type'";
     }

   my(undef,undef,$cuser,$csystem) = times;
   my($time)=sprintf "time: %.3f CPU / %.3f elapsed",$cuser+$csystem,tv_interval($t0);

   # Return the results

   return($time,$lat,$lon,$message);
  }


#
# Fetch the search URL from Nominatim
#

sub DoNominatimSearch
  {
   my($search)=@_;

   $search =~ s% %+%g;

   my($url)="$search_baseurl?format=json&limit=1&q=$search";

   my($ua)=LWP::UserAgent->new;
   my($res)=$ua->get($url);

   my($lat,$lon,$message)=("","","");

   $lat=$1 if($res->content =~ m%"lat" *: *"([-.0-9]+)"%);

   $lon=$1 if($res->content =~ m%"lon" *: *"([-.0-9]+)"%);

   $message="No result" if($lon eq "" || $lat eq "");

   return($lat,$lon,$message);
  }


1;
