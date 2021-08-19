jQuery(document).ready(function ($) {
    $('#edit-color').colorPicker({
        opacity: false, // disables opacity slider
        renderCallback: function($elm, toggled) {
            $elm.val('#' + this.color.colors.HEX);
        }
    });
});