(function ($) {
  Drupal.behaviors.tpps_page_4_genotype = {
    attach: function (context, settings) {
      // Attach event handlers only once.
      $('form[id^=tppsc-main]', context).once('tpps_page_4_genotype', function() {
        // WARNING:
        // When validation failed Form Id is changed. This shouldn't happen
        // and must be fixed at PHP/Drupal side but here is a termporary
        // workaround solution to have it working.
        let $form = $(this);
        if (typeof($form) != 'undefined' && $form.length != 0) {
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // General questions on Genotype Subform.
          // When 'Are your genotype markers identical accross species?' was
          // changed we need to update all '*-repeat-check' genotype checkboxes.
          if (
            typeof(settings.tpps) != 'undefined'
            && typeof(settings.tpps.organismNumber) != 'undefined'
          ) {
            if (settings.tpps.organismNumber > 1) {
              // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
              // Check if genotype data is identical.
              repeatCounter = 1;
              // Note: 1st organism doesn't have checkbox 'genotype-repeat-check'.
              for (let i = 2; i <= settings.tpps.organismNumber; i++) {
                $repeatCheckBox = $('input[name="organism-' + i + '[genotype-repeat-check]"]', $form);
                repeatCounter = +repeatCounter + +$repeatCheckBox.prop('checked');
              }
              if (settings.tpps.organismNumber == repeatCounter) {
                // Only when all organisms has the checkbox checked.
                $('.form-select[name="organism-1[genotype][are_genotype_markers_identical]"]', $form).val('yes');
              }
              else if (repeatCounter) {
                // If at least some of them was set then not identical for sure.
                $('.form-select[name="organism-1[genotype][are_genotype_markers_identical]"]', $form).val('no');
              }
              // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
              $('.form-select[name="organism-1[genotype][are_genotype_markers_identical]"]', $form)
              .blur(function() {
                let value = $(this).val();
                for (let i = 2; i <= settings.tpps.organismNumber; i++) {
                  $repeatCheckBox = $('input[name="organism-' + i
                    + '[genotype-repeat-check]"]', $form);
                  if (value == 'yes') {
                    $repeatCheckBox.prop('checked', true).trigger('change');
                  }
                  else if (value == 'no') {
                    $repeatCheckBox.prop('checked', false).trigger('change');
                  }
                }
              });
              // When manually disabled 'Repeat Check' for non-first genotype
              // then set to 'No' selectbox with title
              // "Are your genotype markers identical accross species?"
              for (let i = 2; i <= settings.tpps.organismNumber; i++) {
                let fieldName = 'genotype-repeat-check';
                $('input[name="organism-' + i + '[' + fieldName + ']"]', $form)
                .click(function() {
                  if (! this.checked) {
                    $('.form-select[name="organism-1[genotype][are_genotype_markers_identical]"]', $form).val('no');
                  }
                });
              }
            }
            // Replacement for selectbox 'Marker Type'.
            // We have 3 select boxes instead of 2 selectbox which allows
            // multiple selections. Original selectbox 'Marker Type' was left
            // to avoid changes in submit_all.php and merge conflicts.
            //
            // organism-1[genotype][does_study_include_other_genotypic_data]
            // organism-1[genotype][does_study_include_snp_data]
            // organism-1[genotype][does_study_include_ssr_cpssr_data]
            for (let i = 1; i <= settings.tpps.organismNumber; i++) {
              let markerTypeName = 'organism-' + i + '[genotype][marker-type][]';
              let $markerTypeField = $(':input[name="' + markerTypeName + '"]', $form);
              // Main field must be hidden.
              // @TODO Remove debug code. Uncomment.
              // $markerTypeField.parent('.form-item').hide();

              for (const [fieldName, markerName] of Object.entries(settings.tpps.markerTypeFieldList)) {
                let $field = $(':input[name="organism-' + i + '[genotype][' + fieldName + ']"]', $form);
                $field.change(function() {
                  let markers = $markerTypeField.val();
                  if (jQuery.isEmptyObject(markers)) {
                    if ($field.val() == 'yes') {
                      // Add.
                      markers = [markerName];
                    }
                    else {
                      // Remove.
                      markers = [];
                    }
                  }
                  else {
                    if ($field.val() == 'yes') {
                      // Add.
                      markers.push(markerName);
                    }
                    else {
                      // Remove.
                      let index = markers.indexOf(markerName);
                      // Need to check index because when 'no' item is not
                      // in the list and index will be '-1' which cause
                      // all items removement from the list.
                      if (index >= 0) {
                        markers.splice(index, 1);
                      }
                    }
                  }
                  $markerTypeField.val(markers).trigger('change');
                });
              }
            }
          }
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          if (
            typeof(settings.tpps) != 'undefined'
            && typeof(settings.tpps.organismNumber) != 'undefined'
            && typeof(settings.tpps.ploidyDescriptions) != 'undefined'
          ) {
            // We can't use Drupal States API because it doesn't work with
            // multiple select form elements.
            for (let i = 1; i <= settings.tpps.organismNumber; i++) {
              // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
              // Update 'Ploidy' field description.
              let ploidySelector = '[name="organism-' + i + '[genotype]['
                + settings.tpps.ssrsFieldset + '][ploidy]"]';
              $(ploidySelector).change(function() {
                for (const [value, description] of Object.entries(settings.tpps.ploidyDescriptions)) {
                  if ($(this).val() == value) {
                    for (const ssrFieldName of settings.tpps.ssrFields) {
                      let ssrSelector = '[name="organism-' + i + '[genotype]['
                        + settings.tpps.ssrsFieldset + '][' + ssrFieldName +']"]';
                      //console.log($(ssrSelector));
                      //console.log(
                      //  $(ssrSelector).parents('.form-item').find('.description').html()
                      //);
                      // @TODO Add 'description' with leading '<br/>
                      // console.log( $(this).val());
                      // console.log(ssrFieldName);
                      // console.log( description );
                    }
                  }
                }
              });
              // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
              // Change name of the other file-field when dropdown changes.
              let otherMarkerSelector = '[name="organism-' + i
                + '[genotype][other][other-marker]';
              $(otherMarkerSelector).change(function() {
                let name = $(this).val();
                let $label = $('#edit-organism-' + i
                  + '-genotype-other-other-upload').prev('label');
                $label.html($label.html()
                  .replace(/^.* (spreadsheet\::?\s)/, $(this).val() + ' $1')
                );
              });
              $(otherMarkerSelector).change();
              // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
            }
          }
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        }
      });
    }
  }
})(jQuery);
