/**
 * @file
 *
 * TPPS Page 1 form specific JS-code.
 */

/* global Drupal, jQuery, dog, context, settings */
(function($, Drupal) {
  var doiSelector = 'input[name="publication[publication_doi]"]';
  var doiMessageBox = '#doi-message';

  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};

  /**
   * Fills Page 1 form with empty values.
   */
  Drupal.tpps.resetForm = function() {
    $('#edit-publication-primaryauthor').val('');
    $('#edit-publication-abstract').val('');
    // Default value for year is '0' ('- Select -').
    $('#edit-publication-year').val(0);
    $('#edit-publication-title').val('');
    $('#edit-publication-journal').val('');
    // Secondary Authors.
    if ($('input[name="publication[secondaryAuthors][number]"]').val() != 0) {
      // Set number of organisms to 0 and force removement of the extra fields.
      $('input[name="publication[secondaryAuthors][number]"]')
        .val(0)
        // Remove empty extra secondary author's fields.
        .parent()
        .find('input[name="Remove Secondary Author"]')
        .mousedown()
        // Hide existing secondary author's fields while form reloads.
        .parent()
        .find('.form-type-textfield') // form-item-publication-secondaryAuthors-1
        .hide();
    }
    // Organism.
    $('input[name="organism[1][name]"]').val('');
    if ($('input[name="organism[number]"]').val() != 1) {
      // Set number of organisms to 1 and force removement of the extra fields.
      $('input[name="organism[number]"]')
        .val(1)
        .parent()
        .find('input[name="Remove Organism"]')
        .mousedown();
    }
    if ($.isFunction(Drupal.tpps.clearMessages)) {
      Drupal.tpps.clearMessages('#doi-message');
      Drupal.tpps.clearMessages('input[name="organism[1][name]"]');
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
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Fill text fields.
      let primaryAuthor = $.trim(data.authors.split(',')[0]);
      $('#edit-publication-primaryauthor').val(primaryAuthor ?? '').removeClass('error');

      $('#edit-publication-abstract').val(data.abstract ?? '').removeClass('error');
      $('#edit-publication-title').val(data.title ?? '').removeClass('error');
      $('#edit-publication-journal').val(data.journal ?? '').removeClass('error');
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Fill select boxes.
      // Default value for year is '0' ('- Select -').
      let year = data.date.match(/^\d{4}/)[0];
      $('#edit-publication-year').val(year ?? 0).removeClass('error');
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Secondary Authors.
      let secondaryAuthors = data.authors.split(',')
        .slice(1).map(item => item.trim());
      if (secondaryAuthors.length > 0) {
        // @TODO Probably outdated. Check it.
        //$('input[name="publication[secondaryAuthors][check]"]')
        //  .val(data.doi_info.secondaryCheck)
        //  .removeClass('error');
        $('input[name="publication[secondaryAuthors][number]"]').val(
          parseInt(secondaryAuthors.length) - 1
        );
        $('input[id^="edit-publication-secondaryauthors-add"]').mousedown();
        $.each(secondaryAuthors, function( key, value ) {
          Drupal.waitForElm('input[name="publication[secondaryAuthors]['
            + ( key + 1 ) + ']"]').then(
              (elm) => { $(elm).val(value); }
          );
        });
      }


// @TODO Update.

      // Organisms.
      let species = data.species.split(',').map(item => item.trim());
      if (species.length > 0) {
        // @TODO Reuse empty fields.
        // @TODO Not overwrite 1st field if it's not empty.
        // 1st field is present at form and can't be removed.
        // So it must be populated manually.
        if (typeof species[0] == 'undefined') {
          $('input[name="organism[1][name]"]').val('');
        }
        else {
          $('input[name="organism[1][name]"]').val(species[0]).removeClass('error');
        }
        // Add necessary number of fields.
        $('input[name="organism[number]"]').val(species.length - 1 );
        $('input[id^="edit-organism-add"]').mousedown();
        // Populate new fields.
        $.each(species, function( key, value ) {
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
  Drupal.tpps.validateOrganismName = function() {
    var featureName = 'Drupal.tpps.validateOrganismName';
    // Format: organism[1][name]
    let organismId = $(this).attr('name').split('[')[1].replace(']', '');

    if ($(this).hasClass('validateOrganismName')) {
      console.log('skipped' + organismId);
      return;
    }

    // WARNING:
    // Each time new field added or exising field removed HTML-id is changed
    // so HTML-id can't be used to find fields.
    let fieldId = 'edit-organism-' + organismId + '-name';
    let fieldName = 'organism[' + organismId + '][name]';
    let fieldSelector = 'input[name="' + fieldName+ '"]';
    let $field = $(fieldSelector);
    // Show messages below or above field.
    let below = true;

    dog('OrganismId: ' + organismId, featureName);
    if ($field.length == 0) {
      dog('Organism field not found.', featureName);
      return;
    }

    // Clean-up HTML from field's value.
    $field.val(Drupal.tpps.stripHtml($field.val().trim()));
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get fid from managed file field.
    let organismName = $field.val();
    if (typeof organismName == 'undefined' || organismName.length == 0) {
      dog('Empty organism name.', featureName);
      Drupal.tpps.clearMessages(fieldSelector);
      return;
    }
    dog('Name of the organism: ' + organismName + '.', featureName);
    $field.addClass('validateOrganismName');

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Check if value really was changed.
    if (
      typeof (Drupal.tpps.lastValue[fieldId]) != 'undefined'
      && Drupal.tpps.lastValue[fieldId] == organismName
    ) {
      dog('Value was\'t changed.', featureName);
      if (
        'ajaxCache' in Drupal.tpps
        && typeof (Drupal.tpps.ajaxCache[organismName]) != 'undefined'
      ) {
        dog('AJAX-request response found in cache.', featureName);
        Drupal.tpps.clearMessages(fieldSelector);
        var data = Drupal.tpps.ajaxCache[organismName];
        Drupal.tpps.showMessages(fieldSelector, data);
        Drupal.tpps.fieldEnable(fieldSelector);
        $(this).removeClass('validateOrganismName');
        return;
      }
      else {
        dog('No AJAX-request response found in cache.', featureName);
      }
    }
    else {
      // Store current value to be able to compare with new one later.
      dog('Value was changed.', featureName);
      dog('Store current value to be able to compare with new one later.', featureName);
      Drupal.tpps.lastValue[fieldId] = organismName;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Basic validation: check if single space exists.
    if (!Drupal.tpps.isValid('organismName', organismName)) {
      dog('Basic validation failed.', featureName);
      dog('Organism name "' + organismName + '" is invalid.', featureName);
      Drupal.tpps.clearMessages(fieldSelector);
      Drupal.tpps.showMessages(fieldSelector, {
        'errors': [Drupal.t('Organism name is invalid.')]
      }, below);
      $(fieldSelector).removeClass('validateOrganismName');
      return;
    }
    dog('Basic validation passed.', featureName);

    Drupal.tpps.clearMessages(fieldSelector);
    Drupal.tpps.showMessages(fieldSelector, {
      'statuses': [Drupal.t('Organism name is valid.')]
    }, below);
    Drupal.tpps.fieldDisable(fieldSelector);

    // Check if there is a cached AJAX-request response.
    if (
      'ajaxCache' in Drupal.tpps
      && typeof (Drupal.tpps.ajaxCache[organismName]) != 'undefined'
    ) {
      $(fieldSelector).removeClass('validateOrganismName');
      dog('AJAX-request response found in cache.', featureName);
      Drupal.tpps.clearMessages(fieldSelector);
      var data = Drupal.tpps.ajaxCache[organismName];
      Drupal.tpps.showMessages(fieldSelector, data);
      Drupal.tpps.fieldEnable(fieldSelector);
    }
    else {
      //let url = Drupal.settings.basePath + Drupal.settings.tpps.ajaxUrl
      //  + '/get_ncbi_taxonomy_id';
      let url = Drupal.settings.basePath + Drupal.settings.tpps.ajaxUrl + '/getNcbiTaxonomyId';
      // Disable button 'Next' because organism fields won't pass validation.
      $('input.next-button').attr('disabled','disabled');
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Requiest NCBI id for organism.
      $.ajax({
        method: 'post',
        data: {'organism': organismName},
        url: url,
        error: function (jqXHR, textStatus, errorThrown) {
          $(fieldSelector).removeClass('validateOrganismName');
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
          $(fieldSelector).removeClass('validateOrganismName');

console.log(fieldSelector);

          // Store response to avoid multiple requests.
          Drupal.tpps.ajaxCache[organismName] = data;
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
          // Make button 'Next' active again.
          $('input.next-button').removeAttr('disabled');
        }
      });
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Behaviors.
  Drupal.behaviors.tppsPage1 = {
    attach: function (context, settings) {
      var featureName = 'Drupal.behaviors.tppsPage1';
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Clear value of last 'Secondary Author' field on 'Remove' button click.
      // Must be reattached to every new form part loaded via AJAX.
      // @TODO Probably tpps_dynamic_list() must be updated.
      $('input[id^="edit-publication-secondaryauthors-add"]', context)
        .on('click', function() {
          let number = $('input[name="publication[secondaryAuthors][number]"]').val();
          let selector = 'input[id="edit-publication-secondaryauthors-' + (parseInt(number) + 1) + '"]';
          if (number >= 0) {
            Drupal.waitForElm(selector).then((elm) => { $(elm).val(''); });
          }
        });

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Validate species names using NCBI Taxomony.
      // Get number of organisms on page. Since this number could be changed
      // on Page 1 we can't use Drupal.settings.tpps.organismNumber here.
      var organismNumber = $('input[name="organism[number]"]').val();
      dog('Total number of organisms on page: ' + organismNumber, featureName);
      // Loop species fields.
      for (let organismId = 1; organismId <= organismNumber; organismId++) {
        let $field = $('input[name="organism[' + organismId + '][name]"]', context);
        if ($field.length !== 0) {
          $field.on('blur', Drupal.tpps.validateOrganismName);
          // Force validation of the non-empty fields on page load or after
          // adding new organism field.
          if ($field.val()) {
            $field.trigger('blur');
          }
        }
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Attach event handlers only once.
      var $label = '';
      $('form[id^=tppsc-main]').once('tpps-page-1-processed', function() {
        if ($('#edit-publication-status').val() == 'In Preparation or Submitted') {
          $label = $('input[name="publication[publication_doi]"]')
            .parent().find('label');
          $label.html($label.html().replace(' *', ''));
          Drupal.tpps.makePublicationFieldsOptional();
        }

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Reset form if status != 'Published'.
        $('#edit-publication-status', context).on('change', function() {
          if ($(this).val() == 'In Preparation or Submitted') {
            // Remove '*' from 'Publication DOI' field because it's optional.
            $label = $('input[name="publication[publication_doi]"]')
              .parents('.form-item').find('label');
            $label.html($label.html().replace(' *', ''));
            Drupal.tpps.makePublicationFieldsOptional();
          }
          else if ($(this).val() == 'Published') {
            // 'Publication DOI' field became required.
            $label = $('input[name="publication[publication_doi]"]')
              .parents('.form-item').find('label');
            // To avoid duplication of asterisk for field which was hidden
            // but Drupal Form API States we force removement of existing
            // asterisks (if any). It's better then duplicate Drupal Form
            // API States and track field's visability.
            $label.html($label.html().replace(' *', '') + ' *');
            Drupal.tpps.makePublicationFieldsRequired();

          }
          else {
            // Nothing to do...
          }
          Drupal.tpps.resetForm();
          $(this).removeClass('error');
        });

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // @TODO Minor. Create more common solution to reuse this code
        // for other fields.
        // Strip HTML tags from 'Dataset DOI' field value.
        $('input[name="publication[dataset_doi"]').blur(function() {
          $(this).val(Drupal.tpps.stripHtml($(this).val()));
        });
      });
    }
  }
})(jQuery, Drupal);
