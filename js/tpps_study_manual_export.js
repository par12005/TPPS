/**
 * @file
 *
 * TPPS Study Manual Export Page specific JS-code.
 */
(function($, Drupal) {
  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Copy to clipboard Submission State field value if any.
  Drupal.tpps.copy_code = function() {
    state = $('#edit-tpps-manual-export-code').val();
    if (typeof(state) != "undefined") {
      navigator.clipboard.writeText(state);
      console.log('Code was copied to the clipboard.');
    }
  }
  Drupal.tpps.copy_code();
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Behavior.
  Drupal.behaviors.tpps_study_manual_export = {
    attach: function (context, settings) {
      // Clear textarea with prev submission state.
      $('#edit-tpps-manual-export-accession', context).on('change', function(e) {
        //Drupal.tpps.copy_code();
        $('#tpps-study-manual-export-form').submit();
      });
    }
  }
})(jQuery, Drupal);
