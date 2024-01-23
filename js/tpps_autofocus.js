/**
 * @file
 *
 * TPPS AutoFocus feature.
 *
 * See function tpps_form_autofocus().
 */
(function($, Drupal) {
  $(document).ready(function() {
    setTimeout(
      function() {
        $('[name="' + Drupal.settings.tpps.autoFocus.FieldName + '"')
          .get(0).focus();
      },
      Drupal.settings.tpps.autoFocus.Timeout
    );
  });
})(jQuery, Drupal);
