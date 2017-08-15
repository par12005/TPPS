

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
            }
            
            oldNumSpecies = currentNumSpecies;
        }
        
        else if (jQuery('#edit-speciesnumber').val() < oldNumSpecies){
            
            for(var i = oldNumSpecies; i >= currentNumSpecies; i--){
                
                var currentSpeciesForm = jQuery('#genusSpecies' + i);
                
                currentSpeciesForm.hide().attr('enabled', false);
                            
            }
            
            oldNumSpecies = currentNumSpecies;       
        }
        
        else{
            
        }
    });
    
});  