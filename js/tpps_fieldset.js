/**
 * @file
 *
 * TPPS Fieldset Collapsed Status.
 *
 * Remembers status of the fieldset when form submitted.
 */
(function ($) {
  Drupal.behaviors.tppsFieldsetCollapsedStatus = {
    attach: function (context) {
      // Allows to store status of the fieldset (collapsed or not).
      $('.' + Drupal.settings.tpps.fieldsetCollapsedStatus.class, context).each(function() {
        let fieldsetName = $(this).attr('fieldset');
        if (typeof fieldsetName == 'undefined' || !fieldsetName.length) {
          return;
        }
        let fieldsetId = '#edit-' + fieldsetName.replace(/_/g, '-');
        $(fieldsetId + ' legend a.fieldset-title')
          .on('click', function(e) {
            // Store via form submit.
            let phpFieldsetName = $(this).parents('fieldset').attr('id')
              .replace('edit-', '').replace(/-/g, '_');
            // @TODO Get field name from settings.
            let hiddenFieldName = Drupal.settings.tpps.fieldsetCollapsedStatus.prefix
              + phpFieldsetName;
            let $hiddenField = $("[name*='" + hiddenFieldName + "']");
            $hiddenField.val(($hiddenField.val() == 1 ? 0 : 1));
            // Store via ajax-request to server side.
            $.ajax({
                url: Drupal.settings.tpps.fieldsetCollapsedStatus.ajaxUrl,
                method: 'post',
                dataType: 'json',
                data: {
                  'name': hiddenFieldName,
                  'value': $hiddenField.val(),
                },
                success: function(data){
                  console.log(data);
                },
            });
          });
      });
    }
  };
}(jQuery));
