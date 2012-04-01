#!/usr/bin/perl
#
# Routino data visualiser custom link CGI
#
# Part of the Routino routing software.
#
# This file Copyright 2008-2012 Andrew M. Bishop
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

# Use the perl CGI module
use CGI ':cgi';

# Create the query and get the parameters

$query=new CGI;

# Redirect to the HTML page.

$params="";

foreach $key ($query->param)
  {
   if($params eq "")
     {
      $params="?";
     }
   else
     {
      $params.="&";
     }

   $params.="$key=".$query->param($key);
  }

print $query->redirect("visualiser.html".$params);
