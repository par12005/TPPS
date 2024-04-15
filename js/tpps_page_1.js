/**
 * @file
 *
 * TPPS Page 1 form specific JS-code.
 */
(function($, Drupal) {
  var doiSelector = 'input[name="publication[publication_doi]"]';
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
        + data.errors.join('</div><div class="error">') + '</div>')
        .fadeIn(500);
    }
    if (data.warnings !== undefined) {
      $doiMessageBox.append('<div class="warning">'
        + data.warnings.join('</div><div class="warning">') + '</div>')
        .fadeIn(500);
    }
    if (data.statuses !== undefined) {
      $doiMessageBox.append('<div class="status">'
        + data.statuses.join('</div><div class="status">') + '</div>')
        .fadeIn(500);
    }
  }

  /**
   * Clears given message box.
   */
  Drupal.tpps.ClearMessages = function(selector) {
    var $doiMessageBox = $(selector);
    $doiMessageBox.fadeOut(500).empty();
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
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Fill text fields.
      $('#edit-publication-primaryauthor')
        .val(data.doi_info.primary ?? '')
        .removeClass('error');
      $('#edit-publication-abstract')
        .val(data.doi_info.abstract ?? '')
        .removeClass('error');
      $('#edit-publication-title')
        .val(data.doi_info.title ?? '')
        .removeClass('error');
      // @TODO Get Journal field value.
      // $('#edit-publication-journal').val(data.doi_info.journal);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Fill select boxes.
      // Default value for year is '0' ('- Select -').
      $('#edit-publication-year')
        .val(data.doi_info.year ?? 0)
        .removeClass('error');
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Secondary Authors.
      if (data.doi_info.secondaryNumber > 0) {
        // @TODO Probably outdated. Check it.
        $('input[name="publication[secondaryAuthors][check]"]')
          .val(data.doi_info.secondaryCheck)
          .removeClass('error');
        $('input[name="publication[secondaryAuthors][number]"]').val(
          parseInt(data.doi_info.secondaryNumber) - 1
        );
        $('input[id^="edit-publication-secondaryauthors-add"]').mousedown();
        $.each(data.doi_info.secondary, function( key, value ) {
          Drupal.waitForElm('input[name="publication[secondaryAuthors]['
            + ( key + 1 ) + ']"]').then(
              (elm) => { $(elm).val(value); }
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
          $('input[name="organism[1][name]"]')
            .val(data.doi_info.species[0])
            .removeClass('error');
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

  /**
   * Label fields as optional (not required).
   *
   * Asterisk will be removed to the field label.
   */
  Drupal.tpps.makePublicationFieldsOptional = function() {
    let publicationFieldList = ['year', 'title', 'abstract', 'journal'];
    for (let i in publicationFieldList) {
      let $label = $('#edit-publication-' + publicationFieldList[i])
        .parents('.form-item').find('label');
      if ($label.length) {
        $label.html($label.html().replace(' *', ''));
      }
    }
  }

  /**
   * Label fields as required.
   *
   * Asterisk will be added to the field label.
   */
  Drupal.tpps.makePublicationFieldsRequired = function() {
    let publicationFieldList = ['year', 'title', 'abstract', 'journal'];
    for (let i in publicationFieldList) {
      let $label = $('#edit-publication-' + publicationFieldList[i])
        .parents('.form-item').find('label');
      if ($label.length) {
        $label.html(
          // To avoid duplication of asterisk for field which was hidden
          // but Drupal Form API States we force removement of existing
          // asterisks (if any). It's better then duplicate Drupal Form
          // API States and track field's visability.
          $label.html().replace(' *', '')
          + ' *'
        );
      }
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Behavior.
  Drupal.behaviors.tpps_page_1 = {
    attach: function (context, settings) {
      // Clear value of last 'Secondary Author' field on 'Remove' button click.
      // Must be reattached to every new form part loaded via AJAX.
      // @TODO Probably tpps_dynamic_list() must be updated.
      $('input[id^="edit-publication-secondaryauthors-add"]').on('click', function(e) {
          let number = $('input[name="publication[secondaryAuthors][number]"]').val();
          let selector = 'input[id="edit-publication-secondaryauthors-' + (parseInt(number) + 1) + '"]';
          if (number >= 0) {
            Drupal.waitForElm(selector).then((elm) => { $(elm).val(''); });
          }
        }
      );

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Attach event handlers only once.
      $('form[id^=tppsc-main]').once('tpps_page_1', function() {
        if ($('#edit-publication-status').val() == 'In Preparation or Submitted') {
          var $label = $('input[name="publication[publication_doi]"]').parent().find('label');
          $label.html($label.html().replace(' *', ''));
          Drupal.tpps.makePublicationFieldsOptional();
        }

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Reset form if status != 'Published'.
        $('#edit-publication-status').on('change', function(e) {
          if ($(this).val() == 'In Preparation or Submitted') {
            // Remove '*' from 'Publication DOI' field because it's optional.
            var $label = $('input[name="publication[publication_doi]"]').parents('.form-item').find('label');
            $label.html($label.html().replace(' *', ''));
            Drupal.tpps.makePublicationFieldsOptional();
          }
          else if ($(this).val() == 'Published') {
            // 'Publication DOI' field became required.
            var $label = $('input[name="publication[publication_doi]"]').parents('.form-item').find('label');
            // To avoid duplication of asterisk for field which was hidden
            // but Drupal Form API States we force removement of existing
            // asterisks (if any). It's better then duplicate Drupal Form
            // API States and track field's visability.
            $label.html($label.html().replace(' *', '') + ' *');
            Drupal.tpps.makePublicationFieldsRequired();
          }
          else {
            Drupal.tpps.resetForm();
          }
          $(this).removeClass('error');
        });

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // @TODO Minor. Create more common solution to reuse this code
        // for other fields.
        // Strip HTML tags from 'Dataset DOI' field value.
        $('input[name="publication[dataset_doi"]').blur(function(e) {
          $(this).val(Drupal.tpps.stripHtml($(this).val()));
        });

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // 'Publication DOI' field change.
        $(doiSelector).blur(function(e) {
          e.preventDefault();
          Drupal.tpps.fieldDisable(doiSelector);

          // Strip HTML tags.
          $(this).val(Drupal.tpps.stripHtml($(this).val()));
          if (typeof(settings.tpps.ajaxUrl) !== 'undefined') {
            var doi = $(this).val();

            // Check if DOI value really was changed.
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
            // Check if DOI value is empty.
            if (typeof (doi) == 'undefined' || doi == '') {
              $(doiMessageBox).empty();
              var data = {"errors": [Drupal.t('Empty DOI.')]};
              Drupal.tpps.ShowMessages(doiMessageBox, data);
              Drupal.tpps.fieldEnable(doiSelector);
              return;
            }
            // Check DOI format.
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
              Drupal.tpps.ClearMessages(doiMessageBox);
              var data = Drupal.tpps.doi[doi];
              Drupal.tpps.ShowMessages(doiMessageBox, data);
              Drupal.tpps.fieldEnable(doiSelector);
              Drupal.tpps.doiFill(data);
            }
            else {
              var url = settings.basePath + settings.tpps.ajaxUrl + '/get_doi';
              // Remove existing messages.
              Drupal.tpps.ClearMessages(doiMessageBox);
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
          }
        });
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      });
    }
  }
})(jQuery, Drupal);
