<?php

/**
 * @file
 */

/**
 *
 */
function tpps_submit_all(&$form_state) {

  $values = $form_state['saved_values'];
  $firstpage = $values[PAGE_1];
  $file_rank = 0;

  $project_id = tpps_chado_insert_record('project', array(
    'name' => $firstpage['publication']['title'],
    'description' => $firstpage['publication']['abstract'],
  ));

  dpm($project_id);

  $organism_ids = tpps_submit_page_1($form_state, $project_id, $file_rank);

  tpps_submit_page_2($form_state, $project_id, $file_rank);

  tpps_submit_page_3($form_state, $project_id, $file_rank, $organism_ids);

  tpps_submit_page_4($form_state, $project_id, $file_rank, $organism_ids);

  // For simplicity and efficiency, all fourth page submissions take place in the TPPS File Parsing Tripal Job.
}

/**
 *
 */
function tpps_submit_page_1(&$form_state, $project_id, &$file_rank) {

  $dbxref_id = $form_state['dbxref_id'];
  $firstpage = $form_state['saved_values'][PAGE_1];

  tpps_chado_insert_record('project_dbxref', array(
    'project_id' => $project_id,
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
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'schema',
        ),
        'name' => 'url',
        'is_obsolete' => 0,
      ),
      'value' => file_create_url(file_load($firstpage['publication']['secondaryAuthors']['file'])->uri),
      'rank' => $file_rank,
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
    $file_rank++;
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
    'project_id' => $project_id,
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
    $content = tpps_parse_xlsx($location);
    $column_vals = $firstpage['publication']['secondaryAuthors']['file-columns'];
    $groups = $firstpage['publication']['secondaryAuthors']['file-groups'];

    if (!empty($firstpage['publication']['secondaryAuthors']['file-no-header'])) {
      tpps_content_no_header($content);
    }

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

  $organism_ids = array();
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
    $organism_ids[$i] = tpps_chado_insert_record('organism', array(
      'genus' => $genus,
      'species' => $species,
      'infraspecific_name' => $infra,
    ));
    tpps_chado_insert_record('project_organism', array(
      'organism_id' => $organism_ids[$i],
      'project_id' => $project_id,
    ));
  }
  return $organism_ids;
}

/**
 *
 */
function tpps_submit_page_2(&$form_state, $project_id, &$file_rank) {

  $secondpage = $form_state['saved_values'][PAGE_2];

  $start = $secondpage['StartingDate']['month'] . " " . $secondpage['StartingDate']['year'];
  $end = $secondpage['EndingDate']['month'] . " " . $secondpage['EndingDate']['year'];

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $project_id,
  // This cvterm was created custom for TPPS.
    'type_id' => array(
      'cv_id' => array(
        'name' => 'local',
      ),
      'name' => 'study_start',
      'is_obsolete' => 0,
    ),
    'value' => $start,
  ));

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $project_id,
  // This cvterm was created custom for TPPS.
    'type_id' => array(
      'cv_id' => array(
        'name' => 'local',
      ),
      'name' => 'study_end',
      'is_obsolete' => 0,
    ),
    'value' => $end,
  ));

  if ($secondpage['studyLocation']['type'] !== '2') {
    $standard_coordinate = explode(',', tpps_standard_coord($secondpage['studyLocation']['coordinates']));
    $latitude = $standard_coordinate[0];
    $longitude = $standard_coordinate[1];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
    // This cvterm was created custom for TPPS.
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'gps_latitude',
        'is_obsolete' => 0,
      ),
      'value' => $latitude,
    ));

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
    // This cvterm was created custom for TPPS.
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'gps_longitude',
        'is_obsolete' => 0,
      ),
      'value' => $longitude,
    ));
  }
  else {
    $location = $secondpage['studyLocation']['custom'];

    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
    // This cvterm was created custom for TPPS.
      'type_id' => array(
        'cv_id' => array(
          'name' => 'local',
        ),
        'name' => 'experiment_location',
        'is_obsolete' => 0,
      ),
      'value' => $location,
    ));
  }

  $datatype = $secondpage['dataType'];

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $project_id,
  // This cvterm was created custom for TPPS.
    'type_id' => array(
      'cv_id' => array(
        'name' => 'local',
      ),
      'name' => 'association_results_type',
      'is_obsolete' => 0,
    ),
    'value' => $datatype,
  ));

  $studytype_options = array(
    0 => '- Select -',
    1 => 'Natural Population (Landscape)',
    2 => 'Growth Chamber',
    3 => 'Greenhouse',
    4 => 'Experimental/Common Garden',
    5 => 'Plantation',
  );

  $study_type = $studytype_options[$secondpage['studyType']];

  tpps_chado_insert_record('projectprop', array(
    'project_id' => $project_id,
  // This cvterm was created custom for TPPS.
    'type_id' => array(
      'cv_id' => array(
        'name' => 'local',
      ),
      'name' => 'study_type',
      'is_obsolete' => 0,
    ),
    'value' => $study_type,
  ));

  switch ($secondpage['studyType']) {
    case ('1'):
      $natural_population = $secondpage['naturalPopulation'];
      $number_assessions = $natural_population['assessions'];
      $seasons = "";
      foreach ($natural_population['season'] as $key => $item) {
        if ($item != '0') {
          $seasons .= $key . ', ';
        }
      }

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'assession_season',
          'is_obsolete' => 0,
        ),
        'value' => $seasons,
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'assession_number',
          'is_obsolete' => 0,
        ),
        'value' => $number_assessions,
      ));
      break;

    case ('2'):
      $growth_chamber = $secondpage['growthChamber'];
      $co2 = $growth_chamber['co2Control'];
      $humidity = $growth_chamber['humidityControl'];
      $light = $growth_chamber['lightControl'];
      $temp_high = $growth_chamber['temp']['high'];
      $temp_low = $growth_chamber['temp']['low'];
      $rooting = $growth_chamber['rooting'];
      $rooting_type = $rooting['option'];
      $soil = $rooting['soil'];
      $soil_container = $soil['container'];
      $ph = $rooting['ph'];
      $treatments = $rooting['treatment'];

      if ($co2['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'co2_control',
            'is_obsolete' => 0,
          ),
          'value' => 'True',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'co2_level',
            'is_obsolete' => 0,
          ),
          'value' => $co2['controlled'],
        ));
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'co2_control',
            'is_obsolete' => 0,
          ),
          'value' => 'False',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'co2_level',
            'is_obsolete' => 0,
          ),
          'value' => $co2['uncontrolled'],
        ));
      }

      if ($humidity['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'humidity_control',
            'is_obsolete' => 0,
          ),
          'value' => 'True',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'humidity_level',
            'is_obsolete' => 0,
          ),
          'value' => $humidity['controlled'],
        ));
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'humidity_control',
            'is_obsolete' => 0,
          ),
          'value' => 'False',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'humidity_level',
            'is_obsolete' => 0,
          ),
          'value' => $humidity['uncontrolled'],
        ));
      }

      if ($light['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'light_control',
            'is_obsolete' => 0,
          ),
          'value' => 'True',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'light_level',
            'is_obsolete' => 0,
          ),
          'value' => $light['controlled'],
        ));
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'light_control',
            'is_obsolete' => 0,
          ),
          'value' => 'False',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'light_level',
            'is_obsolete' => 0,
          ),
          'value' => $light['uncontrolled'],
        ));
      }

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'temperature_high',
          'is_obsolete' => 0,
        ),
        'value' => $temp_high,
      ));
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'temperature_low',
          'is_obsolete' => 0,
        ),
        'value' => $temp_low,
      ));

      switch ((string) $rooting_type) {
        case '1':
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'rooting_type',
              'is_obsolete' => 0,
            ),
            'value' => 'Aeroponics',
          ));
          break;

        case '2':
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'rooting_type',
              'is_obsolete' => 0,
            ),
            'value' => 'Hydroponics',
          ));
          break;

        case '3':
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'rooting_type',
              'is_obsolete' => 0,
            ),
            'value' => 'Soil',
          ));
          $soil_options = array(
            0 => '- Select -',
            1 => 'Sand',
            2 => 'Peat',
            3 => 'Clay',
            4 => 'Mixed',
            5 => 'Other',
          );
          $soil_type = $soil_options[$soil['type']];
          if ($soil_type == 'Other') {
            $soil_type = $soil['other'];
          }

          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'soil_type',
              'is_obsolete' => 0,
            ),
            'value' => $soil_type,
          ));
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'soil_container',
              'is_obsolete' => 0,
            ),
            'value' => $soil_container,
          ));
          break;

        default:
          break;
      }

      if ($ph['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'pH_control',
            'is_obsolete' => 0,
          ),
          'value' => 'True',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'pH_level',
            'is_obsolete' => 0,
          ),
          'value' => $ph['controlled'],
        ));
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'pH_control',
            'is_obsolete' => 0,
          ),
          'value' => 'False',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'pH_level',
            'is_obsolete' => 0,
          ),
          'value' => $ph['uncontrolled'],
        ));
      }

      $is_description = FALSE;
      $rank = 0;
      foreach ($treatments as $item) {
        if (!$is_description) {
          if ($item == '1') {
            $record_next = TRUE;
          }
          else {
            $record_next = FALSE;
          }
          $is_description = TRUE;
        }
        else {
          if ($record_next) {
            tpps_chado_insert_record('projectprop', array(
              'project_id' => $project_id,
            // This cvterm was created custom for TPPS.
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'treatment',
                'is_obsolete' => 0,
              ),
              'value' => $item,
              'rank' => $rank,
            ));
            $rank++;
          }
          $is_description = FALSE;
        }
      }
      break;

    case ('3'):
      $greenhouse = $secondpage['greenhouse'];
      $humidity = $greenhouse['humidityControl'];
      $light = $greenhouse['lightControl'];
      $temp_high = $greenhouse['temp']['high'];
      $temp_low = $greenhouse['temp']['low'];
      $rooting = $greenhouse['rooting'];
      $rooting_type = $rooting['option'];
      $soil = $rooting['soil'];
      $soil_container = $soil['container'];
      $ph = $rooting['ph'];
      $treatments = $rooting['treatment'];

      if ($humidity['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'humidity_control',
            'is_obsolete' => 0,
          ),
          'value' => 'True',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'humidity_level',
            'is_obsolete' => 0,
          ),
          'value' => $humidity['controlled'],
        ));
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'humidity_control',
            'is_obsolete' => 0,
          ),
          'value' => 'False',
        ));
      }

      if ($light['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'light_control',
            'is_obsolete' => 0,
          ),
          'value' => 'True',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'light_level',
            'is_obsolete' => 0,
          ),
          'value' => $light['controlled'],
        ));
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'light_control',
            'is_obsolete' => 0,
          ),
          'value' => 'False',
        ));
      }

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'temperature_high',
          'is_obsolete' => 0,
        ),
        'value' => $temp_high,
      ));
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'temperature_high',
          'is_obsolete' => 0,
        ),
        'value' => $temp_low,
      ));

      switch ((string) $rooting_type) {
        case '1':
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'rooting_type',
              'is_obsolete' => 0,
            ),
            'value' => 'Aeroponics',
          ));
          break;

        case '2':
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'rooting_type',
              'is_obsolete' => 0,
            ),
            'value' => 'Hydroponics',
          ));
          break;

        case '3':
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'rooting_type',
              'is_obsolete' => 0,
            ),
            'value' => 'Soil',
          ));
          $soil_options = array(
            0 => '- Select -',
            1 => 'Sand',
            2 => 'Peat',
            3 => 'Clay',
            4 => 'Mixed',
            5 => 'Other',
          );
          $soil_type = $soil_options[$soil['type']];
          if ($soil_type == 'Other') {
            $soil_type = $soil['other'];
          }

          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'soil_type',
              'is_obsolete' => 0,
            ),
            'value' => $soil_type,
          ));
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'soil_container',
              'is_obsolete' => 0,
            ),
            'value' => $soil_container,
          ));
          break;

        default:
          break;
      }

      if ($ph['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'pH_control',
            'is_obsolete' => 0,
          ),
          'value' => 'True',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'pH_level',
            'is_obsolete' => 0,
          ),
          'value' => $ph['controlled'],
        ));
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'pH_control',
            'is_obsolete' => 0,
          ),
          'value' => 'False',
        ));
      }

      $is_description = FALSE;
      $rank = 0;
      foreach ($treatments as $item) {
        if (!$is_description) {
          if ($item == '1') {
            $record_next = TRUE;
          }
          else {
            $record_next = FALSE;
          }
          $is_description = TRUE;
        }
        else {
          if ($record_next) {
            tpps_chado_insert_record('projectprop', array(
              'project_id' => $project_id,
            // This cvterm was created custom for TPPS.
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'treatment',
                'is_obsolete' => 0,
              ),
              'value' => $item,
              'rank' => $rank,
            ));
            $rank++;
          }
          $is_description = FALSE;
        }
      }
      break;

    case ('4'):
      $commonGarden = $secondpage['commonGarden'];
      $salinity = $commonGarden['salinity'];
      $biotic_env = $commonGarden['bioticEnv']['option'];
      $seasons = "";
      $treatments = $commonGarden['treatment'];

      $irrigation_options = array(
        0 => '- Select -',
        1 => 'Irrigation from top',
        2 => 'Irrigation from bottom',
        3 => 'Drip Irrigation',
        4 => 'Other',
        5 => 'No Irrigation',
      );
      $irrigation_type = $irrigation_options[$commonGarden['irrigation']['option']];
      if ($irrigation_type == 'Other') {
        $irrigation_type = $commonGarden['irrigation']['other'];
      }

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'irrigation_type',
          'is_obsolete' => 0,
        ),
        'value' => $irrigation_type,
      ));

      if ($salinity['option'] == '1') {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'salinity_control',
            'is_obsolete' => 0,
          ),
          'value' => 'True',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'salinity_level',
            'is_obsolete' => 0,
          ),
          'value' => $salinity['controlled'],
        ));
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'salinity_control',
            'is_obsolete' => 0,
          ),
          'value' => 'False',
        ));
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'salinity_level',
            'is_obsolete' => 0,
          ),
          'value' => $salinity['uncontrolled'],
        ));
      }

      $biotic_env['Other'] = $commonGarden['bioticEnv']['other'];
      foreach ($biotic_env as $key => $check) {
        if ($check == '1') {
          tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'biotic_environment',
              'is_obsolete' => 0,
            ),
            'value' => $key,
          ));
        }
      }

      foreach ($commonGarden['season'] as $key => $item) {
        if ($item == '1') {
          $seasons .= $key . ', ';
        }
      }
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'assession_season',
          'is_obsolete' => 0,
        ),
        'value' => $seasons,
      ));

      $is_description = FALSE;
      $rank = 0;
      foreach ($treatments as $item) {
        if (!$is_description) {
          if ($item == '1') {
            $record_next = TRUE;
          }
          else {
            $record_next = FALSE;
          }
          $is_description = TRUE;
        }
        else {
          if ($record_next) {
            tpps_chado_insert_record('projectprop', array(
              'project_id' => $project_id,
            // This cvterm was created custom for TPPS.
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'treatment',
                'is_obsolete' => 0,
              ),
              'value' => $item,
              'rank' => $rank,
            ));
            $rank++;
          }
          $is_description = FALSE;
        }
      }
      break;

    case ('5'):
      $plantation = $secondpage['plantation'];
      $number_assessions = $plantation['assessions'];
      $seasons = "";
      $treatments = $plantation['treatment'];

      foreach ($plantation['season'] as $key => $item) {
        if ($item == '1') {
          $seasons .= $key . ', ';
        }
      }

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'assession_season',
          'is_obsolete' => 0,
        ),
        'value' => $seasons,
      ));

      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
      // This cvterm was created custom for TPPS.
        'type_id' => array(
          'cv_id' => array(
            'name' => 'local',
          ),
          'name' => 'assession_number',
          'is_obsolete' => 0,
        ),
        'value' => $number_assessions,
      ));

      if ($plantation['treatment']['check'] != 0) {
        $is_description = FALSE;
        $rank = 0;
        foreach ($treatments as $item) {
          if (!$is_description) {
            if ($item == '1') {
              $record_next = TRUE;
            }
            else {
              $record_next = FALSE;
            }
            $is_description = TRUE;
          }
          else {
            if ($record_next) {
              tpps_chado_insert_record('projectprop', array(
                'project_id' => $project_id,
              // This cvterm was created custom for TPPS.
                'type_id' => array(
                  'cv_id' => array(
                    'name' => 'local',
                  ),
                  'name' => 'treatment',
                  'is_obsolete' => 0,
                ),
                'value' => $item,
                'rank' => $rank,
              ));
              $rank++;
            }
            $is_description = FALSE;
          }
        }
      }
      break;

    default:
      break;
  }
}

/**
 *
 */
function tpps_submit_page_3(&$form_state, $project_id, &$file_rank, $organism_ids) {

  $firstpage = $form_state['saved_values'][PAGE_1];
  $thirdpage = $form_state['saved_values'][PAGE_3];
  $organism_number = $firstpage['organism']['number'];

  $stock_ids = array();

  if ($organism_number == '1' or $thirdpage['tree-accession']['check'] == 0) {
    // Single file.
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $project_id,
      'type_id' => array(
        'cv_id' => array(
          'name' => 'schema',
        ),
        'name' => 'url',
        'is_obsolete' => 0,
      ),
      'value' => file_create_url(file_load($thirdpage['tree-accession']['file'])->uri),
      'rank' => $file_rank,
    ));

    $file = file_load($thirdpage['tree-accession']['file']);
    $location = drupal_realpath($file->uri);
    $content = tpps_parse_xlsx($location);
    $column_vals = $thirdpage['tree-accession']['file-columns'];
    $groups = $thirdpage['tree-accession']['file-groups'];

    foreach ($column_vals as $col => $val) {
      if ($val == '8') {
        $county_col_name = $col;
      }
      if ($val == '9') {
        $district_col_name = $col;
      }
    }

    $id_col_accession_name = $groups['Tree Id']['1'];

    if ($organism_number == '1') {
      // Only one species.
      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        $stock_ids[$tree_id] = tpps_chado_insert_record('stock', array(
          'uniquename' => t($tree_id),
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'obi',
            ),
            'name' => 'organism',
            'is_obsolete' => 0,
          ),
          'organism_id' => $organism_ids[1],
        ));
      }
    }
    else {
      // Multiple species in one tree accession file -> users must define species and genus columns
      // get genus/species column.
      if ($groups['Genus and Species']['#type'] == 'separate') {
        $genus_col_name = $groups['Genus and Species']['6'];
        $species_col_name = $groups['Genus and Species']['7'];
      }
      else {
        $org_col_name = $groups['Genus and Species']['10'];
      }

      // Parse file.
      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        for ($j = 1; $j <= $organism_number; $j++) {
          // Match genus and species to genus and species given on page 1.
          if ($groups['Genus and Species']['#type'] == 'separate') {
            $genus_full_name = "{$content[$i][$genus_col_name]} {$content[$i][$species_col_name]}";
          }
          else {
            $genus_full_name = "{$content[$i][$org_col_name]}";
          }

          if ($firstpage['organism'][$j] == $genus_full_name) {
            // Obtain organism id from matching species.
            $id = $organism_ids[$j];
            break;
          }
        }

        // Create record with the new id.
        $stock_ids[$tree_id] = tpps_chado_insert_record('stock', array(
          'uniquename' => t($tree_id),
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'obi',
            ),
            'name' => 'organism',
            'is_obsolete' => 0,
          ),
          'organism_id' => $id,
        ));
      }
    }

    if ($groups['Location (latitude/longitude or country/state or population group)']['#type'] == 'gps') {
      $lat_name = $groups['Location (latitude/longitude or country/state or population group)']['4'];
      $long_name = $groups['Location (latitude/longitude or country/state or population group)']['5'];

      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        $stock_id = $stock_ids[$tree_id];

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'gps_latitude',
            'is_obsolete' => 0,
          ),
          'value' => $content[$i][$lat_name],
        ));

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'local',
            ),
            'name' => 'gps_longitude',
            'is_obsolete' => 0,
          ),
          'value' => $content[$i][$long_name],
        ));
      }
    }
    elseif ($groups['Location (latitude/longitude or country/state or population group)']['#type'] == 'approx') {
      $country_col_name = $groups['Location (latitude/longitude or country/state or population group)']['2'];
      $state_col_name = $groups['Location (latitude/longitude or country/state or population group)']['3'];

      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        $stock_id = $stock_ids[$tree_id];

        tpps_chado_insert_record('stockprop', array(
          'stock_id' => $stock_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'tripal_contact',
            ),
            'name' => 'Country',
            'is_obsolete' => 0,
          ),
          'value' => $content[$i][$country_col_name],
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
          'value' => $content[$i][$state_col_name],
        ));

        if (isset($county_col_name)) {
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'county',
              'is_obsolete' => 0,
            ),
            'value' => $content[$i][$county_col_name],
          ));
        }

        if (isset($district_col_name)) {
          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'district',
              'is_obsolete' => 0,
            ),
            'value' => $content[$i][$district_col_name],
          ));
        }
      }
    }
    else {
      $pop_group_name = $groups['Location (latitude/longitude or country/state or population group)']['12'];

      for ($i = 0; $i < count($content) - 1; $i++) {
        $tree_id = $content[$i][$id_col_accession_name];
        $stock_id = $stock_ids[$tree_id];

        $loc = $thirdpage['tree-accession']['pop-group'][$content[$i][$pop_group_name]];

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
      }
    }

    $file->status = FILE_STATUS_PERMANENT;
    $file = file_save($file);
    $file_rank++;
  }
  else {
    // Multiple files, sorted by species.
    for ($i = 1; $i <= $organism_number; $i++) {
      tpps_chado_insert_record('projectprop', array(
        'project_id' => $project_id,
        'type_id' => array(
          'cv_id' => array(
            'name' => 'schema',
          ),
          'name' => 'url',
          'is_obsolete' => 0,
        ),
        'value' => drupal_realpath(file_load($thirdpage['tree-accession']["species-$i"]['file'])->uri),
        'rank' => $file_rank,
      ));

      $file = file_load($thirdpage['tree-accession']["species-$i"]['file']);
      $location = drupal_realpath($file->uri);
      $content = tpps_parse_xlsx($location);
      $column_vals = $thirdpage['tree-accession']["species-$i"]['file-columns'];
      $groups = $thirdpage['tree-accession']["species-$i"]['file-groups'];

      $id_col_accession_name = $groups['Tree Id']['1'];

      foreach ($column_vals as $col => $val) {
        if ($val == '8') {
          $county_col_name = $col;
        }
        if ($val == '9') {
          $district_col_name = $col;
        }
      }

      for ($j = 0; $j < count($content) - 1; $j++) {
        $tree_id = $content[$j][$id_col_accession_name];
        $stock_ids[$tree_id] = tpps_chado_insert_record('stock', array(
          'uniquename' => t($tree_id),
        // This cvterm was created custom for TPPS.
          'type_id' => array(
            'cv_id' => array(
              'name' => 'obi',
            ),
            'name' => 'organism',
            'is_obsolete' => 0,
          ),
          'organism_id' => $organism_ids[$i],
        ));

        if ($groups['Location (latitude/longitude or country/state or population group)']['#type'] == 'gps') {
          $lat_name = $groups['Location (latitude/longitude or country/state or population group)']['4'];
          $long_name = $groups['Location (latitude/longitude or country/state or population group)']['5'];

          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_ids[$tree_id],
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'gps_latitude',
              'is_obsolete' => 0,
            ),
            'value' => $content[$j][$lat_name],
          ));

          tpps_chado_insert_record('stockprop', array(
            'stock_id' => $stock_ids[$tree_id],
          // This cvterm was created custom for TPPS.
            'type_id' => array(
              'cv_id' => array(
                'name' => 'local',
              ),
              'name' => 'gps_longitude',
              'is_obsolete' => 0,
            ),
            'value' => $content[$j][$long_name],
          ));
        }
        elseif ($groups['Location (latitude/longitude or country/state or population group)']['#type'] == 'approx') {
          $country_col_name = $groups['Location (latitude/longitude or country/state or population group)']['2'];
          $state_col_name = $groups['Location (latitude/longitude or country/state or population group)']['3'];

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

          if (isset($county_col_name)) {
            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'county',
                'is_obsolete' => 0,
              ),
              'value' => $content[$j][$county_col_name],
            ));
          }

          if (isset($district_col_name)) {
            tpps_chado_insert_record('stockprop', array(
              'stock_id' => $stock_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'local',
                ),
                'name' => 'district',
                'is_obsolete' => 0,
              ),
              'value' => $content[$j][$district_col_name],
            ));
          }
        }
        else {
          $pop_group_name = $groups['Location (latitude/longitude or country/state or population group)']['12'];

          $loc = $thirdpage['tree-accession']['pop-group'][$content[$j][$pop_group_name]];

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
        }
      }

      $file->status = FILE_STATUS_PERMANENT;
      $file = file_save($file);
      $file_rank++;
    }
  }

  foreach ($stock_ids as $tree_id => $stock_id) {
    tpps_chado_insert_record('project_stock', array(
      'stock_id' => $stock_id,
      'project_id' => $project_id,
    ));
  }

  $form_state['file_rank'] = $file_rank;

}

/**
 *
 */
function tpps_submit_page_4(&$form_state, $project_id, &$file_rank, $organism_ids) {
  $fourthpage = $form_state['saved_values'][PAGE_4];
  $organism_number = $form_state['saved_values'][PAGE_1]['organism']['number'];

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
        $organism_id = $organism_ids[$i];
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
