//
// Routino router web page Javascript
//
// Part of the Routino routing software.
//
// This file Copyright 2008,2009,2010 Andrew M. Bishop
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

//
// Parameters for the router (generated using "--help-profile-js").
//

////////////////////////////////////////////////////////////////////////////////

// Transport types
var router_transports={foot: 1, horse: 2, wheelchair: 3, bicycle: 4, moped: 5, motorbike: 6, motorcar: 7, goods: 8, hgv: 9, psv: 10};

// Highway types
var router_highways={motorway: 1, trunk: 2, primary: 3, secondary: 4, tertiary: 5, unclassified: 6, residential: 7, service: 8, track: 9, cycleway: 10, path: 11, steps: 12};

// Property types
var router_properties={paved: 1, multilane: 2, bridge: 3, tunnel: 4};

// Restriction types
var router_restrictions={oneway: 1, weight: 2, height: 3, width: 4, length: 5};

// Allowed highways
var router_profile_highway={
      motorway: {foot:   0, horse:   0, wheelchair:   0, bicycle:   0, moped:   0, motorbike: 100, motorcar: 100, goods: 100, hgv: 100, psv: 100},
         trunk: {foot:  40, horse:  25, wheelchair:  40, bicycle:  30, moped:  90, motorbike: 100, motorcar: 100, goods: 100, hgv: 100, psv: 100},
       primary: {foot:  50, horse:  50, wheelchair:  50, bicycle:  70, moped: 100, motorbike:  90, motorcar:  90, goods:  90, hgv:  90, psv:  90},
     secondary: {foot:  60, horse:  50, wheelchair:  60, bicycle:  80, moped:  90, motorbike:  80, motorcar:  80, goods:  80, hgv:  80, psv:  80},
      tertiary: {foot:  70, horse:  75, wheelchair:  70, bicycle:  90, moped:  80, motorbike:  70, motorcar:  70, goods:  70, hgv:  70, psv:  70},
  unclassified: {foot:  80, horse:  75, wheelchair:  80, bicycle:  90, moped:  70, motorbike:  60, motorcar:  60, goods:  60, hgv:  60, psv:  60},
   residential: {foot:  90, horse:  75, wheelchair:  90, bicycle:  90, moped:  60, motorbike:  50, motorcar:  50, goods:  50, hgv:  50, psv:  50},
       service: {foot:  90, horse:  75, wheelchair:  90, bicycle:  90, moped:  80, motorbike:  80, motorcar:  80, goods:  80, hgv:  80, psv:  80},
         track: {foot:  95, horse: 100, wheelchair:  95, bicycle:  90, moped:   0, motorbike:   0, motorcar:   0, goods:   0, hgv:   0, psv:   0},
      cycleway: {foot:  95, horse:  90, wheelchair:  95, bicycle: 100, moped:   0, motorbike:   0, motorcar:   0, goods:   0, hgv:   0, psv:   0},
          path: {foot: 100, horse: 100, wheelchair: 100, bicycle:  90, moped:   0, motorbike:   0, motorcar:   0, goods:   0, hgv:   0, psv:   0},
         steps: {foot:  80, horse:   0, wheelchair:   0, bicycle:   0, moped:   0, motorbike:   0, motorcar:   0, goods:   0, hgv:   0, psv:   0}
   };

// Speed limits
var router_profile_speed={
      motorway: {foot:   0, horse:   0, wheelchair:   0, bicycle:   0, moped:  48, motorbike: 112, motorcar: 112, goods:  96, hgv:  89, psv:  89},
         trunk: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:  48, motorbike:  96, motorcar:  96, goods:  96, hgv:  80, psv:  80},
       primary: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:  48, motorbike:  96, motorcar:  96, goods:  96, hgv:  80, psv:  80},
     secondary: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:  48, motorbike:  88, motorcar:  88, goods:  88, hgv:  80, psv:  80},
      tertiary: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:  48, motorbike:  80, motorcar:  80, goods:  80, hgv:  80, psv:  80},
  unclassified: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:  48, motorbike:  64, motorcar:  64, goods:  64, hgv:  64, psv:  64},
   residential: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:  48, motorbike:  48, motorcar:  48, goods:  48, hgv:  48, psv:  48},
       service: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:  32, motorbike:  32, motorcar:  32, goods:  32, hgv:  32, psv:  32},
         track: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:  16, motorbike:  16, motorcar:  16, goods:  16, hgv:  16, psv:  16},
      cycleway: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:   0, motorbike:   0, motorcar:   0, goods:   0, hgv:   0, psv:   0},
          path: {foot:   4, horse:   8, wheelchair:   4, bicycle:  20, moped:   0, motorbike:   0, motorcar:   0, goods:   0, hgv:   0, psv:   0},
         steps: {foot:   4, horse:   0, wheelchair:   4, bicycle:   0, moped:   0, motorbike:   0, motorcar:   0, goods:   0, hgv:   0, psv:   0}
   };

// Highway properties
var router_profile_property={
         paved: {foot:  50, horse:  20, wheelchair:  90, bicycle:  50, moped: 100, motorbike: 100, motorcar: 100, goods: 100, hgv: 100, psv: 100},
     multilane: {foot:  25, horse:  25, wheelchair:  25, bicycle:  25, moped:  25, motorbike:  75, motorcar:  75, goods:  75, hgv:  75, psv:  75},
        bridge: {foot:  50, horse:  50, wheelchair:  50, bicycle:  50, moped:  50, motorbike:  50, motorcar:  50, goods:  50, hgv:  50, psv:  50},
        tunnel: {foot:  50, horse:  50, wheelchair:  50, bicycle:  50, moped:  50, motorbike:  50, motorcar:  50, goods:  50, hgv:  50, psv:  50}
   };

// Restrictions
var router_profile_restrictions={
        oneway: {foot:    0, horse:    1, wheelchair:    0, bicycle:    1, moped:    1, motorbike:    1, motorcar:    1, goods:    1, hgv:    1, psv:    1},
        weight: {foot:  0.0, horse:  0.0, wheelchair:  0.0, bicycle:  0.0, moped:  0.0, motorbike:  0.0, motorcar:  0.0, goods:  5.0, hgv: 10.0, psv: 15.0},
        height: {foot:  0.0, horse:  0.0, wheelchair:  0.0, bicycle:  0.0, moped:  0.0, motorbike:  0.0, motorcar:  0.0, goods:  2.5, hgv:  3.0, psv:  3.0},
         width: {foot:  0.0, horse:  0.0, wheelchair:  0.0, bicycle:  0.0, moped:  0.0, motorbike:  0.0, motorcar:  0.0, goods:  2.0, hgv:  2.5, psv:  2.5},
        length: {foot:  0.0, horse:  0.0, wheelchair:  0.0, bicycle:  0.0, moped:  0.0, motorbike:  0.0, motorcar:  0.0, goods:  5.0, hgv:  6.0, psv:  6.0}
   };

////////////////////////////////////////////////////////////////////////////////

// Currently selected transport
var router_transport=false;

// List of route points
var routepoints={shortest: {}, quickest: {}};


//
// Form initialisation - fill in the uninitialised parts
//

function form_init()
{
 var key;

 for(key in router_transports)
    if(document.forms["form"].elements["transport"][router_transports[key]-1].checked)
       router_transport=key;

 if(!router_transport)
    formSetTransport('motorcar');
 else
   {
    for(key in router_profile_highway)
      {
       if(document.forms["form"].elements["highway-" + key].value=="")
          document.forms["form"].elements["highway-" + key].value=router_profile_highway[key][router_transport];
       else
          formSetHighway(key);
      }

    for(key in router_profile_speed)
      {
       if(document.forms["form"].elements["speed-" + key].value=="")
          document.forms["form"].elements["speed-" + key].value=router_profile_speed[key][router_transport];
       else
          formSetSpeed(key);
      }

    for(key in router_profile_property)
      {
       if(document.forms["form"].elements["property-" + key].value=="")
          document.forms["form"].elements["property-" + key].value=router_profile_property[key][router_transport];
       else
          formSetProperty(key);
      }

    for(key in router_restrictions)
      {
       if(key=="oneway")
          formSetRestriction(key);
       else
         {
          if(document.forms["form"].elements["restrict-" + key].value=="")
             document.forms["form"].elements["restrict-" + key].value=router_profile_restrictions[key][router_transport];
          else
             formSetRestriction(key);
         }
      }
   }

 for(var marker=nmarkers;marker>=1;marker--)
   {
    var lon=document.forms["form"].elements["lon" + marker].value;
    var lat=document.forms["form"].elements["lat" + marker].value;

    if(lon != "" && lat != "")
       markerAddMap(marker);
    else
       markerRemove(marker);
   }

 updateCustomURL();
}


//
// Change of transport in the form
//

function formSetTransport(type)
{
 var key;

 router_transport=type;

 for(key in router_transports)
    document.forms["form"].elements["transport"][router_transports[key]-1].checked=(key==router_transport);

 for(key in router_profile_highway)
    document.forms["form"].elements["highway-" + key].value=router_profile_highway[key][router_transport];

 for(key in router_profile_speed)
    document.forms["form"].elements["speed-" + key].value=router_profile_speed[key][router_transport];

 for(key in router_profile_property)
    document.forms["form"].elements["property-" + key].value=router_profile_property[key][router_transport];

 for(key in router_restrictions)
   {
    if(key=="oneway")
       document.forms["form"].elements["restrict-" + key].checked=router_profile_restrictions[key][router_transport];
    else
       document.forms["form"].elements["restrict-" + key].value=router_profile_restrictions[key][router_transport];
   }

 paramschanged=true;

 updateCustomURL();
}


//
// Change of highway in the form
//

function formSetHighway(type)
{
 router_profile_highway[type][router_transport]=document.forms["form"].elements["highway-" + type].value;

 paramschanged=true;

 updateCustomURL();
}


//
// Change of Speed in the form
//

function formSetSpeed(type)
{
 router_profile_speed[type][router_transport]=document.forms["form"].elements["speed-" + type].value;

 paramschanged=true;

 updateCustomURL();
}


//
// Change of Property in the form
//

function formSetProperty(type)
{
 router_profile_property[type][router_transport]=document.forms["form"].elements["property-" + type].value;

 paramschanged=true;

 updateCustomURL();
}


//
// Change of oneway rule in the form
//

function formSetRestriction(type)
{
 if(type=="oneway")
    router_profile_restrictions[type][router_transport]=document.forms["form"].elements["restrict-" + type].checked;
 else
    router_profile_restrictions[type][router_transport]=document.forms["form"].elements["restrict-" + type].value;

 paramschanged=true;

 updateCustomURL();
}


//
// Update custom URL
//

function updateCustomURL()
{
 var custom_url=document.getElementById("custom_url");
 var visualiser_url=document.getElementById("visualiser_url");

 custom_url.href="customrouter.cgi" + buildURLArguments(1) + ";" + map_args;

 visualiser_url.href="customvisualiser.cgi?" + map_args;
}


//
// Block the use of the return key to submit the form
//

function block_return_key()
{
 var form=document.getElementById("form");

 if(form.addEventListener)
    form.addEventListener('keyup', discardReturnKey, false);
 else if(form.attachEvent)
    form.attachEvent('keyup', discardReturnKey); // Internet Explorer
}

//
// Function to discard the return key if pressed
//

function discardReturnKey(ev)
{
 if(ev.keyCode==13)
    return(false);

 return(true);
}


//
// Map configuration
//

var map;
var layerMapOSM, layerVectors, layerGPX;
var epsg4326, epsg900913;
var map_args;

var nmarkers, vismarkers, markers, markersmoved, paramschanged;
var highlights={shortest: null, quickest: null};
var popups={shortest: null, quickest: null};
var gpx_style;


// 
// Initialise the 'map' object
//

function map_init(lat,lon,zoom)
{
 //
 // Create the map
 //

 epsg4326=new OpenLayers.Projection("EPSG:4326");
 epsg900913=new OpenLayers.Projection("EPSG:900913");

 // UK coordinate range: West -11.0, South 49.5, East 2.0, North 61.0

 // EDIT THIS to change the visible map boundary.
 var mapbounds=new OpenLayers.Bounds(-11.0,49.5,2.0,61.0).transform(epsg4326,epsg900913);

 map = new OpenLayers.Map ("map",
                           {
                            controls:[
                                      new OpenLayers.Control.Navigation(),
                                      new OpenLayers.Control.PanZoomBar(),
                                      new OpenLayers.Control.ScaleLine(),
                                      new OpenLayers.Control.LayerSwitcher()
                                      ],

                            projection: epsg900913,
                            displayProjection: epsg4326,

                            // EDIT THIS to set the minimum zoom level
                            minZoomLevel: 4,

                            // EDIT THIS to set the number of zoom levels
                            numZoomLevels: 12, // zoom levels 4-15 inclusive

                            // EDIT THIS if you change the minimum zoom level above
                            maxResolution: 156543.0339 / Math.pow(2,4), // Math.pow(2,minZoomLevel)

                            maxExtent: new OpenLayers.Bounds(-20037508.34, -20037508.34, 20037508.34, 20037508.34),

                            restrictedExtent: mapbounds,

                            units: "m"
                           });

 map.events.register("moveend", map, mapMoved);

 // Add a map tile layer (OpenStreetMap tiles, direct access)

 layerMapOSM = new OpenLayers.Layer.TMS("Original OSM map",
                                        "http://tile.openstreetmap.org/",
                                        {
                                         emptyUrl: "http://openstreetmap.org/openlayers/img/404.png",
                                         type: 'png',
                                         getURL: limitedUrl,
                                         displayOutsideMaxExtent: true,
                                         buffer: 1
                                        });
 map.addLayer(layerMapOSM);

 // Get a URL for the tile; limited to mapbounds.

 function limitedUrl(bounds)
 {
  var z = map.getZoom() + map.minZoomLevel;

  if (z>=7 && (bounds.right  < mapbounds.left ||
               bounds.left   > mapbounds.right ||
               bounds.top    < mapbounds.bottom ||
               bounds.bottom > mapbounds.top))
     return this.emptyUrl;

  var res = map.getResolution();
  var y = Math.round((this.maxExtent.top - bounds.top) / (res * this.tileSize.h));
  var limit = Math.pow(2, z);

  if (y < 0 || y >= limit)
    return this.emptyUrl;

  var x = Math.round((bounds.left - this.maxExtent.left) / (res * this.tileSize.w));

  x = ((x % limit) + limit) % limit;
  return this.url + z + "/" + x + "/" + y + "." + this.type;
 }

 // Define a GPX layer but don't add it yet

 layerGPX={shortest: null, quickest: null};

 gpx_style={shortest: new OpenLayers.Style({},{strokeWidth: 3, strokeColor: "#00FF00"}),
            quickest: new OpenLayers.Style({},{strokeWidth: 3, strokeColor: "#0000FF"})};
 
 // Add a vectors layer
 
 layerVectors = new OpenLayers.Layer.Vector("Markers");
 map.addLayer(layerVectors);

 // A set of markers

 nmarkers=99;
 vismarkers=0;
 markers={};
 markersmoved=false;
 paramschanged=false;

 for(var marker=1;marker<=nmarkers;marker++)
   {
    if(document.forms["form"].elements["lon" + marker] != undefined)
      {
       markers[marker] = new OpenLayers.Feature.Vector(new OpenLayers.Geometry.Point(0,0),{},
                                                       new OpenLayers.Style({},{externalGraphic: 'icons/marker-' + marker + '-red.png',
                                                                                fillColor: "white",
                                                                                graphicYOffset: -25,
                                                                                graphicWidth: 21,
                                                                                graphicHeight: 25,
                                                                                display: "none"}));

       layerVectors.addFeatures([markers[marker]]);
      }
    else
      {
       nmarkers=marker-1;
       vismarkers=marker-1;
       break;
      }
   }

 // A function to drag the markers

 var drag = new OpenLayers.Control.DragFeature(layerVectors,
                                               {onDrag:     dragMove,
                                                onComplete: dragComplete });
 map.addControl(drag);
 drag.activate();

 // Colours for the route highlights and popups

 route_colours={shortest: "#60C060", quickest: "#6060C0"};

 // Markers to highlight a selected point

 var highlight;

 for(highlight in highlights)
   {
    highlights[highlight] = new OpenLayers.Feature.Vector(new OpenLayers.Geometry.Point(0,0),{},
                                                          new OpenLayers.Style({},{strokeColor: route_colours[highlight],
                                                                                   fillColor: "white",
                                                                                   pointRadius: 10,
                                                                                   strokeWidth: 4,
                                                                                   fillOpacity: 0,
                                                                                   display: "none"}));

    layerVectors.addFeatures([highlights[highlight]]);
   }

 // A popup for routing results

 var popup;

 for(popup in popups)
   {
    popups[popup] = new OpenLayers.Popup(popup,
                                         new OpenLayers.LonLat(mapbounds.left,mapbounds.bottom),
                                         new OpenLayers.Size(map.getSize().w-100,60),
                                         popup,
                                         false);

    popups[popup].keepInMap=true;
    popups[popup].panMapIfOutOfView=false;
    popups[popup].setOpacity(0.75);
    popups[popup].setBackgroundColor(route_colours[popup]);
    popups[popup].setBorder("2px");
   }

 // Set the map centre to the limited range specified

 map.setCenter(mapbounds.getCenterLonLat(), map.getZoomForExtent(mapbounds,true));
 map.maxResolution = map.getResolution();

 // Move the map

 if(lon != 'lon' && lat != 'lat' && zoom != 'zoom')
   {
    var lonlat = new OpenLayers.LonLat(lon,lat).transform(epsg4326,map.getProjectionObject());

    map.moveTo(lonlat,zoom-map.minZoomLevel);
   }
}


//
// Map has moved
//

function mapMoved()
{
 var centre = map.getCenter().clone();

 var lonlat = centre.transform(map.getProjectionObject(),epsg4326);

 var zoom = this.getZoom() + map.minZoomLevel;

 map_args="lat=" + lonlat.lat + ";lon=" + lonlat.lon + ";zoom=" + zoom;

 updateCustomURL();
}


//
// OpenLayers.Control.DragFeature callback for a drag occuring.
//

function dragMove(feature,pixel)
{
 for(var marker in markers)
    if(feature==markers[marker])
      {
       markersmoved=true;

       coordsSetForm(marker);
      }
}


//
// OpenLayers.Control.DragFeature callback for completing a drag.
//

function dragComplete(feature,pixel)
{
 for(var marker in markers)
    if(feature==markers[marker])
      {
       markersmoved=true;

       coordsSetForm(marker);
      }
}


//
// Toggle a marker on the map.
//

function markerToggleMap(marker)
{
 var feature=markers[marker];

 if(feature.style.display == "")
    markerRemoveMap(marker);
 else
    markerAddMap(marker);
}


//
// Show a marker on the map.
//

function markerAddMap(marker)
{
 var feature=markers[marker];

 feature.style.display = "";
 document.images["waypoint" + marker].src="icons/marker-" + marker + "-red.png";

 formSetCoords(marker);

 layerVectors.drawFeature(feature);

 markersmoved=true;
}


//
// Remove a marker from the map.
//

function markerRemoveMap(marker)
{
 var feature=markers[marker];

 feature.style.display = "none";
 document.images["waypoint" + marker].src="icons/marker-" + marker + "-grey.png";

 layerVectors.drawFeature(feature);

 markersmoved=true;
}


//
// Clear the current marker.
//

function markerRemove(marker)
{
 for(var marker2=marker;marker2<vismarkers;marker2++)
   {
    document.forms["form"].elements["lon" + marker2].value=document.forms["form"].elements["lon" + (marker2+1)].value;
    document.forms["form"].elements["lat" + marker2].value=document.forms["form"].elements["lat" + (marker2+1)].value;

    if(markers[marker2+1].style.display=="")
       markerAddMap(marker2);
    else
       markerRemoveMap(marker2);
   }

 markerRemoveMap(vismarkers);

 var marker_tr=document.getElementById("point" + vismarkers);

 marker_tr.style.display="none";

 vismarkers--;

 if(vismarkers==1)
    markerAddAfter(1);

 updateCustomURL();
}


//
// Add a marker before the current one.
//

function markerAddBefore(marker)
{
 if(vismarkers==nmarkers || marker==1)
    return false;

 vismarkers++;

 var marker_tr=document.getElementById("point" + vismarkers);

 marker_tr.style.display="";

 for(var marker2=vismarkers;marker2>marker;marker2--)
   {
    document.forms["form"].elements["lon" + marker2].value=document.forms["form"].elements["lon" + (marker2-1)].value;
    document.forms["form"].elements["lat" + marker2].value=document.forms["form"].elements["lat" + (marker2-1)].value;

    if(markers[marker2-1].style.display=="")
       markerAddMap(marker2);
    else
       markerRemoveMap(marker2);
   }

 document.forms["form"].elements["lon" + marker].value="";
 document.forms["form"].elements["lat" + marker].value="";
 markers[marker].style.display="none";

 markerRemoveMap(marker);

 updateCustomURL();
}


//
// Add a marker after the current one.
//

function markerAddAfter(marker)
{
 if(vismarkers==nmarkers)
    return false;

 vismarkers++;

 var marker_tr=document.getElementById("point" + vismarkers);

 marker_tr.style.display="";

 for(var marker2=vismarkers;marker2>(marker+1);marker2--)
   {
    document.forms["form"].elements["lon" + marker2].value=document.forms["form"].elements["lon" + (marker2-1)].value;
    document.forms["form"].elements["lat" + marker2].value=document.forms["form"].elements["lat" + (marker2-1)].value;

    if(markers[marker2-1].style.display=="")
       markerAddMap(marker2);
    else
       markerRemoveMap(marker2);
   }

 document.forms["form"].elements["lon" + (marker+1)].value="";
 document.forms["form"].elements["lat" + (marker+1)].value="";
 markers[marker+1].style.display="none";

 markerRemoveMap(marker+1);

 updateCustomURL();
}


//
// Move this marker up.
//

function markerMoveUp(marker)
{
 if(marker==1)
    return false;

 markerSwap(marker,marker-1);
}


//
// Move this marker down.
//

function markerMoveDown(marker)
{
 if(marker==vismarkers)
    return false;

 markerSwap(marker,marker+1);
}


//
// Swap a pair of markers.
//

function markerSwap(marker1,marker2)
{
 var lon=document.forms["form"].elements["lon" + marker1].value;
 var lat=document.forms["form"].elements["lat" + marker1].value;
 var display=markers[marker1].style.display;

 document.forms["form"].elements["lon" + marker1].value=document.forms["form"].elements["lon" + marker2].value;
 document.forms["form"].elements["lat" + marker1].value=document.forms["form"].elements["lat" + marker2].value;
 if(markers[marker2].style.display=="")
    markerAddMap(marker1);
 else
    markerRemoveMap(marker1);

 document.forms["form"].elements["lon" + marker2].value=lon;
 document.forms["form"].elements["lat" + marker2].value=lat;
 if(display=="")
    markerAddMap(marker2);
 else
    markerRemoveMap(marker2);

 updateCustomURL();
}


//
// Reverse the markers.
//

function markersReverse()
{
 for(var marker=1;marker<=vismarkers/2;marker++)
    markerSwap(marker,vismarkers+1-marker);

 updateCustomURL();
}


//
// Set the feature coordinates in the form.
//

function coordsSetForm(marker)
{
 var feature=markers[marker];

 var lonlat = new OpenLayers.LonLat(feature.geometry.x, feature.geometry.y);
 lonlat.transform(map.getProjectionObject(),epsg4326);

 document.forms["form"].elements["lon" + marker].value=format5f(lonlat.lon);
 document.forms["form"].elements["lat" + marker].value=format5f(lonlat.lat);

 updateCustomURL();
}


//
// Set the feature coordinates from the form when the form changes.
//

function formSetCoords(marker)
{
 var feature=markers[marker];

 var lonlat=map.getCenter().clone();

 lonlat.transform(map.getProjectionObject(),epsg4326);

 var lon=document.forms["form"].elements["lon" + marker].value;
 var lat=document.forms["form"].elements["lat" + marker].value;

 if(lon!="")
   {
    if(lon<-180) lon=-180;
    if(lon>+180) lon=+180;
    lonlat.lon=lon;
   }

 if(lat!="")
   {
    if(lat<-90 ) lat=-90 ;
    if(lat>+90 ) lat=+90 ;
    lonlat.lat=lat;
   }

 var point = lonlat.clone();

 point.transform(epsg4326,map.getProjectionObject());

 feature.move(point);

 markersmoved=true;

 coordsSetForm(marker);
}


//
// Format a number in printf("%.5f") format.
//

function format5f(number)
{
 var newnumber=Math.floor(number*100000+0.5);
 var delta=0;

 if(newnumber>=0 && newnumber<100000) delta= 100000;
 if(newnumber<0 && newnumber>-100000) delta=-100000;

 var string=String(newnumber+delta);

 var intpart =string.substring(0,string.length-5);
 var fracpart=string.substring(string.length-5,string.length);

 if(delta>0) intpart="0";
 if(delta<0) intpart="-0";

 return(intpart + "." + fracpart);
}


//
// Zoom to a specific item in the route
//

function zoomTo(type,line)
{
 var lonlat = new OpenLayers.LonLat(routepoints[type][line].lon,routepoints[type][line].lat).transform(epsg4326,map.getProjectionObject());

 map.moveTo(lonlat,map.numZoomLevels-2);
}


//
// Highlight a specific item in the route
//

function highlight(type,line)
{
 if(line==-1)
   {
    highlights[type].style.display = "none";

    map.removePopup(popups[type]);
   }
 else
   {
    var lonlat = new OpenLayers.LonLat(routepoints[type][line].lon,routepoints[type][line].lat).transform(epsg4326,map.getProjectionObject());

    var popupstr="";

    if(routepoints[type][line].text=="")
       popupstr += "From: ".bold() + "Start Point".italics();
    else
       popupstr += "After: ".bold() + routepoints[type][line].text + " [ " + routepoints[type][line].segdist + " , " + routepoints[type][line].segdura + " ] ";

    popupstr += "<br>";

    if(routepoints[type][line].bearing==undefined)
       popupstr += "Stop: ".bold();
    else if(routepoints[type][line].turn==undefined)
       popupstr += "Go:&nbsp;&nbsp;&nbsp; ".bold() + routepoints[type][line].bearing;
    else
       popupstr += "Go:&nbsp;&nbsp;&nbsp; ".bold() + routepoints[type][line].turn + " (" + routepoints[type][line].bearing + ")";

    if(routepoints[type][line].type=="Waypt")
       popupstr += " [Waypoint]";

    popupstr += "<br>";

    if(routepoints[type][line+1]==undefined)
       popupstr += "At:&nbsp;&nbsp;&nbsp; ".bold() + "Finish Point".italics();
    else
       popupstr += "Onto: ".bold() + routepoints[type][line+1].text;

    popupstr += "<br>";

    popupstr += "Total: ".bold() + routepoints[type][line].totdist + " , " + routepoints[type][line].totdura;

    popups[type].setContentHTML(popupstr);

    highlights[type].move(lonlat);

    if(highlights[type].style.display = "none")
      {
       highlights[type].style.display = "";

       map.addPopup(popups[type]);

       var lonlat=new OpenLayers.LonLat(map.getCenter().lon,map.getExtent().top);
       var position=map.getPixelFromLonLat(lonlat);

       position.x=position.x-popups[type].size.w/2;
       position.y=position.y+50;

       popups[type].lonlat=map.getLonLatFromPixel(position);
       popups[type].updatePosition();
      }
   }

 layerVectors.drawFeature(highlights[type]);
}


//
// Build a set of URL arguments
//

function buildURLArguments(all)
{
 var url="?";

 url=url + "transport=" + router_transport;

 for(var marker=1;marker<=vismarkers;marker++)
    if(markers[marker].style.display == "" || all)
      {
       url=url + ";lon" + marker + "=" + document.forms["form"].elements["lon" + marker].value;
       url=url + ";lat" + marker + "=" + document.forms["form"].elements["lat" + marker].value;
      }

 for(key in router_profile_highway)
    url=url + ";highway-" + key + "=" + document.forms["form"].elements["highway-" + key].value;

 for(key in router_profile_speed)
    url=url + ";speed-" + key + "=" + document.forms["form"].elements["speed-" + key].value;

 for(key in router_profile_property)
    url=url + ";property-" + key + "=" + document.forms["form"].elements["property-" + key].value;

 for(key in router_restrictions)
   {
    if(key=="oneway")
       url=url + ";" + key + "=" + document.forms["form"].elements["restrict-" + key].checked;
    else
       url=url + ";" + key + "=" + document.forms["form"].elements["restrict-" + key].value;
   }

 return(url);
}


//
// Display data statistics
//

function displayStatistics()
{
 // Use AJAX to get the statistics

 OpenLayers.loadURL("statistics.cgi",null,null,runStatisticsSuccess);
}


//
// Success in running router.
//

function runStatisticsSuccess(response)
{
 var statistics_data=document.getElementById("statistics_data");
 var statistics_link=document.getElementById("statistics_link");

 statistics_data.innerHTML="<pre>" + response.responseText + "</pre>";

 statistics_link.style.display="none";
}


//
// Submit form - performing the routing
//

function findRoute(type)
{
 tab_select("results");

 hideshow_hide('help_options');
 hideshow_hide('shortest');
 hideshow_hide('quickest');

 var div_status=document.getElementById("result_status");
 div_status.innerHTML = "Running...";

 var url="router.cgi" + buildURLArguments(0) + ";type=" + type;

 // Destroy the existing layer(s)

 if(markersmoved || paramschanged)
   {
    if(layerGPX.shortest!=null)
       removeGPXTrace("shortest");
    if(layerGPX.quickest!=null)
       removeGPXTrace("quickest");
    markersmoved=false;
    paramschanged=false;
   }
 else if(layerGPX[type]!=null)
    removeGPXTrace(type);

 // Use AJAX to run the router

 routing_type=type;

 OpenLayers.loadURL(url,null,null,runRouterSuccess,runRouterFailure);
}


//
// Remove a GPX trace
//

function removeGPXTrace(type)
{
 map.removeLayer(layerGPX[type]);
 layerGPX[type].destroy();
 layerGPX[type]=null;

 var span_status=document.getElementById(type + "_status");
 span_status.innerHTML = "No Information".italics();

 var div_data=document.getElementById(type + "_data");
 div_data.innerHTML = "";

 var div_text=document.getElementById(type + "_text");
 div_text.innerHTML = "";

 hideshow_hide(type);
}


//
// Success in running router.
//

function runRouterSuccess(response)
{
 var lines=response.responseText.split('\n');

 var uuid=lines[0];
 var cpuinfo=lines[1];
 var distinfo=lines[2];
 var message=lines[3];

 // Update the status message

 var div_status=document.getElementById("result_status");

 if(message!="")
   {
    div_status.innerHTML = message.bold() + "<br>" + cpuinfo.small();
    hideshow_show('help_route');
    return;
   }
 else
   {
    div_status.innerHTML = "Routing Completed".bold() + "<br>" + cpuinfo.small();
    hideshow_hide('help_route');
   }

 // Update the routing result message

 var span_status=document.getElementById(routing_type + "_status");
 span_status.innerHTML = distinfo.bold();

 var div_data=document.getElementById(routing_type + "_data");

 var result="<table>";
 url="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=gpx-track";
 result=result + "<tr><td>GPX track file:<td onclick='window.open(\"" + url + "\")'><span>Open Popup</span><td>";
 url="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=gpx-route";
 result=result + "<tr><td>GPX route file:<td onclick='window.open(\"" + url + "\")'><span>Open Popup</span><td>";
 url="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=txt-all";
 result=result + "<tr><td>Full text file:<td onclick='window.open(\"" + url + "\")'><span>Open Popup</span><td>";
 url="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=txt";
 result=result + "<tr><td>Text file:<td onclick='window.open(\"" + url + "\")'><span>Open Popup</span>";
 result=result + "</table>";

 div_data.innerHTML=result;

 // Add a GPX layer

 var url="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=gpx-track";

 layerGPX[routing_type] = new OpenLayers.Layer.GML("GPX (" + routing_type + ")", url,
                                                   {
                                                    format:     OpenLayers.Format.GPX,
                                                    style:      gpx_style[routing_type],
                                                    projection: map.displayProjection
                                                   });

 map.addLayer(layerGPX[routing_type]);

 hideshow_show(routing_type);

 displayResult(routing_type,uuid);
}


//
// Failure in running router.
//

function runRouterFailure(response)
{
 var div_status=document.getElementById("result_status");
 div_status.innerHTML = String("Failed to run router!").bold();
}


//
// Display the route
//

function displayResult(type,uuid)
{
 routing_type = type;

 // Add the route

 var url="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=txt";

 // Use AJAX to get the route

 OpenLayers.loadURL(url,null,null,getRouteSuccess,getRouteFailure);
}


//
// Success in getting route.
//

var turn_instruct={"-4": "Very sharp left", "-3": "Sharp left", "-2": "Left", "-1": "Slight left",
                   "0": "Straight on",
                   "4": "Very sharp right", "3": "Sharp right", "2": "Right", "1": "Slight right"};

var bearing_instruct={"-4": "South", "-3": "South-West", "-2": "West", "-1": "North-West",
                      "0": "North",
                      "4": "South", "3": "South-East", "2": "East", "1": "North-East"};

function getRouteSuccess(response)
{
 var lines=response.responseText.split('\n');
 var div_text=document.getElementById(routing_type + "_text");

 routepoints[routing_type]={};

 var result="<hr><table onmouseout='highlight(\"" + routing_type + "\",-1)'>";

 var lat,lon,segdist,segdura,totdist,totdura,turn,text;

 for(line in lines)
   {
    var words=lines[line].split('\t');

//    fprintf(textfile,"#Latitude\tLongitude\tSection \tSection \tTotal   \tTotal   \tPoint\tTurn\tBearing\tHighway\n");
//    fprintf(textfile,"#        \t         \tDistance\tDuration\tDistance\tDuration\tType \t    \t       \t       \n");

    if(lines[line].match(/^#/))
       ;
    else if(words[1]==undefined)
       ;
    else
      {
       routepoints[routing_type][line]={lat: Number(words[0]), lon: Number(words[1]),
                                        segdist: words[2], segdura: words[3],
                                        totdist: words[4], totdura: words[5],
                                        type: words[6], text: words[9]};

       if(words[7] != "")
          routepoints[routing_type][line].turn=turn_instruct[Number(words[7])];

       if(words[8] != "")
          routepoints[routing_type][line].bearing=bearing_instruct[Number(words[8])];

       if(words[9]=="")
          words[9]="Start Point".italics();

       result=result + "<tr onclick='zoomTo(\"" + routing_type + "\"," + line + ")'" +
                          " onmouseover='highlight(\"" + routing_type + "\"," + line + ")'>" +
                          "<td class='right'>" + words[4] +
                          "<td class='right'>" + words[5] +
                          "<td class='left'>" + "&nbsp;" + words[9];
      }
   }

 result=result + "</table>";

 div_text.innerHTML=result;
}


//
// Failure in getting route.
//

function getRouteFailure(response)
{
 var div_text=document.getElementById(routing_type + "_text");
 div_text.innerHTML = "Failed to get route information!".bold();
}
