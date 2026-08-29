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
    tpps_admin_panel_show_all($form, $form_state);
  }
  else {
    // Shows single study.
    tpps_manage_submission_form($form, $form_state, $accession);
  }
  $form['#attributes']['class'][] = 'tpps-admin-panel';
  tpps_add_css_js('main', $form);
  return $form;
}

/**
 * Shows lists of studies with different statuses.
 *
 * List of statuses:
 * - 'Pending',
 * - Submission::STATUS_APPROVED,
 * - Submission::STATUS_INCOMPLETE.
 *
 * @param array $form
 *   The form element of the TPPS admin panel page.
 * @param array $form_state
 *   Drupal Form API State array.
 * @param bool $reset
 *   Flag is cache must be resetted. Default if FALSE.
 */
function tpps_admin_panel_show_all(array &$form, array $form_state, $reset = FALSE) {
  global $base_url;

  $reset = $_GET['reset'] ?? $reset;
  tpps_admin_panel_reports($form);
  $submission_list = SubmissionCache::getAll($reset);

  $output = [
    'unpublished_old' => [],
    'pending' => [],
    'approved' => [],
    'incomplete' => [],
    'resubmit' => [],
  ];
  foreach ($submission_list as $accession => $item) {
    tpps_report_admin_panel_theme_single_row($accession, $item, $output);
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // List of general tasks.
  $params = ['attributes' => ['target' => '_blank']];
  $task_list = theme('item_list', [
    'items' => [
      l(t('Refresh all genotype materialized views'),
        'tpps-admin-panel/refresh-genotypes-materialized-views'),
      l(t('TPPS Submission Tools'), 'tpps/submission', $params),
      l(t('NCBI Taxonomy Id Batch Updates'), 'admin/config/tpps/ncbi_batch', $params),
      l(t('Clear "TPPS" or "TPPS Submission" cache bin'), 'admin/config/tpps/cache', $params),
      l(t('Create new Phenotype Synonym'), 'admin/config/tpps/synonym', $params),
    ],
  ]);
  $form['general_tasks'] = [
    '#type' => 'fieldset',
    '#title' => t('General Tasks'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    'tasks' => ['#markup' => $task_list],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show lists.
  // Table header.
  $header = [
    'unpublished_old' => [
      t('Accession Number'),
      t('Approval date'),
      t('Publication Status'),
      t('Submission Owner'),
      t('Actions'),
    ],
    'pending' => [
      t('Accession Number'),
      t('Submitting User'),
      t('Title'),
      t('Date Submitted'),
      t('Actions'),
    ],
    'approved' => [
      t('Accession Number'),
      t('Submitting User'),
      t('Title'),
      t('Status'),
      t('Actions'),
    ],
    'incomplete' => [
      t('Accession Number'),
      t('Submitting User'),
      t('Title'),
      t('Stage'),
      t('Last Updated'),
      t('Actions'),
    ],
  ];

  // Fieldset title.
  $title = [
    'unpublished_old' => t('Unpublished approved TPPS submissions'),
    'pending' => t('Pending TPPS submissions'),
    'approved' => t('Approved TPPS submissions'),
    'incomplete' => t('Incomplete TPPS submissions'),
  ];
  foreach ($output as $status_name => $status_list) {
    if ($status_name == 'resubmit') {
      continue;
    }
    if (!empty($status_list)) {
      krsort($status_list);
      $form[$status_name] = [
        '#type' => 'fieldset',
        '#title' => $title[$status_name],
        '#collapsible' => TRUE,
      ];
      TppsForm::addFieldsetCollapsedState($form, $form_state, $status_name);
      // @TODO Minor. Use Report::print().
      $table = theme('table', [
        'header' => $header[$status_name],
        'rows' => $status_list,
        'caption' => $title[$status_name],
        'attributes' => ['class' => ['view', 'tpps_table', 'sticky-caption']],
      ]) . SubmissionCache::showClearCacheWidget();
      $form[$status_name]['table'] = ['#markup' => $table];
    }
  }
  tpps_add_css_js('sticky_caption', $form);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Old TGDR Submissions to be resubmitted.
  $title = t('Old TGDR Submissions to be resubmitted');
  $form['resubmit'] = [
    '#type' => 'fieldset',
    '#title' => "<img src='$base_url/misc/message-16-warning.png'> " . $title,
    '#collapsible' => TRUE,
    // @TODO Minor. Use Report::print().
    'table' => [
      '#markup' => theme('table', [
        'caption' => $title,
        'header' => ['Accession'],
        'rows' => $output['resubmit'],
        'attributes' => ['class' => ['view', 'tpps_table', 'sticky-caption']],
      ]) . SubmissionCache::showClearCacheWidget(),
    ],
  ];
  TppsForm::addFieldsetCollapsedState($form, $form_state, 'resubmit');

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // 'New Species' table.
  $cid = $key = 'tpps_admin_panel_cache_new_species_list';
  // @TODO Minor. Create 'TppsCache' class and move this code there.
  // See SubmissionCache.
  if (($_GET['reset'] ?? NULL) == $cid) {
    $message = t('Cleared "@cid" cache id in "tpps" cache bin.', ['@cid' => $cid]);
    drupal_set_message($message);
    cache_set($cid, NULL, TPPS_CACHE_BIN ?? 'cache');
    drupal_goto($_GET['q']);
  }

  $cache = cache_get($cid, TPPS_CACHE_BIN ?? 'cache');
  $db = chado_get_db(['name' => 'NCBI Taxonomy']);
  if (!empty($db)) {
    if (
      $reset || empty($cache) || empty($cache->data[$key])
      || !variable_get($key, FALSE)
    ) {
      $rows = [];
      $query = db_select('chado.organism', 'o');
      $query->fields('o', ['organism_id', 'genus', 'species']);

      $query_e = db_select('chado.organism_dbxref', 'odb');
      $query_e->join('chado.dbxref', 'd', 'd.dbxref_id = odb.dbxref_id');
      $query_e->condition('d.db_id', $db->db_id)
        ->where('odb.organism_id = o.organism_id');

      $query->notExists($query_e);
      $query = $query->execute();

      $org_bundle = tripal_load_bundle_entity(['label' => 'Organism']);
      $i = 0;
      while (($org = $query->fetchObject())) {
        $name = $org->genus . ' ' . $org->species;
        $id = chado_get_record_entity_by_bundle($org_bundle, $org->organism_id);
        $rows[] = [
          ++$i,
          empty($id) ? $name : l($name, '/bio_data/' . $id . '/edit',
            ['attributes' => ['target' => '_blank']]),
        ];
      }

      $cache->data[$key] = $rows;
      cache_set($cid, $cache->data, TPPS_CACHE_BIN ?? 'cache');
    }
    if (!empty($cache->data[$key])) {
      $title = t('New Species');
      $form['new_species'] = [
        '#type' => 'fieldset',
        '#title' => $title,
        '#description' => t('The species listed below likely need to be '
          . 'updated, because they do not have NCBI Taxonomy identifiers '
          . 'in the database.'),
        '#collapsible' => TRUE,
        'table' => [
          '#markup' => theme('table',
            [
              'header' => ['#', t('Name')],
              'rows' => $cache->data[$key],
              'sticky' => TRUE,
              'attributes' => [
                'class' => ['view', 'tpps_table'],
                'id' => 'new_species',
              ],
              'caption' => $title,
              'attributes' => [
                'class' => ['view', 'tpps_table', 'sticky-caption']
              ],
            ]
          )
          // Add link to clear cache.
          // @TODO Minor. Create 'TppsCache' class and move this code there.
          // See SubmissionCache.
          . theme('item_list', [
            'items' => [
              t('Report was cached. <a href="@url">Update</a> to get most '
                . 'recent changes.',
                ['@url' => url($_GET['q'], ['query' => ['reset' => $cid]])]
              ),
            ]
          ]),
        ],
      ];
      TppsForm::addFieldsetCollapsedState($form, $form_state, 'new_species');
    }
  }
}

/**
 * Adds fieldset with links to custom report pages.
 *
 * @param array $form
 *   The form element of the TPPS admin panel page.
 */
function tpps_admin_panel_reports(array &$form) {
  $form['report_menu'] = [
    '#type' => 'fieldset',
    '#title' => t('TPPS Reports'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];
  $form['report_menu']['table'] = [
    '#markup' => Report::printOverviewPage($admin_panel = TRUE),
  ];
}

/**
 * Themes single study row.
 *
 * @param mixed $accession
 *   Study accession.
 * @param array $row
 *   SubmissionCache record for single submission.
 * @param array $output
 *   Will contain output.
 */
function tpps_report_admin_panel_theme_single_row($accession, array $row, array &$output) {
  $number = (int) str_replace('TGDR', '', $accession);
  if (is_array($row) && empty($row)) {
    // Fill list of studies which must be resubmitted.
    $output['resubmit'][$number] = [$accession];
    return;
  }

  $view_link = theme('submission_accession_link', [
    'accession' => $accession,
    'path' => 'tpps-admin-panel/' . $accession,
  ]);

  // Build list of Submission Tags.
  $tags = [];
  foreach ($row['tags'] as $tag_id) {
    $tags[] = SubmissionTag::load($tag_id);
  }

  switch ($row['status']) {
    case Submission::STATUS_PENDING_APPROVAL:
      $output['pending'][$number] = [
        $view_link . theme('submission_tag_list', ['tags' => $tags]),
        theme('submission_submitter', $row),
        $row['publication']['title'],
        theme('submission_date', ['date' => $row['date']['completed']]),
        theme('submission_actions', [
          'accession' => $accession,
          'deposit_id' => $row['zenodo']['deposit_id'] ?? NULL,
          'edit_publication_information' => FALSE,
        ]),
      ];
      break;

    case Submission::STATUS_APPROVED:
    case Submission::STATUS_SUBMISSION_JOB_RUNNING:
    case Submission::STATUS_APPROVED_DELAYED:

      if ($row['status'] == Submission::STATUS_APPROVED) {
        $status_label = !empty($row['date']['loaded'])
          ? "Approved - load completed on " . date("F j, Y, \a\\t g:i a",
          $row['date']['loaded']) : Submission::STATUS_APPROVED;
      }
      elseif ($row['status'] == Submission::STATUS_SUBMISSION_JOB_RUNNING) {
        $status_label = $status_label ?? (
          !empty($row['date']['approved'])
          ? ("Submission Job Running - job started on "
            . date("F j, Y, \a\t g:i a", $row['date']['approved']))
          : Submission::STATUS_SUBMISSION_JOB_RUNNING
        );
      }
      elseif ($row['status'] == Submission::STATUS_APPROVED_DELAYED) {
        $release = $row['date']['release'];
        $release = strtotime("{$release['day']}-{$release['month']}-{$release['year']}");
        $status_label = "Approved - Delayed Submission Release on "
          . date("F j, Y", $release);
      }

      if (!empty($row['date']['loaded'])) {
        $days_since_load = (time() - $row['date']['loaded']) / (60 * 60 * 24);
        $unpublished_threshold = variable_get('tpps_unpublished_days_threshold', 180);
        $pub_status = $row['publication']['status'] ?? NULL;
        if (
          !empty($pub_status) && ($pub_status) != 'Published'
          && $days_since_load >= $unpublished_threshold
        ) {
          $output['unpublished_old'][$number] = [
            $view_link,
            date("F j, Y", $row['date']['loaded']) . " (" . round($days_since_load) . " days ago)",
            $pub_status,
            theme('submission_submitter', $row),
            theme('submission_actions', [
              'accession' => $accession,
              'deposit_id' => $row['zenodo']['deposit_id'] ?? NULL,
              'edit_publication_information' => tpps_access('view own tpps submission', $accession),
            ]),
          ];
        }
        else {
          $output['approved'][$number] = [
            $view_link . theme('submission_tag_list', ['tags' => $tags]),
            theme('submission_submitter', $row),
            $row['publication']['title'],
            $status_label,
            theme('submission_actions', [
              'accession' => $accession,
              'deposit_id' => $row['zenodo']['deposit_id'] ?? NULL,
              'edit_publication_information' => FALSE,
            ]),
          ];
        }
      }
      else {
        $output['approved'][$number] = [
          $view_link . theme('submission_tag_list', ['tags' => $tags]),
          theme('submission_submitter', $row),
          $row['publication']['title'],
          $status_label,
          theme('submission_actions', [
            'accession' => $accession,
            'deposit_id' => $row['zenodo']['deposit_id'] ?? NULL,
            'edit_publication_information' => FALSE,
          ]),
        ];
      }
      break;

    default:
      switch ($row['stage'] ?? NULL) {
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

      $output['incomplete'][$number] = [
        $view_link . theme('submission_tag_list', ['tags' => $tags]),
        theme('submission_submitter', $row),
        $row['publication']['title'] ?? t('Title not provided yet'),
        check_plain($stage),
        theme('submission_date', ['date' => $row['date']['updated']]),
        theme('submission_actions', [
          'accession' => $accession,
          'deposit_id' => $row['zenodo']['deposit_id'] ?? NULL,
          'edit_publication_information' => FALSE,
        ]),
      ];
      break;
  }
}
