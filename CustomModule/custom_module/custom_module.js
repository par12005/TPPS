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
              
              if (formVal == 0){
                  
                customForm.attr('disabled', false).show();                  
              }
              
              else{
                customForm.attr('disabled', true).hide();
              }
              
          });
          
          jQuery('#edit-species' + i).change(function(){
              
              var speciesFormVal = jQuery(this).val();
              var speciesCustomForm = jQuery('#edit-customspecies' + jQuery(this).attr('id').slice('-1'));
            
              if (speciesFormVal == 0){
                  speciesCustomForm.attr('disabled', false).hide();
              }
              else{
                  speciesCustomForm.attr('disabled', true).hide();
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
                jQuery('#edit-species' + i).attr('disabled', false);
            }
            
            oldNumSpecies = currentNumSpecies;
        }
        
        else if (jQuery('#edit-speciesnumber').val() < oldNumSpecies){
            
            for(var i = oldNumSpecies; i >= currentNumSpecies; i--){
                jQuery('#genusSpecies' + i).hide();
                jQuery('#edit-genus' + i).attr('disabled', true);
                jQuery('#edit-species' + i).attr('disabled', true);
      
            }
            
            oldNumSpecies = currentNumSpecies;       
        }
        
        else{
            
        }
    });
   
    jQuery('#growthChamberClass').hide();
    jQuery('#commonGardenClass').hide();
    
//    Common garden funcitons below. The follow code is extremely redundant and should be refactored.
    
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
    
// Growth chamber functions below

    function hideAndShow(param1, param2, param3, param4){
        jQuery(param1).show();
        jQuery(param2).hide();
        jQuery(param3).attr('disabled', false);
        jQuery(param4).attr('disabled', true);
        
    }

    jQuery('#edit-growthchamberco2').change(function()
    {
        if (jQuery(this).val() == 0){
            hideAndShow('.growthChamberChildrenControlledCO2',
                        '.growthChamberChildrenUncontrolledCO2',
                        '#edit-growthchamber-co2controlled',
                        '#edit-growthchamber-co2uncontrolled'
                        );
            
        }
        else if (jQuery(this).val() == 1){
            hideAndShow('.growthChamberChildrenUncontrolledCO2',
                        '.growthChamberChildrenControlledCO2',
                        '#edit-growthchamber-co2uncontrolled',
                        '#edit-growthchamber-co2controlled');
        }
        else {
        }
    });
    
    jQuery('#edit-growthchamberairhumidity').change(function()
    {
        
        if (jQuery(this).val() == 0)
        {
        hideAndShow('.growthChamberChildrenAirHumidityControlled',
                    '.growthChamberChildrenAirHumidityUncontrolled',
                    '#edit-growthchamber-airhumiditycontrolled',
                    '#edit-growthchamber-airhumidityuncontrolled'
                    );
        }
        
        else if (jQuery(this).val() == 1)
        {
         hideAndShow('.growthChamberChildrenAirHumidityUncontrolled',
                    '.growthChamberChildrenAirHumidityControlled',
                    '#edit-growthchamber-airhumidityuncontrolled',
                    '#edit-growthchamber-airhumiditycontrolled'
                    );
        }
        else{
         
        }
    });
        
    jQuery('#edit-growthchamberlightintensity').change(function()
    {
        if (jQuery(this).val() == 0){
            jQuery('.growthChamberChildrenLight').show();
            jQuery('#edit-growthchamber-lightintensitycontrolled').attr('dsiabled', false);  
        }
        else if (jQuery(this).val() == 1){
            jQuery('.growthChamberChildrenLight').hide();
            jQuery('#edit-growthchamber-lightintensitycontrolled').attr('dsiabled', true);
            
        }
        else{
            
        }
        
    });
        
    jQuery('#edit-growthchambersoil').change(function()
    {
        if (jQuery(this).val() == 0){
            jQuery("[id^='edit-growthchamber-soil']").attr('disabled', false);
            jQuery("[id^=growthChamberChildrenSoil]").show();
        }
        
        else if (jQuery(this).val() == 1){
            jQuery("[id^='edit-growthchamber-soil']").attr('disabled', true);
            jQuery("[id^=growthChamberChildrenSoil]").hide();
            
        }
        
        else{
            
        }
        
    }   
            );
    
    jQuery('#edit-growthchamberph').change(function()
    {
        if (jQuery(this).val() == 0){
            console.log('current function was triggered');
            hideAndShow(
                    '.growthChamberChildrenPHControlled',
                    '.growthChamberChildrenPHUncontrolled',
                    '#edit-growthchamber-phcontrolled',
                    '#edit-growthchamber-phuncontrolled',
                    );            
        }
        
        else if (jQuery(this).val() == 1){
            hideAndShow(
                    '.growthChamberChildrenPHUncontrolled',
                    '.growthChamberChildrenPHControlled',
                    '#edit-growthchamber-phuncontrolled',
                    '#edit-growthchamber-phcontrolled',
                    );
        }
        
        else{
            
        }
    });
    
//    there is obviously a much better and more readable way to do this

    jQuery('#edit-studytype').change(function()
    {
        var studyTypeVal = jQuery('#edit-studytype').val();
            if (studyTypeVal == 1){
                jQuery("#commonGardenClass").attr('disabled', true).hide();

                function growthChamberInOrder(){
                    
                    var deferred = jQuery.Deferred(),
                    promise = deferred.promise();
                    promise.then(jQuery('#growthChamberClass').show()).then(
                                    jQuery("[id^='edit-growthchamber']").show()).then(
                                        jQuery("[id^='edit-growthchamber']").attr('disabled', false)).then(                                   
                                            jQuery("[class^='growthChamberChildren']").hide()).then(
                                                jQuery("[id^='edit-growthchamber-']").attr('disabled', true));
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