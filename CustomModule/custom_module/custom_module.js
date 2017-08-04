

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

    jQuery('#button').insertAfter('#edit-primaryauthor');

    var numAuthors = 0;
    
    jQuery('#button').click(function(){
       jQuery('#secondaryAuthor' + numAuthors).show();
       numAuthors++;
    });
    
//    Secondary Authors forms are hidden and their corresponding buttons are created
   

    for(var i = 0; i <= 30; i++){
        if (i <= 10){
          jQuery('#edit-species' + i).hide();
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
                customForm.attr('val', '');
              }
              
          });
              

          
          jQuery('#button').clone().attr('id', '#removeAuthor' + i).attr('value', 'Remove Author').appendTo('#secondaryAuthor' + i).click(function(){
              
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
    
//    Below function doesn't always show and hide the correct number of forms.

    jQuery('#edit-speciesnumber').change(function(){
        var currentNumSpecies = jQuery('#edit-speciesnumber').val();
                    
        if (jQuery('#edit-speciesnumber').val() > oldNumSpecies){
            
            for(var i = oldNumSpecies; i < currentNumSpecies; i++){
                jQuery('#edit-species' + i).show();            
            }
            
            oldNumSpecies = currentNumSpecies;
        }
        
        else if (jQuery('#edit-speciesnumber').val() < oldNumSpecies){
            
            for(var i = oldNumSpecies; i >= currentNumSpecies; i--){
                
                var currentSpeciesForm = jQuery('#edit-species' + i);
                
                currentSpeciesForm.hide().attr('enabled', false);
                            
            }
            
            oldNumSpecies = currentNumSpecies;       
        }
        
        else{
            
        }
    });
    
});  