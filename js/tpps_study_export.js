/**
 * @file
 *
 * TPPS Study Export Page specific JS-code.
 */
(function($, Drupal) {
  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Copy to clipboard Submission State field value if any.
  Drupal.tpps.copy_study_state = function() {
    // We need to focus to avoid error message on remote server when
    // 'Manual Export' code is used and this JS file attached from dev-server
    // This line is not necessary for regular 'Study Export' form.
    window.focus();
    state = $('#edit-tpps-export-state').val();
    if (typeof(state) != "undefined") {
      navigator.clipboard.writeText(state);
      if (state.trim().length === 0) {
        console.log('Submission State was removed from the clipboard.');
      }
      else {
        console.log('Submission State was copied to the clipboard.');
      }
    }
  }
  Drupal.tpps.copy_study_state();
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Behavior.
  Drupal.behaviors.tpps_study_export = {
    attach: function (context, settings) {
      // Clear textarea with prev submission state.
      $('#edit-tpps-export-accession', context).on('change', function(e) {
        console.log('Accession was changed.');
        $('#edit-tpps-export-state').val('');
        // Probably it's a overkill but just to be sure we always have
        // up-to-date submission state in clipboard.
        Drupal.tpps.copy_study_state();
      });
    }
  }
})(jQuery, Drupal);
