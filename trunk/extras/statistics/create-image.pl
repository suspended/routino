#!/usr/bin/perl
#
# OSM data statistics Perl script
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2016 Andrew M. Bishop
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

use Graphics::Magick;

# Range of tiles and zoom - PARAMETERS THAT CAN BE EDITED

@xrange=(120..129);             # At zoom level 8
@yrange=(73..87);               # At zoom level 8

$zbase=6;                       # Zoom level of images in basemap.

$z=13;                          # Granularity of data points.

# Base image dimensions

$basescale=2**(8-$zbase);

$width =(1+$#xrange)*256/$basescale;
$height=(1+$#yrange)*256/$basescale;

# Chosen zoom level

$scale=2**(8-$z);
$tilesize=256*$scale/$basescale;

# Get the command line arguments

$prefix=$ARGV[0];
$name  =$ARGV[1];
$number=$ARGV[2]+0;

# Graph annotations

$annotation{highway}  ="'highway=$name'";
$annotation{property} ="$name property";
$annotation{speed}    ="'maxspeed=$name'";
$annotation{transport}="$name allowed";

# Read in the data

%density=();

while(<STDIN>)
  {
   ($x,$y,@distances)=split(/ +/);

   $distance=$distances[$number];

   if($distance > 0)
     {
      $area=&xy_area($z,$x,$y);

      $density{$x,$y}=($distance/$area);
     }
  }

# Find the maximum value

$max=0;

foreach $xy (keys %density)
  {
   $density=$density{$xy};

   $max=$density if($density>$max);
  }

$max=500.0*int(($max+499)/500);
$max=500.0 if($max<500);

# Create the overlay image

$colour0=&colour(0);

$image=Graphics::Magick->new(size => "${width}x${height}");
$image->ReadImage("xc:$colour0");

foreach $xy (keys %density)
  {
   ($x,$y)=split($;,$xy);

   $colour=&colour($density{$x,$y}/$max);

   $x1=(($x*$scale)-$xrange[0])*256/$basescale;
   $y1=(($y*$scale)-$yrange[0])*256/$basescale;

   if($tilesize==1)
     {
      $image->Draw(primitive => 'point', points => "$x1,$y1",
                   fill => $colour,
                   antialias => 'false');
     }
   else
     {
      $x2=$x1+$tilesize-1;
      $y2=$y1+$tilesize-1;

      $image->Draw(primitive => 'rectangle', points => "$x1,$y1 $x2,$y2",
                   strokewidth => 0, stroke => $colour, fill => $colour,
                   antialias => 'false');
     }
  }

# Create the scale indicator

$indicator=Graphics::Magick->new(size => "${width}x40");
$indicator->ReadImage('xc:white');

foreach $v (0..($width-2*5))
  {
   $x=$v+5;
   $y1=12; $y2=23;
   $density=$v/($width-2*5);

   $indicator->Draw(primitive => 'line', points => "$x,$y1,$x,$y2",
                    stroke => &colour($density),
                    antialias => 'false');
  }

$indicator->Annotate(text => "0", font => 'Helvetica', pointsize => '12', style => Normal,
                     fill => 'black',
                     x => 5, y => 11, align => Left);

foreach $frac (0.25,0.5,0.75)
  {
   $indicator->Annotate(text => $max*$frac, font => 'Helvetica', pointsize => '12', style => Normal,
                        fill => 'black',
                        x => 5+($width-2*5)*$frac, y => 11, align => Center);
  }

$indicator->Annotate(text => $max, font => 'Helvetica', pointsize => '12', style => Normal,
                     fill => 'black',
                     x => $width-5, y => 11, align => Right);

$indicator->Annotate(text => "Highway density (metres/square km) for $annotation{$prefix} per zoom $z tile", font => 'Helvetica', pointsize => '14', style => Normal,
                     fill => 'black',
                     x => $width/2, y => 36, align => Center);

# Create the combined images

$base=Graphics::Magick->new;
$base->ReadImage("basemap.png");

$base->Composite(image => $image, compose => Dissolve, x => 0, y => 0, opacity => 50);

$final=Graphics::Magick->new(size => ($base->Get('width')+2)."x".($base->Get('height')+$indicator->Get('height')+3));
$final->ReadImage('xc:black');

$final->Composite(image => $base     , compose => Over, x => 1, y => 1                     );
$final->Composite(image => $indicator, compose => Over, x => 1, y => $base->Get('height')+2);

undef $base;
undef $image;
undef $indicator;

# Write out the images

print "Writing '$prefix-$name.png'\n";

$final->Write("$prefix-$name.png");

$final->Resize(width => $width/4, height => $final->Get('height')/4);
$final->Write("$prefix-$name-small.png");

undef $final;

#
# Subroutines
#

sub xy_ll_rad
{
 my($zoom,$x,$y)=@_;

 $PI=3.14159265358979323846;

 my($longitude)=$PI*(($x * (2**(1-$zoom)))-1);
 my($latitude)=($PI/2)*((4/$PI)*atan2(exp($PI*(1-($y * (2**(1-$zoom))))),1) - 1);

 return ($longitude,$latitude);
}


sub xy_area
{
 my($zoom,$x,$y)=@_;

 $RADIUS=6378.137;

 my($width,$height);

 if(defined $width{$y})
   {
    $width=$width{$y};
   }
 else
   {
    my($lon1,$lat1)=&xy_ll_rad($z,$x  ,$y);
    my($lon2,$lat2)=&xy_ll_rad($z,$x+1,$y);

    $width=$RADIUS*($lon2-$lon1)*cos($lat1);

    $width{$y}=$width;
   }

 if(defined $height{$y})
   {
    $height=$height{$y};
   }
 else
   {
    my($lon1,$lat1)=&xy_ll_rad($z,$x,$y  );
    my($lon2,$lat2)=&xy_ll_rad($z,$x,$y+1);

    $height=$RADIUS*($lat1-$lat2);

    $height{$y}=$height;
   }

 return $width*$height;
}

sub colour
{
 my($density)=@_;

 $density=sqrt($density);

 $density=0 if($density<0);
 $density=1 if($density>1);

 my($r,$g,$b);

 if($density<0.5)
   {
    $r=0;
    $g=int(255*(2*$density));
    $b=int(255*(1-2*$density));
   }
 else
   {
    $density-=0.5;

    $r=int(255*(2*$density));
    $g=int(255*(1-2*$density));
    $b=0;
   }

 sprintf("#%02x%02x%02x",$r,$g,$b);
}
