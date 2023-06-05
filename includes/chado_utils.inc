<?php

/**
 * @file
 * Defines useful functions that interact with Chado.
 */

/**
 * Inserts a record to a chado table using the Tripal Chado API.
 *
 * This function uses the Tripal Chado API to insert a record. However, if the
 * record already exists, then this function will update that record in the
 * database instead. This function also supports insertion of multiple records
 * at a time, while the standard Tripal chado_insert_record function does not.
 *
 * @param string $table
 *   The name of the Chado table being inserted to.
 * @param mixed $records
 *   The record or array of records to be inserted to chado.
 * @param array $options
 *   An array of additional options which can be passed to chado_insert_record.
 *
 * @return mixed
 *   The id of the new record in the database. False if insertion aborted.
 *   This is only supported for single record insertions
 */
function tpps_chado_insert_record($table, $records, array $options = array()) {
  if (!array_key_exists('multi', $options)) {
    $options['multi'] = FALSE;
  }

  if (!$options['multi']) {
    $records = array($records);
  }
  else {
    $has_fields = FALSE;
    $insert = db_insert("chado.$table");
    $fields_arr = array();
    foreach ($records[0] as $field => $val) {
      $fields_arr[] = $field;
    }
    $insert->fields($fields_arr);
  }

  $table_desc = chado_get_schema($table);
  if (!$table_desc) {
    return FALSE;
  }
  $primary = !empty($table_desc['primary key'][0]) ? $table_desc['primary key'][0] : NULL;
  foreach ($records as $values) {
    // Populate insert_values array and check if a record with the same unique
    // fields already exists. If it does, return the existing primary key.
    $insert_values = array();

    foreach ($values as $field => $value) {
      if (is_array($value)) {
        $results = chado_schema_get_foreign_key($table_desc, $field, $value);
        if (count($results) != 1) {
          return FALSE;
        }
        else {
          $insert_values[$field] = $results[0];
        }
      }
      else {
        $insert_values[$field] = $value;
      }
    }

    $unique = array();
    if (array_key_exists('unique keys', $table_desc)) {
      $unique = $table_desc['unique keys'];
    }
    $unique['primary'] = array($primary);

    $exists = FALSE;
    foreach ($unique as $fields) {
      $u_cols = array($primary);
      $u_vals = array();
      foreach ($fields as $field) {
        $u_cols[] = $field;
        if (!array_key_exists($field, $insert_values)) {
          if (array_key_exists('default', $table_desc['fields'][$field])) {
            $u_vals[$field] = $table_desc['fields'][$field]['default'];
          }
        }
        else {
          $u_vals[$field] = $insert_values[$field];
        }
      }

      if (!empty($u_vals) and ($results = chado_select_record($table, $u_cols, $u_vals))) {
        $exists = TRUE;
        // If the new values match with exactly one record, update that one
        // and return the primary key.
        if (!$options['multi'] and count($results) == 1) {
          chado_update_record($table, $u_vals, $insert_values);
          return $results[0]->{$primary};
        }
      }
    }

    if (!$exists and $options['multi']) {
      $insert->values($insert_values);
      $has_fields = TRUE;
    }

    if (!$options['multi']) {
      if ($exists) {
        return $results[0]->{$primary};
      }
      $new_record = chado_insert_record($table, $insert_values, $options);
      if (!empty($new_record[$primary])) {
        return $new_record[$primary];
      }
      else {
        return;
      }
    }
  }
  if ($options['multi'] and $has_fields) {
    $insert->execute();
  }
}

/**
 * This function attempts to perform submits using multi_insert
 * and also copy for genotype_call data. It is an attempt to understand 
 * the differences in speed using COPY vs INSERTS where applicable.
 */
function tpps_chado_insert_hybrid(array $record_groups, array $options = array()) {
  // NEED TO BE A SUPERUSER;
  // ALTER USER drupal WITH SUPERUSER;

  foreach ($record_groups as $table => $records) {
    $records_count = count($records);
    if ($records_count > 0) {

      if ($table != 'genotype_call') {
        // This will allow updates to 
        tpps_chado_insert_multi([$table => &$record_groups[$table]], $options);
      }
      else {
        // Skip if genotype_call - don't worry, we do this at the end of the function
      }
    }
  }

  // DEBUG CODE - Leave this debug code to determine if record_groups per table is getting populated correctly
  // foreach ($record_groups as $table => $records) {
  //   echo "Table $table record_count found in hybrid process: " . count($record_groups[$table]) . "\n";
  // }
  // throw new Exception('DEBUG');

  // This flag will basically store a csv file and use the command line copy
  // This doesn't work for us since we use transactions
  // and an external process doesn't get to see the transaction inserts etc.
  $use_external_copy = false;
  
  $table = 'genotype_call';
  $records = $record_groups['genotype_call'];

  // These variables are set within the Drupal admin panel /admin/config/content/tpps
  $tpps_db_csv_directory = variable_get('tpps_db_csv_directory', '/tmp/tpps/');
  $tpps_db_directory_user = variable_get('tpps_db_directory_user', 'postgres');

  // Attempt to make directory if it does not exist
  mkdir($tpps_db_csv_directory, 0755, true);
  // Try to set the tpps_db_csv_directory to have the db_directory_user permission
  chown($tpps_db_csv_directory, $tpps_db_directory_user);

  $file_location = $tpps_db_csv_directory . time() . '-copy-table.csv';
  $csv_file = fopen($file_location, 'w+');
  
  print_r('TABLE:' . $table . "\n");

  // print_r("Records:\n");
  // print_r($records);
  // print_r("\n");
  $line_index = -1; // SET LINE INDEX
  $copy_cmd = '';
  $csv_data_array = []; // RESET THE CSV DATA ARRAY 
  $found_field_names = false;
  $copy_cmd = 'copy chado.' . $table . ' (';
  // Assume this is CHADO since the name of the function also has chado in it
  foreach ($records as $unique_field_name_value  => $row) {
    // print_r("row:\n");
    // print_r($row);
    // print_r("\n");
    // KEEP TRACK OF LINE INDEX PER TABLE FILE
    $line_index = $line_index + 1;
    $row_data = '';
    $unique_field_name = '';
    // Depending on the table, we set the unique_field_name (eg uniquename column)
    if ($found_field_names == FALSE) {
      $unique_field_name = '';
      switch ($table) {
        case 'feature':
          $unique_field_name = 'uniquename';
          break;
        case 'genotype':
          $unique_field_name = 'uniquename';
          break;
        case 'genotype_call': 
          break;
        case 'genotypeprop':
          break;
        case 'stock_genotype':
          break;
      }
      if ($unique_field_name != '') {
        $copy_cmd .= $unique_field_name . ',';
      }
    }

    // IF UNIQUE FIELD NAME WAS FOUND, INCLUDE THE VALUE FOR THIS UNIQUE FIELD NAME (SOME TABLES DON'T NEED THIS)
    // ADD THIS TO THE ROW DATA LINE
    if ($unique_field_name != '') {
      // $row_data .= $unique_field_name_value . ',';
      $row[$unique_field_name] = $unique_field_name_value;
    }

    foreach ($row as $field_name => $value) {
      // GENERATE THE COPY COMMAND IF field_names found is FALSE
      if ($found_field_names == FALSE) {
        $copy_cmd .= $field_name . ',';
      }

      // GENERATE THE ROW DATA LINE USING THIS VALUE
      // if (!$use_external_copy) {
      //   $row_data .= $value . ',';
      // }
    }
    
    // FINISH UP GENERATING THE COPY COMMAND IF field_names found is FALSE
    if ($found_field_names == FALSE) {
      $copy_cmd = rtrim($copy_cmd, ',');
      $copy_cmd .= ')';
      // $copy_cmd .= ' FROM STDIN WITH CSV';
      $found_field_names = TRUE;
    }

    fputcsv($csv_file, $row);

    if ($use_external_copy) {
      
    }
    else {
      // GENERAL ERROR 4
      // If this is the first line, we need to run the actual COPY statement first
      // if($line_index == 0) {
      //   chado_query($copy_cmd . " FROM STDIN (Delimiter ',')");
      //   echo "COPYCMD: $copy_cmd\n";
      // }
      // // Then always do the line with the data.
      // $row_data = rtrim($row_data, ',');
      // $row_data = rtrim($row_data);
      // $row_data .= "\n";
      // echo "ROW_DATA: $row_data\n";
      // chado_query($row_data);
    }

    // CLEAN UP ROW_DATA ENDING COMMA
    // $row_data = rtrim($row_data, ',');
    

    // ADD THIS ROW TO THE CSV_DATA_ARRAY
    // $csv_data_array[] = $row_data . "\n";

    // SINCE RECORD ADDED (DEBUG)
    // print_r('DEBUG1:' . $copy_cmd . "\n");
    // print_r('ROW DATA:' . $row_data . "\n");
    // throw new Exception('DEBUG');

  }

  fclose($csv_file);
  do {
    if (file_exists($file_location)) {
        echo "The CSV file was found: " . date("d-m-Y h:i:s") . ". Continuing the import process...\n";
        break;
    }
    else {
      echo "Waiting for CSV file to get written to the filesystem\n";
    }
    sleep(100);
  } while(true);

  // We could do this earlier but it would be harder to understand the logic
  // We need to count the records (genotype_call records) to ensure there
  // is at least one record since the field names are generated from the first record
  // and the copy command would only be run if it at least one record exists
  $records_count = count($records);
  if ($records_count > 0) {
    if ($use_external_copy) {
      // throw new Exception('Test CSV write');

      // WRITE A TEMP FILE
      // print_r($csv_data_array);
      // file_put_contents($file_location, $csv_data_array);

      $connection_string = `drush sql-connect`;
      $connection_string = trim($connection_string);
      // REMOVE THE PGPASSFILE STRING SINCE THERE SEEMS TO BE A BUG WITH THIS
      $connection_string_parts = explode('psql ', $connection_string);
      $connection_string = 'psql ' . $connection_string_parts[1];

      // GET THE DB PASSWORD
      $db_password_output = `drush status --show-passwords --fields=db-password`;
      $db_password_output = explode("\n", $db_password_output);
      $db_password_output = $db_password_output[0]; // just get line
      $db_password_output = explode(":",$db_password_output);
      $pg_pass = trim($db_password_output[1]);


      // NOW WE NEED TO SET THE PSQL PASSWORD USING export PGPASSWORD='password';
      $connection_string = "export PGPASSWORD=$pg_pass;\n" . $connection_string;
      echo "Connection_string: $connection_string \n";

      $command = $connection_string . ' -c "\\' . $copy_cmd . ' FROM STDIN WITH CSV " < ' . $file_location;
      $result = `$command`;
    }
    else {
      // This ends the copy insert data lines
      // chado_query("\\.\n");

      echo "COPY $file_location DATA:\n";
      echo file_get_contents($file_location);
      echo "\n";

      $sql = $copy_cmd . " FROM  '" . $file_location . "' WITH DELIMITER AS ','";
      echo $sql . "\n";
      chado_query($sql);
    }
    

    // CONVERT CSV DATA ARRAY TO A STRING
    // $csv_data_array[] = "\\.\n";
    // $csv_data_array = implode("\n", $csv_data_array); // turns it into a string
    // chado_query($copy_cmd . " FROM STDIN DELIMITER ',';" . $csv_data_array, []);
    echo "COPY completed for chado.$table with $records_count records\n";
    // echo "CSV File location: $file_location\n";
    unlink($file_location); 
  } 

  // ALL RECORDS FOR THIS TABLE ITERATED, SAVE THE CSV_DATA_ARRAY
  // print_r('DEBUG2:' .  $copy_cmd . "\n");
  // throw new Exception('DEBUG');
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
function tpps_chado_insert_multi(array $record_groups, array $options = array()) {

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
          $values[$col_name] = $fks[$fk_table][$foreign_record_id];
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
              $fks[$table][$id] = chado_update_record($table, $u_vals, $values, array('return_record' => TRUE))->{$primary};
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

    if ($has_fields) {
      if ($table == 'featureloc') {
        $previous = chado_set_active('chado');
        $last_id = @$inserts[$table]->execute();
        chado_set_active($previous);
      }
      else {
        try {
          $last_id = @$inserts[$table]->execute();
        }
        catch (Exception $ex) {
          print_r($values);
          throw new Exception($ex);
        }
      }
      $fks[$table] += array_combine($fks_to_populate, range($last_id - count($fks_to_populate) + 1, $last_id));
    }

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

  if (!empty($options['entities'])) {
    tpps_tripal_entity_publish($options['entities']['label'], ($fks[$options['entities']['table']] ?? array()), array(
      'multi' => TRUE,
      'prefix' => ($options['entities']['prefix'] ?? NULL),
    ));
  }

  if (!empty($options['fks'])) {
    return $fks[$options['fks']];
  }
  else {
    return $fks;
  }
}

/**
 * Publish a Tripal Entity based on the bundle name, title, and record id.
 *
 * @param string $bundle_name
 *   The name of the bundle, for example, "Organism", "Gene", "Project", etc.
 * @param array $vals
 *   The an array of entity values to be published.
 * @param array $options
 *   Additional options for publishing.
 */
function tpps_tripal_entity_publish($bundle_name, array $vals, array $options = array()) {
  if (!empty($options['multi'])) {
    $bundle = tripal_load_bundle_entity(array('label' => $bundle_name));
    if (!isset($bundle)) {
      return;
    }

    $time = time();
    $entity_insert = db_insert('tripal_entity')
      ->fields(array('type', 'bundle', 'term_id', 'title', 'created', 'changed'));
    $entity_count = 0;
    foreach ($vals as $title => $id) {
      if (empty(chado_get_record_entity_by_bundle($bundle, $id))) {
        if (!empty($options['prefix'])) {
          $title = $options['prefix'] . $title;
        }
        $entity_insert->values(array(
          'type' => 'TripalEntity',
          'bundle' => 'bio_data_' . $bundle->id,
          'term_id' => $bundle->term_id,
          'title' => $title,
          'created' => $time,
          'changed' => $time,
        ));
        $entity_count++;
      }
    }
    $entity_id = $entity_insert->execute() - $entity_count + 1;

    $bio_data_insert = db_insert('chado_bio_data_' . $bundle->id)
      ->fields(array('entity_id', 'record_id'));
    foreach ($vals as $id) {
      if (empty(chado_get_record_entity_by_bundle($bundle, $id))) {
        $bio_data_insert->values(array(
          'entity_id' => $entity_id,
          'record_id' => $id,
        ));

        $entity = new stdClass();
        $entity->id = $entity_id;
        $entity->bundle = 'bio_data_' . $bundle->id;
        $alias = str_replace(' ', '', $bundle->term->name) . '/' . $entity_id;
        $controller = entity_get_controller('TripalEntity');
        $controller->setAlias($entity, $alias);
        $controller->resetCache();

        $entity_id++;
      }
    }
    $bio_data_insert->execute();
  }
  else {
    $bundle = tripal_load_bundle_entity(array('label' => $bundle_name));

    if (!isset($bundle)) {
      return;
    }

    if (empty(chado_get_record_entity_by_bundle($bundle, $vals[1]))) {
      $entity_id = db_insert('tripal_entity')
        ->fields(array(
          'type' => 'TripalEntity',
          'bundle' => 'bio_data_' . $bundle->id,
          'term_id' => $bundle->term_id,
          'title' => $vals[0],
          'created' => time(),
          'changed' => time(),
        ))
        ->execute();

      db_insert('chado_bio_data_' . $bundle->id)
        ->fields(array(
          'entity_id' => $entity_id,
          'record_id' => $vals[1],
        ))
        ->execute();

      $entity = tripal_load_entity('TripalEntity', array($entity_id))[$entity_id];
      $controller = entity_get_controller('TripalEntity');
      $controller->setAlias($entity);
      $controller->resetCache();
    }
  }
}

/**
 * Checks to see if a property of an item already exists in chado.
 *
 * @param string $base_table
 *   The name of the base chado table.
 * @param int $id
 *   The record if of the base item.
 * @param string $name
 *   The name of the type_id of the property we are looking for.
 * @param array $options
 *   Additional options.
 *
 * @return bool
 *   TRUE if a property exists, otherwise FALSE.
 */
function tpps_chado_prop_exists($base_table, $id, $name, array $options = array()) {
  $prop_table = $base_table . 'prop';
  $cvterm = array('name' => $name);

  if (array_key_exists('prop_table', $options)) {
    $prop_table = $options['prop_table'];
  }

  if (array_key_exists('cv', $options)) {
    $cvterm['cv_id'] = array(
      'name' => $options['cv'],
    );
  }

  $prop_query = chado_select_record($prop_table, array($prop_table . '_id'), array(
    'type_id' => $cvterm,
    $base_table . '_id' => $id,
  ));
  return !empty($prop_query);
}

/**
 * Returns an array of field names associated with a db table.
 *
 * This function first checks the chado schema of the table. If there is no
 * existing chado schema, then it will check the drupal schema. If there is no
 * existing drupal schema, it will return an empty array.
 *
 * @param string $table
 *   The name of the table.
 *
 * @return array
 *   An array of field names.
 */
function tpps_table_columns($table) {
  $schema = chado_get_schema($table);
  if (empty($schema)) {
    $schema = drupal_get_schema($table);
    if (empty($schema)) {
      return array();
    }
  }
  $cols = array();
  foreach ($schema['fields'] as $name => $info) {
    $cols[] = $name;
  }
  return $cols;
}

/**
 * Tries to retrieve taxonomy information about an organism from the db.
 *
 * If taxonomy information for the genus does not already exist, then the
 * function tries to retrieve taxonomy information from NCBI.
 *
 * @param string $org_name
 *   The full name (genus, species) of the organism.
 * @param string $rank
 *   The taxonomic rank (subkingdom, order, family, etc).
 *
 * @return string
 *   The the taxonomy information of the organism.
 */
function tpps_get_taxon($org_name, $rank) {
  $parts = explode(' ', $org_name);
  $genus = $parts[0];

  $query = db_select('chado.organismprop', 'op');
  $query->join('chado.organism', 'o', 'o.organism_id = op.organism_id');
  $query->fields('op', array('value'));
  $query->condition('op.type_id', tpps_load_cvterm($rank)->cvterm_id);
  $query->condition('o.genus', $genus);
  $query->range(0, 1);
  $query = $query->execute();
  $result = $query->fetchObject()->value ?? NULL;
  if (!empty($result)) {
    return $result;
  }
  return tpps_ncbi_get_taxonomy($org_name, $rank);
}

/**
 * Add a file path as a projectprop to a submission.
 *
 * @param array $state
 *   The TPPS submission associated with the file.
 * @param int $fid
 *   The managed file id number.
 */
function tpps_add_project_file(array &$state, &$fid) {
  $file = file_load($fid) ?? NULL;
  if ($file) {
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('file_path')->cvterm_id,
      'value' => file_create_url($file->uri),
      'rank' => $state['file_rank'],
    ));
    db_insert('tpps_project_file_managed')
    ->fields(array(
      'project_id' => $state['ids']['project_id'],
      'fid' => $fid,
    ))
    ->execute();
  $state['file_rank']++;
  }

  if (!empty($state['revised_files'][$fid]) and ($file = file_load($state['revised_files'][$fid]))) {
    $fid = $state['revised_files'][$fid];
    tpps_chado_insert_record('projectprop', array(
      'project_id' => $state['ids']['project_id'],
      'type_id' => tpps_load_cvterm('file_path')->cvterm_id,
      'value' => file_create_url($file->uri),
      'rank' => $state['file_rank'],
    ));
    db_insert('tpps_project_file_managed')
    ->fields(array(
      'project_id' => $state['ids']['project_id'],
      'fid' => $fid,
    ))
    ->execute();
  $state['file_rank']++;
  }
}
