/*
 * @file
 * Contains code which must be loaded in the header of the document.
 *
 * Note: tpps.js file will be added to the footer of the document.
 */

Drupal.tpps = Drupal.tpps || {};
Drupal.tpps.lastValue = Drupal.tpps.lastValue || {};
// Used to cache AJAX-requests responces to avoid extra server load.
// Controlled by:
// JS-variable 'Drupal.settings.tpps.cacheAjaxResponses' and
// Drupal-variable 'tpps_page_1_cache_ajax_responses';
Drupal.tpps.ajaxCache = Drupal.tpps.ajaxCache || {};

(function ($, Drupal) {

  Drupal.behaviors.tppsHeader = {
    attach:function (context) {
    }
  };

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

  /**
   * Checks if value was changed.
   *
   * @param string key
   *   Unique key of the field. It could be HTML-name or HTML-id.
   * @param string value
   *   New value to be compared with stored before.
   *
   * @return bool
   *   Returns TRUE if new and stored values are different and FALSE otherwise.
   *
   * @TODO Move to tpps_header.js
   */
  Drupal.tpps.wasValueChanged = function(key, value) {
    return (
      'lastValue' in Drupal.tpps
      && typeof Drupal.tpps.lastValue[key] != 'undefined'
      && Drupal.tpps.lastValue[key] != value
    );
  }

  /**
   * Enable HTML field.
   *
   * @param string $selector
   *   JQuery selector for input element.
   */
  Drupal.tpps.fieldEnable = function(selector) {
    $(selector).prop('disabled', false);
  }

  /**
   * Disable HTML field.
   *
   * @param string $selector
   *   JQuery selector for input element.
   */
  Drupal.tpps.fieldDisable = function(selector) {
    $(selector).prop('disabled', true);
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

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
   *
   * Usage example:
   *    var data = {
   *      "errors": [
   *        Drupal.t('Invalid DOI format. Example DOI: 10.1111/dryad.111')
   *      ]
   *    };
   *    Drupal.tpps.showMessages(doiMessageBox, data);
   * Colors:
   * 'status' = green,
   * 'error' = red
   * 'warning' = yellow.
   *
   * @param string selector
   *   Selector of element to show messages below it.
   * @param array data
   *   Messages. Keys are: 'errors', 'warnings' and 'statuses'.
   *   WARNING:
   *   Values are arrays (not strings).
   * @param bool after
   *   Flag which defines where messages must be shown.
   *   Default is true which means 'show messages below the selector'.
   */
  Drupal.tpps.showMessages = function(selector, data, below = true) {
    var $element = $(selector);
    var featureName = 'Drupal.tpps.showMessages'
    if (!$element.length) {
      dog('Element wasn\'t found. Selector: ' + selector, featureName);
    }
    if (!Object.keys(data).length) {
      dog('There is no messages to show. Data object is empty.', featureName);
    }
    else {
      dog('Messages to be shown:', featureName);
      dog(data, featureName);
    }
    dog('Number of current messages for ' + selector + ': ', {
      'Errors': $(selector).nextAll('.tpps-message.error').length,
      'Warnings': $(selector).nextAll('.tpps-message.warning').length,
      'Statuses': $(selector).nextAll('.tpps-message.status').length
    }, featureName);

    // @TODO Minor. Loop statuses.
    if ('errors' in data) {
      var content = '<div class="error tpps-message">'
          + data.errors.join('</div><div class="error tpps-message">') + '</div>';
      if (below) {
        $element.after(content).fadeIn(500);
      }
      else {
        $element.before(content).fadeIn(500);
      }
    }
    if ('warnings' in data) {
      var content = '<div class="warning tpps-message">'
          + data.warnings.join('</div><div class="warning tpps-message">') + '</div>';
      if (below) {
        $element.after(content).fadeIn(500);
      }
      else {
        $element.before(content).fadeIn(500);
      }
    }
    if ('statuses' in data) {
      var content = '<div class="status tpps-message">'
          + data.statuses.join('</div><div class="status tpps-message">') + '</div>';
      if (below) {
        $element.after(content).fadeIn(500);
      }
      else {
        $element.before(content).fadeIn(500);
      }
    }
  }

  /**
   * Clears given message box.
   */
  Drupal.tpps.clearMessages = function(selector) {
    var featureName = 'Drupal.tpps.clearMessages'
    dog('Number of current messages for ' + selector + ': ', {
      'Errors': $(selector).nextAll('.tpps-message.error').length,
      'Warnings': $(selector).nextAll('.tpps-message.warning').length,
      'Statuses': $(selector).nextAll('.tpps-message.status').length
    }, featureName);
    $(selector).nextAll('.tpps-message.error').fadeOut(500).empty().remove();
    $(selector).nextAll('.tpps-message.warning').fadeOut(500).empty().remove();
    $(selector).nextAll('.tpps-message.status').fadeOut(500).empty().remove();
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

  /**
   * Validates string using predefined regex rules.
   *
   * See PHP-version: tpps_is_valid().
   * See /admin/config/tpps/misc for settings.
   *
   * @param rule string
   *   Type of data to be validated.
   *   Posible values are: 'doi', 'organism', 'singleSpace'.
   * @param string string
   *   String to be validated.
   *
   * @return bool
   *   Returns TRUE if validation passed or validation regex is empty.
   *   Returns FALSE if string was empty (after trim()) or validation failed.
   */
  Drupal.tpps.isValid = function (rule, string) {
    let featureName = 'Drupal.tpps.isValid';
    dog(rule, featureName);
    string = $.trim(string);
    if (string == '') {
      console.error(Drupal.t(
        'Empty string for validation using @rule_name rule.',
        {'@rule_name': rule}
      ));
      return false;
    }
    // Check if rule was defined.
    if ('tpps' in Drupal.settings && 'validationRegex' in Drupal.settings.tpps) {
      if (typeof Drupal.settings.tpps.validationRegex[rule] == 'undefined') {
        console.log(Drupal.t('Validation Regex not found for @rule_name rule.',
          {'@rule_name': rule}));
        return true;
      }
    }
    else {
      console.error(Drupal.t('No validation regexes found at all.'));
      return false;
    }
    // Get regex.
    let pattern = Drupal.settings.tpps.validationRegex[rule];
    dog(pattern, featureName);
    // Validate.
    //let re = new RegExp(pattern + 'g');
    //dog(re.test(string), featureName);
    return string.match(pattern) ? true : false;
  }

}(jQuery, Drupal));
