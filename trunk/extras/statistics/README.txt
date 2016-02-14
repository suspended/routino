                               Database Statistics
                               ===================

The Routino database is a concentrated source of information about the highways
in the selected region.  By stepping through the database the properties of each
segment can be checked and recorded.

The scripts and program here group the data into regions that correspond to the
standard OSM tiles (default of a zoom 13 tile).  The highway properties are
written out to a text file indexed by tile position.  A separate Perl script is
provided to create "heatmap" images from this data which is overlayed on a base
map created from standard OSM tiles.

The Perl scripts 'create-basemap.pl' and 'create-image.pl' will need modifying
to set the range of coordinates (quoted as x and y values for zoom level 8
tiles) to plot and the zoom level of the basemap and data granularity.

The shell script 'update.sh' will need modifying to set the location of the
Routino database and the zoom level of the data analysis.

The script 'update.sh' will perform all the actions to dump the database into a
text file, create the basemap and plot the data over the top.
