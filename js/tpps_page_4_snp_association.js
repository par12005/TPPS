/**
 * @file
 *
 * Manages SNP Association file field on Page 4.
 */

(function ($) {
  Drupal.behaviors.tpps_page_4_snp_association = {
    attach: function (context, settings) {

      var featureName = 'Drupal.behaviors.tpps_page_4_snp_association';
      dog('loaded', featureName);

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


// @TODO Implement case: More then one organism and checkboxes 'Phenotype
// the same' and/or 'Genotype the same' was checked. We need to use data from
// organism 1.
      //organism-2[phenotype-repeat-check]
      //organism-2[genotype-repeat-check]

      // SNP Association 'Year' option.
      for (let i = 1; i <= Drupal.settings.tpps.organismNumber; i++) {
        // Disable all 'Year' options by default so if SNP Association
        // file will be uploaded before Phenotype Data file 'Year' option
        // will be disabled.
        SnpAssociation.YearOption(i, 'disable');
        // SNP Association file field must have 'Year' option only when
        // Phenotype Data file has column 'Year'.
        // fieldset#edit-organism-1-phenotype-file-columns table.view select.form-select
        let phenotypeDataFileFieldsetId = 'edit-organism-' + i
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
