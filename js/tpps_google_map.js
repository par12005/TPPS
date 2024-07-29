/**
 * @file
 *
 * TPPS Google Map.
 * Usage: tpps_add_css_js('google_map', $form);
 */
(function($, Drupal) {
  Drupal.behaviors.tppsGoogleMap = {
    attach:function (context) {
      // @TODO Add buttons.

    }
  }
  $(document).ready(function() {



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

    jQuery.fn.mapButtonsClick = function (selector) {
      jQuery(selector).off('click');
      jQuery(selector).click(getCoordinates);
      initMap();
    }


    var maps = {};

    function initMap() {
      var detail_regex = /tpps\/details\/TGDR.*/g;

      if (typeof Drupal.settings.tpps !== 'undefined'
        && typeof Drupal.settings.tpps.map_buttons !== 'undefined'
      ) {
        var mapButtons = Drupal.settings.tpps.map_buttons;
        jQuery.each(mapButtons, function() {
          var fid = this.fid;
          maps[fid] = new google.maps.Map(document.getElementById(this.wrapper), {
            center: {lat:0, lng:0},
            zoom: 5,
          });
          maps[fid + '_markers'] = [];
          maps[fid + '_total_lat'];
          maps[fid + '_total_long'];
        });
      }
      if (
        typeof Drupal.settings.tpps != 'undefined'
        && typeof Drupal.settings.tpps.tree_info != 'undefined'
        && (
          window.location.pathname.match(detail_regex)
          || typeof Drupal.settings.tpps.study_locations !== 'undefined'
        )
      ) {
        maps[''] = new google.maps.Map(document.getElementById('_map_wrapper'), {
          center: {lat:0, lng:0},
          zoom: 5,
        });
        maps['_markers'] = [];
        maps['_total_lat'];
        maps['_total_long'];
        jQuery.fn.updateMap(Drupal.settings.tpps.tree_info);
      }
    }

    function clearMarkers(prefix) {

      for (var i = 0; i < maps[prefix + '_markers'].length; i++) {
        maps[prefix + '_markers'][i].setMap(null);
      }
      maps[prefix + '_markers'] = [];
    }

    function getCoordinates(){
      let fid = this.id.match(/(.*)_map_button/)[1];
      let file;
      try {
        file = Drupal.settings.tpps.accession_files[fid];
        if (typeof file === 'undefined') {
          // Why?!
          jQuery.each(Drupal.settings.tpps.accession_files, function() {
            if (this.fid == fid) {
              file = this;
            }
          })
          if (typeof file === 'undefined') {
            return;
          }
        }
      }
      catch(err){
        console.log(err);
        return;
      }

      if ( typeof file.id_col === 'undefined'
        || typeof file.lat_col === 'undefined'
        || typeof file.long_col === 'undefined'
      ) {
        jQuery('#' + Drupal.settings.tpps.map_buttons[fid].wrapper).hide();
        return;
      }

      var request = jQuery.post('/tpps-accession', file);
      request.done(function (data) {
        jQuery.fn.updateMap(data, fid);
      });
    }

    jQuery.fn.updateMap = function(locations, fid = "") {
      let $wrapper = jQuery("#" + fid + "_map_wrapper");
      $wrapper.show();
      let detail_regex = /tpps\/details\/TGDR.*/g;
      if (
        typeof Drupal.settings.tpps !== 'undefined'
        && typeof Drupal.settings.tpps.stage !== 'undefined'
        && Drupal.settings.tpps.stage == 3
      ) {
        $wrapper.css({"height": "450px"});
        $wrapper.css({"max-width": "800px"});
      }
      else if(jQuery("#tpps_table_display").length > 0) {
        $wrapper.css({"height": "450px"});
      }
      else if (window.location.pathname.match(detail_regex)) {
        $wrapper.css({"height": "450px"});
      }
      else {
        $wrapper.css({"height": "100px"});
      }

      clearMarkers(fid);
      maps[fid + '_total_lat'] = 0;
      maps[fid + '_total_long'] = 0;
      timeout = 2000/locations.length;

      maps[fid + '_markers'] = locations.map(function (location, i) {
        maps[fid + '_total_lat'] += parseInt(location[1]);
        maps[fid + '_total_long'] += parseInt(location[2]);
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

      maps[fid + '_cluster'] = new markerClusterer.MarkerClusterer(
        maps[fid],
        maps[fid + '_markers'],
        {imagePath: 'https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/m'}
      );

      var center = new google.maps.LatLng(
        maps[fid + '_total_lat']/locations.length,
        maps[fid + '_total_long']/locations.length
      );
      maps[fid].panTo(center);
    };

  });
})(jQuery, Drupal);
