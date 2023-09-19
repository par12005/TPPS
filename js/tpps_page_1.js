/**
 * @file
 *
 * TPPS Page 1 form specific JS-code.
 */
(function($, Drupal) {

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
  function validateStrings(string) {
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
   * @param string selector
   *   jQuery selector of element to show messages.
   * @param array data
   *   Messages. Keys are: 'errors', 'warnings' and 'statuses'.
   *   WARNING:
   *   Values are arrays (not strings).
   */
  function tppsShowMessages(selector, data) {
    var $messageBox = $(selector);
    if (data.errors !== undefined) {
      $messageBox.append('<div class="error">'
        + data.errors.join('</div><div class="error">') + '</div>');
    }
    if (data.warnings !== undefined) {
      $messageBox.append('<div class="warning">'
        + data.warnings.join('</div><div class="warning">') + '</div>');
    }
    if (data.statuses !== undefined) {
      $messageBox.append('<div class="status">'
        + data.statuses.join('</div><div class="status">') + '</div>');
    }
  }

  Drupal.behaviors.tpps_page_1 = {
    attach: function (context, settings) {

      // Attach event handlers only once.
      $('form[id^=tppsc-main').once('tpps_page_1', function() {
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // 'Publication DOI' field change.
        if (typeof(settings.tpps.ajaxUrl) !== 'undefined') {
          var selector = '#edit-doi';
          var messageBox = '#doi-message';

          $(selector).blur(function(e) {
            // @TODO Check if DOI value really was changed.
            var doi = $(this).val();
            e.preventDefault();
            // This is the same DOI value so nothing to do.
            if (Drupal.tpps.doiLastValue != doi) {
              return;
            }
            if (!validateStrings(doi)) {
              $(messageBox).empty();
              var data = {
                "errors": [
                  Drupal.t('Invalid DOI format. Example DOI: 10.1111/dryad.111')
                ]
              };
              tppsShowMessages(messageBox, data);
              return;
            }
            // Store current DOI value to be able to compare with new one later.
            Drupal.tpps.doiLastValue = doi;

            // Check if we have cached result first.
            if (Drupal.settings.tpps.cache && typeof (Drupal.tpps.doi[doi]) != 'undefined') {
              $(messageBox).empty();
              var data = Drupal.tpps.doi[doi];
              tppsShowMessages(messageBox, data);
            }
            else {
              var url = settings.basePath + settings.tpps.ajaxUrl + '/get_doi';
              // Remove existing messages.
              $(messageBox).empty();
              $.ajax({
                method: 'post',
                data: {'doi': doi},
                url: url,
                error: function (jqXHR, textStatus, errorThrown) {
                  // Server/Network errors.
                  tppsShowMessages(messageBox, [{
                    "errors": [
                      Drupal.t("DOI value wasn't completed")
                    ]
                  }]);
                  var errorMessage = jqXHR.status + " " + jqXHR.statusText
                    + "\n\n" + jqXHR.responseText;
                  console.log(errorMessage);
                },
                success: function (data) {
                  // Store DOI check result to avoid multiple requests.
                  // @TODO Check if data not empty.
                  if (typeof (data) !== 'undefined') {
                    Drupal.tpps.doi[doi] = data;
                    tppsShowMessages(messageBox, data);
                    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                    // Store DOI check result to avoid multiple requests.
                    // @TODO Check if data not empty.
                    if (typeof (data) != 'undefined' && data.success) {
                      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::
                      // Fill text fields.
                      if (!$('#edit-publication-primaryauthor').val()) {
                        $('#edit-publication-primaryauthor').val(data.doi_info.primary);
                      }
                      if (!$('#edit-publication-abstract').val()) {
                        $('#edit-publication-abstract').val(data.doi_info.abstract);
                      }
                      // Default value for year is '0' ('- Select -').
                      if (
                        $('#edit-publication-year').val() == 0
                        || !$('#edit-publication-year').val()
                      ) {
                        $('#edit-publication-year').val(data.doi_info.year);
                      }
                      if (!$('#edit-publication-title').val()) {
                        $('#edit-publication-title').val(data.doi_info.title);
                      }
                      // @TODO Get Journal field value.
                      // $('#edit-publication-journal').val(data.doi_info.journal);

                      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::
                      // Secondary Authors.
                      if (data.doi_info.secondaryNumber >= 1) {
                        // @TODO Probably outdated. Check it.
                        $('input[name="publication[secondaryAuthors][check]"]')
                          .val(data.doi_info.secondaryCheck)
                        // @TODO Reuse empty fields.
                        var secondaryStart = parseInt(
                          $('input[name="publication[secondaryAuthors][number]"]').val()
                        );
                        $('input[name="publication[secondaryAuthors][number]"]').val(
                          // We are adding an extra fields (not update existing)
                          // to leave user's input.
                          secondaryStart + parseInt(data.doi_info.secondaryNumber) - 1
                        );
                        $('input[id^="edit-publication-secondaryauthors-add"]').mousedown();
                        $.each(data.doi_info.secondary, function( key, value ) {
                          waitForElm('input[name="publication[secondaryAuthors]['
                            + ( secondaryStart + key + 1 ) + ']"]')
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
                        $('input[name="organism[1][name]"]').val(
                          data.doi_info.species[0]
                        );
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
                    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                  }
                  else {
                    var data = [{
                      "errors": [Drupal.t('Received empty response.')]
                    }];
                    tppsShowMessages(messageBox, data);
                    return;
                  }
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
