<?php

/**
 * @file
 * Defines functions used to display complete and incomplete TPPS submissions.
 */

/**
 * Returns a formatted HTML table based on a TPPS Submission form state.
 *
 * @param array $state
 *   The form_state of the TPPS Submission form.
 *   Better use Submission Interface Array because $form_state of old studies
 *   (Submission Form Version 1) won't have some fields.
 *
 * @return string
 *   The TPPS Submission formatted in an HTML table.
 */
function tpps_table_display(array &$state, array $options = []) {
  // Note: $state could be modified version of original sharedState received
  // by accession so we use it here without saving/updating original study.
  return theme('submission_page', [
    'form_state' => $state,
    'options' => $options,
  ]);
}

/**
 * Creates a details page for a completed TPPS Submission.
 *
 * @return string
 *   The rendered page.
 */
function tpps_completed_display($accession = NULL) {
  $output = '';
  $submission = new Submission($accession);
  if ($submission->isEmpty()) {
    drupal_set_message(t('Study "@accession" not found.',
      ['@accession' => $accession]), 'error');
    drupal_goto('tpps-admin-panel');
  }
  $saved_values = $submission->sharedState['saved_values'];

  // @TODO Move CSS to /css/tpps.css. Check if scope must be 'header'.
  $css = "
  .tgdr_complete_submission {
    background: #ffffff;
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 0px 10px;
  }
  ";
  drupal_add_css($css, 'inline');
  $title = $saved_values[TPPS_PAGE_1]['publication']['title'];

  $output .= "<label for='tpps_$accession'>{$title}</label><p>"
    . t('You can view the TPPS Details page with completed and approved TPPS '
      . 'and TGDR data <a href="@link">here</a>.',
      ['@link' => url('tpps/details/' . $accession)]
    );
  if (tpps_is_admin_or_curation()) {
    $output .= ' [ ' . l(t('TPPS Admin Panel'),
      'tpps-admin-panel/' . $accession) . ' ].</p>';
  }
  if (
    $saved_values[TPPS_PAGE_1]['publication']['status'] != 'Published'
    && $submission->isCreator()
  ) {
    // @TODO Minor. Do we really need an absolute link here?
    $output .= l(t('Edit publication information'),
      'tpps/' . $accession . '/edit-publication', ['absolute' => TRUE]);
  }

  $output .= tpps_table_display($submission->sharedState);
  tpps_add_css_js('main');
  return render($output);
}

/**
 * This function populates the TPPS Submissions user tab.
 *
 * @param int $uid
 *   The id of the user.
 *
 * @global string $base_url
 *   The base URL of the site.
 *
 * @return string
 *   The HTML of the TPPS user tab.
 */
function tpps_tab_create($uid) {

  // @TODO [VS] Minor. Replace $user with $account. $user is a current user.
  $user = user_load($uid);
  if (empty($user)) {
    drupal_goto('user');
  }

  global $base_url;
  $params = drupal_get_query_parameters();

  if (isset($params['accession']) && ($params['action'] ?? NULL) === 'DELETE') {
    $submission = new Submission($params['accession']);
    $submission->delete();
    drupal_goto("user/{$uid}/tpps");
  }

  $submission_list = tpps_load_submission_multiple(
    [
      'status' => Submission::STATUS_INCOMPLETE,
      'uid' => $user->uid,
    ]
  );

  $rows = [];
  foreach ($submission_list as $accession => $submission) {
    $prefix = !empty($submission->state['tpps_type'])
      ? $submission->state['tpps_type'] : 'tpps';
    $page_1 = $submission->state['saved_values'][TPPS_PAGE_1];
    $title = $abstract = $accession_cell = NULL;

    if (isset($page_1['publication']['title'])) {
      $title = "<a href='$base_url/$prefix/{$accession}' style='text-decoration: underline;'>" . check_plain($page_1['publication']['title']) . "</a>";
    }
    else {
      $title = "<a href='$base_url/$prefix/{$accession}' style='text-decoration: underline;'>No Title</a>";
    }

    if (isset($page_1['publication']['abstract'])) {
      $abstract = check_plain($page_1['publication']['abstract']);
      if (strlen($abstract) > 53) {
        $abstract = substr($abstract, 0, 50) . "...";
      }
    }

    if (isset($accession)) {
      $accession_cell = "<input type='submit' id='delete-{$accession}' onclick='";
      $name = isset($page_1['publication']['title']) ? ' titled ' . $page_1['publication']['title'] : '';
      $name = str_replace("'", "", $name); // Remove single quotes.
      $name = str_replace('"', "", $name); // Remove double quotes.
      // @TODO It's not safe to use only JS confirmation.
      $accession_cell .= "if (confirm(\"Attention! \\n\\nAre you sure you want "
        . "to delete your submission{$name}?\\nThis action cannot be undone, "
        . "and all of your data will be lost!\")){window.location.href "
        . "= \"$base_url/tpps-submission/{$accession}/delete\"}";
      $accession_cell .= "' name='op' class='form-button' value='Delete'>";
    }

    $rows[] = [
      $title,
      isset($page_1['primaryAuthor']) ? $page_1['primaryAuthor'] : "",
      isset($page_1['organism']['number']) ? $page_1['organism']['number'] : "",
      isset($abstract) ? $abstract : "",
      isset($page_1['publication']['journal']) ? $page_1['publication']['journal'] : "",
      isset($accession_cell) ? $accession_cell : "",
    ];
  }

  $headers = array(
    'Title',
    'Primary Author',
    'Number of Species',
    'Abstract',
    'Journal',
  );

  $vars = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_incomplete_table',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );

  $final = count($rows) ? "<div class='tpps_table'><label for="
    . "'tpps_incomplete_table'>Incomplete Submissions</label>"
    . theme('table', $vars) . '</div>' : '';

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $submission_list = tpps_load_submission_multiple([
    'status' => [
      Submission::STATUS_PENDING_APPROVAL,
      Submission::STATUS_APPROVED,
    ],
    'uid' => $user->uid,
  ]);

  $rows = [];
  foreach ($submission_list as $accession => $submission) {
    $page_1 = $submission->state['saved_values'][TPPS_PAGE_1];
    $title = $abstract = NULL;

    if (isset($page_1['publication']['title'])) {
      $title = "<a href='$base_url/completed-submission/{$accession}' "
        . "style='text-decoration: underline;'>"
        . check_plain($page_1['publication']['title']) . "</a>";
    }
    else {
      $title = "<a href='$base_url/completed-submission/{$accession}' "
        . "style='text-decoration: underline;'>No Title</a>";
    }

    if (isset($page_1['publication']['abstract'])) {
      $abstract = check_plain($page_1['publication']['abstract']);
      if (strlen($abstract) > 103) {
        $abstract = substr($abstract, 0, 100) . "...";
      }
    }

    $rows[] = [
      $title ?? '',
      $page_1['primaryAuthor'] ?? '',
      $page_1['organism']['number'] ?? '',
      $abstract ?? '',
      $page_1['publication']['journal'] ?? '',
      ($submission->state['status'] ?? ''),
    ];
  }

  $headers[] = 'Status';
  $vars['header'] = $headers;
  $vars['rows'] = $rows;
  $vars['attributes']['id'] = 'tpps_complete_table';

  $final .= count($rows)
    ? "<div class='tpps_table'><label for='tpps_complete_table'>Complete "
    . "Submissions</label>" . theme('table', $vars) . '</div>' : '';

  if (empty($final)) {
    $final .= "<h2>Could not find any Submissions</h2>";
    $final .= "<p>We could not find any incomplete TPPS Submissions or TPPS "
      . "Submissions pending approval. If you would like to start a new TPPS "
      . "Submission, you can start <a href=$base_url/tpps>here.</a></p>";
  }

  tpps_add_css_js('main');
  return $final;
}

/**
 * AJAX Callback. Generates file preview.
 *
 * Shows HTML table of the first 3 lines of a file.
 */
function tpps_preview_file() {
  $params = drupal_get_query_parameters($_POST);
  $no_header = FALSE;
  $fid = $params['fid'] ?? NULL;
  if (empty($fid)) {
    return;
  }
  $rows = $params['rows'] ?? 3;
  $file = file_load($fid);
  $location = tpps_get_location($file->uri);
  $extension = tpps_get_path_extension($location);

  if ($extension == 'vcf') {
    $display = "";
    $count = 0;
    $vcf_content = gzopen($location, 'r');
    while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
      $display .= $vcf_line . "<br>";
      $count++;
      if ($count > $rows) {
        break;
      }
    }
    print(render($display));
  }
  elseif ($file) {
    $content = tpps_parse_file($fid, $rows, !empty($no_header));
    $rows = [];
    for ($i = 0; $i < count($content) - 1; $i++) {
      $rows[] = $content[$i];
    }
    $vars = [
      'header' => $content['headers'],
      'rows' => $rows,
      'attributes' => [
        'class' => ['view', "preview_$fid"],
        'id' => 'tpps_table_display',
      ],
      'sticky' => FALSE,
    ];
    $display = render(theme('table', $vars));
    print($display);
  }
}

/**
 * Returns an entity link based on an id and a name.
 *
 * @param int $id
 *   The record id in CHADO.
 * @param string $name
 *   The display text of the link.
 * @param string $label
 *   The label of the bundle.
 *
 * @return string
 *   The constructed link, or the provided name if no entity is found.
 */
function tpps_entity_link($id, $name, $label) {
  $bundle = tripal_load_bundle_entity(['label' => $label]);
  if (empty($bundle)) {
    return $name;
  }
  $entity_id = chado_get_record_entity_by_bundle($bundle, $id);
  if (!empty($entity_id)) {
    return l($name, 'bio_data/' . $entity_id);
  }
  return $name;
}

/**
 * Displays tags with proper styling.
 *
 * Used at Study Details pages and tabs.
 *
 * @param array $tags
 *   The array of tags to display.
 *
 * @return string
 *   HTML string displaying all of the tags.
 */
function tpps_show_tags(array $tags) {
  foreach ($tags as $tag) {
    $display = ($display ?? '') . '<span class="tag" style="background-color:'
      . (empty($tag['color']) ? 'white' : $tag['color'])
      . '"><span class="tag-text">' . $tag['name'] . '</span></span>';
  }
  return $display ?? '';
}
