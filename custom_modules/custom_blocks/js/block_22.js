/**
 * @file
 *
 * Block 22. Page Title and Title Header for Publication Details
 * https://tgwebdev.cam.uchc.edu/admin/config/development/js-injector/list/js_publication_details_code/edit
 */
(function($, Drupal) {
  $(document).ready(function() {
  /* ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
    console.log('sdfsdf');
    var pub_title = jQuery(".field-name-publication-title .field-item").html();
    jQuery(".field-name-publication-title .field-item").parent().parent().hide();
    jQuery("#publication_title_heading").html(pub_title);

    // This code will alter the labels for the Publication Details fields.
    jQuery(".field-name-tpub--title .field-label").html("Title&nbsp;");
    jQuery(".field-name-tpub--authors .field-label").html("Author(s):&nbsp;");
    jQuery(".field-name-tpub--year .field-label").html("Year:&nbsp;");
    jQuery(".field-name-tpub--series-name .field-label").html("Journal:&nbsp;");
    jQuery(".field-name-tpub--keywords .field-label").html("Keyword(s):&nbsp;");
    jQuery(".field-name-tpub--pages .field-label").html("Page Numbers(s):&nbsp;");

    // This code will hide requested fields if they are empty.
    jQuery(".field").each(function() {
      var field_container = jQuery(this);
      jQuery(this).find(".field-item").each(function() {
        // Assuming item is one.
        if (jQuery(this).html() == '' || jQuery(this).html() == '-') {
          console.log('FOUND AN EMPTY field-item so hiding the container');
          // Value is empty so hide container.
          field_container.parent().hide();
        }
      });
    });
  /* ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
  });
})(jQuery, Drupal);
