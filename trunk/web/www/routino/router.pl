#
# Routino generic router Perl script
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

# Use the perl Time::HiRes module
use Time::HiRes qw(gettimeofday tv_interval);

$t0 = [gettimeofday];

# Filename prefix

# EDIT THIS if the database files were created with the --prefix option.
$data_prefix="";

# Parameters for the router generated using "--help-profile-pl")

################################################################################

# Transport types
@router_transports=('foot', 'horse', 'wheelchair', 'bicycle', 'moped', 'motorbike', 'motorcar', 'goods', 'hgv', 'psv');

# Highway types
@router_highways=('motorway', 'trunk', 'primary', 'secondary', 'tertiary', 'unclassified', 'residential', 'service', 'track', 'cycleway', 'path', 'steps');

# Property types
@router_properties=('paved', 'multilane', 'bridge', 'tunnel');

# Restriction types
@router_restrictions=('oneway', 'weight', 'height', 'width', 'length');

# Allowed highways
%router_profile_highway=(
      motorway => { foot =>   0,  horse =>   0,  wheelchair =>   0,  bicycle =>   0,  moped =>   0,  motorbike => 100,  motorcar => 100,  goods => 100,  hgv => 100,  psv => 100},
         trunk => { foot =>  40,  horse =>  25,  wheelchair =>  40,  bicycle =>  30,  moped =>  90,  motorbike => 100,  motorcar => 100,  goods => 100,  hgv => 100,  psv => 100},
       primary => { foot =>  50,  horse =>  50,  wheelchair =>  50,  bicycle =>  70,  moped => 100,  motorbike =>  90,  motorcar =>  90,  goods =>  90,  hgv =>  90,  psv =>  90},
     secondary => { foot =>  60,  horse =>  50,  wheelchair =>  60,  bicycle =>  80,  moped =>  90,  motorbike =>  80,  motorcar =>  80,  goods =>  80,  hgv =>  80,  psv =>  80},
      tertiary => { foot =>  70,  horse =>  75,  wheelchair =>  70,  bicycle =>  90,  moped =>  80,  motorbike =>  70,  motorcar =>  70,  goods =>  70,  hgv =>  70,  psv =>  70},
  unclassified => { foot =>  80,  horse =>  75,  wheelchair =>  80,  bicycle =>  90,  moped =>  70,  motorbike =>  60,  motorcar =>  60,  goods =>  60,  hgv =>  60,  psv =>  60},
   residential => { foot =>  90,  horse =>  75,  wheelchair =>  90,  bicycle =>  90,  moped =>  60,  motorbike =>  50,  motorcar =>  50,  goods =>  50,  hgv =>  50,  psv =>  50},
       service => { foot =>  90,  horse =>  75,  wheelchair =>  90,  bicycle =>  90,  moped =>  80,  motorbike =>  80,  motorcar =>  80,  goods =>  80,  hgv =>  80,  psv =>  80},
         track => { foot =>  95,  horse => 100,  wheelchair =>  95,  bicycle =>  90,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0},
      cycleway => { foot =>  95,  horse =>  90,  wheelchair =>  95,  bicycle => 100,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0},
          path => { foot => 100,  horse => 100,  wheelchair => 100,  bicycle =>  90,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0},
         steps => { foot =>  80,  horse =>   0,  wheelchair =>   0,  bicycle =>   0,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0}
   );

# Speed limits
%router_profile_speed=(
      motorway => { foot =>   0,  horse =>   0,  wheelchair =>   0,  bicycle =>   0,  moped =>  48,  motorbike => 112,  motorcar => 112,  goods =>  96,  hgv =>  89,  psv =>  89},
         trunk => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  96,  motorcar =>  96,  goods =>  96,  hgv =>  80,  psv =>  80},
       primary => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  96,  motorcar =>  96,  goods =>  96,  hgv =>  80,  psv =>  80},
     secondary => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  88,  motorcar =>  88,  goods =>  88,  hgv =>  80,  psv =>  80},
      tertiary => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  80,  motorcar =>  80,  goods =>  80,  hgv =>  80,  psv =>  80},
  unclassified => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  64,  motorcar =>  64,  goods =>  64,  hgv =>  64,  psv =>  64},
   residential => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  48,  motorbike =>  48,  motorcar =>  48,  goods =>  48,  hgv =>  48,  psv =>  48},
       service => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  32,  motorbike =>  32,  motorcar =>  32,  goods =>  32,  hgv =>  32,  psv =>  32},
         track => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>  16,  motorbike =>  16,  motorcar =>  16,  goods =>  16,  hgv =>  16,  psv =>  16},
      cycleway => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0},
          path => { foot =>   4,  horse =>   8,  wheelchair =>   4,  bicycle =>  20,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0},
         steps => { foot =>   4,  horse =>   0,  wheelchair =>   4,  bicycle =>   0,  moped =>   0,  motorbike =>   0,  motorcar =>   0,  goods =>   0,  hgv =>   0,  psv =>   0}
   );

# Highway properties
%router_profile_property=(
         paved => { foot =>  50,  horse =>  20,  wheelchair =>  90,  bicycle =>  50,  moped => 100,  motorbike => 100,  motorcar => 100,  goods => 100,  hgv => 100,  psv => 100},
     multilane => { foot =>  25,  horse =>  25,  wheelchair =>  25,  bicycle =>  25,  moped =>  25,  motorbike =>  75,  motorcar =>  75,  goods =>  75,  hgv =>  75,  psv =>  75},
        bridge => { foot =>  50,  horse =>  50,  wheelchair =>  50,  bicycle =>  50,  moped =>  50,  motorbike =>  50,  motorcar =>  50,  goods =>  50,  hgv =>  50,  psv =>  50},
        tunnel => { foot =>  50,  horse =>  50,  wheelchair =>  50,  bicycle =>  50,  moped =>  50,  motorbike =>  50,  motorcar =>  50,  goods =>  50,  hgv =>  50,  psv =>  50}
   );

# Restrictions
%router_profile_restrictions=(
        oneway => { foot =>    0,  horse =>    1,  wheelchair =>    0,  bicycle =>    1,  moped =>    1,  motorbike =>    1,  motorcar =>    1,  goods =>    1,  hgv =>    1,  psv =>    1},
        weight => { foot =>  0.0,  horse =>  0.0,  wheelchair =>  0.0,  bicycle =>  0.0,  moped =>  0.0,  motorbike =>  0.0,  motorcar =>  0.0,  goods =>  5.0,  hgv => 10.0,  psv => 15.0},
        height => { foot =>  0.0,  horse =>  0.0,  wheelchair =>  0.0,  bicycle =>  0.0,  moped =>  0.0,  motorbike =>  0.0,  motorcar =>  0.0,  goods =>  2.5,  hgv =>  3.0,  psv =>  3.0},
         width => { foot =>  0.0,  horse =>  0.0,  wheelchair =>  0.0,  bicycle =>  0.0,  moped =>  0.0,  motorbike =>  0.0,  motorcar =>  0.0,  goods =>  2.0,  hgv =>  2.5,  psv =>  2.5},
        length => { foot =>  0.0,  horse =>  0.0,  wheelchair =>  0.0,  bicycle =>  0.0,  moped =>  0.0,  motorbike =>  0.0,  motorcar =>  0.0,  goods =>  5.0,  hgv =>  6.0,  psv =>  6.0}
   );

################################################################################


# Run the router

sub RunRouter
  {
   ($optimise,%params)=@_;

   # Combine all of the parameters together

   $params="--$optimise";

   foreach $key (keys %params)
     {
      $params.=" --$key=$params{$key}";
     }

   # Change directory

   mkdir $results_dir,0755 if(! -d $results_dir);
   chdir $results_dir;

   # Create a unique output directory

   chomp($uuid=`echo '$params' $$ | md5sum | cut -f1 '-d '`);

   mkdir $uuid;
   chmod 0775, $uuid;
   chdir $uuid;

   # Run the router

   $params.=" --dir=$data_dir" if($data_dir);
   $params.=" --prefix=$data_prefix" if($data_prefix);
   $params.=" --quiet";

   $message=`$bin_dir/router $params 2>&1`;

   (undef,undef,$cuser,$csystem) = times;
   $time=sprintf "time: %.3f CPU / %.3f elapsed",$cuser+$csystem,tv_interval($t0);

   if(-f "$optimise.txt")
     {
      $result=`tail -1 $optimise.txt`;
      @result=split(/\t/,$result);
      $result = $result[4]." , ".$result[5];
     }

   # Return the results

   return($uuid,$time,$result,$message);
  }

1;
