/**
 * @file
 *
 * TPPS AutoFocus feature.
 *
 * Sets focus to the field specified in
 * Drupal.settings.tpps.autoFocus.FieldName.
 * See
 * - tpps_add_css_js(),
 * - tpps_form_autofocus().
 */
(function($, Drupal) {
  $(document).ready(function() {
    setTimeout(
      function() {
        var $element = $('[name="' + Drupal.settings.tpps.autoFocus.FieldName + '"');
        if (typeof $element != 'undefined' ) {
          $element.get(0).focus();
        }
      },
      Drupal.settings.tpps.autoFocus.Timeout
    );
  });
})(jQuery, Drupal);
