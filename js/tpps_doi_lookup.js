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
// @TODO Add full URL support beside short DOI value.
        let doi = $.trim($(field).val());
        if (!doi) {
          // Empty DOI. Nothing to do and it's not an error.
          return;
        }





        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Debug code to get new URL after redirect.
        if (0) {
          let pubmed_url = 'https://pubmed.ncbi.nlm.nih.gov/25202587/';
          //let pubmed_url = 'https://pubmed.ncbi.nlm.nih.gov/?term=10.1038/sdata.2015.6';
          console.log(pubmed_url);
          $.ajax({
            url: 'https://tgwebdev.cam.uchc.edu/corsanywhere/' + pubmed_url,
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



        // Block DOI field while processing.
        enableThrobber();
        // Get publication data by DOI.
        doi_lookup.serp_providers.forEach(function(provider_name) {
          // Format: doi_lookup['publication_data'][$provider_name][$doi]
          if (
            $(doi_lookup['publication_data'][provider_name]).length
            && $(doi_lookup['publication_data'][provider_name][doi]).length
          ) {
            console.log(provider_name + ': show already loaded data.');
            showPublicationData(
              provider_name,
              doi_lookup['publication_data'][provider_name][doi]
            );
          }
          else {
            console.log(provider_name + ': data not loadeded yet.');
            // No pre-loaded publication data found.
            // Let's get SERP.
            $(doi_lookup[provider_name].dump_container).html('').hide();
            // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
            // Request publication data from PublicationDOI::TABLE.
            console.log(provider_name + ': request data from backend.');
            $.ajax({
              url: doi_lookup.ajax_get_publication_data_path,
              type: 'POST',
              contentType: 'application/json',
              dataType: 'json',
              data: JSON.stringify({"doi": doi, "source": provider_name}),
              success: function(response) {
                if (response && response.publication_data && $(response.publication_data).length) {
                  console.log(provider_name + ': show received data.');
                  showPublicationData(response.source, response.publication_data);
                }
                else {
                  console.log(provider_name + ': received no data.');
                  // No saved publication data.
                  // Let's get SERP if scraping is allowed.
                  if (
                    $(doi_lookup[provider_name]).length
                     && doi_lookup[provider_name].scrape_allowed
                  ) {
                    // Calculate delay.
                    let delay = 0;
                    let last_request = doi_lookup[provider_name].last_request;
                    if (last_request) {
                      delay = (last_request + (doi_lookup[provider_name].delay * 1000)) - $.now();
                      if (delay < 0) {
                        delay = 0;
                      }
                    }
                    console.log(provider_name + ': delay.');
                    setTimeout(function() {
                      doi_lookup[provider_name].last_request = $.now();
                      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::
                      // GET-request via CORS-proxy to get SERP.
                      let proxy_url = buildUrl(provider_name, doi);
                      if (proxy_url) {
                        // URL for debug:
                        // proxy_url = 'https://cors-anywhere-ldq5.onrender.com/'
                        // + 'https://scholar.google.com/scholar?hl=en'
                        // + '&as_sdt=0%2C5&q=10.5061%2Fdryad.6s82f20&btnG=';
                        $.ajax({
                          method: 'GET',
                          url: proxy_url,
                          headers: {
                            //'x-requested-with': 'https://tgwebdev.cam.uchc.edu'
                            'X-Requested-With': 'XMLHttpRequest'
                          }
                        })
                        .done(function(data) {
                          //console.log('Success:', data);
                          // @TODO Check data.length.
                          // Send SERP to backend.
                          saveSERP(provider_name, data);
                          console.log(provider_name + ': SERP sent to backend.');
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
                    }, delay);
                  }
                }
              },
              error: function(xhr, status, error) {
                console.error('DOI Lookup Error:', error);
              }
            });
          }
        });
      }

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
        $(doi_lookup.field).addClass('tpps-throbber').prop('disabled', true);
      }

      /**
       * Disables throbber icon and enables DOI field.
       */
      function disableThrobber() {
        $(doi_lookup.field).removeClass('tpps-throbber').prop('disabled', false);
      }

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
        doi_lookup['publication_data'][provider_name]
          = doi_lookup['publication_data'][provider_name] ?? {};
        doi_lookup['publication_data'][provider_name][data.doi] = data;
        // Show at page.
        var jsonString = JSON.stringify(data, null, 2);
        $(doi_lookup[provider_name].dump_container).html(
          '<pre style="margin: 5px; color: white !important; text-wrap:auto;">'
          + jsonString + '</pre>'
        ).show();
        disableThrobber();
        $(doi_lookup.field).removeClass('get-publication-data-processed');
      }

      /**
       * Sends SERP Content to backend to store in database.
       */
      function saveSERP(provider_name, serpContent) {
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
        let not_found_message = '- did not match any articles.';
        if (serpContent.includes(not_found_message)) {
          console.error('DOI Lookup. No publication data found.');
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
