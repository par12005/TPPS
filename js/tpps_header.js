/*
 * @file
 * Contains code which must be loaded in the header of the document.
 *
 * Note: tpps.js file will be added to the footer of the document.
 */
(function ($) {

  Drupal.behaviors.tppsHeader = {
    attach:function (context) {
    }
  };

  /**
   * Debug Log.
   *
   * Shows colored and well formatted messages using console.log().
   *
   * @param mix $message
   *   Message to be show.
   * @param callerFunctionName
   *   Optional. Name of the function/feature.
   *   When empty caller function name will be used.
   *   Works bad with attach in Drupal.behaviors.
   */
  function dog(message, callerFunctionName = null) {
    if (Drupal.settings.tpps.googleMap.debugMode ?? false) {
      // Get caller function name to use as a prefix.
      // https://stackoverflow.com/a/39337724/1041470
      var e = new Error('dummy');
      if (callerFunctionName === null) {
        callerFunctionName = e.stack
          .split('\n')[2]
          .replace(/^\s+at\s(.+?)(?:\s.*:|:)(.*?):(.*?)\)?$/g, '$1 [$2:$3]');
      }
      else {
        callerFunctionName = callerFunctionName + e.stack
          .split('\n')[2]
          .replace(/^\s+at\s(.+?)(?:\s.*:|:)(.*?):(.*?)\)?$/g, ' [$2:$3]');
      }

      // https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Operators/typeof
      switch (typeof message) {
        case "object":
          console.log(
            '%c' + callerFunctionName + ':\n  %O',
            'color: #73B1FF', // Sky-blue
            message,
          );
          break;

        case "boolean":
        case "undefined":
        case "symbol":
        case "string":
        case "number":
        default:
          console.log(
            '%c' + callerFunctionName + ':\n  %c' + message,
            'color: #73B1FF', // Sky-blue
            'color: #ff9900'  // dark orange
          );
          break;
      }
    }
  }
  window.dog = dog;

}(jQuery));
