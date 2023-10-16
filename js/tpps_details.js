/**
 * @file
 *
 * TPPS Details page (tpps/details).
 */
(function($, Drupal) {
  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};

  function cl(m, ver = '1') {
    console.log(ver + ': ' + m);
  }

  Drupal.tpps.changeAutocompleteUrl = function() {
    let $form = $('#tpps-details-form');
    let $sourceField = $('#edit-details-type', $form);
    let type = $sourceField.val();
    // Visible text field.
    let $targetField = $('#edit-details-value', $form);

    let $autocompleteField = $targetField.parent().find('input.autocomplete');
    //cl ($autocompleteField);
    // Remove the current autocomplete bindings.
    // And remove the autocomplete class
    $targetField
      .unbind()
      .removeClass('form-autocomplete');

    //$form.find('select:not(.changeAutocompletUrl-processed)')
    //    .addClass('changeAutocompleteUrl-processed')
    //    .change(Drupal.tpps.changeAutocompletUrl);

    $targetField.addClass('form-autocomplete');

    // This is hidden field which stores url.
    $autocompleteField
      .removeClass('autocomplete-processed')
      .val(window.location.origin + '/tpps/autocomplete/' + type);
    //Drupal.detachBehaviors($targetField);

    //Drupal.behaviors.autocomplete($targetField);
    Drupal.attachBehaviors();

    cl($form.find('input.autocomplete').val(), 2);
    //$sourceField.removeClass('tppsDetails-processed');
  }

  Drupal.behaviors.tppsDetails = {
    attach: function (context, settings) {
      $('#edit-details-type', context)
        .once('tppsDetails')
        .on('change', Drupal.tpps.changeAutocompleteUrl);
    }
  }
})(jQuery, Drupal);

