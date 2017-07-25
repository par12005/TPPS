

jQuery(document).ready(function () {

    jQuery('.form-item-commonGardenSelect').addClass('commonGardenClass');
    jQuery('.form-item-plantationSelect').addClass('plantationClass');
    jQuery('.form-item-natPopSelect').addClass('natPopClass');
    jQuery('#edit-species').clone().appendTo('#edit-species');
    
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
    
    var numAuthors = 1;

    jQuery('#AddAuthor').click(function()
        {
            var newAuthorForm = jQuery('#edit-secondaryauthor').clone().attr('id', '#edit-secondaryauthor' + numAuthors);
            
            newAuthorForm.insertAfter('#edit-secondaryauthor');
   
            var newButton = jQuery('#AddAuthor').clone().attr('id', '#AddAuthor' + numAuthors);
            
            newButton.click(function(){
                jQuery(newAuthorForm).remove();                
                jQuery(newButton).remove();   
            });
            
            newButton.insertAfter('#edit-secondaryauthor');
            
        }
            );
    
    var oldNumSpecies = jQuery('#edit-speciesnumber').val();

    jQuery('#edit-speciesnumber').change(function(){
        if (jQuery('#edit-speciesnumber').val() > oldNumSpecies){
            var currentNumSpecies = jQuery('#edit-speciesnumber').val();
            
            for(var i = oldNumSpecies; i < currentNumSpecies; i++){
                var newSpeciesForm = jQuery('#edit-species').clone().attr('id', '#edit-species' + i);
                
                newSpeciesForm.change(function(){
                    if (jQuery('#edit-speciesnumber').val() ){
//                        remove the Form
                    }                    
                })
                        
                newSpeciesForm.insertAfter('#edit-species');
            }
            
            oldNumSpecies = currentNumSpecies;
        }
        
        else if (jQuery('#edit-speciesnumber').val() < oldNumSpecies){
            var currentNumSpecies = jQuery('#edit-speciesnumber').val();
            
            for(var i = oldNumSpecies; i >= currentNumSpecies; --i){
                jQuery('#edit-species1').remove();
            }
            
            oldNumSpecies = currentNumSpecies;``
        }
        else{
        }
    });
    
//    push these changes please!!
        
//    jQuery('#edit-study').change(function () {
//        if (jQuery('#edit-study').val() == 1)
//        {
//            jQuery('.commonGardenClass').show();
//            jQuery('.plantationClass').hide();
//            jQuery('.natPopClass').hide();
//        } else if (jQuery('#edit-study').val() == 2)
//        {
//            jQuery('.commonGardenClass').hide();
//            jQuery('.plantationClass').show();
//            jQuery('.natPopClass').hide();
//        } else if (jQuery('#edit-study').val() == 3)
//        {
//            jQuery('.commonGardenClass').hide();
//            jQuery('.plantationClass').hide();
//            jQuery('.natPopClass').show();
//        }
//    }
//            );

});  