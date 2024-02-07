<?php

/**
 * @file
 * Functions for manipulation of TPPS Submissions.
 *
 * ----------------------------------------------------------------------------
 * Concept.
 *
 * Submission interface initially is a copy of the Submission Form State
 * modified for Submission Pipeline to always have the same structure and
 * data format (type). For this each Submission Form State has version
 * (default version is '1') and function tpps_submission_interface_generate()
 * will convert existing $form_state to the $submission_interface which
 * will be stored in public.tpps_submissions database table.
 *
 * ----------------------------------------------------------------------------
 * Migration.
 *
 * |  tpps_load_submission()    |  tpps_submission_interface_load()    |
 * |  tpps_update_submission()  |  tpps_submission_interface_update()  |
 *
 * ----------------------------------------------------------------------------
 * Usage.
 *
 * To use:
 *   module_load_include('inc', 'tpps', 'includes/submission_interface');
 *
 * Since submission data could be updated from admin's pages and they not use
 * a single function but update DB table directly it's safer for now to update
 * Submission Interface Array on each change of Submission State Array.
 *
 * Study statuses previously stored serialized in colum
 * tpps_submission.submission_state.
 * So we will store status for now inside Submission Interface Array as well.
 *
 * ----------------------------------------------------------------------------
 * Serialization.
 *
 * Submission Interface Array stored in database as serialized array but
 * in code we use normal PHP-array (unserialized).
 *
 * ----------------------------------------------------------------------------
 * Dates.
 *
 * There are several dates which also was stored in $submission_interface.
 *  updated
 *  created
 *  approved
 *  completed
 *  loaded
 *
 *  @todo Better to store them not inside serialized array.
 *  Create separate table for study statuses:
 *  Accession | Status Id | Timestamp
 *
 * ----------------------------------------------------------------------------
 * Usage tips.
 *
 * To get updated and serialized Submission Interface ready to be stored in DB:
 * 'submission_interface' => serialize(tpps_submission_interface_generate($state)),
 *
 * To exclude only 'submission_interface' from the list of query's fields:
 *   $query = db_select('tpps_submission', 's')
 *     // Exclude 'submission_interface' column.
 *     ->fields('s', array_diff(
 *       array_keys(drupal_get_schema('tpps_submission')['fields']),
 *       ['submission_interface']
 *     ));
 *
 * Exclude 'submission_interface' from chado_query().
 *   // Exclude 'submission_interface' column.
 *   $fields = implode(', ', array_diff(
 *     array_keys(drupal_get_schema('tpps_submission')['fields']),
 *     ['submission_interface']
 *   ));
 *   $all_submissions = chado_query("SELECT $fields FROM public.tpps_submission;");
 *
 * ----------------------------------------------------------------------------
 * @todo Use 'study_state' or even 'study' instead of 'Submission Interface'
 * because 'Submission Interface' is a name of the feature but not name of
 * the array with data.
 *
 * @todo See submit_all.php:75. There 'status' is stored twice:
 * 1. $submission_interface['status'].
 * 2. public.tpps_submission.status.
 * Check if $submission_interface['status'] is used anywhere and replace with
 * public.tpps_submission.status. Then update this code to use
 * Submission Interface only. Do not use tpps_update_submission() but
 * update db table directly to avoid $submission_interface update.
 */

/**
 * Generates new Submission Interface by Submission Accession.
 *
 * @param string $accession
 *   Study Accession.
 *
 * @return mixed
 *   Returns array with datastructure prepared for TPPS pipeline
 *   (submit_all.php script).
 *   Returns empty array if $form_state is empty too.
 */
function tpps_submission_interface_new($accession) {
  $form_state = tpps_load_submission_state($accession, 1);
  $interface = tpps_submission_interface_generate($form_state);
  return tpps_submission_interface_update($interface);
}

/**
 * Creates Submission Interface from $form_state array.
 *
 * @param array $form_state
 *   Drupal Form State array.
 *
 * @return mixed
 *   Returns array with datastructure prepared for TPPS pipeline
 *   (submit_all.php script).
 *   Returns empty array if $form_state is empty too.
 */
function tpps_submission_interface_generate(array $form_state = []) {
  $debug_mode = FALSE;
  if (empty($form_state)) {
    watchdog('tpps', 'Empty Submission Form State.', [], WATCHDOG_WARNING);
    return [];
  }

  // Initialize Submission Interface.
  // Do NOT change $form_state but update $interface instead.
  $interface = $form_state;

  // Form State Version.
  // We do not use 'semantic versioning' because there is no sence to have it
  // and each version will be incremental change so all previous changes must
  // be applied first. Using simple integers will allow to easily detect
  // which version must be applied.
  $current_version = tpps_submission_interface_version_get($form_state);

  module_load_include('inc', 'tpps', 'includes/submission_form_version');
  $change_list = tpps_submission_form_get_change_list();
  // We need to apply changes from 1st to $current version to have
  // incremental changes.
  $i = TPPS_SUBMISSION_FORM_START_VERSION;
  for (; $i <= $current_version; $i++) {
    foreach ($change_list[$i] as $action => $action_data) {
      $function = 'tpps_submission_interface_action_' . $action;
      if (!function_exists($function)) {
        watchdog('tpps', 'Submission Interface Action Function not exists. '
          . '<br />Function name: @function_name. <br />Version: @version.',
          ['@function_name' => $function, '@version' => $i], WATCHDOG_ERROR
        );
        continue;
      }
      if (empty($action_data)) {
        watchdog('tpps', 'There is no data for Submission Interface Action. '
          . '<br />Function name: @function_name. <br />Version: @version.',
          ['@function_name' => $function, '@version' => $i], WATCHDOG_ERROR
        );
        continue;
      }
      // Call function.
      // Note: $action_data also couldn't be empty here.
      // Action-functions are (for search):
      // tpps_submission_interface_action_unset();
      // tpps_submission_interface_action_move().
      call_user_func_array($function, [&$interface, $action_data]);
    }
  }
  return $interface;
}

/**
 * Gets Submission Interface for given study.
 *
 * Note:
 * Forked tpps_load_submission(). Code was improved.
 *
 * @param string $accession
 *   The accession of the submission in format 'TGDRxxxx'.
 *
 * @return array
 *   Returns Submission Interface Array which is ready for pipeline processing.
 *   Returns empty array if Submission wan't found.
 *
 * @todo Think about caching. See tpps_load_submission_state().
 * But probably it could cause problems.
 */
function tpps_submission_interface_load($accession) {
  $result = db_select('tpps_submission', 's')
    ->fields('s', ['submission_interface'])
    ->condition('accession', $accession)
    ->range(0, 1)
    ->execute()->fetchAssoc();

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (!$result) {
    // Try to find study by 'accession' in 'tpps_submission_dbxref' table.
    $query = db_select('tpps_submission', 's');
    $query->join(
      'tpps_submission_dbxref',
      's_dbx',
      's.tpps_submission_id = s_dbx.tpps_submission_id'
    );
    $query->join('chado.dbxref', 'dbx', 'dbx.dbxref_id = s_dbx.dbxref_id');
    $query->fields('s', ['submission_interface'])
      ->condition('dbx.accession', $accession)
      ->condition('s.status', 'Approved')
      ->range(0, 1);
    $result = $query->execute()->fetchAssoc();
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Process results.
  if ($result) {
    if (!empty($result['submission_interface'])) {
      return unserialize($result['submission_interface']);
    }
    else {
      // Create new Submission Interface and return it.
      return tpps_submission_interface_new($accession);
    }
  }
  else {
    // We should return empty array to not break other code which expects array.
    return [];
  }
}

/**
 * Updates Submission Interface using given array.
 *
 * By default given array will overwrite SIA stored in DB but it could contain
 * extra data if processing was completed and to store those data we could
 * merge existing SIA with new values. To do this set variable:
 *  variable_set('tpps_submission_interface_merge_on_update', TRUE);
 * or use admin/config/tpps/form/main
 * to check parameter 'Merge Submission Interface on update'.
 *
 * To update submission's status:
 *   tpps_update_submission_info($accession, $data);
 * To update submission's Form State:
 *   tpps_update_submission_state($form_state);
 *
 * @param array $interface
 *   The Submission Interface Array.
 * @param string $status
 *   Optional. Study processing status.
 *   This value will be added to the Submission Interface Array and to
 *   public.tpps_submission.status column.
 *
 * @return mixed
 *   Returns Submission Interface Array which will have updated 'status' and
 *   'updated' fields.
 *   Returns NULL if $form_state or Study accession is empty.
 */
function tpps_submission_interface_update(array &$interface, $status = NULL) {
  if (empty($interface) || empty($interface['accession'])) {
    return NULL;
  }

  $accession = $interface['accession'];
  // Merge or overwrite?
  if (variable_get('tpps_submission_interface_merge_on_update', FALSE)) {
    $interface = array_merge(
      tpps_submission_interface_load($accession, RESET_CACHE),
      $interface
    );
  }
  // Note: REQUEST_TIME couldn't be used because called from CLI script and
  // processing could take long time from script call.
  $interface['updated'] = time();

  // Prepare data for query.
  $fields = [];
  if ($status) {
    $fields['status'] = $interface['status'] = $status;
  }
  $fields['submission_interface'] = serialize($interface);

  db_update('tpps_submission')
    ->fields($fields)
    ->condition('accession', $accession)
    ->execute();
  return $interface;
}

/**
 * Submission Interface Action 'unset'.
 *
 * Given list of fields will be removed/unset in Submission Interface.
 *
 * @param array $interface
 *   Submission Interface passed by reference so no need to return it.
 * @param array $action_data
 *   Data required for this action.
 *   See tpps_submission_form_get_change_list().
 *   Each item is path to the single field.
 *   For more details about format see: tpps_submission_interface_action_path().
 */
function tpps_submission_interface_action_unset(array &$interface, array $action_data) {
  module_load_include('inc', 'tpps', 'includes/form');
  foreach ($action_data as $key => $item) {
    if (empty($item)) {
      continue;
    }
    $target = &tpps_form_get_path($interface, $item);
    if (empty($target['parent_element']) || empty($target['field_name'])) {
      watchdog('tpps', 'Submission Interface Action "unset" #@key failed.'
        . '<br />Value: <pre>@item</pre>.',
        ['@key' => $key, '@item' => print_r($item, 1)], WATCHDOG_ERROR
      );
      continue;
    }
    // Note:
    // $target['parent_element'] is a reference to the element inside $interface.
    unset($target['parent_element'][$target['field_name']]);
  }
}

/**
 * Submission Interface Action 'move'.
 *
 * Given list of fields will be moved/relocate to the new location inside
 * Submission Interface Array.
 *
 * @param array $interface
 *   Submission Interface passed by reference so no need to return it.
 * @param array $action_data
 *   Data required for this action.
 *   See tpps_submission_form_get_change_list().
 *   Each item contains information for a single field relocation and
 *   represented by list of parents (including field name) with 2 keys:
 *   'source' and 'target'.
 */
function tpps_submission_interface_action_move(array &$interface, array $action_data) {
  module_load_include('inc', 'tpps', 'includes/form');
  foreach ($action_data as $key => $item) {
    if (empty($item['source']) || empty($item['target'])) {
      continue;
    }
    $source = &tpps_form_get_path($interface, $item['source']);
    $target = &tpps_form_get_path($interface, $item['target']);
    if (
      empty($source['parent_element']) || empty($source['field_name'])
      || empty($target['parent_element']) || empty($target['field_name'])
    ) {
      watchdog('tpps', 'Submission Interface Action "move" #@key failed.'
        . '<br />Value: <pre>@item</pre>.',
        ['@key' => $key, '@item' => print_r($item, 1)], WATCHDOG_ERROR
      );
      continue;
    }
    // Note: $source['parent_element'] and $target['parent_element']
    // are a references to the elements inside $interface.
    $target['parent_element'][$target['field_name']]
      = $source['parent_element'][$source['field_name']];
    unset($source['parent_element'][$source['field_name']]);
  }
}
