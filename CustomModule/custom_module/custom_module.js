

jQuery(document).ready(function () {

    jQuery('.form-item-commonGardenSelect').addClass('commonGardenClass');
    jQuery('.form-item-plantationSelect').addClass('plantationClass');
    jQuery('.form-item-natPopSelect').addClass('natPopClass');


    jQuery('#submitAjaxButton').click(function () {

        // alert('You clicked on the ajax button');
        jQuery.ajax({

            url: Drupal.settings.basePath + 'getHello',
            data: {

                name: jQuery("#edit-name").val(),
                token: myToken

            },

            success: function (data) {

                jQuery("#msg-display-area").empty();
                jQuery("#msg-display-area").append(data);
                jQuery('#edit-study').hide();

            }

        });

    });

    /* Function to test the hiding of elements
     ** based on the user's selection in a drop down
     */

    jQuery('#edit-study').change(function () {

        if (jQuery('#edit-study').val() == 1)
        {
            jQuery('.commonGardenClass').show();
            jQuery('.plantationClass').hide();
            jQuery('.natPopClass').hide();
        } else if (jQuery('#edit-study').val() == 2)
        {
            jQuery('.commonGardenClass').hide();
            jQuery('.plantationClass').show();
            jQuery('.natPopClass').hide();
        } else if (jQuery('#edit-study').val() == 3)
        {
            jQuery('.commonGardenClass').hide();
            jQuery('.plantationClass').hide();
            jQuery('.natPopClass').show();
        }

    });

});
