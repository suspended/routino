#!/bin/sh -x

# Download the file.

wget http://openlayers.org/download/OpenLayers-2.7.tar.gz

# Uncompress it.

tar -xzf OpenLayers-2.7.tar.gz

# Copy the files.

cp -p  OpenLayers-2.7/OpenLayers.js .
cp -pr OpenLayers-2.7/img .
cp -pr OpenLayers-2.7/theme .
