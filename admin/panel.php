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

  // [VS].
  $module_path = drupal_get_path('module', 'tpps');
  $form['#attached']['js'][] = $module_path . TPPS_JS_PATH;
  $form['#attached']['css'][] = $module_path . TPPS_CSS_PATH;
  // [/VS].
  return $form;
}

/**
 * Generates all materialized views.
 */
function tpps_manage_generate_all_materialized_views(array $form, array &$form_state, $option = NULL) {
  global $user;

  // [VS].
  $module_path = drupal_get_path('module', 'tpps');
  $form['#attached']['js'][] = $module_path . TPPS_JS_PATH;
  $form['#attached']['css'][] = $module_path . TPPS_CSS_PATH;
  // [/VS].
  module_load_include('php', 'tpps', 'forms/submit/submit_all');

  $includes = array();
  $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
  $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
  $args = array();
  $jid = tripal_add_job("Generate materialized views for all studies", 'tpps', 'tpps_generate_all_genotype_materialized_views', $args, $user->uid, 10, $includes, TRUE);

  $markup = "";
  $markup = '<div>A job has been created to (re)generate materialized views for all studies</div>';
  $form['item1'] = array(
    '#type' => 'markup',
    '#markup' => $markup
  );

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
  // dpm($submission);
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

  $form['file_diagnostics'] = array(
    '#markup' => '<a class="btn btn-primary" target="_blank" href="file-diagnostics/' . $accession . '">Files diagnostics</a><br /><br />',
  );

  $submission_tags = tpps_submission_get_tags($submission_state['accession']);
  // dpm($submission_tags);

  $tags_markup .= "<div style='margin-bottom: 10px; font-weight: bold; text-decoration: underline;'><a target=\"_blank\" href=\"/tpps-tag\">Manage Global TPPS Submission Tags</a></div>";

  // Show current tags.
  $tags_markup .= "<label class=\"control-label\">Current Tags:</label><br>";
  $image_path = drupal_get_path('module', 'tpps') . '/images/';
  $query = db_select('tpps_tag', 't')
    ->fields('t')
    ->execute();
  while (($result = $query->fetchObject())) {
    $color = !empty($result->color) ? $result->color : 'white';
    $style = !array_key_exists($result->tpps_tag_id, $submission_tags) ? "display: none" : "";
    // $tooltip = $result->static ? "This tag cannot be removed" : "";
    $tooltip = '';
    $tags_markup .= "<span title=\"$tooltip\" class=\"tag\" style=\"background-color:$color; $style\"><span class=\"tag-text\">{$result->name}</span>";
    if (!$result->static) {
      $tags_markup .= "<span id=\"{$submission_state['accession']}-tag-{$result->tpps_tag_id}-remove\" class=\"tag-close\"><img src=\"/{$image_path}remove.png\"></span>";
    }
    $tags_markup .= "</span>";
  }

  // Show available tags.
  // REMOVED ON 3/8/2023 by Risharde (code does not work)
  // $tags_markup .= "<br><label class=\"control-label\">Available Tags (click to add):</label><br><div id=\"available-tags\">";
  // $query = db_select('tpps_tag', 't')
  //   ->fields('t')
  //   ->condition('static', 0)
  //   ->execute();
  // while (($result = $query->fetchObject())) {
  //   $color = $result->color;
  //   if (empty($color)) {
  //     $color = 'white';
  //   }
  //   $style = "";
  //   if (array_key_exists($result->tpps_tag_id, $submission_tags)) {
  //     $style = 'display: none';
  //   }
  //   $tags_markup .= "<span id=\"{$submission_state['accession']}-tag-{$result->tpps_tag_id}-add\" class=\"tag add-tag\" style=\"background-color:{$color}; $style\"><span class=\"tag-text\">{$result->name}</span></span>";
  // }
  // $tags_markup .= "</div>";
  // $tags_markup .= "<div style='margin-top: 10px;'><a href=\"/tpps-tag\">Manage Global TPPS Submission Tags</a></div>";
  $form['tags'] = array(
    '#markup' => "<div id=\"tags\">$tags_markup</div>",
  );



  $form['TAG_REMOVE_CONTAINER'] = array(
    '#prefix' => '<div class="tag-admin-container" style="display: inline-block; vertical-align: top; text-align: left; padding: 25px;">',
    '#suffix' => '</div>',
  );

  $submission_tags_ids = [];
  foreach ($submission_tags as $submission_tag) {
    $submission_tags_ids = $submission_tag['id'];
  }

  // This code will generate the tag options that we can delete
  $current_tags_options = [];
  $current_tags_results = chado_query('SELECT * FROM tpps_submission_tag tsg
    LEFT JOIN tpps_tag tg ON (tsg.tpps_tag_id = tg.tpps_tag_id)
    WHERE tpps_submission_id = :tpps_submission_id
    AND tsg.tpps_tag_id > 2', [':tpps_submission_id' => $submission->tpps_submission_id]);
  foreach ($current_tags_results as $row) {
    $current_tags_options[$row->tpps_tag_id] = $row->name;
  }


  $form['TAG_REMOVE_CONTAINER']['TAG_REMOVE_OPTION'] = array(
    '#type' => 'select',
    '#title' => 'Remove the following selected tag',
    '#description' => 'This will delete a tag that has been already <br />added to this study',
    '#options' => $current_tags_options,
    '#default_value' => '',
  );
  $form['TAG_REMOVE_CONTAINER']['TAG_REMOVE_OPTION_DO'] = array(
    '#type' => 'submit',
    '#value' => t('Remove tag from this study'),
    '#suffix' => '<div style="margin-bottom: 30px;"></div>'
  );

  $form['TAG_ADD_CONTAINER'] = array(
    '#prefix' => '<div class="tag-admin-container" style="display: inline-block; vertical-align: top; text-align: left; padding: 25px;">',
    '#suffix' => '</div>',
  );

  // This code will generate all tag options that we can add
  $all_add_tags_options = [];
  $all_add_tags_results = chado_query('SELECT * FROM tpps_tag
    WHERE tpps_tag_id NOT IN (SELECT tpps_tag_id FROM tpps_submission_tag
      WHERE tpps_submission_id = :tpps_submission_id) AND tpps_tag_id > 2', [
        ':tpps_submission_id' => $submission->tpps_submission_id
      ]);
  foreach ($all_add_tags_results as $row) {
    $all_add_tags_options[$row->tpps_tag_id] = $row->name;
  }

  $form['TAG_ADD_CONTAINER']['TAG_ADD_OPTION'] = array(
    '#type' => 'select',
    '#title' => 'Add the following selected tag',
    // '#prefix' => '<div style="display: inline-block; width: 45%;">',
    // '#suffix' => '</div>',
    '#description' => 'This will add a tag that isn\'t already <br />added to this study',
    '#options' => $all_add_tags_options,
    '#default_value' => '',
  );
  $form['TAG_ADD_CONTAINER']['TAG_ADD_OPTION_DO'] = array(
    '#type' => 'submit',
    '#value' => t('Add tag to this study'),
    '#suffix' => '<div style="margin-bottom: 30px;"></div>'
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

  $disable_vcf_import = 1;
  if(!isset($submission_state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'])) {
    $disable_vcf_import = 0;
  }
  else {
    $disable_vcf_import = $submission_state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'];
  }

  $form['DISABLE_VCF_IMPORT'] = array(
    '#type' => 'checkbox',
    '#prefix' => '<h2 style="margin-top: 30px;">Disable VCF import</h2>',
    '#title' => 'Disable VCF Import in Tripal Job Submission',
    '#default_value' => $disable_vcf_import,
  );

  $form['DISABLE_VCF_IMPORT_SAVE'] = array(
    '#type' => 'submit',
    '#value' => t('Save VCF Import Setting'),
    '#suffix' => '<div style="margin-bottom: 30px;"></div>'
  );

  $options = [];
  $options['hybrid'] = 'hybrid';
  $options['inserts'] = 'inserts';
  $form['VCF_IMPORT_MODE'] = array(
    '#type' => 'select',
    '#title' => 'VCF Import mode',
    '#prefix' => '<h2 style="margin-top: 30px;">VCF Import Mode</h2>',
    '#description' => 'Hybrid mode is the new method using the COPY command for some of the import.
      This requires the database user to have SUPERUSER rights. Inserts mode is the original
      code that used inserts only (which is much slower but tested and works for most cases).',
    '#options' => $options,
    '#default_value' => 'hybrid',
  );

  $form['VCF_IMPORT_MODE_SAVE'] = array(
    '#type' => 'submit',
    '#value' => t('Save VCF Import Mode'),
    '#suffix' => '<div style="margin-bottom: 30px;"></div>'
  );

  $submitting_user = user_load($submission_state['submitting_uid']);
  $form['change_owner'] = array(
    '#type' => 'textfield',
    '#prefix' => '<h2 style="margin-top: 30px;">Change owner</h2>',
    '#title' => t('Choose a new owner for the submission'),
    '#default_value' => $submitting_user->mail,
    '#autocomplete_path' => 'tpps/autocomplete/user',
  );

  $form['CHANGE_OWNER'] = array(
    '#type' => 'submit',
    '#value' => t('Change Submission Owner'),
  );

  $form['SYNC_PUBLICATION_DATA'] = array(
    '#title' => 'Synchronize publication data',
    '#prefix' => '<h2 style="margin-top: 30px;">Synchronize publication data</h2>This will attempt to pull publication data from the publication content type and update the study info<br />',
    '#type' => 'submit',
    '#value' => t('Sync Publication'),
  );

  $study_role_view = NULL;
  if(isset($submission_state['study_view_role'])) {
    $study_role_view = $submission_state['study_view_role'];
  }
  else {
    $study_role_view = 0;
  }

  $options = user_roles(true);
  $options[0] = 'All users';
  $form['CHANGE_STUDY_VIEW_ROLE'] = array(
    '#type' => 'select',
    '#title' => 'Set this study view role',
    '#prefix' => '<h2 style="margin-top: 30px;">Change study role view</h2>',
    '#description' => 'This will change the user role that is allowed to view this study',
    '#options' => $options,
    '#default_value' => $study_role_view,
  );

  $form['CHANGE_STUDY_VIEW_ROLE_SAVE'] = array(
    '#type' => 'submit',
    '#value' => t('Change Study View Role'),
  );

  $current_tpps_type = '';
  if($submission_state['tpps_type'] == 'tppsc') {
    $current_tpps_type = 'tppsc';
  }
  else {
    $current_tpps_type = 'tpps';
  }
  $form['CHANGE_TPPS_TYPE'] = array(
    '#type' => 'select',
    '#title' => 'Change this study\'s TPPS type',
    '#prefix' => '<h2 style="margin-top: 30px;">Change TPPS type</h2>',
    '#description' => 'This will change the submission state type and also the submission tag to the type you select',
    '#options' => array(
      'tppsc' => t('TPPSc'),
      'tpps' => t('TPPS'),
    ),
    '#default_value' => $current_tpps_type,
  );

  $form['CHANGE_TPPS_TYPE_SAVE'] = array(
    '#type' => 'submit',
    '#value' => t('Change TPPS Type'),
  );

  $form['state-status'] = array(
    '#type' => 'select',
    '#prefix' => '<h2 style="margin-top: 30px;">Change state</h2>',
    '#title' => t('Change state status'),
    '#description' => t('Warning: This feature is experimental and may cause unforseen issues. Please do not change the status of this submission unless you are willing to risk the loss of existing data. The current status of the submission is @status.', array('@status' => $status)),
    '#options' => array(
      'Incomplete' => t('Incomplete'),
      'Pending Approval' => t('Pending Approval'),
      'Approved' => t('Approved'),
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

  // GENERATE_POPSTRUCT_FROM_VCF
  $form['GENERATE_POPSTRUCT_FROM_VCF'] = array(
    '#type' => 'submit',
    '#prefix' => '<h2 style="margin-top: 30px;">Generate PopStruct FROM VCF</h2>',
    '#value' => t("Generate PopStruct FROM VCF"),
  );

  // REFRESH TPPS CVTERMS CACHE
  $form['REFRESH_TPPS_CVTERMS_CACHE'] = array(
    '#type' => 'submit',
    '#prefix' => '<h2 style="margin-top: 30px;">Refresh TPPS CVTERMS CACHE</h2>',
    '#value' => t("Refresh TPPS cvterms cache"),
  );

  $form['REGENERATE_GENOTYPE_MATERIALIZED_VIEW'] = array(
    '#type' => 'submit',
    '#prefix' => '<h2 style="margin-top: 30px;">REGENERATE GENOTYPE MATERIALIZED VIEW</h2>This regenerates the genotype view for the tpps details page.<br />',
    '#value' => t("Regenerate genotype materialized view"),
  );

  // Remove this study's markers and genotypes
  $form['REMOVE_STUDY_MARKERS_GENOTYPES'] = array(
    '#type' => 'submit',
    '#prefix' => '<h2 style="margin-top: 30px;">Clear markers and genotypes</h2>Warning: This will clear all markers and genotypes for this study. You will need to resubmit the study to import back this data.',
    '#value' => t("Remove this study's markers and genotypes"),
  );


  $form['CHANGE_TGDR_NUMBER'] = array(
    '#type' => 'textfield',
    '#prefix' => '<h2 style="margin-top: 30px;">Change TGDR number</h2>Warning: This will clear all data from the database and reimport as a new study.',
    '#title' => t('Specify the new TGDR number only (do not include TGDR)'),
    '#default_value' => '',
  );


  $form['CHANGE_TGDR_NUMBER_SUBMIT'] = array(
    '#type' => 'submit',
    '#value' => t("Change TGDR number"),
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

  // [VS]
  tpps_admin_panel_reports($form);
  // [/VS].
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


  $form['general_tasks'] = array(
    '#type' => 'fieldset',
    '#title' => t('General tasks'),
    '#collapsible' => TRUE,
  );

  $markup_genotype_views = '';
  $markup_genotype_views .= '<a target="_blank" href="/tpps-admin-panel/refresh-genotypes-materialized-views">Refresh all genotype
    materialized views</a><br />';
  $markup_genotype_views .= '<br />';
  $form['general_tasks']['genotype_views'] = array(
    '#type' => 'markup',
    '#markup' => $markup_genotype_views
  );


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

  // dpm($form_state['triggering_element']['#value']);
  switch ($form_state['triggering_element']['#value']) {
    case 'Add tag to this study':
      $tpps_tag_id = $form_state['values']['TAG_ADD_OPTION'];
      $tpps_submission_id = $submission->tpps_submission_id;
      // Insert tag for this tpps_submission_id
      chado_query('INSERT INTO tpps_submission_tag
        (tpps_submission_id, tpps_tag_id)
        VALUES (:tpps_submission_id, :tpps_tag_id)',
        [
          ':tpps_submission_id' => $tpps_submission_id,
          ':tpps_tag_id' => $tpps_tag_id
        ]
      );
      // Get the tag name for the message alert
      $tag_name = "";
      $tag_name_results = chado_query('SELECT * FROM tpps_tag
        WHERE tpps_tag_id = :tpps_tag_id', [
          ':tpps_tag_id' => $tpps_tag_id
        ]
      );
      foreach ($tag_name_results as $row) {
        $tag_name = $row->name;
      }
      drupal_set_message($tag_name . " has been added to the study");
      break;
    case 'Remove tag from this study':
      $tpps_tag_id = $form_state['values']['TAG_REMOVE_OPTION'];
      $tpps_submission_id = $submission->tpps_submission_id;
      // Get the tag name for the message alert
      $tag_name = "";
      $tag_name_results = chado_query('SELECT * FROM tpps_tag
        WHERE tpps_tag_id = :tpps_tag_id', [
          ':tpps_tag_id' => $tpps_tag_id
        ]
      );
      foreach ($tag_name_results as $row) {
        $tag_name = $row->name;
      }
      if ($tpps_tag_id > 2) {
        chado_query('DELETE FROM tpps_submission_tag
          WHERE tpps_submission_id = :tpps_submission_id
          AND tpps_tag_id = :tpps_tag_id', [
            ':tpps_submission_id' => $tpps_submission_id,
            ':tpps_tag_id' => $tpps_tag_id
          ]
        );
        drupal_set_message($tag_name . " successfully removed from study.");
      }
      else {
        drupal_set_message($tag_name . " cannot be removed from study.","error");
      }
      break;
    case 'Save VCF Import Setting':
      // dpm($form_state['values']);
      if($form_state['values']['DISABLE_VCF_IMPORT'] == 1) {
        $state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 1;
      }
      else {
        $state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 0;
      }
      tpps_update_submission($state);
      drupal_set_message(t('VCF disable import setting saved'), 'status');
      break;
    case 'Save VCF Import Mode':
      $vcf_import_mode = $form_state['values']['VCF_IMPORT_MODE'];
      if($form_state['values']['VCF_IMPORT_MODE'] == 'hybrid') {
        $state['saved_values'][TPPS_PAGE_1]['vcf_import_mode'] = 'hybrid';
      }
      else if($form_state['values']['VCF_IMPORT_MODE'] == 'inserts') {
        $state['saved_values'][TPPS_PAGE_1]['vcf_import_mode'] = 'inserts';
      }
      else {
        $state['saved_values'][TPPS_PAGE_1]['vcf_import_mode'] = 'hybrid';
      }
      tpps_update_submission($state);
      drupal_set_message(t("VCF import mode saved as '$vcf_import_mode'."), 'status');
      break;

    case "Regenerate genotype materialized view":
      global $user;
      $project_id = $state['ids']['project_id'] ?? NULL;
      $includes = array();
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = array($project_id);
      $jid = tripal_add_job("Generate materialized view for $accession (project_id=$project_id)", 'tpps', 'tpps_generate_genotype_materialized_view', $args, $user->uid, 10, $includes, TRUE);
      break;

    case "Remove this study's markers and genotypes":
      global $user;
      $includes = array();
      # $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/markers_genotypes_utils');
      $args = array($accession);
      $jid = tripal_add_job("TPPS REMOVE all study markers and genotypes - $accession", 'tpps', 'tpps_remove_all_markers_genotypes', $args, $user->uid, 10, $includes, TRUE);
      // drupal_set_message(t('Tripal Job created to remove all study markers and genotypes from ' . $accession), 'status');
      break;

    case "Change TGDR number":
      // dpm($form_state['values']);

      // Check if a valid tgdr number was supplied
      if(!is_numeric($form_state['values']['CHANGE_TGDR_NUMBER'])) {
        drupal_set_message(t('You did not enter a valid number. Operation aborted.'));
        break;
      }

      $new_accession = 'TGDR' . $form_state['values']['CHANGE_TGDR_NUMBER'];
      // Check if the new tgdr number does not exist in the database
      // if it exists, abort the mission
      $results = chado_query('SELECT count(*) as c1 FROM public.tpps_submission WHERE accession = :new_accession', array(
        ':new_accession' => $new_accession
      ));
      $result_object = $results->fetchObject();
      // dpm($result_object);
      $result_count = $result_object->c1;
      if($result_count > 0) {
        drupal_set_message(t('It seems the TGDR number you wanted to change to is already in use. Operation aborted due to safety concerns.'));
        break;
      }

      // dpm($state); // this doesn't even load on the browser (too big!)

      global $user;
      $includes = array();
      # $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/markers_genotypes_utils');
      $includes[] = module_load_include('inc', 'tpps', 'includes/submissions');
      $args = [$accession];
      // Job to remove genotype information and features
      $jid = tripal_add_job("TPPS REMOVE all study markers and genotypes - $accession", 'tpps', 'tpps_remove_all_markers_genotypes', $args, $user->uid, 10, $includes, TRUE);

      // Job to change the TGDR number to the new TGDR number
      $args = [$accession, $new_accession];
      $jid = tripal_add_job("TPPS rename $accession to $new_accession", 'tpps', 'tpps_change_tgdr_number', $args, $user->uid, 10, $includes, TRUE);


      // Now run the new import for the new accession TGDR number
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = array($new_accession);
      $jid = tripal_add_job("TPPS Record Submission - $new_accession", 'tpps', 'tpps_submit_all', $args, $state['submitting_uid'], 10, $includes, TRUE);
      $state['job_id'] = $jid;
      tpps_update_submission($state);
      break;

    case "Generate PopStruct FROM VCF":
      global $user;
      $includes = array();
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');

      // dpm($state['saved_values'][4]['organism-1']['genotype']['files']['vcf']);
      if(!empty($state['saved_values'][4]['organism-1']['genotype']['files']['vcf'])) {
        $vcf_fid = $state['saved_values'][4]['organism-1']['genotype']['files']['vcf'];
        $vcf_file = file_load($vcf_fid);
        $location = tpps_get_location($vcf_file->uri);
        $args = array($accession,$location);
        // dpm($args);
        $jid = tripal_add_job("TPPS Generate PopStruct FROM VCF - $accession", 'tpps', 'tpps_generate_popstruct', $args, $user->uid, 10, $includes, TRUE);
      }
      else {
        drupal_set_message("Could not find a VCF tied to organism-1, are you sure you linked a VCF file?");
      }
      break;

    case "Refresh TPPS cvterms cache":
      global $user;
      $includes = array();
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/cvterm_utils');
      $args = array();
      $jid = tripal_add_job("TPPS REFRESH CVTERMS CACHE", 'tpps', 'tpps_cvterms_clear_cache', $args, $user->uid, 10, $includes, TRUE);
      // drupal_set_message(t('Tripal Job created to remove all study markers and genotypes from ' . $accession), 'status');
      break;

    case 'Change TPPS Type':
      // dpm($form_state['values']);

      // Get the tpps_submission_id from the public.tpps_submission table
      $results = chado_query('SELECT * FROM public.tpps_submission WHERE accession = :accession LIMIT 1', [
        ':accession' => $accession
      ]);
      $tpps_submission_id = NULL;
      foreach ($results as $row) {
        $tpps_submission_id = $row->tpps_submission_id;
      }
      // dpm('tpps_submission_id = ' . $tpps_submission_id);
      if ($tpps_submission_id == NULL) {
        drupal_set_message(t('Could not find a TPPS SUBMISSION ID for this accession, contact administration'), 'error');
        break;
      }

      if($form_state['values']['CHANGE_TPPS_TYPE'] == 'tppsc') {
        // $state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 1;

        // Set the state tpps_type to tppsc
        $state['tpps_type'] = 'tppsc';

        // Deprecated changing the user id since we adjusted TPPSc
        // to allow curators to see all studies (3/6/2023)
        // global $user;
        // tpps_update_submission($state, array(
        //   'uid' => $user->uid,
        // ));

        // Update the submission tag table which in term will get rippled
        // into the ct_trees_all_view materialized view that filters internal and external submissions
        chado_query('UPDATE public.tpps_submission_tag
          SET tpps_tag_id = 2
          WHERE tpps_submission_id = :tpps_submission_id
          AND (tpps_tag_id = 1 OR tpps_tag_id = 2)',
          [
            ':tpps_submission_id' => $tpps_submission_id
          ]
        );

      }
      else {
        // $state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 0;

        // Set the state tpps_type to tpps
        $state['tpps_type'] = 'tpps';

        // Update the submission tag table which in term will get rippled
        // into the ct_trees_all_view materialized view that filters internal and external submissions
        chado_query('UPDATE public.tpps_submission_tag
          SET tpps_tag_id = 1
          WHERE tpps_submission_id = :tpps_submission_id
          AND (tpps_tag_id = 1 OR tpps_tag_id = 2)',
          [
            ':tpps_submission_id' => $tpps_submission_id
          ]
        );
      }
      tpps_update_submission($state);
      drupal_set_message(t('Updated study TPPS type: ') . $state['tpps_type'], 'status');
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
      dpm($state['status']);
      break;

    case 'Change Study View Role':
      $view_role = $form_state['values']['CHANGE_STUDY_VIEW_ROLE'];
      if ($view_role > 0) {
        $state['study_view_role'] = $view_role;
        drupal_set_message('Study view role has been set to ' . user_roles(true)[$view_role]);
      }
      else {
        unset($state['study_view_role']);
        drupal_set_message('Study view role set to public all users');
      }
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

    case 'Sync Publication':
      // dpm('State title:');
      // dpm($state['title']);
      // dpm('Project ID:');
      $project_id = $state['ids']['project_id'] ?? NULL;
      // dpm($project_id);
      // dpm($state['publication']);
      // dpm(array_keys($state));
      // dpm($state['authors']);
      // dpm($state['pyear']);

      $pub_id = db_select('chado.project_pub', 'p')
      ->fields('p', array('pub_id'))
      ->condition('project_id', $state['ids']['project_id'])
      ->execute()->fetchObject()->pub_id;
      if (!empty($pub_id)) {
        $bundle = tripal_load_bundle_entity(array('label' => 'Publication'));


        $pub_entity_id = NULL;
        try {
          $pub_entity_id = chado_get_record_entity_by_bundle($bundle, $pub_id);
        }
        catch (Exception $ex) {
          // couldn't find a publication entity
        }

        if (!isset($pub_entity_id)) {
          drupal_set_message('Could not find a matching publication safely. Will not synchronize data.');
        }
        else {
          drupal_set_message("Publication entity id found ($pub_entity_id), retrieving publication data...");
          // dpm('pub_entity_id:'. $pub_entity_id);
          // This will return results as an array
          $publication_entity_results = tripal_load_entity('TripalEntity', array($pub_entity_id));
          // Get the entity
          $publication_entity = $publication_entity_results[$pub_entity_id];
          // dpm($publication_entity);
          // dpm($publication_entity->title);
          if (isset($publication_entity->title)) {
            $pub_title = $publication_entity->title;
            // dpm('pub_title:' . $pub_title);
            if ($pub_title != "") {
              drupal_set_message('Found a valid publication title, syncing with study.');
              $state['title'] = $pub_title;
            }
          }
          if (isset($publication_entity->tpub__year['und'][0]['safe_value'])) {
            $pub_year = $publication_entity->tpub__year['und'][0]['safe_value'];
            // dpm('pub_year:' . $pub_year);
            if ($pub_year != "") {
              drupal_set_message('Found a valid publication year, synced with study.');
              $state['pyear'] = $pub_year;
            }
          }
          if (isset($publication_entity->tpub__abstract['und'][0]['value'])) {
            $pub_abstract = $publication_entity->tpub__abstract['und'][0]['value'];
            // dpm('pub_abstract:' . $pub_abstract);
            if ($pub_abstract != "") {
              drupal_set_message('Found a valid publication abstract, synced with study.');
              $state['abstract'] = $pub_abstract;
            }
          }
          if (isset($publication_entity->tpub__authors['und'][0]['value'])) {
            $pub_authors = $publication_entity->tpub__authors['und'][0]['value'];
            // dpm('pub_authors:' . $pub_authors);
            if ($pub_authors != "") {
              preg_match_all('/.[^,]+,*/', $pub_authors, $matches);
              // dpm($matches);
              if (count($matches) > 0) {
                $actual_matches = $matches[0];
                // dpm($actual_matches);
                $filtered_matches = array();
                foreach ($actual_matches as $match) {
                  $match = str_replace(',','',$match);
                  $match = trim($match);
                  array_push($filtered_matches, $match);
                }
                // dpm($filtered_matches);
                if (count($filtered_matches) > 0) {
                  $state['authors'] = $filtered_matches;
                  drupal_set_message('Found valid publication authors, synced with study.');
                }
              }
            }
          }
          // Save the submission state
          tpps_update_submission($state);
          drupal_set_message('Done.');
        }
      }
      else {
        drupal_set_message('Could not find a valid pub_id for this study. Edit via TPPSc and make sure you have connected this study to a valid paper');
      }
      break;

    default:
      break;
  }
}

/**
 * Adds fieldset with links to custom report pages.
 */
function tpps_admin_panel_get_reports() {
  return [
    // Format: <Report Key> => <Path related to $panel_url>.
    'missing_doi' => 'tpps-admin-panel/reports/missing-doi',
  ];
}

/**
 * Adds fieldset with links to custom report pages.
 *
 * @param array $form
 *   The form element of the TPPS admin panel page.
 */
function tpps_admin_panel_reports(array &$form) {
  foreach (tpps_admin_panel_get_reports() as $report_key => $panel_url) {
    if ($title = variable_get('tpps_report_' . $report_key . '_title')) {
      $items[] = l(t($title), $panel_url);
    }
  }
  $form['report_menu'] = [
    '#type' => 'fieldset',
    '#title' => t('TPPS Reports'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    'table' => ['#markup' => theme('item_list', ['items' => $items])],
  ];
}
