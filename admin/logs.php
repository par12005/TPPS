<?php
/**
 * Creates the administrative panel form.
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
 *
 * @return array
 *   The administrative panel logs form.
 */
function tpps_admin_panel_logs(array $form, array &$form_state, $job_log_file = NULL) {
  //   if (empty($accession)) {
  //     tpps_admin_panel_top($form);
  //   }
  //   else {
  //     tpps_manage_submission_form($form, $form_state, $accession);
  //   }

  $form = $form ?? [];
  tpps_add_css_js($form);

  $job_log_file_parts = explode('_', $job_log_file);
  $accession = $job_log_file_parts[0];
  $job_id = $job_log_file_parts[1];
  $job_log_file = $job_log_file . '.txt';

  $markup = "";
  $markup .= "<a href='/tpps-admin-panel/$accession'>Return to TPPS Admin Panel - $accession</a><br />";
  $markup .= "<a target='_blank' href='/admin/tripal/tripal_jobs/view/$job_id'>View Tripal Job ID: $job_id</a><br />";
  $markup .= "This page refreshes every 10 seconds.<br />";
  $markup .= "<iframe id='iframe_log' height='400px;' width='100%' src='/sites/default/files/tpps_job_logs/" . $job_log_file . "'></iframe>";
  $markup .= '<script type="text/javascript">';
  $markup .= "jQuery(document).ready(function() {";
  $markup .= "  setInterval(function() {";
  $markup .= "    var url='/sites/default/files/tpps_job_logs/$job_log_file';";
  $markup .= "    var nocache=Math.floor(Date.now() / 1000);";
  $markup .= "    jQuery('#iframe_log').attr('src', url + '?nocache=' + nocache);";
  $markup .= "  }, 10000);";
  $markup .= "  jQuery('#iframe_log').on('load', function() {";
  // $markup .= "    console.log('iframe reloaded'); console.log(jQuery('#iframe_log').height());";
  $markup .= "    jQuery('#iframe_log').contents().scrollTop(jQuery('#iframe_log').contents().height());";
  $markup .= "  });";
  $markup .= "});";
  $markup .= '</script>';

  $form['markup'] = [
    '#type' => 'markup',
    '#markup' => $markup,
  ];
  return $form;
}
