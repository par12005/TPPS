/**
 * @file
 *
 * TPPS Page 1 form specific JS-code.
 */
(function($, Drupal) {
  var doiSelector = '#edit-doi';
  var doiMessageBox = '#doi-message';
  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};
  Drupal.tpps.doi = Drupal.tpps.doi || {};

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
   * Validate Strings with Regex.
   *
   * @param string string
   *
   * @return bool
   *   Returns TRUE if validation passed and FALSE otherwise.
   */
  Drupal.tpps.DoiValidate  = function (string) {
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
  Drupal.tpps.ShowMessages = function(selector, data) {
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

  /**
   * Checks if value of DOI field changed.
   *
   * @param string doi
   *   New DOI value.
   *
   * @return bool
   *   Returns TRUE if new and stored values are different and FALSE otherwise.
   */
  Drupal.tpps.wasDoiChanged = function(doi) {
    return (
      typeof (Drupal.tpps.doiLastValue) != 'undefined'
      && Drupal.tpps.doiLastValue != doi
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


  Drupal.tpps.resetForm = function() {
    $('#edit-publication-primaryauthor').val('');
    $('#edit-publication-abstract').val('');
    // Default value for year is '0' ('- Select -').
    $('#edit-publication-year').val(0);
    $('#edit-publication-title').val('');
    $('#edit-publication-journal').val('');
    // Secondary Authors.
    if ($('input[name="publication[secondaryAuthors][number]"]').val() != 0) {
      $('input[name="publication[secondaryAuthors][number]"]').val(0);
      $('input[id^="edit-publication-secondaryauthors-remove"]').mousedown();
    }
    // Organism.
    $('input[name="organism[1][name]"]').val('');
    if ($('input[name="organism[number]"]').val() != 1) {
      $('input[name="organism[number]"]').val(1);
      $('input[id^="edit-organism-remove"]').mousedown();
    }
  }

  /**
   * Fills form using DOI information.
   *
   * @param object $data
   *   Data received from remote DOI database.
   */
  Drupal.tpps.doiFill = function(data) {
    if (typeof (data) != 'undefined') {
      if (!data.success) {
        Drupal.tpps.resetForm();
        return;
      }
      // Fill text fields.
      $('#edit-publication-primaryauthor').val(data.doi_info.primary ?? '');
      $('#edit-publication-abstract').val(data.doi_info.abstract ?? '');
      // Default value for year is '0' ('- Select -').
      $('#edit-publication-year').val(data.doi_info.year ?? 0);
      $('#edit-publication-title').val(data.doi_info.title ?? '');
      // @TODO Get Journal field value.
      // $('#edit-publication-journal').val(data.doi_info.journal);
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
          Drupal.waitForElm('input[name="publication[secondaryAuthors]['
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
          Drupal.waitForElm('input[name="organism[' + ( key + 1 ) + '][name]"]')
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

        // Reset form if status != 'Published'.
        $('#edit-publication-status').on('change', function(e) {
          if ($(this).val() != 'Published') {
            Drupal.tpps.resetForm();
          }
        });
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // 'Publication DOI' field change.
        if (typeof(settings.tpps.ajaxUrl) !== 'undefined') {

          $(doiSelector).blur(function(e) {
            Drupal.tpps.fieldDisable(doiSelector);

            // @TODO Check if DOI value really was changed.
            var doi = $(this).val();
            e.preventDefault();
            // This is the same DOI value so nothing to do.
            if (
              typeof (Drupal.tpps.doiLastValue) != 'undefined'
              && Drupal.tpps.doiLastValue == doi
            ) {
              Drupal.tpps.fieldEnable(doiSelector);
              return;
            }
            else {
              // Store current DOI value to be able to compare with new one later.
              Drupal.tpps.doiLastValue = doi;
            }

            if (!Drupal.tpps.DoiValidate(doi)) {
              $(doiMessageBox).empty();
              var data = {
                "errors": [
                  Drupal.t('Invalid DOI format. Example DOI: 10.1111/dryad.111')
                ]
              };
              Drupal.tpps.ShowMessages(doiMessageBox, data);
              Drupal.tpps.fieldEnable(doiSelector);
              return;
            }

            // Check if we have cached result first.
            if (Drupal.settings.tpps.cache && typeof (Drupal.tpps.doi[doi]) != 'undefined') {
              $(doiMessageBox).empty();
              var data = Drupal.tpps.doi[doi];
              Drupal.tpps.ShowMessages(doiMessageBox, data);
              Drupal.tpps.fieldEnable(doiSelector);
              Drupal.tpps.doiFill(data);
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
                  if (Drupal.tpps.wasDoiChanged(doi)) { return; }
                  // Server/Network errors.
                  Drupal.tpps.ShowMessages(doiMessageBox, [{
                    "errors": [
                      Drupal.t("DOI value wasn't completed")
                    ]
                  }]);
                  var errorMessage = jqXHR.status + " " + jqXHR.statusText
                    + "\n\n" + jqXHR.responseText;
                  console.log(errorMessage);
                  Drupal.tpps.fieldEnable(doiSelector);
                },
                success: function(data) {
                  // Store DOI check result to avoid multiple requests.
                  Drupal.tpps.doi[doi] = data;
                  // User changed DOI during AJAX-request.
                  if (Drupal.tpps.wasDoiChanged(doi)) { return; }
                  if (typeof (data) == 'undefined') {
                    var data = [{
                      "errors": [Drupal.t('Received empty response.')]
                    }];
                  }
                  Drupal.tpps.ShowMessages(doiMessageBox, data);
                  Drupal.tpps.fieldEnable(doiSelector);
                  Drupal.tpps.doiFill(data);
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