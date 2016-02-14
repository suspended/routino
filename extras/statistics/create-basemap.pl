#!/usr/bin/perl
#
# Base map creation Perl script
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

# Range and source of tiles - PARAMETERS THAT CAN BE EDITED

$baseurl='http://a.tile.openstreetmap.org/${z}/${x}/${y}.png'; # URL of tile server.

@xrange=(120..129);             # At zoom level 8
@yrange=(73..87);               # At zoom level 8

$zbase=6;                       # Zoom level of images to use.

# Base image dimensions

$basescale=2**(8-$zbase);
$tilesize=256/$basescale;

$width =(1+$#xrange)*$tilesize;
$height=(1+$#yrange)*$tilesize;

# Create a new base image

$image=Graphics::Magick->new(size => "${width}x${height}");
$image->ReadImage('xc:white');

$xb=-1;

foreach $x (@xrange)
  {
   $xbase=int($x/$basescale);

   next if($xbase==$xb);

   $yb=-1;

   foreach $y (@yrange)
     {
      $ybase=int($y/$basescale);

      next if($ybase==$yb);

      $tile=Graphics::Magick->new;

      $url=$baseurl;
      $url =~ s%\$\{x\}%$xbase%;
      $url =~ s%\$\{y\}%$ybase%;
      $url =~ s%\$\{z\}%$zbase%;

      `wget $url -O ${xbase}_${ybase}.png > /dev/null 2>&1`;

      $tile->Read("${xbase}_${ybase}.png");

      unlink "${xbase}_${ybase}.png";

      $xpos=(($xbase*$basescale)-$xrange[0])*$tilesize;
      $ypos=(($ybase*$basescale)-$yrange[0])*$tilesize;

      $image->Composite(image => $tile, compose => Over, x => $xpos, y => $ypos);

      undef $tile;

      $yb=$ybase;
     }

   $xb=$xbase;
  }

print "Writing 'basemap.png'\n";

$image->Write("basemap.png");

undef $image;
