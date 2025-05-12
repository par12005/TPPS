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
 * Shows lists of studies with statue 'Pending', 'Approved', and 'Incomplete'.
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
  $submission_list = SubmissionCache::get($reset);

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
      l(t('Clear "tpps" cache bin'), 'admin/config/tpps/cache', $params),
    ],
  ]);
  $form['general_tasks'] = [
    '#type' => 'fieldset',
    '#title' => t('General tasks'),
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
      t('Tags'),
      t('Actions'),
    ],
    'approved' => [
      t('Accession Number'),
      t('Submitting User'),
      t('Title'),
      t('Status'),
      t('Tags'),
      t('Actions'),
    ],
    'incomplete' => [
      t('Accession Number'),
      t('Submitting User'),
      t('Title'),
      t('Stage'),
      t('Last Updated'),
      t('Tags'),
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
      tpps_form_add_fieldset_collapsed_state($form, $form_state, $status_name);
      $table = theme('table', [
        'header' => $header[$status_name],
        'rows' => $status_list,
        'caption' => $title[$status_name],
        'attributes' => ['class' => ['view', 'tpps_table', 'sticky-caption']],
      ]);
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
    'table' => [
      '#markup' => theme('table', [
        'caption' => $title,
        'header' => ['Accession'],
        'rows' => $output['resubmit'],
        'attributes' => ['class' => ['view', 'tpps_table', 'sticky-caption']],
      ]),
    ],
  ];
  tpps_form_add_fieldset_collapsed_state($form, $form_state, 'resubmit');

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // 'New Species' table.
  $cid = $key = 'tpps_admin_panel_cache_new_species_list';
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
      while (($org = $query->fetchObject())) {
        $name = $org->genus . ' ' . $org->species;
        $id = chado_get_record_entity_by_bundle($org_bundle, $org->organism_id);
        $rows[] = [
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
              'header' => [t('Name')],
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
          ),
        ],
      ];
      tpps_form_add_fieldset_collapsed_state($form, $form_state, 'new_species');
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
  $action_list = [
    l(t('Edit'), 'tppsc/' . $accession),
    l(t('View'), 'tpps/submission/' . $accession . '/view'),
    l(t('State JSON'), 'api/submission/' . $accession . '/view/state'),
    l(t('Shared State JSON'), 'api/submission/' . $accession . '/view/shared_state'),
    l(t('Export'), 'tpps/submission/' . $accession . '/export'),
    l(t('Files'), 'tpps-admin-panel/file-diagnostics/' . $accession),
    l(t('Rename local VCF'), 'tpps/submission/' . $accession . '/vcf/local/rename'),
  ];
  // To add items use $action_list and recreate element.
  $actions = theme('item_list', [
    'items' => $action_list,
    'attributes' => ['class' => ['admin-panel-action-list']],
  ]);



  switch ($row['status']) {
    case TPPS_SUBMISSION_STATUS_PENDING_APPROVAL:
      $output['pending'][$number] = [
        $view_link,
        theme('submission_submitter', $row),
        $row['publication']['title'],
        theme('submission_date', ['date' => $row['date']['completed']]),
        theme('submission_tag_list', ['tags' => $row['tags']]),
        $actions,
      ];
      break;

    //case TPPS_SUBMISSION_STATUS_APPROVED:
    //  $status_label = $row['date']['loaded'] ? "Approved - load completed on "
    //      . date("F j, Y, \a\\t g:i a", $row['date']['loaded'])
    //    : TPPS_SUBMISSION_STATUS_APPROVED;
    //  if (!empty($row['date']['loaded'])) {
    //    $days_since_load = (time() - $submission->getLoadTime()) / (60 * 60 * 24);
    //    $unpublished_threshold = variable_get('tpps_unpublished_days_threshold', 180);
    //    $pub_status = $submission->state['saved_values'][TPPS_PAGE_1]['publication']['status'] ?? NULL;
    //    if (
    //      !empty($pub_status)
    //      and $pub_status != 'Published'
    //      and $days_since_load >= $unpublished_threshold
    //    ) {
    //      $owner = $submitting_user;
    //      $contact_bundle = tripal_load_bundle_entity(
    //        ['label' => 'Tripal Contact Profile']
    //      );
    //      //$owner_mail = tpps_get_user_email($uid);
    //      $owner = "$submitting_user ($owner_mail)";

    //      // If Tripal Contact Profile is available, we want to link to the
    //      // profile of the owner instead of just displaying the name.
    //      if ($contact_bundle) {
    //        $query = new EntityFieldQuery();
    //        $results = $query->entityCondition('entity_type', 'TripalEntity')
    //          ->entityCondition('bundle', $contact_bundle->name)
    //          ->fieldCondition('local__email', 'value', $owner_mail)
    //          ->range(0, 1)
    //          ->execute();
    //        // [VS]
    //        if (!empty($results['TripalEntity'])) {
    //          // Commented out because there is a lot of warning messages
    //          // shown at page.
    //          //
    //          //  $entity = current(
    //          //    array_reverse(
    //          //      entity_load(
    //          //        'TripalEntity',
    //          //        array_keys($results['TripalEntity'])
    //          //      )
    //          //    )
    //          //  );
    //          //  $owner = "<a href=\"$base_url/TripalContactProfile/"
    //          //    . "{$entity->id}\">$submitting_user</a>";
    //        }
    //        // [/VS]
    //      }
    //      if (tpps_access('view own tpps submission', $accession)) {
    //        $new_action_list = $action_list;
    //        $new_action_list[] = l(t('Edit publication information'),
    //          'tpps/' . $accession . '/edit-publication'
    //        );
    //      }
    //      $row = [
    //        $view_link,
    //        date("F j, Y", $submission->getLoadTime())
    //          . " (" . round($days_since_load) . " days ago)",
    //        $pub_status,
    //        $owner,
    //        theme('item_list', [
    //          'items' => $new_action_list,
    //          'attributes' => ['class' => ['admin-panel-action-list']],
    //        ]),
    //      ];
    //      $output['unpublished_old'][$number] = $row;
    //    }
    //  }

    case TPPS_SUBMISSION_STATUS_SUBMISSION_JOB_RUNNING:
    case TPPS_SUBMISSION_STATUS_APPROVED_DELAYED:

      if ($row['status'] == TPPS_SUBMISSION_STATUS_SUBMISSION_JOB_RUNNING) {
        $status_label = $status_label ?? (
          !empty($row['date']['approved'])
          ? ("Submission Job Running - job started on "
            . date("F j, Y, \a\t g:i a", $row['date']['approved']))
          : TPPS_SUBMISSION_STATUS_SUBMISSION_JOB_RUNNING
        );
      }
      elseif ($row['status'] == TPPS_SUBMISSION_STATUS_APPROVED_DELAYED) {
        $release = $row['date']['release'];
        $release = strtotime("{$release['day']}-{$release['month']}-{$release['year']}");
        $status_label = "Approved - Delayed Submission Release on "
          . date("F j, Y", $release);
      }
      $output['approved'][$number] = [
        $view_link,
        theme('submission_submitter', $row),
        $row['publication']['title'],
        $status_label,
        theme('submission_tag_list', ['tags' => $row['tags']]),
        $actions,
      ];
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
        $view_link,
        theme('submission_submitter', $row),
        $row['publication']['title'] ?? t('Title not provided yet'),
        check_plain($stage),
        theme('submission_date', ['date' => $row['date']['updated']]),
        theme('submission_tag_list', ['tags' => $row['tags']]),
        $actions,
      ];
      break;
  }
}

function ___old($context) {

  $submission = $context['submission'] ?? NULL;
  if (empty($submission) && !empty($context['accession'])) {
    $submission = new Submission($context['accession']);
  }
  $reset = $context['reset'] ?? FALSE;
  $output = &$context['ouput'];

  $data = [];
  $submitting_user_cache = [];
  $debug_mode = FALSE;
  $accession = $submission->accession;
    // 0.03 msec

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $uid = $submission->uid ?? NULL;

  if (empty($submitting_user_cache[$uid])) {
    $mail = $submission->getSubmitterMail();
    $query = db_select('chado.contact', 'c');
    $query->join('chado.contactprop', 'cp', 'cp.contact_id = c.contact_id');
    $query->condition('cp.value', $mail);
    $query->condition('cp.type_id', tpps_load_cvterm('email')->cvterm_id);

    $query->fields('c', array('name'));
    $query->range(0, 1);
    $query = $query->execute();
    $name = $query->fetchObject()->name ?? NULL;

    $submitting_user_cache[$uid] = $name ?? $mail;
  }
  $submitting_user = $submitting_user_cache[$uid] ?? NULL;
  $view_link = l($accession, 'tpps-admin-panel/' . $accession);
  $action_list = [
    l(t('Edit'), 'tppsc/' . $accession),
    l(t('View'), 'tpps/submission/' . $accession . '/view'),
    l(t('State JSON'), 'api/submission/' . $accession . '/view/state'),
    l(t('Shared State JSON'), 'api/submission/' . $accession . '/view/shared_state'),
    l(t('Export'), 'tpps/submission/' . $accession . '/export'),
    l(t('Files'), 'tpps-admin-panel/file-diagnostics/' . $accession),
  ];
  // To add items use $action_list and recreate element.
  $actions = theme('item_list', [
    'items' => $action_list,
    'attributes' => ['class' => ['admin-panel-action-list']],
  ]);
  if ($submission->doesExist()) {
    switch ($submission->status) {
      case TPPS_SUBMISSION_STATUS_PENDING_APPROVAL:
        $row = [
          $view_link,
          $submitting_user,
          $submission->state['saved_values'][TPPS_PAGE_1]['publication']['title'],
          !empty($submission->state['completed'])
          ? date("F j, Y, g:i a", $submission->state['completed']) : "Unknown",
          tpps_show_tags(tpps_submission_get_tags($accession)),
          $actions,
        ];
        $output['pending'][(int) substr($accession, 4)] = $row;
        break;

      case TPPS_SUBMISSION_STATUS_APPROVED:
        $status_label = $submission->isLoaded() ? "Approved - load completed on "
            . date("F j, Y, \a\\t g:i a", $submission->getLoadTime())
          : TPPS_SUBMISSION_STATUS_APPROVED;
        if ($submission->isLoaded()) {
          $days_since_load = (time() - $submission->getLoadTime()) / (60 * 60 * 24);
          $unpublished_threshold = variable_get('tpps_unpublished_days_threshold', 180);
          $pub_status = $submission->state['saved_values'][TPPS_PAGE_1]['publication']['status'] ?? NULL;
          if (
            !empty($pub_status)
            and $pub_status != 'Published'
            and $days_since_load >= $unpublished_threshold
          ) {
            $owner = $submitting_user;
            $contact_bundle = tripal_load_bundle_entity(
              ['label' => 'Tripal Contact Profile']
            );
            //$owner_mail = tpps_get_user_email($uid);
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
            if (tpps_access('view own tpps submission', $accession)) {
              $new_action_list = $action_list;
              $new_action_list[] = l(t('Edit publication information'),
                'tpps/' . $accession . '/edit-publication'
              );
            }
            $row = [
              $view_link,
              date("F j, Y", $submission->getLoadTime())
                . " (" . round($days_since_load) . " days ago)",
              $pub_status,
              $owner,
              theme('item_list', [
                'items' => $new_action_list,
                'attributes' => ['class' => ['admin-panel-action-list']],
              ]),
            ];
            $output['unpublished_old'][(int) substr($accession, 4)] = $row;
          }
        }

      case TPPS_SUBMISSION_STATUS_SUBMISSION_JOB_RUNNING:
        $status_label = $status_label ?? (
          !empty($submission->state['approved'])
          ? ("Submission Job Running - job started on "
            . date("F j, Y, \a\t g:i a", $submission->state['approved']))
          : TPPS_SUBMISSION_STATUS_SUBMISSION_JOB_RUNNING
        );

      case TPPS_SUBMISSION_STATUS_APPROVED_DELAYED:
        if (empty($status_label)) {
          $release = $submission->state['saved_values']['summarypage']['release-date'] ?? NULL;
          $release = strtotime("{$release['day']}-{$release['month']}-{$release['year']}");
          $status_label = "Approved - Delayed Submission Release on " . date("F j, Y", $release);
        }
        $row = [
          $view_link,
          $submitting_user,
          $submission->state['saved_values'][TPPS_PAGE_1]['publication']['title'],
          $status_label,
          tpps_show_tags(tpps_submission_get_tags($accession)),
          $actions,
        ];
        $output['approved'][(int) substr($accession, 4)] = $row;
        break;

      default:
        switch ($submission->state['stage'] ?? NULL) {
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

        $date = (
          !empty($submission->state['updated'])
          ? date("F j, Y, g:i a", $submission->state['updated'])
          : t('Unknown')
        );
        $title = $submission->state['saved_values'][TPPS_PAGE_1]['publication']['title']
          ?? t('Title not provided yet');
        $output['incomplete'][(int) substr($accession, 4)] = [
          $view_link,
          $submitting_user,
          $title,
          $stage,
          $date,
          tpps_show_tags(tpps_submission_get_tags($accession)),
          $actions,
        ];
        break;
    }
  }
  return $output;
}

