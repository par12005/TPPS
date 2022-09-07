<?php

/**
 * @file
 * Defines contents of TPPS administrative panel.
 *
 * Site administrators will use this form to approve or reject completed TPPS
 * submissions.
 */

/**
 * Creates the administrative panel form.
 *
 * If the administrator is looking at one specific TPPS submission, they are
 * provided with options to reject the submission and leave a reason for the
 * rejection, or to approve the submission and start loading the data into the
 * database. If the submission includes CartograPlant layers with environmental
 * parameters, the administrator will need to select the kind of parameter the
 * user has selected - an attr_id, or a cvterm. This will be important when the
 * submission is recording the environmental data of the plants.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 *
 * @return array
 *   The administrative panel form.
 */
function tpps_admin_panel(array $form, array &$form_state, $accession = NULL) {
  if (empty($accession)) {
    tpps_admin_panel_top($form);
  }
  else {
    tpps_manage_submission_form($form, $form_state, $accession);
  }

  drupal_add_js(drupal_get_path('module', 'tpps') . TPPS_JS_PATH);
  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);

  return $form;
}

/**
 * Build form to manage TPPS submissions from admin panel.
 *
 * This includes options to change the status or release date of the
 * submission, as well as options to upload revised versions of files.
 *
 * @param array $form
 *   The form element to be populated.
 * @param array $form_state
 *   The state of the form element to be populated.
 * @param string $accession
 *   The accession number of the submission being managed.
 */
function tpps_manage_submission_form(array &$form, array &$form_state, $accession = NULL) {
  global $base_url;
  $submission = tpps_load_submission($accession, FALSE);
  $status = $submission->status;
  $submission_state = unserialize($submission->submission_state);
  if (empty($submission_state['status'])) {
    $submission_state['status'] = $status;
    tpps_update_submission($submission_state);
  }
  $options = array();
  $display = l(t("Back to TPPS Admin Panel"), "$base_url/tpps-admin-panel");

  // Check for log file
  // Step 1 - Look for the last tripal job that has the accession
  $results = db_query("SELECT * FROM public.tripal_jobs WHERE job_name LIKE 'TPPS Record Submission - $accession' ORDER BY submit_date DESC LIMIT 1;");
  $job_id = -1;
  while($row_array = $results->fetchObject()) {
    // dpm($row_array);
    // $display .= print_r($row_array, true);
    $job_id = $row_array->job_id;
  }
  if($job_id == -1) {
    $display .= "<div style='padding: 10px;'>No log file exists for this study (resubmit this study to generate a log file if necessary)</div>";
  }
  else {
    $log_path = drupal_realpath('public://') . '/tpps_job_logs/';
    // dpm($log_path . $accession . "_" . $job_id . ".txt");
    if(file_exists($log_path . $accession . "_" . $job_id . ".txt")) {
      $display .= "<div style='padding: 10px;background: #e9f9ef;border: 1px solid #90bea9;font-size: 18px;'><a target='_blank' href='../tpps-admin-panel-logs/" . $accession . "_" . $job_id . "'>Latest job log file ($accession - $job_id)</a></div>";
    }
    else {
      $display .= "<div style='padding: 10px;'>Could not find job log file (this can happen if the log file was deleted - resubmit study if necessary to regenerate log file)</div>";
    }
  }

  if ($status == "Pending Approval") {
    $options['files'] = array(
      'revision_destination' => TRUE,
    );
    $options['skip_phenotypes'] = TRUE;

    foreach ($submission_state['file_info'] as $files) {
      foreach ($files as $fid => $file_type) {
        $file = file_load($fid) ?? NULL;

        $form["edit_file_{$fid}_check"] = array(
          '#type' => 'checkbox',
          '#title' => t('I would like to upload a revised version of this file'),
          '#prefix' => "<div id=\"file_{$fid}_options\">",
        );

        $form["edit_file_{$fid}_file"] = array(
          '#type' => 'managed_file',
          '#title' => 'Upload new file',
          '#upload_location' => dirname($file->uri),
          '#upload_validators' => array(
            'file_validate_extensions' => array(),
          ),
          '#states' => array(
            'visible' => array(
              ":input[name=\"edit_file_{$fid}_check\"]" => array('checked' => TRUE),
            ),
          ),
        );
        $form["edit_file_{$fid}_markup"] = array(
          '#markup' => '</div>',
        );
      }
    }
  }
  $display .= tpps_table_display($submission_state, $options);

  if ($status == 'Pending Approval' and preg_match('/P/', $submission_state['saved_values'][TPPS_PAGE_2]['data_type'])) {
    $new_cvterms = array();
    for ($i = 1; $i <= $submission_state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
      $phenotype = $submission_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype'];
      for ($j = 1; $j <= $phenotype['phenotypes-meta']['number']; $j++) {
        if ($phenotype['phenotypes-meta'][$j]['structure'] === 'other') {
          $new_cvterms[] = $phenotype['phenotypes-meta'][$j]['struct-other'];
        }
        if ($phenotype['phenotypes-meta'][$j]['attribute'] === 'other') {
          $new_cvterms[] = $phenotype['phenotypes-meta'][$j]['attr-other'];
        }
        if ($phenotype['phenotypes-meta'][$j]['units'] === 'other') {
          $new_cvterms[] = $phenotype['phenotypes-meta'][$j]['unit-other'];
        }
      }
    }
    // @todo get new/custom cvterms from metadata file.
    if (count($new_cvterms) > 0) {
      $message = 'This submission will create the following new local cvterms: ' . implode(', ', $new_cvterms);
      $display .= "<div class=\"alert alert-block alert-dismissible alert-warning messages warning\">
        <a class=\"close\" data-dismiss=\"alert\" href=\"#\">×</a>
        <h4 class=\"element-invisible\">Warning message</h4>
        {$message}</div>";
    }
  }

  $form['accession'] = array(
    '#type' => 'hidden',
    '#value' => $accession,
  );

  $form['form_table'] = array(
    '#markup' => $display,
  );

  $submission_tags = tpps_submission_get_tags($submission_state['accession']);

  // Show current tags.
  $tags_markup = "<label class=\"control-label\">Current Tags:</label><br>";
  $image_path = drupal_get_path('module', 'tpps') . '/images/';
  $query = db_select('tpps_tag', 't')
    ->fields('t')
    ->execute();
  while (($result = $query->fetchObject())) {
    $color = !empty($result->color) ? $result->color : 'white';
    $style = !array_key_exists($result->tpps_tag_id, $submission_tags) ? "display: none" : "";
    $tooltip = $result->static ? "This tag cannot be removed" : "";
    $tags_markup .= "<span title=\"$tooltip\" class=\"tag\" style=\"background-color:$color; $style\"><span class=\"tag-text\">{$result->name}</span>";
    if (!$result->static) {
      $tags_markup .= "<span id=\"{$submission_state['accession']}-tag-{$result->tpps_tag_id}-remove\" class=\"tag-close\"><img src=\"/{$image_path}remove.png\"></span>";
    }
    $tags_markup .= "</span>";
  }

  // Show available tags.
  $tags_markup .= "<br><label class=\"control-label\">Available Tags (click to add):</label><br><div id=\"available-tags\">";
  $query = db_select('tpps_tag', 't')
    ->fields('t')
    ->condition('static', 0)
    ->execute();
  while (($result = $query->fetchObject())) {
    $color = $result->color;
    if (empty($color)) {
      $color = 'white';
    }
    $style = "";
    if (array_key_exists($result->tpps_tag_id, $submission_tags)) {
      $style = 'display: none';
    }
    $tags_markup .= "<span id=\"{$submission_state['accession']}-tag-{$result->tpps_tag_id}-add\" class=\"tag add-tag\" style=\"background-color:{$color}; $style\"><span class=\"tag-text\">{$result->name}</span></span>";
  }
  $tags_markup .= "</div>";
  $tags_markup .= "<a href=\"/tpps-tag\">Manage TPPS Submission Tags</a>";
  $form['tags'] = array(
    '#markup' => "<div id=\"tags\">$tags_markup</div>",
  );

  if ($status == "Pending Approval") {

    if ($submission_state['saved_values'][TPPS_PAGE_2]['study_type'] != 1) {
      module_load_include('php', 'tpps', 'forms/build/page_3_helper');
      module_load_include('php', 'tpps', 'forms/build/page_3_ajax');
      $submission_state['values'] = $form_state['values'] ?? $submission_state['values'];
      $submission_state['complete form'] = $form_state['complete form'] ?? $submission_state['complete form'];
      tpps_study_location($form, $submission_state);
      $study_location = $submission_state['saved_values'][TPPS_PAGE_3]['study_location'];
      $form['study_location']['type']['#default_value'] = $study_location['type'] ?? NULL;
      for ($i = 1; $i <= $study_location['locations']['number']; $i++) {
        $form['study_location']['locations'][$i]['#default_value'] = $study_location['locations'][$i];
      }
      unset($form['study_location']['locations']['add']);
      unset($form['study_location']['locations']['remove']);

      $form['study_location']['#collapsed'] = TRUE;
    }

    $form['params'] = array(
      '#type' => 'fieldset',
      '#title' => 'Select Environmental parameter types:',
      '#tree' => TRUE,
      '#description' => '',
    );

    $orgamism_num = $submission_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
    $show_layers = FALSE;
    for ($i = 1; $i <= $orgamism_num; $i++) {
      if (!empty($submission_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['environment'])) {
        foreach ($submission_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['environment']['env_layers'] as $layer => $layer_id) {
          if (!empty($layer_id)) {
            foreach ($submission_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['environment']['env_params'][$layer] as $param_id) {
              if (!empty($param_id)) {
                $type = variable_get("tpps_param_{$param_id}_type", NULL);
                if (empty($type)) {
                  $query = db_select('cartogratree_fields', 'f')
                    ->fields('f', array('display_name'))
                    ->condition('field_id', $param_id)
                    ->execute();
                  $result = $query->fetchObject();
                  $name = $result->display_name;

                  $form['params'][$param_id] = array(
                    '#type' => 'radios',
                    '#title' => "Select Type for environmental layer parameter \"$name\":",
                    '#options' => array(
                      'attr_id' => t('@attr_id', array('@attr_id' => 'attr_id')),
                      'cvterm' => t('@cvterm', array('@cvterm' => 'cvterm')),
                    ),
                    '#required' => TRUE,
                  );
                  $show_layers = TRUE;
                }
              }
            }
          }
        }
      }
    }

    if (!$show_layers) {
      unset($form['params']);
    }

    if (preg_match('/P/', $submission_state['saved_values'][TPPS_PAGE_2]['data_type'])) {
      tpps_phenotype_editor($form, $form_state, $submission_state);
    }

    $form['approve-check'] = array(
      '#type' => 'checkbox',
      '#title' => t('This submission has been reviewed and approved.'),
    );

    $form['reject-reason'] = array(
      '#type' => 'textarea',
      '#title' => t('Reason for rejection:'),
      '#states' => array(
        'invisible' => array(
          ':input[name="approve-check"]' => array('checked' => TRUE),
        ),
      ),
    );

    $form['REJECT'] = array(
      '#type' => 'submit',
      '#value' => t('Reject'),
      '#states' => array(
        'invisible' => array(
          ':input[name="approve-check"]' => array('checked' => TRUE),
        ),
      ),
    );
  }

  $disable_vcf_import = 1;
  if(!isset($submission_state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'])) {
    $disable_vcf_import = 0;
  }
  else {
    $disable_vcf_import = $submission_state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'];
  }

  $form['DISABLE_VCF_IMPORT'] = array(
    '#type' => 'checkbox',
    '#title' => 'Disable VCF Import in Tripal Job Submission',
    '#default_value' => $disable_vcf_import,
  );

  $form['DISABLE_VCF_IMPORT_SAVE'] = array(
    '#type' => 'submit',
    '#value' => t('Save VCF Import Setting'),
  );  

  $form['admin-comments'] = array(
    '#type' => 'textarea',
    '#title' => t('Additional comments (administrator):'),
    '#default_value' => $submission_state['admin_comments'] ?? NULL,
    '#prefix' => '<div id="tpps-admin-comments">',
    '#suffix' => '</div>',
  );

  if ($status == "Pending Approval") {
    $form['APPROVE'] = array(
      '#type' => 'submit',
      '#value' => t('Approve'),
      '#states' => array(
        'visible' => array(
          ':input[name="approve-check"]' => array('checked' => TRUE),
        ),
      ),
    );
  }


  if ($status != "Pending Approval") {
    $form['SAVE_COMMENTS'] = array(
      '#type' => 'button',
      '#value' => t('Save Comments'),
      '#ajax' => array(
        'callback' => 'tpps_save_admin_comments',
        'wrapper' => 'tpps-admin-comments',
      ),
    );
  }



  $date = $submission_state['saved_values']['summarypage']['release-date'] ?? NULL;
  if (!empty($date)) {
    $datestr = "{$date['day']}-{$date['month']}-{$date['year']}";
    if ($status != 'Approved' or strtotime($datestr) > time()) {
      $form['date'] = array(
        '#type' => 'date',
        '#title' => t('Change release date'),
        '#description' => t('You can use this field and the button below to change the release date of a submission.'),
        '#default_value' => $date,
      );

      $form['CHANGE_DATE'] = array(
        '#type' => 'submit',
        '#value' => t('Change Date'),
        '#states' => array(
          'invisible' => array(
            ':input[name="date[day]"]' => array('value' => $date['day']),
            ':input[name="date[month]"]' => array('value' => $date['month']),
            ':input[name="date[year]"]' => array('value' => $date['year']),
          ),
        ),
      );
    }
  }

  if ($status == "Approved") {
    $alt_acc = $submission_state['alternative_accessions'] ?? '';

    $form['alternative_accessions'] = array(
      '#type' => 'textfield',
      '#title' => t('Alternative accessions'),
      '#default_value' => $alt_acc,
      '#description' => t('Please provide a comma-delimited list of alternative accessions you would like to assign to this submission.'),
    );

    $form['SAVE_ALTERNATIVE_ACCESSIONS'] = array(
      '#type' => 'submit',
      '#value' => t('Save Alternative Accessions'),
    );

  }

  $submitting_user = user_load($submission_state['submitting_uid']);
  $form['change_owner'] = array(
    '#type' => 'textfield',
    '#title' => t('Choose a new owner for the submission'),
    '#default_value' => $submitting_user->mail,
    '#autocomplete_path' => 'tpps/autocomplete/user',
  );

  $form['CHANGE_OWNER'] = array(
    '#type' => 'submit',
    '#value' => t('Change Submission Owner'),
  );  

  $form['state-status'] = array(
    '#type' => 'select',
    '#title' => t('Change state status'),
    '#description' => t('Warning: This feature is experimental and may cause unforseen issues. Please do not change the status of this submission unless you are willing to risk the loss of existing data. The current status of the submission is @status.', array('@status' => $status)),
    '#options' => array(
      'Incomplete' => t('Incomplete'),
      'Pending Approval' => t('Pending Approval'),
      'Submission Job Running' => t('Submission Job Running'),
    ),
    '#default_value' => $status,
  );

  $form['CHANGE_STATUS'] = array(
    '#type' => 'submit',
    '#value' => t('Change Status'),
    '#states' => array(
      'invisible' => array(
        ':input[name="state-status"]' => array('value' => $status),
      ),
    ),
  );
}

/**
 * Build form for administrators to edit phenotypes.
 *
 * @param array $form
 *   The form element to be populated.
 * @param array $form_state
 *   The state of the form element to be populated.
 * @param array $submission
 *   The submission being managed.
 */
function tpps_phenotype_editor(array &$form, array &$form_state, array &$submission) {
  $form['phenotypes_edit'] = array(
    '#type' => 'fieldset',
    '#title' => t('Admin Phenotype Editor'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Note: The phenotype editor does not have any validation measures in place. This means that fields in this section that are left blank will be accepted by TPPS, and they will override any user selections. Please be careful when editing information in this section.'),
  );

  $phenotypes = array();
  for ($i = 1; $i <= $submission['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
    $phenotype = $submission['saved_values'][TPPS_PAGE_4]["organism-$i"]['phenotype'];
    for ($j = 1; $j <= $phenotype['phenotypes-meta']['number']; $j++) {
      $phenotypes[$j] = $phenotype['phenotypes-meta'][$j];
    }
  }

  // @todo get phenotypes from metadata file.
  $attr_options = array();
  $terms = array(
    'age' => 'Age',
    'alive' => 'Alive',
    'amount' => 'Amount',
    'angle' => 'Angle',
    'area' => 'Area',
    'bent' => 'Bent',
    'circumference' => 'Circumerence',
    'color' => 'Color',
    'composition' => 'Composition',
    'concentration_of' => 'Concentration of',
    'damage' => 'Damage',
    'description' => 'Description',
    'diameter' => 'Diameter',
    'distance' => 'Distance',
    'growth_quality_of_occurrent' => 'Growth Quality of Occurrent',
    'growth_rate' => 'Growth Rate',
    'has_number_of' => 'Has number of',
    'height' => 'Height',
    'humidity_level' => 'Humidity Level',
    'intensity' => 'Intensity',
    'length' => 'Length',
    'lesioned' => 'Lesioned',
    'maturity' => 'Maturity',
    'position' => 'Position',
    'pressure' => 'Pressure',
    'proportionality_to' => 'Proportionality to',
    'rate' => 'Rate',
    'rough' => 'Rough',
    'shape' => 'Shape',
    'size' => 'Size',
    'temperature' => 'Temperature',
    'texture' => 'Texture',
    'thickness' => 'Thickness',
    'time' => 'Time',
    'volume' => 'Volume',
    'weight' => 'Weight',
    'width' => 'Width',
  );
  foreach ($terms as $term => $label) {
    $attr_id = tpps_load_cvterm($term)->cvterm_id;
    $attr_options[$attr_id] = $label;
  }
  $attr_options['other'] = 'My attribute term is not in this list';

  $unit_options = array();
  $terms = array(
    'boolean' => 'Boolean (Binary)',
    'centimeter' => 'Centimeter',
    'cubic_centimeter' => 'Cubic Centimeter',
    'day' => 'Day',
    'degrees_celsius' => 'Degrees Celsius',
    'degrees_fahrenheit' => 'Dgrees Fahrenheit',
    'grams_per_square_meter' => 'Grams per Square Meter',
    'gram' => 'Gram',
    'luminous_intensity_unit' => 'Luminous Intensity Unit',
    'kilogram' => 'Kilogram',
    'kilogram_per_cubic_meter' => 'Kilogram per Cubic Meter',
    'liter' => 'Liter',
    'cubic_meter' => 'Cubic Meter',
    'pascal' => 'Pascal',
    'meter' => 'Meter',
    'milligram' => 'Milligram',
    'milliliter' => 'Milliliter',
    'millimeter' => 'Millimeter',
    'micrometer' => 'Micrometer',
    'percent' => 'Percent',
    'qualitative' => 'Qualitative',
    'square_micrometer' => 'Square Micrometer',
    'square_millimeter' => 'Square Millimeter',
    'watt_per_square_meter' => 'Watt per Square Meter',
    'year' => 'Year',
  );
  foreach ($terms as $term => $label) {
    $unit_id = tpps_load_cvterm($term)->cvterm_id;
    $unit_options[$unit_id] = $label;
  }
  $unit_options['other'] = 'My unit is not in this list';

  $struct_options = array();
  $terms = array(
    'whole plant' => 'Whole Plant',
    'bark' => 'Bark',
    'branch' => 'Branch',
    'bud' => 'Bud',
    'catkin_inflorescence' => 'Catkin Inflorescence',
    'endocarp' => 'Endocarp',
    'floral_organ' => 'Floral Organ',
    'flower' => 'Flower',
    'flower_bud' => 'Flower Bud',
    'flower_fascicle' => 'Flower Fascicle',
    'fruit' => 'Fruit',
    'leaf' => 'Leaf',
    'leaf_rachis' => 'Leaf Rachis',
    'leaflet' => 'Leaflet',
    'nut_fruit' => 'Nut Fruit (Acorn)',
    'petal' => 'Petal',
    'petiole' => 'Petiole',
    'phloem' => 'Phloem',
    'plant_callus' => 'Plant Callus (Callus)',
    'primary_thickening_meristem' => 'Primary Thickening Meristem',
    'root' => 'Root',
    'secondary_xylem' => 'Secondary Xylem (Wood)',
    'seed' => 'Seed',
    'shoot_system' => 'Shoot System (Crown)',
    'stem' => 'Stem (Trunk, Primary Stem)',
    'stomatal_complex' => 'Stomatal Complex (Stomata)',
    'strobilus' => 'Strobilus',
    'terminal_bud' => 'Terminal Bud',
    'vascular_leaf' => 'Vascular Leaf (Needle)',
  );
  foreach ($terms as $term => $label) {
    $struct_id = tpps_load_cvterm($term)->cvterm_id;
    $struct_options[$struct_id] = $label;
  }
  $struct_options['other'] = 'My structure term is not in this list';

  foreach ($phenotypes as $num => $info) {
    $form['phenotypes_edit'][$num] = array(
      '#type' => 'fieldset',
      '#title' => t('Phenotype @num (@name):', array(
        '@num' => $num,
        '@name' => $info['name'],
      )),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      'name' => array(
        '#type' => 'textfield',
        '#title' => t('Name'),
        '#value' => $info['name'],
        '#disabled' => TRUE,
      ),
      'description' => array(
        '#type' => 'textfield',
        '#title' => t('Description'),
        '#default_value' => $info['description'],
      ),
      'attribute' => array(
        '#type' => 'select',
        '#title' => t('Attribute'),
        '#options' => $attr_options,
        '#default_value' => $info['attribute'],
      ),
      'attr-other' => array(
        '#type' => 'textfield',
        '#title' => t('Other Attribute'),
        '#autocomplete_path' => 'tpps/autocomplete/attribute',
        '#states' => array(
          'visible' => array(
            ':input[name="phenotypes_edit[' . $num . '][attribute]"]' => array('value' => 'other'),
          ),
        ),
        '#default_value' => $info['attr-other'],
      ),
      'structure' => array(
        '#type' => 'select',
        '#title' => t('Structure'),
        '#options' => $struct_options,
        '#default_value' => $info['structure'],
      ),
      'struct-other' => array(
        '#type' => 'textfield',
        '#title' => t('Other Structure'),
        '#autocomplete_path' => 'tpps/autocomplete/structure',
        '#states' => array(
          'visible' => array(
            ':input[name="phenotypes_edit[' . $num . '][structure]"]' => array('value' => 'other'),
          ),
        ),
        '#default_value' => $info['struct-other'],
      ),
      'units' => array(
        '#type' => 'select',
        '#title' => t('Unit'),
        '#options' => $unit_options,
        '#default_value' => $info['units'],
      ),
      'unit-other' => array(
        '#type' => 'textfield',
        '#title' => t('Other Unit'),
        '#autocomplete_path' => 'tpps/autocomplete/unit',
        '#states' => array(
          'visible' => array(
            ':input[name="phenotypes_edit[' . $num . '][units]"]' => array('value' => 'other'),
          ),
        ),
        '#default_value' => $info['unit-other'],
      ),
    );
  }
}

/**
 * Ajax callback to save admin comments.
 *
 * @param array $form
 *   The admin panel form element.
 * @param array $form_state
 *   The state of the admin panel form element.
 *
 * @return array
 *   The updated form element.
 */
function tpps_save_admin_comments(array $form, array $form_state) {
  $state = tpps_load_submission($form_state['values']['accession']);
  $state['admin_comments'] = $form_state['values']['admin-comments'];
  tpps_update_submission($state);
  drupal_set_message(t('Comments saved successfully'), 'status');
  return $form['admin-comments'];
}

/**
 * Create tables for pending, approved, and incomplete TPPS Submissions.
 *
 * @param array $form
 *   The form element of the TPPS admin panel page.
 */
function tpps_admin_panel_top(array &$form) {
  global $base_url;

  $submissions = tpps_load_submission_multiple(array(), FALSE);

  $pending = array();
  $approved = array();
  $incomplete = array();
  $unpublished_old = array();

  $submitting_user_cache = array();
  $mail_cvterm = tpps_load_cvterm('email')->cvterm_id;

  foreach ($submissions as $submission) {
    $state = unserialize($submission->submission_state);
    $status = $submission->status;
    if (empty($state['status'])) {
      $state['status'] = $status;
      tpps_update_submission($state);
    }

    if (empty($submitting_user_cache[$submission->uid])) {
      $mail = user_load($submission->uid)->mail;
      $query = db_select('chado.contact', 'c');
      $query->join('chado.contactprop', 'cp', 'cp.contact_id = c.contact_id');
      $query->condition('cp.value', $mail);
      $query->condition('cp.type_id', $mail_cvterm);
      $query->fields('c', array('name'));
      $query->range(0, 1);
      $query = $query->execute();
      $name = $query->fetchObject()->name ?? NULL;

      $submitting_user_cache[$submission->uid] = $name ?? $mail;
    }
    $submitting_user = $submitting_user_cache[$submission->uid] ?? NULL;

    if (!empty($state)) {
      switch ($state['status']) {
        case 'Pending Approval':
          $row = array(
            l($state['accession'], "$base_url/tpps-admin-panel/{$state['accession']}"),
            $submitting_user,
            $state['saved_values'][TPPS_PAGE_1]['publication']['title'],
            !empty($state['completed']) ? date("F j, Y, g:i a", $state['completed']) : "Unknown",
            tpps_show_tags(tpps_submission_get_tags($state['accession'])),
          );
          $pending[(int) substr($state['accession'], 4)] = $row;
          break;

        case 'Approved':
          $status_label = !empty($state['loaded']) ? "Approved - load completed on " . date("F j, Y, \a\\t g:i a", $state['loaded']) : "Approved";
          if (!empty($state['loaded'])) {
            $days_since_load = (time() - $state['loaded']) / (60 * 60 * 24);
            $unpublished_threshold = variable_get('tpps_unpublished_days_threshold', 180);
            $pub_status = $state['saved_values'][TPPS_PAGE_1]['publication']['status'] ?? NULL;
            if (!empty($pub_status) and $pub_status != 'Published' and $days_since_load >= $unpublished_threshold) {
              $owner = $submitting_user;
              $contact_bundle = tripal_load_bundle_entity(array('label' => 'Tripal Contact Profile'));

              // If Tripal Contact Profile is available, we want to link to the
              // profile of the owner instead of just displaying the name.
              if ($contact_bundle) {
                $owner_mail = user_load($submission->uid)->mail;
                $query = new EntityFieldQuery();
                $results = $query->entityCondition('entity_type', 'TripalEntity')
                  ->entityCondition('bundle', $contact_bundle->name)
                  ->fieldCondition('local__email', 'value', $owner_mail)
                  ->range(0, 1)
                  ->execute();
                $entity = current(array_reverse(entity_load('TripalEntity', array_keys($results['TripalEntity']))));
                $owner = "<a href=\"$base_url/TripalContactProfile/{$entity->id}\">$submitting_user</a>";
              }
              else {
                $owner_mail = user_load($submission->uid)->mail;
                if ($owner_mail != $owner) {
                  $owner = "$submitting_user ($owner_mail)";
                }
              }
              $row = array(
                l($state['accession'], "$base_url/tpps-admin-panel/{$state['accession']}"),
                date("F j, Y", $state['loaded']) . " (" . round($days_since_load) . " days ago)",
                $pub_status,
                $owner,
              );
              if (tpps_access('view own tpps submission', $state['accession'])) {
                $row[] = l(t('Edit publication information'), "tpps/{$state['accession']}/edit-publication");
              }
              $unpublished_old[(int) substr($state['accession'], 4)] = $row;
            }
          }
        case 'Submission Job Running':
          $status_label = $status_label ?? (!empty($state['approved']) ? ("Submission Job Running - job started on " . date("F j, Y, \a\t g:i a", $state['approved'])) : "Submission Job Running");
        case 'Approved - Delayed Submission Release':
          if (empty($status_label)) {
            $release = $state['saved_values']['summarypage']['release-date'] ?? NULL;
            $release = strtotime("{$release['day']}-{$release['month']}-{$release['year']}");
            $status_label = "Approved - Delayed Submission Release on " . date("F j, Y", $release);
          }
          $row = array(
            l($state['accession'], "$base_url/tpps-admin-panel/{$state['accession']}"),
            $submitting_user,
            $state['saved_values'][TPPS_PAGE_1]['publication']['title'],
            $status_label,
            tpps_show_tags(tpps_submission_get_tags($state['accession'])),
          );
          $approved[(int) substr($state['accession'], 4)] = $row;
          break;

        default:
          switch ($state['stage']) {
            case TPPS_PAGE_1:
              $stage = "Author and Species Information";
              break;

            case TPPS_PAGE_2:
              $stage = "Experimental Conditions";
              break;

            case TPPS_PAGE_3:
              $stage = "Plant Accession";
              break;

            case TPPS_PAGE_4:
              $stage = "Submit Data";
              break;

            case 'summarypage':
              $stage = "Review Data and Submit";
              break;

            default:
              $stage = "Unknown";
              break;
          }

          $row = array(
            l($state['accession'], "$base_url/tpps-admin-panel/{$state['accession']}"),
            $submitting_user,
            $state['saved_values'][TPPS_PAGE_1]['publication']['title'] ?? 'Title not provided yet',
            $stage,
            !empty($state['updated']) ? date("F j, Y, g:i a", $state['updated']) : "Unknown",
            tpps_show_tags(tpps_submission_get_tags($state['accession'])),
          );
          $incomplete[(int) substr($state['accession'], 4)] = $row;
          break;
      }
    }
  }

  krsort($pending);
  krsort($approved);
  krsort($incomplete);

  $vars = array(
    'attributes' => array(
      'class' => array('view', 'tpps_table'),
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );

  $vars['header'] = array(
    'Accession Number',
    'Approval date',
    'Publication Status',
    'Submission Owner',
  );
  $vars['rows'] = $unpublished_old;
  $unpublished_table = theme('table', $vars);

  $vars['header'] = array(
    'Accession Number',
    'Submitting User',
    'Title',
    'Date Submitted',
    'Tags',
  );
  $vars['rows'] = $pending;
  $pending_table = theme('table', $vars);

  $vars['header'] = array(
    'Accession Number',
    'Submitting User',
    'Title',
    'Status',
    'Tags',
  );
  $vars['rows'] = $approved;
  $approved_table = theme('table', $vars);

  $vars['header'] = array(
    'Accession Number',
    'Submitting User',
    'Title',
    'Stage',
    'Last Updated',
    'Tags',
  );
  $vars['rows'] = $incomplete;
  $incomplete_table = theme('table', $vars);

  if (!empty($unpublished_old)) {
    $form['unpublished_old'] = array(
      '#type' => 'fieldset',
      '#title' => t('Unpublished Approved TPPS Submissions'),
      '#collapsible' => TRUE,
      'table' => array(
        '#markup' => $unpublished_table,
      ),
    );
  }

  if (!empty($pending)) {
    $form['pending'] = array(
      '#type' => 'fieldset',
      '#title' => t('Pending TPPS submissions'),
      '#collapsible' => TRUE,
      'table' => array(
        '#markup' => $pending_table,
      ),
    );
  }

  if (!empty($approved)) {
    $form['approved'] = array(
      '#type' => 'fieldset',
      '#title' => t('Approved TPPS submissions'),
      '#collapsible' => TRUE,
      'table' => array(
        '#markup' => $approved_table,
      ),
    );
  }

  if (!empty($incomplete)) {
    $form['incomplete'] = array(
      '#type' => 'fieldset',
      '#title' => t('Incomplete TPPS submissions'),
      '#collapsible' => TRUE,
      'table' => array(
        '#markup' => $incomplete_table,
      ),
    );
  }

  $subquery = db_select('tpps_submission', 's');
  $subquery->fields('s', array('accession'));
  $query = db_select('chado.dbxref', 'dbx');
  $query->join('chado.project_dbxref', 'pd', 'pd.dbxref_id = dbx.dbxref_id');
  $query->condition('dbx.accession', $subquery, 'NOT IN');
  $query->condition('dbx.accession', 'TGDR%', 'ILIKE');
  $query->fields('dbx', array('accession'));
  $query->orderBy('dbx.accession');
  $query = $query->execute();
  $to_resubmit = array();
  while (($result = $query->fetchObject())) {
    $to_resubmit[] = array($result->accession);
  }
  if (!empty($to_resubmit)) {
    $vars['header'] = array(
      'Accession',
    );
    $vars['rows'] = $to_resubmit;
    $to_resubmit_table = theme('table', $vars);
    $form['resubmit'] = array(
      '#type' => 'fieldset',
      '#title' => "<img src='$base_url/misc/message-16-warning.png'> " . t('Old TGDR Submissions to be resubmitted'),
      '#collapsible' => TRUE,
      'table' => array(
        '#markup' => $to_resubmit_table,
      ),
    );
  }

  $tpps_new_orgs = variable_get('tpps_new_organisms', NULL);
  $db = chado_get_db(array('name' => 'NCBI Taxonomy'));
  if (!empty($db)) {
    $rows = array();
    $query = db_select('chado.organism', 'o');
    $query->fields('o', array('organism_id', 'genus', 'species'));

    $query_e = db_select('chado.organism_dbxref', 'odb');
    $query_e->join('chado.dbxref', 'd', 'd.dbxref_id = odb.dbxref_id');
    $query_e->condition('d.db_id', $db->db_id)
      ->where('odb.organism_id = o.organism_id');
    $query->notExists($query_e);
    $query = $query->execute();

    $org_bundle = tripal_load_bundle_entity(array('label' => 'Organism'));
    while (($org = $query->fetchObject())) {
      $id = chado_get_record_entity_by_bundle($org_bundle, $org->organism_id);
      if (!empty($id)) {
        $rows[] = array(
          "<a href=\"$base_url/bio_data/{$id}/edit\" target=\"_blank\">$org->genus $org->species</a>",
        );
        continue;
      }
      $rows[] = array(
        "$org->genus $org->species",
      );
    }

    if (!empty($rows)) {
      $headers = array();

      $vars = array(
        'header' => $headers,
        'rows' => $rows,
        'attributes' => array(
          'class' => array('view', 'tpps_table'),
          'id' => 'new_species',
        ),
        'caption' => '',
        'colgroups' => NULL,
        'sticky' => FALSE,
        'empty' => '',
      );

      $form['new_species']['#markup'] = "<div class='tpps_table'><label for='new_species'>New Species: the species listed below likely need to be updated, because they do not have NCBI Taxonomy identifiers in the database.</label>" . theme('table', $vars) . "</div>";
    }
    variable_set('tpps_new_organisms', $tpps_new_orgs);
  }
}

/**
 * Implements hook_form_validate().
 *
 * Checks that the reject reason has been filled out if the submission was
 * rejected.
 */
function tpps_admin_panel_validate($form, &$form_state) {
  if ($form_state['submitted'] == '1') {
    if (isset($form_state['values']['reject-reason']) and $form_state['values']['reject-reason'] == '' and $form_state['triggering_element']['#value'] == 'Reject') {
      form_set_error('reject-reason', t('Please explain why the submission was rejected.'));
    }

    if ($form_state['triggering_element']['#value'] == 'Approve') {
      $accession = $form_state['values']['accession'];
      $state = tpps_load_submission($accession);
      foreach ($state['file_info'] as $files) {
        foreach ($files as $fid => $file_type) {
          if (!empty($form_state['values']["edit_file_{$fid}_check"]) and empty($form_state['values']["edit_file_{$fid}_file"])) {
            form_set_error("edit_file_{$fid}_file", t('Please upload a revised version fo the user-provided file.'));
          }
          if (!empty($form_state['values']["edit_file_{$fid}_file"])) {
            $file = file_load($form_state['values']["edit_file_{$fid}_file"]);
            file_usage_add($file, 'tpps', 'tpps_project', substr($accession, 4));
          }
        }
      }

      if (!empty($form_state['values']['study_location'])) {
        for ($i = 1; $i <= $form_state['values']['study_location']['locations']['number']; $i++) {
          if (empty($form_state['values']['study_location']['locations'][$i])) {
            form_set_error("study_location][locations][$i", "Study location $i: field is required.");
          }
        }
      }
    }

    if ($form_state['triggering_element']['#value'] == 'Save Alternative Accessions') {
      $alt_acc = explode(',', $form_state['values']['alternative_accessions']);
      foreach ($alt_acc as $acc) {
        if (!preg_match('/^TGDR\d{3,}$/', $acc)) {
          form_set_error('alternative_accessions', "The accession, $acc is not a valid TGDR### accession number.");
          continue;
        }
        $result = db_select('tpps_submission', 's')
          ->fields('s')
          ->condition('accession', $acc)
          ->range(0, 1)
          ->execute()->fetchObject();
        if (!empty($result)) {
          form_set_error('alternative_accessions', "The accession, $acc is already in use.");
        }
      }
    }

    if ($form_state['triggering_element']['#value'] == 'Change Submission Owner') {
      $new_user = user_load_by_mail($form_state['values']['change_owner']);
      if (empty($new_user)) {
        form_set_error('change_owner', t('Invalid user account'));
      }
    }

    drupal_add_js(drupal_get_path('module', 'tpps') . TPPS_JS_PATH);
    drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);
  }
}

/**
 * Implements hook_form_submit().
 *
 * Either rejects or approves the TPPS submission, and notifies the user of the
 * status update via email. If the submission was approved, starts a tripal job
 * for file parsing.
 */
function tpps_admin_panel_submit($form, &$form_state) {

  global $base_url;
  $type = $form_state['tpps_type'] ?? 'tpps';
  $type_label = ($type == 'tpps') ? 'TPPS' : 'TPPSC';

  $accession = $form_state['values']['accession'];
  $submission = tpps_load_submission($accession, FALSE);
  $owner = user_load($submission->uid);
  $to = $owner->mail;
  $state = unserialize($submission->submission_state);
  $state['admin_comments'] = $form_state['values']['admin-comments'] ?? NULL;
  $params = array();

  $from = variable_get('site_mail', '');
  $params['subject'] = "$type_label Submission Rejected: {$state['saved_values'][TPPS_PAGE_1]['publication']['title']}";
  $params['uid'] = $owner->uid;
  $params['reject-reason'] = $form_state['values']['reject-reason'] ?? NULL;
  $params['base_url'] = $base_url;
  $params['title'] = $state['saved_values'][TPPS_PAGE_1]['publication']['title'];
  $params['body'] = '';

  $params['headers'][] = 'MIME-Version: 1.0';
  $params['headers'][] = 'Content-type: text/html; charset=iso-8859-1';

  if (isset($form_state['values']['params'])) {
    foreach ($form_state['values']['params'] as $param_id => $type) {
      variable_set("tpps_param_{$param_id}_type", $type);
    }
  }

  switch ($form_state['triggering_element']['#value']) {
    case 'Save VCF Import Setting':
      // dpm($form_state['values']);
      if($form_state['values']['DISABLE_VCF_IMPORT'] == 1) {
        $state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 1;
      }
      else {
        $state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 0;
      }
      tpps_update_submission($state);
      break;

    case 'Reject':
      drupal_mail($type, 'user_rejected', $to, user_preferred_language($owner), $params, $from, TRUE);
      $state['status'] = 'Incomplete';
      tpps_update_submission($state);
      drupal_set_message(t('Submission Rejected. Message has been sent to user.'), 'status');
      drupal_goto('<front>');
      break;

    case 'Approve':
      module_load_include('php', 'tpps', 'forms/submit/submit_all');
      global $user;
      $uid = $user->uid;
      $state['submitting_uid'] = $uid;

      $params['subject'] = "$type_label Submission Approved: {$state['saved_values'][TPPS_PAGE_1]['publication']['title']}";
      $params['accession'] = $state['accession'];
      drupal_set_message(t('Submission Approved! Message has been sent to user.'), 'status');
      drupal_mail($type, 'user_approved', $to, user_preferred_language(user_load_by_name($to)), $params, $from, TRUE);

      $state['revised_files'] = $state['revised_files'] ?? array();
      foreach ($state['file_info'] as $files) {
        foreach ($files as $fid => $file_type) {
          if (!empty($form_state['values']["edit_file_{$fid}_check"])) {
            $state['revised_files'][$fid] = $form_state['values']["edit_file_{$fid}_file"];
          }
        }
      }

      if (!empty($form_state['values']['phenotypes_edit'])) {
        $state['phenotypes_edit'] = $form_state['values']['phenotypes_edit'];
      }

      if (!empty($form_state['values']['study_location'])) {
        $state['saved_values'][TPPS_PAGE_3]['study_location']['type'] = $form_state['values']['study_location']['type'];
        for ($i = 1; $i <= $form_state['values']['study_location']['locations']['number']; $i++) {
          $state['saved_values'][TPPS_PAGE_3]['study_location']['locations'][$i] = $form_state['values']['study_location']['locations'][$i];
        }
      }

      $includes = array();
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = array($accession);
      if ($state['saved_values']['summarypage']['release']) {
        $jid = tripal_add_job("$type_label Record Submission - $accession", 'tpps', 'tpps_submit_all', $args, $state['submitting_uid'], 10, $includes, TRUE);
        $state['job_id'] = $jid;
      }
      else {
        $date = $state['saved_values']['summarypage']['release-date'];
        $time = strtotime("{$date['year']}-{$date['month']}-{$date['day']}");
        if (time() > $time) {
          $jid = tripal_add_job("$type_label Record Submission - $accession", 'tpps', 'tpps_submit_all', $args, $state['submitting_uid'], 10, $includes, TRUE);
          $state['job_id'] = $jid;
        }
        else {
          $delayed_submissions = variable_get('tpps_delayed_submissions', array());
          $delayed_submissions[$accession] = $accession;
          variable_set('tpps_delayed_submissions', $delayed_submissions);
          $state['status'] = 'Approved - Delayed Submission Release';
        }
      }
      tpps_update_submission($state);
      break;

    case 'Change Date':
      $state['saved_values']['summarypage']['release-date'] = $form_state['values']['date'];
      tpps_update_submission($state);
      break;

    case 'Change Status':
      $state['status'] = $form_state['values']['state-status'];
      tpps_update_submission($state);
      break;

    case 'Save Alternative Accessions':
      $old_alt_acc = $state['alternative_accessions'] ?? '';
      $new_alt_acc = $form_state['values']['alternative_accessions'];
      if ($old_alt_acc != $new_alt_acc) {
        tpps_submission_add_alternative_accession($state, explode(',', $new_alt_acc));

        $state['alternative_accessions'] = $new_alt_acc;
        tpps_update_submission($state);
      }
      break;

    case 'Change Submission Owner':
      $new_user = user_load_by_mail($form_state['values']['change_owner']);
      $state['submitting_uid'] = $new_user->uid;
      tpps_update_submission($state, array(
        'uid' => $new_user->uid,
      ));
      break;

    default:
      break;
  }
}
