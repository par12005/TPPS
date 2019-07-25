<?php

/**
 * @file
 * Defines function tpps_submit_all and its helper functions.
 *
 * The functions defined in this file do not actually submit the genotype,
 * phenotype, or environmental data collected from page 4. That data is instead
 * submitted through a Tripal job due to the size of the data.
 */

/**
 * Creates a record for the project and calls the submission helper functions.
 *
 * @param string $accession
 *   The accession number of the form being submitted.
 */
function tpps_submit_all($accession) {

  $transaction = db_transaction();

  try {
    $form_state = tpps_load_submission($accession);
    $values = $form_state['saved_values'];
    $firstpage = $values[TPPS_PAGE_1];
    $form_state['file_rank'] = 0;
    $form_state['ids'] = array();

    $form_state['ids']['project_id'] = tpps_chado_insert_record('project', array(
      'name' => $firstpage['publication']['title'],
      'description' => $firstpage['publication']['abstract'],
    ));

    tpps_submit_page_1($form_state);

    tpps_submit_page_2($form_state);

    tpps_submit_page_3($form_state);

    tpps_submit_page_4($form_state);

    tpps_submit_summary($form_state);

    tpps_update_submission($form_state);

    tpps_file_parsing($accession);
    $form_state['status'] = 'Approved';
    tpps_update_submission($form_state, array('status' => 'Approved'));
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('tpps', $e);
  }
}

/**
 * Submits Publication and Species data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 */
function tpps_submit_page_1(array &$form_state) {

  $dbxref_id = $form_state['dbxref_id'];
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];

  tpps_chado_insert_record('project_dbxref', array(
    'project_id' => $form_state['ids']['project_id'],
    'dbxref_id' => $dbxref_id,
  ));

  tpps_chado_insert_record('contact', array(
    'name' => $firstpage['primaryAuthor'],
    'type_id' => array(
      'cv_id' => array(
        'name' => 'tripal_contact',
      ),
      'name' => 'Person',
      'is_obsolete' => 0,
    ),
  ));

  $author_string = $firstpage['primaryAuthor'];
  if ($firstpage['publication']['secondaryAuthors']['check'] == 0 and $firstpage['publication']['secondaryAuthors']['number'] != 0) {

    for ($i = 1; $i <= $firstpage['publication']['secondaryAuthors']['number']; $i++) {
      tpps_chado_insert_record('contact', array(
        'name' => $firstpage['publication']['secondaryAuthors'][$i],
        'type_id' => array(
          'cv_id' => array(
            'name' => 'tripal_contact',
          ),
          'name' => 'Person',
          'is_obsolete' => 0,
        ),
      ));
      $author_string .= "; {$firstpage['publication']['secondaryAuthors'][$i]}";
    }
  }
  elseif ($firstpage['publication']['secondaryAuthors']['check'] != 0) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => array(
        'cv_id' => array(
          'name' => 'schema',
        ),
        'name' => 'url',
        'is_obsolete' => 0,
      ),
      'value' => file_create_url(file_load($firstpage['publication']['secondaryAuthors']['file'])->uri),
      'rank' => $form_state['file_rank'],
    ));

    $file = file_load($firstpage['publication']['secondaryAuthors']['file']);
    $location = drupal_realpath($file->uri);
    $content = tpps_parse_xlsx($location);
    $column_vals = $firstpage['publication']['secondaryAuthors']['file-columns'];

    foreach ($column_vals as $col => $val) {
      if ($val == '1') {
        $first_name = $col;
      }
      if ($val == '2') {
        $last_name = $col;
      }
      if ($val == '3') {
        $middle_initial = $col;
      }
    }

    for ($i = 0; $i < count($content) - 1; $i++) {
      tpps_chado_insert_record('contact', array(
        'name' => "{$content[$i][$last_name]}, {$content[$i][$first_name]} {$content[$i][$middle_initial]}",
        'type_id' => array(
          'cv_id' => array(
            'name' => 'tripal_contact',
          ),
          'name' => 'Person',
          'is_obsolete' => 0,
        ),
      ));
      $author_string .= "; {$content[$i][$last_name]}, {$content[$i][$first_name]} {$content[$i][$middle_initial]}";
    }
    $file->status = FILE_STATUS_PERMANENT;
    $file = file_save($file);
    $form_state['file_rank']++;
  }

  $publication_id = tpps_chado_insert_record('pub', array(
    'title' => $firstpage['publication']['title'],
    'series_name' => $firstpage['publication']['journal'],
    'type_id' => array(
      'cv_id' => array(
        'name' => 'tripal_pub',
      ),
      'name' => 'Journal Article',
      'is_obsolete' => 0,
    ),
    'pyear' => $firstpage['publication']['year'],
    'uniquename' => "$author_string {$firstpage['publication']['title']}. {$firstpage['publication']['journal']}; {$firstpage['publication']['year']}",
  ));

  tpps_chado_insert_record('project_pub', array(
    'project_id' => $form_state['ids']['project_id'],
    'pub_id' => $publication_id,
  ));

  tpps_chado_insert_record('contact', array(
    'name' => $firstpage['organization'],
    'type_id' => array(
      'cv_id' => array(
        'name' => 'tripal_contact',
      ),
      'name' => 'Organization',
      'is_obsolete' => 0,
    ),
  ));

  $names = explode(" ", $firstpage['primaryAuthor']);
  $first_name = $names[0];
  $last_name = implode(" ", array_slice($names, 1));

  tpps_chado_insert_record('pubauthor', array(
    'pub_id' => $publication_id,
    'rank' => '0',
    'surname' => $last_name,
    'givennames' => $first_name,
  ));

  if ($firstpage['publication']['secondaryAuthors']['check'] == 0 and $firstpage['publication']['secondaryAuthors']['number'] != 0) {
    for ($i = 1; $i <= $firstpage['publication']['secondaryAuthors']['number']; $i++) {
      $names = explode(" ", $firstpage['publication']['secondaryAuthors'][$i]);
      $first_name = $names[0];
      $last_name = implode(" ", array_slice($names, 1));
      tpps_chado_insert_record('pubauthor', array(
        'pub_id' => $publication_id,
        'rank' => "$i",
        'surname' => $last_name,
        'givennames' => $first_name,
      ));
    }
  }
  elseif ($firstpage['publication']['secondaryAuthors']['check'] != 0) {

    $file = file_load($firstpage['publication']['secondaryAuthors']['file']);
    $location = drupal_realpath($file->uri);
    $content = tpps_parse_xlsx($location, 0, !empty($firstpage['publication']['secondaryAuthors']['file-no-header']));
    $column_vals = $firstpage['publication']['secondaryAuthors']['file-columns'];
    $groups = $firstpage['publication']['secondaryAuthors']['file-groups'];

    $first_name = $groups['First Name']['1'];
    $last_name = $groups['Last Name']['2'];

    foreach ($column_vals as $col => $val) {
      if ($val == '3') {
        $middle_initial = $col;
        break;
      }
    }

    for ($i = 0; $i < count($content) - 1; $i++) {
      $rank = $i + 1;
      tpps_chado_insert_record('pubauthor', array(
        'pub_id' => $publication_id,
        'rank' => "$rank",
        'surname' => $content[$i][$last_name],
        'givennames' => $content[$i][$first_name] . " " . $content[$i][$middle_initial],
      ));
    }
  }

  $form_state['ids']['organism_ids'] = array();
  $organism_number = $firstpage['organism']['number'];

  for ($i = 1; $i <= $organism_number; $i++) {
    $parts = explode(" ", $firstpage['organism'][$i]);
    $genus = $parts[0];
    $species = implode(" ", array_slice($parts, 1));
    if (isset($parts[2]) and ($parts[2] == 'var.' or $parts[2] == 'subsp.')) {
      $infra = implode(" ", array_slice($parts, 2));
    }
    else {
      $infra = NULL;
    }
    $form_state['ids']['organism_ids'][$i] = tpps_chado_insert_record('organism', array(
      'genus' => $genus,
      'species' => $species,
      'infraspecific_name' => $infra,
    ));

    $and = db_and()
      ->condition('type_id', chado_get_cvterm(array('name' => 'organism 4 letter code'))->cvterm_id)
      ->condition('organism_id', $form_state['ids']['organism_ids'][$i]);
    $code_query = db_select('chado.organismprop', 'o')
      ->fields('o', array('value'))
      ->condition($and)
      ->execute();

    if (!($code = $code_query->fetchObject())) {
      $g_offset = 0;
      $s_offset = 0;
      do {
        if (isset($trial_code)) {
          if ($s_offset < strlen($species) - 2) {
            $s_offset++;
          }
          elseif ($g_offset < strlen($genus) - 2) {
            $s_offset = 0;
            $g_offset++;
          }
          else {
            throw new Exception("TPPS was unable to create a 4 letter species code for the species '$genus $species'.");
          }
        }
        $trial_code = substr($genus, $g_offset, 2) . substr($species, $s_offset, 2);
        $and = db_and()
          ->condition('type_id', chado_get_cvterm(array('name' => 'organism 4 letter code'))->cvterm_id)
          ->condition('value', $trial_code);
        $new_code_query = db_select('chado.organismprop', 'o')
          ->fields('o', array('value'))
          ->condition($and)
          ->execute();
      } while (($new_code = $new_code_query->fetchObject()));

      tpps_chado_insert_record('organismprop', array(
        'organism_id' => $form_state['ids']['organism_ids'][$i],
        'type_id' => chado_get_cvterm(array('name' => 'organism 4 letter code'))->cvterm_id,
        'value' => $trial_code,
      ));
    }

    tpps_chado_insert_record('project_organism', array(
      'organism_id' => $form_state['ids']['organism_ids'][$i],
      'project_id' => $form_state['ids']['project_id'],
    ));
  }
}

/**
 * Submits Study Design data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 */
function tpps_submit_page_2(array &$form_state) {

  $secondpage = $form_state['saved_values'][TPPS_PAGE_2];

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $form_state['ids']['project_id'],
    'type_id' => array(
      'name' => 'study_start',
      'is_obsolete' => 0,
    ),
    'value' => $secondpage['StartingDate']['month'] . " " . $secondpage['StartingDate']['year'],
  ));

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $form_state['ids']['project_id'],
    'type_id' => array(
      'name' => 'study_end',
      'is_obsolete' => 0,
    ),
    'value' => $secondpage['EndingDate']['month'] . " " . $secondpage['EndingDate']['year'],
  ));

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $form_state['ids']['project_id'],
    'type_id' => array(
      'name' => 'association_results_type',
      'is_obsolete' => 0,
    ),
    'value' => $secondpage['data_type'],
  ));

  $studytype_options = array(
    0 => '- Select -',
    1 => 'Natural Population (Landscape)',
    2 => 'Growth Chamber',
    3 => 'Greenhouse',
    4 => 'Experimental/Common Garden',
    5 => 'Plantation',
  );

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $form_state['ids']['project_id'],
    'type_id' => array(
      'name' => 'study_type',
      'is_obsolete' => 0,
    ),
    'value' => $studytype_options[$secondpage['study_type']],
  ));

  if (!empty($secondpage['study_info']['season'])) {
    $seasons = implode($secondpage['study_info']['season']);

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => array(
        'name' => 'assession_season',
        'is_obsolete' => 0,
      ),
      'value' => $seasons,
    ));
  }

  if (!empty($secondpage['study_info']['assessions'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => array(
        'name' => 'assession_number',
        'is_obsolete' => 0,
      ),
      'value' => $secondpage['study_info']['assessions'],
    ));
  }

  if (!empty($secondpage['study_info']['temp'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => array(
        'name' => 'temperature_high',
        'is_obsolete' => 0,
      ),
      'value' => $secondpage['study_info']['temp']['high'],
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => array(
        'name' => 'temperature_low',
        'is_obsolete' => 0,
      ),
      'value' => $secondpage['study_info']['temp']['low'],
    ));
  }

  $types = array(
    'co2',
    'humidity',
    'light',
    'salinity',
  );

  foreach ($types as $type) {
    if (!empty($secondpage['study_info'][$type])) {
      $set = $secondpage['study_info'][$type];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => array(
          'name' => "{$type}_control",
          'is_obsolete' => 0,
        ),
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => array(
            'name' => "{$type}_level",
            'is_obsolete' => 0,
          ),
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => array(
            'name' => "{$type}_level",
            'is_obsolete' => 0,
          ),
          'value' => $set['uncontrolled'],
        ));
      }
    }
  }

  if (!empty($secondpage['study_info']['rooting'])) {
    $root = $secondpage['study_info']['rooting'];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => array(
        'name' => 'rooting_type',
        'is_obsolete' => 0,
      ),
      'value' => $root['option'],
    ));

    if ($root['option'] == 'Soil') {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => array(
          'name' => 'soil_type',
          'is_obsolete' => 0,
        ),
        'value' => ($root['soil']['type'] == 'Other') ? $root['soil']['other'] : $root['soil']['type'],
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => array(
          'name' => 'soil_container',
          'is_obsolete' => 0,
        ),
        'value' => $root['soil']['container'],
      ));
    }

    if (!empty($secondpage['study_info']['rooting']['ph'])) {
      $set = $secondpage['study_info']['rooting']['ph'];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => array(
          'name' => "pH_control",
          'is_obsolete' => 0,
        ),
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => array(
            'name' => "pH_level",
            'is_obsolete' => 0,
          ),
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => array(
            'name' => "pH_level",
            'is_obsolete' => 0,
          ),
          'value' => $set['uncontrolled'],
        ));
      }
    }

    $description = FALSE;
    $rank = 0;
    foreach ($root['treatment'] as $value) {
      if (!$description) {
        if ($value) {
          $record_next = TRUE;
        }
        else {
          $record_next = FALSE;
        }
        $description = TRUE;
        continue;
      }
      elseif ($record_next) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => array(
            'name' => 'treatment',
            'is_obsolete' => 0,
          ),
          'value' => $value,
          'rank' => $rank,
        ));
        $rank++;
      }
      $description = FALSE;
    }
  }

  if (!empty($form_state['values']['study_info']['irrigation'])) {
    $irrigation = $form_state['values']['study_info']['irrigation'];
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => array(
        'name' => 'irrigation_type',
        'is_obsolete' => 0,
      ),
      'value' => ($irrigation['option'] == 'Other') ? $irrigation['other'] : $irrigation['option'],
    ));
  }

  if (!empty($form_state['values']['study_info']['biotic_env']['option'])) {
    foreach ($form_state['values']['study_info']['biotic_env']['option'] as $key => $check) {
      if ($check) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => array(
            'name' => 'biotic_environment',
            'is_obsolete' => 0,
          ),
          'value' => ($key == 'Other') ? $form_state['values']['study_info']['biotic_env']['other'] : $key,
        ));
      }
    }
  }

  if (!empty($form_state['values']['study_info']['treatment']) and $form_state['values']['study_info']['treatment']['check']) {
    $description = FALSE;
    $rank = 0;

    foreach ($treatment as $field => $value) {
      if ($field != 'check') {
        if (!$description) {
          $description = TRUE;
          $record_next = $value;
          continue;
        }
        elseif ($record_next) {
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $form_state['ids']['project_id'],
            'type_id' => array(
              'name' => 'treatment',
              'is_obsolete' => 0,
            ),
            'value' => $value,
            'rank' => $rank,
          ));
          $rank++;
        }
        $description = FALSE;
      }
    }
  }
}

/**
 * Submits Tree Accession data to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 */
function tpps_submit_page_3(array &$form_state) {

  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];
  $organism_number = $firstpage['organism']['number'];
  $geo_api_key = variable_get('tpps_geocode_api_key', NULL);
  $form_state['locations'] = array();

  if (!empty($thirdpage['study_location'])) {
    if ($thirdpage['study_location']['type'] !== '2') {
      $standard_coordinate = explode(',', tpps_standard_coord($thirdpage['study_location']['coordinates']));
      $latitude = $standard_coordinate[0];
      $longitude = $standard_coordinate[1];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => array(
          'name' => 'gps_latitude',
          'is_obsolete' => 0,
        ),
        'value' => $latitude,
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => array(
          'name' => 'gps_longitude',
          'is_obsolete' => 0,
        ),
        'value' => $longitude,
      ));
    }
    else {
      $location = $thirdpage['study_location']['custom'];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => array(
          'name' => 'experiment_location',
          'is_obsolete' => 0,
        ),
        'value' => $location,
      ));

      if (isset($geo_api_key)) {
        $query = urlencode($location);
        $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
        $response = json_decode(file_get_contents($url));

        if ($response->total_results) {
          $result = $response->results[0]->geometry;
          $form_state['locations'][$location] = $result;

          tpps_chado_insert_record('projectprop', array(
            'project_id' => $form_state['ids']['project_id'],
            'type_id' => array(
              'name' => 'gps_latitude',
              'is_obsolete' => 0,
            ),
            'value' => $result->lat,
          ));

          tpps_chado_insert_record('projectprop', array(
            'project_id' => $form_state['ids']['project_id'],
            'type_id' => array(
              'name' => 'gps_longitude',
              'is_obsolete' => 0,
            ),
            'value' => $result->lng,
          ));
        }
      }
    }
  }

  $form_state['ids']['stock_ids'] = array();

  for ($i = 1; $i <= $organism_number; $i++) {
    if ($organism_number == '1' or $thirdpage['tree-accession']['check'] == 0) {
      $tree_accession = $thirdpage['tree-accession'];
    }
    else {
      $tree_accession = $thirdpage['tree-accession']["species-$i"];
    }
    $fid = $tree_accession['file'];
    $column_vals = $tree_accession['file-columns'];
    $groups = $tree_accession['file-groups'];
    $loc_group = $groups['Location (latitude/longitude or country/state or population group)'];
    $loc_type = $loc_group['#type'];

    if ($organism_number != 1 and $thirdpage['tree-accession']['check'] == 0) {
      if ($groups['Genus and Species']['#type'] == 'separate') {
        $genus_col_name = $groups['Genus and Species']['6'];
        $species_col_name = $groups['Genus and Species']['7'];
      }
      else {
        $org_col_name = $groups['Genus and Species']['10'];
      }
    }

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => array(
        'cv_id' => array(
          'name' => 'schema',
        ),
        'name' => 'url',
        'is_obsolete' => 0,
      ),
      'value' => file_create_url(file_load($fid)->uri),
      'rank' => $form_state['file_rank'],
    ));

    $file = file_load($fid);
    $location = drupal_realpath($file->uri);
    $content = tpps_parse_xlsx($location);

    foreach ($column_vals as $col => $val) {
      if ($val == '8') {
        $county_col_name = $col;
      }
      if ($val == '9') {
        $district_col_name = $col;
      }
      if ($val == '13') {
        $clone_col_name = $col;
      }
    }

    $id_col_accession_name = $groups['Tree Id']['1'];

    for ($j = 0; $j < count($content) - 1; $j++) {
      $tree_id = $content[$j][$id_col_accession_name];
      if ($organism_number != 1 and $thirdpage['tree-accession']['check'] == 0) {
        if ($groups['Genus and Species']['#type'] == 'separate') {
          $genus_full_name = "{$content[$j][$genus_col_name]} {$content[$j][$species_col_name]}";
        }
        else {
          $genus_full_name = "{$content[$j][$org_col_name]}";
        }
        $id = $form_state['ids']['organism_ids'][array_search($genus_full_name, $firstpage['organism'])];
      }
      else {
        $id = $form_state['ids']['organism_ids'][$i];
      }

      $form_state['ids']['stock_ids'][$tree_id] = tpps_chado_insert_record('stock', array(
        'uniquename' => $form_state['accession'] . '-' . $tree_id,
        'type_id' => array(
          'cv_id' => array(
            'name' => 'obi',
          ),
          'name' => 'organism',
          'is_obsolete' => 0,
        ),
        'organism_id' => $id,
      ));

      if (isset($clone_col_name) and !empty($content[$j][$clone_col_name]) and $content[$j][$clone_col_name] !== $tree_accession['file-empty']) {
        $clone_name = $tree_id . '-' . $content[$j][$clone_col_name];

        $clone_id = $form_state['ids']['stock_ids'][$clone_name] = tpps_chado_insert_record('stock', array(
          'uniquename' => $form_state['accession'] . '-' . $clone_name,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'sequence',
            ),
            'name' => 'clone',
            'is_obsolete' => 0,
          ),
          'organism_id' => $id,
        ));

        tpps_chado_insert_record('stock_relationship', array(
          'subject_id' => $form_state['ids']['stock_ids'][$tree_id],
          'object_id' => $clone_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'sequence',
            ),
            'name' => 'has_part',
            'is_obsolete' => 0,
          ),
        ));
      }

      $stockprops = array();
      if (isset($clone_col_name) and !empty($content[$j][$clone_col_name]) and $content[$j][$clone_col_name] !== $tree_accession['file-empty']) {
        $tree_id .= '-' . $content[$i][$clone_col_name];
      }
      $stock_id = $form_state['ids']['stock_ids'][$tree_id];

      if (!empty($loc_group['4']) and !empty($content[$j][$loc_group['4']]) and !empty($loc_group['5']) and !empty($content[$j][$loc_group['5']])) {
        $lat_name = $loc_group['4'];
        $lng_name = $loc_group['5'];
        $raw_coord = $content[$j][$lat_name] . $content[$j][$lng_name];
        $standard_coord = explode(',', tpps_standard_coord($raw_coord));

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'name' => 'gps_latitude',
            'is_obsolete' => 0,
          ),
          'value' => $standard_coord[0],
        ));

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'name' => 'gps_longitude',
            'is_obsolete' => 0,
          ),
          'value' => $standard_coord[0],
        ));
      }
      elseif (!empty($loc_group['2']) and !empty($content[$j][$loc_group['2']]) and !empty($loc_group['3']) and !empty($content[$j][$loc_group['3']])) {
        $country_col_name = $loc_group['2'];
        $state_col_name = $loc_group['3'];

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'tripal_contact',
            ),
            'name' => 'Country',
            'is_obsolete' => 0,
          ),
          'value' => $content[$j][$country_col_name],
        ));

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'tripal_contact',
            ),
            'name' => 'State',
            'is_obsolete' => 0,
          ),
          'value' => $content[$j][$state_col_name],
        ));

        $location = "{$content[$j][$state_col_name]}, {$content[$j][$country_col_name]}";

        if (isset($county_col_name)) {
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'name' => 'county',
              'is_obsolete' => 0,
            ),
            'value' => $content[$j][$county_col_name],
          ));
          $location = "{$content[$j][$county_col_name]}, $location";
        }

        if (isset($district_col_name)) {
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'name' => 'district',
              'is_obsolete' => 0,
            ),
            'value' => $content[$j][$district_col_name],
          ));
          $location = "{$content[$j][$district_col_name]}, $location";
        }

        if (isset($geo_api_key)) {
          if (!array_key_exists($location, $form_state['locations'])) {
            $query = urlencode($location);
            $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
            $response = json_decode(file_get_contents($url));

            if ($response->total_results) {
              $results = $response->results;
              if ($response->total_results > 1 and !isset($district_col_name) and !isset($country_col_name)) {
                foreach ($results as $item) {
                  if ($item->components->_type == 'state') {
                    $result = $item->geometry;
                    break;
                  }
                }
                if (!isset($result)) {
                  $result = $results[0]->geometry;
                }
              }
              else {
                $result = $results[0]->geometry;
              }
            }
            $form_state['locations'][$location] = isset($result) ? $result : NULL;
          }
          else {
            $result = $form_state['locations'][$location];
          }

          if (!empty($result)) {
            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'name' => 'gps_latitude',
                'is_obsolete' => 0,
              ),
              'value' => $result->lat,
            ));

            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'name' => 'gps_longitude',
                'is_obsolete' => 0,
              ),
              'value' => $result->lng,
            ));
          }
        }
      }
      else {
        $pop_group_name = $loc_group['12'];

        $loc = $thirdpage['tree-accession']['pop-group'][$content[$j][$pop_group_name]];
        $coord = tpps_standard_coord($loc);

        if ($coord) {
          $parts = explode(',', $coord);
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'name' => 'gps_latitude',
              'is_obsolete' => 0,
            ),
            'value' => $parts[0],
          ));

          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'name' => 'gps_longitude',
              'is_obsolete' => 0,
            ),
            'value' => $parts[1],
          ));
        }
        else {
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'nd_geolocation_property',
              ),
              'name' => 'Location',
              'is_obsolete' => 0,
            ),
            'value' => $loc,
          ));

          if (isset($geo_api_key)) {
            if (!array_key_exists($location, $form_state['locations'])) {
              $query = urlencode($loc);
              $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
              $response = json_decode(file_get_contents($url));
              $result = ($response->total_results) ? $response->results[0]->geometry : NULL;
              $form_state['locations'][$location] = $result;
            }
            else {
              $result = $form_state['locations'][$location];
            }

            if (!empty($result)) {
              tpps_chado_insert_record('stockprop', array(
                'stock_id' => $stock_id,
                'type_id' => array(
                  'name' => 'gps_latitude',
                  'is_obsolete' => 0,
                ),
                'value' => $result->lat,
              ));

              tpps_chado_insert_record('stockprop', array(
                'stock_id' => $stock_id,
                'type_id' => array(
                  'name' => 'gps_longitude',
                  'is_obsolete' => 0,
                ),
                'value' => $result->lng,
              ));
            }
          }
        }
      }
    }

    $file->status = FILE_STATUS_PERMANENT;
    $file = file_save($file);
    $form_state['file_rank']++;
  }

  foreach ($form_state['ids']['stock_ids'] as $tree_id => $stock_id) {
    tpps_chado_insert_record('project_stock', array(
      'stock_id' => $stock_id,
      'project_id' => $form_state['ids']['project_id'],
    ));
  }

  if (!empty($thirdpage['existing_trees'])) {
    tpps_matching_trees($form_state['ids']['project_id']);
  }
}

/**
 * Submits Tripal FASTAImporter job for reference genome.
 *
 * The remaining data for the fourth page is submitted during the TPPS File
 * Parsing Tripal Job due to its size.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 */
function tpps_submit_page_4(array &$form_state) {
  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];

  for ($i = 1; $i <= $organism_number; $i++) {
    if (isset($fourthpage["organism-$i"]['genotype'])) {
      $ref_genome = $fourthpage["organism-$i"]['genotype']['ref-genome'];

      if ($ref_genome === 'url' or $ref_genome === 'manual' or $ref_genome === 'manual2') {
        // Create job for tripal fasta importer.
        $class = 'FASTAImporter';
        tripal_load_include_importer_class($class);

        $fasta = $fourthpage["organism-$i"]['genotype']['tripal_fasta'];

        $file_upload = isset($fasta['file']['file_upload']) ? trim($fasta['file']['file_upload']) : 0;
        $file_existing = isset($fasta['file']['file_upload_existing']) ? trim($fasta['file']['file_upload_existing']) : 0;
        $file_remote = isset($fasta['file']['file_remote']) ? trim($fasta['file']['file_remote']) : 0;
        $analysis_id = $fasta['analysis_id'];
        $seqtype = $fasta['seqtype'];
        $organism_id = $form_state['ids']['organism_ids'][$i];
        $re_accession = $fasta['db']['re_accession'];
        $db_id = $fasta['db']['db_id'];

        $run_args = array(
          'importer_class' => $class,
          'file_remote' => $file_remote,
          'analysis_id' => $analysis_id,
          'seqtype' => $seqtype,
          'organism_id' => $organism_id,
          'method' => '2',
          'match_type' => '0',
          're_name' => '',
          're_uname' => '',
          're_accession' => $re_accession,
          'db_id' => $db_id,
          'rel_type' => '',
          're_subject' => '',
          'parent_type' => '',
        );

        $file_details = array();

        if ($file_existing) {
          $file_details['fid'] = $file_existing;
        }
        elseif ($file_upload) {
          $file_details['fid'] = $file_upload;
        }
        elseif ($file_remote) {
          $file_details['file_remote'] = $file_remote;
        }

        try {
          $importer = new $class();
          $form = array();
          $importer->formSubmit($form, $form_state);

          $importer->create($run_args, $file_details);

          $importer->submitJob();

        }
        catch (Exception $ex) {
          drupal_set_message('Cannot submit import: ' . $ex->getMessage(), 'error');
        }
      }
    }
  }
}

/**
 * Submits additional data provided in the summary page to the database.
 *
 * @param array $form_state
 *   The state of the form being submitted.
 */
function tpps_submit_summary(array &$form_state) {
  $analysis_options = array(
    'diversity' => 'Diversity',
    'population_structure' => 'Population Structure',
    'association_genetics' => 'Association Genetics',
    'landscape_genomics' => 'Landscape Genomics',
    'phenotype_environment' => 'Phenotype-Environment',
  );

  foreach ($analysis_options as $option => $label) {
    if (!empty($form_state['saved_values']['summarypage']['analysis']["{$option}_check"])) {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => array(
          'cv_id' => array(
            'name' => 'analysis_property',
          ),
          'name' => 'Analysis Type',
          'is_obsolete' => 0,
        ),
        'value' => $label,
      ));

      if (!empty($form_state['saved_values']['summarypage']['analysis']["{$option}_file"]) and file_load($form_state['saved_values']['summarypage']['analysis']["{$option}_file"])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($form_state['saved_values']['summarypage']['analysis']["{$option}_file"])->uri),
          'rank' => $form_state['file_rank'],
        ));
        $form_state['file_rank']++;

        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => array(
            'name' => 'source_description',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($form_state['saved_values']['summarypage']['analysis']["{$option}_file_description"])->uri),
        ));
      }
    }
  }
}
