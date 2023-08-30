(function ($) {
  Drupal.behaviors.tpps_page_4 = {
    attach: function (context, settings) {
      // TPPS Form Page 4 Buttons.
      // @TODO Avoid duplicate code if possible.
      // @TODO [VS] Convert icons to unicode symbols.

      // Start of 'Check VCF Tree Ids'.
      $('.button-check-vcf-tree-ids', context).on('click', function(e) {
        e.preventDefault();
        $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking VCF Tree IDs...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession + '/vcf-tree-ids',
          error: function (err) {
            $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that this VCF '
                + 'is just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '<div>🎄 Unique trees found: '
                + data['unique_count'] + ' | '
                + '🎄 Total trees found: ' + data['count'] + '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate tree IDs in this VCF '
                  + 'since unique count does not match count</div>';
                  + '<hr /><div>Duplicate Tree IDs ('
                  + data['duplicate_values'].length
                  + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate Tree IDs found in the VCF file</div>';
              }
              html += '<hr /><div>Unique Tree IDs (' + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">'
                  + data['values'][i] + '</div>';
              }
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html(html);
            }
            else {
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html('<h1 class=\"cd-inline\">🆘</h1>No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check VCF existence as well.'
                );
            }
          }
        });
      });
      // End of Click on 'Check VCF Tree Ids' button.
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Click 'Check VCF Markers' button.
      $('.button-check-vcf-markers', context).on('click', function(e) {
        e.preventDefault();
        $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking VCF Markers...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession + '/vcf-markers',
          error: function (err) {
            $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that this '
                + 'VCF is just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '<div>🧬 Unique markers found: '
                + data['unique_count'] + ' | '
                + '🧬 Total markers found: ' + data['count'] + '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate markers in this VCF '
                  + 'since unique count does not match count</div>'
                  + '<hr /><div>Duplicate Markers ('
                  + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate markers found in the VCF file</div>';
              }
              html += '<hr /><div>Unique Markers ('
                + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">'
                  + data['values'][i] + '</div>';
              }
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html(html);
            }
            else {
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html('<h1 class=\"cd-inline\">🆘</h1>No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check VCF existence as well.'
                );
            }
          }
        });
      });
      // End of Click 'Check VCF Markers' button.
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Start of 'Check VCF Markers'.
      $('.button-check-snps-assay-markers', context).on('click', function(e) {
        e.preventDefault();
        $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking SNPs Assay Markers...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession + '/snps-assay-markers',
          error: function (err) {
            $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that this '
                + 'SNPs Assay is just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '<div>🧬 Unique markers found: '
                + data['unique_count'] + ' | '
                + '🧬 Total markers found: ' + data['count'] + '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate markers in this SNPs '
                  + 'assay file since unique count does not match count</div>'
                  + '<hr /><div>Duplicate Markers ('
                  + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate markers found in the SNPs Assay file</div>';
              }
              html += '<hr /><div>Unique Markers ('
                + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">'
                  + data['values'][i] + '</div>';
              }
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId).html(html);
            }
            else {
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html('<h1 class=\"cd-inline\">🆘</h1> No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check SNPs Assay file existence as well.'
                );
            }
          }
        });
      });
      // End of Click 'Check VCF Markers' button.
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Start of 'Check Accession File Tree IDs'.
      $('.button-check-accession-file-tree-ids', context).on('click', function(e) {
        e.preventDefault();
        $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking Accession File Tree IDs...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession + '/accession-file-tree-ids',
          error: function (err) {
            $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that this '
                + 'accession file is just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '<div>🎄 Unique trees found: '
                + data['unique_count'] + ' | '
                + '🎄 Total trees found: ' + data['count'] + '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate tree IDs in this Accession '
                  + 'file since unique count does not match count</div>'
                  + '<hr /><div>Duplicate Tree IDs ('
                  + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate Tree IDs found in the Accession file</div>';
              }
              html += '<hr /><div>Unique Tree IDs ('
                + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">'
                  + data['values'][i] + '</div>';
              }
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html(html);
            }
            else {
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html('<h1 class=\"cd-inline\">🆘</h1>No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check Accession File existence as well.'
                );
            }
          }
        });
      });
    // End of 'Check Accession File Tree IDs'.
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Start of 'Compare Accession and VCF Tree IDs'.
      $('.button-compare-accession-tree-ids-vs-vcf-tree-ids', context).on('click', function(e) {
        e.preventDefault();
        $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
          .html('<h1 class=\"cd-inline\">⏰</h1>Comparing Accession Tree IDs and VCF Tree IDs...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession
            + '/compare-accession-file-vs-vcf-file-tree-ids',
          error: function (err) {
            $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that these '
                + 'files are just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            data = JSON.parse(data);
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '<div>🎄 None overlapping Accession trees found: '
                + data['tree_ids_not_in_accession_count'] + ' | '
                + '🎄 None overlapping VCF trees found: '
                + data['tree_ids_not_in_vcf_count'] + '</div>';
              if (
                data['tree_ids_not_in_accession'] !== null
                && data['tree_ids_not_in_accession'].length > 0
              ) {
                html += '<div>⚡ There are VCF trees that do not overlap '
                  + 'with the Accession file</div>';
                // html += '<hr /><div>Duplicate Tree IDs (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['tree_ids_not_in_accession'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['tree_ids_not_in_accession'][i] + '</div>';
                }
              }
              if (
                data['tree_ids_not_in_vcf'] !== null
                && data['tree_ids_not_in_vcf'].length > 0
              ) {
                html += '<div>⚡ There are Accession trees that do not '
                  + 'overlap with the VCF file</div>';
                // html += '<hr /><div>Duplicate Tree IDs (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['tree_ids_not_in_vcf'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['tree_ids_not_in_vcf'][i] + '</div>';
                }
              }
              // else {
              //   html += '<div>🆗 No duplicate Tree IDs found in the VCF file</div>';
              // }
              // html += '<hr /><div>Unique Tree IDs (' + data['values'].length + ')</div>';
              // for (var i=0; i<data['values'].length; i++) {
              //   html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              // }
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html(html);
            }
            else {
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html('<h1 class=\"cd-inline\">🆘</h1>No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check Accession and VCF existence as well.'
                );
            }
          }
        });
      });

    // End of 'Compare Accession and VCF Tree IDs'.
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Start of 'Compare VCF and SNPs Assay markers',
      $('.button-compare-vcf-makers-vs-snps-assay-markers', context).on('click', function(e) {
        e.preventDefault();
        $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
          .html('<h1 class=\"cd-inline\">⏰</h1>Comparing VCF and SNPs Assay Markers...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession
            + '/compare-vcf-markers-vs-snps-assay-markers',
          error: function (err) {
            $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that these '
                + 'files are just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            data = JSON.parse(data);
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '<div>🧬 None overlapping VCF markers found: '
                + data['markers_not_in_snps_assay_count'] + ' | '
                + '🧬 None overlapping SNPs Assay markers found: '
                + data['markers_not_in_vcf_count'] + '</div>';
              if (data['markers_not_in_snps_assay'].length > 0) {
                html += '<div>⚡ There are VCF markers that do not overlap '
                  + 'with the SNPs Assay file</div>';
                // html += '<hr /><div>Duplicate Markers (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['markers_not_in_snps_assay'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['markers_not_in_snps_assay'][i] + '</div>';
                }
              }
              if (data['markers_not_in_vcf'].length > 0) {
                html += '<div>⚡ There are SNPs Assay markers that do not '
                  + 'overlap with the VCF file</div>';
                // html += '<hr /><div>Duplicate Markers (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['markers_not_in_vcf'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['markers_not_in_vcf'][i] + '</div>';
                }
              }
              // else {
              //   html += '<div>🆗 No duplicate Tree IDs found in the VCF file</div>';
              // }
              // html += '<hr /><div>Unique Tree IDs (' + data['values'].length + ')</div>';
              // for (var i=0; i<data['values'].length; i++) {
              //   html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              // }
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html(html);
            }
            else {
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html('<h1 class=\"cd-inline\">🆘</h1>No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check Accession and VCF existence as well.'
                );
            }
          }
        });
      });

    // End of 'Compare VCF and SNPs Assay markers',
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Start of 'Check SNPs Design Markers'.
      $('.button-check-snps-design-markers', context).on('click', function(e) {
        e.preventDefault();
        $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking SNPs Design Markers...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession + '/assay-design-markers',
          error: function (err) {
            $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that this '
                + 'SNPs Design File is just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            if (!Array.isArray(data)) {
              var html = '<div>🧬 Unique markers found: '
                + data['unique_count'] + ' | '
                + '🧬 Total markers found: ' + data['count'] + '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate markers in this SNPs '
                  + 'design file since unique count does not match count</div>'
                  + '<hr /><div>Duplicate Markers ('
                  + data['duplicate_values'].length
                  + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">'
                    + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate markers found in the SNPs Design file</div>';
              }
              html += '<hr /><div>Unique Markers ('
                + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">'
                  + data['values'][i] + '</div>';
              }
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html(html);
            }
            else {
              $(Drupal.settings.tpps.curationDiagnosticResultsElementId)
                .html('<h1 class=\"cd-inline\">🆘</h1> No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check SNPs Assay file existence as well.'
                );
            }
          }
        });
      });
    // End of 'Check SNPs Design Markers'.
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }
  }
})(jQuery);
