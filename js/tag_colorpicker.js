jQuery(document).ready(function ($) {
  var picker = false;
  $('#color-picker').click(function() {
    if (!picker) {
      console.log('adding color picker');
      $('#edit-color').colorPicker({
        opacity: false, // disables opacity slider
      });
      picker = true;
    }
    else {
      console.log('removing color picker');
      $('.cp-color-picker').remove();
      picker = false;
    }
  });

  jQuery('#edit-color').css('background-color', jQuery('#edit-color')[0].value);
  $('#edit-color').on('input', function() {
    jQuery('#edit-color').css('background-color', jQuery('#edit-color')[0].value);
  });
});