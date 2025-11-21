<?php

/**
 * @file
 * Removes parts of the TPPS Submission.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. TPPS Submission Remove Form.
 */
function tpps_submission_remove_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  $submission = new Submission($accession);
  $message = t(
    'Are you sure you want to remove Submission @accession?',
    ['@accession' => $accession]
  );
  $cancel_redirect = 'tpps/submission';
  $description = t("Submission @accession and it's files will be removed.",
      ['@accession' => $accession]
    ) . '<br />' . l(
        t('Submistion @accession view page', ['@accession' => $accession]),
        'tpps/submission/' . $accession . '/view'
      );
  $confirm_button_text = t('Remove');
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
  // Removement options.
  // @TODO Check each option and show status.
  $form['options'] = [
    '#type' => 'fieldset',
    '#title' => t('Removement options'),
    '#collapsible' => TRUE,
  ];
  $remove_options = tpps_submission_get_remove_options($accession);
  foreach ($remove_options as $name => $item) {

    // @TODO Disable empty parts.
    $status = NULL;
    if (!empty($item['status_method'])) {
      if (method_exists('Submission', $item['status_method'])) {
        $status = call_user_func([$submission, $item['status_method']]);
      }
    }
    $status_message = ' [' . ($status ? '<strong>NOT empty</strong>' : 'Empty') . ']';
    $form['options'][$name] = [
      '#type' => 'checkbox',
      '#title' => $item['title'] . (is_null($status) ? '' : $status_message),
      '#default_value' => $item['default_value'],
      '#description' => ($item['description'] ?? NULL),
    ];
    if (!is_null($status)) {
      $form['options'][$name]['#default_value'] = $status;
      if ($status === FALSE) {
        $form['options'][$name]['#disabled'] = TRUE;
      }
    }
  }
  tpps_add_css_js('theme', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_remove_form_submit(array $form, array &$form_state) {
  $accession = $form_state['values']['tpps_submission_accession'] ?? NULL;
  if (empty($accession)) {
    drupal_set_message(t("Empty submission's accession."));
    drupal_goto('tpps/submission');
  }

  dpm(var_export(
    $form_state['values']
  , 1));

  if ($form_state['values']['#triggered_element']['#name'] == 'Purge') {
    drupal_goto('tpps/submission/' . $accession . '/purge');
  }


drupal_goto('tpps/submission/' . $accession . '/remove');
return;



  tpps_submission_list_validate_accession($accession);
  $submission = new Submission($accession);

  // Get removement options.
  $options = [];
  $removement_options = $submission->getRemovementOptions();
  foreach (array_keys($removement_options) as $name) {
    $options[$name] = (bool) $form_state['values'][$name];
  }
  // Remove study.
  $submission->remove($options);
  drupal_goto('tpps/submission/' . $accession . '/remove');
}
