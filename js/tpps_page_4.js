(function ($) {
  Drupal.behaviors.tppsPage4 = {
    attach: function (context, settings) {
      // TPPS Form Page 4 Common.
      // Attach event handlers only once.
      $('form[id^=tppsc-main]', context).once('tppsPage4', function() {
        let $form = $(this);
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      });
    }
  }
})(jQuery);
