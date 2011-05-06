#!/bin/sh

# Exit on error

set -e

# Test name

name=`basename $0 .sh`

# Slim or non-slim

if [ "$1" = "slim" ]; then
    slim="-slim"
    dir="slim"
else
    slim=""
    dir="fat"
fi

[ -d $dir ] || mkdir $dir

# Name related options

osm=$name.osm
log=$name$slim.log

option_prefix="--prefix=$name"
option_dir="--dir=$dir"

# Generic program options

option_planetsplitter="--loggable --tagging=../../xml/routino-tagging.xml"
option_router="--loggable --transport=motorcar --profiles=../../xml/routino-profiles.xml --translations=../../xml/routino-translations.xml"

# Run planetsplitter

echo ../planetsplitter$slim $option_dir $option_prefix $option_planetsplitter $osm
echo ../planetsplitter$slim $option_dir $option_prefix $option_planetsplitter $osm > $log
../planetsplitter$slim $option_dir $option_prefix $option_planetsplitter $osm >> $log

# Waypoints

waypoints=`perl waypoints.pl $osm list`

waypoint_start=`perl waypoints.pl $osm WPstart 1`
waypoint_finish=`perl waypoints.pl $osm WPfinish 3`

# Run the router for each waypoint

for waypoint in $waypoints; do

    [ ! $waypoint = "WPstart"  ] || continue
    [ ! $waypoint = "WPfinish" ] || continue

    echo "Waypoint : $waypoint"

    waypoint_test=`perl waypoints.pl $osm $waypoint 2`

    [ -d $dir/$name-$waypoint ] || mkdir $dir/$name-$waypoint

    echo ../router$slim $option_dir $option_prefix $option_osm $option_router $waypoint_start $waypoint_test $waypoint_finish
    echo ../router$slim $option_dir $option_prefix $option_osm $option_router $waypoint_start $waypoint_test $waypoint_finish >> $log
    ../router$slim $option_dir $option_prefix $option_osm $option_router $waypoint_start $waypoint_test $waypoint_finish >> $log

    mv shortest* $dir/$name-$waypoint

done
