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

  $submission = new Submission();
  $submission->state = $form_state;
  $is_tppsc = $submission->isTppsc();

  $page1_values = $form_state['saved_values'][TPPS_PAGE_1] ?? NULL;
  $page2_values = $form_state['saved_values'][TPPS_PAGE_2] ?? NULL;
  $from = variable_get('site_mail', '');

  $params = [];
  $params['type'] = ($is_tppsc) ? 'tppsc' : 'tpps';
  $params['type_label'] = ($is_tppsc) ? t('TPPSc') : t('TPPS');
  $params['subject'] = t('@type_label Submission Received: @title',
    [
      '@title' => $page1_values['publication']['title'],
      '@type_label' => $params['type_label'],
    ]
  );
  // Get human readable title of study type.
  $params['study_type'] = tpps_form_get_study_type($page2_values['study_type']);

  // List of organism names.
  $params['organism_list'] = [];
  for ($i = 1; $i <= $page1_values['organism']['number']; $i++) {
    $params['organism_list'][$i] = $page1_values['organism'][$i]['name'];
  }
  $params['organism_number'] = $page1_values['organism']['number'];
  $params['author'] = $page1_values['primaryAuthor'];
  $params['title'] = $page1_values['publication']['title'];
  $params['journal'] = $page1_values['publication']['journal'];
  $params['data_type'] = $page2_values['data_type'];
  $params['accession'] = $form_state['accession'];
  $params['body'] = '';
  $send = TRUE;

  // To user.
  $user_lang = user_preferred_language($user);
  drupal_mail('tpps', 'user_recieved', $user->mail, $user_lang, $params, $from, $send);

  // To admin.
  // Note: admin will get a short email without study details.
  $admin_email = variable_get('tpps_admin_email', "treegenesdb@gmail.com");
  $params['user_mail'] = $user->mail;
  // Since this email couldn't belong to any Drupal User we use default language.
  drupal_mail('tpps', 'admin_recieved', $admin_email, language_default(), $params, $from, $send);
}
