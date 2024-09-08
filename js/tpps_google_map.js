(function ($) {
  Drupal.behaviors.tppsGoogleMap = {
    attach:function (context) {

// @TODO Add on click event handler to hide map and remove removed file column
      // metadata.
      // Name of the 'Remove' button:
      // 'tree-accession_species-1_file_remove_button'

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
    let debugMode = Drupal.settings.tpps.googleMap.debugMode ?? false;
    if (debugMode) {
      console.log('initMap() called.');
    }
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

    // Check if we have locations for markers.
    if ( !locations.length ) {
      if (debugMode) {
        console.log(featureName + ': no locations to show on map');
      }
      return;
    }
    else {
      if (debugMode) {
        console.log('List of locations for markers');
        console.table(locations);
      }
    }

    // Map Id at page.
    let id = fid ?? 'map';

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
      zoom: Number(Drupal.settings.tpps.googleMap.zoom.defaultLevel ?? 3),
      mapId: Drupal.settings.tpps.googleMap.id,
    });

    const infoWindow = new InfoWindow({
      content: "",
      disableAutoPan: true,
    });

    var bounds = new google.maps.LatLngBounds();

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
      bounds.extend(markerPosition);
      // markers can only be keyboard focusable when they have click listeners
      // open info window when marker is clicked
      marker.addListener("click", () => {
        infoWindow.setContent(position[1] + ", " + position[2]);
        infoWindow.open(maps[id], marker);
      });
      return marker;
    });

    // Zoom to show all markers.
    var customZoom = google.maps.event.addListener(maps[id], 'bounds_changed', function() {
      // This must be done only once.
      customZoom.remove();
      // When all coordinates are the same set zoom to show number if markers
      // instead of zoom too much and show single marker.
      if (bounds.getNorthEast().equals(bounds.getSouthWest())) {
        smoothZoom(
          this,
          Number((Drupal.settings.tpps.googleMap.zoom.customLevel ?? 17) - 3),
          Number(Drupal.settings.tpps.googleMap.zoom.customLevel ?? 17)
          //this.getZoom()
        );
      }
      else {
        // Zoom to show all existing markers.
        this.fitBounds(bounds);
      }
    });

    // Add a marker clusterer to manage the markers.
    new markerClusterer.MarkerClusterer({
      markers: markers,
      map: maps[id],
    });
    $mapWrapper[0].scrollIntoView({block: "center", behavior: "smooth"});
  }

  window.initMap = initMap;

  /**
   * Zoom google map smooth.
   *
   * @param map
   *   Google Map object.
   * @param cnt
   *  Starting zoom level
   * @param max
   *   Final zoom level.
   */
  function smoothZoom (map, cnt, max) {
    if (cnt >= max) {
      return;
    }
    else {
      z = google.maps.event.addListener(map, 'zoom_changed', function(event){
        google.maps.event.removeListener(z);
        smoothZoom(map, cnt + 1, max);
      });
      setTimeout(
        function() {map.setZoom(Number(cnt))},
        Number(Drupal.settings.tpps.googleMap.zoom.smoothTimeout ?? 100)
      );
    }
  }

}(jQuery));


/**
 * Gets locations for specific accession-file.
 */
function getCoordinates(){

  // @TODO Replace 'jQuery' with '$'.
  let debugMode = Drupal.settings.tpps.googleMap.debugMode ?? false;

  if (debugMode) {
    console.log('getCoordinates');
  }


  var no_header, id_col, lat_col, long_col;
  let fileMeta;
  let organismFid;
  let $fileIdField;
  let commonName;
  let organismId;
  let organismNumber;
  let $fileWrapper;
  let columnName;
  let selectBoxName;
  let value;

  var fid = this.id.match(/(.*)_map_button/)[1];
  if (
    ! ('tpps' in Drupal.settings)
    || (
      'tpps' in Drupal.settings
      && ! ('accession_files' in Drupal.settings.tpps)
    )
    || (
      'tpps' in Drupal.settings
      && 'accession_files' in Drupal.settings.tpps
      // @TODO Must be not under 'settings':
      // Drupal.tpps.accession_files[fid]
      && typeof Drupal.settings.tpps.accession_files[fid] == 'undefined'
    )
  ) {
    // File Id not found. Get data from form.
    if (debugMode) {
      console.log('File Id not found. Get data from form.');
    }

    // Column name are letters.
    // Note: Change 65 to 97 for lowercase.
    // WARNING: Won't work if accession file has more then A-Z columns.
    const alphabets = [...Array(26).keys()].map((n) => String.fromCharCode(65 + n));
    organismNumber = Drupal.settings.tpps.organismNumber;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Loop organisms to find accession files.
    for (organismId = 1; organismId <= organismNumber; organismId++) {
      // @TODO Stop Loop when 'fid' was found.
      commonName = 'tree-accession[species-' + organismId + '][file]';
      $fileIdField = jQuery('input[name="' + commonName + '[fid]"]');
      organismFid = $fileIdField.val();
      if (debugMode) {
        console.log('Organism ' + organismId + '; File Id: ' + organismFid );
      }
      $fileWrapper = $fileIdField.parent('.form-managed-file');
      // @TODO Get real value of the no-header field.
      Drupal.settings.tpps['accession_files'] = [];
      Drupal.settings.tpps.accession_files[organismFid] = [];
      Drupal.settings.tpps.accession_files[organismFid]['fid'] = organismFid;
      Drupal.settings.tpps.accession_files[organismFid]['no_header'] = null;
      for (columnName of alphabets) {
        selectBoxName = commonName + '[columns][' + columnName + ']';
        value = $fileWrapper.find('select[name="' + selectBoxName + '"]')
          .find(":selected").val();

        // @TODO Create name constants for those numbers.
        switch (value) {
          case '1':
            // Plant Identifier.
            Drupal.settings.tpps.accession_files[organismFid]['id_col'] = columnName;
            break;
          case '4':
            // Latitude
            Drupal.settings.tpps.accession_files[organismFid]['lat_col'] = columnName;
            break;
          case '5':
            // Longitude
            Drupal.settings.tpps.accession_files[organismFid]['long_col'] = columnName;
            break;
        }
        // Check if all required fields was set.
        if (
             'id_col'   in Drupal.settings.tpps.accession_files[organismFid]
          && 'lat_col'  in Drupal.settings.tpps.accession_files[organismFid]
          && 'long_col' in Drupal.settings.tpps.accession_files[organismFid]
        ) {
          break;
        }
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get data from Drupal.settings.
  // Works fine.
  if (
    'tpps' in Drupal.settings && 'accession_files' in Drupal.settings.tpps
    && typeof Drupal.settings.tpps.accession_files[fid] != 'undefined'
  ) {
    // File Id was found. Let's check properties.
    if ( !(
         'id_col'   in Drupal.settings.tpps.accession_files[fid]
      && 'lat_col'  in Drupal.settings.tpps.accession_files[fid]
      && 'long_col' in Drupal.settings.tpps.accession_files[fid]
    )) {
      delete Drupal.settings.tpps.accession_files[fid];
      if (debugMode) {
        console.log(
          'No file metadata in Drupal.settings.tpps.accession_files for FileId: ' + fid
        );
        //console.table(Drupal.settings.tpps.accession_files[fid]);
      }
      console.log('Going to hide the map for File: ' + fid);
      // Since columns set incorrectly we hide the map.
      var $mapWrapper = jQuery('#' + fid + '_map_wrapper');
      // Works!
      $mapWrapper.hide();
      if (debugMode) {
        if ($mapWrapper.is(":visible")){
          console.log('Visible');
        } else{
          console.log('Not Visible');
        }
      }
      return;
    }
  }
  // At tpps-admin-panel/TGDR978 we doesn't have 'fid' element.
  // @TODO Add at server side.
  Drupal.settings.tpps.accession_files[fid]['fid'] = fid;

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get data from server.
  var request = jQuery.post(
    '/tpps-accession',
    {...Drupal.settings.tpps.accession_files[fid]}
  );
  request.done(function (data) {
    Drupal.settings.tpps.locations = data;
    Drupal.settings.tpps.fid = fid;
    initMap();
  });
}

/**
 * AJAX-callback called when managed file columns changed.
 *
 * Called from page_3_ajax.php function tpps_accession_pop_group():
 * Called everytime user changes column datatype using dropdown menu.
 *
 * @param selector
 *   Selector of the button which shows the map.
 *
 * @param int fid
 *   File Id of changed/updated file.
 *
 */
jQuery.fn.mapButtonsClick = function (selector, fid) {
  let debugMode = Drupal.settings.tpps.googleMap.debugMode ?? false;
  if (debugMode) {
    console.log('mapButtonClick');
    console.log(selector);
  }
  // Disable all handlers for 'click' event.
  jQuery(selector).off('click');

  // Since user changed columns don't use previously stored data but get
  // updated data from form in getCoordinates().
  if (typeof fid == 'undefined') {
    // @TODO check if this works when file was removed and new one uploaded.
    delete Drupal.settings.tpps.accession_files;
    console.log('Removed accession files columns information: all');
  }
  else {
    delete Drupal.settings.tpps.accession_files[fid];
    if (debugMode) {
      console.log('Removed accession files columns information: ' + fid);
    }
  }
  delete Drupal.settings.tpps.locations;

  // @TODO Called twice on Page3 button click.
  jQuery(selector).on('click', getCoordinates);
  jQuery(selector).trigger('click');

  initMap();
}

