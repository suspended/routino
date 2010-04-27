                  Routino - OpenStreetMap Routing Software
                  ========================================


   Routino is an application for finding a route between two points using
   the dataset of topographical information collected by
   http://www.OpenStreetMap.org.

   Starting from the raw OpenStreetMap data (in the form of the '.osm'
   XML files available on the internet) a custom database is generated
   that contains the information useful for routing. With this database
   and two points specified by latitude and longitude an optimum route
   (either shortest or quickest) is determined and output as a text
   description and a track in GPX (GPS eXchange) XML format. The route
   is calculated for OpenStreetMap highways (roads, paths etc) using one
   of the common forms of transport defined in OpenStreetMap (foot,
   bicycle, horse, motorcar, motorbike etc).

   When processing the OpenStreetMap data the types of highways are
   recorded and these set default limits on the types of traffic allowed.
   More specific information about permissions for different types of
   transport are also recorded as are maximum speed limits. Further
   restrictions like oneway streets, weight, height, width and length
   limits are also included where specified.

   When calculating a route the type of transport to be used is taken into
   account to ensure that the known restrictions are followed. Each of the
   different highway types can further be allowed or disallowed depending
   on preferences. For each type of highway a default speed limit is
   defined (although the actual speed used will be the lowest of the
   default and any specified in the original data). To make use of the
   information about restrictions the weight, height, width and length of
   the transport can also be specified.

   One of the design aims of Routino was to make the software are flexible
   as possible in selecting routing preferences but also have a sensible
   set of default values. Another design aim was that finding the optimum
   route should be very fast and most of the speed increases come from the
   carefully chosen and optimised data format.


Disclaimer
----------

   The route that is calculated by this software is only as good as the
   input data.

   Routino comes with ABSOLUTELY NO WARRANTY for the software itself or
   the route that is calculated.


Demonstration
-------------

   A live demonstration of the router is available on the internet:

   http://www.gedanken.org.uk/mapping/routino/router.html

   The source code download available also includes a set of files that can
   be used to create your own interactive map.

   The interactive map is made possible by use of the OpenLayers Javascript
   library from http://www.openlayers.org/.


Documentation
-------------

   The algorithm used and the way that the OpenStreetMap data tags
   are used are described in detail in their own files, USAGE.txt and
   TAGGING.txt.

   Detailed information about how to use the programs is available in the
   file USAGE.txt and how to install it is in INSTALL.txt.


Status
------

   Version 1.0 of Routino was released on 8th April 2009.
   Version 1.1 of Routino was released on 13th June 2009.
   Version 1.2 of Routino was released on 21st October 2009.
   Version 1.3 of Routino was released on 21st January 2010.


License
-------

   This program is free software: you can redistribute it and/or modify it
   under the terms of the GNU Affero General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   It is important to note that for this program I have decided to use the
   Affero GPLv3 instead of just using the GPL. This license adds
   additional requirements to anybody who provides a networked service
   using this software.


Copyright
---------

   Routino is copyright Andrew M. Bishop 2008-2010.

   Contact amb@gedanken.demon.co.uk for any questions or queries.


Homepage
--------

   The latest information about the program can be found on the homepage:

   http://www.gedanken.org.uk/software/routino/


Download
--------

   The program can be downloaded from:

   http://www.gedanken.org.uk/software/routino/download/


--------

Copyright 2008-2010 Andrew M. Bishop.
