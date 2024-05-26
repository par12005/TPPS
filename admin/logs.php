<?php

/**
 * @file
 * Tripal Job Log.
 */

/**
 * Menu callback. Shows Tripal Job log page.
 *
 * Shows Tripal Job log file and updates it every 10 seconds.
 *
 * WARNING: This log files could be accessed by anonymous visitors:
 * https://tgwebdev.cam.uchc.edu/sites/default/files/tpps_job_logs/TGDR925_273188.txt
 *
 * If the administrator is looking at one specific TPPS submission, they are
 * provided with options to reject the submission and leave a reason for the
 * rejection, or to approve the submission and start loading the data into the
 * database. If the submission includes CartograPlant layers with environmental
 * parameters, the administrator will need to select the kind of parameter the
 * user has selected - an attr_id, or a cvterm. This will be important when the
 * submission is recording the environmental data of the plants.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 * @param string $job_log_file
 *   Name of the Tripal log file under /sites/default/files/tpps_job_logs/'
 *   folder without '.txt' suffix. E.g., 'TGDR925_273188', where
 *   'TGDR925' is a study accession and '273188' is Tripal Job Id.
 *   Log file: /sites/default/files/tpps_job_logs/TGDR925_273188.txt
 *
 * @return array
 *   The administrative panel logs form.
 */
function tpps_admin_panel_logs(array $form, array &$form_state, $job_log_file = NULL) {
  $form = $form ?? [];
  tpps_add_css_js('main', $form);

  $job_log_file_parts = explode('_', $job_log_file);
  $accession = $job_log_file_parts[0];
  $job_id = $job_log_file_parts[1];
  $job_log_file = $job_log_file . '.txt';

  // @todo Move JS to separate JS file and use drupal_add_js() instead.
  $markup = l(
      t('Return to TPPS Admin Panel - @accession', ['@accession' => $accession]),
      'tpps-admin-panel/' . $accession
    ) .'<br />'
    . "<a target='_blank' href='/admin/tripal/tripal_jobs/view/$job_id'>View Tripal Job ID: $job_id</a><br />"
    . "This page refreshes every 10 seconds.<br />"
    . "<iframe id='iframe_log' height='400px;' width='100%' src='/sites/default/files/tpps_job_logs/" . $job_log_file . "'></iframe>"
    . '<script type="text/javascript">'
    . "jQuery(document).ready(function() {"
    . "  setInterval(function() {"
    . "    var url='/sites/default/files/tpps_job_logs/$job_log_file';"
    . "    var nocache=Math.floor(Date.now() / 1000);"
    . "    jQuery('#iframe_log').attr('src', url + '?nocache=' + nocache);"
    . "  }, 10000);"
    . "  jQuery('#iframe_log').on('load', function() {"
    // . "    console.log('iframe reloaded'); console.log(jQuery('#iframe_log').height());"
    . "    jQuery('#iframe_log').contents().scrollTop(jQuery('#iframe_log').contents().height());"
    . "  });"
    . "});"
    . '</script>';

  $form['markup'] = [
    '#type' => 'markup',
    '#markup' => $markup,
  ];
  return $form;
}
