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
 * database. If the submission includes CartograTree layers with environmental
 * parameters, the administrator will need to select the kind of parameter the
 * user has selected - an attr_id, or a cvterm. This will be important when the
 * submission is recording the environmental data of the trees.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 *
 * @global stdClass $user
 *   The user accessing the administrative panel.
 * @global string $base_url
 *   The base url of the site.
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
  $submission = tpps_load_submission($accession, False);
  $status = $submission->status;
  $submission_state = unserialize($submission->submission_state);
  if (empty($submission_state['status'])) {
    $submission_state['status'] = $status;
    tpps_update_submission($submission_state);
  }
  $options = array();
  $display = l(t("Back to TPPS Admin Panel"), "$base_url/tpps-admin-panel");

  if ($status == "Pending Approval") {
    $options['files'] = array(
      'revision_destination' => TRUE,
    );

    foreach ($submission_state['file_info'] as $page => $files) {
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

  $form['accession'] = array(
    '#type' => 'hidden',
    '#value' => $accession,
  );

  $form['form_table'] = array(
    '#markup' => $display,
  );

  if ($status == "Pending Approval") {

    $form['params'] = array(
      '#type' => 'fieldset',
      '#title' => 'Select Environmental parameter types:',
      '#tree' => TRUE,
      '#description' => '',
    );

    $orgamism_num = $submission_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
    $show_layers = FALSE;
    for ($i = 1; $i <= $orgamism_num; $i++) {
      if (!empty($submission_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['environment']['use_layers'])) {
        foreach ($submission_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['environment']['env_layers'] as $layer => $layer_id) {
          if (!empty($layer_id)) {
            foreach ($submission_state['saved_values'][TPPS_PAGE_4]["organism-$i"]['environment']['env_params'][$layer] as $param_name => $param_id) {
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
                      'attr_id' => 'attr_id',
                      'cvterm' => 'cvterm',
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

  $form['state-status'] = array(
    '#type' => 'select',
    '#title' => t('Change state status'),
    '#description' => t('Warning: This feature is experimental and may cause unforseen issues. Please do not change the status of this submission unless you are willing to risk the loss of existing data. The current status of the submission is @status.', array('@status' => $status)),
    '#options' => array(
      'Incomplete' => 'Incomplete',
      'Pending Approval' => 'Pending Approval',
      'Submission Job Running' => 'Submission Job Running',
      'Approved' => 'Approved',
      'Approved - Delayed Submission Release' => 'Approved - Delayed Submission Release',
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
      $query->range(0,1);
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
          );
          $pending[(int) substr($state['accession'], 4)] = $row;
          break;

        case 'Approved':
          $status_label = !empty($state['loaded']) ? "Approved - load completed on " . date("F j, Y, \a\t g:i a", $state['loaded']) : "Approved";
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
              $stage = "Tree Accession";
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
          );
          $incomplete[(int) substr($state['accession'], 4)] = $row;
          break;
      }
    }
  }

  ksort($pending);
  ksort($approved);
  ksort($incomplete);

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
    'Submitting User',
    'Title',
    'Date Submitted',
  );
  $vars['rows'] = $pending;
  $pending_table = theme_table($vars);

  $vars['header'] = array(
    'Accession Number',
    'Submitting User',
    'Title',
    'Status',
  );
  $vars['rows'] = $approved;
  $approved_table = theme_table($vars);

  $vars['header'] = array(
    'Accession Number',
    'Submitting User',
    'Title',
    'Stage',
    'Last Updated',
  );
  $vars['rows'] = $incomplete;
  $incomplete_table = theme_table($vars);

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

      $form['new_species']['#markup'] = "<div class='tpps_table'><label for='new_species'>New Species: the species listed below likely need to be updated, because they do not have NCBI Taxonomy identifiers in the database.</label>" . theme_table($vars) . "</div>";
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
      form_set_error('reject-reason', 'Please explain why the submission was rejected.');
    }

    if ($form_state['triggering_element']['#value'] == 'Approve') {
      $accession = $form_state['values']['accession'];
      $state = tpps_load_submission($accession);
      foreach ($state['file_info'] as $page => $files) {
        foreach ($files as $fid => $file_type) {
          if (!empty($form_state['values']["edit_file_{$fid}_check"]) and empty($form_state['values']["edit_file_{$fid}_file"])) {
            form_set_error("edit_file_{$fid}_file", 'Please upload a revised version fo the user-provided file.');
          }
          if (!empty($form_state['values']["edit_file_{$fid}_file"])) {
            $file = file_load($form_state['values']["edit_file_{$fid}_file"]);
            file_usage_add($file, 'tpps', 'tpps_project', substr($accession, 4));
          }
        }
      }
    }

    if ($form_state['triggering_element']['#value'] == 'Save Alternative Accessions') {
      $alt_acc = explode(',', $form_state['values']['alternative_accessions']);
      foreach ($alt_acc as $acc) {
        if (!preg_match('/^TGDR\d{3,}$/', $acc)) {
          form_set_error('alternative_accessions', "The accession, $acc is not a valid TGDR### accession number.");
        }
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
  $user = user_load($submission->uid);
  $to = $user->mail;
  $state = unserialize($submission->submission_state);
  $state['admin_comments'] = $form_state['values']['admin-comments'] ?? NULL;
  $params = array();

  $from = variable_get('site_mail', '');
  $params['subject'] = "$type_label Submission Rejected: {$state['saved_values'][TPPS_PAGE_1]['publication']['title']}";
  $params['uid'] = $user->uid;
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
    case 'Reject':
      drupal_mail($type, 'user_rejected', $to, user_preferred_language($user), $params, $from, TRUE);
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

      $state['revised_files'] = array();
      foreach ($state['file_info'] as $page => $files) {
        foreach ($files as $fid => $file_type) {
          if (!empty($form_state['values']["edit_file_{$fid}_check"])) {
            $state['revised_files'][$fid] = $form_state['values']["edit_file_{$fid}_file"];
          }
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

    default:
      break;
  }
}
