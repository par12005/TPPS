/*
 * @file
 * Contains code which must be loaded in the header of the document.
 *
 * Note: tpps.js file will be added to the footer of the document.
 */

Drupal.tpps = Drupal.tpps || {};

(function ($, Drupal) {

  Drupal.behaviors.tppsHeader = {
    attach:function (context) {
    }
  };

  /**
   * Waits until element will appear at page.
   *
   * Source: https://stackoverflow.com/questions/5525071
   *
   * @param string selector
   *   JQuery selector.
   */
  Drupal.waitForElm = function(selector) {
    return new Promise(resolve => {
      if (document.querySelector(selector)) {
        return resolve(document.querySelector(selector));
      }
      const observer = new MutationObserver(mutations => {
        if (document.querySelector(selector)) {
          observer.disconnect();
          resolve(document.querySelector(selector));
        }
      });
      observer.observe(document.body, {
        childList: true,
        subtree: true
      });
    });
  }

  /**
   * Shows messages in given element.
   *
   * All existing messages will be shown.
   *
   * Colors:
   * 'status' = green,
   * 'error' = red
   * 'warning' = yellow.
   *
   * Usage example:
   *    var data = {
   *      "errors": [
   *        Drupal.t('Invalid DOI format. Example DOI: 10.1111/dryad.111')
   *      ]
   *    };
   *    Drupal.tpps.showMessages(doiMessageBox, data);
   *
   * @param string selector
   *   Selector of element to show messages above it.
   * @param array data
   *   Messages. Keys are: 'errors', 'warnings' and 'statuses'.
   *   WARNING:
   *   Values are arrays (not strings).
   */
  Drupal.tpps.showMessages = function(selector, data) {
    var $element = $(selector);
    var featureName = 'Drupal.tpps.showMessages'
    if (!$element.length) {
      dog('Element wasn\'t found. Selector: ' + selector, featureName);
    }
    if (!Object.keys(data).length) {
      dog('There is no messages to show. Data object is empty.', featureName);
    }
    if ('errors' in data) {
      $element.after('<div class="error tpps-message">'
        + data.errors.join('</div><div class="error tpps-message">') + '</div>')
        .fadeIn(500);
    }
    if ('warnings' in data) {
      $element.after('<div class="warning tpps-message">'
        + data.warnings.join('</div><div class="warning tpps-message">') + '</div>')
        .fadeIn(500);
    }
    if ('statuses' in data) {
      $element.after('<div class="status tpps-message">'
        + data.statuses.join('</div><div class="status tpps-message">') + '</div>')
        .fadeIn(500);
    }
  }

  /**
   * Clears given message box.
   */
  Drupal.tpps.clearMessages = function(selector) {
    $(selector).nextAll('.tpps-message.error').fadeOut(500).empty();
    $(selector).nextAll('.tpps-message.warning').fadeOut(500).empty();
    $(selector).nextAll('.tpps-message.status').fadeOut(500).empty();
  }

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
  function dog(message, callerFunctionName = null, options = null) {
    if ( !(
      'tpps' in Drupal.settings
      && 'debugMode' in Drupal.settings.tpps
      && Object.keys(Drupal.settings.tpps.debugMode).length
    )) {
      // JS Debug Mode wasn't enabled.
      return;
    }
    let typeOfMessage = typeof message;
    if (callerFunctionName != null && typeof callerFunctionName == 'object') {
      typeOfMessage = 'table';
      var table = callerFunctionName;
      callerFunctionName = options;
    }

    // Get caller function name.
    // https://stackoverflow.com/a/39337724/1041470
    var e = new Error('dummy');
    if (callerFunctionName === null || callerFunctionName == '') {
      callerFunctionName = e.stack
        .split('\n')[2]
        .replace(/^\s+at\s(.+?)(?:\s.*:|:)(.*?):(.*?)\)?$/g, '$1');
    }

    if (typeof Drupal.settings.tpps.debugMode[callerFunctionName] != 'undefined') {
      // Get line number and position in the line.
      var lineAndPosition = e.stack
          .split('\n')[2]
          .replace(/^\s+at\s(.+?)(?:\s.*:|:)(.*?):(.*?)\)?$/g, '[$2:$3]');

      // https://developer.mozilla.org/ru/docs/Web/JavaScript/Reference/Operators/typeof
      switch (typeOfMessage) {
        case "table":
          console.log(
            '%c' + callerFunctionName + ' ' + lineAndPosition + ':\n  %c' + message,
            'color: #73B1FF', // Sky-blue
            'color: #ff9900'  // dark orange
          );
          console.table(table);
          break;

        case "object":
          console.log(
            '%c' + callerFunctionName + ' ' + lineAndPosition + ':\n  %O',
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
            '%c' + callerFunctionName + ' ' + lineAndPosition + ':\n  %c' + message,
            'color: #73B1FF', // Sky-blue
            'color: #ff9900'  // dark orange
          );
          break;
      }
    }
  }
  window.dog = dog;

}(jQuery, Drupal));
