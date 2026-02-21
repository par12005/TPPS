<?php

/**
 * @file
 * Shows list of all Zenodo Depositon Files.
 */

/**
 * Menu callback. Shows list of all Zenodo Depositon Files.
 */
function tpps_submission_zenodo_files_list_form(array $form, array &$form_state = [], $accession = NULL) {
  global $user;
  // Show Zenodo Host on page.
  if (
    empty($form_state['triggering_element'])
    && ZenodoFilesWidget::isActionAllowed($user)
  ) {
    ZenodoHost::printStatus();
  }

  TppsForm::validateAccession($accession);

  // Zenodo Deposition Widget.
  $path = 'tpps/submission';
  $context = [
    'debug_mode' => FALSE,
    'form' => &$form,
    'accession' => $accession,
    'zenodo_deposit_page_path' => $path . '/' . $accession . '/zenodo',
    'zenodo_files_page_path' => $path . '/' . $accession . '/zenodo/files',
  ];
  ZenodoFilesWidget::add($context);
  return $form;
}

/**
 * Form submitter for 'tpps_submission_list_form' form.
 */
function tpps_submission_zenodo_files_list_form_submit(array &$form, array &$form_state) {
  $accession = $form_state['values']['accession'] ?? '';
  $action = $form_state['triggering_element']['#name'];

  // 'Back' button.
  if ($action == 'back') {
    // For some reasons messages from export/import pages duplicates when 'Back'
    // button clicked so we simply clear all messages out.
    drupal_get_messages();
    drupal_goto('tpps/submission/' . $accession . '/zenodo');
  }
  elseif ($action == 'upload_all') {
    // Existing files will be overwritten and will got new ids.
    drupal_get_messages();
    $submission = new Submission($accession);
    if ($submission->doesNotExist()) {
      drupal_set_message(t('Submission @accession not found.',
        ['@accession' => $accession]
      ));
      drupal_goto('tpps/submission');
    }
    if (empty($submission->getZenodoDepositId())) {
      drupal_set_message(
        t('Submission "@accession" has empty Zenodo Deposit Id.',
        ['@accession' => $accession]
      ), 'error');
      drupal_goto('tpps/submission/' . $accession . '/zenodo');
    }
    // @TODO Minor. Use 'update' to be sure old files removed.
    $submission->actionAllFilesZenodo('create');
  }
  $form_state['#rebuild'] = TRUE;
}
