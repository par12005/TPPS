/**
 * @file
 *
 * TPPS Page 1 form specific JS-code.
 */
(function($, Drupal) {
  var doiSelector = 'input[name="publication[publication_doi]"]';
  var doiMessageBox = '#doi-message';
  // Regex to check if string contains only one space.
  var checkSingleSpace = /^[^\s]*\s[^\s]*$/;

  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};
  Drupal.tpps.doi = Drupal.tpps.doi || {};

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


  /**
   * Validates organism name using NCBI database to get taxonomy id.
   */
  Drupal.tpps.validateOrganismName = function(event) {
    let featureName = 'Drupal.tpps.validateOrganismName';
    let fieldId = 'edit-organism-' + event.data.organismId + '-name';
    let fieldSelector = '#' + fieldId;
    let $field = $(fieldSelector);
    let below = true;

    dog('OrganismId: ' + event.data.organismId, featureName);
    if ($field.length == 0) {
      dog('Empty Organism Name field', featureName);
      return;
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get fid from managed file field.
    let organismName = $field.val().trim();
    dog('Name of the organism: ' + organismName + '.', featureName);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if value really was changed.
    if (
      typeof (Drupal.tpps.lastValue[fieldId]) != 'undefined'
      && Drupal.tpps.lastValue[fieldId] == organismName
    ) {
      dog('Value was\'t changed.', featureName);
      Drupal.tpps.fieldEnable(fieldSelector);
      return;
    }
    else {
      // Store current value to be able to compare with new one later.
      dog('Value was changed.', featureName);
      dog('Store current value to be able to compare with new one later.', featureName);
      Drupal.tpps.lastValue[fieldId] = organismName;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Basic validation: check if single space exists.
    if (!checkSingleSpace.test(organismName)) {
      dog('Basic validation failed.', featureName);
      dog('Organism name must contain both genus and species separated '
        + 'with space.', featureName);
      Drupal.tpps.clearMessages(fieldSelector);
      Drupal.tpps.showMessages(fieldSelector, {
        'errors': [
          Drupal.t('Wrong organism name. Valid format: "[genus] [species]".'),
        ]
      }, below);
      return;
    }
    dog('Basic validation passed.', featureName);
    Drupal.tpps.clearMessages(fieldSelector);
    Drupal.tpps.showMessages(fieldSelector, {
      'statuses': [Drupal.t('Organism name is valid.')]
    }, below);
    Drupal.tpps.fieldDisable(fieldSelector);


    let url = Drupal.settings.basePath + Drupal.settings.tpps.ajaxUrl
      + '/get_ncbi_taxonomy_id';
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Requiest NCBI id for organism.
    $.ajax({
      method: 'post',
      data: {'organism': organismName},
      url: url,
      error: function (jqXHR, textStatus, errorThrown) {
        // User changed value of the field during AJAX-request.
        if (Drupal.tpps.wasValueChanged(fieldId, organismName)) { return; }
        // Server/Network errors.
        let data = {
          'errors': [Drupal.t("Organism name validation failed.")]
        };
        Drupal.tpps.showMessages(fieldSelector, data, below);

        let errorMessage = jqXHR.status + " " + jqXHR.statusText
          + "\n\n" + jqXHR.responseText;
        console.log(errorMessage);
        dog("Organism name wasn't validated.", data, featureName)
        Drupal.tpps.fieldEnable(fieldSelector);
      },

      success: function(data) {
        // Store DOI check result to avoid multiple requests.
        // @TODO Update.
        // Note: DOI field stores AJAX-request response to avoid multiple requests.
        // Drupal.tpps.doi[doi] = data;


        // User changed value of the field during AJAX-request.
        if (Drupal.tpps.wasValueChanged(fieldId, organismName)) { return; }
        if (typeof (data) == 'undefined') {
          var data = {
            'errors': [Drupal.t('Received empty response.')],
          };
        }
        if (below) {
          // When messages are shown below selector then newer message appears
          // at the top of previous (old one) messages which look not good.
          // Let's just clear not important messages about basic pre-validation.
          Drupal.tpps.clearMessages(fieldSelector);
        }
        Drupal.tpps.showMessages(fieldSelector, data);
        Drupal.tpps.fieldEnable(fieldSelector);
      }
    });
  }


  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Behavior.
  Drupal.behaviors.tppsPage1 = {
    attach: function (context, settings) {
      var featureName = 'Drupal.behaviors.tppsPage1';
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
      // Validate species names using NCBI Taxomony.
      var organismNumber = Drupal.settings.tpps.organismNumber;
      dog('Number of organisms: ' + organismNumber, featureName);
      // Loop species fields.
      for (let organismId = 1; organismId <= organismNumber; organismId++) {
        $('input#edit-organism-' + organismId + '-name', context).on(
          'blur',
          // @TODO Minor. Get organism Id from event object in callback.
          {'organismId': organismId},
          Drupal.tpps.validateOrganismName
        );
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Attach event handlers only once.
      $('form[id^=tppsc-main]').once('tpps-page-1-processed', function() {
        if ($('#edit-publication-status').val() == 'In Preparation or Submitted') {
          var $label = $('input[name="publication[publication_doi]"]')
            .parent().find('label');
          $label.html($label.html().replace(' *', ''));
          Drupal.tpps.makePublicationFieldsOptional();
        }

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Reset form if status != 'Published'.
        $('#edit-publication-status').on('change', function(e) {
          if ($(this).val() == 'In Preparation or Submitted') {
            // Remove '*' from 'Publication DOI' field because it's optional.
            var $label = $('input[name="publication[publication_doi]"]')
              .parents('.form-item').find('label');
            $label.html($label.html().replace(' *', ''));
            Drupal.tpps.makePublicationFieldsOptional();
          }
          else if ($(this).val() == 'Published') {
            // 'Publication DOI' field became required.
            var $label = $('input[name="publication[publication_doi]"]')
              .parents('.form-item').find('label');
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
              typeof (Drupal.tpps.lastValue['doi']) != 'undefined'
              && Drupal.tpps.lastValue['doi'] == doi
            ) {
              Drupal.tpps.fieldEnable(doiSelector);
              return;
            }
            else {
              // Store current DOI value to be able to compare with new one later.
              Drupal.tpps.lastValue['doi'] = doi;
            }
            // Check if DOI value is empty.
            if (typeof (doi) == 'undefined' || doi == '') {
              $(doiMessageBox).empty();
              var data = {"errors": [Drupal.t('Empty DOI.')]};
              Drupal.tpps.showMessages(doiMessageBox, data);
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
              Drupal.tpps.showMessages(doiMessageBox, data);
              Drupal.tpps.fieldEnable(doiSelector);
              return;
            }

            // Check if we have cached result first.
            if (Drupal.settings.tpps.cache && typeof (Drupal.tpps.doi[doi]) != 'undefined') {
              Drupal.tpps.clearMessages(doiMessageBox);
              var data = Drupal.tpps.doi[doi];
              Drupal.tpps.showMessages(doiMessageBox, data);
              Drupal.tpps.fieldEnable(doiSelector);
              Drupal.tpps.doiFill(data);
            }
            else {
              var url = Drupal.settings.basePath + Drupal.settings.tpps.ajaxUrl
                + '/get_doi';
              // Remove existing messages.
              Drupal.tpps.clearMessages(doiMessageBox);
              $.ajax({
                method: 'post',
                data: {'doi': doi},
                url: url,
                error: function (jqXHR, textStatus, errorThrown) {
                  // User changed value of the field during AJAX-request.
                  if (Drupal.tpps.wasValueChanged('doi', doi)) { return; }
                  // Server/Network errors.
                  var data = [{
                    "errors": [Drupal.t("DOI value wasn't completed")]
                  }];
                  Drupal.tpps.showMessages(doiMessageBox, data);

                  var errorMessage = jqXHR.status + " " + jqXHR.statusText
                    + "\n\n" + jqXHR.responseText;
                  console.log(errorMessage);
                  Drupal.tpps.fieldEnable(doiSelector);
                },
                success: function(data) {
                  // Store DOI check result to avoid multiple requests.
                  Drupal.tpps.doi[doi] = data;
                  // User changed value of the field during AJAX-request.
                  if (Drupal.tpps.wasValueChanged('doi', doi)) { return; }
                  if (typeof (data) == 'undefined') {
                    var data = [{
                      "errors": [Drupal.t('Received empty response.')]
                    }];
                  }
                  Drupal.tpps.showMessages(doiMessageBox, data);
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
