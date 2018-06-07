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
        var organism_number = jQuery('#edit-organism-number')[0].value;
        var organisms = jQuery('#edit-organism').children('div').children('fieldset');
        
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
                jQuery('#edit-organism-number')[0].value = organism_number;
                
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
                jQuery('#edit-organism-number')[0].value = organism_number;
                
                for (var i = 0; i < organism_number; i++){
                    jQuery(organisms[i]).show();
                }

                for (var i = organism_number; i < 5; i++){
                    jQuery(organisms[i]).hide();
                }
            }
        });
        
    }
    
    function Phenotype(organism_number){
        
        var phenotype_button_add = jQuery("#edit-organism-" + organism_number + "-phenotype-add");
        var phenotype_button_remove = jQuery("#edit-organism-" + organism_number + "-phenotype-remove");
        var phenotype_number = jQuery("#edit-organism-" + organism_number + "-phenotype-number")[0].value;
        var phenotype_check = jQuery("#edit-organism-" + organism_number + "-phenotype-check");
        var phenotypes = jQuery("fieldset").filter(function(){ return this.id.match(organism_number + "-phenotype-[0-9]*$"); });
        
        phenotype_button_add.attr('type', 'button');
        phenotype_button_remove.attr('type', 'button');
        
        phenotypes.hide();
        jQuery("#edit-organism-" + organism_number + "-phenotype-number").hide(); //hide phenotype number field
        
        if (phenotype_number >= 1 && !phenotype_check[0].checked){
            for(var i = 0; i < phenotype_number; i++){
                jQuery(phenotypes[i]).show();
            }
            for(var i = phenotype_number; i <= 20; i++){
                jQuery(phenotypes[i]).hide();
            }
        }
        
        phenotype_button_add.on('click', function(){
            if (phenotype_number < 20){
                phenotype_number++;
                jQuery("#edit-organism-" + organism_number + "-phenotype-number")[0].value = phenotype_number;
                
                for(var i = 0; i < phenotype_number; i++){
                    jQuery(phenotypes[i]).show();
                }
                for(var i = phenotype_number; i <= 20; i++){
                    jQuery(phenotypes[i]).hide();
                }
            }
        });
        
        phenotype_button_remove.on('click', function(){
            if (phenotype_number > 1){
                phenotype_number--;
                jQuery("#edit-organism-" + organism_number + "-phenotype-number")[0].value = phenotype_number;
                
                for(var i = 0; i < phenotype_number; i++){
                    jQuery(phenotypes[i]).show();
                }
                for(var i = phenotype_number; i <= 20; i++){
                    jQuery(phenotypes[i]).hide();
                }
            }
        });
        
        phenotype_check.on('click', function(){
            console.log('test');
            if (phenotype_check[0].checked){
                phenotype_button_add.hide();
                phenotype_button_remove.hide();
                phenotypes.hide();
            }
            else{
                phenotype_button_add.show();
                phenotype_button_remove.show();
                for(var i = 0; i < phenotype_number; i++){
                    jQuery(phenotypes[i]).show();
                }
                for(var i = phenotype_number; i <= 20; i++){
                    jQuery(phenotypes[i]).hide();
                }
            }
        });
        
        /*console.log(phenotype_button_add);
        console.log(phenotype_button_remove);
        console.log(phenotypes);
        console.log(phenotype_number);*/
        
    }
    
    jQuery("#edit-step").hide();
    
    if (jQuery("#edit-step").length > 0){
        jQuery(".tpps-status-block").prependTo(".region-sidebar-second");

        jQuery("#progress").css('font-size', '1.5rem');
        jQuery("#progress").css('margin-bottom', '30px');
        
        if (jQuery("#edit-step")[0].value === 'Hellopage'){
            Secondary_Authors();
            Organism();
        }
        else if (jQuery("#edit-step")[0].value === 'fourthPage'){
            var number_of_organisms = jQuery("fieldset").filter(function(){ return this.id.match(/edit-organism-.$/); }).length;
            var phenotypes = jQuery("fieldset").filter(function(){ return this.id.match(/edit-organism-.-phenotype/);});
            var genotypes = jQuery("fieldset").filter(function(){ return this.id.match(/edit-organism-.-genotype/);});

            for(var i = 1; i <= number_of_organisms; i++){
                if (phenotypes.length !== 0){
                    Phenotype(i);
                }
            }
        }
        
        if (jQuery("#edit-step")[0].value === 'summarypage'){
            jQuery("#tpps-status").insertAfter(".tgdr_form_status");
        }
    }
    
    var first = true;
    
    jQuery("#map_wrapper").hide();
    jQuery("#edit-tree-accession-map-button").trigger('mousedown');
    
    $.fn.updateMap = function(locations) {
        jQuery("#map_wrapper").show();
        var map = initMap();
        var total_lat = 0;
        var total_long = 0;
        global_locations = locations;

        for (i = 0; i < locations.length; i++) {
            total_lat += parseInt(locations[i][1]);
            total_long += parseInt(locations[i][2]);
            marker = new google.maps.Marker({
                position: new google.maps.LatLng(locations[i][1], locations[i][2]),
                map: map
            });
        }
        
        var center = new google.maps.LatLng(total_lat/locations.length, total_long/locations.length);
        map.panTo(center);
    };
});

function initMap() {
  var map = new google.maps.Map(document.getElementById('map_wrapper'), {
    center: {lat: 0, lng: 0},
    zoom: 5
  });
  
  return map;
}
