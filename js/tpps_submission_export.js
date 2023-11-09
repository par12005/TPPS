/**
 * @file
 *
 * TPPS Submission Export Page specific JS-code.
 */
(function($, Drupal) {
  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Copy to clipboard Submission State field value if any.
  Drupal.tpps.copy_submission_state = function() {
    // We need to focus to avoid error message on remote server when
    // 'Manual Export' code is used and this JS file attached from dev-server
    // This line is not necessary for regular 'Submission Export' form.
    window.focus();
    state = $('#edit-tpps-submission-export-state').val();
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
  Drupal.tpps.copy_submission_state();
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Behavior.
  Drupal.behaviors.tpps_submission_export = {
    attach: function (context, settings) {
      // Clear textarea with prev submission state.
      $('#edit-tpps-submission-export-accession', context).on('change', function(e) {
        console.log('Accession was changed.');
        $('#edit-tpps-submission-export-state').val('');
        // Probably it's a overkill but just to be sure we always have
        // up-to-date submission state in clipboard.
        Drupal.tpps.copy_submission_state();
      });
    }
  }
})(jQuery, Drupal);
