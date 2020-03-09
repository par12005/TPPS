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

  global $user;
  global $base_url;

  if (empty($accession)) {

    $submissions = tpps_load_submission_multiple(array(
      'status' => array(
        'Pending Approval',
        'Submission Job Running',
        'Approved',
        'Approved - Delayed Submission Release',
      ),
    ), FALSE);

    $pending = array();
    $approved = array();
    $contact_bundle = tripal_load_bundle_entity(array('label' => 'Tripal Contact Profile'));
    foreach ($submissions as $submission) {
      $state = unserialize($submission->submission_state);
      $mail = user_load($submission->uid)->mail;
      if ($contact_bundle) {
        $query = new EntityFieldQuery();
        $results = $query->entityCondition('entity_type', 'TripalEntity')
          ->entityCondition('bundle', $contact_bundle->name)
          ->fieldCondition('local__email', 'value', $mail)
          ->range(0, 1)
          ->execute();
        if (!empty($results['TripalEntity'])) {
          $id = current($results['TripalEntity'])->id;
          $entity = current(tripal_load_entity('TripalEntity', array($id))) ?? NULL;
        }
      }
      if (!empty($state)) {
        $row = array(
          l($state['accession'], "$base_url/tpps-admin-panel/{$state['accession']}"),
          $entity->title ?? NULL,
          $state['saved_values'][TPPS_PAGE_1]['publication']['title'],
          $state['status'],
        );
        if ($state['status'] == 'Pending Approval') {
          $pending[(int) substr($state['accession'], 4)] = $row;
        }
        else {
          $approved[(int) substr($state['accession'], 4)] = $row;
        }
      }
    }
    ksort($pending);
    ksort($approved);
    $rows = array_merge($pending, $approved);

    $headers = array(
      'Accession Number',
      'Submitting User',
      'Title',
      'Status',
    );

    $vars = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'class' => array('view', 'tpps_table'),
        'id' => 'tpps_table_display',
      ),
      'caption' => '',
      'colgroups' => NULL,
      'sticky' => FALSE,
      'empty' => '',
    );

    $form['#attributes'] = array('class' => array('hide-me'));
    $form['#suffix'] = "<div class='tpps_table'><label for='tpps_table_display'>Completed TPPS Submissions</label>" . theme_table($vars) . "</div>";

    $tpps_new_orgs = variable_get('tpps_new_organisms', NULL);
    $db = chado_get_db(array('name' => 'NCBI Taxonomy'));
    if (!empty($db)) {
      $rows = array();
      $query = db_select('chado.organism', 'o');
      $query->fields('o', array('genus', 'species'));

      $query_e = db_select('chado.organism_dbxref', 'odb');
      $query_e->join('chado.dbxref', 'd', 'd.dbxref_id = odb.dbxref_id');
      $query_e->condition('d.db_id', $db->db_id)
        ->where('odb.organism_id = o.organism_id');
      $query->notExists($query_e);
      $query = $query->execute();

      while (($org = $query->fetchObject())) {
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

        $form['#suffix'] .= "<div class='tpps_table'><label for='new_species'>New Species: the species listed below likely need to be updated, because they do not have NCBI Taxonomy identifiers in the database.</label>" . theme_table($vars) . "</div>";
      }
      variable_set('tpps_new_organisms', $tpps_new_orgs);
    }
  }
  else {
    $submission = tpps_load_submission($accession, False);
    $status = $submission->status;
    $submission_state = unserialize($submission->submission_state);
    $submission_state['status'] = $status;
    tpps_update_submission($submission_state);
    $display = l(t("Back to TPPS Admin Panel"), "$base_url/tpps-admin-panel");
    $display .= tpps_table_display($submission_state);

    $form['form_table'] = array(
      '#type' => 'hidden',
      '#value' => $accession,
      '#suffix' => $display,
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

    $date = $submission_state['saved_values']['summarypage']['release-date'] ?? NULL;
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

  drupal_add_js(drupal_get_path('module', 'tpps') . TPPS_JS_PATH);
  drupal_add_css(drupal_get_path('module', 'tpps') . TPPS_CSS_PATH);

  return $form;
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

  $accession = $form_state['values']['form_table'];
  $submission = tpps_load_submission($accession, FALSE);
  $user = user_load($submission->uid);
  $to = $user->mail;
  $state = unserialize($submission->submission_state);
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
      unset($state['status']);
      tpps_update_submission($state, array('status' => 'Incomplete'));
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

      $includes = array();
      $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
      $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
      $args = array($accession);
      if ($state['saved_values']['summarypage']['release']) {
        $jid = tripal_add_job("$type_label Record Submission - $accession", 'tpps', 'tpps_submit_all', $args, $state['submitting_uid'], 10, $includes, TRUE);
        $state['job_id'] = $jid;
        tpps_update_submission($state);
      }
      else {
        $date = $state['saved_values']['summarypage']['release-date'];
        $time = strtotime("{$date['year']}-{$date['month']}-{$date['day']}");
        if (time() > $time) {
          $jid = tripal_add_job("$type_label Record Submission - $accession", 'tpps', 'tpps_submit_all', $args, $state['submitting_uid'], 10, $includes, TRUE);
          $state['job_id'] = $jid;
          tpps_update_submission($state);
        }
        else {
          $delayed_submissions = variable_get('tpps_delayed_submissions', array());
          $delayed_submissions[$accession] = $accession;
          variable_set('tpps_delayed_submissions', $delayed_submissions);
          $state['status'] = 'Approved - Delayed Submission Release';
          tpps_update_submission($state);
        }
      }
      break;

    case 'Change Date':
      $state['saved_values']['summarypage']['release-date'] = $form_state['values']['date'];
      tpps_update_submission($state);
      break;

    case 'Change Status':
      $state['status'] = $form_state['values']['state-status'];
      tpps_update_submission($state);
      break;

    default:
      break;
  }
}
