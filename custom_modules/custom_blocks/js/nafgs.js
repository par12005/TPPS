/**
 * @file
 *
 * NAFGS Bootstrap code
 *
 * For pages: /nafgs, /nafgs/*.
 */
(function($, Drupal) {
  $(document).ready(function() {
  /* ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
    jQuery('div.logo').css('display', 'none');
    jQuery('body').css('background-color', '#FFFFFF');
    setTimeout(function() {
      jQuery('#main-menu').removeClass('mobile-active');
    }, 1000);

    jQuery(window).on('resize', function(){
      var win = jQuery(this); //this = window
      if (win.width() >= 600) {
         setTimeout(function() {
          jQuery('#main-menu').removeClass('mobile-active');
          console.log('Removing class mobile-active');
         }, 1000);
         console.log('Keeping menu open');
      }
    });
  /* ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
  });
})(jQuery, Drupal);
