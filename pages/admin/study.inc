<?php

/**
 * @file
 * Shows single study admin panel.
 */

module_load_include('inc', 'tpps', 'includes/common');

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
//function tpps_admin_panel(array $form, array &$form_state, $accession = NULL) {
//  if (empty($accession)) {
//    tpps_admin_panel_show_all($form);
//  }
//  else {
//    // Shows single study.
//    tpps_manage_submission_form($form, $form_state, $accession);
//  }
//  $form['#attributes']['class'][] = 'tpps-admin-panel';
//  tpps_add_css_js('main', $form);
//  return $form;
//}

/**
 * Build form to manage TPPS submissions from admin panel.
 *
 * Shows huge form which allows to manage submission.
 * Menu path: /tpps-admin-panel/TGDRxxxxx.
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
function tpps_manage_submission_form(array $form, array $form_state, $accession = NULL) {
  global $base_url;

  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    drupal_set_message(t('Study "@accession" not found.',
      ['@accession' => $accession]), 'error');
    drupal_goto('tpps-admin-panel');
  }

  $page1_values = &$submission->sharedState['saved_values'][TPPS_PAGE_1] ?? NULL;
  $page2_values = &$submission->sharedState['saved_values'][TPPS_PAGE_2] ?? NULL;
  $page3_values = &$submission->sharedState['saved_values'][TPPS_PAGE_3] ?? NULL;
  $page4_values = &$submission->sharedState['saved_values'][TPPS_PAGE_4] ?? NULL;

  if (empty($submission->sharedState['status'])) {
    $submission->save($submission->status);
  }
  $options = [];
  $display = l(t('Back to TPPS Admin Panel'), 'tpps-admin-panel');

  // Check for log file
  // Step 1 - Look for the last tripal job that has the accession.
  $results = db_query("SELECT * FROM public.tripal_jobs WHERE job_name LIKE 
    :search ORDER BY submit_date DESC LIMIT 1;", [
      ':search' => "%Record Submission - $accession%"
    ]);
  $job_id = -1;
  while ($row_array = $results->fetchObject()) {
    // $display .= print_r($row_array, true);
    $job_id = $row_array->job_id;
  }
  if ($job_id == -1) {
    $display .= "<div style='padding: 10px;'>No log file exists for this "
      . "study (resubmit this study to generate a log file if necessary)</div>";
  }
  else {
    $log_path = tpps_realpath('public://') . '/tpps_job_logs/';
    // dpm($log_path . $accession . "_" . $job_id . ".txt");
    if (file_exists($log_path . $accession . "_" . $job_id . ".txt")) {
      $display .= "<div style='padding: 10px;background: #e9f9ef;border: "
        . "1px solid #90bea9;font-size: 18px;'><a target='_blank' "
        . "href='../tpps-admin-panel-logs/" . $accession . "_" . $job_id
        . "'>Latest job log file ($accession - $job_id) - submit pipeline</a></div>";
    }
    else {
      $display .= "<div style='padding: 10px;'>Could not find job log file "
        . "(this can happen if the log file was deleted - resubmit study "
        . "if necessary to regenerate log file)</div>";
    }
  }

  // Check for log file
  // Step 1b - Look for the last tripal job for Flat File processing if exists.
  $results = db_query("SELECT * FROM public.tripal_jobs WHERE job_name LIKE 
    :search ORDER BY submit_date DESC LIMIT 1;", [
      ':search' => "%Generate VCF to SNPs Flat file - %$accession%"
    ]);
  $job_id = -1;
  while ($row_array = $results->fetchObject()) {
    // $display .= print_r($row_array, true);
    $job_id = $row_array->job_id;
  }
  if ($job_id == -1) {
    $display .= "<div style='padding: 10px;'>No log file exists for this "
      . "study (flat file processing)</div>";
  }
  else {
    $log_path = tpps_realpath('public://') . '/tpps_job_logs/';
    // dpm($log_path . $accession . "_" . $job_id . ".txt");
    if (file_exists($log_path . $accession . "_" . $job_id . ".txt")) {
      $display .= "<div style='padding: 10px;background: #e9f9ef;border: "
        . "1px solid #90bea9;font-size: 18px;'><a target='_blank' "
        . "href='../tpps-admin-panel-logs/" . $accession . "_" . $job_id
        . "'>Latest job log file ($accession - $job_id) - flat file processing</a></div>";
    }
    else {
      $display .= "<div style='padding: 10px;'>Could not find job log file "
        . "(this can happen if the log file was deleted - flat file processing "
        . "if necessary to regenerate log file)</div>";
    }
  } 

  // Check for log file
  // Step 1c - Look for the last tripal job for marker removal if exists.
  $results = db_query("SELECT * FROM public.tripal_jobs WHERE job_name LIKE 
    :search ORDER BY submit_date DESC LIMIT 1;", [
      ':search' => "%TPPS REMOVE all%$accession%"
    ]);
  $job_id = -1;
  while ($row_array = $results->fetchObject()) {
    // $display .= print_r($row_array, true);
    $job_id = $row_array->job_id;
  }
  if ($job_id == -1) {
    $display .= "<div style='padding: 10px;'>No log file exists for this "
      . "study (removing markers and genotypes)</div>";
  }
  else {
    $log_path = tpps_realpath('public://') . '/tpps_job_logs/';
    // dpm($log_path . $accession . "_" . $job_id . ".txt");
    if (file_exists($log_path . $accession . "_" . $job_id . ".txt")) {
      $display .= "<div style='padding: 10px;background: #e9f9ef;border: "
        . "1px solid #90bea9;font-size: 18px;'><a target='_blank' "
        . "href='../tpps-admin-panel-logs/" . $accession . "_" . $job_id
        . "'>Latest job log file ($accession - $job_id) - marker and genotype removals</a></div>";
    }
    else {
      $display .= "<div style='padding: 10px;'>Could not find job log file "
        . "(this can happen if the log file was deleted - marker and genotype removals "
        . "if necessary to regenerate log file)</div>";
    }
  } 




  if ($submission->status == TPPS_SUBMISSION_STATUS_PENDING_APPROVAL) {
    $options['files'] = [
      'revision_destination' => TRUE,
    ];
    $options['skip_phenotypes'] = TRUE;

    foreach ($submission->sharedState['file_info'] as $files) {
      foreach ($files as $fid => $file_type) {
        if ($file = tpps_file_load($fid)) {
          $form["edit_file_{$fid}_check"] = [
            '#type' => 'checkbox',
            '#title' => t('I would like to upload a revised version of this file'),
            '#prefix' => "<div id=\"file_{$fid}_options\">",
          ];
          $form["edit_file_{$fid}_file"] = [
            '#type' => 'managed_file',
            '#title' => 'Upload new file',
            '#upload_location' => dirname($file->uri),
            '#upload_validators' => ['file_validate_extensions' => []],
            '#states' => [
              'visible' => [
                ":input[name=\"edit_file_{$fid}_check\"]" => ['checked' => TRUE],
              ],
            ],
          ];
          $form["edit_file_{$fid}_markup"] = ['#markup' => '</div>'];
        }
      }
    }
  }
  $display .= tpps_table_display($submission->sharedState, $options);

  if (
    $submission->status == TPPS_SUBMISSION_STATUS_PENDING_APPROVAL
    && preg_match('/P/', $submission->sharedState['saved_values'][TPPS_PAGE_2]['data_type'])
  ) {
    $new_cvterms = array();
    $page4 = &$submission->sharedState['saved_values'][TPPS_PAGE_4];
    for ($i = 1; $i <= $submission->sharedState['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
      // @TODO Could we just skip when it's checked?
      $phenotype = (!empty($page4["organism-$i"]['phenotype-repeat-check']))
        ? $page4["organism-1"]['phenotype'] : $page4["organism-$i"]['phenotype'];
      for ($j = 1; $j <= $phenotype['phenotypes-meta']['number']; $j++) {
        if ($phenotype['phenotypes-meta'][$j]['structure'] === 'other') {
          $new_cvterms[] = $phenotype['phenotypes-meta'][$j]['struct-other'];
        }
        if ($phenotype['phenotypes-meta'][$j]['attribute'] === 'other') {
          $new_cvterms[] = $phenotype['phenotypes-meta'][$j]['attr-other'];
        }
        // [VS]
        // Custom Unit must be created by admin by hand.
        // [/VS]
      }
    }
    // @todo get new/custom units from metadata file.
    if (count($new_cvterms) > 0) {
      $message = 'This submission will create the following new local cvterms: '
        . implode(', ', array_unique($new_cvterms));
      $display .= "<div class=\"alert alert-block alert-dismissible alert-warning messages warning\">
        <a class=\"close\" data-dismiss=\"alert\" href=\"#\">×</a>
        <h4 class=\"element-invisible\">Warning message</h4>
        {$message}</div>";
    }
  }

  $form['accession'] = ['#type' => 'hidden', '#value' => $accession];
  $form['form_table'] = ['#markup' => $display];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // File Diagnostics.
  $form['file_diagnostics'] = [
    '#markup' => l(t('Files diagnostics'),
      'tpps-admin-panel/file-diagnostics/' . $accession,
      [
        'attributes' => [
          'class' => ['btn', 'btn-primary', 'form-submit'],
          'target' => '_blank',
        ],
      ]) . '<br /><br />',
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Tags.
  $submission_tags = tpps_submission_get_tags($submission->accession);
  $tags_markup = "<div style='margin-bottom: 10px; font-weight: bold; "
    . "text-decoration: underline;'><a target=\"_blank\" href=\"/tpps-tag\">"
    . "Manage Global TPPS Submission Tags</a></div>";
  // Show current tags.
  $tags_markup .= "<label class=\"control-label\">Current Tags:</label><br>";
  $image_path = TPPS_IMAGES_PATH;
  $query = db_select('tpps_tag', 't')
    ->fields('t')
    ->execute();
  while (($result = $query->fetchObject())) {
    $color = !empty($result->color) ? $result->color : 'white';
    $style = !array_key_exists($result->tpps_tag_id, $submission_tags) ? "display: none" : "";
    // $tooltip = $result->static ? "This tag cannot be removed" : "";
    $tooltip = '';
    $tags_markup .= "<span title=\"$tooltip\" class=\"tag\" "
      . "style=\"background-color:$color; $style\"><span "
      . "class=\"tag-text\">{$result->name}</span>";
    if (!$result->static) {
      $tags_markup .= "<span id=\"{$submission->accession}-tag-"
        . "{$result->tpps_tag_id}-remove\" "
        . "class=\"tag-close\"><img src=\"/{$image_path}remove.png\"></span>";
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
  //   $tags_markup .= "<span id=\"{$submission->accession}-tag-{$result->tpps_tag_id}-add\" class=\"tag add-tag\" style=\"background-color:{$color}; $style\"><span class=\"tag-text\">{$result->name}</span></span>";
  // }
  // $tags_markup .= "</div>";
  // $tags_markup .= "<div style='margin-top: 10px;'><a href=\"/tpps-tag\">Manage Global TPPS Submission Tags</a></div>";
  $form['tags'] = ['#markup' => "<div id=\"tags\">$tags_markup</div>"];

  $form['TAG_REMOVE_CONTAINER'] = [
    '#prefix' => '<div class="tag-admin-container" style="display: '
      . 'inline-block; vertical-align: top; text-align: left; padding: 25px;">',
    '#suffix' => '</div>',
  ];

  $submission_tags_ids = [];
  foreach ($submission_tags as $submission_tag) {
    // When study has no tags $submission_tags will be empty and cause warnings.
    // This happens for TPPS Studies which has no tags at all.
    if (isset($submission_tag['id'])) {
      $submission_tags_ids = $submission_tag['id'];
    }
  }

  // This code will generate the tag options that we can delete.
  $current_tags_options = [];
  $current_tags_results = chado_query('SELECT * FROM tpps_submission_tag tsg
    LEFT JOIN tpps_tag tg ON (tsg.tpps_tag_id = tg.tpps_tag_id)
    WHERE tpps_submission_id = :tpps_submission_id
    AND tsg.tpps_tag_id > 2',
    [':tpps_submission_id' => $submission->id]
  );
  foreach ($current_tags_results as $row) {
    $current_tags_options[$row->tpps_tag_id] = $row->name;
  }

  $form['TAG_REMOVE_CONTAINER']['TAG_REMOVE_OPTION'] = [
    '#type' => 'select',
    '#title' => t('Remove the following selected tag'),
    '#description' => t('This will delete a tag that has been already '
      . '<br />added to this study'),
    '#options' => $current_tags_options,
    '#attributes' => ['style' => 'width: 100%'],
    '#default_value' => '',
  ];
  $form['TAG_REMOVE_CONTAINER']['TAG_REMOVE_OPTION_DO'] = [
    '#type' => 'submit',
    '#name' => 'remove_tag',
    '#value' => t('Remove tag from this study'),
    '#suffix' => '<div style="margin-bottom: 30px;"></div>'
  ];

  $form['TAG_ADD_CONTAINER'] = [
    '#prefix' => '<div class="tag-admin-container" style="display: '
      . 'inline-block; vertical-align: top; text-align: left; padding: 25px;">',
    '#suffix' => '</div>',
  ];

  // This code will generate all tag options that we can add.
  $all_add_tags_options = [];
  $all_add_tags_results = chado_query('SELECT * FROM tpps_tag
    WHERE tpps_tag_id NOT IN (
      SELECT tpps_tag_id FROM tpps_submission_tag
      WHERE tpps_submission_id = :tpps_submission_id
    ) AND tpps_tag_id > 2',
    [':tpps_submission_id' => $submission->id]
  );
  foreach ($all_add_tags_results as $row) {
    $all_add_tags_options[$row->tpps_tag_id] = $row->name;
  }

  $form['TAG_ADD_CONTAINER']['TAG_ADD_OPTION'] = [
    '#type' => 'select',
    '#title' => 'Add the following selected tag',
    // '#prefix' => '<div style="display: inline-block; width: 45%;">',
    // '#suffix' => '</div>',
    '#description' => 'This will add a tag that isn\'t already <br />added to this study',
    '#options' => $all_add_tags_options,
    '#attributes' => ['style' => 'width: 100%'],
    '#default_value' => '',
  ];
  $form['TAG_ADD_CONTAINER']['TAG_ADD_OPTION_DO'] = [
    '#type' => 'submit',
    '#value' => t('Add tag to this study'),
    '#name' => 'add_tag',
    '#suffix' => '<div style="margin-bottom: 30px;"></div>'
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Reject/Approve and comments.
  if ($submission->status == TPPS_SUBMISSION_STATUS_PENDING_APPROVAL) {

    if ($page2_values['study_type'] != 1) {
      module_load_include('php', 'tpps', 'forms/build/page_3_helper');
      module_load_include('php', 'tpps', 'forms/build/page_3_ajax');
      $submission->sharedState['values'] = $form_state['values'] ?? $submission->sharedState['values'];
      $submission->sharedState['complete form']
        = $form_state['complete form'] ?? $submission->sharedState['complete form'];
      tpps_study_location($form, $submission->sharedState);
      $study_location = $page3_values['study_location'];
      $form['study_location']['type']['#default_value'] = $study_location['type'] ?? NULL;
      for ($i = 1; $i <= $study_location['locations']['number']; $i++) {
        $form['study_location']['locations'][$i]['#default_value']
          = $study_location['locations'][$i];
      }
      unset($form['study_location']['locations']['add']);
      unset($form['study_location']['locations']['remove']);

      $form['study_location']['#collapsed'] = TRUE;
    }

    $form['params'] = [
      '#type' => 'fieldset',
      '#title' => 'Select Environmental parameter types:',
      '#tree' => TRUE,
      '#description' => '',
    ];

    $orgamism_num = $page1_values['organism']['number'];
    $show_layers = FALSE;
    for ($i = 1; $i <= $orgamism_num; $i++) {
      if (!empty($page4_values["organism-$i"]['environment'])) {
        foreach ($page4_values["organism-$i"]['environment']['env_layers'] as $layer => $layer_id) {
          if (!empty($layer_id)) {
            foreach ($page4_values["organism-$i"]['environment']['env_params'][$layer] as $param_id) {
              if (!empty($param_id)) {
                $type = variable_get("tpps_param_{$param_id}_type", NULL);
                if (empty($type)) {
                  $query = db_select('cartogratree_fields', 'f')
                    ->fields('f', ['display_name'])
                    ->condition('field_id', $param_id)
                    ->execute();
                  $result = $query->fetchObject();
                  $name = $result->display_name;

                  $form['params'][$param_id] = [
                    '#type' => 'radios',
                    '#title' => "Select Type for environmental layer parameter \"$name\":",
                    '#options' => [
                      'attr_id' => t('@attr_id', ['@attr_id' => 'attr_id']),
                      'cvterm' => t('@cvterm', ['@cvterm' => 'cvterm']),
                    ],
                    '#required' => TRUE,
                  ];
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

    $form['approve-check'] = [
      '#type' => 'checkbox',
      '#title' => t('This submission has been reviewed and approved.'),
    ];
    $form['reject-reason'] = [
      '#type' => 'textarea',
      '#title' => t('Reason for rejection:'),
      '#states' => [
        'invisible' => [
          ':input[name="approve-check"]' => ['checked' => TRUE],
        ],
      ],
    ];
    $form['REJECT'] = [
      '#type' => 'submit',
      '#value' => t('Reject'),
      '#name' => 'reject',
      '#states' => [
        'invisible' => [
          ':input[name="approve-check"]' => ['checked' => TRUE],
        ],
      ],
    ];
  }
  $form['admin-comments'] = [
    '#type' => 'textarea',
    '#title' => t('Additional comments (administrator):'),
    '#default_value' => $submission->sharedState['admin_comments'] ?? NULL,
    '#prefix' => '<div id="tpps-admin-comments">',
    '#suffix' => '</div>',
  ];
  if ($submission->status == TPPS_SUBMISSION_STATUS_PENDING_APPROVAL) {
    $form['APPROVE'] = [
      '#type' => 'submit',
      '#value' => t('Approve'),
      '#name' => 'approve',
      '#states' => [
        'visible' => [
          ':input[name="approve-check"]' => ['checked' => TRUE],
        ],
      ],
    ];
  }

  if ($submission->status != TPPS_SUBMISSION_STATUS_PENDING_APPROVAL) {
    $form['SAVE_COMMENTS'] = [
      '#type' => 'button',
      '#value' => t('Save Comments'),
      '#name' => 'save_comments',
      '#ajax' => [
        'callback' => 'tpps_save_admin_comments',
        'wrapper' => 'tpps-admin-comments',
      ],
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'change_date',
    'title' => t('Change Date'),
  ];
  $date = $submission->sharedState['saved_values']['summarypage']['release-date'] ?? NULL;
  if (!empty($date)) {
    $datestr = "{$date['day']}-{$date['month']}-{$date['year']}";
    if (
      $submission->status != TPPS_SUBMISSION_STATUS_APPROVED
      || strtotime($datestr) > time()
    ) {
      tpps_admin_panel_add_section($form, $section);
      $form[$section['key']]['date'] = [
        '#type' => 'date',
        '#title' => t('Change release date'),
        '#description' => t('You can use this field and the button below '
          . 'to change the release date of a submission.'),
        '#default_value' => $date,
      ];
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'save_alternative_accessions',
    'title' => t('Save Alternative Accessions'),
  ];
  if ($submission->status == TPPS_SUBMISSION_STATUS_APPROVED) {
    tpps_admin_panel_add_section($form, $section);
    $form[$section['key']]['alternative_accessions'] = [
      '#type' => 'textfield',
      '#title' => t('Alternative accessions'),
      '#default_value' => $submission->sharedState['alternative_accessions'] ?? '',
      '#description' => t('Please provide a comma-delimited list of '
        . 'alternative accessions you would like to assign to this submission.'
      ),
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'save_vcf_import_setting',
    'title' => t('Save VCF Import Setting'),
  ];
  tpps_admin_panel_add_section($form, $section);
  $form[$section['key']]['DISABLE_VCF_IMPORT'] = [
    '#type' => 'checkbox',
    '#title' => t('Disable VCF Import in Tripal Job Submission'),
    '#default_value' => $page1_values['disable_vcf_import'] ?? 0,
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'save_vcf_import_mode',
    'title' => t('Save VCF Import Mode'),
  ];
  tpps_admin_panel_add_section($form, $section);
  $form[$section['key']]['VCF_IMPORT_MODE'] = [
    '#type' => 'select',
    '#title' => 'VCF Import mode',
    '#description' => t('Hybrid mode is the new method using the COPY command '
      . 'for some of the import. This requires the database user to have '
      . 'SUPERUSER rights. Inserts mode is the original code that used inserts '
      . 'only (which is much slower but tested and works for most cases).'
    ),
    '#options' => ['hybrid' => t('hybrid'), 'inserts' => t('inserts')],
    '#default_value' => 'hybrid',
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'run_vcf_snps_to_flat_file',
    'title' => t('Run VCF SNPS to Flat File'),
    'description' => t('Converts VCF SNPS into FLAT files including Tree IDs.'),
  ];
  tpps_admin_panel_add_section($form, $section);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'change_submission_owner',
    'title' => t('Change Submission Owner'),
  ];
  tpps_admin_panel_add_section($form, $section);
  $form[$section['key']]['new_owner'] = [
    '#type' => 'textfield',
    '#title' => t('Choose a new owner for the submission'),
    '#default_value' => tpps_get_user_email($submission->uid),
    '#autocomplete_path' => 'tpps/autocomplete/user',
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'sync_publication_data',
    'title' => t('Synchronize publication data'),
  ];
  tpps_admin_panel_add_section($form, $section);
  $form[$section['key']]['SYNC_PUBLICATION_DATA'] = [
    '#markup' => '<p>' . t('This will attempt to pull publication data from '
      . 'the publication content type and update the study info.') . '</p>',
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'change_study_view_role',
    'title' => t('Change Study View Role'),
  ];
  tpps_admin_panel_add_section($form, $section);
  $options = user_roles(TRUE);
  $options[0] = t('All users');
  $form[$section['key']]['CHANGE_STUDY_VIEW_ROLE'] = [
    '#type' => 'select',
    '#title' => t('Set this study view role'),
    '#description' => t('This will change the user role that is allowed '
      . 'to view this study'),
    '#options' => $options,
    '#default_value' => $submission->sharedState['study_view_role'] ?? 0,
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'change_tpps_type',
    'title' => t('Change TPPS type'),
  ];
  tpps_admin_panel_add_section($form, $section);
  $form[$section['key']]['CHANGE_TPPS_TYPE'] = [
    '#type' => 'select',
    '#title' => t("Change this study's TPPS type"),
    '#description' => t('This will change the submission state type and '
      . 'also the submission tag to the type you select'),
    '#options' => [
      'tppsc' => t('TPPSc'),
      'tpps' => t('TPPS'),
    ],
    '#default_value' => ($submission->sharedState['tpps_type'] == 'tppsc'
      ? 'tppsc' : 'tpps'),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'change_state_status',
    'title' => t('Change Status'),
  ];
  tpps_admin_panel_add_section($form, $section);
  $form[$section['key']]['state-status'] = [
    '#type' => 'select',
    '#title' => $section['title'],
    '#options' => tpps_form_get_submission_status_list(),
    '#default_value' => $submission->status,
    '#description' => t('Warning: This feature is experimental and may '
      . 'cause unforseen issues. Please do not change the status of this '
      . 'submission unless you are willing to risk the loss of existing data. '
      . '<br /><strong>The current status of the submission is @status</strong>.',
      ['@status' => $submission->status]
    ),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'generate_popstruct_from_vcf',
    'title' => t("Generate PopStruct FROM VCF"),
  ];
  tpps_admin_panel_add_section($form, $section);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'refresh_tpps_cvterms_cache',
    'title' => t('Refresh TPPS cvterms cache'),
  ];
  tpps_admin_panel_add_section($form, $section);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'regenerate_genotype_materialized_view',
    'title' => t('Regenerate genotype materialized view'),
    'description' => t('This regenerates the genotype view for the tpps details page.<br />'),
  ];
  tpps_admin_panel_add_section($form, $section);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'remove_study_markers_and_genotypes',
    'title' => t("Remove this study's markers and genotypes"),
    'description' => t('Warning: This will clear all markers and genotypes '
      . 'for this study. You will need to resubmit the study '
      . 'to import back this data.<br />'),
  ];
  tpps_admin_panel_add_section($form, $section);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $section = [
    'key' => 'change_tgdr_number',
    'title' => t('Change TGDR Number'),
    'description' => t('Warning: This will clear all data from the '
      . 'database and reimport as a new study.'),
  ];
  tpps_admin_panel_add_section($form, $section);
  $form[$section['key']]['CHANGE_TGDR_NUMBER'] = [
    '#type' => 'textfield',
    '#title' => t('Specify the new TGDR number only (do not include TGDR)'),
    // '#field_prefix' => 'TGDR',
    '#description' => t('WARNING: do not include TGDR.'),
  ];
  $form['#attributes']['class'][] = 'tpps-admin-panel';
  // @TODO Rename function and use default names for submitter and validator.
  $form['#validate'][] = 'tpps_admin_panel_validate';
  $form['#submit'][] = 'tpps_admin_panel_submit';

  tpps_add_css_js('main', $form);
  tpps_add_css_js('google_map', $form);
  return $form;
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
  $accession = $form_state['values']['accession'] ?? NULL;
  $submission = new Submission($accession);
  $submission->state['admin_comments']
    = $form_state['values']['admin-comments'] ?? NULL;
  $submission->save();

  drupal_set_message(t('Comments saved successfully'), 'status');
  return $form['admin-comments'];
}

/**
 * Implements hook_form_validate().
 *
 * Checks that the reject reason has been filled out if the submission was
 * rejected.
 */
function tpps_admin_panel_validate($form, &$form_state) {
  if ($form_state['submitted'] == '1') {
    if (
      isset($form_state['values']['reject-reason'])
      && $form_state['values']['reject-reason'] == ''
      && $form_state['triggering_element']['#name'] == 'reject'
    ) {
      form_set_error('reject-reason',
        t('Please explain why the submission was rejected.')
      );
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Button 'Approve'.
    if ($form_state['triggering_element']['#name'] == 'approve') {
      $accession = $form_state['values']['accession'];
      $submission = new Submission($accession);
      foreach ($submission->state['file_info'] as $files) {
        foreach ($files as $fid => $file_type) {
          if (
            !empty($form_state['values']["edit_file_{$fid}_check"])
            and empty($form_state['values']["edit_file_{$fid}_file"])
          ) {
            form_set_error("edit_file_{$fid}_file",
              t('Please upload a revised version fo the user-provided file.')
            );
          }
          if (!empty($form_state['values']["edit_file_{$fid}_file"])) {
            if ($file = tpps_file_load($fid)) {
              file_usage_add($file, 'tpps', 'tpps_project', substr($accession, 4));
            }
          }
        }
      }

      if (!empty($form_state['values']['study_location'])) {
        for ($i = 1; $i <= $form_state['values']['study_location']['locations']['number']; $i++) {
          if (empty($form_state['values']['study_location']['locations'][$i])) {
            form_set_error("study_location][locations][$i",
              t("Study location $i: field is required.")
            );
          }
        }
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    if ($form_state['triggering_element']['#name'] == 'save_alternative_accessions') {
      $alt_acc = explode(',', $form_state['values']['alternative_accessions']);
      foreach ($alt_acc as $acc) {
        if (!preg_match('/^TGDR\d{3,}$/', $acc)) {
          form_set_error('alternative_accessions',
            t("The accession, $acc is not a valid TGDR### accession number.")
          );
          continue;
        }
        $result = db_select('tpps_submission', 's')
          ->fields('s')
          ->condition('accession', $acc)
          ->range(0, 1)
          ->execute()->fetchObject();
        if (!empty($result)) {
          form_set_error('alternative_accessions',
            t("The accession, $acc is already in use.")
          );
        }
      }
    }

    if ($form_state['triggering_element']['#name'] == 'change_submission_owner') {
      $new_user = user_load_by_mail($form_state['values']['new_owner']);
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

  $accession = $form_state['values']['accession'];
  $submission = new Submission($accession);
  $tpps_submission_id = $submission->id;
  $owner = user_load($submission->uid);
  $to = $owner->mail;

  $submission->state['admin_comments'] = $form_state['values']['admin-comments'] ?? NULL;
  $page1_values = $submission->state['saved_values'][TPPS_PAGE_1] ?? NULL;
  $from = variable_get('site_mail', '');

  // @TODO Minor. We could try find type using '#form_id' under $state['values'].
  $type = $submission->state['tpps_type'] ?? 'tpps';
  $type_label = ($type == 'tpps') ? t('TPPS') : t('TPPSC');

  $params = [];
  $params['subject'] = t('@type_label Submission Rejected: @title',
    [
      '@type_label' => $type_label,
      '@title' => $page1_values['publication']['title'] ?? NULL,
    ]
  );
  $params['uid'] = $owner->uid;
  $params['reject-reason'] = $form_state['values']['reject-reason'] ?? NULL;
  $params['base_url'] = $base_url;
  $params['title'] = $page1_values['publication']['title'] ?? NULL;
  $params['body'] = '';
  $params['type'] = $type;
  $params['type_label'] = $type_label;

  // @TODO Check why this variables are set because I didn't found them in
  // database but each variable is an extra DB query.
  if (isset($form_state['values']['params'])) {
    foreach ($form_state['values']['params'] as $param_id => $type) {
      variable_set("tpps_param_{$param_id}_type", $type);
    }
  }

  // We use '#name' instead of human-readable '#value' because '#value' could
  // be localized and this code became broken.
  switch ($form_state['triggering_element']['#name']) {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'add_tag':
      $tpps_tag_id = $form_state['values']['TAG_ADD_OPTION'];
      // Insert tag for this tpps_submission_id.
      chado_query('INSERT INTO tpps_submission_tag
        (tpps_submission_id, tpps_tag_id)
        VALUES (:tpps_submission_id, :tpps_tag_id)',
        [
          ':tpps_submission_id' => $tpps_submission_id,
          ':tpps_tag_id' => $tpps_tag_id,
        ]
      );
      // Get the tag name for the message alert.
      $tag_name = "";
      $tag_name_results = chado_query(
        'SELECT * FROM tpps_tag WHERE tpps_tag_id = :tpps_tag_id',
        [':tpps_tag_id' => $tpps_tag_id]
      );
      foreach ($tag_name_results as $row) {
        $tag_name = $row->name;
      }
      drupal_set_message($tag_name . " has been added to the study");
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'remove_tag':
      $tpps_tag_id = $form_state['values']['TAG_REMOVE_OPTION'];
      // Get the tag name for the message alert.
      $tag_name = "";
      $tag_name_results = chado_query(
        'SELECT * FROM tpps_tag WHERE tpps_tag_id = :tpps_tag_id',
        [':tpps_tag_id' => $tpps_tag_id]
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'run_vcf_snps_to_flat_file':
      // TODO Flat files functions and Tripal job.
      global $user;
      $project_id = $submission->sharedState['ids']['project_id'] ?? NULL;
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $jid = tripal_add_job("Generate VCF to SNPs Flat file - $accession "
        . "(project_id=$project_id) " . date("F j, Y, g:i a"), 'tpps',
        'tpps_genotypes_to_flat_files_and_find_studies_overlaps',
        [$submission->state, $submission->sharedState, true], $user->uid, 10, $includes, TRUE
      );
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'save_vcf_import_setting':
      $mode = ($form_state['values']['DISABLE_VCF_IMPORT'] == 1) ? 1 : 0;
      $submission->sharedState['saved_values'][TPPS_PAGE_1]['disable_vcf_import']
        = $submission->state['saved_values'][TPPS_PAGE_1]['disable_vcf_import']
          = $value;
      $submission->save();
      drupal_set_message(t('VCF disable import setting saved'));
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'save_vcf_import_mode':
      $mode = $form_state['values']['VCF_IMPORT_MODE'] ?? 'hybrid';
      $submission->sharedState['saved_values'][TPPS_PAGE_1]['vcf_import_mode']
        = $submission->state['saved_values'][TPPS_PAGE_1]['vcf_import_mode']
          = $mode;
      $submission->save();
      drupal_set_message(
        t('VCF import mode saved as @mode".', ['@mode' => $mode])
      );
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'regenerate_genotype_materialized_view':
      global $user;
      $project_id = $submission->state['ids']['project_id'] ?? NULL;
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $jid = tripal_add_job(
        t('Generate materialized view for @accession (project_id=@project_id)',
          ['@accession' => $accession, '@project_id' => $project_id]
        ),
        'tpps', 'tpps_generate_genotype_materialized_view', [$project_id],
        $user->uid, 10, $includes, TRUE
      );
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'remove_study_markers_and_genotypes':
      global $user;
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/markers_genotypes_utils');
      $jid = tripal_add_job("TPPS REMOVE all study markers and genotypes - $accession",
        'tpps', 'tpps_remove_all_markers_genotypes', [$accession],
        $user->uid, 10, $includes, TRUE
      );
      // drupal_set_message(t('Tripal Job created to remove all study '
      // . 'markers and genotypes from @accession.', ['@accession' => $accession]
      // ));
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_tgdr_number':
      // Check if a valid tgdr number was supplied.
      if (!is_numeric($form_state['values']['CHANGE_TGDR_NUMBER'] ?? NULL)) {
        drupal_set_message(t('You did not enter a valid number. Operation aborted.'));
        break;
      }

      $new_accession = 'TGDR' . $form_state['values']['CHANGE_TGDR_NUMBER'];
      // Check if the new tgdr number does not exist in the database
      // if it exists, abort the mission.
      $results = chado_query('SELECT count(*) as c1 FROM public.tpps_submission '
        . 'WHERE accession = :new_accession',
        [':new_accession' => $new_accession]
      );
      $result_object = $results->fetchObject();
      $result_count = $result_object->c1;
      if ($result_count > 0) {
        drupal_set_message(t('It seems the TGDR number you wanted to change '
          . 'to is already in use. Operation aborted due to safety concerns.'
        ));
        break;
      }
      global $user;
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/markers_genotypes_utils');
      $includes[] = module_load_include('inc', 'tpps', 'includes/submissions');
      // Job to remove genotype information and features.
      $jid = tripal_add_job(
        "TPPS REMOVE all study markers and genotypes - $accession",
        'tpps', 'tpps_remove_all_markers_genotypes', [$accession],
        $user->uid, 10, $includes, TRUE
      );

      // Job to change the TGDR number to the new TGDR number.
      $jid = tripal_add_job("TPPS rename $accession to $new_accession",
        'tpps', 'tpps_change_tgdr_number', [$accession, $new_accession],
        $user->uid, 10, $includes, TRUE
      );

      // Now run the new import for the new accession TGDR number.
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $jid = tripal_add_job("TPPS Record Submission - $new_accession",
        'tpps', 'tpps_submit_all', [$new_accession],
        $submission->state['submitting_uid'], 10, $includes, TRUE
      );
      $submission->sharedState['job_id']
        = $submission->state['job_id']
          = $jid;
      $submission->save();
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'generate_popstruct_from_vcf':
      global $user;
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $page4_values = $submission->state['saved_values'][TPPS_PAGE_4] ?? NULL;
      $snps_fieldset = 'SNPs';

      // Uploaded VCF.
      $vcf_fid = ($page4_values['organism-1']['genotype'][$snps_fieldset]['vcf'] ?? NULL);
      $vcf_file = tpps_file_load($vcf_fid);
      // Cluster (local) VCF.
      $local_vcf = ($page4_values['organism-1']['genotype'][$snps_fieldset]['local_vcf'] ?? NULL);

      if (empty($vcf_file) && empty($local_vcf)) {
        $message = t("Could not find a VCF tied to organism-1, "
          . "are you sure you linked a VCF file?");
        drupal_set_message($message);
      }
      else {
        if ($vcf_file) {
          $location = tpps_get_location($vcf_file->uri);
        }
        elseif ($local_vcf) {
          $location = strip_tags($local_vcf);
        }
        $jid = tripal_add_job("TPPS Generate PopStruct FROM VCF - $accession",
          'tpps', 'tpps_generate_popstruct',
          [$accession, $location], $user->uid, 10, $includes, TRUE
        );
      }
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'refresh_tpps_cvterms_cache':
      global $user;
      $includes = [
        module_load_include('php', 'tpps', 'forms/submit/submit_all'),
        module_load_include('inc', 'tpps', 'includes/cvterm_utils'),
      ];
      $jid = tripal_add_job('TPPS REFRESH CVTERMS CACHE', 'tpps',
        'tpps_cvterms_clear_cache', [], $user->uid, 10, $includes, TRUE
      );
      // drupal_set_message(t('Tripal Job created to remove all study '
      //   . 'markers and genotypes from @accession.',
      //   ['@accession' => $accession]
      // ));
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_tpps_type':
      // Get the tpps_submission_id from the public.tpps_submission table.
      $results = chado_query('SELECT * FROM public.tpps_submission
        WHERE accession = :accession LIMIT 1',
        [':accession' => $accession]
      );
      $tpps_submission_id = NULL;
      foreach ($results as $row) {
        $tpps_submission_id = $row->tpps_submission_id;
      }
      if ($tpps_submission_id == NULL) {
        $message = t('Could not find a TPPS SUBMISSION ID for this '
          . 'accession, contact administration');
        drupal_set_message($message, 'error');
        break;
      }

      if ($form_state['values']['CHANGE_TPPS_TYPE'] == 'tppsc') {
        // $state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 1;
        // Set the state tpps_type to tppsc.
        $submission->sharedState['tpps_type']
          = $submission->state['tpps_type']
            = 'tppsc';

        // Update the submission tag table which in term will get rippled
        // into the ct_trees_all_view materialized view that filters
        // internal and external submissions.
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
        $submission->sharedState['tpps_type']
          = $submission->state['tpps_type']
            = 'tpps';

        // Update the submission tag table which in term will get rippled
        // into the ct_trees_all_view materialized view that filters
        // internal and external submissions.
        chado_query('UPDATE public.tpps_submission_tag
          SET tpps_tag_id = 1
          WHERE tpps_submission_id = :tpps_submission_id
          AND (tpps_tag_id = 1 OR tpps_tag_id = 2)',
          [
            ':tpps_submission_id' => $tpps_submission_id
          ]
        );
      }
      $submission->save();
      $message = t('Updated study TPPS type: @type.',
        ['@type' => $submission->state['tpps_type']]);
      drupal_set_message($message);
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'reject':
      $submission->save(TPPS_SUBMISSION_STATUS_INCOMPLETE);
      cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
      // Email notification.
      $lang = user_preferred_language($owner);
      drupal_mail('tpps', 'user_rejected', $to, $lang, $params, $from, TRUE);
      drupal_set_message(t('Submission Rejected. Message has been sent to user.'));

      drupal_goto('<front>');
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'approve':
      module_load_include('php', 'tpps', 'forms/submit/submit_all');
      global $user;
      $submission->status = TPPS_SUBMISSION_STATUS_APPROVED;

      // Email notification.
      $params['subject'] = "$type_label Submission Approved: "
        . $submission->get(['saved_values', TPPS_PAGE_1, 'publication', 'title']);
      $params['accession'] = $accession;
      drupal_set_message(t('Submission Approved! Message has been sent to user.'));
      $lang = user_preferred_language(user_load_by_name($to));
      drupal_mail('tpps', 'user_approved', $to, $lang, $params, $from, TRUE);

      // Can't move to Submission::setStatus because it uses $form_state of
      // just submitted form (not Submission).
      $revised_files = $submission->state['revised_files'] ?? [];
      $submission->set(['revised_files'], $revised_files, TRUE);
      foreach ($submission->state['file_info'] as $files) {
        foreach ($files as $fid => $file_type) {
          if (!empty($form_state['values']["edit_file_{$fid}_check"])) {
            $submission->set(
              ['revised_files', $fid],
              $form_state['values']["edit_file_{$fid}_file"], TRUE
            );
          }
        }
      }
      if (!empty($form_state['values']['study_location'])) {
        $submission->set(
          ['saved_values', TPPS_PAGE_3, 'study_location', 'type'],
          $form_state['values']['study_location']['type'], TRUE
        );
        $locations_number = $form_state['values']['study_location']['locations']['number'];
        for ($i = 1; $i <= $locations_number; $i++) {
          $submission->set(
            ['saved_values', TPPS_PAGE_3, 'study_location', 'locations', $i],
            $form_state['values']['study_location']['locations'][$i], TRUE
          );
        }
      }

      $submission->save();
      cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_date':
      $submission->state['saved_values']['summarypage']['release-date']
        = $form_state['values']['date'];
      $submission->sharedState['saved_values']['summarypage']['release-date']
        = $form_state['values']['date'];
      $submission->save();
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_state_status':
      drupal_set_message(t('Changed status from "@old_status" to "@new_status".',
        [
          '@old_status' => $submission->status,
          '@new_status' => $form_state['values']['state-status'],
        ]
      ));
      $submission->save($form_state['values']['state-status']);
      cache_clear_all('*', (TPPS_CACHE_BIN ?? 'cache'), TRUE);
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_study_view_role':
      $view_role = $form_state['values']['CHANGE_STUDY_VIEW_ROLE'];
      if ($view_role > 0) {
        $submission->set(['study_view_role'], $view_role, TRUE);
        drupal_set_message(t(
          'Study view role has been set to "@role".',
          ['@role' => user_roles(TRUE)[$view_role]]
        ));
      }
      else {
        unset($submission->state['study_view_role']);
        unset($submission->sharedState['study_view_role']);
        drupal_set_message(t('Study view role set to public all users'));
      }
      $submission->save();
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'save_alternative_accessions':
      $old_alt_acc = $submission->state['alternative_accessions'] ?? '';
      $new_alt_acc = $form_state['values']['alternative_accessions'];
      if ($old_alt_acc != $new_alt_acc) {
        tpps_submission_add_alternative_accession(
          $submission->state, explode(',', $new_alt_acc)
        );
        $submission->set(['alternative_accessions'], $new_alt_acc, TRUE);
        $submission->save();
      }
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_submission_owner':
      $new_user = user_load_by_mail($form_state['values']['new_owner']);
      drupal_set_message(t(
        'Changed submission owner (from "@old_user" to "@new_user").',
        [
          '@old_user' => $submission->uid,
          '@new_user' => $new_user->uid,
        ]
      ));
      $submission->setUid($new_user->uid);
      $submission->save();
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'sync_publication_data':
      $project_id = $submission->state['ids']['project_id'] ?? NULL;
      $pub_id = db_select('chado.project_pub', 'p')
        ->fields('p', ['pub_id'])
        ->condition('project_id', $project_id)
        ->execute()
        ->fetchObject()
        ->pub_id;
      if (!empty($pub_id)) {
        $bundle = tripal_load_bundle_entity(['label' => 'Publication']);

        $pub_entity_id = NULL;
        try {
          $pub_entity_id = chado_get_record_entity_by_bundle($bundle, $pub_id);
        }
        catch (Exception $ex) {
          // Couldn't find a publication entity.
        }

        if (!isset($pub_entity_id)) {
          drupal_set_message('Could not find a matching publication safely. Will not synchronize data.');
        }
        else {
          drupal_set_message("Publication entity id found ($pub_entity_id), "
            . "retrieving publication data...");
          // This will return results as an array.
          $publication_entity_results = tripal_load_entity('TripalEntity',
            [$pub_entity_id]
          );
          // Get the entity.
          $publication_entity = $publication_entity_results[$pub_entity_id];
          if (isset($publication_entity->title)) {
            $pub_title = $publication_entity->title;
            if ($pub_title != "") {
              drupal_set_message('Found a valid publication title, syncing with study.');
              $submission->sharedState['title']
                = $submission->state['title']
                  = $pub_title;
            }
          }
          if (
            !empty($pub_year = $publication_entity->tpub__year['und'][0]['safe_value'] ?? NULL)
          ) {
            drupal_set_message('Found a valid publication year, synced with study.');
            $submission->sharedState['pyear']
              = $submission->state['pyear']
                = $pub_year;
          }
          if (
            !empty($pub_abstract = $publication_entity->tpub__abstract['und'][0]['value'] ?? NULL)
          ) {
            drupal_set_message('Found a valid publication abstract, synced with study.');
            $submission->sharedState['abstract']
              = $submission->state['abstract']
                = $pub_abstract;
          }
          if (isset($publication_entity->tpub__authors['und'][0]['value'])) {
            $pub_authors = $publication_entity->tpub__authors['und'][0]['value'];
            if ($pub_authors != "") {
              preg_match_all('/.[^,]+,*/', $pub_authors, $matches);
              if (count($matches) > 0) {
                $actual_matches = $matches[0];
                $filtered_matches = array();
                foreach ($actual_matches as $match) {
                  $match = str_replace(',', '', $match);
                  $match = trim($match);
                  array_push($filtered_matches, $match);
                }
                if (count($filtered_matches) > 0) {
                  $submission->sharedState['authors']
                    = $submission->state['authors']
                      = $filtered_matches;
                  $message = t('Found valid publication authors, synced with study.');
                  drupal_set_message($message);
                }
              }
            }
          }
          $submission->save();
          drupal_set_message(t('Done.'));
        }
      }
      else {
        $message = t('Could not find a valid pub_id for this study. '
          . 'Edit via TPPSc and make sure you have connected this '
          . 'study to a valid paper'
        );
        drupal_set_message($message);
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
  $panel_url = 'tpps-admin-panel/reports';
  // Format: <Report Key> => <Path related to $panel_url>.
  return [
    'missing_doi' => $panel_url . '/missing-doi',
    'no_synonym' => $panel_url . '/no-synonyms',
    'unit_warning' => $panel_url . '/unit-warning',
    'order_family_not_exist' => $panel_url . '/order-family-not-exist',
    'missing_files' => $panel_url . '/missing-files',
    'submissions_missing_files' => $panel_url . '/submissions-missing-files',
    'submissions_without_files' => $panel_url . '/submissions-without-files',
    'submissions_all_files' => $panel_url . '/submissions-all-files',
    'organism_list' => $panel_url . '/organism-list',
    'doi_duplicates' => $panel_url . '/doi-duplicates',
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
  $items[] = l(t('Imported Studies'), 'admin/reports/tpps/imported-studies');
  $form['report_menu'] = [
    '#type' => 'fieldset',
    '#title' => t('TPPS Reports'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    'table' => ['#markup' => theme('item_list', ['items' => $items])],
  ];
}

/**
 * Builds fieldset with single submit button.
 *
 * @param array $form
 *   Admin Panel Form.
 * @param array $data
 *   Section's data. Keys are:
 *   'key' string
 *      Unique Section Key. Will be used for fieldset and submit button '#name'.
 *   'title' string
 *     Localized title for Fieldset and action button.
 *   'description' string
 *      Optional. Localized description of section.
 */
function tpps_admin_panel_add_section(array &$form, array $data) {
  $form[$data['key']] = [
    '#type' => 'fieldset',
    '#title' => $data['title'] ?? $data['key'] ?? '',
    '#description' => $data['description'] ?? '',
    'submit' => [
      '#type' => 'submit',
      '#value' => $data['title'],
      '#name' => $data['key'],
      '#weight' => 100,
    ],
  ];
}
