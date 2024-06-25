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
  $accession = $state['accession'] ?? NULL;
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    return;
  }
  // Note: $state could be modified version of original sharedState received
  // by accession so we use it here without saving/updating original study.
  // @TODO Check if this code could be merged (and cached) with Submission::flatten().
  $submission->sharedState = $state;

  $flat = $submission->flatten();

  $rows = [];
  $content = '';
  foreach ($flat as $key => $val) {
    if ($key[0] != '#') {
      // [VS] Added extra space which must not be visible at site but
      // will divide key and it's value in CLI.
      $key_str = filter_xss(check_plain($key)) . ' ';
      // Some items are arrays and this cause a lot of warnings about using
      // check_plain() for arrays. Other way is to skip array processing at all
      // or use empty string for arrays.
      // @TODO Probably better way is to use check_plain() for each field and
      // count it's content.
      if (
        // VCF files have a hyperlink instead of File Id. Exclude from processing.
        stripos($key_str, 'VCF File (cluster)') !== FALSE
        || stripos($key_str, 'VCF File (uploaded)') !== FALSE
      ) {
        $val_str = $val;
      }
      elseif (is_array($val)) {
        foreach ($val as $key => $value) {
          $value = is_array($value) ? var_export($value) : $value;
          $items[] = check_plain($key . ': ' . $value);
        }
        $val_str = theme('item_list', ['items' => ($items ?? [])]);
      }
      else {
        $val_str = filter_xss(check_plain($val));
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      if ($key == 'tpps_submission_import') {
        $key_str = t('Submission Import Metadata');
        $import_meta = $submission->getImportMeta();
        if (!empty($import_meta)) {
          $version = $import_meta[TPPS_SUBMISSION_FORM_VERSION_KEY] ?? '-';
          $val_str = theme('item_list', [
            'items' => [
              t('Original Accession: !url', ['!url' => $import_meta['accession_link']]),
              t('Import Date: @date.', ['@date' => $import_meta['full_date']]),
              t('Original site: !site.',
                ['!site' => l($import_meta['site'], $import_meta['site'])]
              ),
              t('Original Submision Form Version: @version.', ['@version' => $version]),
            ],
          ]);
        }
      }
      if ($key == 'submission_form_version') {
        // This is not Submission State Array but Submission Interface which
        // version is always '1'. So we don't need to show this.
        continue;
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Files.
      if (
        preg_match('/File/', $key)
        && ($file = tpps_file_load($val) === FALSE)
        // VCF files are gzipped value is not the File Id but a hyperlink to
        // file which could be downloaded and manually reviewed because
        // preview not work for gzipped files.
        && stripos($key_str, 'VCF File (cluster)') === FALSE
        && stripos($key_str, 'VCF File (uploaded)') === FALSE
      ) {
        // SOS. FILE NOT FOUND.
        $val_str .= ' <span style="font-size: 32px;">🆘</span><span>(File missing)</span>';
      }
      if (preg_match('/File/', $key) and ($file = file_load($val))) {

        // Create links to files.
        $file_name = $file->filename;

        $file_url = check_plain(file_create_url($file->uri));
        $val_str = "<a href='$file_url' target='blank'>$file_name</a>";
        if ($revised_file = tpps_file_load($state['revised_files'][$val] ?? '')) {
          $val = $state['revised_files'][$val];
          $file_url = check_plain(file_create_url($revised_file->uri));
          $file_name = $revised_file->filename;
          $val_str = "<a href='$file_url' target='blank'>$file_name</a> ( Revised from $val_str )";
        }

        // @TODO [VS] Use Drupal Form API instead of HTML in PHP.
        $val_str .= "<br><input id=\"fid_$val\" class=\"preview_button "
          . "form-button form-submit\" type=\"button\" value=\""
          . t('Preview file') . "\"></input>";
        if (tpps_access('administer tpps module')) {
          $val_str .= "<br><input id=\"fid_$val\" class=\"preview_full_button "
            . "form-button form-submit\" type=\"button\" value=\""
            . t('Show full file') . "\"></input>";
        }
        if (preg_match('/^(.*)Accession File/', $key, $matches)) {
          if (!$flat['#skip_loc']) {
            $fid = $file->fid;
            $wrapper_id = "{$fid}_map_wrapper";
            $button_id = "{$fid}_map_button";
            $val_str .= "<div id=\"$wrapper_id\"></div>"
              . "<input id=\"$button_id\" type=\"button\" "
              . " class=\"form-button form-submit\" value=\""
              . t('Click here to view plants on map!') . "\"></input>";
            $js_settings = [
              'map_buttons' => [
                $fid => [
                  'wrapper' => $wrapper_id,
                  'button' => $button_id,
                  'fid' => $fid,
                ],
              ],
            ];
            drupal_add_js(['tpps' => $js_settings], 'setting');
          }
          else {
            $val_str .= "<br>This submission was submitted without "
              . "validating location information. As a result, we did not "
              . "import location information for these plants.";
          }
        }

        if (!empty($options['files']['revision_destination'])) {
          $val_str .= "<div id=\"file_{$file->fid}_options_dest\"></div>";
        }
      }
      elseif (
        preg_match('/species photo/', $key)
        && (file_load($val))
        && ($file_name = file_load($val)->filename)
      ) {
        $file_url = check_plain(file_create_url(file_load($val)->uri));
        $val_str = "<img src='$file_url' width='100%'>";
        $val_str .= "<br><a href='$file_url' target='blank'>$file_name</a>";
      }
      elseif (preg_match('/Month/', $key)) {
        // Skip experiment start/end month.
        continue;
      }
      elseif (preg_match('/^(.*)Year/', $key, $matches)) {
        // Combine experiment start/end month + year.
        if (isset($flat[$matches[1] . 'Month'])) {
          $key_str = $matches[1] . 'Date';
          $val_str = "{$flat[$matches[1] . 'Month']} $val_str";
        }
        else {
          continue;
        }
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      elseif (
        preg_match('/Study Location/', $key)
        && isset($flat['#location_type'])
        && $flat['#location_type']
      ) {
        // Add map capability to location.
        if ($flat['#location_type'] == '2') {
          $query = $val_str;
        }
        elseif ($flat['#location_type'] != '2' and ($standard = tpps_standard_coord($val_str))) {
          $query = $standard;
        }
        $val_str = "<a id=\"sidebar_map\" style=\"cursor:pointer\">$val_str</a>";
        if (isset($query)) {
          $map = "<iframe frameborder=\\\"0\\\" style=\\\"border:0;width:100%;"
            . "height:100%\\\" src=\\\"https://www.google.com/maps?q=$query"
            . "&output=embed&key=AIzaSyDkeQ6KN6HEBxrIoiSCrCHFhIbipycqouY&z=5\\\" "
            . "allowfullscreen> </iframe></div>";
          $js = "jQuery(document).ready(function ($) {
            jQuery(\"#sidebar_map\").click(function() {
              jQuery(\"#sidebar_map_wrapper\").remove();
              jQuery(\"#sidebar_map\").after( \"<br><div
                id=\\\"sidebar_map_wrapper\\\">$map</div>\" );
            });
          });";
          drupal_add_js($js, 'inline');
        }
      }
      elseif ($key == 'Publication Abstract' and strlen($val_str) > 50) {
        $val_str = substr($val_str, 0, 47) . '...';
      }
      elseif (in_array($key, ['Publication DOI', 'Dataset DOI'])) {
        $link = preg_match('/https?:\/\//', $val_str) ? $val_str : "https://doi.org/$val_str";
        $val_str = "<a href=\"$link\" target=\"blank\">$val_str</a>";
      }
      elseif (!empty($options['skip_phenotypes']) and preg_match('/Phenotype/', $key)) {
        continue;
      }

      $rows[] = [ucwords(str_replace('_', ' ', $key_str)), $val_str];
    }
    elseif (preg_match('/^#(.*)accession_(.*)/', $key, $matches)) {
      $val_str = filter_xss(check_plain($val));
      $parts = explode(" ", substr($matches[0], 1));
      $item_id = implode("_", $parts);
      $content .= "<div id=\"$item_id\" style=\"display:none;\">$val_str</div>";
    }
  }
  $map_api_key = variable_get('tpps_maps_api_key', NULL);
  if (!empty($map_api_key)) {
    $content .= "<script src=\"https://developers.google.com/maps/"
      . "documentation/javascript/examples/markerclusterer/markerclusterer.js\">"
      . "</script><script src=\"https://maps.googleapis.com/maps/api/js"
      . "?key=$map_api_key&callback=initMap\" async defer></script>";
  }

  $headers = ['Field', 'Value'];
  $vars = [
    'header' => $headers,
    'rows' => $rows,
    'attributes' => [
      'class' => ['view'],
      'id' => 'tpps_table_display',
    ],
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  ];
  if (!empty($flat['#study_photo']) && $file = file_load($flat['#study_photo'])) {
    $photo = '<img src="' . check_plain(file_create_url($file->uri)) . '" '
      . 'style="display: block; margin-left: auto; margin-right: auto;"></img>';
  }
  $display = ($photo ?? '') . theme('table', $vars) . $content;
  return $display;
}

/**
 * Creates a details page for a completed TPPS Submission.
 *
 * @global string $base_url
 *   The base url of the site.
 *
 * @return string
 *   The rendered page.
 */
function tpps_completed_display($accession = NULL) {
  global $base_url;
  global $user;
  $output = '';

  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
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
      'status' => TPPS_SUBMISSION_STATUS_INCOMPLETE,
      'uid' => $user->uid,
    ]
  );

  $rows = [];
  foreach ($submission_list as $accession => $submission) {
    $prefix = !empty($submission->state['tpps_type'])
      ? $submission->state['tpps_type'] : 'tpps';
    $page_1 = &$submission->state['saved_values'][TPPS_PAGE_1];
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

  $final = $submission->doesExist() ? "<div class='tpps_table'><label for="
    . "'tpps_incomplete_table'>Incomplete Submissions</label>"
    . theme('table', $vars) . '</div>' : "";
  $submission_list = [];

  $submission_list = tpps_load_submission_multiple([
    'status' => [
      TPPS_SUBMISSION_STATUS_PENDING_APPROVAL,
      TPPS_SUBMISSION_STATUS_APPROVED,
    ],
    'uid' => $user->uid,
  ]);

  $rows = array();
  foreach ($submission_list as $accession => $submission) {
    $page_1 = &$submission->state['saved_values'][TPPS_PAGE_1];
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

  $final .= $submission->doesExist()
    ? "<div class='tpps_table'><label for='tpps_complete_table'>Complete "
    . "Submissions</label>" . theme('table', $vars) . '</div>' : "";

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
  global $base_url;
  $bundle = tripal_load_bundle_entity(array('label' => $label));
  if (empty($bundle)) {
    return $name;
  }
  $entity_id = chado_get_record_entity_by_bundle($bundle, $id);
  if (!empty($entity_id)) {
    return "<a href=\"$base_url/bio_data/$entity_id\">$name</a>";
  }
  return $name;
}

/**
 * Displays tags with proper styling.
 *
 * @param array $tags
 *   The array of tags to display.
 *
 * @return string
 *   HTML string displaying all of the tags.
 */
function tpps_show_tags(array $tags) {
  $display = "";
  foreach ($tags as $tag) {
    $color = !empty($tag['color']) ? $tag['color'] : 'white';
    $display .= "<span class=\"tag\" style=\"background-color:$color\"><span class=\"tag-text\">{$tag['name']}</span></span>";
  }
  return $display;
}
