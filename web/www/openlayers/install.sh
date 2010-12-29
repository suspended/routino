#!/bin/sh -x

# Download the file.

wget http://openlayers.org/download/OpenLayers-2.8.tar.gz

# Uncompress it.

tar -xzf OpenLayers-2.8.tar.gz

# Copy the files.

cp -p  OpenLayers-2.8/OpenLayers.js .
cp -pr OpenLayers-2.8/img .
cp -pr OpenLayers-2.8/theme .

# Delete the remainder

rm -rf OpenLayers-2.8
