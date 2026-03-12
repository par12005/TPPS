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
      let doi_field_selector = settings.tpps.doi_lookup.field;
      $(doi_field_selector).blur(function() {
        let doi = $(this).val();
        $('#dumpContainer').html('').hide();
        // 1. Request publication data from PublicationDOI::TABLE.
        $.ajax({
          url: settings.tpps.doi_lookup.ajax_path,
          type: 'POST',
          contentType: 'application/json',
          dataType: 'json',
          data: JSON.stringify({"doi": doi}),
          success: function(response) {
console.log('Success:', response);
            if (response.publication_data.length !== 0) {
              Drupal.tpps.publicationDOI[doi] = response.publication_data;
              // Show at page.
              var jsonString = JSON.stringify(Drupal.tpps.publicationDOI[doi], null, 2);
              $('#dumpContainer').html(
                '<pre style="margin: 5px; color: white !important;">'
                + jsonString + '</pre>'
              ).show();


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
console.log(delay);
              // Build iframe for Google Scholar SERP.
              setTimeout(function() {
                settings.tpps.doi_lookup.last_request = $.now();
                // Build iframe.
console.log('build iframe.');
                let $iframe=$(settings.tpps.doi_lookup.iframe);
                let proxy_url = buildUrl(settings, doi);
console.log(proxy_url);
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

        // @TODO
        // 3. Build new url and create iframe.
        // 4. Get iframe content and send to backend.
        //    - store into 'tpps_google_scholar' table.
        //    - parse content
        //    - store to PublicationDIO::TABLE.
        //    - send back publication information.
        // 5. Show publication information at page.
        //


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

      // Get Google Scholar iframe content.
      $(settings.tpps.doi_lookup.iframe).on('load', function() {
        // The iframe and its contents have finished loading
        let iframeContent = $(this).contents().find('html').html();
        console.log(iframeContent);
// @TODO Send content to backend.
        //
        //$iframeContents.find("body").append("<p>Content added after load.</p>");
      });

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }
  }
})(jQuery, Drupal);
