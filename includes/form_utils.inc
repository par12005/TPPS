<?php

/**
 * @file
 * Defines useful functions to be used with the TPPS form.
 */

/**
 * Creates a dynamic list fieldset complete with add/remove buttons and ajax.
 *
 * This function accepts a form object, form state object, an id, and a field
 * to repeat, and creates a fieldset with buttons to add/remove items from the
 * list. This function also supports a variety of additional options, which can
 * be specified in the $options array. The list will be inserted at $form[$id],
 * unless parents are specified in the options, in which case the list will be
 * inserted at $form[...$parents][$id].
 *
 * @param array $form
 *   The form array that the new list will be generated for.
 * @param array $form_state
 *   The form state array of the provided form.
 * @param string $id
 *   The id of the dynamic list.
 * @param array $repeat
 *   The field to repeat.
 * @param array $options
 *   Additional options.
 */
function tpps_dynamic_list(array &$form, array &$form_state, $id, array $repeat, array $options = array()) {
  $label = $options['label'] ?? $id;
  $parents = $options['parents'] ?? [];
  $number_parents = $parents;
  array_push($number_parents, $id, 'number');
  $up = $options['up'] ?? "Add $label";
  $down = $options['down'] ?? "Remove $label";
  $name_suffix = $options['name_suffix'] ?? "";
  $default = $options['default'] ?? 0;
  $minimum = $options['minimum'] ?? $default;
  $fieldset_title = $options['title'] ?? "$label information:";
  $button_callback = $options['callback'] ?? "tpps_{$id}_callback";
  $list_wrapper = $options['wrapper'] ?? "$id-wrapper";
  $sub_keys = $options['substitute_keys'] ?? array();
  $sub_fields = $options['substitute_fields'] ?? array();
  $replace_pattern = $options['replacement_pattern'] ?? '/!num/';
  $alt_buttons = $options['alternative_buttons'] ?? array();
  $button_weights = $options['button_weights'] ?? array();

  $element = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => "<div class=\"fieldset-title\">$fieldset_title</div>",
    '#collapsible' => $options['collapsible'] ?? TRUE,
    '#prefix' => "<div id=\"$list_wrapper\">",
    '#suffix' => '</div>',
    // [VS] Added ability to set Drupal States for fieldsets.
    '#states' => $options['states'] ?? [],
    // [/VS]
    'add' => array(
      '#type' => 'button',
      '#button_type' => 'button',
      '#value' => $up,
      '#name' => $up . $name_suffix,
      '#ajax' => array(
        'wrapper' => $list_wrapper,
        'callback' => $button_callback,
      ),
      '#weight' => $button_weights[$up] ?? NULL,
    ),
    'remove' => array(
      '#type' => 'button',
      '#button_type' => 'button',
      '#value' => $down,
      '#name' => $down . $name_suffix,
      '#ajax' => array(
        'wrapper' => $list_wrapper,
        'callback' => $button_callback,
      ),
      '#weight' => $button_weights[$down] ?? NULL,
    ),
  );

  $number_options = array();
  foreach ($alt_buttons as $button => $inc_value) {
    $element[$button] = array(
      '#type' => 'button',
      '#button_type' => 'button',
      '#value' => $button,
      '#name' => $button . $name_suffix,
      '#ajax' => array(
        'wrapper' => $list_wrapper,
        'callback' => $button_callback,
      ),
      '#weight' => $button_weights[$button] ?? NULL,
    );
    $number_options[$button . $name_suffix] = $inc_value;
  }

  $number = tpps_get_ajax_number($form_state, $number_parents, $up . $name_suffix, $down . $name_suffix, $default, $minimum, $number_options);

  $element['number'] = array(
    '#type' => 'hidden',
    '#value' => $number,
  );

  for ($i = 1; $i <= $number; $i++) {
    $instance = $repeat;
    foreach ($sub_fields as $field) {
      if (!is_array($field)) {
        $instance[$field] = preg_replace($replace_pattern, $i, $repeat[$field]);
      }
      else {
        $new_value = preg_replace($replace_pattern, $i, drupal_array_get_nested_value($repeat, $field));
        drupal_array_set_nested_value($instance, $field, $new_value);
      }
    }
    foreach ($sub_keys as $key) {
      $value = drupal_array_get_nested_value($instance, $key);
      drupal_array_set_nested_value($instance, $key, NULL);
      $new_key = array();
      foreach ($key as $key_item) {
        $new_key[] = preg_replace($replace_pattern, $i, $key_item);
      }
      drupal_array_set_nested_value($instance, $new_key, $value);
    }
    $element[$i] = $instance;
  }

  $element_parents = $parents;
  array_push($element_parents, $id);
  drupal_array_set_nested_value($form, $element_parents, $element);
}

/**
 * Retrieves the value of the number of a tpps_dynamic_list fieldset.
 *
 * This function first checks the values of the form state, then the
 * saved_values, and finally returns the default if it cannot find the number.
 * This function also accepts an optional $options array, which contains
 * alternative increment values, or functions which return a new number. This
 * can be useful for adding buttons to a dynamic list such as "Add 5 of x".
 *
 * @param array $state
 *   The state of the form.
 * @param array $parents
 *   The parents leading to the list number.
 * @param string $up
 *   The #name of the button to increase the list number.
 * @param string $down
 *   The #name of the button to decrease the list number.
 * @param int $default
 *   The default number if the number does not exist.
 * @param int $minimum
 *   The minimum number we are allowed to reach.
 * @param array $options
 *   Optional additional increment values.
 *
 * @return int
 *   The ajax number of the dynamic list, or the default number.
 */
function tpps_get_ajax_number(array &$state, array $parents, $up, $down, $default = 0, $minimum = 0, array $options = array()) {
  // Get page number.
  $page = $state['stage'] ?? '';

  // Check form_state values, update if necessary.
  $value_parents = $parents;
  // If the triggering element is an upload/remove file button, then the
  // form_state['values'] array will not be loaded correctly, so we will need
  // to use the 'complete form' array instead.
  $button_name = $state['triggering_element']['#name'] ?? '';
  if (preg_match('/^.*_(upload|remove)_button$/', $button_name)) {
    array_unshift($value_parents, 'complete form');
    $val = drupal_array_get_nested_value($state, $value_parents);
    if (isset($val['#type']) and $val['#type'] == 'hidden') {
      array_push($value_parents, '#value');
    }
  }
  else {
    // [VS] There was 'values' as a parent for $value_parents but it doesn't
    // have recent changes from browser on AJAX-requests but 'input' has!
    array_unshift($value_parents, 'input');
  }
  $val = drupal_array_get_nested_value($state, $value_parents);
  if (isset($val) and array_key_exists($button_name, $options)) {
    $increment = $options[$button_name];
    $new_val = $val;
    if (is_int($increment)) {
      $new_val = $val + $increment;
      if ($new_val < $minimum) {
        $new_val = $minimum;
      }
    }
    if (is_string($increment) and function_exists($increment)) {
      $new_val = $increment($button_name, $val);
    }
    drupal_array_set_nested_value($state, $value_parents, $new_val);
  }
  elseif (isset($val) and $button_name == $up) {
    drupal_array_set_nested_value($state, $value_parents, $val + 1);
  }
  elseif (isset($val) and $button_name == $down and $val > $minimum) {
    drupal_array_set_nested_value($state, $value_parents, $val - 1);
  }
  $val = drupal_array_get_nested_value($state, $value_parents);
  if (isset($val)) {
    return $val;
  }

  // Check saved values.
  // [VS] 'saved_values' has only previous step changes and current
  // step changes sent using AJAX are not present. Element 'input' is the only
  // place with actual data for AJAX-requests.
  $saved_value_parents = $parents;
  array_unshift($saved_value_parents, 'saved_values', $page);
  $saved_val = drupal_array_get_nested_value($state, $saved_value_parents);
  if (isset($saved_val)) {
    return $saved_val;
  }

  // Found nothing, return default.
  return $default;
}


/**
 * Retrieves the value of a field that may have been altered through ajax.
 *
 * This function first checks the 'values' of the form state,
 * then 'complete form', then the 'saved_values', and finally
 * returns the default if it cannot find the value.
 * If the value we are trying to reach is part of a TPPS managed_file, then we
 * can also pass the file name in order to find saved_values such as
 * 'file-columns', or 'file-no-header'.
 *
 * @param array $form_state
 *   Drupal Form API state of the form.
 * @param array $parents
 *   The parents leading to the list number.
 *   Path to the form element which value must to be found.
 * @param mixed $default
 *   The default value to return.
 *   Will be returned if form element not found.
 * @param string $file_name
 *   Optional - the name of the file.
 *
 * @return mixed
 *   The value of the specified field, or the default value.
 */
function tpps_get_ajax_value(array &$form_state, array $parents, $default = NULL, $file_name = "") {
  if ($value = tpps_form_get_field_value($form_state['values'], $parents)) {
    return $value;
  }
  if ($value = tpps_form_get_field_value($form_state['complete form'], $parents)) {
    return $value;
  }

  // @TODO Try to reuse tpps_form_get_field_value(). Maybe $file_name could
  // be obtained from $form_state using field's type?
  $saved_value_parents = $parents;
  if (!empty($file_name)) {
    $saved_value_parents = array();
    $last = '';
    foreach ($parents as $item) {
      if ($last == $file_name) {
        $item = "$file_name-$item";
      }
      $last = $item;
      if ($item == $file_name) {
        continue;
      }
      $saved_value_parents[] = $item;
    }
  }
  // Note: Value of $form_state['stage'] is a page number.
  array_unshift($saved_value_parents, 'saved_values', $form_state['stage']);
  return drupal_array_get_nested_value($form_state, $saved_value_parents) ?? $default;
}

/**
 * Migrates necessary information from the old form state to the new one.
 *
 * This function is usually only called from tpps_main().
 *
 * @param array $new
 *   The new form state to be populated.
 * @param array $old
 *   The old form state with the old information.
 */
function tpps_form_state_info(array &$new, array &$old) {
  $new['saved_values'] = $old['saved_values'];
  $new['stage'] = $old['stage'];
  $new['accession'] = $old['accession'];
  $new['dbxref_id'] = $old['dbxref_id'];
  $new['stats'] = $old['stats'] ?? NULL;
  $new['ids'] = $old['ids'] ?? NULL;
  $new['tpps_type'] = $old['tpps_type'] ?? NULL;
  $new['file_info'] = $old['file_info'] ?? NULL;
  $new['status'] = $old['status'] ?? NULL;
  $new['updated'] = $old['updated'] ?? time();
  $new['created'] = $old['created'] ?? NULL;
  $new['approved'] = $old['approved'] ?? NULL;
  $new['completed'] = $old['completed'] ?? NULL;
  $new['loaded'] = $old['loaded'] ?? NULL;
  $new['submitting_uid'] = $old['submitting_uid'] ?? NULL;
  $new['job_id'] = $old['job_id'] ?? NULL;
  $new['revised_files'] = $old['revised_files'] ?? NULL;
  $new['admin_comments'] = $old['admin_comments'] ?? NULL;
  $new['alternative_accessions'] = $old['alternative_accessions'] ?? NULL;
  $new['data'] = $old['data'] ?? NULL;
  $new['phenotypes_edit'] = $old['phenotypes_edit'] ?? NULL;
}

/**
 * If there have been no form errors, then the file is valid, so preserve it.
 *
 * Also adds the standardized name and fid to the submission state file_info.
 *
 * @param array $form_state
 *   The form state being validated.
 * @param int $fid
 *   The Drupal file ID.
 * @param mixed $org_num
 *   The species number to get the species name from the form state array.
 * @param mixed $prefix
 *   The standardized file name prefix.
 */
function tpps_preserve_valid_file(array &$form_state, $fid, $org_num = NULL, $prefix = NULL) {
  // [VS]
  if (!form_get_errors() && $file = file_load($fid)) {
    // Label file as permanent when it's passed validation.
    $file->status = FILE_STATUS_PERMANENT;
    file_save($file);
    file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    if (!empty($org_num) and !empty($prefix)) {
      $species = implode('_', explode(' ', $form_state['saved_values'][TPPS_PAGE_1]['organism'][$org_num]['name']));
      $form_state['file_info'][TPPS_PAGE_3][$file->fid] = "{$prefix}_{$species}";
    }
  }
  // [/VS]
}

/**
 * Initializes TPPS VCF file pre-validation jobs.
 *
 * This function also requires that the body of the pre-validation request
 * contains the file ids of all of the VCF files that need to be pre-validated.
 *
 * @param string $accession
 *   The accession of the submission to pre-validate.
 */
function tpps_pre_validate_init($accession = NULL) {
  $state = tpps_load_submission($accession);
  if (empty($accession) or empty($state)) {
    drupal_json_output("Submission could not be loaded from accession");
    return;
  }
  $params = drupal_get_query_parameters(NULL, array());
  $vcfs = $params['vcfs'] ?? NULL;
  if (!is_array($vcfs) or empty($vcfs)) {
    drupal_json_output("No VCF file ids were provided");
    return;
  }

  global $user;
  $state['vcf_replace'] = array();
  foreach ($vcfs as $org_num => $fid) {
    if (!file_load($fid)) {
      if (!is_string($fid)) {
        drupal_json_output("Could not load one or more VCFs from file ID");
        return;
      }
      elseif (!file_exists($fid)) {
        drupal_json_output("One or more VCF local paths was invalid");
        return;
      }
      else {
        $existing_files = file_load_multiple(FALSE, array('uri' => $fid));
        if (!empty($existing_files)) {
          $file = current($existing_files);
        }
        else {
          $file = new stdClass();
          $file->fid = NULL;
          $file->uri = $fid;
          $file->filename = drupal_basename($fid);
          $file->filemime = file_get_mimetype($file->uri);
          $file->uid = $user->uid;
          $file->status = FILE_STATUS_PERMANENT;
          $file = file_save($file);
        }

        $vcfs[$org_num] = $file->fid;
        $state['vcf_replace'][$org_num] = $file->fid;
      }
    }
  }
  if (empty($state['vcf_replace'])) {
    unset($state['vcf_replace']);
  }

  $state['vcf_val_errors'] = array();
  $state['vcf_validated'] = FALSE;
  tpps_update_submission($state);

  $jobs = array();
  foreach ($vcfs as $org_num => $fid) {
    $includes = array();
    $includes[] = module_load_include('inc', 'tpps', 'includes/form_utils');
    $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
    $args = array($accession, $fid, $org_num);
    $jobs[] = tripal_add_job("{$accession} Pre-validate VCF {$fid}", 'tpps', 'tpps_pre_validate', $args, $user->uid, 10, $includes, TRUE);
  }
  drupal_get_messages('status', TRUE);

  drupal_json_output($jobs);
}

/**
 * This function will show the diff between accession file tree ids
 * and vcf tree ids
 */
function tpps_compare_accession_file_vs_vcf_file_tree_ids($accession = NULL, $i = 1) {
  require_once(__DIR__ . '/../forms/submit/submit_all.php');
  $form_state = tpps_load_submission($accession); // get the form state
  $results_accession_file_tree_ids = tpps_accession_get_accession_file_tree_ids($form_state, $i);

  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  $vcf_location = tpps_vcf_location($form_state, $i); // returns array['status','location'];
  $results_vcf_file_tree_ids = [];
  if ($vcf_location['status'] == 'exists') {
    $results_vcf_file_tree_ids = tpps_genotype_get_vcf_tree_ids($vcf_location['location']);
  }
  // echo json_encode($results_accession_file_tree_ids);
  // echo json_encode($results_vcf_file_tree_ids);

  $results_not_overlapping_in_vcf_file = array_diff($results_accession_file_tree_ids['values'], $results_vcf_file_tree_ids['values']);
  $results_not_overlapping_in_accession_file = array_diff($results_vcf_file_tree_ids['values'], $results_accession_file_tree_ids['values']);
  $overall_results = [
    'tree_ids_not_in_vcf' => $results_not_overlapping_in_vcf_file,
    'tree_ids_not_in_vcf_count' => count($results_not_overlapping_in_vcf_file),
    'tree_ids_not_in_accession' => $results_not_overlapping_in_accession_file,
    'tree_ids_not_in_accession_count' => count($results_not_overlapping_in_accession_file),
  ];
  echo json_encode($overall_results);

}

/**
 * This helper function is used in the api call /tpps/<accession>/accession-file-tree-ids
 * to get all tree_ids detected from a tree accession file
 */
function tpps_accession_file_tree_ids($accession = NULL, $i = 1) {
  require_once(__DIR__ . '/../forms/submit/submit_all.php');
  $form_state = tpps_load_submission($accession); // get the form state
  drupal_json_output(tpps_accession_get_accession_file_tree_ids($form_state, $i));


}

/** This function will generate the tree_ids and place it in a standardized
 *  array format similar to other functions like vcf markers etc. The actually
 *  array design is a specific key format to make things easy to understand
 */
function tpps_accession_get_accession_file_tree_ids($form_state, $i) {
  require_once(__DIR__ . '/../includes/file_utils.inc');

  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  // $location = tpps_accession_file_location($form_state, $i);
  $fid = $thirdpage['tree-accession']['species-' . $i]['file'];
  $options = [
    'tree_id_column' => $thirdpage['tree-accession']['species-' . $i]['file-groups']['Tree Id'][1]
  ];
  $options['no_header'] = !empty($tree_accession['file-no-header']); // detect if there is a header or not
  tpps_file_iterator($fid, 'tpps_accession_file_tree_ids_iterate_line', $options);

  $output = [
    'values' => array_keys($options['values']),
    'duplicate_values' => array_keys($options['duplicate_values']),
    'count' => $options['count'],
    'unique_count' => $options['unique_count']
  ];
  return $output;
}

/**
 * This function iterates through each row of the accession file and returns
 * the tree_ids based on the column specified in TPPS
 */
function tpps_accession_file_tree_ids_iterate_line($row, &$options, $job = NULL) {
  $tree_id_column = $options['tree_id_column']; // eg A
  if (!isset($options['values'])) {
    $options['values'] = [];
    $options['duplicate_values'] = [];
    $options['unique_count'] = 0;
    $options['count'] = 0;
  }
  $tree_id = $row[$tree_id_column];

  $options['count'] = $options['count'] + 1; // increase count by 1
  if (isset($options['values'][$tree_id])) {
    $options['duplicate_values'][$tree_id] = 1; // set as duplicate_value
  }
  else {
    $options['unique_count'] = $options['unique_count'] + 1; // it's unique so increment
  }
  $options['values'][$tree_id] = 1; // record that the tree exists
}


/**
 * This helper function is used in the api call /tpps/<accession>/vcf_tree_ids
 * to get all tree-ids detected from a vcf file
 */
function tpps_vcf_tree_ids($accession = NULL, $i = 1) {
  require_once(__DIR__ . '/../forms/submit/submit_all.php');

  $form_state = tpps_load_submission($accession);
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  $vcf_location = tpps_vcf_location($form_state, $i); // returns array['status','location'];
  $result = [];
  if ($vcf_location['status'] == 'exists') {
    $result = tpps_genotype_get_vcf_tree_ids($vcf_location['location']);
  }
  drupal_json_output($result);
}


/**
 * This helper function is used in the api call /tpps/<accession>/vcf_markers
 * to get all markers detected from a vcf file
 */
function tpps_vcf_markers($accession = NULL, $i = 1) {
  require_once(__DIR__ . '/../forms/submit/submit_all.php');

  $form_state = tpps_load_submission($accession);
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  $vcf_location = tpps_vcf_location($form_state, $i); // returns array['status','location'];
  $result = [];
  if ($vcf_location['status'] == 'exists') {
    $result = tpps_genotype_get_vcf_markers($vcf_location['location']);
  }
  drupal_json_output($result);
}

function tpps_snps_assay_markers($accession = NULL, $i = 1) {
  require_once(__DIR__ . '/../forms/submit/submit_all.php');
  $form_state = tpps_load_submission($accession);
  $snps_assay_location = tpps_snps_assay_location($form_state, $i); // returns array['status', 'location'];
  // print_r($snps_assay_location);
  $result = [];
  if ($snps_assay_location['status'] == 'exists') {
    $result = tpps_genotype_get_snps_assay_markers($snps_assay_location['fid']);
  }
  drupal_json_output($result);
}

// function tpps_assay_design_markers($accession = NULL, $i = 1) {
//   require_once(__DIR__ . '/../forms/submit/submit_all.php');
//   $form_state = tpps_load_submission($accession);
//   $assay_design_location = tpps_assay_design_location($form_state, $i); // returns array['status', 'location'];
//   // print_r($snps_assay_location);
//   $result = [];
//   if ($assay_design_location['status'] == 'exists') {
//     $result = tpps_genotype_get_snps_assay_markers($assay_design_location['fid']);
//   }
//   drupal_json_output($result);
// }

function tpps_assay_design_markers($accession = NULL, $i = 1) {
  require_once(__DIR__ . '/../forms/submit/submit_all.php');
  $form_state = tpps_load_submission($accession); // get the form state
  drupal_json_output(tpps_get_assay_design_file_markers($form_state, $i));


}

/** This function will generate the markers and place it in a standardized
 *  array format similar to other functions like vcf markers etc. The actually
 *  array design is a specific key format to make things easy to understand
 */
function tpps_get_assay_design_file_markers($form_state, $i) {
  require_once(__DIR__ . '/../includes/file_utils.inc');

  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  $fid = $genotype['files']['assay-design'];
  $file_headers = tpps_file_headers($fid, FALSE);
  $marker_name_column = NULL; // empty until found, this will contain A or B or C etc
  foreach ($file_headers as $column => $column_name) {
    $column_name_normalized = strtolower($column_name);
    $column_name_normalized = str_replace(' ', '_', $column_name_normalized);
    if ($column_name_normalized == 'marker_name') {
      $marker_name_column = $column;
      break;
    }
  }
  // print_r($file_headers);
  // print_r($marker_name_column);
  // exit;
  $options = [
    'marker_name_column' => $marker_name_column
  ];
  $options['no_header'] = FALSE; // detect if there is a header or not
  tpps_file_iterator($fid, 'tpps_get_assay_design_file_markers_iterate_line', $options);

  $output = [
    'values' => array_keys($options['values']),
    'duplicate_values' => array_keys($options['duplicate_values']),
    'count' => $options['count'],
    'unique_count' => $options['unique_count']
  ];
  return $output;
}

/**
 * This function iterates through each row of the accession
 * file and return the markers based on the column 'marker_name' in TPPS
 */
function tpps_get_assay_design_file_markers_iterate_line($row, &$options, $job = NULL) {
  $marker_name_column = $options['marker_name_column']; // eg A
  if (!isset($options['values'])) {
    $options['values'] = [];
    $options['duplicate_values'] = [];
    $options['unique_count'] = 0;
    $options['count'] = 0;
  }
  $marker_name = $row[$marker_name_column];

  $options['count'] = $options['count'] + 1; // increase count by 1
  if (isset($options['values'][$marker_name])) {
    $options['duplicate_values'][$marker_name] = 1; // set as duplicate_value
  }
  else {
    $options['unique_count'] = $options['unique_count'] + 1; // it's unique so increment
  }
  $options['values'][$marker_name] = 1; // record that the marker name exists
}

/**
 * This function will show the diff between vcf markers
 * and snps assay markers - outputs JSON output and used for API call
 */
function tpps_compare_vcf_markers_vs_snps_assay_markers($accession = NULL, $i = 1) {
  $overall_results = tpps_compare_vcf_markers_vs_snps_assay_markers_results_array($accession = NULL, $i = 1);
  echo json_encode($overall_results);
}

/**
 * This function will do the actual comparing between vcf_markers and snps_assay markers
 * and returns an array
 */
function tpps_compare_vcf_markers_vs_snps_assay_markers_results_array($accession = NULL, $i = 1) {
  require_once(__DIR__ . '/../forms/submit/submit_all.php');

  // VCF markers
  $form_state = tpps_load_submission($accession);
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
  $vcf_location = tpps_vcf_location($form_state, $i); // returns array['status','location'];
  $results_vcf_markers = [
    'values' => []
  ];
  if ($vcf_location['status'] == 'exists') {
    $results_vcf_markers = tpps_genotype_get_vcf_markers($vcf_location['location']);
  }

  // SNPS Assay Markers
  $form_state = tpps_load_submission($accession);
  $snps_assay_location = tpps_snps_assay_location($form_state, $i); // returns array['status', 'location'];
  // print_r($snps_assay_location);
  $results_snps_assay_markers = [
    'values' => []
  ];
  if ($snps_assay_location['status'] == 'exists') {
    $results_snps_assay_markers = tpps_genotype_get_snps_assay_markers($snps_assay_location['fid']);
  }

  $results_not_overlapping_in_vcf = array_diff($results_snps_assay_markers['values'], $results_vcf_markers['values']);
  $results_not_overlapping_in_snps_assay = array_diff($results_vcf_markers['values'], $results_snps_assay_markers['values']);
  $overall_results = [
    'markers_not_in_vcf' => $results_not_overlapping_in_vcf,
    'markers_not_in_vcf_count' => count($results_not_overlapping_in_vcf),
    'markers_not_in_snps_assay' => $results_not_overlapping_in_snps_assay,
    'markers_not_in_snps_assay_count' => count($results_not_overlapping_in_snps_assay),
  ];
  return $overall_results;
}

/**
 * This code can be used to check missing tree ids in accession file
 * It gets all vcf tree_ids and compares it with the accession file to see
 * what is missing from the accession file.
 */
function tpps_vcf_tree_validation($accession = NULL) {
  $state = tpps_load_submission($accession);
  $firstpage = $state['saved_values'][TPPS_PAGE_1];
  $thirdpage = $state['saved_values'][TPPS_PAGE_3];
  $fourthpage = $state['saved_values'][TPPS_PAGE_4];
  $organism_number = $firstpage['organism']['number'];
  for ($i = 1; $i <= $organism_number; $i++) {
    $genotype = $fourthpage["organism-$i"]['genotype'] ?? NULL;
    $vcf_fid = $genotype['files']['vcf'];

    // Check the type of VCF file (uploaded or in the cluser)
    // Get file location
    if ($vcf_fid > 0) {
      // File uploaded
      $vcf_file = file_load($vcf_fid);
      $location = tpps_get_location($vcf_file->uri);
    }
    else {
      // File linked by curation in the cluster
      $location = $genotype['files']['local_vcf'];
    }
    if ($location == null || $location == "") {
      throw new Exception('Could not find location of VCF even though the VCF option was specified.
      File ID was 0 so its not an uploaded file. local_vcf variable returned empty so cannot use that');
    }
    echo "VCF location: $location\n";

    $vcf_content = gzopen($location, 'r');
    $columns = [];
    $tree_ids_vcf = [];
    while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
      if (strpos($vcf_line, '#CHROM') !== FALSE) {
        // get tabbed values
        $columns = explode("\t", $vcf_line);
        $columns_count = count($columns);
        for ($j = 9; $j < $columns_count; $j++) {
          $tree_ids_vcf[trim($columns[$j])] = true;
        }
        break;
      }
    }
    echo $tree_ids_vcf['ALAA-20-1'];
    // Now we have the tree_ids from the VCF in $tree_ids_vcf
    // print_r($tree_ids_vcf);

    // Step 2, get all tree_ids from the accession file
    $tree_accession = $thirdpage['tree-accession']["species-$i"];
    $column_vals = $tree_accession['file-columns'];
    $groups = $tree_accession['file-groups'];
    $fid = $tree_accession['file'];
    $tree_ids_accession = [];
    $options['tree_ids'] = &$tree_ids_accession;
    $options['column_tree_id'] = $groups['Tree Id']['1'];
    $options['no_header'] = !empty($tree_accession['file-no-header']);

    tpps_file_iterator($fid, 'tpps_get_tree_ids_from_iterator', $options);
    // print_r($tree_ids_accession);

    // Step 3, compare vcf tree_ids count with accession tree_ids count
    $trees_ids_count_vcf = count(array_keys($tree_ids_vcf));
    $trees_ids_count_accession = count(array_keys($tree_ids_accession));
    echo $tree_ids_accession['ALAA-20-1'];
    echo "Unique Tree ID count from VCF: $trees_ids_count_vcf\n";
    echo "Unique Tree ID count from accession: $trees_ids_count_accession\n";

    if ($trees_ids_count_vcf != $trees_ids_count_accession) {
      // Perform a diff between VCF and Accession Tree Ids
      $trees_ids_vcf_array = array_keys($tree_ids_vcf);
      $trees_ids_accession_array = array_keys($tree_ids_accession);

      $missing_tree_ids = [];
      foreach ($trees_ids_vcf_array as $tree_id_vcf) {
        $tree_id_vcf = trim($tree_id_vcf);
        if (array_search(trim($tree_id_vcf), $trees_ids_accession_array) === false) {
          $missing_tree_ids[$tree_id_vcf] = true;
        }
      }
      echo "The following trees are missing from accession file:\n";
      $results = implode(',', array_keys($missing_tree_ids));
      echo $results;
    }
  }
}

function tpps_get_tree_ids_from_iterator($row, array &$options, $job = NULL) {
  // print_r($options);
  // print_r($row);
  $options['tree_ids'][trim($row[$options['column_tree_id']])] = true;
}

/**
 * Check the status of a TPPS pre-validation job.
 *
 * @param string $accession
 *   The accession of the submission being pre-validated.
 * @param int $jid
 *   The id of the pre-validation job.
 */
function tpps_pre_validate_status($accession = NULL, $jid = NULL) {
  $job = tripal_get_job($jid);

  if ($job->status == 'Completed') {
    $state = tpps_load_submission($accession);
    $job->val_errors = $state['vcf_val_errors'] ?? array();
    if (empty($job->val_errors)) {
      $state['vcf_validated'] = TRUE;
      tpps_update_submission($state);
    }
  }

  drupal_json_output($job);
}

/**
 * Executes a TPPS pre-validation job.
 *
 * @param string $accession
 *   The accession of the submission being pre-validated.
 * @param int $fid
 *   The id of the file to be pre-validated.
 * @param int $org_num
 *   The organism number that this file is associated with.
 * @param mixed $job
 *   The TripalJob object of the current job.
 */
function tpps_pre_validate($accession, $fid, $org_num, $job = NULL) {
  if (empty($job)) {
    return;
  }
  $job->logMessage("[INFO] TGDR Accession: $accession");
  $job->logMessage("[INFO] VCF File ID: $fid");
  $job->logMessage("[INFO] Organism Number: $org_num");
  $vcf_file = file_load($fid);
  if (!$vcf_file) {
    $job->logMessage("[ERROR] VCF File failed to load", array(), TRIPAL_ERROR);
    return;
  }
  $job->logMessage("[INFO] VCF File Location: {$vcf_file->uri}");

  $state = tpps_load_submission($accession);
  if (empty($state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
    $species_index = 'species-1';
  }
  else {
    $species_index = "species-$org_num";
  }
  $tree_accession_file = $state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file'];
  $id_col_accession_name = $state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];
  $accession_ids = tpps_parse_file_column($tree_accession_file, $id_col_accession_name);

  $job->logMessage("[INFO] Extracting VCF archive...");
  $location = tpps_get_location($vcf_file->uri);
  $job->logMessage("[INFO] Opening $location...");
  $vcf_content = gzopen($location, 'r');
  $stocks = array();
  while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
    if (preg_match('/#CHROM/', $vcf_line)) {
      $vcf_line = explode("\t", $vcf_line);
      for ($j = 9; $j < count($vcf_line); $j++) {
        $stocks[] = trim($vcf_line[$j]);
      }
      break;
    }
  }

  $state_errors = $state['vcf_val_errors'] ?? array();
  if (count($stocks) == 0) {
    $message = "unable to parse Plant Identifiers. The format of your VCF file must be invalid";
    $job->logMessage("[ERROR] $message", array(), TRIPAL_ERROR);
    $state_errors[] = "Genotype VCF File: $message";
  }
  else {
    $missing_plants = array();
    foreach ($stocks as $stock_id) {
      if (array_search($stock_id, $accession_ids) === FALSE) {
        $missing_plants[] = $stock_id;
      }
    }
    if (count($missing_plants) > 0) {
      $missing_plants = implode(', ', $missing_plants);
      $message = "We found Plant Identifiers in your VCF file that were not present in your accession file. Please either add these plants to your accession file or remove them from your VCF file. The missing plants are: {$missing_plants}.";
      $job->logMessage("[ERROR] $message", array(), TRIPAL_ERROR);
      $state_errors[] = "Genotype VCF File: $message";
    }
  }

  if (empty($state_errors)) {
    $job->logMessage("[INFO] VCF Validated successfully - no errors!");
  }
  else {
    $state['vcf_val_errors'] = $state_errors;
    tpps_update_submission($state);
  }
}
