/**
 * @file
 *
 * TPPS DOI Lookup.
 */
(function ($) {
  Drupal.behaviors.doi_lookup = {
    attach: function (context, settings) {
      // TPPS DOI Lookup.
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      let doi_field_selector = settings.tpps.doi_lookup.field;
      $(doi_field_selector).blur(function() {
        let doi = $(this).val();

        // @TODO
        // 1. Request publication data from PublicationDOI::TABLE.
        // 2. If there is no data - check delay.
        let delay = settings.tpps.doi_lookup.delay;



        // 3. Build new url and create iframe.
        // 4. Get iframe content and send to backend.
        //    - store into 'tpps_google_scholar' table.
        //    - parse content
        //    - store to PublicationDIO::TABLE.
        //    - send back publication information.
        // 5. Show publication information at page.
        //


        let $iframe=$('#google-scholar');
        let proxy_url = buildUrl (settings, doi);
        $iframe.attr('src', proxy_url).innerHTML(proxy_url);
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
        let query = settings.tpps.doi_lookup.query;
        query["q"] = doi;
        return settings.tpps.doi_lookup.proxy
          + settings.tpps.doi_lookup.endpoint + '?' + $.param(query);
      }

      // Get Google Scholar iframe content.
      $('iframe#google-scholar').on('load', function() {
        // The iframe and its contents have finished loading
        let $iframeContents = $(this).contents();




// @TODO Send content to backend.
        //
        //$iframeContents.find("body").append("<p>Content added after load.</p>");
      });

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    }
  }
})(jQuery, Drupal);
