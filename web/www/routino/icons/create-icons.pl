#!/usr/bin/perl

use Graphics::Magick;

# Markers for routing

foreach $character ('0'..'9')
  {
   $image=Graphics::Magick->new;
   $image->Set(size => "63x75");

   $image->ReadImage('xc:transparent');

   $image->Draw(primitive => polygon, points => '1,32 32,73 61,32 32,10',
                stroke => 'black', fill => 'white', strokewidth => 6,
                antialias => false);

   $image->Draw(primitive => arc,     points => '1,1 61,61 -180,0',
                stroke => 'black', fill => 'white', strokewidth => 6,
                antialias => false);

   ($x_ppem, $y_ppem, $ascender, $descender, $width, $height, $max_advance) = $image->QueryFontMetrics(text => $character);

   $image->Annotate(text => $character, font => 'Helvetica', pointsize => '36',
                    fill => 'red',
                    x => 32, y => 34+$y_ppem/2, align => Center,
                    antialias => false);

   $image->Resize(width => 21, height => 25);

   $image->Write("marker-$character.png");

   undef $image;
  }

# Balls for visualiser descriptions

@colours=("#FFFFFF",
          "#FF0000",
          "#FFFF00",
          "#00FF00",
          "#8B4513",
          "#00BFFF",
          "#FF69B4",
          "#000000",
          "#000000",
          "#000000");

foreach $colour (0..9)
  {
   $image=Graphics::Magick->new;
   $image->Set(size => "9x9");

   $image->ReadImage('xc:transparent');

   $image->Draw(primitive => circle, points => '4,4 4,8',
                fill => $colours[$colour], stroke => $colours[$colour],
                antialias => false);

   $image->Write("ball-$colour.png");

   undef $image;
  }

# Limit signs

foreach $limit (1..160)
  {
   &draw_limit($limit);
  }

foreach $limit (10..200)
  {
   &draw_limit(sprintf "%.1f",$limit/10);
  }

&draw_limit("no");

link "limit-no.png","limit-0.png";

sub draw_limit
  {
   ($limit)=@_;

   $image=Graphics::Magick->new;
   $image->Set(size => "57x57");

   $image->ReadImage('xc:transparent');

   $image->Draw(primitive => circle, points => '28,28 28,55',
                stroke => 'red', fill => 'white', strokewidth => 3,
                antialias => false);

   ($x_ppem, $y_ppem, $ascender, $descender, $width, $height, $max_advance) = $image->QueryFontMetrics(text => "$limit");

   $image->Annotate(text => "$limit", font => 'Helvetica', pointsize => '22',
                    fill => 'black',
                    x => 28, y => 28+$y_ppem/2, align => Center,
                    antialias => false);

   $image->Resize(width => 19, height => 19);

   $image->Write("limit-$limit.png");

   undef $image;
  }
