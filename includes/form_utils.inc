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
    array_unshift($value_parents, 'values');
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
 * @param array $state
 *   The state of the form.
 * @param array $parents
 *   The parents leading to the list number.
 * @param mixed $default
 *   The default value to return.
 * @param string $file_name
 *   Optional - the name of the file.
 *
 * @return mixed
 *   The value of the specified field, or the default value.
 */
function tpps_get_ajax_value(array &$state, array $parents, $default = NULL, $file_name = "") {
  $page = $state['stage'];
  $value_parents = $parents;
  array_unshift($value_parents, 'values');
  $element = drupal_array_get_nested_value($state, $value_parents);
  if (isset($element['#type']) and $element['#type'] != 'fieldset') {
    array_push($value_parents, '#value');
  }
  $val = drupal_array_get_nested_value($state, $value_parents);
  if (isset($val)) {
    return $val;
  }

  // @TODO [VS] Avoid code duplication. See above.
  $complete_parents = $parents;
  array_unshift($complete_parents, 'complete form');
  $element = drupal_array_get_nested_value($state, $complete_parents);
  if (isset($element['#type']) and $element['#type'] != 'fieldset') {
    array_push($complete_parents, '#value');
  }
  $val = drupal_array_get_nested_value($state, $complete_parents);
  if (isset($val)) {
    return $val;
  }

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
  array_unshift($saved_value_parents, 'saved_values', $page);
  $saved_val = drupal_array_get_nested_value($state, $saved_value_parents);
  return $saved_val ?? $default;
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
 * This helper function is used in the api call /tpps/<accession>/accession_tree_ids
 * to get all tree_ids detected from a tree accession file
 * TODO
 */
function tpps_accession_tree_ids($accession = NULL, $i) {
  require_once(__DIR__ . '/../forms/submit/submit_all.php');
  $form_state = tpps_load_submission($accession);
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  // $file = file_load($thirdpage['tree-accession']['species-' . $i]['file']);
  // $location = $file->uri;
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
 * to get all tree-ids detected from a vcf file
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

/**
 * Gets domain name.
 *
 * @return string
 *   Returns domain name.
 */
function tpps_get_hostname() {
  global $base_url;
  return explode('://', $base_url)[1];
}

/**
 * Adds buttons Back/Save/Next to the forms.
 *
 * The same buttons will be added above and below main form.
 *
 * @param array $form
 *   Drupal Form API array.
 * @param string $page
 *   Page number with 'page_' prefix to improve readability.
 * @param array
 *   Submission metadata.
 *   'organism_number' - int Organism Number;
 *   'data_type' - Contains 'G' for genotype, 'P' for phenotype, 'E' for environment;
 */
function tpps_add_buttons(array &$form, $page, $meta = []) {
  global $user;

  $scope_list = [
    'header' => -100,
    'footer' => 100,
  ];
  foreach ($scope_list as $scope => $weight) {
    $form[$scope] = [
      '#type' => 'fieldset',
      '#tree' => FALSE,
      '#weight' => $weight,
    ];
    // @todo [VS] Use Drupal Form API 'required' attribute.
    $form[$scope]['input_description'] = [
      '#markup' => '<div class="input-description">* : Required Field</div>',
      '#weight' => -($weight),
    ];
    switch ($page) {
      case 'page_2':
      case 'page_3':
        $form[$scope]['Back'] = ['#type' => 'submit', '#value' => t('Back')];

      default:
      case 'page_1':
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['Next'] = ['#type' => 'submit', '#value' => t('Next')];
        break;

      case 'page_4':
        $form[$scope]['Back'] = ['#type' => 'submit', '#value' => t('Back')];
        $form[$scope]['Save'] = ['#type' => 'submit', '#value' => t('Save')];
        $form[$scope]['submit'] = [
          '#type' => 'submit',
          '#value' => t('Review Information and Submit'),
        ];

        // Pre-validate VCF.
        if (preg_match('/G/', $meta['data_type'])) {
          $pre_valid_states = array();
          for ($i = 1; $i <= $meta['organism_number']; $i++) {
            $pre_valid_states["input[name=\"organism-{$i}[genotype][files][file-type][VCF]\"]"]
              = ['checked' => FALSE];
          }
          $form[$scope]['pre_validate'] = [
            '#type' => 'button',
            '#value' => t('Pre-validate my VCF files'),
            '#states' => ['invisible' => $pre_valid_states],
            '#suffix' => '<div>Sometimes large VCF files can cause problems '
              . 'during the validation step. If your VCF file is very large, '
              . 'you can click the "Pre-validate my VCF files" button to '
              . 'validate your VCF file in the background, before clicking '
              . '"Review Information and Submit".</div>',
          ];
          $form[$scope]['Back']['#prefix'] .= "<div id=\"pre-validate-message\"></div>";
        }

        // Diagnostic utilities for curation.
        if (in_array('Curation', $user->roles) || in_array('administrator', $user->roles)) {
          $module_path = drupal_get_path('module', 'tpps');
          $form['#attached']['js'][] = $module_path . '/js/tpps_page_4.js';
          $form['#attached']['css'][] = $module_path . '/css/tpps_page_4.css';

          $form[$scope]['diagnostics-curation'] = [
            '#type' => 'markup',
            '#markup' => '<h2 style="margin-top:10px;">🌟 Curation Diagnostics</h2>'
              . '<div>These diagnostics <b>require you to save this package</b> '
              . 'with data before functions will work</div>',
          ];

          $form[$scope]['diagnostics-curation-style'] = [
            '#type' => 'markup',
            '#markup' => '
              <style>
                .cd-inline {
                  display: inline-block;
                }
                .cd-inline-round-blue {
                  display: inline-block;
                  background-color: #00a5ff;
                  color: white;
                  padding: 3px;
                  margin-top: 3px;
                  margin-right: 3px;
                  border-radius: 5px;
                  width: 150px;
                }
                .cd-inline-round-red {
                  display: inline-block;
                  background-color: red;
                  color: white;
                  padding: 3px;
                  margin-top: 3px;
                  margin-right: 3px;
                  border-radius: 5px;
                  width: 150px;
                }
              </style>
            '
          ];
// @todo Add accession number to Drupal.settings.
        $accession = $form_state['accession'];


          // Check VCF Tree Ids.
          $form[$scope]['button-check-vcf-tree-ids'] = [
            '#type' => 'button',
            '#value' => 'Check VCF Tree IDs',
            '#attributes' => ['class' => 'button-check-vcf-tree-ids'],
          ];

          $js_onclick_code = "
            <script>
            function check_vcf_markers() {
              jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">⏰</h1>Checking VCF Markers...');
              jQuery.ajax({
                url: '/tpps/" . $accession . "/vcf-markers',
                error: function (err) {
                  console.log(err);
                  jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>It might be that this VCF is just too big to process it in time. Please contact Administration.');
                },
                success: function (data) {
                  console.log(data);
                  if (!Array.isArray(data)) {
                    // Data was returned, this is good
                    var html = '';
                    html += '<div>';
                    html += '🧬 Unique markers found: ' + data['unique_count'];
                    html += ' | ';
                    html += '🧬 Total markers found: ' + data['count'];
                    html += '</div>';
                    if (data['unique_count'] != data['count']) {
                      html += '<div>⚡ There are duplicate markers in this VCF since unique count does not match count</div>';
                      html += '<hr /><div>Duplicate Markers (' + data['duplicate_values'].length + ')</div>';
                      for (var i=0; i<data['duplicate_values'].length; i++) {
                        html += '<div class=\"cd-inline-round-red\">' + data['duplicate_values'][i] + '</div>';
                      }
                    }
                    else {
                      html += '<div>🆗 No duplicate markers found in the VCF file</div>';
                    }
                    html += '<hr /><div>Unique Markers (' + data['values'].length + ')</div>';
                    for (var i=0; i<data['values'].length; i++) {
                      html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
                    }
                    jQuery('#diagnostic-curation-results').html(html);
                  }
                  else {
                    jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>No results returned, make sure you saved valid data on this page and retry. Double check VCF existence as well.');
                  }
                }
              });
            }
            </script>
          ";
          $form[$scope]['button-check-vcf-markers'] = array(
            '#type' => 'button',
            '#prefix' => $js_onclick_code . '',
            '#value' => 'Check VCF Markers',
            '#attributes' => array(
              "onclick" => "javascript:check_vcf_markers(); return false;"
            ),
          );


          $js_onclick_code = "
            <script>
            function check_snps_assay_markers() {
              jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">⏰</h1>Checking SNPs Assay Markers...');
              jQuery.ajax({
                url: '/tpps/" . $accession . "/snps-assay-markers',
                error: function (err) {
                  console.log(err);
                  jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1>It might be that this SNPs Assay is just too big to process it in time. Please contact Administration.');
                },
                success: function (data) {
                  console.log(data);
                  if (!Array.isArray(data)) {
                    // Data was returned, this is good
                    var html = '';
                    html += '<div>';
                    html += '🧬 Unique markers found: ' + data['unique_count'];
                    html += ' | ';
                    html += '🧬 Total markers found: ' + data['count'];
                    html += '</div>';
                    if (data['unique_count'] != data['count']) {
                      html += '<div>⚡ There are duplicate markers in this SNPs assay file since unique count does not match count</div>';
                      html += '<hr /><div>Duplicate Markers (' + data['duplicate_values'].length + ')</div>';
                      for (var i=0; i<data['duplicate_values'].length; i++) {
                        html += '<div class=\"cd-inline-round-red\">' + data['duplicate_values'][i] + '</div>';
                      }
                    }
                    else {
                      html += '<div>🆗 No duplicate markers found in the SNPs Assay file</div>';
                    }
                    html += '<hr /><div>Unique Markers (' + data['values'].length + ')</div>';
                    for (var i=0; i<data['values'].length; i++) {
                      html += '<div class=\"cd-inline-round-blue\">' + data['values'][i] + '</div>';
                    }
                    jQuery('#diagnostic-curation-results').html(html);
                  }
                  else {
                    jQuery('#diagnostic-curation-results').html('<h1 class=\"cd-inline\">🆘</h1> No results returned, make sure you saved valid data on this page and retry. Double check SNPs Assay file existence as well.');
                  }
                }
              });
            }
            </script>
          ";
          $form[$scope]['button-check-snps-assay-markers'] = array(
            '#type' => 'button',
            '#prefix' => $js_onclick_code . '',
            '#value' => 'Check SNPs Assay Markers',
            '#attributes' => array(
              "onclick" => "javascript:check_snps_assay_markers(); return false;"
            ),
          );

          // @todo check!
          //
          $form[$scope]['diagnostic-curation-results'] = [
            '#type' => 'markup',
            '#markup' => '<div id="diagnostic-curation-results" style="max-height: 500px; overflow-y: auto;"></div>'
          ];
        }
        break;

    }
  }
}
