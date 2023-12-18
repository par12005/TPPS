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
              $('.form-select[name="organism-1[genotype][are_genotype_markers_identical]"]', $form)
              .change(function() {
                for (let i = 2; i <= settings.tpps.organismNumber; i++) {
                  $repeatCheckBox = $('input[name="organism-' + i
                    + '[genotype-repeat-check]"]', $form);
                  if ($(this).val() == 'yes') {
                    $repeatCheckBox.prop('checked', true).trigger('change');
                  }
                  else if ($(this).val() == 'no') {
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
            for (let i = 1; i <= settings.tpps.organismNumber; i++) {
              let markerTypeName = 'organism-' + i + '[genotype][marker-type][]';
              let $markerTypeField = $(':input[name="' + markerTypeName + '"]', $form);
              // Main field must be hidden.
              $markerTypeField.parent('.form-item').hide();
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
                      markers.splice($.inArray(markerName, markers), 1);
                    }
                  }
                  $markerTypeField.val(markers);
                });
              }
            }
          }
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          if (
            typeof(settings.tpps) != 'undefined'
            && typeof(settings.tpps.organismNumber) != 'undefined'
          ) {
            // We can't use Drupal States API because it doesn't work with
            // multiple select form elements.
            for (let i = 1; i <= settings.tpps.organismNumber; i++) {
              var organism_name = '#edit-organism-' + i;
              $(organism_name + '-genotype-snps-genotyping-design').parent().hide();
              $(organism_name + '-genotype-ssrscpssrs').parent().hide();
              $(organism_name + '-genotype-marker-type', context).bind('change', function() {
                if ($.inArray('SNPs', $(this).val()) !== -1) {
                  $(organism_name + '-genotype-snps').show();
                  $(organism_name + '-genotype-snps-genotyping-design').parent().show();
                }
                else {
                  $(organism_name + '-genotype-snps').hide();
                }

                if ($.inArray('SSRs/cpSSRs', $(this).val()) !== -1) {
                  $(organism_name + '-genotype-ssrscpssrs').parent().show();
                }
                else {
                  $(organism_name + '-genotype-ssrscpssrs').parent().hide();
                }
                if ($.inArray('Other', $(this).val()) !== -1) {
                  $(organism_name + '-genotype-other-marker').parent().show();
                }
                else {
                  $(organism_name + '-genotype-other-marker').parent().hide();
                }
              });
              // Hide all Genotype related fields by default.
              $(organism_name + '-genotype-marker-type').trigger('change');
            }
          }
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        }
      });
    }
  }
})(jQuery);
