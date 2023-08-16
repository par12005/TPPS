(function ($) {
  Drupal.behaviors.tpps_page_4 = {
    attach: function (context, settings) {
      // TPPS Form Page 4 Buttons.

      // Click on 'Check VCF Tree Ids' button.
      $('.button-check-vcf-tree-ids', context).on('click', function(e) {
        console.log('clicked');
        e.preventDefault();
        $('#diagnostic-curation-results')
          .html('<h1 class=\"cd-inline\">⏰</h1>Checking VCF Tree IDs...');
        $.ajax({
          url: '/tpps/" . $accession . "/vcf-tree-ids',
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



    }
  };
})(jQuery);
