jQuery(document).ready(function ($) {
  var picker = true;
  $('#edit-color').colorPicker({
    opacity: false, // disables opacity slider
  });
  $('#color-picker').click(function() {
    if (!picker) {
      $('#edit-color').colorPicker({
        opacity: false, // disables opacity slider
      });
      picker = true;
    }
    else {
      $('.cp-color-picker').remove();
      picker = false;
    }
  });

  $('#edit-color').css(
    'background-color',
    $('#edit-color')[0].value
  );
  $('#edit-color').on('input', function() {
    $('#edit-color').css(
      'background-color',
      $('#edit-color')[0].value
    );
  });
})(jQuery);
