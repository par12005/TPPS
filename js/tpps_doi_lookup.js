/**
 * @file
 *
 * TPPS DOI Lookup.
 */
(function ($) {
  Drupal.tpps = Drupal.tpps || {};
  Drupal.tpps.publicationDOI = Drupal.tpps.publicationDOI || {};

  Drupal.behaviors.doi_lookup = {
    attach: function (context, settings) {
      // TPPS DOI Lookup.
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $(settings.tpps.doi_lookup.field).blur(function() {
        let doi = $(this).val();
        // Check if doi was really changed.
        if (Drupal.tpps.publicationDOI[doi]) {
          //console.log('Re-used already received data.');
          return;
        }
        $('#dumpContainer').html('').hide();
        // Request publication data from PublicationDOI::TABLE.
        $.ajax({
          url: settings.tpps.doi_lookup.ajax_get_path,
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
              let last_request = settings.tpps.doi_lookup.last_request;
              if (last_request) {
                delay = (last_request + (settings.tpps.doi_lookup.delay * 1000)) - $.now();
                if (delay < 0) {
                  delay = 0;
                }
              }
              // Build iframe for Google Scholar SERP.
              setTimeout(function() {
                settings.tpps.doi_lookup.last_request = $.now();
                // Build iframe.
                let $iframe=$(settings.tpps.doi_lookup.iframe);
                let proxy_url = buildUrl(settings, doi);
                if (proxy_url) {
                  $iframe.attr('src', proxy_url);
                  // .innerHTML(proxy_url);
                }
              }, delay);
            }
          },
          error: function(xhr, status, error) {
            console.error('Error:', error);
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
        let query = settings.tpps.doi_lookup.query;
        query["q"] = doi;
        if (settings.tpps.doi_lookup.proxy != '') {
          url = settings.tpps.doi_lookup.proxy
            + settings.tpps.doi_lookup.debug
            + settings.tpps.doi_lookup.endpoint
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
        Drupal.tpps.publicationDOI[data.doi] = data;
        // Show at page.
        var jsonString = JSON.stringify(data, null, 2);
        $('#dumpContainer').html(
          '<pre style="margin: 5px; color: white !important;">'
          + jsonString + '</pre>'
        ).show();
      }

      // Get Google Scholar iframe content.
      $(settings.tpps.doi_lookup.iframe).on('load', function() {
        let doi = $(settings.tpps.doi_lookup.field).val();
        // The iframe and its contents have finished loading
        let iframeContent = $(this).contents().find('html').html();
        // Send content to backend.
        $.ajax({
          url: settings.tpps.doi_lookup.ajax_save_path,
          type: 'POST',
          contentType: 'application/json',
          dataType: 'json',
          data: JSON.stringify({
            "doi": doi,
            "serp": btoa(unescape(encodeURIComponent(iframeContent)))
          }),
          success: function(response) {
            if (response && response.publication_data && response.publication_data.length !== 0) {
              showPublicationData(response.publication_data);
            }
            else {
              console.log('Got empty data');
            }
          },
          error: function(xhr, status, error) {
            console.error('Error:', error);
          }
        });
      });
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }
  }
})(jQuery, Drupal);
