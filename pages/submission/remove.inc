<?php

/**
 * @file
 * Removes parts of the TPPS Submission.
 */

/**
 * Menu callback. TPPS Submission Remove Form.
 */
function tpps_submission_remove_form(array $form, array &$form_state, $accession = '') {
  if (empty($accession)) {
    drupal_set_message(t('Empty accession.'), 'warning');
    drupal_goto('tpps/submission');
  }
  $submission = new Submission($accession);
  $removement_options = $submission->getRemovementOptions($accession);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Confirmation form.
  if (isset($form_state['saved_values']['confirmed'])) {
    // Get removement options.
    $selected_options = [];
    foreach ($removement_options as $name => $item) {
      if (!empty(FormState::get($form_state['saved_values'], [$name]))) {
        $selected_options[$name] = $item['title'];
      }
    }
    if (empty($selected_options)) {
      drupal_set_message(t("Nothing was selected to remove."), 'warning');
      drupal_goto('tpps/submission/' . $accession . '/remove');
    }
    $message = t(
      'Confirm removement of the Submission @accession?',
      ['@accession' => $accession]
    );
    $cancel_redirect = 'tpps/submission';
    $description = t("Are you sure?");
    // Show list of the selected removement options.
    $description .= '<br />' . t("Removement options of the submission "
      . "@accession:", ['@accession' => $accession]);
    $description .= theme('item_list', [
      'items' => $selected_options,
    ]);

    $confirm_button_text = t('Remove');
    $cancel_button_text = t('Cancel');
    // @TODO Get accession from URL (if it's safe).

    $form = confirm_form(
      $form,
      $message,
      $cancel_redirect,
      $description,
      $confirm_button_text,
      $cancel_button_text
    );
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Selection form.
  else {
    // Show removement options.
    $form['options'] = [
      '#type' => 'fieldset',
      '#title' => t('Removement options'),
      '#collapsible' => TRUE,
    ];
    foreach ($removement_options as $name => $item) {
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
    // Buttons.
    $form['actions'] = ['#type' => 'container'];
    $form['actions']['submit'] = [
      '#type' => 'submit',
      '#value' => t('Remove'),
      '#name' => 'remove',
    ];
    $form['actions']['button'] = [
      '#type' => 'submit',
      '#value' => t('Purge'),
      '#name' => 'purge',
    ];
  }

  $form['tpps_submission_accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_remove_form_submit(array $form, array &$form_state) {
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Confirmed removement.
  if (FormState::get($form_state, ['saved_values', 'confirmed'])) {
    $accession = FormState::get($form_state,
      ['saved_values', 'tpps_submission_accession']
    );
    if (empty($accession)) {
      drupal_set_message(t("Empty submission's accession."));
      drupal_goto('tpps/submission');
    }
    tpps_submission_list_validate_accession($accession);
    $submission = new Submission($accession);
    $removement_options = $submission->getRemovementOptions();

    // Get removement options.
    $options = [];
    foreach (array_keys($removement_options) as $name) {
      $options[$name] = (bool) $form_state['saved_values'][$name];
    }
    // Remove study.
    if (0) {
      $submission->remove($options);
    }
    else {
      dpm(var_export($options, 1));
    }
    // Show new status. Not all parts could be removed so it makes sense.
    drupal_goto('tpps/submission/' . $accession . '/remove');
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Not yet confirmed.
  else {
    $accession = $form_state['values']['tpps_submission_accession'] ?? NULL;
    if (empty($accession)) {
      drupal_set_message(t("Empty accession."), 'warning');
      drupal_goto('tpps/submission');
    }
    if (FormState::getClickedButton($form_state) == 'purge') {
      drupal_goto('tpps/submission/' . $accession . '/purge');
    }
    $submission = new Submission($accession);
    $form_state['saved_values']['confirmed'] = TRUE;
    $values_to_store = array_keys($submission->getRemovementOptions($accession));
    $values_to_store[] = 'tpps_submission_accession';
    foreach ($values_to_store as $name) {
      $form_state['saved_values'][$name] = FormState::get($form_state,
        ['values', $name]
      );
    }
    // Rebuild form to show confirmation page.
    $form_state['rebuild'] = TRUE;
  }
}
