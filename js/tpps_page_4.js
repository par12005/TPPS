(function ($) {
  Drupal.behaviors.tpps_page_4 = {
    attach: function (context, settings) {
      // TPPS Form Page 4 Common.
      // Attach event handlers only once.
      $('form[id^=tppsc-main]', context).once('tpps_page_4', function() {
        let $form = $(this);
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      });
    }
  }
})(jQuery);
