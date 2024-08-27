/**
 * @file
 *
 * TPPS Suggestion feature.
 *
 * Allows to insert example/suggestion values from textfield descriptions.
 *
 * How to use:
 *
 * '#description' => 'Example: '
 *       . '<a href"#" class="tpps-suggestion">Arabidopsis thaliana</a>.',
 * ...
 * tpps_add_css_js('suggestion', $form);
 *
 * See
 * - tpps_add_css_js(),
 */
(function ($, Drupal) {
  Drupal.behaviors.tpps_suggestion = {
    attach: function (context, settings) {
      // Allows to fill field using suggestions from description.
      // How to use:
      //   add 'tpps-suggestion' class to A tag in
      //   $form[$field_name]['#description'];
      // Example: <a href"#" class="tpps-suggestion">10.25338/B8864J</a>
      $('.tpps-suggestion').not('.tpps-suggestion-processed').on('click', function(e) {
        e.preventDefault();
        let selectedText= $(this).text();
        //console.log(selectedText);
        $(this)
          .parents('.form-item')
          .find('input.form-text')
          .val(selectedText)
          .blur();
        navigator.clipboard.writeText(selectedText);
      }).addClass('tpps-suggestion-processed');
    }
  };
})(jQuery, Drupal);
