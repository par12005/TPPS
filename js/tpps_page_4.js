(function ($) {
  Drupal.behaviors.tpps_page_4 = {
    attach: function (context, settings) {
      // TPPS Form Page 4 Buttons.

      // Click on 'Check VCF Tree Ids' button.
      // @TODO [VS] Convert icons to unicode symbols.
      $('.button-check-vcf-tree-ids', context).on('click', function(e) {
        e.preventDefault();
        $('#diagnostic-curation-results')
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking VCF Tree IDs...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession + '/vcf-tree-ids',
          error: function (err) {
            $('#diagnostic-curation-results')
              .html('<h1 class=\"cd-inline\">🆘</h1>'
                + 'It might be that this VCF is just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            if (!Array.isArray(data)) {
              var html = '';
              html += '<div>';
              html += '🎄 Unique trees found: ' + data['unique_count'];
              html += ' | ';
              html += '🎄 Total trees found: ' + data['count'];
              html += '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate tree IDs in this VCF '
                  + 'since unique count does not match count</div>';
                html += '<hr /><div>Duplicate Tree IDs (' + data['duplicate_values'].length + ')</div>';
                for (var i=0; i<data['duplicate_values'].length; i++) {
                  html += '<div class=\"cd-inline-round-red\">' + data['duplicate_values'][i] + '</div>';
                }
              }
              else {
                html += '<div>🆗 No duplicate Tree IDs found in the VCF file</div>';
              }
              html += '<hr /><div>Unique Tree IDs (' + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              }
              $('#diagnostic-curation-results').html(html);
            }
            else {
              $('#diagnostic-curation-results')
                .html('<h1 class=\"cd-inline\">🆘</h1>No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check VCF existence as well.'
                );
            }
          }
        });
      });
      // End of Click on 'Check VCF Tree Ids' button.
      //
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Click 'Check VCF Markers' button.
      $('.button-check-vcf-markers', context).on('click', function(e) {
        e.preventDefault();
        $('#diagnostic-curation-results')
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking VCF Markers...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession + '/vcf-markers',
          error: function (err) {
            $('#diagnostic-curation-results')
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that this VCF '
                + 'is just too big to process it in time. '
                + 'Please contact Administration.');
          },
          success: function (data) {
            if (!Array.isArray(data)) {
              var html = '';
              html += '<div>';
              html += '🧬 Unique markers found: ' + data['unique_count'];
              html += ' | ';
              html += '🧬 Total markers found: ' + data['count'];
              html += '</div>';
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
              html += '<hr /><div>Unique Markers (' + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              }
              $('#diagnostic-curation-results').html(html);
            }
            else {
              $('#diagnostic-curation-results')
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
      // Click 'Check VCF Markers' button.
      $('.button-check-snps-assay-markers', context).on('click', function(e) {
        e.preventDefault();
        $('#diagnostic-curation-results')
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking SNPs Assay Markers...');
        $.ajax({
          url: '/tpps/' + Drupal.settings.tpps.accession + '/snps-assay-markers',
          error: function (err) {
            jQuery('#diagnostic-curation-results')
              .html('<h1 class=\"cd-inline\">🆘</h1>It might be that this '
                + 'SNPs Assay is just too big to process it in time. '
                + 'Please contact Administration.'
              );
          },
          success: function (data) {
            if (!Array.isArray(data)) {
              // Data was returned, this is good
              var html = '';
              html += '<div>';
              html += '🧬 Unique markers found: ' + data['unique_count'];
              html += ' | ';
              html += '🧬 Total markers found: ' + data['count'];
              html += '</div>';
              if (data['unique_count'] != data['count']) {
                html += '<div>⚡ There are duplicate markers in this '
                  + 'SNPs assay file since unique count does not match count</div>';
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
              html += '<hr /><div>Unique Markers (' + data['values'].length + ')</div>';
              for (var i=0; i<data['values'].length; i++) {
                html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
              }
              $('#diagnostic-curation-results').html(html);
            }
            else {
              $('#diagnostic-curation-results')
                .html('<h1 class=\"cd-inline\">🆘</h1> No results returned, '
                  + 'make sure you saved valid data on this page and retry. '
                  + 'Double check SNPs Assay file existence as well.'
                );
            }
          }
        });
      });
      // End of Click 'Check VCF Markers' button.

    }
  }
})(jQuery);
