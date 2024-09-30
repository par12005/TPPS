// @file
// Manage Google Map and advanced markers.

// Marker used to label map wrapper already processed to avoid double messages,
// AJAX-requests and extra processing.
const TPPS_MAP_WRAPPER_MARKER = 'tpps-google-map-processed';

// We are using array (list) of maps because there could be multiple species
// and multiple accession files on the same page.
// Also there could be a small map (height 100 px) in sidebar.
var maps = {};

Drupal.tpps = Drupal.tpps || {};

// @TODO Be sure to remove class to allow processing managed field again
// on file removement.


/* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
(function ($) {

  async function initMap() {
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
      dog('No locations found to show on map.');
      return;
    }
    else {
      dog('List of locations for markers', locations);
    }

    // Map Id at page.
    let id = fid ?? 'map';

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Map wrapper.
    // Note: There is no fid (File Id) on study details page because map must
    // show all the species at one map (not map per specie).
    var mapWrapperId = fid + '_map_wrapper';
    Drupal.tpps.clearMessages('#' + mapWrapperId);
    var $mapWrapper = $('#' + mapWrapperId);
    if (typeof $mapWrapper[0] == 'undefined') {
      dog('Map wrapper wasn\'t found or outdated. Let\'s create new one.');
      $('.tpps-map-wrapper')
        // Add new DOM-element. Be sure to add map-wrapper at server side even
        // if file not exists yet. Wrapper will be hidden on page load anyway.
        .after( $('<div id="' + mapWrapperId + '" '
          + 'data-fid="' + fid + '" class="tpps-map-wrapper"></div>'
        ))
        // Remove outdated map-wrapper DOM-element to avoid reuse by accident
        // and extra processing of outdated elements.
        .remove();
      // Update pointer to current map-wrapper because previous was removed.
      $mapWrapper = $('#' + mapWrapperId);
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Make map-wrapper visible and set correct height.
    var detail_regex = /tpps\/details\/TGDR.*/g;
    if (
      'tpps' in Drupal.settings && 'stage' in Drupal.settings.tpps
      && Drupal.settings.tpps.stage == 3
    ) {
      // Page 3.
      $mapWrapper.css({'height': '450px', "max-width": "800px"}).show();
    }
    else if($("#tpps_table_display").length) {
      $mapWrapper.css({'height': '450px'}).show();
    }
    else if (window.location.pathname.match(detail_regex)) {
      $mapWrapper.css({'height': '450px'}).show();
    }
    else {
      $mapWrapper.css({'height': '100px'}).show();
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Init Google Map.
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
      let z = google.maps.event.addListener(map, 'zoom_changed', function(event) {
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

      // Feature name will be used for log messages via dog().
      let featureName = 'Drupal.behaviors.tppsGoogleMap';
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Show map on page load if file was uploaded before on Page 3.
      //
      // @TODO Check if page has managed file fields.
      // Loop managed file fields.
      //Drupal.tpps.parseManagedFileFields();
      getColumnsFromManagedFileField();

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Loop map-wrappers.
      // Note: Each time map reloaded by AJAX it will be updated.
      $('.tpps-map-wrapper', context).each(function() {
        if ($(this).hasClass(TPPS_MAP_WRAPPER_MARKER)) {
          dog('Skip processing because Map Wrapper already processed before.', featureName);
          return;
        }
        dog('Unprocessed Map Wrapper found at page.', featureName);
        let fid = $(this).data('fid');
        if (typeof fid != 'undefined' && fid) {
          dog('Update markers on the map for File Id: ' + fid, featureName);
          dog(' > ' + 'getCoordinates(' + fid + ')', featureName);
          Drupal.tpps.getCoordinates(fid);
        }
        else {
          dog('Fid wasn\'t found in map wrapper.', featureName);
          // Check if we have locations for the map.
          if ('tpps' in Drupal.settings && 'tree_info' in Drupal.settings.tpps) {
            // Pages like /tpps/details/TGDR978.
            dog('All the species on the same map.', featureName);
          }
          else {
            dog('Map Wrapper. Review all maps and hide missing.', featureName);
          }
        }
      });
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  /**
   * Gets locations for specific accession-file.
   */
  Drupal.tpps.getCoordinates = function (fid = null) {

    dog('Got File Id: ' + fid);
    if (fid == null) { return; }
    Drupal.settings.tpps = Drupal.settings.tpps || {};
    Drupal.settings.tpps.accession_files = Drupal.settings.tpps.accession_files || {};

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get Columns metadata from managed file field if possible.
    if (typeof Drupal.settings.tpps.accession_files[fid] == 'undefined') {
      dog('Columns Metata not found found under '
        + 'Drupal.settings.tpps.accession_files[' + fid + '].');
      dog('Let\'s get column\'s metadata from managed file field.');
      getColumnsFromManagedFileField(fid);
    }
    else {
      dog('Columns Metata found. No need to parse managed file field.');
    }
    if (typeof Drupal.settings.tpps.accession_files[fid] == 'undefined') {
      dog('No column\'s metadata under Drupal.settings.tpps.accession_files[' + fid + ']');
      return;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Validate Columns metadata.
    dog('Columns Metata for File Id was found. Let\'s validate properties.');
    dog(Drupal.settings.tpps.accession_files[fid]);
    if (
         'id_col'   in Drupal.settings.tpps.accession_files[fid]
      && 'lat_col'  in Drupal.settings.tpps.accession_files[fid]
      && 'long_col' in Drupal.settings.tpps.accession_files[fid]
    ) {
      if (typeof Drupal.settings.tpps.accession_files[fid].fid == 'undefined') {
        // There is no "fid" property which is usual for pages like
        // "/tpps-admin-panel/TGDRxxxx.
        dog('Manually added missing "fid" property');
        // @TODO Minor. Add at server side. Wasn't able to find where to add.
        Drupal.settings.tpps.accession_files[fid]['fid'] = fid;
      }
      else {
        dog('Sub-element "fid" was set before.');
      }
    }
    else {
      delete Drupal.settings.tpps.accession_files[fid];
      dog('No file metadata in Drupal.settings.tpps.accession_files for FileId: ' + fid);
      //console.table(Drupal.settings.tpps.accession_files[fid]);
      dog('Going to hide the map for File: ' + fid);
      // Since columns set incorrectly we hide the map.
      var $mapWrapper = $('#' + fid + '_map_wrapper');
      // Show message.
      var data = {
        "errors": [
          Drupal.t('File is missing or column mapping is wrong.')
        ],
      };
      dog('Add message to Drupal.tpps.showMessages');
      Drupal.tpps.showMessages('#' + fid + '_map_wrapper', data);
      $mapWrapper.hide();
      return;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get data from server.
    //
// @TODO Not work at https://tgwebdev.cam.uchc.edu/tpps-admin-panel/TGDR978
    var request = $.post(
      '/tpps-accession',
      {...Drupal.settings.tpps.accession_files[fid]}
    );
    request.done(function (data) {
      // Store File Id to know which data is stored under
      // Drupal.settings.tpps.locations.
      // @TODO Check if Drupal.settings.tpps.fid is in use.
      Drupal.settings.tpps.fid = fid;
      Drupal.settings.tpps.locations = data;
      initMap();
    });
  }

  /**
   * Gets file's column data from 'managed_file' form field.
   *
   * WARNING: TPPS Form Page 3 ONLY.
   *
   * Loops managed file fields on page to get columns data and fill
   * Drupal.settings.tpps.accession_files[fid] array.
   *
   * @param int fid
   *   Managed File Id.
   */
  function getColumnsFromManagedFileField(fid = null, organismId = null) {
    // @TODO Move under Drupal.tpps.
    let featureName = 'getColumnsFromManagedFileField';
    if (
      fid != null
      && 'tpps' in Drupal.settings
      && 'accession_files' in Drupal.settings.tpps
      && typeof Drupal.settings.tpps.accession_files[fid] != 'undefined'
      && Drupal.settings.tpps.accession_files[fid].length != 0
    ) {
      dog('Column\'s metadata found. Nothing to do.');
      return;
    }
    dog('Column\'s metadata wasn\'t found. Start processing.');

    var no_header, id_col, lat_col, long_col;
    var columnName;
    var organismId;
    var commonName;
    var organismFileId;

    // Column name are letters.
    // Note: Change 65 to 97 for lowercase.
    // WARNING: Won't work if accession file has more then A-Z columns.
    const alphabets = [...Array(26).keys()].map((n) => String.fromCharCode(65 + n));
    const organismNumber = Drupal.settings.tpps.organismNumber;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Loop managed file fields at page to find accession files.
    for (let organismId = 1; organismId <= organismNumber; organismId++) {
      // @TODO Stop Loop when 'fid' was found.
      commonName = 'tree-accession[species-' + organismId + '][file]';
      var $fileIdField = $('input[name="' + commonName + '[fid]"]');
      if ($fileIdField.length == 0) {
        continue;
      }
      dog('Found managed file filed for organism ' + organismId);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Get fid from managed file field.
      var organismFileId = $fileIdField.val();
      var $fileWrapper = $fileIdField.parent('.form-managed-file');
      if (organismFileId == 0) {
        dog('Managed file field has no fid. File wasn\'t yet uploaded or was removed.');
        dog('Let\'s hide map for this File id:' + fid);
        $('#edit-tree-accession-species-' + organismId)
          .find('.tpps-map-wrapper').hide();
        continue;
      }
      if (fid != null && organismFileId != fid) {
        dog('Found Organism fid: ' + organismFileId + ', fid: ' + fid);
        // File Id found in managed file field and fid will be different when
        // new file uploaded so no need to skip processing this file.
        dog('Disabled: Skipped processing because requested different fid');
        //continue;
      }
      dog('Managed file field has fid: ' + organismFileId);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

      // Get column's metadata from field.
      Drupal.settings.tpps['accession_files'] = [];
      Drupal.settings.tpps.accession_files[organismFileId] = [];
      Drupal.settings.tpps.accession_files[organismFileId]['fid'] = organismFileId;
      // @TODO Minor. Get real value of the no-header field.
      Drupal.settings.tpps.accession_files[organismFileId]['no_header'] = null;
      for (columnName of alphabets) {
        var selectBoxName = commonName + '[columns][' + columnName + ']';
        var value = $fileWrapper.find('select[name="' + selectBoxName + '"]')
          .find(":selected").val();

        // @TODO Create name constants for those numbers.
        switch (value) {
          case '1':
            // Plant Identifier.
            Drupal.settings.tpps.accession_files[organismFileId]['id_col'] = columnName;
            break;
          case '4':
            // Latitude
            Drupal.settings.tpps.accession_files[organismFileId]['lat_col'] = columnName;
            break;
          case '5':
            // Longitude
            Drupal.settings.tpps.accession_files[organismFileId]['long_col'] = columnName;
            break;
        }
        // Check if all required fields was set.
        if (
             'id_col'   in Drupal.settings.tpps.accession_files[organismFileId]
          && 'lat_col'  in Drupal.settings.tpps.accession_files[organismFileId]
          && 'long_col' in Drupal.settings.tpps.accession_files[organismFileId]
        ) {
          break;
        }
      }
    }
  }

  /**
   * Loops all managed file fields on Page3 to get columns meta-data.
   *
   * Note: File Id will be obtained from the
   *
   */
  Drupal.tpps.parseManagedFileFields = function() {
    let featureName = 'Drupal.tpps.parseManagedFileFields';
    let organismNumber = Drupal.settings.tpps.organismNumber ?? 1;
    for (let organismId = 1; organismId <= organismNumber; organismId++) {
      let columnTableSelector = '#edit-tree-accession-species-' + organismId
        + '-file-columns';
      var $columnTable = $(columnTableSelector, context);
      if ($columnTable.hasClass(TPPS_MAP_WRAPPER_MARKER)) {
        dog('Skipped processing of managed file field for organism '
          + organismId + ' because it was processed before.', featureName);
        continue;
      }

      if ($columnTable.length) {
        let fid = $('input[name="tree-accession[species-'
          + organismId + '][file][fid]"]').val();
        dog('File was uploaded. Organism: ' + organismId + ', fid: ' + fid, featureName);
        if (typeof fid != 'undefined' && Number(fid) > 0) {
          //getColumnsFromManagedFileField(fid);
          dog(' > ' + 'getCoordinates(' + fid + ')', featureName);
          Drupal.tpps.getCoordinates(fid);
          $columnTable.addClass(TPPS_MAP_WRAPPER_MARKER);
        }
        else {
          dog('No file found. Removed class to allow processing.', featureName);
          $columnTable.removeClass(TPPS_MAP_WRAPPER_MARKER);
        }
      }
    }
  }

})(jQuery);


/**
 * Called when user changes column datatype using dropdown menu.
 *
 * Called from page_3_ajax.php function tpps_accession_pop_group():
 *
 * @param int fid
 *   File Id of changed/updated file.
 * @param int organismId
 *   Organism Id (species id) on page.
 */
jQuery.fn.fileColumnsChange = function (fid, organismId = null) {
  var featureName = 'jQuery.fn.fileColumnsChange';
  dog('Got "fid": ' + fid + ', "organismId": ' + organismId, featureName);
  if (!fid) {
    dog('Empty fid', featureName);
    return;
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
      if (typeof Drupal.settings.tpps.accession_files[fid] == 'undefined') {
        dog('Columns metadata not found.', featureName);
      }
      else {
        delete Drupal.settings.tpps.accession_files[fid];
        dog('Existing columns metadata was removed.', featureName);
      }
    }
  }
  if ('tpps' in Drupal.settings && 'locations' in Drupal.settings.tpps) {
    delete Drupal.settings.tpps.locations;
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  dog('Label map wrapper as already processed.', featureName);
  var mapWrapperId = fid + '_map_wrapper';
  Drupal.tpps.clearMessages('#' + mapWrapperId);
  // WARNING: '$' couldn't be used here instead of 'jQuery'.
  jQuery('#' + mapWrapperId).addClass(TPPS_MAP_WRAPPER_MARKER);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  dog(' > ' + 'getCoordinates(' + fid + ')', featureName);
  Drupal.tpps.getCoordinates(fid);
  dog(' > ' + 'initMap()', featureName);
  initMap();
}
