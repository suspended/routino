//
// Routino router web page Javascript
//
// Part of the Routino routing software.
//
// This file Copyright 2008-2012 Andrew M. Bishop
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

// The number of waypoints to include in the HTML
var maxmarkers=9;

var vismarkers, markers, markersmoved, paramschanged;
var homelat=null, homelon=null;


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Initialisation /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Make a deep copy of the routino profile.

var routino_default={};
for(var l1 in routino)
   if(typeof(routino[l1])!='object')
      routino_default[l1]=routino[l1];
   else
     {
      routino_default[l1]={};
      for(var l2 in routino[l1])
         if(typeof(routino[l1][l2])!='object')
            routino_default[l1][l2]=Number(routino[l1][l2]);
         else
           {
            routino_default[l1][l2]={};
            for(var l3 in routino[l1][l2])
               routino_default[l1][l2][l3]=Number(routino[l1][l2][l3]);
           }
     }

// Store the latitude and longitude in the routino variable

routino.point=[];
for(var marker=1;marker<=maxmarkers;marker++)
  {
   routino.point[marker]={};

   routino.point[marker].lon="";
   routino.point[marker].lat="";
   routino.point[marker].search="";
   routino.point[marker].active=false;
  }

// Process the URL query string and extract the arguments

var legal={"^lon"             : "^[-0-9.]+$",
           "^lat"             : "^[-0-9.]+$",
           "^zoom"            : "^[0-9]+$",

           "^lon[1-9]"        : "^[-0-9.]+$",
           "^lat[1-9]"        : "^[-0-9.]+$",
           "^search[1-9]"     : "^.+$",
           "^transport"       : "^[a-z]+$",
           "^highway-[a-z]+"  : "^[0-9.]+$",
           "^speed-[a-z]+"    : "^[0-9.]+$",
           "^property-[a-z]+" : "^[0-9.]+$",
           "^oneway"          : "^(1|0|true|false|on|off)$",
           "^turns"           : "^(1|0|true|false|on|off)$",
           "^weight"          : "^[0-9.]+$",
           "^height"          : "^[0-9.]+$",
           "^width"           : "^[0-9.]+$",
           "^length"          : "^[0-9.]+$",

           "^language"        : "^[-a-zA-Z]+$"};

var args={};

if(location.search.length>1)
  {
   var query,queries;

   query=location.search.replace(/^\?/,"");
   query=query.replace(/;/g,'&');
   queries=query.split('&');

   for(var i=0;i<queries.length;i++)
     {
      queries[i].match(/^([^=]+)(=(.*))?$/);

      k=RegExp.$1;
      v=unescape(RegExp.$3);

      for(var l in legal)
        {
         if(k.match(RegExp(l)) && v.match(RegExp(legal[l])))
            args[k]=v;
        }
     }
  }


//
// Fill in the HTML - add the missing waypoints
//

function html_init()
{
 var waypoints=document.getElementById("waypoints");

 var waypoint_html=waypoints.rows[0].innerHTML;
 waypoints.deleteRow(0);

 var searchresults_html=waypoints.rows[0].innerHTML;
 waypoints.deleteRow(0);

 for(var marker=maxmarkers;marker>=1;marker--)
   {
    var searchresults=waypoints.insertRow(0);
    searchresults.id="searchresults" + marker;

    var this_searchresults_html=searchresults_html.split('XXX').join(marker);
    searchresults.innerHTML=this_searchresults_html;

    var waypoint=waypoints.insertRow(0);
    waypoint.id="waypoint" + marker;

    var this_waypoint_html=waypoint_html.split('XXX').join(marker);
    waypoint.innerHTML=this_waypoint_html;
   }

 vismarkers=maxmarkers;
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Form handling /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//
// Form initialisation - fill in the uninitialised parts
//

function form_init()
{
 // Fill in the waypoints

 var filled=0;

 for(var marker=maxmarkers;marker>=1;marker--)
   {
    var lon=args["lon" + marker];
    var lat=args["lat" + marker];
    var search=args["search" + marker];

    if(lon != undefined && lat != undefined && search != undefined && lon != "" && lat != "" && search != "")
      {
       formSetSearch(marker,search);
       formSetCoords(marker,lon,lat,true);

       markerSearch(marker);

       filled++;
      }
    else if(lon != undefined && lat != undefined && lon != "" && lat != "")
      {
       formSetCoords(marker,lon,lat,true);

       markerCoords(marker);

       filled++;
      }
    else if(search != undefined && search != "")
      {
       formSetSearch(marker,search);

       markerSearch(marker);

       DoSearch(marker);

       filled++;
      }
    else if(filled==0)
       markerRemove(marker);
   }

 // Update the transport type with the URL settings which updates all HTML forms to defaults.

 var transport=routino.transport;

 if(args["transport"] != undefined)
    transport=args["transport"];

 formSetTransport(transport);

 // Update the HTML with the URL settings

 if(args["language"] != undefined)
    formSetLanguage(args["language"]);

 for(var key in routino.profile_highway)
    if(args["highway-" + key] != undefined)
       formSetHighway(key,args["highway-" + key]);

 for(var key in routino.profile_speed)
    if(args["speed-" + key] != undefined)
       formSetSpeed(key,args["speed-" + key]);

 for(var key in routino.profile_property)
    if(args["property-" + key] != undefined)
       formSetProperty(key,args["property-" + key]);

 for(var key in routino.restrictions)
   {
    if(key=="oneway" || key=="turns")
      {
       if(args[key] != undefined)
          formSetRestriction(key,args[key]);
      }
    else
      {
       if(args["restrict-" + key] != undefined)
          formSetRestriction(key,args["restrict-" + key]);
      }
   }

 // Get the home location cookie and compare to each waypoint

 var cookies=document.cookie.split('; ');

 for(var cookie=0;cookie<cookies.length;cookie++)
    if(cookies[cookie].substr(0,"Routino-home".length)=="Routino-home")
      {
       var data=cookies[cookie].split(/[=:;]/);

       if(data[1]=="lon") homelon=Number(data[2]);
       if(data[3]=="lat") homelat=Number(data[4]);
      }

 if(homelon!=null && homelat!=null)
   {
    for(var marker=maxmarkers;marker>=1;marker--)
      {
       var lon=routino.point[marker].lon;
       var lat=routino.point[marker].lat;

       if(lon==homelon && lat==homelat)
          updateIcon(marker);
      }

    // If the first location is empty and the cookie is set then fill it.

    if(routino.point[1].lon=="" && routino.point[1].lat=="")
       formSetCoords(1,homelon,homelat,true);
   }
}


//
// Change of language in the form
//

function formSetLanguage(value)
{
 if(value == undefined)
   {
    for(var lang=0;lang<document.forms["form"].elements["language"].length;lang++)
       if(document.forms["form"].elements["language"][lang].checked)
          routino.language=document.forms["form"].elements["language"][lang].value;
   }
 else
   {
    for(var lang=0;lang<document.forms["form"].elements["language"].length;lang++)
       if(document.forms["form"].elements["language"][lang].value==value)
          document.forms["form"].elements["language"][lang].checked=true;
       else
          document.forms["form"].elements["language"][lang].checked=false;

    routino.language=value;
   }
}


//
// Change of transport in the form
//

function formSetTransport(value)
{
 routino.transport=value;

 for(var key in routino.transports)
    document.forms["form"].elements["transport"][routino.transports[key]-1].checked=(key==routino.transport);

 for(var key in routino.profile_highway)
    document.forms["form"].elements["highway-" + key].value=routino.profile_highway[key][routino.transport];

 for(var key in routino.profile_speed)
    document.forms["form"].elements["speed-" + key].value=routino.profile_speed[key][routino.transport];

 for(var key in routino.profile_property)
    document.forms["form"].elements["property-" + key].value=routino.profile_property[key][routino.transport];

 for(var key in routino.restrictions)
   {
    if(key=="oneway" || key=="turns")
       document.forms["form"].elements["restrict-" + key].checked=routino.profile_restrictions[key][routino.transport];
    else
       document.forms["form"].elements["restrict-" + key].value=routino.profile_restrictions[key][routino.transport];
   }

 paramschanged=true;
}


//
// Change of highway in the form
//

function formSetHighway(type,value)
{
 if(value == undefined)
    routino.profile_highway[type][routino.transport]=document.forms["form"].elements["highway-" + type].value;
 else
   {
    document.forms["form"].elements["highway-" + type].value=value;
    routino.profile_highway[type][routino.transport]=value;
   }

 paramschanged=true;
}


//
// Change of Speed in the form
//

function formSetSpeed(type,value)
{
 if(value == undefined)
    routino.profile_speed[type][routino.transport]=document.forms["form"].elements["speed-" + type].value;
 else
   {
    document.forms["form"].elements["speed-" + type].value=value;
    routino.profile_speed[type][routino.transport]=value;
   }

 paramschanged=true;
}


//
// Change of Property in the form
//

function formSetProperty(type,value)
{
 if(value == undefined)
    routino.profile_property[type][routino.transport]=document.forms["form"].elements["property-" + type].value;
 else
   {
    document.forms["form"].elements["property-" + type].value=value;
    routino.profile_property[type][routino.transport]=value;
   }

 paramschanged=true;
}


//
// Change of Restriction rule in the form
//

function formSetRestriction(type,value)
{
 if(value == undefined)
   {
    if(type=="oneway" || type=="turns")
       routino.profile_restrictions[type][routino.transport]=document.forms["form"].elements["restrict-" + type].checked;
    else
       routino.profile_restrictions[type][routino.transport]=document.forms["form"].elements["restrict-" + type].value;
   }
 else
   {
    if(type=="oneway" || type=="turns")
       document.forms["form"].elements["restrict-" + type].checked=value;
    else
       document.forms["form"].elements["restrict-" + type].value=value;

    routino.profile_restrictions[type][routino.transport]=value;
   }

 paramschanged=true;
}


//
// Set the feature coordinates from the form when the form changes.
//

function formSetCoords(marker,lon,lat,active)
{
 clearSearchResult(marker);

 if(lon == undefined || lat == undefined)
   {
    routino.point[marker].lon=document.forms["form"].elements["lon" + marker].value;
    routino.point[marker].lat=document.forms["form"].elements["lat" + marker].value;

    if(routino.point[marker].lon=="" || routino.point[marker].lat=="")
       markerCentre(marker);
   }
 else
   {
    document.forms["form"].elements["lon" + marker].value=format5f(lon);
    document.forms["form"].elements["lat" + marker].value=format5f(lat);

    routino.point[marker].lon=lon;
    routino.point[marker].lat=lat;

    if(active != undefined)
      {
       if(active)
          markerAddMap(marker);
       else
          markerRemoveMap(marker);
      }
   }

 var lonlat=map.getCenter().clone();

 lonlat.transform(map.getProjectionObject(),epsg4326);

 if(routino.point[marker].lon!="")
   {
    if(routino.point[marker].lon<-180) routino.point[marker].lon=-180;
    if(routino.point[marker].lon>+180) routino.point[marker].lon=+180;
    lonlat.lon=routino.point[marker].lon;
   }

 if(routino.point[marker].lat!="")
   {
    if(routino.point[marker].lat<-90 ) routino.point[marker].lat=-90 ;
    if(routino.point[marker].lat>+90 ) routino.point[marker].lat=+90 ;
    lonlat.lat=routino.point[marker].lat;
   }

 var point = lonlat.clone();

 point.transform(epsg4326,map.getProjectionObject());

 markers[marker].move(point);

 markersmoved=true;
}


//
// Set the feature coordinates from the form when the form changes.
//

function formSetSearch(marker,search)
{
 clearSearchResult(marker);

 if(search == undefined)
   {
    routino.point[marker].search=document.forms["form"].elements["search" + marker].value;

    DoSearch(marker);
   }
 else
   {
    document.forms["form"].elements["search" + marker].value=search;

    routino.point[marker].search=search;
   }
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
// Build a set of URL arguments
//

function buildURLArguments(lang)
{
 var url= "transport=" + routino.transport;

 for(var marker=1;marker<=vismarkers;marker++)
    if(routino.point[marker].active)
      {
       url=url + ";lon" + marker + "=" + routino.point[marker].lon;
       url=url + ";lat" + marker + "=" + routino.point[marker].lat;
       if(routino.point[marker].search != "")
          url=url + ";search" + marker + "=" + encodeURIComponent(routino.point[marker].search);
      }

 for(var key in routino.profile_highway)
    if(routino.profile_highway[key][routino.transport]!=routino_default.profile_highway[key][routino.transport])
       url=url + ";highway-" + key + "=" + routino.profile_highway[key][routino.transport];

 for(var key in routino.profile_speed)
    if(routino.profile_speed[key][routino.transport]!=routino_default.profile_speed[key][routino.transport])
       url=url + ";speed-" + key + "=" + routino.profile_speed[key][routino.transport];

 for(var key in routino.profile_property)
    if(routino.profile_property[key][routino.transport]!=routino_default.profile_property[key][routino.transport])
       url=url + ";property-" + key + "=" + routino.profile_property[key][routino.transport];

 for(var key in routino.restrictions)
    if(routino.profile_restrictions[key][routino.transport]!=routino_default.profile_restrictions[key][routino.transport])
       url=url + ";" + key + "=" + routino.profile_restrictions[key][routino.transport];

 if(lang && routino.language)
    url=url + ";language=" + routino.language;

 return(url);
}


//
// Build a set of URL arguments for the map location
//

function buildMapArguments()
{
 var centre = map.getCenter().clone();

 var lonlat = centre.transform(map.getProjectionObject(),epsg4326);

 var zoom = map.getZoom() + map.minZoomLevel;

 return "lat=" + format5f(lonlat.lat) + ";lon=" + format5f(lonlat.lon) + ";zoom=" + zoom;
}


//
// Update a URL
//

function updateURL(element)
{
 if(element.id == "permalink_url")
    element.href=location.pathname + "?" + buildURLArguments(true) + ";" + buildMapArguments();

 if(element.id == "visualiser_url")
    element.href="visualiser.html" + "?" + buildMapArguments();

 if(element.id == "edit_url")
    element.href="http://www.openstreetmap.org/edit" + "?" + buildMapArguments();

 if(element.id.match(/^lang_([a-zA-Z-]+)_url$/))
    element.href="router.html" + "." + RegExp.$1 + "?" + buildURLArguments(false) + ";" + buildMapArguments();
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


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Map handling /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

var map;
var layerMap=[], layerVectors, layerGPX;
var epsg4326, epsg900913;

//
// Initialise the 'map' object
//

function map_init()
{
 lon =args["lon"];
 lat =args["lat"];
 zoom=args["zoom"];

 // Map properties (North/South and East/West limits and zoom in/out limits) are now in mapprops.js
 // Map URLs are now in mapprops.js

 //
 // Create the map
 //

 epsg4326=new OpenLayers.Projection("EPSG:4326");
 epsg900913=new OpenLayers.Projection("EPSG:900913");

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

                            minZoomLevel: mapprops.zoomout,
                            numZoomLevels: mapprops.zoomin-mapprops.zoomout+1,
                            maxResolution: 156543.0339 / Math.pow(2,mapprops.zoomout),

                            maxExtent:        new OpenLayers.Bounds(-20037508.34, -20037508.34, 20037508.34, 20037508.34),
                            restrictedExtent: new OpenLayers.Bounds(mapprops.westedge,mapprops.southedge,mapprops.eastedge,mapprops.northedge).transform(epsg4326,epsg900913),

                            units: "m"
                           });

 // Add map tile layers

 for(var l=0;l < mapprops.mapdata.length;l++)
   {
    layerMap[l] = new OpenLayers.Layer.TMS(mapprops.mapdata[l].label,
                                           mapprops.mapdata[l].baseurl,
                                           {
                                            emptyUrl: mapprops.mapdata[l].errorurl,
                                            type: 'png',
                                            getURL: limitedUrl,
                                            displayOutsideMaxExtent: true,
                                            buffer: 1
                                           });
    map.addLayer(layerMap[l]);
   }

 // Get a URL for the tile; limited to map restricted extent.

 function limitedUrl(bounds)
 {
  var z = map.getZoom() + map.minZoomLevel;

  if (z>=7 && (bounds.right  < map.restrictedExtent.left ||
               bounds.left   > map.restrictedExtent.right ||
               bounds.top    < map.restrictedExtent.bottom ||
               bounds.bottom > map.restrictedExtent.top))
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

 markers={};
 markersmoved=false;
 paramschanged=false;

 for(var marker=1;marker<=maxmarkers;marker++)
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

 // A function to drag the markers

 var drag = new OpenLayers.Control.DragFeature(layerVectors,
                                               {onDrag:     dragMove,
                                                onComplete: dragComplete });
 map.addControl(drag);
 drag.activate();

 // Markers to highlight a selected point

 for(var highlight in highlights)
   {
    highlights[highlight] = new OpenLayers.Feature.Vector(new OpenLayers.Geometry.Point(0,0),{},
                                                          new OpenLayers.Style({},{strokeColor: route_dark_colours[highlight],
                                                                                   fillColor: "white",
                                                                                   pointRadius: 10,
                                                                                   strokeWidth: 4,
                                                                                   fillOpacity: 0,
                                                                                   display: "none"}));

    layerVectors.addFeatures([highlights[highlight]]);
   }

 // A popup for routing results

 for(var popup in popups)
    popups[popup] = createPopup(popup);

 // Set the map centre to the limited range specified

 map.setCenter(map.restrictedExtent.getCenterLonLat(), map.getZoomForExtent(map.restrictedExtent,true));
 map.maxResolution = map.getResolution();

 // Move the map

 if(lon != undefined && lat != undefined && zoom != undefined)
   {
    if(lon<mapprops.westedge) lon=mapprops.westedge;
    if(lon>mapprops.eastedge) lon=mapprops.eastedge;

    if(lat<mapprops.southedge) lat=mapprops.southedge;
    if(lat>mapprops.northedge) lat=mapprops.northedge;

    if(zoom<mapprops.zoomout) zoom=mapprops.zoomout;
    if(zoom>mapprops.zoomin)  zoom=mapprops.zoomin;

    var lonlat = new OpenLayers.LonLat(lon,lat).transform(epsg4326,map.getProjectionObject());

    map.moveTo(lonlat,zoom-map.minZoomLevel);
   }
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

       dragSetForm(marker);
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

       dragSetForm(marker);
      }
}


//
// Set the feature coordinates in the form after dragging.
//

function dragSetForm(marker)
{
 var lonlat = new OpenLayers.LonLat(markers[marker].geometry.x, markers[marker].geometry.y);
 lonlat.transform(map.getProjectionObject(),epsg4326);

 var lon=format5f(lonlat.lon);
 var lat=format5f(lonlat.lat);

 formSetCoords(marker,lon,lat);
}


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Marker handling ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//
// Toggle a marker on the map.
//

function markerToggleMap(marker)
{
 clearSearchResult(marker);

 if(routino.point[marker].active)
    markerRemoveMap(marker);
 else
    markerAddMap(marker);
}


//
// Show a marker on the map.
//

function markerAddMap(marker)
{
 clearSearchResult(marker);

 markers[marker].style.display = "";
 routino.point[marker].active=true;

 updateIcon(marker);

 markersmoved=true;
}


//
// Remove a marker from the map.
//

function markerRemoveMap(marker)
{
 clearSearchResult(marker);

 markers[marker].style.display = "none";
 routino.point[marker].active=false;

 updateIcon(marker);

 markersmoved=true;
}


//
// Display search string for the marker
//

function markerSearch(marker)
{
 clearSearchResult(marker);

 var search_span=document.getElementById("search" + marker);
 var coords_span=document.getElementById("coords" + marker);

 search_span.style.display="";
 coords_span.style.display="none";
}


//
// Display coordinates for the marker
//

function markerCoords(marker)
{
 clearSearchResult(marker);

 var search_span=document.getElementById("search" + marker);
 var coords_span=document.getElementById("coords" + marker);

 search_span.style.display="none";
 coords_span.style.display="";
}


//
// Centre the marker on the map
//

function markerCentre(marker)
{
 clearSearchResult(marker);

 var lonlat=map.getCenter().clone();

 lonlat.transform(map.getProjectionObject(),epsg4326);

 formSetCoords(marker,lonlat.lon,lonlat.lat,true);
}


//
// Centre the map on the marker
//

function markerRecentre(marker)
{
 clearSearchResult(marker);

 lon=routino.point[marker].lon;
 lat=routino.point[marker].lat;

 var lonlat = new OpenLayers.LonLat(lon,lat).transform(epsg4326,map.getProjectionObject());

 map.panTo(lonlat);
}


//
// Clear the current marker.
//

function markerRemove(marker)
{
 clearSearchResult(marker);

 for(var marker2=marker;marker2<vismarkers;marker2++)
    formSetCoords(marker2,routino.point[marker2+1].lon,routino.point[marker2+1].lat,routino.point[marker2+1].active);

 markerRemoveMap(vismarkers);

 var marker_tr=document.getElementById("waypoint" + vismarkers);
 marker_tr.style.display="none";

 vismarkers--;

 if(vismarkers==1)
    markerAddAfter(1);
}


//
// Add a marker before the current one.
//

function markerAddBefore(marker)
{
 clearSearchResult(marker);

 if(vismarkers==maxmarkers || marker==1)
    return false;

 vismarkers++;

 var marker_tr=document.getElementById("waypoint" + vismarkers);
 marker_tr.style.display="";

 for(var marker2=vismarkers;marker2>marker;marker2--)
    formSetCoords(marker2,routino.point[marker2-1].lon,routino.point[marker2-1].lat,routino.point[marker2-1].active);

 formSetCoords(marker,"","",false);

 markerRemoveMap(marker);
}


//
// Add a marker after the current one.
//

function markerAddAfter(marker)
{
 clearSearchResult(marker);

 if(vismarkers==maxmarkers)
    return false;

 vismarkers++;

 var marker_tr=document.getElementById("waypoint" + vismarkers);
 marker_tr.style.display="";

 for(var marker2=vismarkers;marker2>(marker+1);marker2--)
    formSetCoords(marker2,routino.point[marker2-1].lon,routino.point[marker2-1].lat,routino.point[marker2-1].active);

 formSetCoords(marker+1,"","",false);

 markerRemoveMap(marker+1);
}


//
// Set this marker as the home location.
//

function markerHome(marker)
{
 clearSearchResult(marker);

 if(markerHomeCookie(marker))
    for(marker=1;marker<=maxmarkers;marker++)
       updateIcon(marker);
}


//
// Set this marker as the current location.
//

function markerLocate(marker)
{
 clearSearchResult(marker);

 if(navigator.geolocation)
    navigator.geolocation.getCurrentPosition(
                                             function(position) {
                                              formSetCoords(marker,position.coords.longitude,position.coords.latitude,true);
                                             });
}


//
// Update an icon to set colours and home or normal marker.
//

function updateIcon(marker)
{
 var lon=routino.point[marker].lon;
 var lat=routino.point[marker].lat;

 if(lon==homelon && lat==homelat)
   {
    if(routino.point[marker].active)
       document.images["waypoint" + marker].src="icons/marker-home-red.png";
    else
       document.images["waypoint" + marker].src="icons/marker-home-grey.png";

    markers[marker].style.externalGraphic="icons/marker-home-red.png";
   }
 else
   {
    if(routino.point[marker].active)
       document.images["waypoint" + marker].src="icons/marker-" + marker + "-red.png";
    else
       document.images["waypoint" + marker].src="icons/marker-" + marker + "-grey.png";

    markers[marker].style.externalGraphic="icons/marker-" + marker + "-red.png";
   }

 layerVectors.drawFeature(markers[marker]);
}


//
// Set or clear the home marker icon
//

function markerHomeCookie(marker)
{
 var lon=routino.point[marker].lon;
 var lat=routino.point[marker].lat;

 if(lon=="" || lat=="")
    return(false);

 var cookie;
 var date = new Date();

 if((homelat==null && homelon==null) ||
    (homelat!=lat  && homelon!=lon))
   {
    cookie="Routino-home=lon:" + lon + ":lat:" + lat;

    date.setUTCFullYear(date.getUTCFullYear()+5);

    homelat=lat;
    homelon=lon;
   }
 else
   {
    cookie="Routino-home=unset";

    date.setUTCFullYear(date.getUTCFullYear()-1);

    homelat=null;
    homelon=null;
   }

 document.cookie=cookie + ";expires=" + date.toGMTString();

 return(true);
}


//
// Move this marker up.
//

function markerMoveUp(marker)
{
 if(marker==1)
   {
    for(var m=1;m<vismarkers;m++)
       markerSwap(m,m+1);
   }
 else
    markerSwap(marker,marker-1);
}


//
// Move this marker down.
//

function markerMoveDown(marker)
{
 if(marker==vismarkers)
   {
    for(var m=vismarkers;m>1;m--)
       markerSwap(m,m-1);
   }
 else
    markerSwap(marker,marker+1);
}


//
// Swap a pair of markers.
//

function markerSwap(marker1,marker2)
{
 var lon=routino.point[marker1].lon;
 var lat=routino.point[marker1].lat;
 var active=routino.point[marker1].active;

 formSetCoords(marker1,routino.point[marker2].lon,routino.point[marker2].lat,routino.point[marker2].active);

 formSetCoords(marker2,lon,lat,active);
}


//
// Reverse the markers.
//

function markersReverse()
{
 for(var marker=1;marker<=vismarkers/2;marker++)
    markerSwap(marker,vismarkers+1-marker);
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////////// Route results handling ////////////////////////////
////////////////////////////////////////////////////////////////////////////////

var route_light_colours={shortest: "#60C060", quickest: "#6060C0"};
var route_dark_colours ={shortest: "#408040", quickest: "#404080"};

var highlights={shortest: null, quickest: null};
var popups={shortest: null, quickest: null};
var routepoints={shortest: {}, quickest: {}};
var gpx_style={shortest: null, quickest: null};

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

    drawPopup(popups[type],null);
   }
 else
   {
    // Marker

    var lonlat = new OpenLayers.LonLat(routepoints[type][line].lon,routepoints[type][line].lat).transform(epsg4326,map.getProjectionObject());

    highlights[type].move(lonlat);

    if(highlights[type].style.display = "none")
       highlights[type].style.display = "";

    // Popup

    drawPopup(popups[type],"<table>" + routepoints[type][line].html + "</table>");
   }

 layerVectors.drawFeature(highlights[type]);
}


//
// Create a popup - not using OpenLayers because want it fixed on screen not fixed on map.
//

function createPopup(type)
{
 var popup=document.createElement('div');

 popup.className = "popup";

 popup.innerHTML = "<span></span>";

 popup.style.display = "none";

 popup.style.position = "fixed";
 popup.style.top = "-4000px";
 popup.style.left = "-4000px";
 popup.style.zIndex = "100";

 popup.style.padding = "5px";

 popup.style.opacity=0.85;
 popup.style.backgroundColor=route_light_colours[type];
 popup.style.border="4px solid " + route_dark_colours[type];

 document.body.appendChild(popup);

 return(popup);
}


//
// Draw a popup - not using OpenLayers because want it fixed on screen not fixed on map.
//

function drawPopup(popup,html)
{
 if(html==null)
   {
    popup.style.display="none";
    return;
   }

 if(popup.style.display=="none")
   {
    var map_div=document.getElementById("map");

    popup.style.left  =map_div.offsetParent.offsetLeft+map_div.offsetLeft+60 + "px";
    popup.style.top   =                                map_div.offsetTop +30 + "px";
    popup.style.width =map_div.clientWidth-100 + "px";

    popup.style.display="";
   }

 popup.innerHTML=html;
}


//
// Remove a GPX trace
//

function removeGPXTrace(type)
{
 map.removeLayer(layerGPX[type]);
 layerGPX[type].destroy();
 layerGPX[type]=null;

 displayStatus(type,"no_info");

 var div_links=document.getElementById(type + "_links");
 div_links.style.display = "none";

 var div_route=document.getElementById(type + "_route");
 div_route.innerHTML = "";

 hideshow_hide(type);
}


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Server handling ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//
// Display data statistics
//

function displayStatistics()
{
 // Use AJAX to get the statistics

 OpenLayers.loadURL("statistics.cgi",null,null,runStatisticsSuccess);
}


//
// Success in running data statistics generation.
//

function runStatisticsSuccess(response)
{
 var statistics_data=document.getElementById("statistics_data");
 var statistics_link=document.getElementById("statistics_link");

 statistics_data.innerHTML="<pre>" + response.responseText + "</pre>";

 statistics_link.style.display="none";
}


//
// Submit form - perform the routing
//

function findRoute(type)
{
 tab_select("results");

 hideshow_hide('help_options');
 hideshow_hide('shortest');
 hideshow_hide('quickest');

 displayStatus("result","running");

 var url="router.cgi" + "?" + buildURLArguments(true) + ";type=" + type;

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
// Success in running router.
//

function runRouterSuccess(response)
{
 var lines=response.responseText.split('\n');

 var uuid=lines[0];
 var cpuinfo=lines[1];  // not used
 var distinfo=lines[2]; // not used
 var message=lines[3];  // content not used

 var link;

 // Update the status message

 if(message!="")
   {
    displayStatus("result","error");
    hideshow_show('help_route');

    link=document.getElementById("router_log_error");
    link.href="results.cgi?uuid=" + uuid + ";type=router;format=log";

    return;
   }
 else
   {
    displayStatus("result","complete");
    hideshow_hide('help_route');

    link=document.getElementById("router_log_complete");
    link.href="results.cgi?uuid=" + uuid + ";type=router;format=log";
   }

 // Update the routing result message

 link=document.getElementById(routing_type + "_html");
 link.href="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=html";
 link=document.getElementById(routing_type + "_gpx_track");
 link.href="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=gpx-track";
 link=document.getElementById(routing_type + "_gpx_route");
 link.href="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=gpx-route";
 link=document.getElementById(routing_type + "_text_all");
 link.href="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=text-all";
 link=document.getElementById(routing_type + "_text");
 link.href="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=text";

 var div_links=document.getElementById(routing_type + "_links");
 div_links.style.display = "";

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
 displayStatus("result","failed");
}


//
// Display the status
//

function displayStatus(type,subtype,content)
{
 var div_status=document.getElementById(type + "_status");

 var child=div_status.firstChild;

 do
   {
    if(child.id != undefined)
       child.style.display="none";

    child=child.nextSibling;
   }
 while(child != undefined);

 var chosen_status=document.getElementById(type + "_status_" + subtype);

 chosen_status.style.display="";

 if(content != null)
    chosen_status.innerHTML=content;
}


//
// Display the route
//

function displayResult(type,uuid)
{
 routing_type = type;

 // Add the route

 var url="results.cgi?uuid=" + uuid + ";type=" + routing_type + ";format=html";

 // Use AJAX to get the route

 OpenLayers.loadURL(url,null,null,getRouteSuccess,getRouteFailure);
}


//
// Success in getting route.
//

function getRouteSuccess(response)
{
 var lines=response.responseText.split('\n');
 var div_route=document.getElementById(routing_type + "_route");

 routepoints[routing_type]=[];

 var points=routepoints[routing_type];

 var table=0;
 var point=0;
 var total_table,total_word;

 for(var line=0;line<lines.length;line++)
   {
    var thisline=lines[line];

    if(table==0)
      {
       if(thisline.match('<table>'))
          table=1;
       else
          continue;
      }

    if(thisline.match('</table>'))
       break;

    if(thisline.match('<tr class=\'([a-z])\'>'))
      {
       var rowtype=RegExp.$1;

       if(rowtype=='c')
         {
          thisline.match('<td class=\'r\'> *([-0-9.]+) *([-0-9.]+)');
          points[point]={lat: Number(RegExp.$1), lon: Number(RegExp.$2), html: "", highway: "", distance: "", total: ""};

          point++;
         }
       else if(rowtype=='n')
         {
          points[point-1].html += thisline;
         }
       else if(rowtype=='s')
         {
          thisline.match('<span class=\'h\'>([^<]+)</span>');
          points[point-1].highway = RegExp.$1;

          thisline.match('<span class=\'d\'>([^<]+)</span>');
          points[point-1].distance = RegExp.$1;

          thisline.match('(<span class=\'j\'>[^<]+</span>)');
          points[point-1].total = RegExp.$1;

          thisline.match('^(.*).<span class=\'j\'>');

          points[point-1].html += RegExp.$1;
         }
       else if(rowtype=='t')
         {
          points[point-1].html += thisline;

          thisline.match('^(.*<td class=\'r\'>)');
          total_table = RegExp.$1;

          thisline.match('<td class=\'l\'>([^<]+)<');
          total_word = RegExp.$1;

          thisline.match('<span class=\'j\'>([^<]+)</span>');
          points[point-1].total = RegExp.$1;
         }
      }
   }

 displayStatus(routing_type,"info",points[point-1].total.bold());

 var result="<table onmouseout='highlight(\"" + routing_type + "\",-1)'>";

 for(var p=0;p<point-1;p++)
   {
    points[p].html += total_table + points[p].total;

    result=result + "<tr onclick='zoomTo(\"" + routing_type + "\"," + p + ")'" +
                    " onmouseover='highlight(\"" + routing_type + "\"," + p + ")'>" +
                    "<td class='distance' title='" + points[p].distance + "'>#" + (p+1) +
                    "<td class='highway'>" + points[p].highway;
   }

 result=result + "<tr onclick='zoomTo(\"" + routing_type + "\"," + p + ")'" +
                 " onmouseover='highlight(\"" + routing_type + "\"," + p + ")'>" +
                 "<td colspan='2'>" + total_word + " " + points[p].total;

 result=result + "</table>";

 div_route.innerHTML=result;
}


//
// Failure in getting route.
//

function getRouteFailure(response)
{
 var div_route=document.getElementById(routing_type + "_route");
 div_route.innerHTML = "";
}


//
// Perform a search
//

function DoSearch(marker)
{
 // Use AJAX to get the search result

 var search=routino.point[marker].search;

 var url="search.cgi?marker=" + marker + ";search=" + encodeURIComponent(search);

 OpenLayers.loadURL(url,null,null,runSearchSuccess);
}


var searchresults=[];

//
// Success in running search.
//

function runSearchSuccess(response)
{
 var lines=response.responseText.split('\n');

 var marker=lines[0];
 var cpuinfo=lines[1];  // not used
 var message=lines[2];  // not used

 if(message != "")
   {
    alert(message);
    return;
   }

 searchresults[marker]=[];

 for(var line=3;line<lines.length;line++)
   {
    var thisline=lines[line];

    if(thisline=="")
       break;

    thisline.match('([-.0-9]+) ([-.0-9]+) (.*)');

    searchresults[marker][searchresults[marker].length]={lat: Number(RegExp.$1), lon: Number(RegExp.$2), name: RegExp.$3};
   }

 if(searchresults[marker].length==1)
   {
    formSetSearch(marker,searchresults[marker][0].name);
    formSetCoords(marker,searchresults[marker][0].lon,searchresults[marker][0].lat,true);
   }
 else
   {
    var search=document.getElementById("search" + marker);
    var results=document.getElementById("searchresults" + marker);

    var innerHTML="<td colspan=\"3\">";

    for(var n=0;n<searchresults[marker].length;n++)
      {
       if(n>0)
          innerHTML+="<br>";

       innerHTML+="<a href=\"#\" onclick=\"choseSearchResult(" + marker + "," + n + ")\">" +
                  searchresults[marker][n].name +
                  "</a>";
      }

    results.innerHTML=innerHTML;

    results.style.display="";

    var searchresult_tr=document.getElementById("searchresults" + marker);
    searchresult_tr.style.display="";
   }
}


//
// Display search results.
//

function choseSearchResult(marker,n)
{
 if(n>=0)
   {
    formSetSearch(marker,searchresults[marker][n].name);
    formSetCoords(marker,searchresults[marker][n].lon,searchresults[marker][n].lat,true);
   }
}


//
// Clear search results.
//

function clearSearchResult(marker)
{
 var searchresult_tr=document.getElementById("searchresults" + marker);
 searchresult_tr.style.display="none";
}
