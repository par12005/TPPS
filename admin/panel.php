<?php

/**
 * @file
 * Defines contents of TPPS administrative panel.
 *
 * Site administrators will use this form to approve or reject completed TPPS
 * submissions.
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
function tpps_admin_panel(array $form, array &$form_state, $accession = NULL) {
  if (empty($accession)) {
    tpps_admin_panel_top($form);
  }
  else {
    tpps_manage_submission_form($form, $form_state, $accession);
  }
  $form['#attributes']['class'][] = 'tpps-admin-panel';
  tpps_add_css_js('main', $form);
  return $form;
}

/**
 * Generates all materialized views.
 */
function tpps_manage_generate_all_materialized_views(array $form, array &$form_state, $option = NULL) {
  global $user;

  $form = $form ?? [];
  tpps_add_css_js('main', $form);
  module_load_include('php', 'tpps', 'forms/submit/submit_all');

  $includes = [];
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
function tpps_manage_submission_form(array &$form, array &$form_state, $accession = NULL) {
  global $base_url;

  $submission = new Submission($accession);

  $page1_values = &$submission->sharedState['saved_values'][TPPS_PAGE_1] ?? NULL;
  $page2_values = &$submission->sharedState['saved_values'][TPPS_PAGE_2] ?? NULL;
  $page3_values = &$submission->sharedState['saved_values'][TPPS_PAGE_3] ?? NULL;
  $page4_values = &$submission->sharedState['saved_values'][TPPS_PAGE_4] ?? NULL;

  if (empty($submission->sharedState['status'])) {
    $submission->save($submission->info['status']);
  }
  $options = [];
  $display = l(t("Back to TPPS Admin Panel"), "$base_url/tpps-admin-panel");

  // Check for log file
  // Step 1 - Look for the last tripal job that has the accession.
  $results = db_query("SELECT * FROM public.tripal_jobs WHERE job_name LIKE "
    . "'TPPS Record Submission - $accession' ORDER BY submit_date DESC LIMIT 1;");
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
    $log_path = drupal_realpath('public://') . '/tpps_job_logs/';
    // dpm($log_path . $accession . "_" . $job_id . ".txt");
    if (file_exists($log_path . $accession . "_" . $job_id . ".txt")) {
      $display .= "<div style='padding: 10px;background: #e9f9ef;border: "
        . "1px solid #90bea9;font-size: 18px;'><a target='_blank' "
        . "href='../tpps-admin-panel-logs/" . $accession . "_" . $job_id
        . "'>Latest job log file ($accession - $job_id)</a></div>";
    }
    else {
      $display .= "<div style='padding: 10px;'>Could not find job log file "
        . "(this can happen if the log file was deleted - resubmit study "
        . "if necessary to regenerate log file)</div>";
    }
  }

  if ($submission->info['status'] == "Pending Approval") {
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
    $submission->info['status'] == 'Pending Approval'
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
    [':tpps_submission_id' => $submission->info['tpps_submission_id']]
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
    [':tpps_submission_id' => $submission->info['tpps_submission_id']]
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
  if ($submission->info['status'] == "Pending Approval") {

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
  if ($submission->info['status'] == "Pending Approval") {
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

  if ($submission->info['status'] != "Pending Approval") {
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
    if ($submission->info['status'] != 'Approved' || strtotime($datestr) > time()) {
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
  if ($submission->info['status'] == "Approved") {
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
    '#default_value' => tpps_get_user_email($submission->info['uid']),
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
    // @TODO Convert statuses into constants and use function to get list.
    '#options' => [
      'Incomplete' => t('Incomplete'),
      'Pending Approval' => t('Pending Approval'),
      'Approved' => t('Approved'),
      'Submission Job Running' => t('Submission Job Running'),
    ],
    '#default_value' => $submission->info['status'],
    '#description' => t('Warning: This feature is experimental and may '
      . 'cause unforseen issues. Please do not change the status of this '
      . 'submission unless you are willing to risk the loss of existing data. '
      . '<br /><strong>The current status of the submission is @status</strong>.',
      ['@status' => $submission->info['status']]
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
  $submission->state['admin_comments'] = $form_state['values']['admin-comments'] ?? NULL;
  $submission->save();

  drupal_set_message(t('Comments saved successfully'), 'status');
  return $form['admin-comments'];
}

/**
 * Shows lists of studies with statue 'Pending', 'Approved', and 'Incomplete'.
 *
 * @param array $form
 *   The form element of the TPPS admin panel page.
 */
function tpps_admin_panel_top(array &$form) {
  global $base_url;

  tpps_admin_panel_reports($form);
  $submissions = tpps_load_submission_multiple([], FALSE);

  $pending = $approved = $incomplete = $unpublished_old = $submitting_user_cache = [];
  $mail_cvterm = tpps_load_cvterm('email')->cvterm_id;

  foreach ($submissions as $submission) {
    $state = unserialize($submission->submission_state);
    $accession = $state['accession'] ?? NULL;
    $submission->info = tpps_load_submission_info($accession, $reset = TRUE);

    if (empty($submitting_user_cache[$submission->info['uid']])) {
      $mail = tpps_get_user_email($submission->info['uid']);
      $query = db_select('chado.contact', 'c');
      $query->join('chado.contactprop', 'cp', 'cp.contact_id = c.contact_id');
      $query->condition('cp.value', $mail);
      $query->condition('cp.type_id', $mail_cvterm);
      $query->fields('c', array('name'));
      $query->range(0, 1);
      $query = $query->execute();
      $name = $query->fetchObject()->name ?? NULL;

      $submitting_user_cache[$submission->info['uid']] = $name ?? $mail;
    }
    $submitting_user = $submitting_user_cache[$submission->info['uid']] ?? NULL;

    if (!empty($state)) {
      switch ($submission->info['status']) {
        case 'Pending Approval':
          $row = array(
            l($accession, 'tpps-admin-panel/' . $accession),
            $submitting_user,
            $state['saved_values'][TPPS_PAGE_1]['publication']['title'],
            !empty($state['completed']) ? date("F j, Y, g:i a", $state['completed']) : "Unknown",
            tpps_show_tags(tpps_submission_get_tags($accession)),
          );
          $pending[(int) substr($accession, 4)] = $row;
          break;

        case 'Approved':
          $status_label = !empty($state['loaded'])
            ? "Approved - load completed on " . date("F j, Y, \a\\t g:i a", $state['loaded'])
            : "Approved";
          if (!empty($state['loaded'])) {
            $days_since_load = (time() - $state['loaded']) / (60 * 60 * 24);
            $unpublished_threshold = variable_get('tpps_unpublished_days_threshold', 180);
            $pub_status = $state['saved_values'][TPPS_PAGE_1]['publication']['status'] ?? NULL;
            if (
              !empty($pub_status)
              and $pub_status != 'Published'
              and $days_since_load >= $unpublished_threshold
            ) {
              $owner = $submitting_user;
              $contact_bundle = tripal_load_bundle_entity(
                ['label' => 'Tripal Contact Profile']
              );
              $owner_mail = tpps_get_user_email($submission->info['uid']);
              $owner = "$submitting_user ($owner_mail)";

              // If Tripal Contact Profile is available, we want to link to the
              // profile of the owner instead of just displaying the name.
              if ($contact_bundle) {
                $query = new EntityFieldQuery();
                $results = $query->entityCondition('entity_type', 'TripalEntity')
                  ->entityCondition('bundle', $contact_bundle->name)
                  ->fieldCondition('local__email', 'value', $owner_mail)
                  ->range(0, 1)
                  ->execute();
                // [VS]
                if (!empty($results['TripalEntity'])) {
                  // Commented out because there is a lot of warning messages
                  // shown at page.
                  //
                  //  $entity = current(
                  //    array_reverse(
                  //      entity_load(
                  //        'TripalEntity',
                  //        array_keys($results['TripalEntity'])
                  //      )
                  //    )
                  //  );
                  //  $owner = "<a href=\"$base_url/TripalContactProfile/"
                  //    . "{$entity->id}\">$submitting_user</a>";
                }
                // [/VS]
              }
              $row = [
                l($accession, 'tpps-admin-panel/' . $accession),
                date("F j, Y", $state['loaded'])
                  . " (" . round($days_since_load) . " days ago)",
                $pub_status,
                $owner,
              ];
              if (tpps_access('view own tpps submission', $accession)) {
                $row[] = l(t('Edit publication information'),
                  'tpps/' . $accession . '/edit-publication'
                );
              }
              $unpublished_old[(int) substr($accession, 4)] = $row;
            }
          }

        case 'Submission Job Running':
          $status_label = $status_label ?? (
            !empty($state['approved'])
            ? ("Submission Job Running - job started on "
              . date("F j, Y, \a\t g:i a", $state['approved']))
            : "Submission Job Running"
          );

        case 'Approved - Delayed Submission Release':
          if (empty($status_label)) {
            $release = $state['saved_values']['summarypage']['release-date'] ?? NULL;
            $release = strtotime("{$release['day']}-{$release['month']}-{$release['year']}");
            $status_label = "Approved - Delayed Submission Release on " . date("F j, Y", $release);
          }
          $row = [
            l($accession, 'tpps-admin-panel/' . $accession),
            $submitting_user,
            $state['saved_values'][TPPS_PAGE_1]['publication']['title'],
            $status_label,
            tpps_show_tags(tpps_submission_get_tags($accession)),
          ];
          $approved[(int) substr($accession, 4)] = $row;
          break;

        default:
          switch ($state['stage'] ?? NULL) {
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

          $row = [
            l($accession, '/tpps-admin-panel/' . $accession),
            $submitting_user,
            $state['saved_values'][TPPS_PAGE_1]['publication']['title'] ?? t('Title not provided yet'),
            $stage,
            !empty($state['updated']) ? date("F j, Y, g:i a", $state['updated']) : t('Unknown'),
            tpps_show_tags(tpps_submission_get_tags($accession)),
          ];
          $incomplete[(int) substr($accession, 4)] = $row;
          break;
      }
    }
  }

  krsort($pending);
  krsort($approved);
  krsort($incomplete);

  $params = ['attributes' => ['target' => '_blank']];
  $general_tasks = [
    l(t('Refresh all genotype materialized views'),
      'tpps-admin-panel/refresh-genotypes-materialized-views', $params
    ),
    l(t('TPPS Submission Tools'), 'tpps/submission', $params),
  ];
  $form['general_tasks'] = [
    '#type' => 'fieldset',
    '#title' => t('General tasks'),
    '#collapsible' => TRUE,
    'table' => ['#markup' => theme('item_list', ['items' => $general_tasks])],
  ];

  $vars = [
    'attributes' => ['class' => ['view', 'tpps_table']],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  ];

  $vars['header'] = [
    t('Accession Number'),
    t('Approval date'),
    t('Publication Status'),
    t('Submission Owner'),
    t('Actions'),
  ];
  $vars['rows'] = $unpublished_old;
  $unpublished_table = theme('table', $vars);

  $vars['header'] = [
    t('Accession Number'),
    t('Submitting User'),
    t('Title'),
    t('Date Submitted'),
    t('Tags'),
  ];
  $vars['rows'] = $pending;
  $pending_table = theme('table', $vars);

  $vars['header'] = [
    t('Accession Number'),
    t('Submitting User'),
    t('Title'),
    t('Status'),
    t('Tags'),
  ];
  $vars['rows'] = $approved;
  $approved_table = theme('table', $vars);

  $vars['header'] = [
    t('Accession Number'),
    t('Submitting User'),
    t('Title'),
    t('Stage'),
    t('Last Updated'),
    t('Tags'),
  ];
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
    $form['pending'] = [
      '#type' => 'fieldset',
      '#title' => t('Pending TPPS submissions'),
      '#collapsible' => TRUE,
      'table' => ['#markup' => $pending_table],
    ];
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
    $to_resubmit[] = [$result->accession];
  }
  if (!empty($to_resubmit)) {
    $vars['header'] = ['Accession'];
    $vars['rows'] = $to_resubmit;
    $to_resubmit_table = theme('table', $vars);
    $form['resubmit'] = [
      '#type' => 'fieldset',
      '#title' => "<img src='$base_url/misc/message-16-warning.png'> "
        . t('Old TGDR Submissions to be resubmitted'),
      '#collapsible' => TRUE,
      'table' => ['#markup' => $to_resubmit_table],
    ];
  }

  $tpps_new_orgs = variable_get('tpps_new_organisms', NULL);
  $db = chado_get_db(['name' => 'NCBI Taxonomy']);
  if (!empty($db)) {
    $rows = array();
    $query = db_select('chado.organism', 'o');
    $query->fields('o', ['organism_id', 'genus', 'species']);

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
        $rows[] = [
          "<a href=\"$base_url/bio_data/{$id}/edit\" target=\"_blank\">"
          . "$org->genus $org->species</a>",
        ];
        continue;
      }
      $rows[] = ["$org->genus $org->species"];
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // New Species table.
    if (!empty($rows)) {
      $vars = [
        'header' => [],
        'rows' => $rows,
        'attributes' => [
          'class' => ['view', 'tpps_table'],
          'id' => 'new_species',
        ],
        'caption' => '',
        'colgroups' => NULL,
        'sticky' => FALSE,
        'empty' => '',
      ];

      $form['new_species'] = [
        '#type' => 'fieldset',
        '#title' => t('New Species'),
        '#description' => t('The species listed below likely need to be '
          . 'updated, because they do not have NCBI Taxonomy identifiers '
          . 'in the database.'),
        '#collapsible' => TRUE,
        'table' => ['#markup' => theme('table', $vars)],
      ];
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
    if (
      isset($form_state['values']['reject-reason'])
      && $form_state['values']['reject-reason'] == ''
      && $form_state['triggering_element']['#name'] == 'reject'
    ) {
      form_set_error('reject-reason',
        t('Please explain why the submission was rejected.')
      );
    }

    if ($form_state['triggering_element']['#name'] == 'approve') {
      $accession = $form_state['values']['accession'];
      $state = tpps_load_submission_state($accession);
      foreach ($state['file_info'] as $files) {
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

  $accession = $form_state['values']['accession'];
  $submission = new Submission($accession);
  $tpps_submission_id = $submission->info['tpps_submission_id'];
  $owner = user_load($submission->info['uid']);
  $to = $owner->mail;
  $state = $submission->state;

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
      $project_id = $submission->state['ids']['project_id'] ?? NULL;
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = [$submission->state];
      $jid = tripal_add_job("Generate VCF to SNPs Flat file - $accession "
        . "(project_id=$project_id)", 'tpps',
        'tpps_genotypes_to_flat_files_and_find_studies_overlaps',
        $args, $user->uid, 10, $includes, TRUE
      );
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'save_vcf_import_mode':
      if ($form_state['values']['DISABLE_VCF_IMPORT'] == 1) {
        $submission->state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 1;
      }
      else {
        $submission->state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 0;
      }
      $submission->save();
      drupal_set_message(t('VCF disable import setting saved'));
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'save_vcf_import_mode':
      $mode = $form_state['values']['VCF_IMPORT_MODE'] ?? 'hybrid';
      $submission->state['saved_values'][TPPS_PAGE_1]['vcf_import_mode'] = $mode;
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
      $args = [$project_id];
      $jid = tripal_add_job(
        t('Generate materialized view for @accession (project_id=@project_id)'
          ['@accession' => $accession, '@project_id' => $project_id]
        ),
        'tpps', 'tpps_generate_genotype_materialized_view',
        $args, $user->uid, 10, $includes, TRUE
      );
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'remove_study_markers_and_genotypes':
      global $user;
      $includes = [];
      // $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/markers_genotypes_utils');
      $args = [$accession];
      $jid = tripal_add_job("TPPS REMOVE all study markers and genotypes - $accession",
        'tpps', 'tpps_remove_all_markers_genotypes',
        $args, $user->uid, 10, $includes, TRUE
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
      // $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/markers_genotypes_utils');
      $includes[] = module_load_include('inc', 'tpps', 'includes/submissions');
      $args = [$accession];
      // Job to remove genotype information and features.
      $jid = tripal_add_job(
        "TPPS REMOVE all study markers and genotypes - $accession",
        'tpps', 'tpps_remove_all_markers_genotypes',
        $args, $user->uid, 10, $includes, TRUE
      );

      // Job to change the TGDR number to the new TGDR number.
      $args = [$accession, $new_accession];
      $jid = tripal_add_job("TPPS rename $accession to $new_accession",
        'tpps', 'tpps_change_tgdr_number',
        $args, $user->uid, 10, $includes, TRUE
      );

      // Now run the new import for the new accession TGDR number.
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = array($new_accession);
      $jid = tripal_add_job("TPPS Record Submission - $new_accession",
        'tpps', 'tpps_submit_all',
        $args, $state['submitting_uid'], 10, $includes, TRUE
      );
      $submission->state['job_id'] = $jid;
      $submission->save();
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'generate_popstruct_from_vcf':
      global $user;
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $page4_values = &$submission->state['saved_values'][TPPS_PAGE_4] ?? NULL;
      $snps_fieldset = 'SNPs';

      if (
        $vcf_file = tpps_file_load(
          ($page4_values['organism-1']['genotype'][$snps_fieldset]['vcf'] ?? NULL)
        )
      ) {
        $location = tpps_get_location($vcf_file->uri);
        $args = array($accession, $location);
        $jid = tripal_add_job("TPPS Generate PopStruct FROM VCF - $accession",
          'tpps', 'tpps_generate_popstruct',
          $args, $user->uid, 10, $includes, TRUE
        );
      }
      elseif (!empty($page4_values['organism-1']['genotype'][$snps_fieldset]['local_vcf'])) {
        $location = strip_tags($page4_values['organism-1']['genotype'][$snps_fieldset]['local_vcf']);
        $args = array($accession, $location);
        $jid = tripal_add_job("TPPS Generate PopStruct FROM VCF - $accession",
          'tpps', 'tpps_generate_popstruct',
          $args, $user->uid, 10, $includes, TRUE
        );
      }
      else {
        drupal_set_message(t("Could not find a VCF tied to organism-1, "
          . "are you sure you linked a VCF file?"
        ));
      }
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'refresh_tpps_cvterms_cache':
      global $user;
      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/cvterm_utils');
      $args = [];
      $jid = tripal_add_job('TPPS REFRESH CVTERMS CACHE', 'tpps',
        'tpps_cvterms_clear_cache', $args, $user->uid, 10, $includes, TRUE
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
      // dpm('tpps_submission_id = ' . $tpps_submission_id);
      if ($tpps_submission_id == NULL) {
        drupal_set_message(t('Could not find a TPPS SUBMISSION ID for this '
          . 'accession, contact administration'), 'error'
        );
        break;
      }

      if ($form_state['values']['CHANGE_TPPS_TYPE'] == 'tppsc') {
        // $state['saved_values'][TPPS_PAGE_1]['disable_vcf_import'] = 1;
        // Set the state tpps_type to tppsc.
        $submission->state['tpps_type'] = 'tppsc';

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
        $submission->state['tpps_type'] = 'tpps';

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
      drupal_set_message(t('Updated study TPPS type: @type.',
        ['@type' => $submission->state['tpps_type'])
      );
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'reject':
      $lang = user_preferred_language($owner);
      drupal_mail('tpps', 'user_rejected', $to, $lang, $params, $from, TRUE);
      $submission->state['status'] = 'Incomplete';
      $submission->save();

// @TODO Update.
      tpps_update_submission_info($accession, ['status' => $submission->state['status']]);

      drupal_set_message(t('Submission Rejected. Message has been sent to user.'));
      drupal_goto('<front>');
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'approve':
      module_load_include('php', 'tpps', 'forms/submit/submit_all');
      global $user;
      $uid = $user->uid;
      $submission->state['submitting_uid'] = $user->uid;
      $submission->state['status'] = 'Approved';

      $params['subject'] = "$type_label Submission Approved: "
        . "{$submission->state['saved_values'][TPPS_PAGE_1]['publication']['title']}";
      $params['accession'] = $submission->state['accession'];
      drupal_set_message(t('Submission Approved! Message has been sent to user.'));
      $lang = user_preferred_language(user_load_by_name($to));
      drupal_mail('tpps', 'user_approved', $to, $lang, $params, $from, TRUE);
      $submission->state['revised_files'] = $submission->state['revised_files'] ?? [];
      foreach ($submission->state['file_info'] as $files) {
        foreach ($files as $fid => $file_type) {
          if (!empty($form_state['values']["edit_file_{$fid}_check"])) {
            $submission->state['revised_files'][$fid]
              = $form_state['values']["edit_file_{$fid}_file"];
          }
        }
      }
      if (!empty($form_state['values']['study_location'])) {
        $submission->state['saved_values'][TPPS_PAGE_3]['study_location']['type']
          = $form_state['values']['study_location']['type'];
        for ($i = 1; $i <= $form_state['values']['study_location']['locations']['number']; $i++) {
          $submission->state['saved_values'][TPPS_PAGE_3]['study_location']['locations'][$i]
            = $form_state['values']['study_location']['locations'][$i];
        }
      }

      $includes = [];
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = [$accession];
      if ($submission->state['saved_values']['summarypage']['release']) {
        $jid = tripal_add_job("$type_label Record Submission - $accession",
          'tpps', 'tpps_submit_all',
          $args, $submission->state['submitting_uid'], 10, $includes, TRUE
        );
        $submission->state['job_id'] = $jid;
      }
      else {
        $date = $submission->state['saved_values']['summarypage']['release-date'];
        $time = strtotime("{$date['year']}-{$date['month']}-{$date['day']}");
        if (time() > $time) {
          $jid = tripal_add_job("$type_label Record Submission - $accession",
            'tpps', 'tpps_submit_all',
            $args, $submission->state['submitting_uid'], 10, $includes, TRUE
          );
          $submission->state['job_id'] = $jid;
        }
        else {
          $delayed_submissions = variable_get('tpps_delayed_submissions', []);
          $delayed_submissions[$accession] = $accession;
          variable_set('tpps_delayed_submissions', $delayed_submissions);
          $submission->state['status'] = 'Approved - Delayed Submission Release';
        }
      }
      $submission->save();


      tpps_update_submission_info($accession, ['status' => $state['status']]);

      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_date':
      $submission->state['saved_values']['summarypage']['release-date']
        = $form_state['values']['date'];
      $submission->save();
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_state_status':
      $submission->state['status'] = $form_state['values']['state-status'];
      $submission->save();
      tpps_update_submission_info($accession,
        ['status' => $submission->state['status']]
      );
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_study_view_role':
      $view_role = $form_state['values']['CHANGE_STUDY_VIEW_ROLE'];
      if ($view_role > 0) {
        $submission->state['study_view_role'] = $view_role;
        drupal_set_message(t(
          'Study view role has been set to @role',
          ['@role' => user_roles(TRUE)[$view_role]]
        ));
      }
      else {
        unset($submission->state['study_view_role']);
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
        $submission->state['alternative_accessions'] = $new_alt_acc;
        $submission->save();
      }
      break;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    case 'change_submission_owner':
      $new_user = user_load_by_mail($form_state['values']['new_owner']);
      $submission->state['submitting_uid'] = $new_user->uid;
      tpps_update_submission_info($accession, ['uid' => $new_user->uid]);
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
              $submission->state['title'] = $pub_title;
            }
          }
          if (isset($publication_entity->tpub__year['und'][0]['safe_value'])) {
            $pub_year = $publication_entity->tpub__year['und'][0]['safe_value'];
            if ($pub_year != "") {
              drupal_set_message('Found a valid publication year, synced with study.');
              $submission->state['pyear'] = $pub_year;
            }
          }
          if (isset($publication_entity->tpub__abstract['und'][0]['value'])) {
            $pub_abstract = $publication_entity->tpub__abstract['und'][0]['value'];
            if ($pub_abstract != "") {
              drupal_set_message('Found a valid publication abstract, synced with study.');
              $submission->state['abstract'] = $pub_abstract;
            }
          }
          if (isset($publication_entity->tpub__authors['und'][0]['value'])) {
            $pub_authors = $publication_entity->tpub__authors['und'][0]['value'];
            if ($pub_authors != "") {
              preg_match_all('/.[^,]+,*/', $pub_authors, $matches);
              if (count($matches) > 0) {
                $actual_matches = $matches[0];
                $filtered_matches = array();
                foreach ($actual_matches as $match) {
                  $match = str_replace(',','',$match);
                  $match = trim($match);
                  array_push($filtered_matches, $match);
                }
                if (count($filtered_matches) > 0) {
                  $submission->state['authors'] = $filtered_matches;
                  drupal_set_message(t('Found valid publication authors, '
                    . 'synced with study.')
                  );
                }
              }
            }
          }
          // Update Submision Interface.
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
    '#collapsed' => FALSE,
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
