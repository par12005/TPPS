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

  $form_state = tpps_load_submission($accession);
  $form_state['status'] = 'Submission Job Running';
  tpps_update_submission($form_state, array('status' => 'Submission Job Running'));
  tpps_submission_clear_db($accession);
  $project_id = $form_state['ids']['project_id'] ?? NULL;
  $transaction = db_transaction();

  try {
    $form_state = tpps_load_submission($accession);
    tpps_clean_state($form_state);
    $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
    $form_state['file_rank'] = 0;
    $form_state['ids'] = array();

    $form_state['title'] = $firstpage['publication']['title'];
    $form_state['abstract'] = $firstpage['publication']['abstract'];
    $project_record = array(
      'name' => $firstpage['publication']['title'],
      'description' => $firstpage['publication']['abstract'],
    );
    if (!empty($project_id)) {
      $project_record['project_id'] = $project_id;
    }
    $form_state['ids']['project_id'] = tpps_chado_insert_record('project', $project_record);

    tpps_tripal_entity_publish('Project', array($firstpage['publication']['title'], $form_state['ids']['project_id']));

    tpps_submit_page_1($form_state);

    tpps_submit_page_2($form_state);

    tpps_submit_page_3($form_state);

    tpps_submit_page_4($form_state);

    tpps_submit_summary($form_state);

    tpps_update_submission($form_state);

    tpps_file_parsing($accession);

    tpps_submission_rename_files($accession);
    $form_state = tpps_load_submission($accession);
    $form_state['status'] = 'Approved';
    $form_state['loaded'] = time();
    tpps_update_submission($form_state, array('status' => 'Approved'));
  }
  catch (Exception $e) {
    $transaction->rollback();
    $form_state = tpps_load_submission($accession);
    $form_state['status'] = 'Pending Approval';
    tpps_update_submission($form_state, array('status' => 'Pending Approval'));
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
  $seconds = $firstpage['publication']['secondaryAuthors'];

  tpps_chado_insert_record('project_dbxref', array(
    'project_id' => $form_state['ids']['project_id'],
    'dbxref_id' => $dbxref_id,
  ));

  if (!empty($form_state['tpps_type']) and $form_state['tpps_type'] == 'tppsc' and !empty($form_state['saved_values'][TPPS_PAGE_1]['doi'])) {
    $dryad_db = chado_get_db(array('name' => 'dryad'));
    $dryad_dbxref = chado_insert_dbxref(array(
      'db_id' => $dryad_db->db_id,
      'accession' => $form_state['saved_values'][TPPS_PAGE_1]['doi'],
    ))->dbxref_id;
    tpps_chado_insert_record('project_dbxref', array(
      'project_id' => $form_state['ids']['project_id'],
      'dbxref_id' => $dryad_dbxref,
    ));
  }

  if (!empty($firstpage['photo'])) {
    tpps_add_project_file($form_state, $firstpage['photo']);
  }

  $primary_author_id = tpps_chado_insert_record('contact', array(
    'name' => $firstpage['primaryAuthor'],
    'type_id' => tpps_load_cvterm('person')->cvterm_id,
  ));

  tpps_chado_insert_record('project_contact', array(
    'project_id' => $form_state['ids']['project_id'],
    'contact_id' => $primary_author_id,
  ));

  $authors = array($firstpage['primaryAuthor']);
  if ($seconds['number'] != 0) {
    for ($i = 1; $i <= $seconds['number']; $i++) {
      tpps_chado_insert_record('contact', array(
        'name' => $seconds[$i],
        'type_id' => tpps_load_cvterm('person')->cvterm_id,
      ));

      $names = explode(" ", $seconds[$i]);
      $first_name = implode(" ", array_slice($names, 0, -1));
      $last_name = end($names);

      $pubauthors[] = array(
        'rank' => "$i",
        'surname' => $last_name,
        'givennames' => $first_name,
      );
      $authors[] = $seconds[$i];
    }
  }

  $publication_id = tpps_chado_insert_record('pub', array(
    'title' => $firstpage['publication']['title'],
    'series_name' => $firstpage['publication']['journal'],
    'type_id' => tpps_load_cvterm('article')->cvterm_id,
    'pyear' => $firstpage['publication']['year'],
    'uniquename' => implode('; ', $authors) . " {$firstpage['publication']['title']}. {$firstpage['publication']['journal']}; {$firstpage['publication']['year']}",
  ));
  tpps_tripal_entity_publish('Publication', array($firstpage['publication']['title'], $publication_id));
  $form_state['pyear'] = $firstpage['publication']['year'];
  $form_state['journal'] = $firstpage['publication']['journal'];

  if (!empty($firstpage['publication']['abstract'])) {
    tpps_chado_insert_record('pubprop', array(
      'pub_id' => $publication_id,
      'type_id' => tpps_load_cvterm('abstract')->cvterm_id,
      'value' => $firstpage['publication']['abstract'],
    ));
  }

  tpps_chado_insert_record('pubprop', array(
    'pub_id' => $publication_id,
    'type_id' => tpps_load_cvterm('authors')->cvterm_id,
    'value' => implode(', ', $authors),
  ));
  $form_state['authors'] = $authors;

  tpps_chado_insert_record('project_pub', array(
    'project_id' => $form_state['ids']['project_id'],
    'pub_id' => $publication_id,
  ));

  if (!empty($firstpage['organization'])) {
    $organization_id = tpps_chado_insert_record('contact', array(
      'name' => $firstpage['organization'],
      'type_id' => tpps_load_cvterm('organization')->cvterm_id,
    ));

    tpps_chado_insert_record('contact_relationship', array(
      'type_id' => tpps_load_cvterm('contact_part_of')->cvterm_id,
      'subject_id' => $primary_author_id,
      'object_id' => $organization_id,
    ));
  }

  $names = explode(" ", $firstpage['primaryAuthor']);
  $first_name = implode(" ", array_slice($names, 0, -1));
  $last_name = end($names);

  tpps_chado_insert_record('pubauthor', array(
    'pub_id' => $publication_id,
    'rank' => '0',
    'surname' => $last_name,
    'givennames' => $first_name,
  ));

  if (!empty($pubauthors)) {
    foreach ($pubauthors as $info) {
      $info['pub_id'] = $publication_id;
      tpps_chado_insert_record('pubauthor', $info);
    }
  }

  $form_state['ids']['organism_ids'] = array();
  $organism_number = $firstpage['organism']['number'];

  for ($i = 1; $i <= $organism_number; $i++) {
    $parts = explode(" ", $firstpage['organism'][$i]);
    $genus = $parts[0];
    $species = implode(" ", array_slice($parts, 1));
    $infra = NULL;
    if (isset($parts[2]) and ($parts[2] == 'var.' or $parts[2] == 'subsp.')) {
      $infra = implode(" ", array_slice($parts, 2));
    }

    $record = array(
      'genus' => $genus,
      'species' => $species,
      'infraspecific_name' => $infra,
    );

    if (preg_match('/ x /', $species)) {
      $record['type_id'] = tpps_load_cvterm('speciesaggregate')->cvterm_id;
    }
    $form_state['ids']['organism_ids'][$i] = tpps_chado_insert_record('organism', $record);

    $code_exists = tpps_chado_prop_exists('organism', $form_state['ids']['organism_ids'][$i], 'organism 4 letter code');

    if (!$code_exists) {
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
        $new_code_query = chado_select_record('organismprop', array('value'), array(
          'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
          'value' => $trial_code,
        ));
      } while (!empty($new_code_query));

      tpps_chado_insert_record('organismprop', array(
        'organism_id' => $form_state['ids']['organism_ids'][$i],
        'type_id' => tpps_load_cvterm('organism 4 letter code')->cvterm_id,
        'value' => $trial_code,
      ));
    }

    $fam_exists = tpps_chado_prop_exists('organism', $form_state['ids']['organism_ids'][$i], 'family');

    if (!$fam_exists) {
      $family = tpps_get_family($firstpage['organism'][$i]);
      tpps_chado_insert_record('organismprop', array(
        'organism_id' => $form_state['ids']['organism_ids'][$i],
        'type_id' => tpps_load_cvterm('family')->cvterm_id,
        'value' => $family,
      ));
    }

    $sub_exists = tpps_chado_prop_exists('organism', $form_state['ids']['organism_ids'][$i], 'subkingdom');

    if (!$sub_exists) {
      $subkingdom = tpps_get_subkingdom($firstpage['organism'][$i]);
      tpps_chado_insert_record('organismprop', array(
        'organism_id' => $form_state['ids']['organism_ids'][$i],
        'type_id' => tpps_load_cvterm('subkingdom')->cvterm_id,
        'value' => $subkingdom,
      ));
    }

    tpps_chado_insert_record('project_organism', array(
      'organism_id' => $form_state['ids']['organism_ids'][$i],
      'project_id' => $form_state['ids']['project_id'],
    ));

    tpps_chado_insert_record('pub_organism', array(
      'organism_id' => $form_state['ids']['organism_ids'][$i],
      'pub_id' => $publication_id,
    ));

    tpps_tripal_entity_publish('Organism', array("$genus $species", $form_state['ids']['organism_ids'][$i]));
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

  if (!empty($secondpage['StartingDate'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('study_start')->cvterm_id,
      'value' => $secondpage['StartingDate']['month'] . " " . $secondpage['StartingDate']['year'],
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('study_end')->cvterm_id,
      'value' => $secondpage['EndingDate']['month'] . " " . $secondpage['EndingDate']['year'],
    ));
  }

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $form_state['ids']['project_id'],
    'type_id' => tpps_load_cvterm('association_results_type')->cvterm_id,
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
    'type_id' => tpps_load_cvterm('study_type')->cvterm_id,
    'value' => $studytype_options[$secondpage['study_type']],
  ));

  if (!empty($secondpage['study_info']['season'])) {
    $seasons = implode($secondpage['study_info']['season']);

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('assession_season')->cvterm_id,
      'value' => $seasons,
    ));
  }

  if (!empty($secondpage['study_info']['assessions'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('assession_number')->cvterm_id,
      'value' => $secondpage['study_info']['assessions'],
    ));
  }

  if (!empty($secondpage['study_info']['temp'])) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('temperature_high')->cvterm_id,
      'value' => $secondpage['study_info']['temp']['high'],
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('temperature_low')->cvterm_id,
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
        'type_id' => tpps_load_cvterm("{$type}_control")->cvterm_id,
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm("{$type}_level")->cvterm_id,
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm("{$type}_level")->cvterm_id,
          'value' => $set['uncontrolled'],
        ));
      }
    }
  }

  if (!empty($secondpage['study_info']['rooting'])) {
    $root = $secondpage['study_info']['rooting'];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $form_state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('rooting_type')->cvterm_id,
      'value' => $root['option'],
    ));

    if ($root['option'] == 'Soil') {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('soil_type')->cvterm_id,
        'value' => ($root['soil']['type'] == 'Other') ? $root['soil']['other'] : $root['soil']['type'],
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('soil_container')->cvterm_id,
        'value' => $root['soil']['container'],
      ));
    }

    if (!empty($secondpage['study_info']['rooting']['ph'])) {
      $set = $secondpage['study_info']['rooting']['ph'];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('pH_control')->cvterm_id,
        'value' => ($set['option'] == '1') ? 'True' : 'False',
      ));

      if ($set['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('pH_level')->cvterm_id,
          'value' => $set['controlled'],
        ));
      }
      elseif (!empty($set['uncontrolled'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('pH_level')->cvterm_id,
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
          'type_id' => tpps_load_cvterm('treatment')->cvterm_id,
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
      'type_id' => tpps_load_cvterm('irrigation_type')->cvterm_id,
      'value' => ($irrigation['option'] == 'Other') ? $irrigation['other'] : $irrigation['option'],
    ));
  }

  if (!empty($form_state['values']['study_info']['biotic_env']['option'])) {
    foreach ($form_state['values']['study_info']['biotic_env']['option'] as $key => $check) {
      if ($check) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('biotic_environment')->cvterm_id,
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
            'type_id' => tpps_load_cvterm('treatment')->cvterm_id,
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
  $form_state['locations'] = array();
  $form_state['tree_info'] = array();
  $stock_count = 0;
  $loc_name = 'Location (latitude/longitude or country/state or population group)';

  if (!empty($thirdpage['study_location'])) {
    if ($thirdpage['study_location']['type'] !== '2') {
      $standard_coordinate = explode(',', tpps_standard_coord($thirdpage['study_location']['coordinates']));
      $latitude = $standard_coordinate[0];
      $longitude = $standard_coordinate[1];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('gps_latitude')->cvterm_id,
        'value' => $latitude,
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('gps_longitude')->cvterm_id,
        'value' => $longitude,
      ));
    }
    else {
      $location = $thirdpage['study_location']['custom'];

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $form_state['ids']['project_id'],
        'type_id' => tpps_load_cvterm('experiment_location')->cvterm_id,
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
            'type_id' => tpps_load_cvterm('gps_latitude')->cvterm_id,
            'value' => $result->lat,
          ));

          tpps_chado_insert_record('projectprop', array(
            'project_id' => $form_state['ids']['project_id'],
            'type_id' => tpps_load_cvterm('gps_longitude')->cvterm_id,
            'value' => $result->lng,
          ));
        }
      }
    }
  }

  $cvterms = array(
    'org' => tpps_load_cvterm('organism')->cvterm_id,
    'clone' => tpps_load_cvterm('clone')->cvterm_id,
    'has_part' => tpps_load_cvterm('has_part')->cvterm_id,
    'lat' => tpps_load_cvterm('gps_latitude')->cvterm_id,
    'lng' => tpps_load_cvterm('gps_longitude')->cvterm_id,
    'country' => tpps_load_cvterm('country')->cvterm_id,
    'state' => tpps_load_cvterm('state')->cvterm_id,
    'county' => tpps_load_cvterm('county')->cvterm_id,
    'district' => tpps_load_cvterm('district')->cvterm_id,
    'loc' => tpps_load_cvterm('location')->cvterm_id,
  );

  $records = array(
    'stock' => array(),
    'stockprop' => array(),
    'stock_relationship' => array(),
    'project_stock' => array(),
  );
  $overrides = array(
    'stock_relationship' => array(
      'subject' => array(
        'table' => 'stock',
        'columns' => array(
          'subject_id' => 'stock_id',
        ),
      ),
      'object' => array(
        'table' => 'stock',
        'columns' => array(
          'object_id' => 'stock_id',
        ),
      ),
    ),
  );

  $multi_insert_options = array(
    'fk_overrides' => $overrides,
    'fks' => 'stock',
    'entities' => array(
      'label' => 'Stock',
      'table' => 'stock',
      'prefix' => $form_state['accession'] . '-',
    ),
  );

  $options = array(
    'cvterms' => $cvterms,
    'records' => $records,
    'overrides' => $overrides,
    'locations' => &$form_state['locations'],
    'accession' => $form_state['accession'],
    'single_file' => empty($thirdpage['tree-accession']['check']),
    'org_names' => $firstpage['organism'],
    'saved_ids' => &$form_state['ids'],
    'stock_count' => &$stock_count,
    'multi_insert' => $multi_insert_options,
    'tree_info' => &$form_state['tree_info'],
  );

  for ($i = 1; $i <= $organism_number; $i++) {
    $tree_accession = $thirdpage['tree-accession']["species-$i"];
    $fid = $tree_accession['file'];

    tpps_add_project_file($form_state, $fid);

    $column_vals = $tree_accession['file-columns'];
    $groups = $tree_accession['file-groups'];

    $options['org_num'] = $i;
    $options['no_header'] = !empty($tree_accession['file-no-header']);
    $options['empty'] = $tree_accession['file-empty'];
    $options['pop_group'] = $tree_accession['pop-group'];
    $county = array_search('8', $column_vals);
    $district = array_search('9', $column_vals);
    $clone = array_search('13', $column_vals);
    $options['column_ids'] = array(
      'id' => $groups['Tree Id']['1'],
      'lat' => $groups[$loc_name]['4'] ?? NULL,
      'lng' => $groups[$loc_name]['5'] ?? NULL,
      'country' => $groups[$loc_name]['2'] ?? NULL,
      'state' => $groups[$loc_name]['3'] ?? NULL,
      'county' => ($county !== FALSE) ? $county : NULL,
      'district' => ($district !== FALSE) ? $district : NULL,
      'clone' => ($clone !== FALSE) ? $clone : NULL,
      'pop_group' => $groups[$loc_name]['12'] ?? NULL,
    );

    if ($organism_number != 1 and empty($thirdpage['tree-accession']['check'])) {
      if ($groups['Genus and Species']['#type'] == 'separate') {
        $options['column_ids']['genus'] = $groups['Genus and Species']['6'];
        $options['column_ids']['species'] = $groups['Genus and Species']['7'];
      }
      else {
        $options['column_ids']['org'] = $groups['Genus and Species']['10'];
      }
    }

    tpps_file_iterator($fid, 'tpps_process_accession', $options);

    $new_ids = tpps_chado_insert_multi($options['records'], $multi_insert_options);
    foreach ($new_ids as $t_id => $stock_id) {
      $form_state['tree_info'][$t_id]['stock_id'] = $stock_id;
    }
    unset($options['records']);
    $stock_count = 0;
    if (empty($thirdpage['tree-accession']['check'])) {
      break;
    }
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
      elseif ($ref_genome === 'bio') {
        $eutils = $fourthpage["organism-$i"]['genotype']['tripal_eutils'];
        $class = 'EutilsImporter';
        tripal_load_include_importer_class($class);

        $run_args = array(
          'importer_class' => $class,
          'db' => $eutils['db'],
          'accession' => $eutils['accession'],
          'linked_records' => $eutils['options']['linked_records']
        );

        try {
          $importer = new $class();
          $importer->create($run_args);
          $importer->submitJob();
        }
        catch (Exception $ex) {
          drupal_set_message('Cannot submit BioProject: ' . $ex->getMessage(), 'error');
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
        'type_id' => tpps_load_cvterm('analysis_type')->cvterm_id,
        'value' => $label,
      ));

      $fid = $form_state['saved_values']['summarypage']['analysis']["{$option}_file"];
      if (!empty($fid)) {
        tpps_add_project_file($form_state, $fid);

        tpps_chado_insert_record('projectprop', array(
          'project_id' => $form_state['ids']['project_id'],
          'type_id' => tpps_load_cvterm('source_description')->cvterm_id,
          'value' => $form_state['saved_values']['summarypage']['analysis']["{$option}_file_description"],
        ));
      }
    }
  }

  if (!empty($form_state['saved_values']['summarypage']['tree_pictures'])) {
    foreach ($form_state['saved_values']['summarypage']['tree_pictures'] as $name => $fid) {
      if (!empty($fid)) {
        $form_state['file_info']['summarypage'][$fid] = implode('_', explode(' ', $name)) . '.jpg';
      }
    }
  }
}

/**
 * This function processes a single row of an accession file.
 *
 * This function is meant to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_accession($row, array &$options) {
  $cvterm = $options['cvterms'];
  $records = &$options['records'];
  $accession = $options['accession'];
  $cols = $options['column_ids'];
  $saved_ids = &$options['saved_ids'];
  $stock_count = &$options['stock_count'];
  $multi_insert_options = $options['multi_insert'];
  $tree_info = &$options['tree_info'];
  $record_group = variable_get('tpps_record_group', 10000);
  $geo_api_key = variable_get('tpps_geocode_api_key', NULL);

  $tree_id = $row[$cols['id']];
  $id = $saved_ids['organism_ids'][$options['org_num']];
  if ($options['org_names']['number'] != 1 and $options['single_file']) {
    $org_full_name = $row[$cols['org']] ?? "{$row[$cols['genus']]} {$row[$cols['species']]}";
    $id = $saved_ids['organism_ids'][array_search($org_full_name, $options['org_names'])];
  }

  $records['stock'][$tree_id] = array(
    'uniquename' => "$accession-$tree_id",
    'type_id' => $cvterm['org'],
    'organism_id' => $id,
  );
  $tree_info[$tree_id] = array(
    'organism_id' => $id,
  );

  $records['project_stock'][$tree_id] = array(
    'project_id' => $saved_ids['project_id'],
    '#fk' => array(
      'stock' => $tree_id,
    ),
  );

  if (isset($row[$cols['clone']]) and $row[$cols['clone']] !== $options['empty']) {
    $clone_name = $tree_id . '-' . $row[$cols['clone']];

    $records['stock'][$clone_name] = array(
      'uniquename' => $accession . '-' . $clone_name,
      'type_id' => $cvterm['clone'],
      'organism_id' => $id,
    );
    $tree_info[$clone_name] = array(
      'organism_id' => $id,
    );

    $records['project_stock'][$clone_name] = array(
      'project_id' => $saved_ids['project_id'],
      '#fk' => array(
        'stock' => $clone_name,
      ),
    );

    $records['stock_relationship'][$clone_name] = array(
      'type_id' => $cvterm['has_part'],
      '#fk' => array(
        'subject' => $tree_id,
        'object' => $clone_name,
      ),
    );

    $tree_id = $clone_name;
  }

  if (!empty($row[$cols['lat']]) and !empty($row[$cols['lng']])) {
    $raw_coord = $row[$cols['lat']] . ',' . $row[$cols['lng']];
    $standard_coord = explode(',', tpps_standard_coord($raw_coord));
    $lat = $standard_coord[0];
    $lng = $standard_coord[1];
  }
  elseif (!empty($row[$cols['state']]) and !empty($row[$cols['country']])) {
    $records['stockprop']["$tree_id-country"] = array(
      'type_id' => $cvterm['country'],
      'value' => $row[$cols['country']],
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );

    $records['stockprop']["$tree_id-state"] = array(
      'type_id' => $cvterm['state'],
      'value' => $row[$cols['state']],
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );

    $location = "{$row[$cols['state']]}, {$row[$cols['country']]}";

    if (!empty($row[$cols['county']])) {
      $records['stockprop']["$tree_id-county"] = array(
        'type_id' => $cvterm['county'],
        'value' => $row[$cols['county']],
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );
      $location = "{$row[$cols['county']]}, $location";
    }

    if (!empty($row[$cols['district']])) {
      $records['stockprop']["$tree_id-district"] = array(
        'type_id' => $cvterm['district'],
        'value' => $row[$cols['district']],
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );
      $location = "{$row[$cols['district']]}, $location";
    }

    $tree_info[$tree_id]['location'] = $location;

    if (isset($geo_api_key)) {
      if (!array_key_exists($location, $options['locations'])) {
        $query = urlencode($location);
        $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
        $response = json_decode(file_get_contents($url));

        if ($response->total_results) {
          $results = $response->results;
          $result = $results[0]->geometry;
          if ($response->total_results > 1 and !isset($cols['district']) and !isset($cols['county'])) {
            foreach ($results as $item) {
              if ($item->components->_type == 'state') {
                $result = $item->geometry;
                break;
              }
            }
          }
        }
        $options['locations'][$location] = $result ?? NULL;
      }
      else {
        $result = $options['locations'][$location];
      }

      if (!empty($result)) {
        $lat = $result->lat;
        $lng = $result->lng;
      }
    }
  }
  else {
    $location = $options['pop_group'][$row[$cols['pop_group']]];
    $coord = tpps_standard_coord($location);

    if ($coord) {
      $parts = explode(',', $coord);
      $lat = $parts[0];
      $lng = $parts[1];
    }
    else {
      $records['stockprop']["$tree_id-location"] = array(
        'type_id' => $cvterm['loc'],
        'value' => $location,
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );

      $tree_info[$tree_id]['location'] = $location;

      if (isset($geo_api_key)) {
        if (!array_key_exists($location, $options['locations'])) {
          $query = urlencode($location);
          $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
          $response = json_decode(file_get_contents($url));
          $result = ($response->total_results) ? $response->results[0]->geometry : NULL;
          $options['locations'][$location] = $result;
        }
        else {
          $result = $options['locations'][$location];
        }

        if (!empty($result)) {
          $lat = $result->lat;
          $lng = $result->lng;
        }
      }
    }
  }

  if (!empty($lat) and !empty($lng)) {
    $records['stockprop']["$tree_id-lat"] = array(
      'type_id' => $cvterm['lat'],
      'value' => $lat,
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );

    $records['stockprop']["$tree_id-long"] = array(
      'type_id' => $cvterm['lng'],
      'value' => $lng,
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );
    $tree_info[$tree_id]['lat'] = $lat;
    $tree_info[$tree_id]['lng'] = $lng;
  }

  $stock_count++;
  if ($stock_count >= $record_group) {
    $new_ids = tpps_chado_insert_multi($records, $multi_insert_options);
    foreach ($new_ids as $t_id => $stock_id) {
      $tree_info[$t_id]['stock_id'] = $stock_id;
    }

    $records = array(
      'stock' => array(),
      'stockprop' => array(),
      'stock_relationship' => array(),
      'project_stock' => array(),
    );
    $stock_count = 0;
  }
}

/**
 * Cleans unnecessary information from the form state.
 *
 * Uses tpps_form_state_info() as a helper function.
 *
 * @param array $form_state
 *   The form state to be cleaned.
 */
function tpps_clean_state(array &$form_state) {
  $new = array();
  unset($form_state['ids']);
  tpps_form_state_info($new, $form_state);
  $form_state = $new;
}
