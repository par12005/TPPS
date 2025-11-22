<?php

/**
 * @file
 * Contains theme-function which shows Submission as flat table.
 */

/**
 * Shows submission as a table in block.
 *
 * @return string
 *   Returns rendered table with submission's data.
 */
function theme_submission_block(array $variables) {
  global $base_url;
  $data = $variables['data'] ?? NULL;

  $css = "
  .tgdr_status_block img {
      vertical-align: middle;
      margin-bottom: 4px;
      margin-right: 2px;
  }
  ";
  drupal_add_css($css, 'inline');

  $data['subject'] = t('TPPS Status');

  $content = "<div class='tgdr_status_block'>";
  if (preg_match('/tppsc?\/(TGDR.*)/', $_GET['q'], $matches)) {
    $accession = $matches[1];
  }
  elseif (isset($_REQUEST['accession'])) {
    $accession = $_REQUEST['accession'];
  }
  else {
    $content .= '</div>';
    $content = filter_xss($content);
    return $content;
  }

  $submission = new Submission($accession);
  $state = $submission->state;
  $state['saved_values'][TPPS_PAGE_2]['study_info'] = NULL;
  $flat = tpps_submission_flatten($state);

  $ok = "<img src='$base_url/misc/message-16-ok.png'>";
  $map_api_key = variable_get('tpps_maps_api_key', NULL);
  foreach ($flat as $item => $val) {
    if ($item[0] !== '#') {
      $item_str = filter_xss(check_plain($item));
      $val_str = filter_xss(check_plain($val));
      if (preg_match('/(Abstract|Organization)/', $item)) {
        continue;
      }
      elseif (preg_match('/File/', $item) && $file = tpps_file_load($val)) {
        // Create links to files.
        $file_url = check_plain(file_create_url($file->uri));
        // @TODO Use Drupal API: l().
        $val_str = "<a href='$file_url' target='blank'>$file->filename</a>";
        if (!$flat['#skip_loc'] and preg_match('/^(.*)Accession File/', $item, $matches)) {
          $fid = $val;
          $val_str .= tpps_build_google_map_wrapper($fid);
        }
      }
      elseif (
        preg_match('/species photo/', $item)
        && $file = tpps_file_load($val)
      ) {
        $file_url = check_plain(file_create_url($file->uri));
        $val_str = "<img src='$file_url' width='100%'>";
        $val_str .= "<br><a href='$file_url' target='blank'>$file->filename</a>";
      }
      elseif (preg_match('/Month/', $item)) {
        // Skip experiment start/end month.
        continue;
      }
      elseif (preg_match('/^(.*)Year/', $item, $matches)) {
        // Combine experiment start/end month + year.
        if (isset($flat[$matches[1] . 'Month'])) {
          $item_str = $matches[1] . 'Date';
          $val_str = "{$flat[$matches[1] . 'Month']} $val_str";
        }
        else {
          continue;
        }
      }
      elseif (
        preg_match('/Study Location/', $item)
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
        if (isset($query) and !empty($map_api_key)) {
          $url = url('https://www.google.com/maps', [
            'query' => [
              'q' => $query,
              'output' => 'embed',
              'key' => $map_api_key,
              'z' => '5',
            ],
          ]);
          // @TODO See /js/google_map.js.
          $map = '<iframe frameborder="0"'
            . ' style="border: 0; width: 100%; height: 100%;"'
            . ' src="' . $url . '" allowfullscreen> </iframe></div>';
          // @TODO Move to JS file.
          $js = "jQuery(document).ready(function ($) {
            jQuery(\"#sidebar_map\").click(function() {
              jQuery(\"#sidebar_map_wrapper\").remove();
              jQuery(\"#sidebar_map\").after( \"<br><div id=\\\"sidebar_map_wrapper\\\">$map</div>\" );
            });
          });";
          drupal_add_js($js, 'inline');
        }
      }
      elseif (
        preg_match('/Publication DOI/', $item)
        || preg_match('/Dataset DOI/', $item)
      ) {
        $link = preg_match('/https?:\/\//', $val_str) ? $val_str : "https://doi.org/$val_str";
        $val_str = "<a href=\"$link\" target=\"blank\">$val_str</a>";
      }
      $content .= "$ok $item_str: $val_str <br>";
    }
    elseif (preg_match('/#page_(.)_cleared/', $item, $matches)) {
      // Break between pages.
      if ($val) {
        $step_arr = array(
          1 => 'Author and Species Information',
          2 => 'Experimental Conditions',
          3 => 'Plant Accession',
          4 => 'Submit Data',
        );
        if ($matches[1] > 1) {
          $content .= '<br />';
        }
        $content .= "$ok Step {$matches[1]} - Complete<br />{$step_arr[$matches[1]]}<br /><hr />";
      }
      else {
        // If page not cleared, do not display remaining form state data.
        return $content;
      }
    }
    elseif (preg_match('/^#(.*)accession_(.*)/', $item, $matches)) {
      $val_str = filter_xss(check_plain($val));
      $parts = explode(" ", substr($matches[0], 1));
      $item_id = implode("_", $parts);
      $content .= "<div id=\"$item_id\" style=\"display:none;\">$val_str</div>";
    }
  }
  if (!empty($flat['#page_3_cleared'])) {
    $content .= tpps_get_markercluster_code(FALSE);
  }
  $content .= "</div>";

  return $content;
}

/**
 * Shows Submission as a table on page.
 *
 * This is useful for any functions that display all of the
 * detailed contents of a TPPS Submission.
 *
 * @return string
 *   Returns rendered table with submission's data.
 */
function theme_submission_page(array $variables) {

  $state = $variables['form_state'] ?? NULL;
  $options = $variables['options'] ?? [];
  $flat = tpps_submission_flatten($state, $options);

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
        $val_str = TppsArray::printList($val);
      }
      else {
        $val_str = filter_xss(check_plain($val));
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      if ($key == 'tpps_submission_import') {
        $key_str = t('Import Metadata');
        $import_meta = FormState::getImportMeta($variables['form_state']);
        if (!empty($import_meta)) {
          $version = $import_meta[SubmissionVersion::FORM_KEY] ?? '-';
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
      if ($key == SubmissionVersion::FORM_KEY) {
        // This is not Submission State Array but Submission Shared State which
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
        // @TODO Use data-fid to store only file id.
        $val_str .= '<br><input id="fid_' . $val . '" type="button" '
          . 'class="preview_button form-button form-submit" '
          . 'value="' . t('Preview file') . '"></input>';

        if (tpps_access('administer tpps module')) {
          // @TODO Use data-fid to store only file id.
          $val_str .= '<br><input id="fid_' . $val . '" type="button" '
            . 'class="preview_full_button form-button form-submit" '
            . 'value="' . t('Show full file') . '"></input>';
        }
        if (preg_match('/^(.*)Accession File/', $key, $matches)) {
          if (!$flat['#skip_loc']) {
            $fid = $file->fid;
            $val_str .= tpps_build_google_map_wrapper($fid);
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
        // Why?
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
          tpps_add_css_js('google_map', $form);
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
  if (variable_get('tpps_maps_api_key', NULL)) {
    $content .= tpps_get_markercluster_code(FALSE);
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
 * Prepares data for theme_submission_flatten().
 *
 * @param array $state
 *   Drupal Form API array.
 * @param array $options
 *   Options.
 *
 * @return array
 *   Returns array with data to be rendered.
 *
 * @todo Clean up code. Cache result.
 */
function tpps_submission_flatten(array $state, array $options = []) {
  if (empty($state['saved_values'])) {
    return [];
  }
  $data = $state['saved_values'];

  $hide_list = [
    // Note: Seems 'dbxref_id' and 'status' didn't show before class
    // Submission was implemented.
    'status',
    'frontpage',
    'form_build_id',
    'dbxref_id',
    // Some old/broken studies on dev-site has it but it's safe just hide it.
    'organism-1',
  ];
  foreach ($hide_list as $item) {
    unset($data[$item]);
  }

  $page_1 = &$data[TPPS_PAGE_1];
  $pub = &$page_1['publication'];
  $org = &$page_1['organism'];

  $data['#page_1_cleared'] = (bool) (isset($page_1['#cleared']) && $page_1['#cleared']);
  $data['Project ID'] = $state['ids']['project_id'] ?? NULL;
  $data['#study_photo'] = $page_1['photo'] ?? NULL;
  $data['Publication DOI'] = $page_1['doi'] ?? NULL;
  $data['Dataset DOI'] = $page_1['dataset_doi'] ?? NULL;

  $data['Publication Title'] = isset($pub['title']) ? $pub['title'] : NULL;
  $data['Primary Author'] = isset($page_1['primaryAuthor']) ? $page_1['primaryAuthor'] : NULL;
  $data['Publication Status'] = isset($pub['status']) ? $pub['status'] : NULL;
  $data['Publication Year'] = isset($pub['year']) ? $pub['year'] : NULL;
  $data['Publication Abstract'] = isset($pub['abstract']) ? $pub['abstract'] : NULL;
  $data['Publication Journal'] = isset($pub['journal']) ? $pub['journal'] : NULL;

  if (isset($pub['secondaryAuthors']['number'])) {
    $data['#number_secondary_authors'] = $pub['secondaryAuthors']['number'];
    for ($i = 1; $i <= $data['#number_secondary_authors']; $i++) {
      if (!empty($pub['secondaryAuthors'][$i])) {
        $data["Secondary Author $i"] = $pub['secondaryAuthors']["$i"];
      }
    }
  }

  if (isset($org['number'])) {
    $data['#number_organisms'] = $org['number'];
    for ($i = 1; $i <= $data['#number_organisms']; $i++) {
      if (!empty($org[$i]['name'])) {
        $data["Species $i"] = $org["$i"]['name'];
      }
    }
  }
  unset($data[TPPS_PAGE_1]);

  // Page 2.
  $page_2 = &$data[TPPS_PAGE_2];
  $data['#page_2_cleared'] = $page_2['#cleared'] ?? FALSE;
  $data['Experiment Starting Month'] = $page_2['StartingDate']['month'] ?? NULL;
  $data['Experiment Starting Year'] = $page_2['StartingDate']['year'] ?? NULL;
  $data['Experiment Ending Month'] = $page_2['EndingDate']['month'] ?? NULL;
  $data['Experiment Ending Year'] = $page_2['EndingDate']['year'] ?? NULL;
  if (!empty($page_2['study_location'])) {
    // Legacy support for old TPPS submissions.
    $legacy_loc = &$page_2['study_location'];
    $data['#location_type'] = $legacy_loc['type'] ?? NULL;

    if ($data['#location_type'] == '2' and isset($legacy_loc['custom'])) {
      $data['Study Location'] = $legacy_loc['custom'];
    }
    elseif ($data['#location_type'] and isset($legacy_loc['coordinates'])) {
      $data['Study Location'] = $legacy_loc['coordinates'];
    }
  }

  if (isset($page_2['data_type']) and $page_2['data_type']) {
    $data['Data Type'] = $page_2['data_type'];
  }

  if (!empty($page_2['study_type'])) {
    module_load_include('inc', 'tpps', 'includes/form');
    $data['Study Type'] = tpps_form_get_study_type($page_2['study_type']);

    if (!empty($page_2['study_info']['season'])) {
      $data['Seasons Assessed'] = implode($page_2['study_info']['season']);
    }

    if (!empty($page_2['study_info']['assessions'])) {
      $data['Average number of assessions'] = $page_2['study_info']['assessions'];
    }

    $data['Average High Temperature'] = $page_2['study_info']['temp']['high'] ?? NULL;
    $data['Average Low Temperature'] = $page_2['study_info']['temp']['low'] ?? NULL;

    $types = array(
      'co2' => 'CO2',
      'humidity' => 'Air Humidity',
      'light' => 'Light Intensity',
      'salinity' => 'Salinity',
    );

    foreach ($types as $type => $label) {
      if (!empty($page_2['study_info'][$type])) {
        $set = $page_2['study_info'][$type];
        if (!empty($set['option'])) {
          $data["$label Control"] = ($set['option'] == '1') ? 'Controlled' : 'Uncontrolled';
          if ($set['option'] == '1' and !empty($set['controlled'])) {
            $data["$label Value"] = $set['controlled'];
          }
          elseif (!empty($set['uncontrolled'])) {
            $data["$label Value"] = $set['uncontrolled'];
          }
        }
      }
    }

    if (!empty($page_2['study_info']['rooting']['option'])) {
      $data['Rooting Type'] = $page_2['study_info']['rooting']['option'];

      if (
        $data['Rooting Type'] == 'Soil'
        and !empty($page_2['study_info']['rooting']['soil']['type'])
      ) {
        $soil = &$page_2['study_info']['rooting']['soil'];
        $data['Soil Type'] = $soil['type'];

        if ($data['Soil Type'] == 'Other') {
          $data['Custom Soil Type'] = $soil['other'];
        }

        $data['Soil Container Type'] = $soil['container'] ?? NULL;
      }
    }

    if (!empty($page_2['study_info']['rooting']['ph']['option'])) {
      $set = $page_2['study_info']['rooting']['ph'];
      $data['pH Control'] = ($set['option'] == '1') ? 'Controlled' : 'Uncontrolled';

      if ($set['option'] == '1' and !empty($set['controlled'])) {
        $data["pH Value"] = $set['controlled'];
      }
      elseif (!empty($set['uncontrolled'])) {
        $data["pH Value"] = $set['uncontrolled'];
      }
    }

    if (!empty($page_2['study_info']['rooting']['treatment'])) {
      $treatment = $page_2['study_info']['rooting']['treatment'];
      $selected = FALSE;
      $description = FALSE;

      foreach ($treatment as $field => $value) {
        if (!$description) {
          $description = TRUE;
          $selected = ($value) ? $field : FALSE;
          continue;
        }
        elseif ($selected and !empty($value)) {
          $data["Treatment: $selected description"] = $value;
        }
        $description = FALSE;
      }
    }

    if (
      !empty($page_2['study_info']['irrigation'])
      && !empty($page_2['study_info']['irrigation']['option'])
    ) {
      $data['Irrigation Type'] = $page_2['study_info']['irrigation']['option'];
      if (
        $data['Irrigation Type'] == 'Other'
        && !empty($page_2['study_info']['irrigation']['other'])
      ) {
        $data['Custom Irrigation Type'] = $page_2['study_info']['irrigation']['other'];
      }
    }

    if (!empty($page_2['study_info']['biotic_env']['option'])) {
      $biotic_envs = [];
      foreach ($page_2['study_info']['biotic_env']['option'] as $key => $val) {
        if ($val) {
          $biotic_envs[] = $key;
        }
      }

      if (!empty($biotic_envs)) {
        $data['Biotic Environment'] = implode(', ', $biotic_envs);
        if (
          in_array('Other', $biotic_envs)
          && !empty($page_2['study_info']['biotic_env']['other'])
        ) {
          $data['Custom Biotic Environment'] = $page_2['study_info']['biotic_env']['other'];
        }
      }
    }

    if (!empty($page_2['study_info']['treatment']['check'])) {
      $treatment = &$page_2['study_info']['treatment'];
      $selected = FALSE;
      $description = FALSE;

      foreach ($treatment as $field => $value) {
        if ($field != 'check') {
          if (!$description) {
            $description = TRUE;
            $selected = ($value) ? $field : FALSE;
            continue;
          }
          elseif ($selected and !empty($value)) {
            $data["Treatment: $selected description"] = $value;
          }
          $description = FALSE;
        }
      }
    }
  }
  unset($data[TPPS_PAGE_2]);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $page_3 = &$data[TPPS_PAGE_3];

  if (isset($page_3['#cleared']) and $page_3['#cleared']) {
    $data['#page_3_cleared'] = TRUE;
  }
  else {
    $data['#page_3_cleared'] = FALSE;
  }

  // Do not show button to show on map because it works only when we have
  // exact coordinates in accession file.
  $data['#skip_loc'] = TRUE;
  $loc = &$page_3['study_location'];
  $data['#location_type'] = $loc['type'] ?? NULL;

  if ($data['#location_type'] == '2' and isset($loc['custom'])) {
    $data['Study Location'] = $loc['custom'];
  }
  elseif ($data['#location_type'] and isset($loc['coordinates'])) {
    $data['Study Location'] = $loc['coordinates'];
  }
  $page_3 = &$data[TPPS_PAGE_3]['tree-accession'];

  if (!empty($data['#number_organisms'])) {
    for ($i = 1; $i <= $data['#number_organisms']; $i++) {
      if (!empty($page_3["species-$i"]['file'])) {
        $organism_name = $data["Species $i"] . ' Accession File';
        if ($data['#number_organisms'] > 1 and empty($page_3['check'])) {
          $organism_name = 'Plant Accession File';
        }

        $data[$organism_name] = $page_3["species-$i"]['file'];
        $name = 'Location (latitude/longitude or country/state or population group)';
        $loc_type = $page_3["species-$i"]['file-groups'][$name]['#type'] ?? NULL;
        if (
          $loc_type == "gps"
          // Note: $loc_type is  a string not an array.
          // This condition was left because I wasn't able to find code
          // which sets/changes this field.
          ||  (is_array($loc_type) && in_array('gps', $loc_type))
        ) {
          $fid = $page_3["species-$i"]['file'];
          $js_settings = [
            'accession_files' => [
              $fid => [
                'no_header' => $page_3["species-$i"]['file-no-header'] ?? '0',
                'id_col'
                => $page_3["species-$i"]['file-groups']['Tree Id'][TPPS_COLUMN_PLANT_IDENTIFIER],
                'lat_col'
                => $page_3["species-$i"]['file-groups'][$name][TPPS_COLUMN_LATITUDE],
                'long_col'
                => $page_3["species-$i"]['file-groups'][$name][TPPS_COLUMN_LONGITUDE],
              ],
            ],
          ];
          drupal_add_js(['tpps' => $js_settings], 'setting');
          // Do not skip and show button to view on map.
          $data['#skip_loc'] = FALSE;
        }
      }
      if (empty($page_3['check'])) {
        break;
      }
    }
  }

  $data["Total number of plants"] = $state['stats']['tree_count'] ?? NULL;
  unset($data[TPPS_PAGE_3]);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $page_4 = &$data[TPPS_PAGE_4];

  if (isset($page_4['#cleared']) and $page_4['#cleared']) {
    $data['#page_4_cleared'] = TRUE;
  }
  else {
    $data['#page_4_cleared'] = FALSE;
  }
  if (!empty($data['#number_organisms'])) {
    for ($i = 1; $i <= $data['#number_organisms']; $i++) {
      $organism_key = 'organism-' . $i;
      if (isset($page_4[$organism_key])) {
        $org = &$page_4[$organism_key];
        $org_name = isset($data["Species $i"]) ? $data["Species $i"] : NULL;

        if (isset($org['phenotype'])) {
          $pheno = &$org['phenotype'];

          if (!empty($pheno['iso-check'])) {
            $data["$org_name Phenotype Isotope/Mass Spectrometry File"] = $pheno['iso'];
          }
          else {
            if (isset($pheno['phenotypes-meta']['number'])) {
              $data['#number_phenotypes'] = $pheno['phenotypes-meta']['number'];
              $pheno_meta = &$pheno['phenotypes-meta'];
              for ($j = 1; $j <= $data['#number_phenotypes']; $j++) {
                $fields = array(
                  'name' => 'Name',
                  'attribute' => 'Attribute',
                  'description' => 'Description',
                  'unit' => 'Unit',
                );
                if (isset($pheno_meta["$j"]['struct-check']) and $pheno_meta["$j"]['struct-check']) {
                  $fields['structure'] = 'Structure';
                }
                // [VS] #8669rmrw5
                //if (isset($pheno_meta["$j"]['val-check']) and $pheno_meta["$j"]['val-check']) {
                //  $fields['min'] = 'Minimum Value';
                //  $fields['max'] = 'Maximum Value';
                //}
                // [/VS] #8669rmrw5

                foreach ($fields as $field => $field_string) {
                  if (isset($pheno_meta["$j"][$field]) and $pheno_meta["$j"][$field] != '') {
                    if ($field == 'unit') {
                      $value = $pheno_meta[$j][$field];
                      $value = tpps_unit_get_name($value);
                    }
                    else {
                      $value = $pheno_meta[$j][$field];
                    }
                    $data["$org_name Phenotype $j $field_string"] = $value;
                  }
                }
              }
            }

            if (!empty($pheno['check']) && !empty($pheno['metadata'])) {
              $data["$org_name Phenotype Metadata File"] = $pheno['metadata'];
            }

            if (!empty($pheno['file'])) {
              $data["$org_name Phenotype File"] = $pheno['file'];
            }
            $data["$org_name Phenotypes Assessed"] = $pheno['phenotype_count'] ?? NULL;
          }

          // Rish comments: 21 March 2023
          // Based on the example of TGDR465, there is a single ISO file that can be uploaded
          // I *think* the headers should match each phenotype name that is added manually
          // from the previous part of page 4
          if (!empty($pheno['file'])) {
            $data["$org_name Phenotype File"] = $pheno['file'];
          }
        }

        if (isset($org['genotype'])) {
          $geno = &$org['genotype'];
          $snps_fieldset = $geno['SNPs'];

          $marker_types = [];
          // Note: $geno['marker-type'] is not set for phenotype studies.
          foreach (($geno['marker-type'] ?? []) as $key => $val) {
            if ($val) {
              $marker_types[] = $key;
            }
          }
          $data["$org_name Genotype Marker Types"]
            = implode(', ', ($marker_types ?? []));

          if (!empty($geno['marker-type']['SNPs'])) {
            $genotyping_design_field_value
              = $snps_fieldset['genotyping-design'] ?? NULL;
            $genotyping_design_field_title
              = tpps_form_get_genotyping_design_field_options(
                $genotyping_design_field_value
              );
            if (!empty($genotyping_design_field_title)) {
              // @TODO Compare with function tpps_page_4_marker_info().
              $data["$org_name Genotyping Design"]
                = $genotyping_design_field_title;

              // GBS Type field.
              $gbs_type_field_value = $snps_fieldset['GBS'] ?? NULL;
              $gbs_type_field_title
                = tpps_form_get_gbs_type_field_options($gbs_type_field_value);
              if (
                $genotyping_design_field_value == TPPS_GENOTYPING_DESIGN_GBS
                && !empty($gbs_type_field_title)
              ) {
                $data["$org_name GBS Type"] = $gbs_type_field_title;

                if (
                  $gbs_type_field_value == TPPS_GBS_TYPE_OTHER
                  && !empty($snps_fieldset['GBS-other'])
                ) {
                  // @TODO Use check_plain() for all output.
                  $data["$org_name Custom GBS Type"] = $snps_fieldset['GBS-other'];
                }
              }

              if (
                $genotyping_design_field_value == TPPS_GENOTYPING_DESIGN_TARGETED_CAPTURE
                && !empty($snps_fieldset['targeted-capture'])
              ) {
                $targeted_capture_options = array(
                  1 => 'Exome Capture',
                  2 => 'Other',
                );
                if (isset($targeted_capture_options[$snps_fieldset['targeted-capture']])) {
                  $data["$org_name Targeted Capture Type"]
                    = $targeted_capture_options[$snps_fieldset['targeted-capture']];
                }

                if (
                  ($data["$org_name Targeted Capture Type"] ?? NULL) == 'Other'
                  && !empty($snps_fieldset['targeted-capture-other'])
                ) {
                  $data["$org_name Custom Targeted Capture Type"]
                    = $snps_fieldset['targeted-capture-other'];
                }
              }
            }
          }
          if (
            !empty($geno['marker-type']['SSRs/cpSSRs'])
            && !empty($geno['SSRs/cpSSRs'])
          ) {
            $data["$org_name SSRs/cpSSRs Type"] = $geno['SSRs/cpSSRs'];
          }
          if (
            !empty($geno['marker-type']['Other'])
            && !empty($geno['other-marker'])
          ) {
            $data["$org_name Other Marker Type"] = $geno['other-marker'];
          }

          if (
            ($snps_fieldset['ref-genome'] ?? NULL) === 'bio'
            && ($geno['BioProject-id'] ?? NULL) != ''
          ) {
            $data["$org_name BioProject Id"] = $geno['BioProject-id'];

            if (is_array($geno['assembly-auto'] ?? NULL)) {
              $count = 1;
              foreach ($geno['assembly-auto'] as $id => $check) {
                if (isset($check) and $check) {
                  $data["$org_name Linked Accession $count"] = $id;
                  $count++;
                }
              }
            }
          }
          elseif (
            in_array(
              ($snps_fieldset['ref-genome'] ?? NULL), ['manual', 'manual2']
            )
            && (
              ($geno['tripal_fasta']['file']['file_upload'] ?? NULL)
              || ($geno['tripal_fasta']['file']['file_upload_existing'] ?? NULL)
            )
          ) {
            if ($geno['tripal_fasta']['file']['file_upload']) {
              $data["$org_name Uploaded Reference File"]
                = $geno['tripal_fasta']['file']['file_upload'];
            }
            if ($geno['tripal_fasta']['file']['file_upload_existing']) {
              $data["$org_name Reference File"]
                = $geno['tripal_fasta']['file']['file_upload_existing'];
            }
          }
          elseif (
            ($snps_fieldset['ref-genome'] ?? NULL) == 'url'
            && !empty($geno['tripal_fasta']['file']['file_remote'])
          ) {
            $data["$org_name Website of reference file"] = $geno['tripal_fasta']['file']['file_remote'];
          }
          elseif (!empty($snps_fieldset['ref-genome'])) {
            $data["$org_name Reference Genome"] = $snps_fieldset['ref-genome'];
          }

          if (!empty($geno['files']['snps-assay'])) { // Shared State. flatten().
            $data["$org_name SNP Genotype Assay File"] = $geno['files']['snps-assay'];
          }
          if (!empty($geno['files']['snps-association'])) {
            $data["$org_name SNP Association File"] = $geno['files']['snps-association'];
          }
          if (!empty($geno['files']['assay-design'])) {
            $data["$org_name Assay Design File"] = $geno['files']['assay-design'];
          }
          if (!empty($geno['files']['ssrs'])) {
            $data["$org_name SSRs Genotype File"] = $geno['files']['ssrs'];
          }
          if (!empty($geno['files']['ssrs_extra'])) {
            $data["$org_name cpSSRs Additional Genotype File"] = $geno['files']['ssrs_extra'];
          }
          //if (!empty($geno['files']['indels'])) {
          //  $data["$org_name Indel Genotype File"] = $geno['files']['indels'];
          //}
          if (!empty($geno['files']['other'])) {
            $data["$org_name Other Marker Genotype File"] = $geno['files']['other'];
          }
          // Note: Shared State.
          if (!empty($geno['files']['local_vcf'])) {
            $data["$org_name VCF File (cluster)"] = $geno['files']['local_vcf'];
          }

          if (($geno['files']['vcf'] ?? NULL) > 0) {
            if ($file = tpps_file_load($geno['files']['vcf'])) {
              $file_url = file_create_url($file->uri);
              $data["$org_name VCF File (uploaded)"] = l($file->filename,
                $file_url, ['attributes' => ['target' => 'blank']]
              );
            }
            else {
              drupal_set_message(t('Uploaded VCF file is missing'), 'error');
              // Value must be not empty to fire error message.
              $data["$org_name VCF File (uploaded)"] = '&nbsp';
            }
          }
        }

        if (isset($org['environment'])) {
          $env = &$org['environment'];

          if (variable_get('tpps_cartogratree_env', FALSE)) {
            $data["CartograPlant Layers"] = array();
            foreach ($env['env_layers'] as $layer => $layer_id) {
              if (!empty($layer_id)) {
                $data["CartograPlant Layers"][] = $layer;
              }
              if (!empty($layer_id) and !empty($env['env_params'][$layer])) {
                $params = array();
                foreach ($env['env_params'][$layer] as $param => $param_id) {
                  if (!empty($param_id)) {
                    $params[] = $param;
                  }
                }
                $data["CartograPlant Layer: $layer Parameters"] = implode(', ', $params);
              }
            }
            if (!empty($data["CartograPlant Layers"])) {
              $data["CartograPlant Layers"] = implode(', ', $data["CartograPlant Layers"]);
            }
            else {
              unset($data["CartograPlant Layers"]);
            }
          }
        }
      }
    }
  }
  unset($data[TPPS_PAGE_4]);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $sum_page = &$data['summarypage'];
  if (!empty($sum_page['comments'])) {
    $data["Additional Comments"] = $sum_page['comments'];
  }
  for ($i = 1; $i <= 10; $i++) {
    if (!empty($sum_page['files'][$i])) {
      $data["Supplemental File $i"] = $sum_page['files'][$i];
    }
  }
  if (!empty($sum_page['tree_pictures'])) {
    foreach ($sum_page['tree_pictures'] as $name => $fid) {
      if (
        substr($name, -4) == '_url'
        or substr($name, -12) == '_attribution' or substr($name, -8) == '_license'
      ) {
        continue;
      }
      $data["$name species photo"] = (!$fid) ? t('None provided') : $fid;
    }
  }
  unset($data['summarypage']);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // @TODO Use array_filter().
  $data['#dbxref_id'] = $state['dbxref_id'] ?? NULL;
  $data['#accession'] = $state['accession'] ?? NULL;
  $data['#status'] = $state['status'] ?? NULL;
  foreach ($data as $key => $val) {
    if (!isset($val)) {
      unset($data[$key]);
    }
  }

  return $data;
}
