jQuery(document).ready(function ($) {
  function Supplemental_Files(){
    var files_add = jQuery('#edit-files-add');
    var files_remove = jQuery('#edit-files-remove');
    var number_object = jQuery('#edit-files div input:hidden');
    var files_number = number_object[0].value;
    var files = jQuery('#edit-files div div.form-type-managed-file');
    
    files.hide();
    
    if (files_number > 0){
      for (var i = 0; i < files_number; i++){
        jQuery(files[i]).show();
      }

      for (var i = files_number; i < 10; i++){
        jQuery(files[i]).hide();
      }
    }
    
    files_add.attr('type', 'button');
    files_remove.attr('type', 'button');

    files_add.on('click', function(){
      if (files_number < 10){
        files_number++;
        number_object[0].value = files_number;
        
        for (var i = 0; i < files_number; i++){
          jQuery(files[i]).show();
        }

        for (var i = files_number; i < 10; i++){
          jQuery(files[i]).hide();
        }
      }
    });
    
    files_remove.on('click', function(){
      if (files_number > 0){
        files_number--;
        number_object[0].value = files_number;
        
        for (var i = 0; i < files_number; i++){
          jQuery(files[i]).show();
        }

        for (var i = files_number; i < 10; i++){
          jQuery(files[i]).hide();
        }
      }
    });
    
  }
  
  jQuery("#edit-step").hide();

  if (jQuery("#edit-step").length > 0){
    var status_block = jQuery(".tpps-status-block");
    jQuery(".region-sidebar-second").empty();
    status_block.prependTo(".region-sidebar-second");

    jQuery("#progress").css('font-size', '1.5rem');
    jQuery("#progress").css('margin-bottom', '30px');

    if (jQuery("#edit-step")[0].value === 'summarypage'){
      Supplemental_Files();
      jQuery("#tpps-status").insertAfter(".tgdr_form_status");
      jQuery("#edit-next").on('click', function(){
        jQuery("#tpps-status").html("<label>Loading... </label><br>This step may take several minutes.");
      });
    }
  }

  if (typeof Drupal.settings.tpps !== 'undefined' && typeof Drupal.settings.tpps.map_buttons !== 'undefined') {
    var map_buttons = Drupal.settings.tpps.map_buttons;
    jQuery.each(map_buttons, function() {
      jQuery('#' + this.button).click(getCoordinates);
    })
  }

  var buttons = jQuery('input').filter(function() { return (this.id.match(/map_button/) || this.id.match(/map-button/)); });
  jQuery.each(buttons, function(){
    jQuery(this).attr('type', 'button')
  });

  var detail_regex = /tpps\/details\/TGDR.*/g;
  if (!window.location.pathname.match(detail_regex)) {
    jQuery("#map_wrapper").hide();
  }

  var preview_record_buttons = jQuery('input').filter(function() { return this.id.match(/-tripal-eutils-callback/); });
  jQuery.each(preview_record_buttons, function() {
    jQuery(this).attr('type', 'button');
  });

  var preview_buttons = jQuery('input.preview_button');
  jQuery.each(preview_buttons, function() {
    jQuery(this).attr('type', 'button');
    jQuery(this).click(function() {
      previewFile(this, 3);
    });
  });

  var preview_full_buttons = jQuery('input.preview_full_button');
  jQuery.each(preview_full_buttons, function() {
    jQuery(this).attr('type', 'button');
    jQuery(this).click(function() {
      previewFile(this, 0);
    });
  });

  var pending_jobs = {};
  function checkVCFJob(jid) {
    var accession = jQuery(':input[name="accession"]')[0].value;
    var request = jQuery.get('/tpps/' + accession + '/pre-validate/' + jid + '/status');
    request.done(function(job) {
      if (job === null || typeof job !== 'object' || !job.hasOwnProperty('status')) {
        jQuery('#pre-validate-message').html("<div class=\"alert alert-block alert-danger messages error\">There was a problem checking the status of job with id " + jid + "</div>");
      }
      // If this job is completed, check all the other jobs.
      else if (job.status === "Completed") {
        // This job is no longer pending.
        pending_jobs[jid] = false;
        var jobs_complete = true;
        // Iterate through pending jobs.
        for (jid in pending_jobs) {
          if (pending_jobs[jid]) {
            jobs_complete = false;
          }
        }
        // If no jobs are pending, enable the submit button.
        if (jobs_complete) {
          jQuery('#edit-pre-validate')[0].disabled = false;
          console.log('jobs completed!');
          var message = "<div class=\"alert alert-block alert-success messages status\">VCF File pre-validation completed!</div>";
          var errors = job.val_errors;
          if (errors.length > 0) {
            for (error of errors) {
              message = message + "<div class=\"alert alert-block alert-danger messages error\">" + error + "</div>";
            }
          }
          else {
            jQuery('#edit-submit')[0].disabled = false;
          }
          jQuery('#pre-validate-message').html(message);
        }
        // Otherwise, log the pending jobs.
        else {
          console.log(pending_jobs);
        }
      }
      // Otherwise, wait 5 seconds and check again.
      else {
        console.log('job ' + jid + ' status: ' + job.status);
        setTimeout(() => {
          checkVCFJob(job.job_id);
        }, 5000);
      }
    });
  }

  jQuery('#edit-pre-validate').attr('type', 'button');
  jQuery('#edit-pre-validate').click(function() {
    var accession = jQuery(':input[name="accession"]')[0].value;
    console.log('initializing jobs...');

    // Get array of vcf fids.
    var fids = jQuery('input').filter(function () { return this.name.match(/organism-[0-9]+\[genotype\]\[files\]\[vcf\]\[fid\]/); });
    var missing_vcf = false;
    vcfs = {};
    jQuery.each(fids, function() {
      if (this.value === "0") {
        missing_vcf = true;
      }
      var org_num = this.name.match(/organism-([0-9]+)\[genotype\]\[files\]\[vcf\]\[fid\]/)[1];
      vcfs[org_num] = this.value;
    });

    if (!missing_vcf) {
      jQuery('#edit-submit')[0].disabled = true;
      jQuery('#edit-pre-validate')[0].disabled = true;
      jQuery.makeArray(vcfs).map((element) => { return element.value; });

      // Initialize vcf jobs and begin checkVCFJob routines.
      var request = jQuery.get('/tpps/' + accession + '/pre-validate', {"vcfs": vcfs});
      request.done(function(jobs) {
        if (typeof jobs === 'string') {
          jQuery('#pre-validate-message').html("<div class=\"alert alert-block alert-danger messages error\">" + jobs + "</div>");
        }
        else if (!Array.isArray(jobs) || jobs.length == 0) {
          jQuery('#pre-validate-message').html("<div class=\"alert alert-block alert-danger messages error\">There was a problem with pre-validating your VCF files. Please reload the page and try again</div>");
        }
        else {
          console.log('jobs initialized!');
          console.log(jobs);
          for (job of jobs) {
            checkVCFJob(job);
            pending_jobs[job] = true;
          }
          jQuery('#pre-validate-message').html('<img src="/misc/throbber-active.gif"> Pre-validating VCF files. This may take some time...');
        }
      });
    }
    else {
      jQuery('#pre-validate-message').html("<div class=\"alert alert-block alert-danger messages error\">Please upload your VCF file before clicking the pre-validate button</div>");
    }
  });

  jQuery('#edit-save-comments').attr('type', 'button');

  var details_tabs = jQuery('.nav-tabs > .nav-item > .nav-link');
  jQuery.each(details_tabs, function() {
    jQuery(this).click(detailsTab);
  });
  jQuery('[href="#species"]').trigger('click');
  jQuery('a:contains("Return to TPPS List")').hide();
  jQuery('#edit-details-search').attr('type', 'button');
  jQuery('#edit-details-search').click(detailSearch);
  jQuery('#tpps-details-search').submit(function() {
    detailSearch();
    return false;
  });

  var file_options = jQuery('div').filter(function() { return this.id.match(/^file_.*_options$/); });
  jQuery.each(file_options, function() {
    jQuery(this).prependTo('#' + this.id + '_dest');
  });

  initDetailPages();

  var admin_panel_regex = /tpps-admin-panel\/TGDR.*/g;
  if (window.location.pathname.match(admin_panel_regex)) {
    var tag_buttons = jQuery('span').filter(function() { return (this.id.match(/TGDR[0-9]+-tag-[0-9]*-add|remove/) && !this.disabled); });
    jQuery.each(tag_buttons, function() {
      jQuery(this).click(function() {
        var tag_button = this;
        var info = this.id.match(/(TGDR[0-9]+)-tag-([0-9]*)-(add|remove)/);
        if (info.length == 4) {
          var request = jQuery.get('/tpps-tag/' + info[3] + '/' + info[1] + '/' + info[2]);
          request.done(function (data) {
            if (info[3] == 'remove') {
              jQuery(tag_button).parent().hide();
              var pattern = new RegExp('TGDR[0-9]+-tag-' + info[2] + '-add');
              jQuery('span').filter(function() { return this.id.match(pattern); }).show();
            }
            else {
              jQuery(tag_button).hide();
              var pattern = new RegExp('TGDR[0-9]+-tag-' + info[2] + '-remove');
              jQuery('span.tag-close').filter(function() { return this.id.match(pattern); }).parent().show();
            }
          });
        }
        else {
          console.log('Could not parse accession/tag id/edit type from button id');
          console.log(info);
        }
      });
    });

    var add_tags = jQuery('span').filter(function() { return this.id.match(/TGDR[0-9]+-add-tags/); });
    jQuery.each(add_tags, function() {
      jQuery(this).click(function() {
        console.log(this);
      })
    })
  }
});

jQuery.fn.mapButtonsClick = function (selector) {
  jQuery(selector).off('click');
  jQuery(selector).click(getCoordinates);
  initMap();
}

function previewFile(element, num_rows = 3) {
  var fid;
  if (element.id.match(/fid_(.*)/) !== null) {
    fid = element.id.match(/fid_(.*)/)[1];

    var request = jQuery.post('/tpps-preview-file', {
      fid: fid,
      rows: num_rows
    });

    request.done(function (data) {
      if (jQuery('.preview_' + fid).length === 0) {
        jQuery('#fid_' + fid).before(data);
      }
    });
  }
  else {
    return;
  }
}

var detail_pages = {
  "trees": 0,
  "phenotype": 0,
  "genotype": 0,
  "environment": 0,
};

function detailsTab() {
  var clicked_tab = jQuery(this)[0];
  var path = clicked_tab.pathname;
  var detail_type = clicked_tab.hash.substr(1);
  var page = detail_pages[detail_type];
  if (clicked_tab.hash.match(/#(.*):(.*)/) !== null) {
    detail_type = clicked_tab.hash.match(/#(.*):(.*)/)[1];
    page = clicked_tab.hash.match(/#(.*):(.*)/)[2];
    detail_pages[detail_type] = page;
  }
  else {
    if (jQuery('#' + detail_type)[0].innerHTML !== "") {
      // If we aren't loading a new page and we already have data for this tab,
      // then we don't need to change any of the HTML.
      return;
    }
  }
  jQuery('#' + detail_type)[0].innerHTML = "Loading...";

  var request = jQuery.post(path + '/' + detail_type, {
    page: page
  });

  request.done(function (data) {
    jQuery('#' + detail_type)[0].innerHTML = data;
    var details_pagers = jQuery('#' + detail_type + ' > div > ul');
    if (details_pagers.length > 0) {
      var pages = jQuery('#' + detail_type + ' > div > ul > li > a');
      jQuery.each(pages, function() {
        var page = 0;
        if (this.search.match(/\?page=(.*)/) !== null) {
          page = this.search.match(/\?page=(.*)/)[1];
        }
        this.href = '#' + detail_type + ':' + page;
        jQuery(this).click(detailsTab);
      });
    }
  });
}

function initDetailPages() {
  var details_pages = jQuery('#tpps-details-table > div > ul > li > a');
  if (details_pages.length > 0) {
    jQuery.each(details_pages, function() {
      var page = 0;
        if (this.search.match(/\?page=(.*)/) !== null) {
          page = this.search.match(/\?page=(.*)/)[1];
        }
      this.href = '#top:' + page;
      jQuery(this).click(detailSearch);
    });
  }
}

function detailSearch() {
  var path = '/tpps/details/top';
  var page = 0;
  if (this.hash != null && this.hash.match(/#.*:(.*)/) != null) {
    page = this.hash.match(/#.*:(.*)/)[1];
  }

  jQuery('#tpps-details-table')[0].innerHTML = 'Loading...';
  var request = jQuery.post(path, {
    type: jQuery('select').filter(function() { return this.id.match(/edit-details-type/); })[0].value,
    value: jQuery('input').filter(function() { return this.id.match(/edit-details-value/); })[0].value,
    op: jQuery('select').filter(function() { return this.id.match(/edit-details-op/); })[0].value,
    page: page
  });

  request.done(function (data) {
    jQuery('#tpps-details-table')[0].innerHTML = data;
    initDetailPages();
  });
}

var maps = {};

function initMap() {
  var detail_regex = /tpps\/details\/TGDR.*/g;

  if (typeof Drupal.settings.tpps !== 'undefined' && typeof Drupal.settings.tpps.map_buttons !== 'undefined') {
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
  if (window.location.pathname.match(detail_regex) || (typeof Drupal.settings.tpps.study_locations !== 'undefined')) {
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
    jQuery.fn.updateMap(data, fid);
  });
}

jQuery.fn.updateMap = function(locations, fid = "") {
  jQuery("#" + fid + "_map_wrapper").show();
  var detail_regex = /tpps\/details\/TGDR.*/g;
  if (jQuery("#edit-step").length > 0 && jQuery("#edit-step")[0].value == 3){
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

  maps[fid + '_cluster'] = new MarkerClusterer(maps[fid], maps[fid + '_markers'], {imagePath: 'https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/m'});

  var center = new google.maps.LatLng(maps[fid + '_total_lat']/locations.length, maps[fid + '_total_long']/locations.length);
  maps[fid].panTo(center);
};
