/**
 * @file
 *
 * TPPS DOI Lookup.
 */
(function ($) {
  Drupal.behaviors.doi_lookup = {
    attach: function (context, settings) {
      let doi_lookup = settings.tpps.doi_lookup;
      doi_lookup.serp_providers.forEach(function(provider_name) {

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Event: iframe got content.
        $(doi_lookup[provider_name].iframe).on('load', function() {
          // Save Google Scholar iframe content when it was loaded.
          $(doi_lookup.field)
            .addClass('tpps-throbber')
            .prop('disabled', true);
          saveIframeContent(provider_name);
          $(doi_lookup.field)
            .removeClass('tpps-throbber')
            .prop('disabled', false);
        });

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Event: DOI entered.
        $(doi_lookup.field).blur(function() {
          // Get publication data by DOI.
          let doi = $.trim($(this).val());
          if (!doi) {
            // Empty DOI. Nothing to do and it's not an error.
            return;
          }
          // Block DOI field while processing.
          $(this)
            .addClass('tpps-throbber')
            .prop('disabled', true);
          // Check if doi was really changed.
          if (
            doi_lookup[provider_name].publication_data != undefined
            && doi_lookup[provider_name].publication_data[doi] != undefined
          ) {
            showPublicationData(
              provider_name,
              doi_lookup[provider_name].publication_data[doi]
            );
            $(doi_lookup.iframe).hide();
            return;
          }
          $(doi_lookup[provider_name].dump_container).html('').hide();

          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // Request publication data from PublicationDOI::TABLE.
          $.ajax({
            url: doi_lookup.ajax_get_publication_data_path,
            type: 'POST',
            contentType: 'application/json',
            dataType: 'json',
            data: JSON.stringify({"doi": doi, "source": provider_name}),
            success: function(response) {
              if (response && response.publication_data && response.publication_data.length !== 0) {
                showPublicationData(
                  response.source,
                  response.publication_data
                );
              }
              else {
                // Calculate delay.
                let delay = 0;
                let last_request = doi_lookup[provider_name].last_request;
                if (last_request) {
                  delay = (last_request + (doi_lookup[provider_name].delay * 1000)) - $.now();
                  if (delay < 0) {
                    delay = 0;
                  }
                }
                // Build iframe for Google Scholar SERP.
                setTimeout(function() {
                  doi_lookup[provider_name].last_request = $.now();
                  // Build iframe.
                  let $iframe=$(doi_lookup[provider_name].iframe);
                  let proxy_url = buildUrl(provider_name, doi);
                  if (proxy_url) {
                    $iframe.attr('src', proxy_url).show();
                  }
                  $(doi_lookup.field)
                    .removeClass('tpps-throbber')
                    .prop('disabled', false);
                }, delay);
              }
            },
            error: function(xhr, status, error) {
              console.error('DOI Lookup Error:', error);
            }
          });


          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // Debug code to get new URL after redirect.
          if (0) {
            $.ajax({
              url: 'https://tgwebdev.cam.uchc.edu/corsanywhere/https://pubmed.ncbi.nlm.nih.gov/?term=10.1038/sdata.2015.6',
              type: 'GET',
              success: function(response) {
                console.log(response);
              },
              error: function(xhr, status, error) {
                console.log(status);
                console.log(xhr);
                console.error('DOI Lookup Error:', error);
              }
            });
          }



        });
      });

      /**
       * Build URL for Google Scholar iFrame.
       *
       * @param string provider_name
       *   Key of the SERP provider.
       * @param string $doi
       *   DOI
       *
       * @return string
       *   Retuns URL with proxy (CORS anywhere).
       */
      function buildUrl(provider_name, doi) {
        let url = '';
        let query = doi_lookup[provider_name].query;
        let query_param = doi_lookup[provider_name].query_param;
        query[query_param] = doi;
        if (doi_lookup.proxy != '') {
          url = doi_lookup.proxy
            + '/' + doi_lookup[provider_name].endpoint
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
      function showPublicationData(provider_name, data) {
        doi_lookup[provider_name].publication_data
          = doi_lookup[provider_name].publication_data ?? {};
        doi_lookup[provider_name].publication_data[data.doi] = data;
        // Show at page.
        var jsonString = JSON.stringify(data, null, 2);
        $(doi_lookup[provider_name].dump_container).html(
          '<pre style="margin: 5px; color: white !important; text-wrap:auto;">'
          + jsonString + '</pre>'
        ).show();
        //$(doi_lookup.iframe).hide();
        $(doi_lookup.field)
          .removeClass('tpps-throbber')
          .prop('disabled', false);
      }

      /**
       * Sends content of the iframe to backend to store in database.
       */
      function saveIframeContent(provider_name) {
        let doi = $.trim($(doi_lookup.field).val());
        if (!doi) {
          console.log("DOI Lookup: Empty DOI. Can't save iframe content");
          return;
        }

        // The iframe and its contents have finished loading
        let iframeContent = $(this).contents().find('html').html();

        // Check if CORS Anywhere Proxy failed.
        let error_message = 'Not found because of proxy error: Error: '
          + 'getaddrinfo ENOTFOUND scholar.google.com';
        if (iframeContent.includes(error_message)) {
          console.error('DOI Lookup. Proxy server is down.');
          return;
        }

        // Send content to backend.
        $.ajax({
          url: doi_lookup[provider_name].ajax_save_path,
          type: 'POST',
          contentType: 'application/json',
          dataType: 'json',
          data: JSON.stringify({
            "doi": doi,
            "source": provider_name,
            "serp": btoa(unescape(encodeURIComponent(iframeContent)))
          }),
          success: function(response) {
            if (
              response && response.publication_data
              && response.publication_data.length !== 0
            ) {
              showPublicationData(
                response.source,
                response.publication_data
              );
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
    }
  }
})(jQuery, Drupal);
