(function ($) {
  Drupal.behaviors.tppsGoogleMap = {
    attach:function (context) {
      if (
        typeof Drupal.settings.tpps !== 'undefined'
        && typeof Drupal.settings.tpps.map_buttons !== 'undefined'
      ) {
        var map_buttons = Drupal.settings.tpps.map_buttons;
        $.each(map_buttons, function() {
          $('#' + this.button).click(getCoordinates);
        })
      }

      var buttons = $('input').filter(function() {
        return (this.id.match(/map_button/) || this.id.match(/map-button/));
      });
      $.each(buttons, function(){
        $(this).attr('type', 'button')
      });

      var detail_regex = /tpps\/details\/TGDR.*/g;
      if (!window.location.pathname.match(detail_regex)) {
        $("#map_wrapper").hide();
      }


    }
  }
})(jQuery);

jQuery.fn.mapButtonsClick = function (selector) {
  jQuery(selector).off('click');
  jQuery(selector).click(getCoordinates);
  initMap();
}

var maps = {};

/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
(function ($) {

  async function initMap() {
    var detail_regex = /tpps\/details\/TGDR.*/g;
    const { Map, InfoWindow } = await google.maps.importLibrary("maps");
    const { AdvancedMarkerElement, PinElement } = await google.maps.importLibrary(
      "marker",
    )
    if (
      typeof Drupal.settings.tpps !== 'undefined'
      && typeof Drupal.settings.tpps.map_buttons !== 'undefined'
    ) {
      // Buttons.
      var mapButtons = Drupal.settings.tpps.map_buttons;
      jQuery.each(mapButtons, function() {
        var fid = this.fid;
        var mapWrapper = jQuery('#' + fid + '_map_wrapper').show().css({'height': '450px'})[0];
        if (Drupal.settings.tpps.markerClusterUrlType == 'new') {
          maps[fid] = new Map(mapWrapper, {
            center: {lat:0, lng:0},
            zoom: 5,
            mapId: Drupal.settings.tpps.googleMapId,
          });
        }
        else {
          maps[fid] = new google.maps.Map(mapWrapper, {
            center: {lat:0, lng:0},
            zoom: 5,
          });
        }

        maps[fid + '_markers'] = [];
        maps[fid + '_total_lat'];
        maps[fid + '_total_long'];
      });
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Study details page.
    if (
      typeof Drupal.settings.tpps != 'undefined'
      && typeof Drupal.settings.tpps.tree_info != 'undefined'
      && (
        window.location.pathname.match(detail_regex)
        || typeof Drupal.settings.tpps.study_locations !== 'undefined'
      )
    ) {
      //var mapWrapper = document.getElementById('_map_wrapper');
      var mapWrapper = jQuery('#_map_wrapper').show().css({'height': '450px'})[0];

      if (Drupal.settings.tpps.markerClusterUrlType == 'new') {
        maps['map'] = new Map(mapWrapper, {
          center: {
            lat: Number(Drupal.settings.tpps.tree_info[0][1]),
            lng: Number(Drupal.settings.tpps.tree_info[0][2]),
          },
          zoom: 5,
          mapId: Drupal.settings.tpps.googleMapId,
        });

        const infoWindow = new InfoWindow({
          content: "",
          disableAutoPan: true,
        });
        // Create an array of alphabetical characters used to label the markers.
        const labels = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        // Add some markers to the map.
        const markers = Drupal.settings.tpps.tree_info.map((position, i) => {
          const label = labels[position[0]];
          const pinGlyph = new PinElement({
            glyph: label,
            glyphColor: "white",
          });
          const markerPosition = { lat: Number(position[1]), lng: Number(position[2]) };
          const marker = new AdvancedMarkerElement({
            map: maps['map'],
            //position: { lat: -25.344, lng: 131.031 },
            position: markerPosition,
            content: pinGlyph.element,
          });

          // markers can only be keyboard focusable when they have click listeners
          // open info window when marker is clicked
          marker.addListener("click", () => {
            infoWindow.setContent(position[1] + ", " + position[2]);
            infoWindow.open(maps['map'], marker);
          });
          return marker;
        });

        // Add a marker clusterer to manage the markers.
        new markerClusterer.MarkerClusterer({
          markers: markers,
          map: maps['map'],
        });
      }
      else {
        maps[''] = new Map(mapWrapper, {
          center: { lat: 0, lng: 0 },
          zoom: 5,
        });
        maps['_markers'] = [];
        maps['_total_lat'];
        maps['_total_long'];
        jQuery.fn.updateMap(
          Drupal.settings.tpps.tree_info
        );
      }
    }
  }

  window.initMap = initMap;
}(jQuery));


function clearMarkers(prefix) {
  for (var i = 0; i < maps[prefix + '_markers'].length; i++) {
    maps[prefix + '_markers'][i].setMap(null);
  }
  maps[prefix + '_markers'] = [];
}

function getCoordinates(){
  var fid = this.id.match(/(.*)_map_button/)[1];

  var fid, no_header, id_col, lat_col, long_col;
  try{
    file = Drupal.settings.tpps.accession_files[fid];
    if (typeof file === 'undefined') {
      jQuery.each(Drupal.settings.tpps.accession_files, function() {
        if (this.fid == fid) {
          file = this;
        }
      })
      if (typeof file === 'undefined') {
        return;
      }
    }

    no_header = file.no_header;
    id_col = file.id_col;
    lat_col = file.lat_col;
    long_col = file.long_col;
  }
  catch(err){
    console.log(err);
    return;
  }

  if (typeof id_col === 'undefined' || typeof lat_col === 'undefined' || typeof long_col === 'undefined'){
    jQuery("#" + Drupal.settings.tpps.map_buttons[fid].wrapper).hide();
    return;
  }

  var request = jQuery.post('/tpps-accession', {
    fid: fid,
    no_header: no_header,
    id_col: id_col,
    lat_col: lat_col,
    long_col: long_col
  });

  request.done(function (data) {

// @TODO Update to !

    jQuery.fn.updateMap(data, fid);
  });
}

jQuery.fn.updateMap = function(locations, fid = "") {
  jQuery("#" + fid + "_map_wrapper").show();
  var detail_regex = /tpps\/details\/TGDR.*/g;
  if (
    typeof Drupal.settings.tpps !== 'undefined'
    && typeof Drupal.settings.tpps.stage !== 'undefined'
    && Drupal.settings.tpps.stage == 3
  ) {
    // Page 3.
    jQuery("#" + fid + "_map_wrapper").css({"height": "450px"});
    jQuery("#" + fid + "_map_wrapper").css({"max-width": "800px"});
  }
  else if(jQuery("#tpps_table_display").length > 0) {
    jQuery("#" + fid + "_map_wrapper").css({"height": "450px"});
  }
  else if (window.location.pathname.match(detail_regex)) {
    jQuery("#" + fid + "_map_wrapper").css({"height": "450px"});
  }
  else {
    jQuery("#" + fid + "_map_wrapper").css({"height": "100px"});
  }

  clearMarkers(fid);
  maps[fid + '_total_lat'] = 0;
  maps[fid + '_total_long'] = 0;
  timeout = 2000/locations.length;

  maps[fid + '_markers'] = locations.map(function (location, i) {
    maps[fid + '_total_lat'] += parseInt(location[1]);
    maps[fid + '_total_long'] += parseInt(location[2]);

    // Marker.
    var marker = new google.maps.Marker({
      position: new google.maps.LatLng(location[1], location[2])
    });
    var infowindow = new google.maps.InfoWindow({
      content: location[0] + '<br>Location: ' + location[1] + ', ' + location[2]
    });
    marker.addListener('click', function() {
      infowindow.open(maps[fid], maps[fid + '_markers'][i]);
    });
    return marker;
  });

  if (typeof maps[fid + '_cluster'] !== 'undefined') {
    maps[fid + '_cluster'].clearMarkers();
  }

  if (Drupal.settings.tpps.markerClusterUrlType == 'new') {
    maps[fid + '_cluster'] = new markerClusterer.MarkerClusterer(
      maps[fid],
      maps[fid + '_markers'],
    );
  }
  else {
    maps[fid + '_cluster'] = new MarkerClusterer(
      maps[fid],
      maps[fid + '_markers'],
      {imagePath: 'https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/m'}
    );
    var center = new google.maps.LatLng(
      maps[fid + '_total_lat']/locations.length,
      maps[fid + '_total_long']/locations.length
    );
    maps[fid].panTo(center);
  }



// https://developers.google.com/maps/documentation/javascript/markers?hl=ru
//marker.setMap(maps[fid]);

};
