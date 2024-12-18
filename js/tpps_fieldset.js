/**
 * @file
 *
 * TPPS Remember Fieldset Collapsed State.
 *
 * Remembers status of collapsible fieldsets.
 */
(function ($) {
  Drupal.behaviors.tppsFieldsetCollapsedState = {
    attach: function (context) {
      /**
       * Stores state of the fieldset.
       *
       * @param object element
       *   JQuery object of the element.
       */
      function storeState(element, op) {
        let $fieldset = element.parents('fieldset');
        let value = $fieldset.hasClass('collapsed');
        // Strange but values are reverted. Probably because we check
        // before it was changed by Drupal API.
        if (op == 'click') {
          value = !value;
        }
        $.ajax({
            url: Drupal.settings.tpps.fieldsetCollapsedState.ajaxUrl,
            method: 'post',
            dataType: 'json',
            data: {
              'form_id': element.parents('form').attr('id'),
              'name': $fieldset.attr('id'),
              'value': value ? 1 : 0,
            },
        });
      }

      // Store fieldset state (collapsed or expanded).
      $('fieldset.collapsible legend a.fieldset-title', context)
        // 'click' event can't be used because fieldset not have 'collapsed'
        // class yet.
        .on('mouseup keyup', function() {
          storeState($(this), 'click');
      })
      .each(function() {
        storeState($(this), 'load');
      });
    }
  };
}(jQuery));
