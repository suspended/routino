#!/usr/bin/perl
#
# Routino log summary tool.
#
# Part of the Routino routing software.
#
# This file Copyright 2011-2014 Andrew M. Bishop
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

# Command line

my $verbose=0;
$verbose=1 if($#ARGV==0 && $ARGV[0] eq "-v");

my $html=0;
$html=1 if($#ARGV==0 && $ARGV[0] eq "-html");

die "Usage: $0 [-v | -html] < <error-log-file>\n" if($#ARGV>0 || ($#ARGV==0 && !$verbose && !$html));


# Read in each line from the error log and store them

my %errors=();
my %errorids=();
my %errortypes=();

while(<STDIN>)
  {
   s%\r*\n%%;

   my $errorid="";
   my $errortype="";

   if(m%nodes ([0-9]+) and ([0-9]+) in way ([0-9]+)%i) # Special case pair of nodes and a way
     {
      $errorid="($1 $2 $3)";
      $errortype="N2W";
      s%nodes [0-9]+ and [0-9]+ in way [0-9]+%nodes <node-id1> and <node-id2> in way <way-id>%;
     }

   elsif(m%node ([0-9]+) in way ([0-9]+)%i) # Special case node and a way
     {
      $errorid="($1 $2)";
      $errortype="NW";
      s%Node [0-9]+ in way [0-9]+%Node <node-id> in way <way-id>%;
     }

   elsif(m%way ([0-9]+) contains node ([0-9]+)%i) # Special case way and node
     {
      $errorid="($1 $2)";
      $errortype="WN";
      s%Way [0-9]+ contains node [0-9]+%Way <way-id> contains node <node-id>%;
     }

   elsif(m%nodes ([0-9]+) and ([0-9]+)%i) # Special case pair of nodes
     {
      $errorid="($1 $2)";
      $errortype="N2";
      s%nodes [0-9]+ and [0-9]+%nodes <node-id1> and <node-id2>%;
     }

   elsif(m%Segment (contains|connects) node ([0-9]+)%) # Special case node
     {
      $errorid=$2;
      $errortype="N";
      s%node [0-9]+%node <node-id>%;
     }

   elsif(m%Relation ([0-9]+).* contains Node ([0-9]+)%) # Special case relation/node
     {
      $errorid="($1 $2)";
      $errortype="RN";
      s%Relation [0-9]+%Relation <relation-id>%;
      s%Node [0-9]+%node <node-id>%;
     }

   elsif(m%Relation ([0-9]+).* contains Way ([0-9]+)%) # Generic case relation/way
     {
      $errorid="($1 $2)";
      $errortype="RW";
      s%Relation [0-9]+%Relation <relation-id>%;
      s%Way [0-9]+%way <way-id>%;
     }

   elsif(!m%Way ([0-9]+)% && !m%Relation ([0-9]+)% && m%Node ([0-9]+)%) # Generic node
     {
      $errorid=$1;
      $errortype="N";
      s%Node [0-9]+%Node <node-id>%;
     }

   elsif(!m%Node ([0-9]+)% && !m%Relation ([0-9]+)% && m%Way ([0-9]+)%) # Generic way
     {
      $errorid=$1;
      $errortype="W";
      s%Way [0-9]+%Way <way-id>%;
     }

   elsif(!m%Node ([0-9]+)% && !m%Way ([0-9]+)% && m%Relation ([0-9]+)%) # Generic relation
     {
      $errorid=$1;
      $errortype="R";
      s%Relation [0-9]+%Relation <relation-id>%;
     }

   else
     {
      $errorid="ERROR";
      $errortype="E";
      warn "Unrecognised error message '$_'\n";
     }

   $errors{$_}++;

   if($verbose || $html)
     {
      if(defined $errorids{$_})
        {
         push(@{$errorids{$_}},$errorid);
        }
      else
        {
         $errorids{$_}=[$errorid];
        }
     }

   if($html)
     {
      $errortypes{$_}=$errortype;
     }
  }


# Print out the results as text

if( ! $html )
  {
   foreach my $error (sort { if ( $errors{$b} == $errors{$a} ) { return $errors{$a} cmp $errors{$b} }
                             else                              { return $errors{$b} <=> $errors{$a} } } (keys %errors))
     {
      printf "%9d : $error\n",$errors{$error};

      if($verbose)
        {
         my @ids=sort({ return $a <=> $b } @{$errorids{$error}});

         print "            ".join(",",@ids)."\n";
        }
     }
  }

# Print out the results as HTML

else
  {

   print "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n".
         "<HTML>\n".
         "\n".
         "<HEAD>\n".
         "<TITLE>Routino Error Log File Summary</TITLE>\n".
         "<META http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n".
         "<STYLE type=\"text/css\">\n".
         "<!--\n".
         "   body {font-family: sans-serif; font-size: 12px;}\n".
         "   h1   {font-family: sans-serif; font-size: 14px; font-style: bold;}\n".
         "   h2   {font-family: sans-serif; font-size: 13px; font-style: bold;}\n".
         "   h3   {font-family: sans-serif; font-size: 12px; font-style: bold;}\n".
         "-->\n".
         "</STYLE>\n".
         "</HEAD>\n".
         "\n".
         "<BODY>\n".
         "\n".
         "<h1>Routino Error Log File Summary</h1>\n".
         "\n".
         "This HTML file contains a summary of the Routino OSM parser error log file with\n".
         "links to the OSM website that allow browsing each of the nodes, ways or relations\n".
         "that are responsible for the error messages.\n".
         "\n";

   my %errortypeorder=(
                       "N"   , 1,
                       "NW"  , 2,
                       "WN"  , 3,
                       "N2W" , 4,
                       "N2"  , 5,
                       "W"   , 6,
                       "R"   , 7,
                       "RN"  , 8,
                       "RW"  , 9,
                       "E"   , 10
                      );

   my %errortypelabel=(
                       "N"   , "Nodes",
                       "NW"  , "Node in a Way",
                       "WN"  , "Way contains Node",
                       "N2W" , "Node Pairs in a Way",
                       "N2"  , "Node Pairs",
                       "W"   , "Ways",
                       "R"   , "Relations",
                       "RN"  , "Relations/Nodes",
                       "RW"  , "Relations/Ways",
                       "E"   , "ERROR"
                      );

   my $lasterrortype="";

   foreach my $error (sort { if    ( $errortypes{$b} ne $errortypes{$a} ) { return $errortypeorder{$errortypes{$a}} <=> $errortypeorder{$errortypes{$b}} }
                             elsif ( $errors{$b}     == $errors{$a} )     { return $errors{$a} cmp $errors{$b} }
                             else                                         { return $errors{$b} <=> $errors{$a} } } (keys %errors))
     {
      my $errorhtml=$error;

      $errorhtml =~ s/&/&amp;/g;
      $errorhtml =~ s/</&lt;/g;
      $errorhtml =~ s/>/&gt;/g;

      if($errortypes{$error} ne $lasterrortype)
        {
         print "<h2>$errortypelabel{$errortypes{$error}}</h2>\n";
         $lasterrortype=$errortypes{$error};
        }

      print "<h3>$errorhtml</h3>\n";

      if($errors{$error}>100)
        {
         print "$errors{$error} occurences (not listed).\n";
        }
      else
        {
         my @ids=sort({ return $a <=> $b } @{$errorids{$error}});

         my $first=1;

         foreach my $id (@ids)
           {
            if($first)
              {
               print "$errortypelabel{$errortypes{$error}}:\n";
              }
            else
              {
               print ",";
              }

            $first=0;

            print "<a href=\"http://www.openstreetmap.org/browse/node/$id\">$id</a>" if($errortypes{$error} eq "N");
            print "<a href=\"http://www.openstreetmap.org/browse/way/$id\">$id</a>" if($errortypes{$error} eq "W");
            print "<a href=\"http://www.openstreetmap.org/browse/relation/$id\">$id</a>" if($errortypes{$error} eq "R");

            if($errortypes{$error} eq "NW" || $errortypes{$error} eq "WN" || $errortypes{$error} eq "N2" || $errortypes{$error} eq "RN" || $errortypes{$error} eq "RW")
              {
               $id =~ m%\(([0-9]+) ([0-9]+)\)%;
               print "(<a href=\"http://www.openstreetmap.org/browse/node/$1\">$1</a> <a href=\"http://www.openstreetmap.org/browse/way/$2\">$2</a>)" if($errortypes{$error} eq "NW");
               print "(<a href=\"http://www.openstreetmap.org/browse/way/$1\">$1</a> <a href=\"http://www.openstreetmap.org/browse/node/$2\">$2</a>)" if($errortypes{$error} eq "WN");
               print "(<a href=\"http://www.openstreetmap.org/browse/node/$1\">$1</a> <a href=\"http://www.openstreetmap.org/browse/node/$2\">$2</a>)" if($errortypes{$error} eq "N2");
               print "(<a href=\"http://www.openstreetmap.org/browse/relation/$1\">$1</a> <a href=\"http://www.openstreetmap.org/browse/node/$2\">$2</a>)" if($errortypes{$error} eq "RN");
               print "(<a href=\"http://www.openstreetmap.org/browse/relation/$1\">$1</a> <a href=\"http://www.openstreetmap.org/browse/way/$2\">$2</a>)" if($errortypes{$error} eq "RW");
              }

            if($errortypes{$error} eq "N2W")
              {
               $id =~ m%\(([0-9]+) ([0-9]+) ([0-9]+)\)%;
               print "(<a href=\"http://www.openstreetmap.org/browse/node/$1\">$1</a> <a href=\"http://www.openstreetmap.org/browse/node/$2\">$2</a> <a href=\"http://www.openstreetmap.org/browse/way/$3\">$3</a>)" if($errortypes{$error} eq "N2W");
              }

            print "\n";
           }
        }
     }

   print "\n".
         "</BODY>\n".
         "\n".
         "</HTML>\n";
}
