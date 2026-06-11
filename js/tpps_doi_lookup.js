/**
 * @file
 *
 * TPPS DOI Lookup.
 */

/* global jQuery:readonly, Drupal:writable */
(function ($, Drupal) {
  Drupal.behaviors.doi_lookup = {
    attach: function (context, settings) {
      let doi_lookup = settings.tpps.doi_lookup;

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Event: DOI entered.
      function getPublicationData(field) {
        $(field).addClass('get-publication-data-processed');

        // Note: there is no need to check if it's new DOI (or was it really
        // changed) because we store server's responses and re-use them and
        // DOI field is blocked during AJAX-requests to prevent changes and
        // spam-requests to the backend server.

        // Get value of the DOI field.
        let doi = $.trim($(field).val());
        // Convert full DOI format into short.
        doi = doi.replace("https://doi.org/", "");
        if (!doi) {
          // Empty DOI. Nothing to do and it's not an error.
          return;
        }
        // Block DOI field while processing.
        enableThrobber();

        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Format: doi_lookup['publication_data'][$provider_class_name][$doi]
        if ($(doi_lookup['publication_data'][doi]).length) {
          console.log('Show already loaded data.');
          showPublicationData(doi_lookup['publication_data'][doi]);
        }
        else {
          console.log('Data not loadeded yet.');
          // No publication data found.
          $(doi_lookup.dump_container).html('').hide();

          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // Request publication data from PublicationDOI::TABLE.
          console.log('Request data from backend.');
          $.ajax({
            url: doi_lookup.ajax_get_publication_data_path,
            type: 'POST',
            contentType: 'application/json',
            dataType: 'json',
            data: JSON.stringify({
              "doi": doi,
              "callback": doi_lookup.ajax_callback,
            }),
            success: function(response) {
              if (
                response
                && response.publication_data
                && response?.is_complete === true
              ) {
                console.log('Show received data.');
                showPublicationData(response.publication_data);
              }
              else {
                console.log("Received no data or it's incomplete.");
                // This is a flag which used to remove throbber when all SERP
                // providers are disabled at settings page and we shouldn't
                // wait for the response to AJAX requests.
                let wait_for_serp_provider = false;

                doi_lookup.serp_providers.forEach(function(provider_class_name) {
                  // Let's get SERP if scraping is allowed.
                  if (
                    $(doi_lookup[provider_class_name]).length
                    && doi_lookup[provider_class_name].scrape_allowed
                  ) {
                    wait_for_serp_provider = true;

                    // Calculate delay.
                    let delay = 0;
                    let last_request = doi_lookup[provider_class_name].last_request;
                    if (last_request) {
                      delay = (last_request + (doi_lookup[provider_class_name].delay * 1000)) - $.now();
                      if (delay < 0) {
                        delay = 0;
                      }
                    }
                    console.log(provider_class_name + ': delay.');
                    setTimeout(function() {
                      doi_lookup[provider_class_name].last_request = $.now();

                      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::
                      // GET-request via CORS-proxy to get SERP.
                      let proxy_url = buildUrl(provider_class_name, doi);
                      if (proxy_url) {
                        // URL for debug:
                        // proxy_url = 'https://cors-anywhere-ldq5.onrender.com/'
                        // + 'https://scholar.google.com/scholar?hl=en'
                        // + '&as_sdt=0%2C5&q=10.5061%2Fdryad.6s82f20&btnG=';

                        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::
                        // Get SERP.
                        $.ajax({
                          method: 'GET',
                          url: proxy_url,
                          headers: {'X-Requested-With': 'XMLHttpRequest'}
                        })
                        .done(function(data) {
                          //console.log('Success:', data);
                          // @TODO Check data.length.
                          // Send SERP to backend.
                          saveSERP(provider_class_name, data);
                          console.log(provider_class_name + ': SERP sent to backend.');
                        })
                        .fail(function(error) {
                          console.error('Error: ', error);
                        })
                        .always(function() {
                          disableThrobber();
                          $(field).removeClass('get-publication-data-processed');
                          console.log('Request via CORS Proxy completed.');
                        });
                      }
                      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::

                    }, delay);
                  }
                });

                // No AJAX-requests was started so nosense to wait for response.
                if (! wait_for_serp_provider) {
                  disableThrobber();
                  $(field).removeClass('get-publication-data-processed');
                  console.log('All SERP providers are disabled.');

                  console.log('Show incomplete publication data from APIs.');
                  showPublicationData(response.publication_data);
                }
              }
            },
            error: function(xhr, status, error) {
              console.error('DOI Lookup Error:', error);
              console.error('Status:', xhr.status);
              console.error('Response:', xhr.responseText);
              console.error('ReadyState:', xhr.readyState);
            }
          });
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      }



      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Change-event for select-or-other field.
      $(doi_lookup.field).change(function() {
        if (
          !($(this).hasClass('get-publication-data-processed'))
          && $.trim($(this).val())
        ) {
          getPublicationData(this);
        }
      });
      // Blur-event for textfield.
      $(doi_lookup.field).blur(function() {
        if (
          !($(this).hasClass('get-publication-data-processed'))
          && $.trim($(this).val())
        ) {
          getPublicationData(this);
        }
      });

      /**
       * Enables throbber icon and disables DOI field.
       */
      function enableThrobber() {
        if ($(doi_lookup.field).hasClass('form-select')) {
          // Select box.
          $(doi_lookup.field).prop('disabled', true)
            .parent('.form-type-select').addClass('tpps-select-throbber');
        }
        else {
          // Textfield.
          $(doi_lookup.field).addClass('tpps-throbber').prop('disabled', true);
        }
      }

      /**
       * Disables throbber icon and enables DOI field.
       */
      function disableThrobber() {
        if ($(doi_lookup.field).hasClass('form-select')) {
          // Select box.
          $(doi_lookup.field).prop('disabled', false)
            .parent('.form-type-select').removeClass('tpps-select-throbber');
        }
        else {
          $(doi_lookup.field).removeClass('tpps-throbber').prop('disabled', false);
        }
      }

      /**
       * Build URL for Google Scholar iFrame.
       *
       * @param string provider_class_name
       *   Key of the SERP provider.
       * @param string $doi
       *   DOI
       *
       * @return string
       *   Retuns URL with proxy (CORS anywhere).
       */
      function buildUrl(provider_class_name, doi) {
        let url = '';
        let query = doi_lookup[provider_class_name].query;
        let query_param = doi_lookup[provider_class_name].query_param;
        query[query_param] = doi;
        if (doi_lookup.proxy != '') {
          url = doi_lookup.proxy
            + '/' + doi_lookup[provider_class_name].endpoint
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


// @TODO Check if is_complete.

        doi_lookup['publication_data'][data.doi] = data;
        // Show at page.
        var jsonString = JSON.stringify(data, null, 2);
        $(doi_lookup.dump_container).html(
          '<pre style="margin: 5px; color: white !important; text-wrap:auto;">'
          + jsonString + '</pre>'
        ).show();
        disableThrobber();
        $(doi_lookup.field).removeClass('get-publication-data-processed');
      }

      /**
       * Sends SERP Content to backend to store in database.
       */
      function saveSERP(provider_class_name, serpContent) {
        let doi = $.trim($(doi_lookup.field).val());
        if (!doi) {
          console.log("DOI Lookup: Empty DOI. Can't send SERP to backend.");
          return;
        }

        // Check if CORS Anywhere Proxy failed.
        let error_message = 'Not found because of proxy error: Error: '
          + 'getaddrinfo ENOTFOUND scholar.google.com';
        if (serpContent.includes(error_message)) {
          console.error('DOI Lookup. Proxy server is down.');
          return;
        }

        // Check if there is any publication data for this DOI present at SERP.
        if (
          doi_lookup[provider_class_name].not_found_token != ''
          && serpContent.includes(doi_lookup[provider_class_name].not_found_token)
        ) {
          console.error('DOI Lookup. No publication data found.');
          return;
        }

        // Send content to backend.
        $.ajax({
          url: doi_lookup[provider_class_name].ajax_save_path,
          type: 'POST',
          contentType: 'application/json',
          dataType: 'json',
          data: JSON.stringify({
            "doi": doi,
            "callback": doi_lookup[provider_class_name].ajax_callback,
            "provider": provider_class_name,
            "serp": btoa(unescape(encodeURIComponent(serpContent)))
            // Modern fix:
            //"serp": btoa(serpContent)
          }),
          success: function(response) {
            if (
              response && response.publication_data
              && $(response.publication_data).length
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
