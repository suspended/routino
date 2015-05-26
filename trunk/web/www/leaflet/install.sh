#!/bin/sh -x

version=0.7.3

# Download the file.

wget http://leaflet-cdn.s3.amazonaws.com/build/leaflet-$version.zip

# Uncompress it.

unzip leaflet-$version.zip
