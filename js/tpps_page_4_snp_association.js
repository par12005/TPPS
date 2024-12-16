/**
 * @file
 *
 * Manages SNP Association file field on Page 4.
 */

(function ($) {
  Drupal.behaviors.tpps_page_4_snp_association = {
    attach: function (context, settings) {

      var featureName = 'Drupal.behaviors.tpps_page_4_snp_association';

      // Common namespace for all managed files.
      // @TODO Minor. Create a JS class for managed files.
      var ManagedFile = {};
      // SNP Association custom namespace.
      // @TODO Class SnpAssociation must extend ManagedFile class.
      var SnpAssociation = {};

      /**
       * Enables/Disables 'Year' option of the SNP Association column data type.
       *
       * @param int organismId
       *   Ordinal number of organism on page.
       * @param string action
       *   Posible values: 'enable' and 'disable'.
       */
      SnpAssociation.YearOption = function(organismId, action = 'disable') {
        let $snpAssociationColomnSelectList = ManagedFile.getColumnSelects(
          organismId, 'genotype-snps-snps-association'
        );
        if ($snpAssociationColomnSelectList.length) {
          // SNP Association file was uploaded and table with
          // column data type selectors exists.
          $yearOptions = $snpAssociationColomnSelectList.find('option[value="'
            + Drupal.settings.tpps.snpAssociation.dataTypeYear + '"]');

          if (action  == 'disable') {
            $yearOptions.attr('disabled', 'disabled');
          }
          else if (action  == 'enable') {
            $yearOptions.removeAttr('disabled');
          }
        }
      };

      /**
       * Syncs Phenotype Data file 'Year' option and SNP Association file option.
       *
       * SNP Association file column data type must have 'Year' option only
       * when Phenotype Data file has 'Year' column.
       *
       * @param object element
       *  Phenotype Data file column selection dropdown menu.
       */
      SnpAssociation.syncYearOption = function(element) {
        let organismId = $(element).parents('.form-managed-file')
          .prop('id').replace('edit-organism-', '')
          .replace('-phenotype-file-upload', '');
        // Check if any column has 'Year' selected.
        let $phenotypeDataColomnSelectList = ManagedFile
          .getColumnSelects(organismId, 'phenotype-file');
        // Default action is to have disabled 'Year' option of the
        // column data types dropdowns of SNP Association file field.
        // Which means 'Year' column data type of the Phenotype Data
        // file was NOT defined.
        let action = 'disable';
        // Check if any of Phenotype Data file column was defined
        // as 'Year' data type.
        if (
          $phenotypeDataColomnSelectList.find(":selected")
          .map(function() {return $(this).val();}).get()
          .includes(
            String(Drupal.settings.tpps.phenotypeData.dataTypeYear)
          )
        ) {
          // 'Year' column was defined.
          action = 'enable';
        }
        SnpAssociation.YearOption(organismId, action);

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Check if other organisms reuse this phenotype data file.
        // Note: 'action' could be reused.
        if (organismId == 1 && Drupal.settings.tpps.organismNumber >= 2) {
          for (let i = 1; i <= Drupal.settings.tpps.organismNumber; i++) {
            if (
              i == 1
              || $('[name="organism-' + i + '[genotype-repeat-check]"]').is(":checked")
            ) {
              continue;
            }
            // Non-first organism and checkbox 'Genotype is the same' not checked.
            // Disable by default.
            SnpAssociation.YearOption(i, action);
          }
        }
      };

      /**
       * Gets all dropdowns of the column selection widget.
       *
       * @params int organismId
       *   Ordinal number of organism on page.
       * @param string fieldName
       *   Part of fieldset id which includes path to field.
       */
      ManagedFile.getColumnSelects = function(organismId, fieldName) {
        return $('fieldset#edit-organism-' + organismId + '-'
          + fieldName + '-columns table .form-select');
      };


      //organism-2[phenotype-repeat-check]
      //organism-2[genotype-repeat-check]

      // SNP Association 'Year' option must be available only when
      // Phenotype Data file also has column 'Year'.
      for (let i = 1; i <= Drupal.settings.tpps.organismNumber; i++) {
        // Non-first organism has checkbox 'Genotype information is the same'
        // and when it's checked genotype fields (including SNP Association file
        // field) will not be shown at page so we don't need to process them.
        if (
          i >= 2
          && $('[name="organism-' + i + '[genotype-repeat-check]"]').is(":checked")
        ) {
          dog('Skipped because genotype is the same as 1st organism.', featureName);
          continue;
        }
        dog('Processing SNP Association file for ' + i + ' organism.', featureName);
        // Disable all 'Year' options by default so if SNP Association
        // file will be uploaded before Phenotype Data file 'Year' option
        // will be disabled.
        SnpAssociation.YearOption(i, 'disable');

        // Check phenotype data file field and update 'Year' option of
        // SNP Association file field.
        // When more then one organism in study and checkboxes
        // 'Phenotype the same' and/or 'Genotype the same' was checked
        // sync with phenotype data file for organism 1.
        let organismId = i;
        if (
          i >= 2
          && $('[name="organism-' + i + '[phenotype-repeat-check]"]').is(":checked")
        ) {
          organismId = 1;
        }
        dog('Sync SNP Association for organism ' + i
          + ' with phenotype data file for organism ' + organismId + '.',
          featureName
        );

        let phenotypeDataFileFieldsetId = 'edit-organism-' + organismId
          + '-phenotype-file-columns';
        $('fieldset#' + phenotypeDataFileFieldsetId + ' table .form-select')
          .on('change', function() {
            // @TODO Minor. Get element from even object to remove
            // function() wrapper.
            SnpAssociation.syncYearOption($(this));
          })
          .each(function() {
            SnpAssociation.syncYearOption($(this));
          });
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }
  }
})(jQuery);
