<?php

/**
 * @file
 * Defines function to send notification emails upon TPPS Submission completion.
 */

/**
 * This function sends notification emails upon TPPS Submission completion.
 *
 * Emails are sent to the user completing the form as well as the administrator
 * email specified in the TPPS configuration form.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 *
 * @global stdClass $user
 *   The user submitting the form.
 * @global string $base_url
 *   The base URL of the site.
 */
function tpps_submit_email(array &$form_state) {

  global $user;
  global $base_url;

  // @TODO Minor. Combine 2 versions to remove duplicated code.

  if (($form_state['tpps_type'] ?? NULL) == 'tpps') {

    $admin = variable_get('tpps_admin_email', '');

    if ($admin == '') {
      return FALSE;
    }
    $params = array();

    $to = $user->mail;
    $from = variable_get('site_mail', '');
    $params['subject'] = "TPPS Submission Received: {$form_state['saved_values'][TPPS_PAGE_1]['publication']['title']}";

    $study_options = array(
      0 => '- Select -',
      1 => 'Natural Population (Landscape)',
      2 => 'Growth Chamber',
      3 => 'Greenhouse',
      4 => 'Experimental/Common Garden',
      5 => 'Plantation',
    );

    $names = array();
    for ($i = 1; $i <= $form_state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
      $names[$i] = $form_state['saved_values'][TPPS_PAGE_1]['organism'][$i]['name'];
    }
    $names['number'] = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
    $params['author'] = $form_state['saved_values'][TPPS_PAGE_1]['primaryAuthor'];
    $params['title'] = $form_state['saved_values'][TPPS_PAGE_1]['publication']['title'];
    $params['journal'] = $form_state['saved_values'][TPPS_PAGE_1]['publication']['journal'];
    $params['organisms'] = $names;
    $params['study_type'] = $study_options[$form_state['saved_values'][TPPS_PAGE_2]['study_type']];
    $params['data_type'] = $form_state['saved_values'][TPPS_PAGE_2]['data_type'];
    $params['accession'] = $form_state['accession'];
    $params['base_url'] = $base_url;
    $params['body'] = '';

    $params['headers'][] = 'MIME-Version: 1.0';
    $params['headers'][] = 'Content-type: text/html; charset=iso-8859-1';

    drupal_mail('tpps', 'user_recieved', $to, user_preferred_language($user), $params, $from, TRUE);

    $params['user_mail'] = $user->mail;

    drupal_mail('tpps', 'admin_recieved', $admin, user_preferred_language($user), $params, $from, TRUE);
  }
  else {
    $admin = variable_get('tpps_admin_email', "treegenesdb@gmail.com");

    $params = array();

    $to = $user->mail;
    $from = variable_get('site_mail', '');
    $params['subject'] = "TPPSC Submission Received: {$form_state['saved_values'][TPPS_PAGE_1]['publication']['title']}";

    $study_options = array(
      0 => '- Select -',
      1 => 'Natural Population (Landscape)',
      2 => 'Growth Chamber',
      3 => 'Greenhouse',
      4 => 'Experimental/Common Garden',
      5 => 'Plantation',
    );

    $params['author'] = $form_state['saved_values'][TPPS_PAGE_1]['primaryAuthor'];
    $params['title'] = $form_state['saved_values'][TPPS_PAGE_1]['publication']['title'];
    $params['organisms'] = $form_state['saved_values'][TPPS_PAGE_1]['organism'];
    $params['study_type'] = $study_options[$form_state['saved_values'][TPPS_PAGE_2]['study_type']];
    $params['data_type'] = $form_state['saved_values'][TPPS_PAGE_2]['data_type'];
    $params['accession'] = $form_state['accession'];
    $params['base_url'] = $base_url;
    $params['body'] = '';

    $params['headers'][] = 'MIME-Version: 1.0';
    $params['headers'][] = 'Content-type: text/html; charset=iso-8859-1';

    drupal_mail('tpps', 'user_recieved', $to, user_preferred_language($user), $params, $from, TRUE);

    $params['user_mail'] = $user->mail;

    drupal_mail('tpps', 'admin_recieved', $admin, user_preferred_language($user), $params, $from, TRUE);

  }
}
