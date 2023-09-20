/**
 * @file
 *
 * TPPS Page 1 form specific JS-code.
 */
(function($, Drupal) {
  var doiSelector = '#edit-doi';
  var doiMessageBox = '#doi-message';

  // Prepare place to temporary store request's data.
  // Usage: console.log(Drupal.tpps.doi);
  if (typeof Drupal.tpps == 'undefined') {
    Drupal['tpps'] = {};
  }
  if (typeof Drupal.tpps.doi == 'undefined') {
    Drupal['tpps']['doi'] = {};
  }

  // Waits for element on page.
  // Source: https://stackoverflow.com/questions/5525071
  function waitForElm(selector) {
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
   * Validate Strings with Regex.
   *
   * @param string string
   *
   * @return bool
   *   Returns TRUE if validation passed and FALSE otherwise.
   */
  function tppsDoiValidate(string) {
    // @TODO [VS] Minor. Get this validation regex from Drupal.settings.tpps.
    var pattern = /^10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+$/;
    return $.trim(string).match(pattern) ? true : false;
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
   * @param string doiSelector
   *   jQuery doiSelector of element to show messages.
   * @param array data
   *   Messages. Keys are: 'errors', 'warnings' and 'statuses'.
   *   WARNING:
   *   Values are arrays (not strings).
   */
  function tppsShowMessages(selector, data) {
    var $doiMessageBox = $(selector);
    if (data.errors !== undefined) {
      $doiMessageBox.append('<div class="error">'
        + data.errors.join('</div><div class="error">') + '</div>');
    }
    if (data.warnings !== undefined) {
      $doiMessageBox.append('<div class="warning">'
        + data.warnings.join('</div><div class="warning">') + '</div>');
    }
    if (data.statuses !== undefined) {
      $doiMessageBox.append('<div class="status">'
        + data.statuses.join('</div><div class="status">') + '</div>');
    }
  }

  function tppsWasDoiChanged(doi) {
    return (
      typeof (Drupal.tpps.doiLastValue) != 'undefined'
      && Drupal.tpps.doiLastValue != doi
    );
  }
  function tppsFieldEnable(selector) {
    $(selector).prop('disabled', false);
  }
  function tppsFieldDisable(selector) {
    $(selector).prop('disabled', true);
  }

  /**
   * tppsDoiFill
   *
   * @param data $data
   * @access public
   * @return void
   */
  function tppsDoiFill(data) {
    if (typeof (data) != 'undefined') {
      // Fill text fields.
      $('#edit-publication-primaryauthor').val(data.doi_info.primary ?? '');
      $('#edit-publication-abstract').val(data.doi_info.abstract ?? '');
      // Default value for year is '0' ('- Select -').
      $('#edit-publication-year').val(data.doi_info.year ?? 0);
      $('#edit-publication-title').val(data.doi_info.title ?? '');
      // @TODO Get Journal field value.
      // $('#edit-publication-journal').val(data.doi_info.journal);

      if (!data.success) {
        // Clear all fields.
        $('input[name="publication[secondaryAuthors][number]"]').val(0);
        $('input[id^="edit-publication-secondaryauthors-remove"]').mousedown();
        $('input[name="organism[1][name]"]').val('');
        $('input[name="organism[number]"]').val(1);
        $('input[id^="edit-organism-remove"]').mousedown();
        return;
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Secondary Authors.
      if (data.doi_info.secondaryNumber > 0) {
        // @TODO Probably outdated. Check it.
        $('input[name="publication[secondaryAuthors][check]"]')
          .val(data.doi_info.secondaryCheck)
        $('input[name="publication[secondaryAuthors][number]"]').val(
          parseInt(data.doi_info.secondaryNumber) - 1
        );
        $('input[id^="edit-publication-secondaryauthors-add"]').mousedown();
        $.each(data.doi_info.secondary, function( key, value ) {
          waitForElm('input[name="publication[secondaryAuthors]['
            + ( key + 1 ) + ']"]')
            .then((elm) => { $(elm).val(value); }
          );
        });
      }
      // Organisms.
      if (data.doi_info.speciesNumber >= 1) {
        // @TODO Reuse empty fields.
        // @TODO Not overwrite 1st field if it's not empty.
        // 1st field is present at form and can't be removed.
        // So it must be populated manually.
        if (typeof data.doi_info.species[0] == 'undefined') {
          $('input[name="organism[1][name]"]').val('');
        }
        else {
          $('input[name="organism[1][name]"]').val(
            data.doi_info.species[0]
          );
        }
        // Add necessary number of fields.
        $('input[name="organism[number]"]')
            .val( data.doi_info.speciesNumber - 1 );
        $('input[id^="edit-organism-add"]').mousedown();
        // Populate new fields.
        $.each(data.doi_info.species, function( key, value ) {
          waitForElm('input[name="organism[' + ( key + 1 ) + '][name]"]')
            .then((elm) => { $(elm).val(value); }
          );
        });
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  Drupal.behaviors.tpps_page_1 = {
    attach: function (context, settings) {

      // Attach event handlers only once.
      $('form[id^=tppsc-main]').once('tpps_page_1', function() {
        // Allows to click on DOI number to fill text field.
        // Add 'tpps-suggestion' class to A tag.
        // Example: <a href"#" class="tpps-suggestion">10.25338/B8864J</a>
        $('.tpps-suggestion').on('click', function(e) {
          e.preventDefault();
          var selectedText= $(this).text();
          $(this).parents('.form-item').find('input.form-text')
            .val(selectedText).blur();
          navigator.clipboard.writeText(selectedText);
        });

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // 'Publication DOI' field change.
        if (typeof(settings.tpps.ajaxUrl) !== 'undefined') {

          $(doiSelector).blur(function(e) {
            tppsFieldDisable(doiSelector);

            // @TODO Check if DOI value really was changed.
            var doi = $(this).val();
            e.preventDefault();
            // This is the same DOI value so nothing to do.
            if (
              typeof (Drupal.tpps.doiLastValue) != 'undefined'
              && Drupal.tpps.doiLastValue == doi
            ) {
              tppsFieldEnable(doiSelector);
              return;
            }
            else {
              // Store current DOI value to be able to compare with new one later.
              Drupal.tpps.doiLastValue = doi;
            }

            if (!tppsDoiValidate(doi)) {
              $(doiMessageBox).empty();
              var data = {
                "errors": [
                  Drupal.t('Invalid DOI format. Example DOI: 10.1111/dryad.111')
                ]
              };
              tppsShowMessages(doiMessageBox, data);
              tppsFieldEnable(doiSelector);
              return;
            }

            // Check if we have cached result first.
            if (Drupal.settings.tpps.cache && typeof (Drupal.tpps.doi[doi]) != 'undefined') {
              $(doiMessageBox).empty();
              var data = Drupal.tpps.doi[doi];
              tppsShowMessages(doiMessageBox, data);
              tppsFieldEnable(doiSelector);
              tppsDoiFill(data);
            }
            else {
              var url = settings.basePath + settings.tpps.ajaxUrl + '/get_doi';
              // Remove existing messages.
              $(doiMessageBox).empty();
              $.ajax({
                method: 'post',
                data: {'doi': doi},
                url: url,
                error: function (jqXHR, textStatus, errorThrown) {
                  // User changed DOI during AJAX-request.
                  if (tppsWasDoiChanged(doi)) { return; }
                  // Server/Network errors.
                  tppsShowMessages(doiMessageBox, [{
                    "errors": [
                      Drupal.t("DOI value wasn't completed")
                    ]
                  }]);
                  var errorMessage = jqXHR.status + " " + jqXHR.statusText
                    + "\n\n" + jqXHR.responseText;
                  console.log(errorMessage);
                  tppsFieldEnable(doiSelector);
                },
                success: function(data) {
                  // Store DOI check result to avoid multiple requests.
                  Drupal.tpps.doi[doi] = data;
                  // User changed DOI during AJAX-request.
                  if (tppsWasDoiChanged(doi)) { return; }
                  if (typeof (data) == 'undefined') {
                    var data = [{
                      "errors": [Drupal.t('Received empty response.')]
                    }];
                  }
                  tppsShowMessages(doiMessageBox, data);
                  tppsFieldEnable(doiSelector);
                  tppsDoiFill(data);
                }
              });
            }
          });
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      });
    }
  }
})(jQuery, Drupal);
