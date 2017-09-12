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
                customForm.show();                  
              }
              
              else{
                customForm.hide();
              }
              
          });
          
          jQuery('#edit-species' + i).change(function(){
              
              var speciesFormVal = jQuery(this).val();
              var speciesCustomForm = jQuery('#edit-customspecies' + jQuery(this).attr('id').slice('-1'));
            
              if (speciesFormVal == 0){
                  speciesCustomForm.attr.show();
              }
              else{
                  speciesCustomForm.attr.hide();
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
            }
            
            oldNumSpecies = currentNumSpecies;
        }
        
        else if (jQuery('#edit-speciesnumber').val() < oldNumSpecies){
            
            for(var i = oldNumSpecies; i >= currentNumSpecies; i--){
                jQuery('#genusSpecies' + i).hide();
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
        }
        
        else{

            jQuery('#commonGardenIrrigationType').hide();
        }
//        
    });
    
    jQuery('#edit-commongardensalinity').change(function()
    {
        var salinityVal = jQuery(this).val();
        
        if (salinityVal == 1){
            jQuery('#commonGardenSalinityValue').show();         
        }
        else{
            jQuery('#commonGardenSalinityValue').hide();
        }
    });
    
// Growth chamber functions below   

    function hideAndShow(param1, param2){
        jQuery(param1).show();
        jQuery(param2).hide();        
    }

    jQuery('#edit-growthchamberco2').change(function()
    {
        if (jQuery(this).val() == 0){
            hideAndShow('.growthChamberChildrenControlledCO2',
                        '.growthChamberChildrenUncontrolledCO2',
                        );
            
        }
        else if (jQuery(this).val() == 1){
            hideAndShow('.growthChamberChildrenUncontrolledCO2',
                        '.growthChamberChildrenControlledCO2',
                        );
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
                    );
        }
        
        else if (jQuery(this).val() == 1)
        {
         hideAndShow('.growthChamberChildrenAirHumidityUncontrolled',
                    '.growthChamberChildrenAirHumidityControlled',
                    );
        }
        else{
         
        }
    });
        
    jQuery('#edit-growthchamberlightintensity').change(function()
    {
        if (jQuery(this).val() == 0){
            jQuery('.growthChamberChildrenLight').show();
        }
        else if (jQuery(this).val() == 1){
            jQuery('.growthChamberChildrenLight').hide();
                  }
        else{
            
        }
        
    });
        
    jQuery('#edit-growthchambersoil').change(function()
    {
        if (jQuery(this).val() == 0){
            jQuery("[id^=growthChamberChildrenSoil]").show();
        }
        
        else if (jQuery(this).val() == 1){
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
            );            
        }
        
        else if (jQuery(this).val() == 1){
            hideAndShow(
                    '.growthChamberChildrenPHUncontrolled',
                    '.growthChamberChildrenPHControlled',
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
                                        jQuery("[class^='growthChamberChildren']").hide());
                                         
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
                                jQuery('#commonGardenIrrigationType').hide()).then(
                                    jQuery('#commonGardenSalinityValue').hide());
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