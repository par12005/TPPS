(function ($) {
  Drupal.behaviors.tppsMain = {
    attach:function (context) {

      var vcfPreValidateButton = '.vcf-pre-validate-button';
      // Bootstrap tooltip functionality.
      $('[data-toggle="tooltip"]', context).tooltip();

      function Supplemental_Files() {
        var files_add = $('#edit-files-add');
        var files_remove = $('#edit-files-remove');
        var number_object = $('#edit-files div input:hidden');
        var files_number = number_object[0].value;
        var files = $('#edit-files div div.form-type-managed-file');

        files.hide();

        if (files_number > 0){
          for (var i = 0; i < files_number; i++){
            $(files[i]).show();
          }

          for (var i = files_number; i < 10; i++){
            $(files[i]).hide();
          }
        }

        files_add.attr('type', 'button');
        files_remove.attr('type', 'button');

        files_add.on('click', function(){
          if (files_number < 10){
            files_number++;
            number_object[0].value = files_number;

            for (var i = 0; i < files_number; i++){
              $(files[i]).show();
            }

            for (var i = files_number; i < 10; i++){
              $(files[i]).hide();
            }
          }
        });

        files_remove.on('click', function(){
          if (files_number > 0){
            files_number--;
            number_object[0].value = files_number;

            for (var i = 0; i < files_number; i++){
              $(files[i]).show();
            }

            for (var i = files_number; i < 10; i++){
              $(files[i]).hide();
            }
          }
        });

      }

      var stage;
      if (
        typeof Drupal.settings.tpps !== 'undefined'
        && typeof Drupal.settings.tpps.stage !== 'undefined'
      ) {
        stage = Drupal.settings.tpps.stage;

        var status_block = $(".tpps-status-block");
        $(".region-sidebar-second").empty();
        status_block.prependTo(".region-sidebar-second");

        $("#progress").css('font-size', '1.5rem');
        $("#progress").css('margin-bottom', '30px');

        if (stage === 'summarypage'){
          Supplemental_Files();
          $("#tpps-status").insertAfter(".tgdr_form_status");
          $('.next-button').on('click', function(){

            $("#tpps-status")
              .html('<label>' + Drupal.t('Loading...') + '</label><br>'
                + Drupal.t('This step may take several minutes.')
              );
          });
        }
      }

      if (typeof stage !== 'undefined' && stage == 4) {
        var layer_search_buttons = $('input').filter(function() {
          return this.id.match(/edit-organism-[0-9]+-environment-layer-search/);
        });
        layer_search_buttons.keyup(function() {
          var button_value = new RegExp($(this).val(), 'i');
          var org_num = this.id.match(/edit-organism-([0-9]+)-environment-layer-search/)[1];
          var pattern = new RegExp('edit-organism-' + org_num + '-environment-env-layers-');
          var group_pattern = new RegExp('edit-organism-' + org_num + '-environment-env-layers-groups-');
          var layers = $('input').filter(function() {
            return this.id.match(pattern) && !this.id.match(group_pattern);
          });
          $.each(layers, function() {
            var label = $(this).parent().children('label')[0].innerText;
            if (label.match(button_value)) {
              $(this).parent().show();
            }
            else {
              $(this).parent().hide();
            }
          });
        });
      }

      var preview_record_buttons = $('input').filter(
        function() { return this.id.match(/-tripal-eutils-callback/); }
      );
      $.each(preview_record_buttons, function() {
        $(this).attr('type', 'button');
      });

      var preview_buttons = $('input.preview_button');
      $.each(preview_buttons, function() {
        $(this).click(function(e) {
          previewFile(this, 3);
          e.preventDefault();
        });
      });

      var preview_full_buttons = $('input.preview_full_button');
      $.each(preview_full_buttons, function() {
        $(this).click(function(e) {
          e.preventDefault();
          previewFile(this, 0);
        });
      });

      var pending_jobs = {};

      /**
       * Checks status of the Tripal Job 'VCF File Pre-Validation'.
       *
       * @TODO Do not allow re-validation of the same file. Store status of
       * the check in browser.
       */
      function checkVCFJob(jid) {
        // @TODO Use Drupal.settings.tpps.accession.
        var accession = $(':input[name="accession"]')[0].value;
        $.ajax({
          url: '/tpps/' + accession + '/pre-validate/' + jid + '/status',
          type: 'GET',
        })
        .done(function(job){
          if (
            job === null
            || typeof job !== 'object'
            || !job.hasOwnProperty('status')
          ) {
            $('.pre-validate-message')
              .html('<div class="alert alert-block alert-danger messages error">'
                + Drupal.t('There was a problem checking the status of job with '
                + 'id @job_id', {'@job_id': jid})+ '</div>'
              ).show();
          }
          // If this job is completed, check all the other jobs.
          else if (job.status === "Completed") {
            // This job is no longer pending.
            pending_jobs[jid] = false;
            var jobs_complete = true;
            // Iterate through pending jobs.
            for (jid in pending_jobs) {
              if (pending_jobs[jid]) {
                jobs_complete = false;
              }
            }
            // If no jobs are pending, enable the submit button.
            if (jobs_complete) {
              $(vcfPreValidateButton).prop('disabled', false);
              console.log(Drupal.t('All the jobs were completed.'));
              var message = '<div class="alert alert-block alert-success messages '
                + 'status">' + Drupal.t('VCF File pre-validation completed!')
                + '</div>';
              var errors = job.val_errors;
              if (errors.length > 0) {
                for (error of errors) {
                  message = message + '<div class="alert alert-block alert-danger '
                    + 'messages error">' + error + '</div>';
                }
              }
              else {
                $('.review-and-submit-button').prop('disabled', false);
              }
              $('.pre-validate-message').html(message).show();
            }
            // Otherwise, log the pending jobs.
            else {
              console.log(pending_jobs);
            }
          }
          // Otherwise, wait 5 seconds and check again.
          else {
            console.log(Drupal.t('job @job_id status: @status',
              {'@job_id': jid, '@status': job.status}
            ));
            setTimeout(() => {
              checkVCFJob(job.job_id);
            }, 5000);
          }
        })
        .fail(function(job) {
          $('.review-and-submit-button').prop('disabled', false);
          $(vcfPreValidateButton).prop('disabled', false);
          console.log(Drupal.t('Failed.'));
          console.log(job);
        });
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Click on 'Pre-validate my VCF Files' button.
      $(vcfPreValidateButton).attr('type', 'button');
      $(vcfPreValidateButton).click(function() {
        var accession = $(':input[name="accession"]')[0].value;
        console.log(Drupal.t('Initializing jobs...'));

        // Get real number of organisms.
        let organismNumber = Drupal.settings.tpps.organismNumber || 1;
        let vcfs = {};
        let missing_vcf = false;
        // Loop each organism to get VCF fid or path (local/remote).
        for (let organismId = 1; organismId <= organismNumber; organismId++) {
          let organismName = 'organism-' + organismId;
          let vcfFileLocation = $('select[name="' + organismName
            + '[genotype][SNPs][vcf_file-location]'
            + '"]').val();
          // Note: 'remote' means already stored on remote server (path) and
          // 'local' is stored on user's PC and must be uploaded via form.
          if (vcfFileLocation == 'local') {
            // Process File Id. Value 'local'.
            let vcfFid = $('input[name="' + organismName
              + '[genotype][SNPs][vcf][fid]'
              + '"]').val();
            // Validate.
            if (vcfFid === '0') {
              missing_vcf = true;
            }
            else {
              vcfs[organismId] = vcfFid;
            }
          }
          else {
            // Process path. Value 'remote'.
            let vcfPath = $('input[name="' + organismName
              + '[genotype][SNPs][local_vcf]' + '"]').val();
            // Validate.
            if (vcfPath.length == 0) {
              missing_vcf = true;
            }
            else {
              vcfs[organismId] = vcfPath;
            }
          }
        }

        if (missing_vcf) {
          $('.pre-validate-message')
            .html('<div class="alert alert-block alert-danger messages error">'
              + Drupal.t('Please upload your VCF file before clicking the '
              + 'pre-validate button') + '</div>'
            ).show();
        }
        else {
          $('.review-and-submit-button').prop('disabled', true);
          $(vcfPreValidateButton).prop('disabled', true);
          $.makeArray(vcfs).map((element) => { return element.value; });

          // Initialize vcf jobs and begin checkVCFJob routines.
          var request = $.ajax({
            url: '/tpps/' + accession + '/pre-validate',
            // Note: vcfs is an array.
            data: {'vcfs': vcfs},
            type: 'GET',
          })
          .done(function(jobs) {
            if (typeof jobs === 'string') {
              $('.pre-validate-message')
                .html('<div class="alert alert-block alert-danger messages error">'
                  + jobs + '</div>').show();
              $('.review-and-submit-button').prop('disabled', false);
              $(vcfPreValidateButton).prop('disabled', false);
            }
            else if (!Array.isArray(jobs) || jobs.length == 0) {
              $('.pre-validate-message')
                .html('<div class="alert alert-block alert-danger messages error">'
                  + Drupal.t('There was a problem with pre-validating your VCF '
                  + 'files. Please reload the page and try again')
                  + '</div>').show();
              $('.review-and-submit-button').prop('disabled', false);
              $(vcfPreValidateButton).prop('disabled', false);
            }
            else {
              console.log(Drupal.t('Jobs initialized!'));
              console.log(Drupal.t('List of jobs: \n!jobs',
                {'!jobs': JSON.stringify(jobs, null, 2)}
              ));
              for (job of jobs) {
                checkVCFJob(job);
                pending_jobs[job] = true;
              }
              $('.pre-validate-message')
                .html('<img src="/misc/throbber-active.gif"> '
                  + Drupal.t('Pre-validating VCF files. This may take some time...')
                ).show();
            }
          })
          .fail(function(jobs) {
            $('.review-and-submit-button').prop('disabled', false);
            $(vcfPreValidateButton).prop('disabled', false);
            console.log('Check of the status of VCF-file pre-validation failed.');
            console.log(jobs);
          });
        }
      });

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      $('#edit-save-comments').attr('type', 'button');

      var details_tabs = $('.nav-tabs > .nav-item > .nav-link');
      $.each(details_tabs, function() {
        $(this).click(detailsTab);
      });
      $('[href="#species"]').trigger('click');
      $('a:contains("Return to TPPS List")').hide();
      $('#edit-details-search').attr('type', 'button');
      $('#edit-details-search').click(detailSearch);
      $('#tpps-details-search').submit(function() {
        detailSearch();
        return false;
      });

      var file_options = $('div').filter(function() {
        return this.id.match(/^file_.*_options$/);
      });
      $.each(file_options, function() {
        if ($('#' + this.id + '_dest').length > 0) {
          $(this).prependTo('#' + this.id + '_dest');
        }
        else {
          $(this).hide();
        }
      });

      initDetailPages();

      var tags = $('#tpps-tags-filter').children('.tag');
      $.each(tags, function() {
        $(this).click(detailTagSearch);
      });

      var admin_panel_regex = /tpps-admin-panel\/TGDR.*/g;
      if (window.location.pathname.match(admin_panel_regex)) {
        var tag_buttons = $('span').filter(function() {
          return (this.id.match(/TGDR[0-9]+-tag-[0-9]*-add|remove/) && !this.disabled);
        });
        $.each(tag_buttons, function() {
          $(this).click(function() {
            var tag_button = this;
            var info = this.id.match(/(TGDR[0-9]+)-tag-([0-9]*)-(add|remove)/);
            if (info.length == 4) {
              var request = $.get('/tpps-tag/' + info[3] + '/' + info[1] + '/' + info[2]);
              request.done(function (data) {
                if (info[3] == 'remove') {
                  $(tag_button).parent().hide();
                  var pattern = new RegExp('TGDR[0-9]+-tag-' + info[2] + '-add');
                  $('span').filter(function() { return this.id.match(pattern); }).show();
                }
                else {
                  $(tag_button).hide();
                  var pattern = new RegExp('TGDR[0-9]+-tag-' + info[2] + '-remove');
                  $('span.tag-close').filter(function() {
                    return this.id.match(pattern);
                  }).parent().show();
                }
              });
            }
            else {
              console.log('Could not parse accession/tag id/edit type from button id');
              console.log(info);
            }
          });
        });

        var add_tags = $('span').filter(function() {
          return this.id.match(/TGDR[0-9]+-add-tags/);
        });
        $.each(add_tags, function() {
          $(this).click(function() {
            console.log(this);
          })
        })
      }
    }
  };

}(jQuery));


function previewFile(element, num_rows = 3) {
  var fid;
  if (element.id.match(/fid_(.*)/) === null) {
    return;
  }
  fid = element.id.match(/fid_(.*)/)[1];
  let $previewElement = jQuery('.preview_' + fid);
  if ($previewElement.length !== 0) {
    //$previewElement.remove();
    $previewElement.fadeOut(500, function() {
      $previewElement.remove();
    });
  }
  var request = jQuery.post('/tpps-preview-file', {
    fid: fid,
    rows: num_rows
  });
  request.done(function (data) {
    jQuery('#fid_' + fid).before(data);
  });
}

var detail_pages = {
  "trees": 0,
  "phenotype": 0,
  "genotype": 0,
  "environment": 0,
  "submission": 0,
};

function detailsTab() {
  var clicked_tab = jQuery(this)[0];
  var path = clicked_tab.pathname;
  var detail_type = clicked_tab.hash.substr(1);
  var page = detail_pages[detail_type];

  if (clicked_tab.hash.match(/#(.*):(.*)/) !== null) {
    detail_type = clicked_tab.hash.match(/#(.*):(.*)/)[1];
    page = clicked_tab.hash.match(/#(.*):(.*)/)[2];
    detail_pages[detail_type] = page;
  }
  else {
    if (jQuery('#' + detail_type)[0].innerHTML !== "") {
      // If we aren't loading a new page and we already have data for this tab,
      // then we don't need to change any of the HTML.
      return;
    }
  }
  jQuery('#' + detail_type)[0].innerHTML = Drupal.t('Querying database for '
    + '@detail_type information...', {'@detail_type': detail_type})
    + '<div id="query_timer"></div>';

  // create a timer
  var query_timer_current = 0;
  var query_timer = setInterval(function() {
    query_timer_current = query_timer_current + 1;
    if(query_timer_current > 5) {
      jQuery('#query_timer')
        .html(Drupal.t('Querying time: @time seconds.',
          {'@time': query_timer_current})
          + '<br /><strong>' + Drupal.t('Thank you for your patience, '
          + 'first time pulls can take up to a minute to '
          + 'complete depending on the size of our dataset but gets faster '
          + 'after the first page load.') + '</strong>'
        );
    }
  }, 1000);

  // OLD version from Peter
  // var request = jQuery.post(path + '/' + detail_type, {
  //   page: page
  // });

  // New version from Rish with XHR status
  var request = jQuery.ajax({
      xhr: function() {
        var xhr = new window.XMLHttpRequest();

        // Upload progress
        xhr.upload.addEventListener("progress", function(evt){
            if (evt.lengthComputable) {
                var percentComplete = evt.loaded / evt.total;
                //Do something with upload progress
                console.log(percentComplete);
            }
      }, false);

      // Download progress
      xhr.addEventListener("progress", function(evt){
          try {
            clearInterval(query_timer);
          } catch (err) {}
          console.log(evt);
          jQuery('#' + detail_type)[0].innerHTML = "Loading "
            + detail_type + " information... " + Math.ceil(evt.loaded / 100) + ' KB';
          if (evt.lengthComputable) {
              var percentComplete = evt.loaded / evt.total;
              // Do something with download progress
              console.log(percentComplete);
              //jQuery('#' + detail_type)[0].innerHTML = "Loading " + detail_type + " information... " + Math.ceil(percentComplete * 100) + ' %';
          }
      }, false);

      return xhr;
    },
    type: 'POST',
    url: path + '/' + detail_type,
    data: {
      page: page
    }
  });

  request.done(function (data) {
    jQuery('#' + detail_type)[0].innerHTML = data;
    var details_pagers = jQuery('#' + detail_type + ' > div > ul');
    if (details_pagers.length > 0) {
      var pages = jQuery('#' + detail_type + ' > div > ul > li > a');
      jQuery.each(pages, function() {
        var page = 0;
        if (this.search.match(/\?page=(.*)/) !== null) {
          page = this.search.match(/\?page=(.*)/)[1];
        }
        if (detail_type != 'submission') {
          this.href = '#' + detail_type + ':' + page;
        }
        else {
          var path = window.location.pathname;
          this.href = this.href + '/' + path.substring(path.lastIndexOf('/') + 1);
        }
        jQuery(this).click(detailsTab);
      });
    }
  });
}

function initDetailPages() {
  var details_pages = jQuery('#tpps-details-table > div > ul > li > a');
  if (details_pages.length > 0) {
    jQuery.each(details_pages, function() {
      var page = 0;
        if (this.search.match(/\?page=(.*)/) !== null) {
          page = this.search.match(/\?page=(.*)/)[1];
        }
      this.href = '#top:' + page;
      jQuery(this).click(detailSearch);
    });
  }
}

function detailSearch() {
  var path = '/tpps/details/top';
  var page = 0;
  if (this.hash != null && this.hash.match(/#.*:(.*)/) != null) {
    page = this.hash.match(/#.*:(.*)/)[1];
  }

  jQuery('#tpps-details-table')[0].innerHTML = 'Loading...';
  var request = jQuery.post(path, {
    type: jQuery('select').filter(function() { return this.id.match(/edit-details-type/); })[0].value,
    value: jQuery('input').filter(function() { return this.id.match(/edit-details-value/); })[0].value,
    op: jQuery('select').filter(function() { return this.id.match(/edit-details-op/); })[0].value,
    page: page
  });

  request.done(function (data) {
    jQuery('#tpps-details-table')[0].innerHTML = data;
    initDetailPages();
  });
}

function detailTagSearch() {
  var path = '/tpps/details/top';
  var page = 0;
  if (this.hash != null && this.hash.match(/#.*:(.*)/) != null) {
    page = this.hash.match(/#.*:(.*)/)[1];
  }

  jQuery('#tpps-details-table')[0].innerHTML = 'Loading...';
  jQuery('select').filter(function() { return this.id.match(/edit-details-type/); })[0].value = 'tags';
  jQuery('input').filter(function() { return this.id.match(/edit-details-value/); })[0].value = jQuery(this).text();
  jQuery('select').filter(function() { return this.id.match(/edit-details-op/); })[0].value = '=';
  var request = jQuery.post(path, {
    type: 'tags',
    value: jQuery(this).text(),
    op: '=',
    page: page
  });

  request.done(function (data) {
    jQuery('#tpps-details-table')[0].innerHTML = data;
    initDetailPages();
  });
}

/* ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: */
/* [VS] */
(function ($, Drupal) {
  // Create namespaces.
  Drupal.tpps = Drupal.tpps || {};
  // @TODO Minor. Rename 'doi' to 'publication_doi'.
  Drupal.tpps.doi = Drupal.tpps.doi || {};

  /**
   * Strip HTML Tags from given string.
   *
   * See https://stackoverflow.com/a/822486/1041470
   */
  Drupal.tpps.stripHtml = function(html) {
     let tmp = document.createElement('DIV');
     tmp.innerHTML = html;
     return tmp.textContent || tmp.innerText || '';
  }

  Drupal.behaviors.tpps = {
    attach: function (context, settings) {
      // Attach event handlers only once.
      $('form[id^=tppsc-main]').once('tpps_page', function() {
        // Add code here. Will be attached once.
      });
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Allows to fill field using suggestions from description.
      // How to use:
      //   add 'tpps-suggestion' class to A tag in
      //   $form[$field_name]['#description'];
      // Example: <a href"#" class="tpps-suggestion">10.25338/B8864J</a>
      $('.tpps-suggestion').not('.tpps-suggestion-processed').on('click', function(e) {
        e.preventDefault();
        let selectedText= $(this).text();
        //console.log(selectedText);
        $(this)
          .parents('.form-item')
          .find('input.form-text')
          .val(selectedText)
          .blur();
        navigator.clipboard.writeText(selectedText);
      }).addClass('tpps-suggestion-processed');

      // Temporary block status bar links.
      $('.tgdr_form_status a').on('click', function(e) {
        e.preventDefault();
      });

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Hide VCF pre-validation status messages when VCF changed.
      if (
        typeof Drupal.settings.tpps != 'undefined'
        && typeof Drupal.settings.tpps.organismNumber != 'undefined'
        && (
          $('form.tpps-form').length
          || $('form.tppsc-form').length
        )
      ) {
        let organismNumber = Drupal.settings.tpps.organismNumber || 1;
        for (let organismId = 1; organismId <= organismNumber; organismId++) {
          let organismName = 'organism-' + organismId;
          let vcfFileLocationFieldName = 'select[name="' + organismName
            + '[genotype][SNPs][vcf_file-location]' + '"]';
          let vcfFileFieldName = 'input[name="' + organismName
            + '[genotype][SNPs][vcf][fid]' + '"]';
          let vcfPathFieldName = 'input[name="' + organismName
            + '[genotype][SNPs][local_vcf]' + '"]';

          $(vcfFileLocationFieldName).change(function() { $('.pre-validate-message').hide() });
          $(vcfFileFieldName).blur(function() { $('.pre-validate-message').hide() });
          $(vcfPathFieldName).blur(function() { $('.pre-validate-message').hide() });
        }
      }
    }
  };
})(jQuery, Drupal);
/* [/VS] */
