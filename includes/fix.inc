<?php

/**
 * @file
 * Fixes. Not for production.
 *
 * To use:
 * module_load_include('inc', 'tpps', 'includes/fix');
 *
 *
 * To restore 'tree_info':
 * $a = 'TGDR1051';
 * module_load_include('inc', 'tpps', 'includes/fix');
 * dpm(print_r($list = __fix_tree_info($a), 1));
 *
 * // $s = new Submission($a);
 * // $s->state['tree_info'] = $s->sharedState['tree_info'] = $list;
 * // $s->save();
 */

/**
 * Restores 'tree_info' which is used to show map on study details page.
 *
 * WARNING: Do not use! For critical cases only. Not tested well.
 *
 * Could be used when study was processed but
 * $submission->sharedState['tree_info'] was lost.
 *
 * See function tpps_submit_page_3().
 *
 * Note: there will be no 'stock_id'.
 *
 * @return array
 *   Returns list of all tree's ids and locations.
 *
 * @TODO Optimize and move this code to class 'Accession'.
 */
function __fix_tree_info($accession) {
  $submission = new Submission($accession);
  if ($submission->doesNotExist()) {
    return [];
  }

  $page1_values = $submission->sharedState['saved_values'][TPPS_PAGE_1];
  $page3_values = $submission->sharedState['saved_values'][TPPS_PAGE_3];
  $shared_state = $submission->sharedState;

  $loc_name = 'Location (latitude/longitude or country/state or population group)';
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
    'gps_type' => tpps_load_cvterm('gps_type')->cvterm_id,
    'precision' => tpps_load_cvterm('gps_precision')->cvterm_id,
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
      'prefix' => $shared_state['accession'] . '-',
    ),
  );

  $names = [];
  for ($i = 1; $i <= $submission->getOrganismNumber(); $i++) {
    $names[$i] = $page1_values['organism'][$i]['name'];
  }
  $names['number'] = $page1_values['organism']['number'];
  $options = array(
    'cvterms' => $cvterms,
    'records' => $records,
    'overrides' => $overrides,
    'locations' => &$shared_state['locations'],
    'accession' => $shared_state['accession'],
    'single_file' => empty($page3_values['tree-accession']['check']),
    'org_names' => $names,
    'saved_ids' => &$shared_state['ids'],
    'stock_count' => &$stock_count,
    'multi_insert' => $multi_insert_options,
    'tree_info' => &$shared_state['tree_info'],
    'job' => &$job,
  );

  for ($i = 1; $i <= $submission->getOrganismNumber(); $i++) {
    $tree_accession = $page3_values['tree-accession']["species-$i"];
    $fid = $tree_accession['file'];

    $column_vals = $tree_accession['file-columns'];
    $groups = $tree_accession['file-groups'];

    $options['org_num'] = $i;
    $options['no_header'] = !empty($tree_accession['file-no-header']);
    $options['empty'] = $tree_accession['file-empty'];
    $options['pop_group'] = $tree_accession['pop-group'];

    // [VS] #8669py308
    switch ($tree_accession['location_accuracy']) {
      case 'exact':
        $options['exact'] = TRUE;
        $options['precision'] = NULL;
        break;

      case 'approximate':
        $options['exact'] = NULL;
        $options['precision'] = $tree_accession['coord_precision'] ?? NULL;
        //$submission_tag_list = tpps_submission_load($shared_state['accession'])
        //  ->getTagList();
        //if (
        //  !empty($tag_id = SubmissionTag::getId('No Location Information'))
        //  && !array_key_exists($tag_id, $submission_tag_list)
        //) {
        //  tpps_submission_load($shared_state['accession'])
        //    ->addTag('Approximate Coordinates');
        //}
        break;

      case 'descriptive_place':
        // @TODO Major. Store value in database.
        $options['exact'] = NULL;
        $options['precision'] = $tree_accession['descriptive_place'] ?? NULL;
        break;
    }
    // [/VS] #8669py308

    $county = array_search(TPPS_COLUMN_COUNTY, $column_vals);
    $district = array_search(TPPS_COLUMN_DISTRICT, $column_vals);
    $clone = array_search(TPPS_COLUMN_CLONE_NUMBER, $column_vals);
    $options['column_ids'] = [
      'id' => $groups['Tree Id'][TPPS_COLUMN_PLANT_IDENTIFIER],
      'lat' => $groups[$loc_name][TPPS_COLUMN_LATITUDE] ?? NULL,
      'lng' => $groups[$loc_name][TPPS_COLUMN_LONGITUDE] ?? NULL,
      'country' => $groups[$loc_name][TPPS_COLUMN_COUNTRY] ?? NULL,
      'state' => $groups[$loc_name][TPPS_COLUMN_STATE] ?? NULL,
      'county' => ($county !== FALSE) ? $county : NULL,
      'district' => ($district !== FALSE) ? $district : NULL,
      'clone' => ($clone !== FALSE) ? $clone : NULL,
      'pop_group' => $groups[$loc_name][TPPS_COLUMN_POPULATION_GROUP] ?? NULL,
    ];

    if (
      $submission->getOrganismNumber() != 1
      and empty($page3_values['tree-accession']['check'])
    ) {
      if ($groups['Genus and Species']['#type'] == 'separate') {
        $options['column_ids']['genus'] = $groups['Genus and Species']['6'];
        $options['column_ids']['species'] = $groups['Genus and Species']['7'];
      }
      if ($groups['Genus and Species']['#type'] != 'separate') {
        $options['column_ids']['org'] = $groups['Genus and Species']['10'];
      }
    }

    module_load_include('php', 'tpps', 'forms/submit/submit_all');
    // tpps_process_accession().
    $callback = '__fix_tree_info_process_accession';
    tpps_file_iterator($fid, $callback, $options);

    if ($debug_mode ?? FALSE) {
      dpm(print_r($options['tree_info'], 1));
      dpm(print_r(array_keys($options), 1));
    }

    $new_ids = __fix_tree_info_chado_insert_multi($options['records'], $multi_insert_options);
    foreach ($new_ids as $t_id => $stock_id) {
      $options['tree_info'][$t_id]['stock_id'] = $stock_id;
    }
    return $options['tree_info'];

  }
}

/**
 * This function processes a single row of an accession file.
 *
 * WARNING: Do not use it!
 * This is a callback ONLY for '__fix_tree_info()' function.
 * Clone of the function tpps_process_accession().
 * Changes: disabled any changes in DB.
 *
 * This function is meant to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function __fix_tree_info_process_accession($row, array &$options, $job = NULL) {
  global $tpps_job;
  $job = $tpps_job;
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
  $site_based = FALSE;
  $exact = $options['exact'] ?? NULL;
  $precision = $options['precision'] ?? NULL;

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
    //tpps_log('CV Terms Data' . print_r($cvterm, 1), [], TRIPAL_INFO);
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
    $exact = FALSE;
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

    if (isset($geo_api_key) and !array_key_exists($location, $options['locations'])) {
      $query = urlencode($location);
      $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
      $response = json_decode(file_get_contents($url));
      $options['locations'][$location] = $response->results[0]->geometry ?? NULL;

      if ($response->total_results and $response->total_results > 1 and !isset($cols['district']) and !isset($cols['county'])) {
        foreach ($response->results as $item) {
          if ($item->components->_type == 'state') {
            $options['locations'][$location] = $item->geometry;
            break;
          }
        }
      }
    }
    $lat = $options['locations'][$location]->lat ?? NULL;
    $lng = $options['locations'][$location]->lng ?? NULL;
  }
  elseif (!empty($row[$cols['pop_group']])) {
    $site_based = TRUE;
    $location = $options['pop_group'][$row[$cols['pop_group']]];
    $coord = tpps_standard_coord($location);

    if ($coord) {
      $parts = explode(',', $coord);
      $lat = $parts[0];
      $lng = $parts[1];
    }

    if (!$coord) {
      $records['stockprop']["$tree_id-location"] = array(
        'type_id' => $cvterm['loc'],
        'value' => $location,
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );

      $tree_info[$tree_id]['location'] = $location;

      if (variable_get('tpps_submitall_skip_gps_request') && isset($geo_api_key)) {
        $result = $options['locations'][$location] ?? NULL;
        if (empty($result)) {
          $query = urlencode($location);
          $url = "https://api.opencagedata.com/geocode/v1/json?q=$query&key=$geo_api_key";
          $response = json_decode(file_get_contents($url));
          $result = ($response->total_results) ? $response->results[0]->geometry : NULL;
          $options['locations'][$location] = $result;
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

    $gps_type = "Site-based";
    if (!$site_based) {
      $gps_type = "Exact";
      if (!$exact) {
        $gps_type = "Approximate";
      }
    }

    $records['stockprop']["$tree_id-gps-type"] = array(
      'type_id' => $cvterm['gps_type'],
      'value' => $gps_type,
      '#fk' => array(
        'stock' => $tree_id,
      ),
    );

    if ($gps_type == "Approximate" and !empty($precision)) {
      $records['stockprop']["$tree_id-precision"] = array(
        'type_id' => $cvterm['precision'],
        'value' => $precision,
        '#fk' => array(
          'stock' => $tree_id,
        ),
      );
    }
  }
}

/**
 * Inserts multiple records into the chado database.
 *
 * This function will insert multiple records into multiple tables in the chado
 * database. This function is much faster than the tpps_chado_insert_record()
 * function with the 'multi' option set to true, because .
 *
 * @param array $record_groups
 *   An array of tables and the records that will be going into each table.
 * @param array $options
 *   Additional options.
 *
 * @return mixed
 *   An array of primary key ids on success. On failure, return FALSE.
 */
function __fix_tree_info_chado_insert_multi(array $record_groups, array $options = array()) {

  if (!isset($options['devel'])) {
    $options['devel'] = FALSE;
  }

  // Initialize insert statements, schemas, foreign keys, and primary keys.
  $inserts = array();
  $schemas = array();
  $primarys = array();
  $fks = array();
  foreach (array_keys($record_groups) as $table) {
    $current_records = &$record_groups[$table];
    if (empty($current_records)) {
      unset($record_groups[$table]);
      continue;
    }
    $schemas[$table] = chado_get_schema($table);
    if (!$schemas[$table]) {
      return FALSE;
    }

    if (!empty($options['fk_overrides'][$table])) {
      foreach ($options['fk_overrides'][$table] as $fk_type => $props) {
        $schemas[$table]['foreign keys'][$fk_type] = $props;
      }
    }

    $inserts[$table] = db_insert("chado.$table");
    $fields_arr = array();
    foreach (current($current_records) as $field => $val) {
      if ($field != '#fk') {
        $fields_arr[] = $field;
      }
      else {
        foreach ($val as $fk_type => $fk_val) {
          $fields_arr[] = key($schemas[$table]['foreign keys'][$fk_type]['columns']);
        }
      }
    }
    $inserts[$table]->fields($fields_arr);
    $primarys[$table] = !empty($schemas[$table]['primary key'][0]) ? $schemas[$table]['primary key'][0] : NULL;
    $fks[$table] = array();
  }

  if ($options['devel']) {
    $start_time = microtime(TRUE);
    print_r("\nstart time: " . date('r') . "\n");
    $times = array(
      'total' => 0,
      'unique' => 0,
      'unique_query' => 0,
      'unique_other' => 0,
      'insert' => 0,
      'other' => 0,
    );
  }

  foreach ($record_groups as $table => $records) {
    $schema = &$schemas[$table];
    $primary = &$primarys[$table];
    $fks_to_populate = array();
    $has_fields = FALSE;

    if ($options['devel']) {
      $start_unique = microtime(TRUE);
    }
    foreach ($records as $id => $values) {
      $unique = !empty($schema['unique keys']) ? $schema['unique keys'] : array();
      if (!empty($values[$primary])) {
        $unique['primary'] = array($primary);
      }
      $exists = FALSE;
      if (!empty($values['#fk'])) {
        foreach ($values['#fk'] as $fk_type => $foreign_record_id) {
          $fk_table = $schema['foreign keys'][$fk_type]['table'];
          $col_name = key($schema['foreign keys'][$fk_type]['columns']);
          if (!empty($fks[$fk_table][$foreign_record_id])) {
            $values[$col_name] = $fks[$fk_table][$foreign_record_id];
          }
          else {
            $values[$col_name] = '';
            if ($debug_mode ?? FALSE) {
              drupal_set_message(
                'Study was NOT yet processed and "stock_id" will be empty.',
                'warning'
              );
            }
          }
        }
        unset($values['#fk']);
      }

      foreach ($unique as $fields) {
        $u_cols = array('*');
        $u_vals = array();
        foreach ($fields as $field) {
          if (!array_key_exists($field, $values)) {
            if (array_key_exists('default', $schema['fields'][$field])) {
              $u_vals[$field] = $schema['fields'][$field]['default'];
            }
          }
          else {
            $u_vals[$field] = $values[$field];
          }
        }

        $start_query = microtime(TRUE);
        $results = chado_select_record($table, $u_cols, $u_vals);
        if ($options['devel']) {
          $times['unique_query'] += (microtime(TRUE) - $start_query);
        }
        if ($results and isset($primary)) {
          $exists = TRUE;
          if (count($results) == 1) {
            if (!empty($values[$primary])) {
              unset($values[$primary]);
            }
            foreach ($values as $field => $val) {
              if ($val == current($results)->{$field}) {
                unset($values[$field]);
              }
            }
            if (!empty($values)) {
              //$fks[$table][$id] = chado_update_record($table, $u_vals, $values, array('return_record' => TRUE))->{$primary};
              break;
            }
          }
          $fks[$table][$id] = current($results)->{$primary};
          break;
        }
      }

      if (!$exists) {
        $inserts[$table]->values($values);
        $has_fields = TRUE;
        $fks_to_populate[] = $id;
      }
    }

    if ($options['devel']) {
      $times['unique'] += (microtime(TRUE) - $start_unique);
      $start_insert = microtime(TRUE);
    }

    //if (0 && $has_fields) {
    //  if ($table == 'featureloc') {
    //    $previous = chado_set_active('chado');
    //    //$last_id = @$inserts[$table]->execute();
    //    chado_set_active($previous);
    //  }
    //  else {
    //    try {
    //      //$last_id = @$inserts[$table]->execute();
    //    }
    //    catch (Exception $ex) {
    //      print_r($values);
    //      throw new Exception($ex);
    //    }
    //  }
    //  $fks[$table] += array_combine($fks_to_populate, range($last_id - count($fks_to_populate) + 1, $last_id));
    //}

    if ($options['devel']) {
      $times['insert'] += (microtime(TRUE) - $start_insert);
    }
  }

  if ($options['devel']) {
    $times['total'] = microtime(TRUE) - $start_time;
    print_r("\nend time: " . date('r') . "\n");
    $times['other'] = $times['total'] - $times['unique'] - $times['insert'];
    $times['unique_other'] = $times['unique'] - $times['unique_query'];
    print_r("timing breakdown:\n");
    print_r($times);
  }

  //if (!empty($options['entities'])) {
  //  tpps_tripal_entity_publish(
  //    $options['entities']['label'],
  //    ($fks[$options['entities']['table']] ?? array()),
  //    array(
  //      'multi' => TRUE,
  //      'prefix' => ($options['entities']['prefix'] ?? NULL),
  //    )
  //  );
  //}

  if (!empty($options['fks'])) {
    return $fks[$options['fks']];
  }
  else {
    return $fks;
  }
}
