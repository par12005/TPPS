/**
 * @file Page 1.
 */

(function($) {
  console.log('page 1');
    var content = $("#block-system-main").find(".content")
    var link = $("#demo-ajax-link");
    var req;

    link.click(function( e ) {
      e.preventDefault();
      req = $.ajax("/demo/ajax", {
        dataType: "json",
        type: "GET"
      })
      req.done(function( data, textStatus, jqXHR ) {
        content.append( data.data )
      })
      req.fail(function( jqXHR, textStatus, errorThrown ) {
      })
      req.always(function( jqXHR, textStatus ) {
      })
    })
})(jQuery);
