(function ($) {
  Drupal.behaviors.tppsGoogleMap = {
    attach:function (context) {

// @TODO Remove one of those methods because of double processing of 'click' event.


      //if ('tpps' in Drupal.settings && 'map_buttons' in Drupal.settings.tpps) {
      //  var map_buttons = Drupal.settings.tpps.map_buttons;
      //  $.each(map_buttons, function() {
      //    $('#' + this.button).on('click', getCoordinates);
      //  })
      //}

      // Process only 'input#map_button' and 'input#map-button'.
      var buttons = $('input').filter(function() {
        return (this.id.match(/map_button/) || this.id.match(/map-button/));
      });
      $.each(buttons, function(){
        $(this).attr('type', 'button').on('click', getCoordinates);
      });
    }
  }
})(jQuery);

// We are using array (list) of maps because there could be multiple species
// and multiple accession files on the same page.
// Also there could be a small map (height 100 px) in sidebar.
var maps = {};

/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
(function ($) {

  async function initMap() {
    let debugMode = true;
    let featureName = 'GoogleMap MarkerCluster';
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get libraries.
    const { Map, InfoWindow } = await google.maps.importLibrary("maps");
    const { AdvancedMarkerElement, PinElement } = await google.maps.importLibrary(
      "marker",
    )

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get data for markers.
    let fid = '';
    let locations = '';
    if ('tpps' in Drupal.settings) {
      if ('fid' in Drupal.settings.tpps) {
        fid = Drupal.settings.tpps.fid;
      }
      // Get locations.
      if ('tree_info' in Drupal.settings.tpps) {
        locations = Drupal.settings.tpps.tree_info;
      }
      else if ('locations' in Drupal.settings.tpps) {
        locations = Drupal.settings.tpps.locations;
      }
      else if ('study_locations' in Drupal.settings.tpps) {
        locations = Drupal.settings.tpps.study_locations;
      }
    }
    if ( !locations.length ) {
      if (debugMode) {
        console.log(featureName + ': no locations to show on map');
      }
      return;
    }
    if (debugMode) {
      console.log(locations);
    }

    // Map Id at page.
    let id='map';
    if (fid != '') { id = fid; }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Map wrapper.
    // Note: On study page there is no File Id because all species and all
    // files was processed.
    var $mapWrapper = jQuery('#' + fid + '_map_wrapper');

    // Make map wrapper visible and set correct height.
    var detail_regex = /tpps\/details\/TGDR.*/g;
    if (
      'tpps' in Drupal.settings && 'stage' in Drupal.settings.tpps
      && Drupal.settings.tpps.stage == 3
    ) {
      // Page 3.
      $mapWrapper.css({'height': '450px', "max-width": "800px"}).show();
    }
    else if(jQuery("#tpps_table_display").length) {
      $mapWrapper.css({'height': '450px'}).show();
    }
    else if (window.location.pathname.match(detail_regex)) {
      $mapWrapper.css({'height': '450px'}).show();
    }
    else {
      $mapWrapper.css({'height': '100px'}).show();
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Init map and add markers.
    if (typeof $mapWrapper[0] == 'undefined') {
      if (debugMode) {
        console.log(featureName + ': no map wrapper found.');
      }
      return;

    }
    maps[id] = new Map($mapWrapper[0], {
      center: {
        lat: Number(locations[0][1]),
        lng: Number(locations[0][2]),
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
    const markers = locations.map((position, i) => {
      const label = labels[position[0]];
      const pinGlyph = new PinElement({
        glyph: label,
        glyphColor: "white",
      });
      const markerPosition = { lat: Number(position[1]), lng: Number(position[2]) };
      const marker = new AdvancedMarkerElement({
        map: maps[id],
        //position: { lat: -25.344, lng: 131.031 },
        position: markerPosition,
        content: pinGlyph.element,
      });

      // markers can only be keyboard focusable when they have click listeners
      // open info window when marker is clicked
      marker.addListener("click", () => {
        infoWindow.setContent(position[1] + ", " + position[2]);
        infoWindow.open(maps[id], marker);
      });
      return marker;
    });

    // Add a marker clusterer to manage the markers.
    new markerClusterer.MarkerClusterer({
      markers: markers,
      map: maps[id],
    });
  }

  window.initMap = initMap;

}(jQuery));



/**
 * Gets locations for specific accession-file.
 */
function getCoordinates(){

  var fid = this.id.match(/(.*)_map_button/)[1];

  var fid, no_header, id_col, lat_col, long_col;
  try {
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
  catch (err) {
    console.log(err);
    return;
  }

  if (!id_col.length || !lat_col.length || !long_col.length) {
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
    Drupal.settings.tpps.locations = data;
    Drupal.settings.tpps.fid = fid;
    initMap();
  });
}
