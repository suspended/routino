////////////////////////////////////////////////////////////////////////////////
///////////////////// Map loader (OpenLayers or Leaflet) ///////////////////////
////////////////////////////////////////////////////////////////////////////////


function map_load(callbacks)
{
 var head = document.getElementsByTagName("head")[0];

 /* Javascript loader */

 function load_js(url,sync)
 {
  var script = document.createElement("script");
  script.src = url;
  script.type = "text/javascript";

  if(sync===true)
     script.onload = function() {eval(callbacks);};

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

    load_js(location.pathname.replace(/\.html.*/,".leaflet.js"),true);
   }
 else
   {
    load_js("../openlayers/OpenLayers.js");

    load_js(location.pathname.replace(/\.html.*/,".openlayers.js"),true);
   }
}
