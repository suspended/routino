////////////////////////////////////////////////////////////////////////////////
///////////////////// Map loader (OpenLayers or Leaflet) ///////////////////////
////////////////////////////////////////////////////////////////////////////////


function map_load(callbacks)
{
 var pending=1;
 var head = document.getElementsByTagName("head")[0];

 /* Call the callbacks when everything is loaded. */

 function call_callbacks()
 {
  eval(callbacks);
 }

 /* Javascript loader */

 function load_js(url)
 {
  var script = document.createElement("script");
  script.src = url;
  script.type = "text/javascript";

  script.onload = function() { if(!--pending) call_callbacks(); };

  pending++;

  head.appendChild(script);
 }

 /* CSS loader */

 function load_css(url)
 {
  var link = document.createElement("link");
  link.href = url;
  link.type = "text/css";
  link.rel = "stylesheet";

  head.appendChild(link);
 }

 /* Load the external library and local code */

 if(mapprops.library == "leaflet")
   {
    load_css("../leaflet/leaflet.css");
    load_js("../leaflet/leaflet.js");

    load_js(location.pathname.replace(/\.html.*/,".leaflet.js"));
   }
 else
   {
    load_js("../openlayers/OpenLayers.js");

    load_js(location.pathname.replace(/\.html.*/,".openlayers.js"));
   }

 if(!--pending) call_callbacks();
}
