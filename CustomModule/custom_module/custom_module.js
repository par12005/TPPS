jQuery(document).ready(function () {
    
//    jQuery('#submitAjaxButton').click(function () {
//
//        alert('You clicked on the ajax button');
//        jQuery.ajax({
//
//            url: Drupal.settings.basePath + 'getHello',
//            data: {
//
//                name: jQuery("#edit-name").val(),
//                token: myToken
//
//            },
//
//            success: function (data) {
//
//                jQuery("#msg-display-area").empty();
//                jQuery("#msg-display-area").append(data);
//                jQuery('#edit-study').hide();
//
//            }
//
//
//        });                                                                                                          
//
//    });

    var secondaryAuthorTitle = '<div id="secondaryAuthorTitle"> Secondary Author </div>';
    
    jQuery(secondaryAuthorTitle).appendTo('#primaryauthor');

    jQuery('#button').appendTo('#secondaryAuthorTitle');

    var numAuthors = 0;
  
    jQuery('#button').click(function(){
       jQuery('#secondaryAuthor' + numAuthors).show();
       numAuthors++;
    });
    
//    Secondary Authors forms are hidden and their corresponding buttons are created
    
    for(var i = 0; i <= 30; i++){
        if (i <= 15){
          jQuery('#genusSpecies' + i).hide();
          jQuery('#secondaryAuthor' + i).hide();
          
          jQuery('#edit-secondaryauthorform' + i).change(function()
          {
              var formVal = jQuery(this).val();
              var customForm = jQuery('#edit-secondaryauthorcustomform' + jQuery(this).attr('id').slice('-1'));
              
              if (formVal == 3){
                  
                customForm.attr('disabled', false);                  
              }
              
              else{
                customForm.attr('disabled', true);
              }
              
          });
          
          jQuery('#edit-species' + i).change(function(){
              
              var speciesFormVal = jQuery(this).val();
              var speciesCustomForm = jQuery('#edit-customspecies' + jQuery(this).attr('id').slice('-1'));
            
              if (speciesFormVal == 0){
                  speciesCustomForm.attr('disabled', false);
              }
              else{
                  speciesCustomForm.attr('disabled', true);
              }
          });
              

          
          jQuery('#button').clone().attr('id', '#removeAuthor' + i).attr('value', 'Remove Author').appendTo('#secondaryAuthor' + i).click(function(){
              
              numAuthors--;              
              jQuery(this.parentElement).hide();        
          });

          }
          
        else{
            jQuery('#secondaryAuthor' + i).hide().addClass('secondaryAuthor');
            
            jQuery('#button').clone().attr('id', '#removeAuthor' + i).appendTo('#secondaryAuthor' + i).click(function(){
                jQuery(this.parentElement).hide();

          });
          
        }
        
    }
    
       
    var oldNumSpecies = jQuery('#edit-speciesnumber').val();
  
    jQuery('#edit-speciesnumber').change(function(){
        var currentNumSpecies = jQuery('#edit-speciesnumber').val();
                    
        if (jQuery('#edit-speciesnumber').val() > oldNumSpecies){
            
            for(var i = oldNumSpecies; i < currentNumSpecies; i++){
                jQuery('#genusSpecies' + i).show();
                jQuery('#edit-genus' + i).attr('disabled', false);
                jQuery('#edit-species').attr('disabled', false);
            }
            
            oldNumSpecies = currentNumSpecies;
        }
        
        else if (jQuery('#edit-speciesnumber').val() < oldNumSpecies){
            
            for(var i = oldNumSpecies; i >= currentNumSpecies; i--){
                
                var currentSpeciesForm = jQuery('#genusSpecies' + i);
                
                currentSpeciesForm.hide();
                            
            }
            
            oldNumSpecies = currentNumSpecies;       
        }
        
        else{
            
        }
    });
   
    jQuery('#growthChamberClass').hide();
    jQuery('#commonGardenClass').hide();
    
    jQuery('#edit-commongardenirrigation').change(function()
    {
        var irrigationVal = jQuery(this).val();
        
        if (irrigationVal == 1){
            
            jQuery('#commonGardenIrrigationType').show();
            jQuery('#edit-commongardenirrigationtype').attr('disabled', false); 
        }
        
        else{

            jQuery('#commonGardenIrrigationType').hide();
            jQuery('#edit-commongardenirrigationtype').attr('disabled', true); 
        }
//        
    });
    
    jQuery('#edit-commongardensalinity').change(function()
    {
        var salinityVal = jQuery(this).val();
        
        if (salinityVal == 1){
            jQuery('#commonGardenSalinityValue').show();
            jQuery('#edit-commongardensalinityvalue').attr('disabled', false);            
        }
        else{
            jQuery('#commonGardenSalinityValue').hide();
            jQuery('#edit-commongardensalinityvalue').attr('disabled', true);
        }
    });
    
    jQuery('#edit-studytype').change(function()
//    there is obviously a much more better and readable way to make jQuery run synchronously
    {
        var studyTypeVal = jQuery('#edit-studytype').val();
            if (studyTypeVal == 1){
                
                function growthChamberInOrder(){
                    var uncontrolledCO2 = jQuery('#edit-growthChamberCO2Uncontrolled');
                    var controlledCO2 = jQuery('#edit-growthChamberCO2Controlled');
                    var uncontrolledAirHumidity = jQuery('#edit-growthChamberAirHumidityUncontrolled');
                    var controlledAirHumidiy = jQuery('#edit-growthChamberAirHumidityControlled');
                    var controlledLightIntensity = jQuery('#edit-growthChamberLightIntensityControlled');
                    var phControlled = jQuery('#edit-growthChamberpHControlled');
                    var phUncontrolled = jQuery('#edit-growthChamberpHUncontrolled');
                    
                    var deferred = jQuery.Deferred(),
                    promise = deferred.promise();
                    promise.then(
                        jQuery('#growthChamberClass').show()).then(
                            jQuery('#commonGardenClass').hide()).then(
                                jQuery("[id^='edit-growthchamber']").attr('disabled', false)).then(
                                    jQuery("[id^='edit-commongarden']").attr('disabled', true)).then(
                                        uncontrolledCO2.hide().attr('disabled', true)).then(
                                            controlledCO2.hide().attr('disabled', true)).then(
                                                uncontrolledAirHumidity.hide().attr('disabled', true)).then(
                                                    controlledAirHumidiy.hide().attr('disabled', true)).then(
                                                        controlledLightIntensity.hide().attr('disabled', true)).then(
                                                            phControlled.hide().attr('disabled', true)).then(
                                                                phUncontrolled.hide().attr('disabled', true));
                    deferred.resolve();
                }
                
                growthChamberInOrder();
                 
            }
            
            else if (studyTypeVal == 3){
                
                function commonGardenInOrder(){
                    var deferred = jQuery.Deferred(),
                    promise = deferred.promise();
                    promise.then(
                        jQuery('#growthChamberClass').hide()).then(
                            jQuery('#commonGardenClass').show()).then(
                                jQuery("[id^='edit-growthchamber']").attr('disabled', true)).then(
                                    jQuery("[id^='edit-commongarden']").attr('disabled', false)).then(
                                         jQuery('#commonGardenIrrigationType').hide()).then(
                                            jQuery('#edit-commongardenirrigationtype').attr('disabled', true)).then(
                                                jQuery('#commonGardenSalinityValue').hide()).then(
                                                    jQuery('#edit-commongardensalinityvalue').attr('disabled', true));
                    deferred.resolve();
                }
                
                commonGardenInOrder();
                
            }
            
            else{
                 jQuery('#growthChamberClass').hide();
                 jQuery("[id^='edit-growthchamber']").attr('disabled', true);
                 
                 jQuery('#commonGardenClass').hide();
                 jQuery("[id^='edit-commongarden']").attr('disabled', true);
            }
            
    });
      
});