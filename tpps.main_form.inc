<?php

/**
 * @file
 * Contains TPPS Main form, validator and submitter.
 *
 * @TODO [VS] Minor. Change form_id to 'tpps_main_form'. This will require to
 * update all code which uses $form_id.
 */

/**
 * This function creates the main TPPS form.
 *
 * Loads query parameters, ensures the user is logged in, loads old form and
 * form_state information, and calls the appropriate page function.
 *
 * @param array $form
 *   The actual TPPS form.
 * @param array $form_state
 *   The state of the TPPS form - note that this may be overwritten if the user
 *   is loading an old form.
 *
 * @global stdClass $user
 *   The user trying to access the TPPS form.
 *
 * @return array
 *   The created form.
 */
function tpps_main(array $form, array &$form_state, $accession = NULL) {
  global $user;

  $form = $form ?? [];
  module_load_include('php', 'tpps', 'forms/build/front');
  module_load_include('inc', 'tpps', 'includes/form');
  tpps_add_css_js($form);
  // Add class to apply CSS rules differntly for TPPS and TPPSc Forms
  // CSS form id is changing because of AJAX requests so we use form's class
  // in CSS rules instead of form's id with regex.
  $form['#attributes']['class'][] = 'tpps-form';

  if (!isset($form_state['stage'])) {
    if (!isset($accession)) {
      // First access, no accession in link.
      $form_state['stage'] = 'frontpage';

      tpps_front_create_form($form, $form_state);
      return $form;
    }
    else {
      // First access, and with accession in link.
      $old_form_state = tpps_load_submission($accession);
      if (empty($old_form_state)) {
        drupal_goto('tpps');
      }
      tpps_form_state_info($form_state, $old_form_state);
    }
  }
  else {
    // Not first access.
    if (!isset($form_state['saved_values']['frontpage']['accession']) or (isset($form_state['saved_values']['frontpage']['accession']) and $form_state['saved_values']['frontpage']['accession'] === 'new')) {
      // No submissions, or selected to create new submission.
      if (tpps_access('administer tpps module') and !empty($form_state['saved_values']['frontpage']['custom_accession_check'])) {
        $accession = $form_state['saved_values']['frontpage']['custom_accession'];
      }
      tpps_init_project($form_state, $accession);
      tpps_create_submission($form_state, $user->uid);
    }
    $old_form_state = tpps_load_submission($form_state['saved_values']['frontpage']['accession']);
    tpps_form_state_info($form_state, $old_form_state);
  }

  // [VS].
  // @todo Minor.Check if this setting must be added always and move to the
  // beginning of function if possible.
  $form['#attached']['js'][] = [
    'data' => ['tpps' => ['stage' => $form_state['stage']]],
    'type' => 'setting',
  ];
  $form['accession'] = [
    '#type' => 'hidden',
    '#value' => $form_state['accession'],
  ];
  // [/VS].
  if (!isset($form_state['dbxref_id']) or !isset($form_state['accession'])) {
    tpps_init_project($form_state);
  }

  module_load_include('inc', 'tpps', 'includes/status_bar');
  tpps_status_bar($form, $form_state);

  if (
    !isset($form_state['saved_values'][TPPS_PAGE_1]['primaryAuthor'])
    && (
      ($form_state['tpps_type'] ?? NULL) != 'tppsc'
    )
  ) {
    $contact_bundle = tripal_load_bundle_entity(array('label' => 'Tripal Contact Profile'));
    if ($contact_bundle) {
      $query = new EntityFieldQuery();
      $results = $query->entityCondition('entity_type', 'TripalEntity')
        ->entityCondition('bundle', $contact_bundle->name)
        ->fieldCondition('local__email', 'value', $user->mail)
        ->range(0, 1)
        ->execute();
      if (!empty($results['TripalEntity'])) {
        $entity = current(array_reverse(entity_load('TripalEntity',
          array_keys($results['TripalEntity']))));
        $form_state['saved_values'][TPPS_PAGE_1]['primaryAuthor']
          = $form_state['saved_values'][TPPS_PAGE_1]['primaryAuthor'] ?? $entity->title;
      }
    }
  }

  switch ($form_state['stage']) {
    case TPPS_PAGE_1:
      require_once 'forms/build/page_1.php';
      tpps_page_1_create_form($form, $form_state);
      break;

    case TPPS_PAGE_2:
      require_once 'forms/build/page_2.php';
      tpps_page_2_create_form($form, $form_state);
      break;

    case TPPS_PAGE_3:
      require_once 'forms/build/page_3.php';
      tpps_page_3_create_form($form, $form_state);
      break;

    case TPPS_PAGE_4:
      require_once 'forms/build/page_4.php';
      tpps_page_4_create_form($form, $form_state);
      break;

    case 'summarypage':
      require_once 'forms/build/summary.php';
      tpps_summary_create_form($form, $form_state);
      break;

    default:
      drupal_set_message(t('Invalid form stage.'), 'error');
      break;
  }
  return $form;
}

/**
 * Validator for 'tpps_main' form.
 *
 * Validates input to form. Also saves file column information so it is not lost
 * if the input is not valid.
 */
function tpps_main_validate(&$form, &$form_state) {
  global $user;

  if (
    $form_state['triggering_element']['#value'] == 'Back'
    or $form_state['triggering_element']['#value'] == 'Save'
  ) {
    tpps_save_file_columns($form, $form_state);
    return;
  }

  switch ($form_state['stage']) {
    case 'frontpage':
      require_once 'forms/validate/front.php';
      tpps_front_page_validate_form($form, $form_state);
      break;

    case TPPS_PAGE_1:
      require_once 'forms/validate/page_1.php';
      tpps_page_1_validate_form($form, $form_state);
      break;

    case TPPS_PAGE_2:
      require_once 'forms/validate/page_2.php';
      tpps_page_2_validate_form($form, $form_state);
      break;

    case TPPS_PAGE_3:
      require_once 'forms/validate/page_3.php';
      tpps_page_3_validate_form($form, $form_state);
      break;

    case TPPS_PAGE_4:
      require_once 'forms/validate/page_4.php';
      tpps_page_4_validate_form($form, $form_state);
      break;

    default:
      break;
  }

  if ($user->uid !== 0 and !form_get_errors()) {
    $form_state['values']['#cleared'] = TRUE;
    if ($form_state['submitted'] == '1') {
      tpps_submission_update_stats($form_state);
    }
  }
  else {
    $form_state['values']['#cleared'] = FALSE;
  }
}

/**
 * Submitter for 'tpps_main' form.
 *
 * This function handles the multi-step aspect of the form, as well as saving
 * input to persistent variables so they can be reloaded later.
 */
function tpps_main_submit($form, &$form_state) {
  // dpm("ON SUBMIT");
  // dpm(array_keys($form_state));
  // dpm('saved_values key contains:');
  // dpm($form_state['saved_values']);
  // dpm('values key contains:');
  // dpm($form_state['values']);
  $form_state['saved_values'][$form_state['stage']] = $form_state['values'];
  $persist = TRUE;

  // This happens when the user clicks Save, notice this if clause will do a return
  // This means after saving, the form will be rebuilt based on the rebuild key = TRUE.
  if ($form_state['triggering_element']['#value'] == 'Save') {
    drupal_set_message(t('Your information has been saved!'), 'status');
    // dpm($form_state['saved_values']);
    if (isset($form_state['saved_values']['form_build_id'])) {
      $form_state['values']['form_build_id'] = $form_state['saved_values']['form_build_id'];
    }
    $form_state['saved_values']['form_build_id'] = $form_state['values']['form_build_id'];
    $form_state['rebuild'] = TRUE;
    // dpm('ON SAVE');
    // dpm('Form state saved_values:');
    // dpm($form_state['saved_values']);

    tpps_update_submission($form_state);
    return;
  }

  // ELSE, it will do the below logic, read the comment after this switch statement
  switch ($form_state['stage']) {
    case 'frontpage':
      $persist = FALSE;
      $form_state['stage'] = TPPS_PAGE_1;
      break;

    case TPPS_PAGE_1:
      $form_state['stage'] = TPPS_PAGE_2;
      break;

    case TPPS_PAGE_2:
      if ($form_state['triggering_element']['#value'] == 'Back') {
        // [VS]
        // This is not required but to have always up-to-date form fields
        // values which could be modified on validation step.
        $form_state['rebuild'] = TRUE;
        // [/VS]
        $form_state['stage'] = TPPS_PAGE_1;
      }
      else {
        $form_state['stage'] = TPPS_PAGE_3;
      }
      break;

    case TPPS_PAGE_3:
      if ($form_state['triggering_element']['#value'] == 'Back') {
        // [VS]
        // This is not required but to have always up-to-date form fields
        // values which could be modified on validation step.
        $form_state['rebuild'] = TRUE;
        // [/VS]
        $form_state['stage'] = TPPS_PAGE_2;
      }
      else {
        $form_state['stage'] = TPPS_PAGE_4;
      }
      break;

    case TPPS_PAGE_4:
      if ($form_state['triggering_element']['#value'] == 'Back') {
        // [VS]
        // This is not required but to have always up-to-date form fields
        // values which could be modified on validation step.
        $form_state['rebuild'] = TRUE;
        // [/VS]
        $form_state['stage'] = TPPS_PAGE_3;
      }
      else {
        $form_state['stage'] = 'summarypage';
      }
      break;

    case 'summarypage':
      if ($form_state['triggering_element']['#value'] == 'Back') {
        // [VS]
        // On validation metadata field or file could be update so
        // we need to update form.
        $form_state['rebuild'] = TRUE;
        // [/VS]
        $form_state['stage'] = TPPS_PAGE_4;
      }
      else {
        $form_state['status'] = 'Pending Approval';
        $form_state['completed'] = time();
        tpps_update_submission($form_state);
        tpps_submit_email($form_state);
        drupal_goto("completed-submission/{$form_state['accession']}");
        return;
      }
      break;

    default:
      drupal_set_message(t('Invalid form stage.'), 'error');
      break;
  }

  // It'll then come here and also perform a save most likely
  // which I think will cater for NEXT button.
  if (isset($form_state['saved_values']['form_build_id'])) {
    $form_state['values']['form_build_id'] = $form_state['saved_values']['form_build_id'];
  }
  $form_state['saved_values']['form_build_id'] = $form_state['values']['form_build_id'];
  $form_state['rebuild'] = TRUE;

  if ($persist) {
    tpps_update_submission($form_state);
  }
}
