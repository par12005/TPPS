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
    foreach ($unique as $name => $fields) {
      $u_cols = array($primary);
      $u_vals = array();
      foreach ($fields as $index => $field) {
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
      $last_id = $inserts[$table]->execute();
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
    tpps_tripal_entity_publish($options['entities']['label'], $fks[$options['entities']['table']], array(
      'multi' => TRUE,
      'prefix' => $options['entities']['prefix'] ?? NULL,
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
 * Tries to retrieve family information about an organism from the db.
 *
 * If family information for the genus does not already exist, then the
 * function tries to retrieve family information from NCBI.
 *
 * @param string $org_name
 *   The full name (genus, species) of the organism.
 *
 * @return string
 *   The family of the organism.
 */
function tpps_get_family($org_name) {
  $parts = explode(' ', $org_name);
  $genus = $parts[0];

  $fam_query = db_select('chado.organismprop', 'op');
  $fam_query->join('chado.organism', 'o', 'o.organism_id = op.organism_id');
  $fam_query->fields('op', array('value'));
  $fam_query->condition('op.type_id', chado_get_cvterm(array(
    'name' => 'family',
  ))->cvterm_id);
  $fam_query->condition('o.genus', $genus);
  $fam_query->range(0, 1);
  $fam_query = $fam_query->execute();
  $family = $fam_query->fetchObject()->value ?? NULL;
  if (!empty($family)) {
    return $family;
  }
  return tpps_ncbi_get_family($org_name);
}

/**
 * Tries to retrieve subkingdom information about an organism from the db.
 *
 * If subkingdom information for the genus does not already exist, then the
 * function tries to retrieve subkingdom information from NCBI.
 *
 * @param string $org_name
 *   The full name (genus, species) of the organism.
 *
 * @return string
 *   The subkingdom of the organism.
 */
function tpps_get_subkingdom($org_name) {
  $parts = explode(' ', $org_name);
  $genus = $parts[0];

  $sub_query = db_select('chado.organismprop', 'op');
  $sub_query->join('chado.organism', 'o', 'o.organism_id = op.organism_id');
  $sub_query->fields('op', array('value'));
  $sub_query->condition('op.type_id', chado_get_cvterm(array(
    'name' => 'subkingdom',
  ))->cvterm_id);
  $sub_query->condition('o.genus', $genus);
  $sub_query->range(0, 1);
  $sub_query = $sub_query->execute();
  $subkingom = $sub_query->fetchObject()->value ?? NULL;
  if (!empty($subkingom)) {
    return $subkingom;
  }
  return tpps_ncbi_get_subkingom($org_name);
}
