jQuery(document).ready(function ($) {
  
  function Secondary_Authors(){
    var secondary_authors_button_add = jQuery('#edit-publication-secondaryauthors-add');
    var secondary_authors_button_remove = jQuery('#edit-publication-secondaryauthors-remove');
    var secondary_authors = jQuery('#edit-publication-secondaryauthors').children('div').children('div');
    var secondary_authors_check = jQuery('#edit-publication-secondaryauthors-check');
    var secondary_authors_number_field = jQuery(secondary_authors[0]);
    secondary_authors_number_field.hide();
    secondary_authors = secondary_authors.slice(1,31);
    var secondary_authors_number = jQuery('#edit-publication-secondaryauthors-number')[0].value;

    secondary_authors.hide();

    if (secondary_authors_number > 0 && !secondary_authors_check[0].checked){
      for (var i = 0; i < secondary_authors_number; i++){
        jQuery(secondary_authors[i]).show();
      }

      for (var i = secondary_authors_number; i <= 30; i++){
        jQuery(secondary_authors[i]).hide();
      }
    }

    secondary_authors_button_add.attr('type', 'button');
    secondary_authors_button_remove.attr('type', 'button');

    secondary_authors_button_add.on('click', function(){
      if (secondary_authors_number < 30){
        secondary_authors_number++;
        jQuery('#edit-publication-secondaryauthors-number')[0].value = secondary_authors_number;

        for (var i = 0; i < secondary_authors_number; i++){
          jQuery(secondary_authors[i]).show();
        }

        for (var i = secondary_authors_number; i <= 30; i++){
          jQuery(secondary_authors[i]).hide();
        }
      }
    });

    secondary_authors_button_remove.on('click', function(){
      if (secondary_authors_number > 0){
        secondary_authors_number--;
        jQuery('#edit-publication-secondaryauthors-number')[0].value = secondary_authors_number;

        for (var i = 0; i < secondary_authors_number; i++){
          jQuery(secondary_authors[i]).show();
        }

        for (var i = secondary_authors_number; i <= 30; i++){
          jQuery(secondary_authors[i]).hide();
        }
      }
    });
    
    secondary_authors_check.on('click', function(){
      if (secondary_authors_check[0].checked){
        secondary_authors_button_add.hide();
        secondary_authors_button_remove.hide();
        secondary_authors.hide();
      }
      else if (secondary_authors_number >= 0){
        secondary_authors_button_add.show();
        secondary_authors_button_remove.show();
        for (var i = 0; i < secondary_authors_number; i++){
          jQuery(secondary_authors[i]).show();
        }

        for (var i = secondary_authors_number; i <= 30; i++){
          jQuery(secondary_authors[i]).hide();
        }
      }
    });
  }
  
  function Organism(){
    var organism_add = jQuery('#edit-organism-add');
    var organism_remove = jQuery('#edit-organism-remove');
    var number_object = jQuery('#edit-organism div input:hidden');
    var organism_number = number_object[0].value;
    var organisms = jQuery('#edit-organism div div.form-type-textfield');
    
    jQuery('#edit-organism-number').hide();
    organisms.hide();
    
    if (organism_number > 0){
      for (var i = 0; i < organism_number; i++){
        jQuery(organisms[i]).show();
      }

      for (var i = organism_number; i < 5; i++){
        jQuery(organisms[i]).hide();
      }
    }
    
    organism_add.attr('type', 'button');
    organism_remove.attr('type', 'button');

    organism_add.on('click', function(){
      if (organism_number < 5){
        organism_number++;
        number_object[0].value = organism_number;
        
        for (var i = 0; i < organism_number; i++){
          jQuery(organisms[i]).show();
        }

        for (var i = organism_number; i < 5; i++){
          jQuery(organisms[i]).hide();
        }
      }
    });
    
    organism_remove.on('click', function(){
      if (organism_number > 1){
        organism_number--;
        number_object[0].value = organism_number;
        
        for (var i = 0; i < organism_number; i++){
          jQuery(organisms[i]).show();
        }

        for (var i = organism_number; i < 5; i++){
          jQuery(organisms[i]).hide();
        }
      }
    });
    
  }
  
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
    
    if (jQuery("#edit-step")[0].value == 1){
      Secondary_Authors();
      Organism();
    }
    
    if (jQuery("#edit-step")[0].value == 3){
      accessionButtons();
      jQuery("#edit-tree-accession-check").on('click', accessionButtons);
    }
    
    if (jQuery("#edit-step")[0].value === 'summarypage'){
      Supplemental_Files();
      jQuery("#tpps-status").insertAfter(".tgdr_form_status");
      jQuery("#edit-next").on('click', function(){
        jQuery("#tpps-status").html("<label>Loading... </label><br>This step may take several minutes.");
      });
    }
  }
  
  var buttons = jQuery('input').filter(function() { return (this.id.match(/map_button/) || this.id.match(/map-button/)); });
  jQuery.each(buttons, function(){
    jQuery(this).attr('type', 'button')
    jQuery(this).click(getCoordinates);
  });
  jQuery("#map_wrapper").hide();
});

var maps = {};

function accessionButtons(){
  var acc_check = jQuery("#edit-tree-accession-check");
  jQuery('div').filter(function() { return this.id.match(/map_wrapper/); }).hide();
  if (typeof acc_check[0] !== 'undefined' && acc_check[0].checked){
    jQuery("#map_button").hide();
    jQuery.each(jQuery('input').filter(function() { return (this.id.match(/_map_button/)); }), function() {
      jQuery(this).show();
    });
  }
  else {
    jQuery("#map_button").show();
    jQuery.each(jQuery('input').filter(function() { return (this.id.match(/_map_button/)); }), function() {
      jQuery(this).hide();
    });
  }
}

function initMap() {
  var mapElements = jQuery('div').filter(function() { return this.id.match(/map_wrapper/); });
  jQuery.each(mapElements, function(){
    var species_name = this.id.match(/(.*)map_wrapper/)[1];
    maps[species_name] = new google.maps.Map(document.getElementById(species_name + 'map_wrapper'), {
      center: {lat:0, lng:0},
      zoom: 5
    });
    maps[species_name + 'markers'] = [];
    maps[species_name + 'total_lat'];
    maps[species_name + 'total_long'];
  });
}

function clearMarkers(prefix) {
  for (var i = 0; i < maps[prefix + 'markers'].length; i++) {
    maps[prefix + 'markers'][i].setMap(null);
  }
  maps[prefix + 'markers'] = [];
}

function addMarkerWithTimeout(location, time, prefix) {
  window.setTimeout(function() {
    var index = maps[prefix + 'markers'].push(new google.maps.Marker({
      position: new google.maps.LatLng(location[1], location[2]),
      map: maps[prefix],
      animation: google.maps.Animation.DROP,
      title: location[0],
    })) - 1;
    maps[prefix + 'markers'][index].addListener('click', toggleBounce);
  }, time);
}

function toggleBounce(){
  if (this.getAnimation() !== null){
    this.setAnimation(null);
  }
  else {
    this.setAnimation(google.maps.Animation.BOUNCE);
  }
}

function getCoordinates(){
  var species_name;
  if (this.id.match(/(.*)map_button/) !== null){
    species_name = this.id.match(/(.*)map_button/)[1];
  }
  else {
    species_name = "";
  }
  
  var fid, no_header, id_col, lat_col, long_col;
  try{
    if (jQuery("#edit-step")[0].value == 3){
      var species_number;
      if (species_name !== ""){
        species_number = 'species-' + jQuery('div').filter(function() { return this.id.match(new RegExp(species_name + 'species_number')); })[0].innerHTML;
      }
      else {
        species_number = "";
      }
      fid = jQuery('input').filter(function() { return this.name.match(new RegExp('tree-accession\\[?' + species_number + '\\]?\\[file\\]\\[fid\\]')); })[0].value;
      no_header = jQuery('input').filter(function() { return this.name.match(new RegExp('tree-accession\\[?' + species_number + '\\]?\\[file\\]\\[no-header\\]')); })[0].checked;
      var cols = jQuery('select').filter(function() { return this.id.match(new RegExp('edit-tree-accession-?' + species_number + '-file-columns-')); });
      jQuery.each(cols, function(){
        var col_name = this.name.match(new RegExp('tree-accession\\[?' + species_number + '\\]?\\[file\\]\\[columns\\]\\[(.*)\\]'))[1];
        if (jQuery(this)[0].value === "1"){
          id_col = col_name;
        }
        if (jQuery(this)[0].value === "4"){
          lat_col = col_name;
        }
        if (jQuery(this)[0].value === "5"){
          long_col = col_name;
        }
      });
    }
    else {
      fid = jQuery('div').filter(function() { return this.id.match(new RegExp(species_name + 'accession_fid')); })[0].innerHTML;
      no_header = jQuery('div').filter(function() { return this.id.match(new RegExp(species_name + 'accession_no_header')); })[0].innerHTML;
      id_col = jQuery('div').filter(function() { return this.id.match(new RegExp(species_name + 'accession_id_col')); })[0].innerHTML;
      lat_col = jQuery('div').filter(function() { return this.id.match(new RegExp(species_name + 'accession_lat_col')); })[0].innerHTML;
      long_col = jQuery('div').filter(function() { return this.id.match(new RegExp(species_name + 'accession_long_col')); })[0].innerHTML;
    }
    
  }
  catch(err){
    console.log(err);
    return;
  }
  
  if (typeof id_col === 'undefined' || typeof lat_col === 'undefined' || typeof long_col === 'undefined'){
    jQuery("#" + species_name + "map_wrapper").hide();
    return;
  }
  
  var request = jQuery.post('tpps-accession', {
    fid: fid,
    no_header: no_header,
    id_col: id_col,
    lat_col: lat_col,
    long_col: long_col
  });
  
  request.done(function (data) {
    jQuery.fn.updateMap(data, species_name);
  });
}

jQuery.fn.updateMap = function(locations, prefix = "") {
  jQuery("#" + prefix + "map_wrapper").show();
  if (jQuery("#edit-step")[0].value == 3){
    jQuery("#" + prefix + "map_wrapper").css({"height": "450px"});
    jQuery("#" + prefix + "map_wrapper").css({"max-width": "800px"});
  }
  else {
    jQuery("#" + prefix + "map_wrapper").css({"height": "100px"});
  }
  
  clearMarkers(prefix);
  maps[prefix + 'total_lat'] = 0;
  maps[prefix + 'total_long'] = 0;
  timeout = 2000/locations.length;

  for (var i = 0; i < locations.length; i++) {
    maps[prefix + 'total_lat'] += parseInt(locations[i][1]);
    maps[prefix + 'total_long'] += parseInt(locations[i][2]);
    addMarkerWithTimeout(locations[i], timeout*i, prefix);
  }
  var center = new google.maps.LatLng(maps[prefix + 'total_lat']/locations.length, maps[prefix + 'total_long']/locations.length);
  maps[prefix].panTo(center);
};
