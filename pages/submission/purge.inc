<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. TPPS Study View Form.
 */
function tpps_submission_purge_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  $message = t(
    'Are you sure you want to purge Submission @accession?',
    ['@accession' => $accession]
  );
  $cancel_redirect = 'tpps/submission';
  $description = t("Submission @accession will be fully removed.",
    ['@accession' => $accession]
  ) . theme('item_list', [
    'items' => [
      t('To partly remove submission use "!remove_page_link".',
      [
        '!remove_page_link' => l(t('Submission Remove page'),
          'tpps/submission/' . $accession . '/remove'
        ),
      ]),
      t('To review submission submission use "!view_page_link".', [
        '!view_page_link' => l(t('Submistion View page'),
          'tpps/submission/' . $accession . '/view'
        ),
      ]),
    ]
  ]);
  $confirm_button_text = t('Purge');
  $cancel_button_text = t('Cancel');
  // @TODO Get accession from URL (if it's safe).
  $form['tpps_submission_accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];

  $form = confirm_form(
    $form,
    $message,
    $cancel_redirect,
    $description,
    $confirm_button_text,
    $cancel_button_text
  );
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_purge_form_submit(array $form, array &$form_state) {
  $accession = $form_state['values']['tpps_submission_accession'] ?? NULL;
  tpps_submission_list_validate_accession($accession);
  // Purge study.
  $purge_options = [
    'clear_db' => TRUE,
    'remove_files' => TRUE,
  ];
  tpps_submission_load($accession)->purge($purge_options);
  drupal_goto('tpps/submission');
}
