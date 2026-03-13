/**
 * @file
 *
 * TPPS DOI Lookup.
 */
(function ($) {
  Drupal.behaviors.doi_lookup = {
    attach: function (context, settings) {

      let doi_lookup = settings.tpps.doi_lookup;

      // TPPS DOI Lookup.
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $(doi_lookup.field).blur(function() {
        let doi = $.trim($(this).val());
        if (doi == '') {
          console.log('DOI Lookup: Empty value of the DOI.');
          return;
        }

        // Check if doi was really changed.
        if (
          doi_lookup?.publication_data != undefined
          && doi_lookup.publication_data[doi] != undefined
        ) {
          showPublicationData(doi_lookup.publication_data[doi]);
          //console.log('Re-used already received data.');
          return;
        }
        $('#dumpContainer').html('').hide();

        // Request publication data from PublicationDOI::TABLE.
        $.ajax({
          url: doi_lookup.ajax_get_path,
          type: 'POST',
          contentType: 'application/json',
          dataType: 'json',
          data: JSON.stringify({"doi": doi}),
          success: function(response) {
            //console.log('Success:', response);
            if (response && response.publication_data && response.publication_data.length !== 0) {
              showPublicationData(response.publication_data);
            }
            else {
              // Calculate delay.
              let delay = 0;
              let last_request = doi_lookup.last_request;
              if (last_request) {
                delay = (last_request + (doi_lookup.delay * 1000)) - $.now();
                if (delay < 0) {
                  delay = 0;
                }
              }
              // Build iframe for Google Scholar SERP.
              setTimeout(function() {
                doi_lookup.last_request = $.now();
                // Build iframe.
                let $iframe=$(doi_lookup.iframe);
                let proxy_url = buildUrl(settings, doi);
                if (proxy_url) {
                  $iframe.attr('src', proxy_url).show();
                }
              }, delay);
            }
          },
          error: function(xhr, status, error) {
            console.error('DOI Lookup Error:', error);
          }
        });
      });

      /**
       * Build URL for Google Scholar iFrame.
       *
       * @param object $settings
       *   Drupal.settings.
       * @param string $doi
       *   DOI
       *
       * @return string
       *   Retuns URL with proxy (CORS anywhere).
       */
      function buildUrl (settings, doi) {
        let url = '';
        let query = doi_lookup.query;
        query["q"] = doi;
        if (doi_lookup.proxy != '') {
          url = doi_lookup.proxy
            + (doi_lookup.debug ?? '')
            + '/' + doi_lookup.endpoint
            + '?' + $.param(query);
        }
        return url;
      }

      /**
       * Shows Publication Data.
       *
       * @param object data
       *   DOI Publication data
       */
      function showPublicationData(data) {
        doi_lookup.publication_data = doi_lookup.publication_data ?? {};
        doi_lookup.publication_data[data.doi] = data;
        // Show at page.
        var jsonString = JSON.stringify(data, null, 2);
        $('#dumpContainer').html(
          '<pre style="margin: 5px; color: white !important;">'
          + jsonString + '</pre>'
        ).show();
        $(doi_lookup.iframe).hide();
      }

      /**
       * Sends content of the iframe to backend to store in database.
       */
      function saveIframeContent() {
        let doi = $(doi_lookup?.field).val();
        if (doi == '') {
          console.log("DOI Lookup: Empty DOI. Can't save iframe content");
          return;
        }

        // The iframe and its contents have finished loading
        let iframeContent = $(this).contents().find('html').html();
        // Send content to backend.
        $.ajax({
          url: doi_lookup.ajax_save_path,
          type: 'POST',
          contentType: 'application/json',
          dataType: 'json',
          data: JSON.stringify({
            "doi": doi,
            "serp": btoa(unescape(encodeURIComponent(iframeContent)))
          }),
          success: function(response) {
            if (
              response && response.publication_data
              && response.publication_data.length !== 0
            ) {
              showPublicationData(response.publication_data);
            }
            else {
              console.log('DOI Lookup: Got empty data');
            }
          },
          error: function(xhr, status, error) {
            console.error('DOI Lookup Error:', error);
          }
        });
      };

      // Save Google Scholar iframe content when it was loaded.
      $(doi_lookup.iframe).on('load', saveIframeContent);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }
  }
})(jQuery, Drupal);
