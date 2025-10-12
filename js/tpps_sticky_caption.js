/**
 * @file
 *
 * TPPS. Adds table's caption to the sticky header.
 */
(function($, Drupal) {
  $(document).ready(function() {

    // Check if table with sticky header exists.
    $('table.sticky-caption').each(function() {
      // Get title
      let $fieldset = $(this).parents('fieldset');
      let $stickyHeader = $fieldset.find('table.sticky-header');
      let title = $fieldset.find('span.fieldset-legend a.fieldset-title')
        // Get only text without HTML-tags.
        // https://stackoverflow.com/a/33592275/1041470
        .clone().children().remove().end().text().trim();
      // Get number of columns in table.
      let columnNumber = $stickyHeader.find('thead tr').children().length;
      // Update sticky header.
      $stickyHeader.find('thead').prepend('<tr><th class="sticky-caption" '
        + ' colspan="' + columnNumber + '">' + title + '</th></tr>');
    });
  });
})(jQuery, Drupal);
