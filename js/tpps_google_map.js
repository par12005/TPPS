
// We are using array (list) of maps because there could be multiple species
// and multiple accession files on the same page.
// Also there could be a small map (height 100 px) in sidebar.
var maps = {};

/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
(function ($) {

  async function initMap() {
    let featureName = 'initMap';
    dog('called.');
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
      dog('no locations to show on map');
      return;
    }
    else {
      dog('List of locations for markers');
      let debugMode = Drupal.settings.tpps.googleMap.debugMode ?? false;
      if (debugMode) {
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

    // Init map and add markers.
    if (typeof $mapWrapper[0] == 'undefined') {
      dog('Map wrapper wasn\'t found.');
      return;
    }

    // Map-wrapper found at page. Let's make it visible and set height.
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

    // Add some markers to the map.
    const markers = locations.map((position, i) => {
      const label = position[0];
      const pinGlyph = new PinElement({
        glyph: label.toString(),
        glyphColor: "white",
      });
      const markerPosition = { lat: Number(position[1]), lng: Number(position[2]) };
      const marker = new AdvancedMarkerElement({
        map: maps[id],
        // Example:
        // position: { lat: -25.344, lng: 131.031 },
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

  // Make it global.
  window.initMap = initMap;

  /**
   * Zoom google map smoothly.
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  Drupal.behaviors.tppsGoogleMap = {
    attach:function (context) {

      let featureName = 'Drupal.behaviors.tppsGoogleMap';
      dog('called.', featureName);

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Show map on page load if file was uploaded before on Page 3.
      //
      // @TODO Check if page has managed file fields.
      let organismNumber = Drupal.settings.tpps.organismNumber ?? 1;
      for (organismId = 1; organismId <= organismNumber; organismId++) {
        let columnTableSelector = '#edit-tree-accession-species-' + organismId
          + '-file-columns';
        var $columnTable = $(columnTableSelector, context);
        if ($columnTable.length) {
          $columnTable.addClass('tpps-google-map-processed');
          let fid = $('input[name="tree-accession[species-'
            + organismId + '][file][fid]"]').val();
          dog('File was uploaded. Organism: ' + organismId + ', fid: ' + fid, featureName);
          if (typeof fid != 'undefined' && Number(fid) > 0) {
            dog('Going to call getCoordinates(' + fid + ')', featureName);
            getCoordinates(fid);
            //dog('Going to call initMap()', featureName);
            //initMap();
          }
        }
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



      // Note: Each time map reloaded by AJAX it will be updated.
      $('.tpps-map-wrapper', context).each(function() {
        dog('Map Wrapper found at page', featureName);
        let fid = $(this).data('fid');
        if (typeof fid != 'undefined' && fid) {
          dog('Map Wrapper has fid: ' +  fid, featureName);
          // Since we doesn't have
          dog('Map Wrapper. Update markers on the map for ' + fid, featureName);
          getCoordinates(fid);
        }
        else {
          // Check if we have locations for the map.
          if (
            'tpps' in Drupal.settings
            && 'tree_info' in Drupal.settings.tpps
          ) {
            // Pages like /tpps/details/TGDR978
            // where no single file used (no fid) but all files together.
          }
          else {
            //dog('Map Wrapper. Review all maps and hide missing.', featureName);
            //jQuery(this).hide();
          }
        }
      });
    }
  }

  /**
   * Debug Log.
   *
   * @param mix $message
   *   Message to be show.
   * @param mfeatureName $featureName
   */
  function dog(message, callerFunctionName = null) {
    if (Drupal.settings.tpps.googleMap.debugMode ?? false) {
      // Get caller function name to use as a prefix.
      // https://stackoverflow.com/a/39337724/1041470
      var e = new Error('dummy');
      if (callerFunctionName === null) {
        callerFunctionName = e.stack
          .split('\n')[2]
          .replace(/^\s+at\s(.+?)(?:\s.*:|:)(.*?):(.*?)\)?$/g, '$1 [$2:$3]');
      }
      else {
        callerFunctionName = callerFunctionName + e.stack
          .split('\n')[2]
          .replace(/^\s+at\s(.+?)(?:\s.*:|:)(.*?):(.*?)\)?$/g, ' [$2:$3]');
      }

      // https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Operators/typeof
      switch (typeof message) {
        case "object":
          console.log(
            '%c' + callerFunctionName + ':\n  %O',
            'color: #73B1FF', // Sky-blue
            message,
          );
          break;

        case "boolean":
        case "undefined":
        case "symbol":
        case "string":
        case "number":
        default:
          console.log(
            '%c' + callerFunctionName + ':\n  %c' + message,
            'color: #73B1FF', // Sky-blue
            'color: #ff9900'  // dark orange
          );
          break;
      }

    }
  }
  window.dog = dog;


  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  /**
   * Gets locations for specific accession-file.
   */
  function getCoordinates(fid = null){

    // @TODO Replace 'jQuery' with '$'.
    dog(fid);
    if (fid == null) {
      return;
    }

    var no_header, id_col, lat_col, long_col;
    let fileMeta;
    let organismFid;
    let commonName;
    let organismId;
    let $fileWrapper;
    let columnName;
    let selectBoxName;
    let value;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
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
      dog('File metadata not found. Get data from form.');

      // Column name are letters.
      // Note: Change 65 to 97 for lowercase.
      // WARNING: Won't work if accession file has more then A-Z columns.
      const alphabets = [...Array(26).keys()].map((n) => String.fromCharCode(65 + n));
      let organismNumber = Drupal.settings.tpps.organismNumber;

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Loop organisms to find accession files.
      for (organismId = 1; organismId <= organismNumber; organismId++) {
        // @TODO Stop Loop when 'fid' was found.
        commonName = 'tree-accession[species-' + organismId + '][file]';
        var $fileIdField = jQuery('input[name="' + commonName + '[fid]"]');
        if ($fileIdField.length == 0) {
          continue;
        }
        organismFid = $fileIdField.val();
        dog('Organism ' + organismId + '; File Id: ' + organismFid);
        if (organismFid == 0) {
          continue;
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
      'tpps' in Drupal.settings
      && 'accession_files' in Drupal.settings.tpps
      && typeof Drupal.settings.tpps.accession_files[fid] != 'undefined'
    ) {
      // File Id was found. Let's check properties.
      if ( !(
           'id_col'   in Drupal.settings.tpps.accession_files[fid]
        && 'lat_col'  in Drupal.settings.tpps.accession_files[fid]
        && 'long_col' in Drupal.settings.tpps.accession_files[fid]
      )) {
        delete Drupal.settings.tpps.accession_files[fid];
        dog('No file metadata in Drupal.settings.tpps.accession_files for FileId: ' + fid);
        //console.table(Drupal.settings.tpps.accession_files[fid]);
        dog('Going to hide the map for File: ' + fid);
        // Since columns set incorrectly we hide the map.
        var $mapWrapper = jQuery('#' + fid + '_map_wrapper');
        // Works!
        $mapWrapper.hide();
        if ($mapWrapper.is(":visible")) {
          dog('Visible');
        } else{
          dog('NOT Visible');
        }
        return;
      }
      // At tpps-admin-panel/TGDR978 we doesn't have 'fid' element.
      // @TODO Add at server side.
      Drupal.settings.tpps.accession_files[fid]['fid'] = fid;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get data from server.
    //
    //
    // @TODO Not work at https://tgwebdev.cam.uchc.edu/tpps-admin-panel/TGDR978
    //
    //
    //console.log(Drupal.settings.tpps.accession_files);
    if (
      'tpps' in Drupal.settings
      && 'accession_files' in Drupal.settings.tpps
      && typeof Drupal.settings.tpps.accession_files[fid] != 'undefined'
    ) {
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
    else {
      // Hide map.
      console.log('Hide map 2');
      //jQuery('#' + fid + '_map_wrapper')[0].hide();
    }
  }
})(jQuery);

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
jQuery.fn.mapButtonsClick = function (fid, organismId) {

  let debugMode = Drupal.settings.tpps.googleMap.debugMode ?? false;
  let featureName = 'jQuery.fn.mapButtonsClick';

  dog('Organism Id: ' + organismId);

  if (!fid) {
    dog('\n\n Called mapButtonClick with fid: ' + fid, featureName);
    return;
  }
  if (debugMode) {
    console.log('==============================================================')
    dog('\n\n Called mapButtonClick with fid: ' + fid, featureName);
  }

  // Since user changed columns don't use previously stored data but get
  // updated data from form in getCoordinates().
  if (typeof fid == 'undefined') {
    // @TODO check if this works when file was removed and new one uploaded.
    delete Drupal.settings.tpps.accession_files;
    dog('Removed accession files columns information: all', featureName);
  }
  else {
    if (
      'tpps' in Drupal.settings
      && 'accession_files' in Drupal.settings.tpps
      && typeof Drupal.settings.tpps.accession_files[fid] != 'undefined'
    ) {
      delete Drupal.settings.tpps.accession_files[fid];
      dog('Removed accession files columns information: ' + fid, featureName);
    }
    else {
      dog(
        'There was no accession files columns information for Fiie Id: ' + fid,
        featureName
      );
    }
  }
  if ('tpps' in Drupal.settings && 'locations' in Drupal.settings.tpps) {
    delete Drupal.settings.tpps.locations;
  }

  dog('call getCoordinates() from mapButtonClick.', featureName);

  getCoordinates(fid);
  initMap();
}



function toBeAddedToInitMap() {

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Checks if managed file field is present on page.
  // Currently does nothing but shows messages.
  var organismNumber = Drupal.settings.tpps.organismNumber ?? 1;
  for (organismId = 1; organismId <= organismNumber; organismId++) {
      //let organismId = $(this).parent('.form-managed-file')
      //  .attr('id').replace('edit-tree-accession-species-', '')
      //  .replace('-file-uploadh', '');

//console.log('Organism Id: ' + organismId);

      // if there is no fileField at page: /tpps-admin-panel/TGDR978
      // Only when form with managed file field used.
      let fileIdFieldName = 'tree-accession[species-' + organismId + '][file][fid]';
dog($('input[name="' + fileIdFieldName + '"]'));

      let fid = $('input[name="' + fileIdFieldName + '"]').attr('value');
dog('FID: ' + fid);

      if (fid) {
        dog('Show this map');
      }
      else {
        dog('Hide this map');
        //let $mapWrapper = jQuery('#' + fid + '_map_wrapper').hide();
      }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
}
