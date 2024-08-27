/**
 * @file
 *
 * TPPS Submission Export Page specific JS-code.
 *
 * See https://stackoverflow.com/a/75665249/1041470
 */
(function($, Drupal) {
  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};

  async function permissionsCheck() {
    const read = await navigator.permissions.query({
        name: 'clipboard-read',
    });
    const write = await navigator.permissions.query({
        name: 'clipboard-write',
    });
    return write.state === 'granted' && read.state !== 'denied';
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Copy to clipboard Submission State field value if any.
  async function copySubmissionState() {
    console.log('sdfsf');
    try {
      const hasPermissions = await permissionsCheck();
      if (hasPermissions && document.hasFocus()) {
        // We need to focus to avoid error message on remote server when
        // 'Manual Export' code is used and this JS file attached from dev-server
        // This line is not necessary for regular 'Submission Export' form.
        state = $('#edit-tpps-submission-export-state').val();
        if (typeof(state) != "undefined") {
          navigator.clipboard.writeText(state);
          if (state.trim().length === 0) {
            console.log('Clipboard was cleared.');
          }
          else {
            console.log('Submission State was copied to the clipboard.');
          }
        }
      } else {
        console.log('WARNING: Close console so window will get focus and '
          + 'submission state array will be copied into clipboard.');
      }
    } catch (err) {
      console.error(err);
    }
  }

  $(document).ready(function() {
    copySubmissionState();
  });

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
        copySubmissionState();
      });
    }
  }
})(jQuery, Drupal);
