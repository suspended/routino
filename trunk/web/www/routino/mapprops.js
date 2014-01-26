////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Routino map properties /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

var mapprops={ // contains all properties for the map to be displayed.

 // EDIT THIS below to change the map library (either 'openlayers' or 'leaflet').

  library: "openlayers",
  //library: "leaflet",

 // EDIT THIS above to change the map library (either 'openlayers' or 'leaflet').


 // EDIT THIS below to change the visible map limits

  westedge:  -11.0,          // Minimum longitude (degrees)
  eastedge:    2.0,          // Maximum longitude (degrees)

  southedge:  49.5,          // Minimum latitude (degrees)
  northedge:  61.0,          // Maximum latitude (degrees)

  zoomout:       4,          // Minimum zoom
  zoomin:       15,          // Maximum zoom

 // EDIT THIS above to change the visible map limits


 // EDIT THIS below to change the map URL(s) and copyright notices

  mapdata: [
           {
            label: "OpenStreetMap",
            tiles: {
                    url: "http://${s}.tile.openstreetmap.org/${z}/${x}/${y}.png",
                    subdomains: ["a","b","c"]
                   },
            attribution: {
                          data_url:  "http://www.openstreetmap.org/copyright",
                          data_text: "© OpenStreetMap contributors",
                          tile_url:  "http://www.openstreetmap.org/copyright",
                          tile_text: "© OpenStreetMap"
                         }
           },
           {
            label: "MapQuest",
            tiles: {
                    url: "http://otile${s}.mqcdn.com/tiles/1.0.0/map/${z}/${x}/${y}.jpg",
                    subdomains: ["1","2","3","4"]
                   },
            attribution: {
                          data_url:  "http://www.openstreetmap.org/copyright",
                          data_text: "© OpenStreetMap contributors",
                          tile_url:  "http://www.mapquest.com/",
                          tile_text: "© MapQuest <img src=\"http://developer.mapquest.com/content/osm/mq_logo.png\">"
                         }
           }
           ],

 // EDIT THIS above to change the map URL(s) and copyright notices


 // EDIT THIS below to change the map source data editing URL (or leave blank for no link)

  editurl: "http://www.openstreetmap.org/edit",

 // EDIT THIS above to change the map source data editing URL (or leave blank for no link)


 // EDIT THIS below to change the map source data browsing URL (or leave blank for no link)

  browseurl: "http://www.openstreetmap.org/browse",

 // EDIT THIS above to change the map source data browsing URL (or leave blank for no link)


 // EDIT THIS below to change the maximum number of markers to include in the HTML

  maxmarkers: 9

 // EDIT THIS above to change the maximum number of markers to include in the HTML

}; // end of map properties
