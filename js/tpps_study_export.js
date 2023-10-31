/**
 * @file
 *
 * TPPS Study Export Page specific JS-code.
 */
(function($, Drupal) {
  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Behavior.
  Drupal.behaviors.tpps_study_export = {
    attach: function (context, settings) {
      // Clear textarea with prev submission state.
      $('#edit-tpps-export-accession', context).on('change', function(e) {
        console.log('changed');
        $('#edit-tpps-export-state').val('');
      });
    }
  }
})(jQuery, Drupal);
