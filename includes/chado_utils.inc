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
    $u_cols = array($primary);
    $u_vals = array();

    $exists = FALSE;
    foreach ($unique as $name => $fields) {
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

      if (($results = chado_select_record($table, $u_cols, $u_vals))) {
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

      $u_cols = array('*');
      $u_vals = array();
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

  if (!empty($options['fks'])) {
    return $fks[$options['fks']];
  }
  else {
    return $fks;
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