//
// Routino data visualiser web page Javascript
//
// Part of the Routino routing software.
//
// This file Copyright 2008-2014 Andrew M. Bishop
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
// Data types
//

var data_types=[
                "junctions",
                "super",
                "oneway",
                "highway",
                "transport",
                "barrier",
                "turns",
                "speed",
                "weight",
                "height",
                "width",
                "length",
                "property",
                "errorlogs"
               ];


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Initialisation /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Process the URL query string and extract the arguments

var legal={"^lon"  : "^[-0-9.]+$",
           "^lat"  : "^[-0-9.]+$",
           "^zoom" : "^[0-9]+$"};

var args={};

if(location.search.length>1)
  {
   var query,queries;

   query=location.search.replace(/^\?/,"");
   query=query.replace(/;/g,"&");
   queries=query.split("&");

   for(var i=0;i<queries.length;i++)
     {
      queries[i].match(/^([^=]+)(=(.*))?$/);

      var k=RegExp.$1;
      var v=decodeURIComponent(RegExp.$3);

      for(var l in legal)
        {
         if(k.match(RegExp(l)) && v.match(RegExp(legal[l])))
            args[k]=v;
        }
     }
  }


////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Map handling /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

var map;
var layerMap=[], layerHighlights, layerVectors, layerBoxes;
var box;

//
// Initialise the 'map' object
//

function map_init()             // called from visualiser.html
{
 // Create the map (Map URLs and limits are in mapprops.js)

 map = L.map("map",
             {
              attributionControl: false,
              zoomControl: false,

              minZoom: mapprops.zoomout,
              maxZoom: mapprops.zoomin,

              maxBounds: L.latLngBounds(L.latLng(mapprops.southedge,mapprops.westedge),L.latLng(mapprops.northedge,mapprops.eastedge))
              });

 // Add map tile layers

 var baselayers={};

 for(var l=0; l<mapprops.mapdata.length; l++)
   {
    var urls=mapprops.mapdata[l].tiles.url.replace(/\${/g,"{");

    if(mapprops.mapdata[l].tiles.subdomains===undefined)
       layerMap[l] = L.tileLayer(urls);
    else
       layerMap[l] = L.tileLayer(urls, {subdomains: mapprops.mapdata[l].tiles.subdomains});

    baselayers[mapprops.mapdata[l].label]=layerMap[l];

    if(l===0)
       map.addLayer(layerMap[l]);
   }

 // Add the controls

 map.addControl(L.control.zoom());
 map.addControl(L.control.scale());
 map.addControl(L.control.layers(baselayers));

 // Update the attribution if the layer changes

 function change_attribution_event(event)
 {
  for(var l=0; l<mapprops.mapdata.length; l++)
     if(layerMap[l] == event.layer)
        change_attribution(l);
 }

 map.on("baselayerchange",change_attribution_event);

 function change_attribution(l)
 {
  var data_url =mapprops.mapdata[l].attribution.data_url;
  var data_text=mapprops.mapdata[l].attribution.data_text;
  var tile_url =mapprops.mapdata[l].attribution.tile_url;
  var tile_text=mapprops.mapdata[l].attribution.tile_text;

  document.getElementById("attribution_data").innerHTML="<a href=\"" + data_url + "\" target=\"data_attribution\">" + data_text + "</a>";
  document.getElementById("attribution_tile").innerHTML="<a href=\"" + tile_url + "\" target=\"tile_attribution\">" + tile_text + "</a>";
 }

 change_attribution(0);

 // Add two vectors layers (one for highlights that display behind the vectors)

 layerVectors = L.layerGroup();
 map.addLayer(layerVectors);

 layerHighlights = L.layerGroup();
 map.addLayer(layerHighlights);

 // Handle popup

 createPopup();

 // Add a boxes layer

 layerBoxes = L.rectangle(map.options.maxBounds,{stroke: false, color: "#f00", weight: 1, opacity: 1.0,
                                                 fill: false});

 map.addLayer(layerBoxes);

 box=false;

 // Move the map

 map.on("moveend", updateURLs);

 var lon =args["lon"];
 var lat =args["lat"];
 var zoom=args["zoom"];

 if(lon !== undefined && lat !== undefined && zoom !== undefined)
   {
    if(lon<mapprops.westedge) lon=mapprops.westedge;
    if(lon>mapprops.eastedge) lon=mapprops.eastedge;

    if(lat<mapprops.southedge) lat=mapprops.southedge;
    if(lat>mapprops.northedge) lat=mapprops.northedge;

    if(zoom<mapprops.zoomout) zoom=mapprops.zoomout;
    if(zoom>mapprops.zoomin)  zoom=mapprops.zoomin;

    map.setView(L.latLng(lat,lon),zoom);
   }
 else
    map.fitBounds(map.options.maxBounds);

 // Unhide editing URL if variable set

 if(mapprops.editurl !== undefined && mapprops.editurl !== "")
   {
    var edit_url=document.getElementById("edit_url");

    edit_url.style.display="";
    edit_url.href=mapprops.editurl;
   }

 updateURLs();
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
// Build a set of URL arguments for the map location
//

function buildMapArguments()
{
 var lonlat = map.getCenter();

 var zoom = map.getZoom();

 return "lat=" + format5f(lonlat.lat) + ";lon=" + format5f(lonlat.lng) + ";zoom=" + zoom;
}


//
// Update the URLs
//

function updateURLs()
{
 var mapargs=buildMapArguments();

 var links=document.getElementsByTagName("a");

 for(var i=0; i<links.length; i++)
   {
    var element=links[i];

    if(element.id == "permalink_url")
       element.href=location.pathname + "?" + mapargs;

    if(element.id == "router_url")
       element.href="router.html" + "?" + mapargs;

    if(element.id == "edit_url")
       element.href=mapprops.editurl + "?" + mapargs;

    if(element.id.match(/^lang_([a-zA-Z-]+)_url$/))
       element.href="visualiser.html" + "." + RegExp.$1 + "?" + mapargs;
   }
}


////////////////////////////////////////////////////////////////////////////////
///////////////////////// Popup and selection handling /////////////////////////
////////////////////////////////////////////////////////////////////////////////

var popup=null;

//
// Create a popup - independent of map because want it fixed on screen not fixed on map.
//

function createPopup()
{
 popup=document.createElement("div");

 popup.className = "popup";

 popup.innerHTML = "<span></span>";

 popup.style.display = "none";

 popup.style.position = "fixed";
 popup.style.top = "-4000px";
 popup.style.left = "-4000px";
 popup.style.zIndex = "100";

 popup.style.padding = "5px";

 popup.style.opacity=0.85;
 popup.style.backgroundColor="#C0C0C0";
 popup.style.border="4px solid #404040";

 document.body.appendChild(popup);
}


//
// Draw a popup - independent of map because want it fixed on screen not fixed on map.
//

function drawPopup(html)
{
 if(html===null)
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
// Select a circleMarker feature
//

function selectCircleMarkerFeature(feature,dump,event)
{
 if(dump)
    ajaxGET("visualiser.cgi?dump=" + dump, runDumpSuccess);

 layerHighlights.clearLayers();

 var highlight = L.circleMarker(feature.getLatLng(),{radius: 2*feature.getRadius(), fill: true, fillColor: "#F0F000", fillOpacity: 1.0,
                                                     stroke: false});

 layerHighlights.addLayer(highlight);

 highlight.bringToBack();
}


//
// Select a Polyline feature
//

function selectPolylineFeature(feature,dump,event)
{
 if(dump)
    ajaxGET("visualiser.cgi?dump=" + dump, runDumpSuccess);

 layerHighlights.clearLayers();

 var highlight = L.polyline(feature.getLatLngs(),{weight: 8, stroke: true, color: "#F0F000", opacity: 1.0,
                                                  fill: false});

 layerHighlights.addLayer(highlight);

 highlight.bringToBack();
}


//
// Select a Polygon feature
//

function selectPolygonFeature(feature,dump,event)
{
 if(dump)
    ajaxGET("visualiser.cgi?dump=" + dump, runDumpSuccess);

 layerHighlights.clearLayers();

 var highlight = L.polygon(feature.getLatLngs(),{weight: 8, stroke: true, color: "#F0F000", opacity: 1.0,
                                                 fill: false});

 layerHighlights.addLayer(highlight);

 highlight.bringToBack();
}


//
// Un-select a feature
//

function unselectFeature(feature)
{
 layerHighlights.clearLayers();

 drawPopup(null);
}


//
// Display the dump data
//

function runDumpSuccess(response)
{
 var string=response.responseText;

 if(mapprops.editurl !== undefined && mapprops.editurl !== "")
   {
    var types=["node", "way", "relation"];
    var Types=["Node", "Way", "Relation"];

    for(var t in types)
      {
       var Type=Types[t];
       var type=types[t];

       var regexp=RegExp(Type + " [0-9]+");

       var match;

       while((match=string.match(regexp)) !== null)
         {
          match=String(match);

          var id=match.slice(1+type.length,match.length);

          string=string.replace(regexp,Type + " <a href='" + mapprops.browseurl + "/" + type + "/" + id + "' target='" + type + id + "'>" + id + "</a>");
         }
      }
   }

 drawPopup(string.split("\n").join("<br>"));
}


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Server handling ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//
// Define an AJAX request object
//

function ajaxGET(url,success,failure,state)
{
 var ajaxRequest=new XMLHttpRequest();

 function ajaxGOT(options) {
  if(this.readyState==4)
     if(this.status==200)
       { if(typeof(options.success)=="function") options.success(this,options.state); }
     else
       { if(typeof(options.failure)=="function") options.failure(this,options.state); }
 }

 ajaxRequest.onreadystatechange = function(){ ajaxGOT.call(ajaxRequest,{success: success, failure: failure, state: state}); };
 ajaxRequest.open("GET", url, true);
 ajaxRequest.send(null);
}


//
// Display the status
//

function displayStatus(type,subtype,content)
{
 var child=document.getElementById("result_status").firstChild;

 do
   {
    if(child.id !== undefined)
       child.style.display="none";

    child=child.nextSibling;
   }
 while(child !== undefined);

 var chosen_status=document.getElementById("result_status_" + type);

 chosen_status.style.display="";

 if(subtype !== null)
   {
    var format_status=document.getElementById("result_status_" + subtype).innerHTML;

    chosen_status.innerHTML=format_status.replace("#",String(content));
   }
}


//
// Display data statistics
//

function displayStatistics()
{
 // Use AJAX to get the statistics

 ajaxGET("statistics.cgi", runStatisticsSuccess);
}


//
// Success in running data statistics generation.
//

function runStatisticsSuccess(response)
{
 document.getElementById("statistics_data").innerHTML="<pre>" + response.responseText + "</pre>";
 document.getElementById("statistics_link").style.display="none";
}


//
// Get the requested data
//

function displayData(datatype)  // called from visualiser.html
{
 for(var data in data_types)
    hideshow_hide(data_types[data]);

 if(datatype !== "")
    hideshow_show(datatype);

 // Delete the old data

 unselectFeature();

 layerVectors.clearLayers();
 layerHighlights.clearLayers();

 layerBoxes.setStyle({stroke:false});
 box=false;

 // Print the status

 displayStatus("no_data");

 // Return if just here to clear the data

 if(datatype === "")
    return;

 // Get the new data

 var mapbounds=map.getBounds();

 var url="visualiser.cgi";

 url=url + "?lonmin=" + format5f(mapbounds.getWest());
 url=url + ";latmin=" + format5f(mapbounds.getSouth());
 url=url + ";lonmax=" + format5f(mapbounds.getEast());
 url=url + ";latmax=" + format5f(mapbounds.getNorth());
 url=url + ";data=" + datatype;

 // Use AJAX to get the data

 switch(datatype)
   {
   case "junctions":
    ajaxGET(url, runJunctionsSuccess, runFailure);
    break;
   case "super":
    ajaxGET(url, runSuperSuccess, runFailure);
    break;
   case "oneway":
    ajaxGET(url, runOnewaySuccess, runFailure);
    break;
   case "highway":
    var highway;
    var highways=document.forms["highways"].elements["highway"];
    for(var h in highways)
       if(highways[h].checked)
          highway=highways[h].value;
    url+="-" + highway;
    ajaxGET(url, runHighwaySuccess, runFailure);
    break;
   case "transport":
    var transport;
    var transports=document.forms["transports"].elements["transport"];
    for(var t in transports)
       if(transports[t].checked)
          transport=transports[t].value;
    url+="-" + transport;
    ajaxGET(url, runTransportSuccess, runFailure);
    break;
   case "barrier":
    var transport;
    var transports=document.forms["barriers"].elements["barrier"];
    for(var t in transports)
       if(transports[t].checked)
          transport=transports[t].value;
    url+="-" + transport;
    ajaxGET(url, runBarrierSuccess, runFailure);
    break;
   case "turns":
    ajaxGET(url, runTurnsSuccess, runFailure);
    break;
   case "speed":
   case "weight":
   case "height":
   case "width":
   case "length":
    ajaxGET(url, runLimitSuccess, runFailure);
    break;
   case "property":
    var property;
    var properties=document.forms["properties"].elements["property"];
    for(var p in properties)
       if(properties[p].checked)
          property=properties[p].value;
    url+="-" + property;
    ajaxGET(url, runPropertySuccess, runFailure);
    break;
   case "errorlogs":
    ajaxGET(url, runErrorlogSuccess, runFailure);
    break;
   }
}


//
// Success in getting the junctions.
//

function runJunctionsSuccess(response)
{
 var lines=response.responseText.split("\n");

 var junction_colours={
                       0: "#FFFFFF",
                       1: "#FF0000",
                       2: "#FFFF00",
                       3: "#00FF00",
                       4: "#8B4513",
                       5: "#00BFFF",
                       6: "#FF69B4",
                       7: "#000000",
                       8: "#000000",
                       9: "#000000"
                      };

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat=words[1];
       var lon=words[2];
       var count=words[3];

       var lonlat = L.latLng(lat,lon);

       var feature = L.circleMarker(lonlat,{radius: 2, fill: true, fillColor: junction_colours[count], fillOpacity: 1.0,
                                            stroke: false});

       feature.on("click", (function(f,d) { return function(evt) { selectCircleMarkerFeature(f,d,evt); }; }(feature,dump)));

       layerVectors.addLayer(feature);
      }
   }

 displayStatus("data","junctions",lines.length-2);
}


//
// Success in getting the super-node and super-segments
//

function runSuperSuccess(response)
{
 var lines=response.responseText.split("\n");

 var nodelonlat;

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat=words[1];
       var lon=words[2];

       var lonlat = L.latLng(lat,lon);

       if(dump.charAt(0) == "n")
         {
          nodelonlat=lonlat;

          var feature = L.circleMarker(lonlat,{radius: 4, fill: true, fillColor: "#FF0000", fillOpacity: 1.0,
                                               stroke: false});

          feature.on("click", (function(f,d) { return function(evt) { selectCircleMarkerFeature(f,d,evt); }; }(feature,dump)));

          layerVectors.addLayer(feature);
         }
       else
         {
          var feature = L.polyline([nodelonlat,lonlat],{weight: 2, stroke: true, color: "#FF0000", opacity: 1.0,
                                                        fill: false});

          feature.on("click", (function(f,d) { return function(evt) { selectPolylineFeature(f,d,evt); }; }(feature,dump)));

          layerVectors.addLayer(feature);
         }
      }
   }

 displayStatus("data","super",lines.length-2);
}


//
// Success in getting the oneway data
//

function runOnewaySuccess(response)
{
 var hex={0: "00", 1: "11",  2: "22",  3: "33",  4: "44",  5: "55",  6: "66",  7: "77",
          8: "88", 9: "99", 10: "AA", 11: "BB", 12: "CC", 13: "DD", 14: "EE", 15: "FF"};

 var lines=response.responseText.split("\n");

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat1=words[1];
       var lon1=words[2];
       var lat2=words[3];
       var lon2=words[4];

       var lonlat1 = L.latLng(lat1,lon1);
       var lonlat2 = L.latLng(lat2,lon2);

       var point1 = map.options.crs.latLngToPoint(lonlat1,15);
       var point2 = map.options.crs.latLngToPoint(lonlat2,15);

       var dy = point2.y-point1.y;
       var dx = point2.x-point1.x;
       var dist = Math.sqrt(dx*dx+dy*dy)/2;
       var ang  = Math.atan2(-dy,dx);

       var point3 = L.point(point1.x-dy/dist,point1.y+dx/dist);
       var point4 = L.point(point1.x+dy/dist,point1.y-dx/dist);

       var lonlat3 = map.options.crs.pointToLatLng(point3,15);
       var lonlat4 = map.options.crs.pointToLatLng(point4,15);

       var r=Math.round(7.5+7.9*Math.cos(ang));
       var g=Math.round(7.5+7.9*Math.cos(ang+2.0943951));
       var b=Math.round(7.5+7.9*Math.cos(ang-2.0943951));
       var colour = "#" + hex[r] + hex[g] + hex[b];

       var feature = L.polygon([lonlat2,lonlat3,lonlat4],{weight: 2, stroke: true, color: colour, opacity: 1.0,
                                                          fill: false});

       feature.on("click", (function(f,d) { return function(evt) { selectPolygonFeature(f,d,evt); }; }(feature,dump)));

       layerVectors.addLayer(feature);
      }
   }

 displayStatus("data","oneway",lines.length-2);
}


//
// Success in getting the highway data
//

function runHighwaySuccess(response)
{
 var lines=response.responseText.split("\n");

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat1=words[1];
       var lon1=words[2];
       var lat2=words[3];
       var lon2=words[4];

       var lonlat1 = L.latLng(lat1,lon1);
       var lonlat2 = L.latLng(lat2,lon2);

       var feature = L.polyline([lonlat1,lonlat2],{weight: 2, stroke: true, color: "#FF0000", opacity: 1.0,
                                                   fill: false});

       feature.on("click", (function(f,d) { return function(evt) { selectPolylineFeature(f,d,evt); }; }(feature,dump)));

       layerVectors.addLayer(feature);
      }
   }

 displayStatus("data","highway",lines.length-2);
}


//
// Success in getting the transport data
//

function runTransportSuccess(response)
{
 var lines=response.responseText.split("\n");

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat1=words[1];
       var lon1=words[2];
       var lat2=words[3];
       var lon2=words[4];

       var lonlat1 = L.latLng(lat1,lon1);
       var lonlat2 = L.latLng(lat2,lon2);

       var feature = L.polyline([lonlat1,lonlat2],{weight: 2, stroke: true, color: "#FF0000", opacity: 1.0,
                                                   fill: false});

       feature.on("click", (function(f,d) { return function(evt) { selectPolylineFeature(f,d,evt); }; }(feature,dump)));

       layerVectors.addLayer(feature);
      }
   }

 displayStatus("data","transport",lines.length-2);
}


//
// Success in getting the barrier data
//

function runBarrierSuccess(response)
{
 var lines=response.responseText.split("\n");

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat=words[1];
       var lon=words[2];

       var lonlat = L.latLng(lat,lon);

       var feature = L.circleMarker(lonlat,{radius: 2, fill: true, fillColor: "#FF0000", fillOpacity: 1.0,
                                            stroke: false});

       feature.on("click", (function(f,d) { return function(evt) { selectCircleMarkerFeature(f,d,evt); }; }(feature,dump)));

       layerVectors.addLayer(feature);
      }
   }

 displayStatus("data","barrier",lines.length-2);
}


//
// Success in getting the turn restrictions data
//

function runTurnsSuccess(response)
{
 var lines=response.responseText.split("\n");

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat1=words[1];
       var lon1=words[2];
       var lat2=words[3];
       var lon2=words[4];
       var lat3=words[5];
       var lon3=words[6];

       var lonlat1 = L.latLng(lat1,lon1);
       var lonlat2 = L.latLng(lat2,lon2);
       var lonlat3 = L.latLng(lat3,lon3);

       var feature = L.polygon([lonlat1,lonlat2,lonlat3],{weight: 2, stroke: true, color: "#FF0000", opacity: 1.0,
                                                          fill: false});

       feature.on("click", (function(f,d) { return function(evt) { selectPolygonFeature(f,d,evt); }; }(feature,dump)));

       layerVectors.addLayer(feature);
      }
   }

 displayStatus("data","turns",lines.length-2);
}


//
// Success in getting the speed/weight/height/width/length limits
//

function runLimitSuccess(response)
{
 var lines=response.responseText.split("\n");

 var nodelonlat;

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat=words[1];
       var lon=words[2];
       var number=words[3];

       var lonlat = L.latLng(lat,lon);

       if(number === undefined)
         {
          nodelonlat=lonlat;

          var feature = L.circleMarker(lonlat,{radius: 3, fill: true, fillColor: "#FF0000", fillOpacity: 1.0,
                                               stroke: false});

          feature.on("click", (function(f,d) { return function(evt) { selectCircleMarkerFeature(f,d,evt); }; }(feature,dump)));

          layerVectors.addLayer(feature);
         }
       else
         {
          var lonlat = L.latLng(lat,lon);

          var feature = L.polyline([nodelonlat,lonlat],{weight: 2, stroke: true, color: "#FF0000", opacity: 1.0,
                                                        fill: false});

          feature.on("click", (function(f,d) { return function(evt) { selectPolylineFeature(f,d,evt); }; }(feature,dump)));

          layerVectors.addLayer(feature);

          var point1 = map.options.crs.latLngToPoint(nodelonlat,15);
          var point2 = map.options.crs.latLngToPoint(lonlat    ,15);

          var dy = point2.y-point1.y;
          var dx = point2.x-point1.x;
          var dist = Math.sqrt(dx*dx+dy*dy)/24;

          var point = L.point(point1.x+dx/dist,point1.y+dy/dist);

          feature=L.marker(map.options.crs.pointToLatLng(point,15), {clickable: false,
                                                                     icon: L.icon({iconUrl: "icons/limit-" + number + ".png",
                                                                                   iconSize: L.point(19,19),
                                                                                   iconAnchor: L.point(9,10)})});

          layerVectors.addLayer(feature);
         }
      }
   }

 displayStatus("data","limit",lines.length-2);
}


//
// Success in getting the property data
//

function runPropertySuccess(response)
{
 var lines=response.responseText.split("\n");

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat1=words[1];
       var lon1=words[2];
       var lat2=words[3];
       var lon2=words[4];

       var lonlat1 = L.latLng(lat1,lon1);
       var lonlat2 = L.latLng(lat2,lon2);

       var feature = L.polyline([lonlat1,lonlat2],{weight: 2, stroke: true, color: "#FF0000", opacity: 1.0,
                                                   fill: false});

       feature.on("click", (function(f,d) { return function(evt) { selectPolylineFeature(f,d,evt); }; }(feature,dump)));

       layerVectors.addLayer(feature);
      }
   }

 displayStatus("data","property",lines.length-2);
}


//
// Success in getting the error log data
//

function runErrorlogSuccess(response)
{
 var lines=response.responseText.split("\n");

 for(var line=0;line<lines.length;line++)
   {
    var words=lines[line].split(" ");

    if(line === 0)
      {
       var lat1=words[0];
       var lon1=words[1];
       var lat2=words[2];
       var lon2=words[3];

       var bounds = L.latLngBounds(L.latLng(lat1,lon1),L.latLng(lat2,lon2));

       layerBoxes.setBounds(bounds);

       layerBoxes.setStyle({stroke: true});
       box=true;
      }
    else if(words[0] !== "")
      {
       var dump=words[0];
       var lat=words[1];
       var lon=words[2];

       var lonlat = L.latLng(lat,lon);

       var feature = L.circleMarker(lonlat,{radius: 3, fill: true, fillColor: "#FF0000", fillOpacity: 1.0,
                                            stroke: false});

       feature.on("click", (function(f,d) { return function(evt) { selectCircleMarkerFeature(f,d,evt); }; }(feature,dump)));

       layerVectors.addLayer(feature);
      }
   }

 displayStatus("data","errorlogs",lines.length-2);
}


//
// Failure in getting data.
//

function runFailure(response)
{
 displayStatus("failed");
}
