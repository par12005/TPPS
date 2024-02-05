<?php

/**
 * @file
 * Defines useful functions for file management.
 */


/**
 * This function parses a file and returns an array of its content.
 *
 * The process of actually parsing the file is done with the use of helper
 * functions, and this function returns the results.
 *
 * @param int $fid
 *   The Drupal managed file identifier of the file to be parsed.
 * @param int $max_rows
 *   The maximum number of rows to read from the file.
 * @param bool $no_header
 *   Whether or not the file has a "no_header" flag set.
 *
 * @return array
 *   An array containing the content headers and content body.
 */
function tpps_parse_file($fid, $max_rows = 0, $no_header = FALSE, $columns = NULL) {
  $content = array();
  $options = array(
    'no_header' => $no_header,
    'columns' => $columns,
    'max_rows' => $max_rows,
    'content' => &$content,
  );

  tpps_file_iterator($fid, 'tpps_parse_file_helper', $options);
  $content['headers'] = tpps_file_headers($fid, $no_header);
  return $content;
}

/**
 * This function processes a single row of a file.
 *
 * This function populates the content attribute of the options array with
 * content returned by the TPPS file generator. It is used by both the
 * tpps_parse_file and the tpps_file_headers functions. This function is meant
 * to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_parse_file_helper($row, array &$options) {
  $options['content'][] = $row;
}

/**
 * Returns the number of lines or rows in a file.
 *
 * @param int $fid
 *   The Drupal managed file identifier of the file.
 *
 * @return int
 *   The number of lines or rows in the file.
 */
function tpps_file_len($fid) {
  $file = file_load($fid);
  $location = tpps_get_location($file->uri);
  $extension = tpps_get_path_extension($location);
  $count = 0;
  $options = array(
    'count' => &$count,
  );
  if ($extension == 'vcf') {
    $options['skip_prefix'] = '#';
  }
  tpps_file_iterator($fid, 'tpps_file_len_helper', $options);
  return $count;
}

/**
 * This is the helper function for tpps_file_len().
 *
 * This function is passed to the tpps_file_iterator during tpps_file_len, and
 * increments the count attribute of the options array. It also checks whether
 * a skip_prefix was provided, and skips lines if necessary.
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_file_len_helper($row, array &$options = array()) {
  if (empty($options['skip_prefix'])) {
    $options['count']++;
  }
  else {
    if (substr(current($row), 0, strlen($options['skip_prefix'])) != $options['skip_prefix']) {
      $options['count']++;
    }
  }
}

/**
 * This function returns the width of a file.
 *
 * The process of finding the width of the file is done with the use of helper
 * functions, and this function returns the results.
 *
 * @param int $fid
 *   The Drupal managed file identifier of the file to be parsed.
 *
 * @return int
 *   The length of the longest line in the file.
 */
function tpps_file_width($fid) {
  $file = file_load($fid);
  $location = tpps_get_location($file->uri);
  $extension = tpps_get_path_extension($location);
  switch ($extension) {
    case 'xlsx':
      return tpps_xlsx_width($location);

    case 'txt':
    case 'csv':
      return tpps_flat_width($location);

    case 'vcf':
    case 'gz':
      return tpps_flat_width($location, "\t");

    default:
      throw new Exception('Unrecognized file type');
  }
}

/**
 * This function gets the length of the longest line of an xlsx file.
 *
 * @param string $location
 *   The location of the file.
 *
 * @return int
 *   The length of the longest line.
 */
function tpps_xlsx_width($location) {
  $dir = drupal_realpath(TPPS_TEMP_XLSX);

  $zip = new ZipArchive();
  $zip->open($location);
  $zip->extractTo($dir);

  $data_location = $dir . '/xl/worksheets/sheet1.xml';

  $dimension = tpps_xlsx_get_dimension($data_location);
  preg_match('/([A-Z]+)[0-9]+:([A-Z]+)[0-9]+/', $dimension, $matches);
  $left_hex = unpack('H*', $matches[1]);
  $hex = $left_hex[1];
  $right_hex = unpack('H*', $matches[2]);

  $len = 0;
  while (base_convert($hex, 16, 10) <= base_convert($right_hex[1], 16, 10)) {
    $len++;
    $hex = tpps_increment_hex($hex);
  }
  return $len;
}

/**
 * This function gets the length of the longest line of a flat file.
 *
 * @param string $location
 *   The location of the file.
 * @param string $delim
 *   The delimeter for each line in the file. Defaults to ','.
 *
 * @return int
 *   The length of the longest line.
 */
function tpps_flat_width($location, $delim = ',') {
  $original = ini_get('auto_detect_line_endings');
  if (!$original) {
    ini_set('auto_detect_line_endings', TRUE);
  }

  $longest = 0;
  $handle = gzopen($location, 'r');

  while (($line = gzgets($handle))) {
    $line = str_getcsv($line, $delim);
    $longest = max($longest, count($line));
  }

  fclose($handle);
  ini_set('auto_detect_line_endings', $original);
  return $longest;
}

/**
 * This function increments a hexidecimal key.
 *
 * The function expects a hexidecimal encoding of a capital alphabetical key.
 * For example, you might pass "$hex = unpack('H*', 'A')[1];". This function is
 * intended to be used for parsed file keys. Here are some examples:
 *  echo pack('H*', tpps_increment_hex(unpack('H*', 'A')[1]));    // B
 *  echo pack('H*', tpps_increment_hex(unpack('H*', 'AA')[1]));   // AB
 *  echo pack('H*', tpps_increment_hex(unpack('H*', 'Z')[1]));    // AA
 *  echo pack('H*', tpps_increment_hex(unpack('H*', 'AZ')[1]));   // BA
 *  echo pack('H*', tpps_increment_hex(unpack('H*', 'ZZZ')[1]));  // AAAA.
 *
 * @param string $hex
 *   The hexidecimal encoding of a capital alphabetical key.
 *
 * @return string
 *   The encoding of the next capital alphabetical key.
 */
function tpps_increment_hex($hex) {
  $add = 1;
  $new = "";
  for ($i = strlen(pack('H*', $hex)) - 1; $i >= 0; $i--) {
    $num = base_convert(substr($hex, (2 * $i), 2), 16, 10) + $add;
    if ($num > 90) {
      $num = 65;
    }
    else {
      $add = 0;
    }
    $new = base_convert($num, 10, 16) . $new;
    if ($i == 0 && $add) {
      $new = '41' . $new;
    }
  }
  return $new;
}

/**
 * This function will convert a hexadecimal column id to an integer.
 *
 * Here are a few examples of column keys and their converted integers:
 *  echo tpps_convert_colname('A');     // 0
 *  echo tpps_convert_colname('B');     // 1
 *  echo tpps_convert_colname('AA');    // 26
 *  echo tpps_convert_colname('AB');    // 27
 *  echo tpps_convert_colname('BA');    // 52
 *
 * @param string $column
 *   The capital alphabetical key for the column.
 *
 * @return int
 *   The integer conversion of the column key.
 */
function tpps_convert_colname($column) {
  $total = 0;
  $base_16 = unpack('H*', 'A')[1];
  $base = base_convert($base_16, 16, 10) - 1;
  for ($i = 0; $i < strlen($column); $i++) {
    $total *= 26;
    $num_16 = unpack('H*', $column[$i])[1];
    $total += base_convert($num_16, 16, 10) - $base;
  }
  return $total - 1;
}

/**
 * Renames a Drupal managed file.
 *
 * If the "preserve" option is set to true, then the new file will be created
 * as a copy of the old one. Otherwise, the old file will be removed.
 *
 * @param int $fid
 *   The Drupal managed file identifier of the file to be renamed.
 * @param string $new_name
 *   The new name of the file.
 * @param array $options
 *   Additional options to be used when renaming the file.
 *
 * @return object
 *   The new Drupal managed file object.
 */
function tpps_rename_file($fid, $new_name, array $options = array()) {
  if (!array_key_exists('preserve', $options)) {
    $options['preserve'] = FALSE;
  }

  if (!empty($fid) and ($file = file_load($fid))) {
    $new_filename = $new_name . "." . tpps_get_path_extension($file->uri);
    if (!preg_match('/^(.*\/)(.*)$/', $new_name, $matches)) {
      preg_match('/^(.*\/).*$/', $file->uri, $matches);
      $new_filename = $matches[1] . $new_filename;
    }

    if ($options['preserve']) {
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      // Check if file already exists then return that file.
      $result = db_query('SELECT f.fid
              FROM {file_managed} f WHERE f.uri = :uri', array(':uri' => $new_filename));
      $record = $result->fetchObject();
      if(isset($record)) {
        return file_load($record->fid);
      }
      return file_copy($file, $new_filename, FILE_EXISTS_RENAME);
    }
    // Check if file already exists then return that file.
    $result = db_query('SELECT f.fid
    FROM {file_managed} f WHERE f.uri = :uri', array(':uri' => $new_filename));
    $record = $result->fetchObject();

    if(isset($record->fid)) {
      // print_r("FID:" . $record->fid . "\n");
      return file_load($record->fid);
    }
    $file_object = file_move($file, $new_filename, FILE_EXISTS_RENAME);
    // print_r("FILE OBJECT:\n");
    // print_r($file_object);
    // print_r("\n");
    return $file_object;
  }
  throw new Exception("Error: could not open file");
}

/**
 * Parses a single column from a file.
 *
 * @param int $fid
 *   The Drupal managed file identifier of the file to be parsed.
 * @param string $column
 *   The alphabetical key of the column to be parsed.
 * @param bool $no_header
 *   Whether or not the no_header options has been set to true for the file.
 *
 * @return array
 *   An array of values found in the parsed column.
 */
function tpps_parse_file_column($fid, $column, $no_header = FALSE) {
  $content = [];
  $options = [
    'no_header' => $no_header,
    'columns' => [$column],
    'content' => &$content,
  ];
  tpps_file_iterator($fid, 'tpps_parse_file_column_helper', $options);
  return $content;
}

/**
 * This function processes a single row of a file.
 *
 * This function populates the content attribute of the options array with the
 * data in a single column. This function is meant to be used with
 * tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_parse_file_column_helper($row, array &$options) {
  $options['content'][] = $row[current($options['columns'])];
}

/**
 * Compares two file columns.
 *
 * Returns the array of items that are found in file 1, column file_1_id_name,
 * and are not found in file 2, column file_2_id_name.
 *
 * @param int $fid_1
 *   File 1 identifier.
 * @param int $fid_2
 *   File 2 identifier.
 * @param mixed $file_1_id_name
 *   Name of column to check in file 1. String or integer.
 * @param mixed $file_2_id_name
 *   Name of column to check in file 2. String or integer.
 *
 * @return array
 *   The array of items found in file 1 that were not found in file 2.
 */
function tpps_compare_files($fid_1, $fid_2, $file_1_id_name, $file_2_id_name, $file_1_no_header = FALSE, $file_2_no_header = FALSE) {
  $missing = array();
  $file_1_content = array_unique(tpps_parse_file_column($fid_1, $file_1_id_name, $file_1_no_header));
  $file_2_content = array_unique(tpps_parse_file_column($fid_2, $file_2_id_name, $file_2_no_header));
  asort($file_1_content);
  asort($file_2_content);
  reset($file_1_content);
  reset($file_2_content);

  while (current($file_1_content) !== FALSE and current($file_2_content) !== FALSE) {
    if (current($file_1_content) < current($file_2_content)) {
      $missing[] = current($file_1_content);
      next($file_1_content);
      continue;
    }
    elseif (current($file_1_content) > current($file_2_content)) {
      next($file_2_content);
      continue;
    }
    next($file_1_content);
    next($file_2_content);
    continue;
  }

  while (current($file_1_content) !== FALSE) {
    $missing[] = current($file_1_content);
    next($file_1_content);
  }
  return $missing;
}

/**
 * This function validates that the file contains all of the required groups.
 *
 * A "required group" is a column type that must be present within the file for
 * the file to be considered valid. For instance, a required group for a plant
 * accession file would be the "Location" group. The required groups array
 * should satisfy the structure:
 *  array(
 *    '<group name>' => array(
 *      '<type>' => array(<column option index>[, <column option index>...])[,
 *      '<type>' => array(<column option index>[, <column option index>...])...]
 *    )[,
 *    '<group name>' => array(
 *      '<type>' => array(<column option index>[, <column option index>...])[,
 *      '<type>' => array(<column option index>[, <column option index>...])...]
 *    )...]
 *  )
 * where <group name> is the name of the required group, <type> is the name of
 * the set of options that could satisfy the required group, and each of the
 * <column option index>s is a key within the file element['column-options']
 * field.
 *
 * @param array $form_state
 *   The state of the form that the file is part of.
 * @param array $required_groups
 *   The required column option groups for the file.
 * @param array $file_element
 *   The managed_file element of the file within the form.
 *
 * @return array
 *   The groups and the column option types that the file was valid for.
 */
function tpps_file_validate_columns(array &$form_state, array $required_groups, array $file_element) {

  $title_parts = explode(':', $file_element['#title']);
  $error_prompt = $title_parts[0];

  // Check if file has zero length (empty).
  if (!($file_element['#file']->filesize ?? NULL)) {
    // We highlight an empty value field because file field not visible
    // when file was uploaded.
    form_set_error($file_element['#name'] . '[empty]',
      $error_prompt . ': File is empty.'
    );
    // No need to validate empty file.
    return [];
  }

  $cols = $file_element['#value']['columns'] ?? NULL;
  $parents = $file_element['#parents'];
  $new_end_columns = end($parents) . "-columns";
  $new_end_no_header = end($parents) . "-no-header";
  $new_end_empty = end($parents) . "-empty";
  $group_path = array_pop($parents) . "-groups";
  $values = drupal_array_get_nested_value($form_state['values'], $parents);
  // Initialize form column values in form state.
  $values[$new_end_columns] = [];
  // Hold onto the location of the columns in form state.
  $state_column_values = &$values[$new_end_columns];
  $values[$new_end_no_header] = $file_element['#value']['no-header'] ?? NULL;
  $values[$new_end_empty] = $file_element['#value']['empty'] ?? NULL;

  $groups = [];
  $required_groups_flat = [];
  foreach ($required_groups as $group => $combinations) {
    $groups[$group] = array();
    $required_groups_flat[$group] = [];
    foreach ($combinations as $name => $combination) {
      $required_groups_flat[$group] = array_merge($required_groups_flat[$group], $combination);
    }
  }

  if ($cols) {
    // Note: $cols will be empty when user just added file (not clicked 'Upload'
    // button and submitted form (clicked 'Next' button).
    // Probably columns could be detected automatically because they are shown
    // at page and so we could use default values here.
    // @TODO Get default set of columns.
    foreach ($cols as $name => $type) {
      $state_column_values[$name] = $type;
      foreach ($required_groups_flat as $group => $types) {
        if (in_array($type, $types)) {
          if (!isset($groups[$group][$type])) {
            $groups[$group][$type] = array($name);
          }
          else {
            $groups[$group][$type][] = $name;
          }
          break;
        }
      }
    }
  }

  foreach ($required_groups as $group => $combinations) {
    $group_valid = FALSE;
    $groups[$group]['#type'] = [];
    foreach ($combinations as $name => $combination) {
      $combination_valid = TRUE;
      foreach ($combination as $type) {
        if (!isset($groups[$group][$type])) {
          $combination_valid = FALSE;
          break;
        }
      }
      if ($combination_valid) {
        $groups[$group]['#type'][] = $name;
        $group_valid = TRUE;
      }
    }

    if (!$group_valid) {
      form_set_error(
        $file_element['#name'] . "[columns][$group",
        t('@field_name: Please specify a column or columns that hold @group.',
          [
            '@field_name' => $error_prompt,
            '@group' => $group,
          ]
        )
      );
    }
  }

  foreach ($groups as $key => $group) {
    foreach ($group as $opt_num => $col_names) {
      if (count($col_names) == 1) {
        $groups[$key][$opt_num] = $col_names[0];
      }
    }
  }

  $values[$group_path] = $groups;
  drupal_array_set_nested_value($form_state['values'], $parents, $values);
  return $groups;
}

/**
 * Saves file column selections to the values of the form_state.
 *
 * This function is called before the form is validated so that the column
 * options can be saved whenever the user makes a selection.
 *
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 *
 * @TODO Avoid code duplication.
 */
function tpps_save_file_columns(array &$form, array &$form_state) {
  $accession = substr($form_state['accession'], 4);
  if ($form_state['stage'] == TPPS_PAGE_3) {
    for ($i = 1; $i <= $form_state['stats']['species_count']; $i++) {
      if (
        ($file = file_load($form_state['values']['tree-accession']["species-$i"]['file']))
        && $form_state['values']['tree-accession']["species-$i"]['file'] != 0
      ) {
        file_usage_add($file, 'tpps', 'tpps_project', $accession);
      }

      $form_state['values']['tree-accession']["species-$i"]['file-columns'] = array();

      if (isset($form['tree-accession']["species-$i"]['file']['#value']['columns'])) {
        foreach ($form['tree-accession']["species-$i"]['file']['#value']['columns'] as $col => $val) {
          if ($col[0] != '#') {
            $form_state['values']['tree-accession']["species-$i"]['file-columns'][$col]
              = $form['tree-accession']["species-$i"]['file']['#value']['columns'][$col];
          }
        }
      }

      if (empty($form_state['values']['tree-accession']['check'])) {
        break;
      }
    }
  }
  elseif ($form_state['stage'] == TPPS_PAGE_4) {
    if (isset($form['organism-1']['genotype'])) {
      for ($i = 1; $i <= $form_state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
        $file = file_load($form_state['values']["organism-$i"]['genotype']['files']['snps-assay'] ?? '');
        if ($file) {
          file_usage_add($file, 'tpps', 'tpps_project', $accession);

          $form_state['values']["organism-$i"]['genotype']['files']['snps-assay-columns'] = [];

          if (isset($form["organism-$i"]['genotype']['files']['snps-assay']['#value']['columns'])) {
            foreach ($form["organism-$i"]['genotype']['files']['snps-assay']['#value']['columns'] as $col => $val) {
              if ($col[0] != '#') {
                $form_state['values']["organism-$i"]['genotype']['files']['snps-assay-columns'][$col]
                  = $form["organism-$i"]['genotype']['files']['snps-assay']['#value']['columns'][$col];
              }
            }
          }
        }

        $file = file_load($form_state['values']["organism-$i"]['genotype']['files']['other'] ?? '');
        if ($file) {
          file_usage_add($file, 'tpps', 'tpps_project', $accession);

          $form_state['values']["organism-$i"]['genotype']['files']['other-columns'] = [];

          if (isset($form["organism-$i"]['genotype']['files']['other']['#value']['columns'])) {
            foreach ($form["organism-$i"]['genotype']['files']['other']['#value']['columns'] as $col => $val) {
              if ($col[0] != '#') {
                $form_state['values']["organism-$i"]['genotype']['files']['other-columns'][$col]
                  = $form["organism-$i"]['genotype']['files']['other']['#value']['columns'][$col];
              }
            }
          }
        }
      }
    }

    if (isset($form['organism-1']['phenotype'])) {
      for ($i = 1; $i <= $form_state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
        if (!empty($form_state['values']["organism-$i"]['phenotype']['normal-check'])) {
          if (
            ($file = file_load($form_state['values']["organism-$i"]['phenotype']['file'] ?? ''))
            && $form_state['values']["organism-$i"]['phenotype']['file'] != 0
          ) {
            file_usage_add($file, 'tpps', 'tpps_project', $accession);
          }

          $form_state['values']["organism-$i"]['phenotype']['file-columns'] = [];

          if (isset($form["organism-$i"]['phenotype']['file']['#value']['columns'])) {
            foreach ($form["organism-$i"]['phenotype']['file']['#value']['columns'] as $col => $val) {
              if ($col[0] != '#') {
                $form_state['values']["organism-$i"]['phenotype']['file-columns'][$col]
                  = $form["organism-$i"]['phenotype']['file']['#value']['columns'][$col];
              }
            }
          }

          if ($form_state['values']["organism-$i"]['phenotype']['check'] != '0') {
            if (
              ($file = file_load($form_state['values']["organism-$i"]['phenotype']['metadata'] ?? ''))
              && $form_state['values']["organism-$i"]['phenotype']['metadata'] != 0
            ) {
              file_usage_add($file, 'tpps', 'tpps_project', $accession);
            }

            $form_state['values']["organism-$i"]['phenotype']['metadata-columns'] = [];

            if (isset($form["organism-$i"]['phenotype']['metadata']['#value']['columns'])) {
              foreach ($form["organism-$i"]['phenotype']['metadata']['#value']['columns'] as $col => $val) {
                if ($col[0] != '#') {
                  $form_state['values']["organism-$i"]['phenotype']['metadata-columns'][$col]
                    = $form["organism-$i"]['phenotype']['metadata']['#value']['columns'][$col];
                }
              }
            }
          }
        }
      }
    }
  }
}

/**
 * Gets true file path from a uri or Drupal file stream.
 *
 * If the file is compressed, extracts the archive and returns the path of the
 * extracted file.
 *
 * @param string $location
 *   The uri or file stream of the file we want the location of.
 *
 * @return string
 *   The true path of the file.
 */
function tpps_get_location($location) {
  $location = drupal_realpath($location);
  $extension = tpps_get_path_extension($location);
  if ($extension == 'zip' or $extension == 'tar') {
    $files = tpps_get_archive_files($location);
    if ($files and (count($files) == 3 or (count($files) == 4 and substr($files[3], -9) === '/__MACOSX')) and !is_dir($files[2])) {
      $location = $files[2];
    }
  }
  return $location;
}

/**
 * Gets the file extension from a path.
 *
 * @param string $path
 *   The path of the file we want the extension of.
 *
 * @return string
 *   The extension of that file.
 */
function tpps_get_path_extension($path) {
  preg_match('/\.([a-zA-Z0-9]*)$/', $path, $matches);

  if(count($matches) == 0) {
    return ""; // Added by Rish during debugging process. XLSX extracting creates files without paths
  }
  return strtolower($matches[1]);

}

/**
 * Returns the dimension string of an xlsx file.
 *
 * If the dimension string cannot be found, returns NULL.
 *
 * @param string $location
 *   The location of the file.
 *
 * @return mixed
 *   The dimension string of the file, or NULL if it cannot be found.
 */
function tpps_xlsx_get_dimension($location) {
  $reader = new XMLReader();
  $reader->open($location);
  while ($reader->read()) {
    if ($reader->nodeType == XMLReader::ELEMENT and $reader->name == 'dimension') {
      $dim = $reader->getAttribute('ref');
      $reader->close();
      return $dim;
    }
  }
  return NULL;
}

/**
 * Retrieves rows from array of readers.
 *
 * This function recieves an array of XMLReader objects and an array of cached
 * strings, and returns an array of rows produced by tpps_xlsx_get_row() from
 * each reader.
 *
 * @param array $readers
 *   The array of XMLReader objects.
 * @param array $strings
 *   The cached strings for the current XML file.
 * @param bool $plain
 *   Whether or not to attempt to return an array of cell values, or just TRUE.
 * @param mixed $columns
 *   An array of columns to return. If NULL, return all columns.
 *
 * @return mixed
 *   If $plain is set to TRUE, an array of cell values. Otherwise TRUE.
 */
function tpps_xlsx_get_rows(array &$readers, array &$strings = array(), $plain = TRUE, $columns = NULL) {
  $rows = array();
  $empty = TRUE;
  foreach ($readers as $key => $r) {
    $reader = &$readers[$key];
    $row = tpps_xlsx_get_row($reader, $strings, $plain, $columns);
    if ($row) {
      $rows[$key] = $row;
      $empty = FALSE;
    }
    else {
      $rows[$key] = NULL;
    }
  }
  if ($empty) {
    return FALSE;
  }
  return $rows;
}

/**
 * Retrieves a single "row" from an XML file.
 *
 * This function recieves an XMLReader object and a location for a strings XML
 * file, and returns an array of cell values. If the $plain parameter is set to
 * FALSE, then the function will return TRUE when the XMLReader cursor is on
 * the next row element. If no row element can be found, the function returns
 * FALSE.
 *
 * @param object $reader
 *   The XMLReader object being used to read the XML file.
 * @param array $strings
 *   The cached strings for the current XML file.
 * @param bool $plain
 *   Whether or not to attempt to return an array of cell values, or just TRUE.
 * @param mixed $columns
 *   An array of columns to return. If NULL, return all columns.
 *
 * @return mixed
 *   If $plain is set to TRUE, an array of cell values. Otherwise TRUE.
 */
function tpps_xlsx_get_row(&$reader, array &$strings = array(), $plain = TRUE, $columns = NULL) {
  while ($reader->read()) {
    if ($reader->nodeType == XMLReader::ELEMENT and $reader->name == 'row') {
      if ($plain) {
        $cells = array();
        while (!($reader->nodeType == XMLReader::END_ELEMENT and $reader->name == 'row') and $reader->read()) {
          if ($reader->nodeType == XMLReader::ELEMENT and $reader->name == 'c') {
            $cell_type = $reader->getAttribute('t');
            preg_match('/([A-Z]+)([0-9]+)/', $reader->getAttribute('r'), $matches);
            $cell_location = $matches[1];
            if (!empty($columns) and !isset($columns[$cell_location])) {
              continue;
            }
            $cells[$cell_location] = tpps_xlsx_get_cell_value($reader, $cell_type, $strings);
          }
        }
        return $cells;
      }
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Translates the date from an .xlsx file.
 *
 * This function may seem strange at first, but skips $date = 60 on purpose.
 * Day 60 in Excel's encoding represents 2/29/1900, which is not a real date.
 *
 * @param mixed $date
 *   The date to be translated.
 *
 * @return mixed
 *   The original date provided, a re-formated date, or NULL on failure.
 */
function tpps_xlsx_translate_date($date) {
  if (strtotime($date) !== FALSE) {
    return $date;
  }

  if ($date > 60) {
    $date = $date - 1;
    return date("m/d/Y", strtotime("12/31/1899 +$date days"));
  }
  if ($date < 60) {
    return date("m/d/Y", strtotime("12/31/1899 +$date days"));
  }

  return NULL;
}

/**
 * Returns a cell value from an XML file.
 *
 * This function recieves an XMLReader object, a cell type attribute, and a
 * location for a strings XML file, and returns the value of the current cell.
 * If the cell type is a string, then the function returns the string from the
 * strings file with the matching string id. Otherwise, it returns the inner
 * XML from the 'v' element within the cell. If no 'v' element can be found
 * before the end of the cell, it returns NULL.
 *
 * @param object $reader
 *   The XMLReader object being used to read the XML file.
 * @param string $cell_type
 *   The 't' attribute of the current cell.
 * @param array $strings
 *   The cached strings for the current XML file.
 *
 * @return mixed
 *   Either the value in the 'v' element of the cell or the associated string.
 */
function tpps_xlsx_get_cell_value(&$reader, $cell_type, array &$strings = array()) {
  if ($reader->isEmptyElement) {
    return NULL;
  }

  while (!($reader->nodeType == XMLReader::END_ELEMENT and ($reader->name == 'c' or $reader->name == 'row')) and $reader->read()) {
    if ($reader->nodeType == XMLReader::ELEMENT and $reader->name == 'v') {
      $value = $reader->readInnerXml();
      if (!empty($cell_type) and $cell_type == 's') {
        $value = $strings[$value];
      }
      return $value;
    }
  }
  return NULL;
}

/**
 * Retrieves strings from an excel strings XML file.
 *
 * @param string $strings_location
 *   The location of the strings XML file.
 *
 * @return array
 *   The strings from the strings XML file.
 */
function tpps_xlsx_get_strings($strings_location) {
  $strings = array();
  $reader = new XMLReader();
  $reader->open($strings_location);
  $count = 0;
  while ($reader->read()) {
    if ($reader->nodeType == XMLReader::ELEMENT and $reader->name == 'si') {
      $string = "";

      while (!($reader->nodeType == XMLReader::END_ELEMENT and $reader->name == 'si') and $reader->read()) {
        if ($reader->nodeType == XMLReader::ELEMENT and $reader->name == 't') {
          $string .= $reader->readInnerXml();
        }
      }
      $strings[$count++] = $string;
    }
  }
  $reader->close();
  return $strings;
}

/**
 * Recursively removes a directory.
 *
 * We want to clean out the temporary tpps_xlsx directory when we are done with
 * an excel file, so we need to recursively remove all of the links and sub-
 * directories. Use this function with caution.
 *
 * @param string $dir
 *   The directory to be removed.
 */
function tpps_rmdir($dir) {
  if (is_dir($dir)) {
    $children = scandir($dir);
    foreach ($children as $child) {
      if ($child != '.' and $child != '..') {
        if (is_dir($dir . '/' . $child) and !is_link($dir . '/' . $child)) {
          tpps_rmdir($dir . '/' . $child);
        }
        else {
          unlink($dir . '/' . $child);
        }
      }
    }
    rmdir($dir);
  }
}

/**
 * Extracts an archive from a path or Drupal file object.
 *
 * @param mixed $archive
 *   The archive to be extracted. Can be a path or Drupal file object.
 *
 * @return array
 *   Array of file names. NULL if extension is not a valid compression
 *   extension.
 */
function tpps_get_archive_files($archive) {
  $files = array();
  if (is_object($archive) and !empty($archive->uri)) {
    $archive = $archive->uri;
  }
  $loc = drupal_realpath($archive);

  $ext = tpps_get_path_extension($loc);
  $unzip_dir = drupal_realpath('temporary://tpps_tmp');
  if (is_dir($unzip_dir)) {
    tpps_rmdir($unzip_dir);
  }

  if (is_dir($unzip_dir) or mkdir($unzip_dir)) {
    switch ($ext) {
      case 'zip':
        $zip = new \ZipArchive();
        $zip->open($loc);
        $zip->extractTo($unzip_dir);
        break;

      case 'gz':
        $zip = new \Archive_Tar($loc, 'gz');
        $zip->extract($unzip_dir);
        break;

      case 'tar':
        $zip = new \Archive_Tar($loc);
        $zip->extract($unzip_dir);
        break;

      default:
        return array();
    }

    $dir = $unzip_dir;
    $files = scandir($dir);
    if ($files and count($files) == 3 and is_dir($dir . '/' . $files[2])) {
      $dir .= '/' . $files[2];
      $files = scandir($dir);
    }
  }

  foreach ($files as $key => $name) {
    $files[$key] = $dir . '/' . $name;
  }

  return $files;
}

/**
 * Returns the headers of a TPPS file.
 *
 * @param int $fid
 *   The Drupal managed file id of the file.
 * @param bool $no_header
 *   Whether or not the no_header option has been set for the file.
 *
 * @return array
 *   An array of headers for the file.
 */
function tpps_file_headers($fid, $no_header = FALSE) {
  $headers = array();
  if ($no_header) {
    $hex = unpack('H*', 'A')[1];
    $width = tpps_file_width($fid);
    for ($i = 0; $i < $width; $i++) {
      $key = pack('H*', $hex);
      $headers[$key] = $i;
      $hex = tpps_increment_hex($hex);
    }
    return $headers;
  }

  $content = array();
  $options = array(
    'no_header' => TRUE,
    'max_rows' => 1,
    'content' => &$content,
  );
  tpps_file_iterator($fid, 'tpps_parse_file_helper', $options);
  return current($content);
}

/**
 * Iterates over a file and applies a function to each generator result.
 *
 * This function should be used in place of tpps_parse_file() wherever
 * possible, as it uses much less memory and is much faster.
 *
 * @param int $fid
 *   The Drupal managed file id of the file.
 * @param string $function
 *   The funciton to be applied to each item returned by the generator.
 *   Callback function must be defined like this:
 *   $function(array $row, array $options = []);
 *   See tpps_unit_validate_metafile().
 * @param array $options
 *   Options that will be passed to both $function and the generator.
 *
 * @return bool
 *   Returns TRUE on success, otherwise FALSE.
 */
function tpps_file_iterator($fid, $function, array &$options = array()) {
  if (!function_exists($function)) {
    return FALSE;
  }

  if (!empty($options['job'])) {
    $options['job']->setTotalItems(tpps_file_len($fid));
    $options['job']->setItemsHandled(0);
  }

  $file = file_load($fid);
  $file_location = tpps_get_location($file->uri);

  // Item / line counter
  $item_count = 0;

  foreach (tpps_file_generator($fid, $options) as $item) {
    $item_count = $item_count + 1;
    // echo "ITEM COUNT: " . $item_count . "\n";
    if(isset($options['test'])) {
      // This will reduce the amount of processing in the file iterator
      // This is an attempt to speed up coding and testing by just not
      // processing all files per file (and may not always work)
      $iteration_count = $options['test'];
      if ($item_count > $iteration_count) {
        echo "TEST detected - only processing $iteration_count iterations and exiting the loop.\n";
        return TRUE;
      }
    }

    // Assume the item IS NOT EMPTY
    $item_is_not_empty = true;
    if(is_array($item)) {
      // Check each key value
      $values_count = count($item);
      $values_empty_count = 0;
      foreach($item as $key => $value) {
        if($value == '') {
          $values_empty_count = $values_empty_count + 1;
        }
      }
      // Check if $values_count == $values_empty_count (this means item is empty)
      if ($values_count == $values_empty_count) {
        $item_is_not_empty = false;
      }
    }

    // if the item is not empty, proceed to process the line
    if ($item_is_not_empty) {
      $function($item, $options);
      if (!empty($options['job'])) {
        $options['job']->addItemsHandled(1);
      }
    }
    else {
      // print_r($item);
      // echo json_encode($item) . "\n";
      // echo "[TPPS FILE ITERATOR] Found a line/item that was empty, ignoring this item\n";
      // echo "[TPPS FILE ITERATOR] File location: " . $file_location . "\n";
      // throw new Exception('DEBUG STOP - FOUND AN EMPTY LINE');
    }
  }
  return TRUE;
}

/**
 * This function selects the appropriate generator for a file.
 *
 * The generator is selected based on the file extension. If the extension is
 * not recognized, throws an exception.
 *
 * @param int $fid
 *   The Drupal managed file id for the file.
 * @param array $options
 *   Additional options to be passed to the generator.
 *
 * @return Generator|array
 *   The generator for the file.
 */
function tpps_file_generator($fid, array $options = array()) {
  $file = file_load($fid);
  $location = tpps_get_location($file->uri);
  $extension = tpps_get_path_extension($location);
  switch ($extension) {
    case 'xlsx':
      return tpps_xlsx_generator($location, $options);

    case 'txt':
    case 'csv':
    case 'vcf':
    case 'gz':
      return tpps_flat_generator($location, $options);

    default:
      if (empty($extension)) {
        $extension = "NULL";
      }
      $msg = "Unrecognized file type: $extension";
      throw new Exception($msg);
  }
}

/**
 * This function is a generator for xlsx files.
 *
 * If the 'max_rows' option is set, the generator stop after max_rows items are
 * yielded. If the 'columns' option is set, then each item yielded by the
 * generator will contain only the specified columns from the row.
 *
 * @param string $location
 *   The location of the file on the server.
 * @param array $options
 *   Additional options detailing how to parse the file.
 *
 * @return Generator|array
 *   Yields each row of the file (or columns of a row, if applicable).
 */
function tpps_xlsx_generator($location, array $options = array()) {
  $dir = drupal_realpath(TPPS_TEMP_XLSX);
  $no_header = $options['no_header'] ?? FALSE;
  $columns = $options['columns'] ?? NULL;
  $max_rows = $options['max_rows'] ?? NULL;

  if (!empty($columns)) {
    $new_columns = array();
    foreach ($columns as $col) {
      $new_columns[$col] = $col;
    }
    $columns = $new_columns;
  }

  $zip = new ZipArchive();
  $zip->open($location);
  $zip->extractTo($dir);

  $strings_location = $dir . '/xl/sharedStrings.xml';

  // Find all worksheets.
  $sheets = scandir($dir . '/xl/worksheets');
  unset($sheets[array_search('.', $sheets)]);
  unset($sheets[array_search('..', $sheets)]);

  $strings = tpps_xlsx_get_strings($strings_location);

  // Get dimensions, readers.
  $first_left = NULL;
  $last_right = NULL;
  $dims = array();
  $readers = array();
  foreach ($sheets as $sheet) {
    // dpm('SHEET location: ' . $dir . '/xl/worksheets/' . $sheet);
    if (tpps_get_path_extension($dir . '/xl/worksheets/' . $sheet) !== 'xml') {
      continue;
    }
    $loc = $dir . '/xl/worksheets/' . $sheet;
    $dimension = tpps_xlsx_get_dimension($loc);
    // dpm($loc);
    // dpm($dimension);
    preg_match('/([A-Z]+)[0-9]+:([A-Z]+)[0-9]+/', $dimension, $matches);
    // dpm($matches);
    if(count($matches) < 3) {
      // The dimensions are invalid (probably an empty worksheet)
    }
    else {
      // Matches found dimensions, continue processing this as a valid sheet
      $left_hex = unpack('H*', $matches[1]);
      $hex = $left_hex[1];
      $right_hex = unpack('H*', $matches[2]);
      if ($first_left == NULL) {
        $first_left = $left_hex[1];
        $last_right = $left_hex[1] - 1;
      }
      while (base_convert($hex, 16, 10) <= base_convert($right_hex[1], 16, 10)) {
        $hex = tpps_increment_hex($hex);
        $last_right = tpps_increment_hex($last_right);
      }
      $dims[] = array($left_hex[1], $right_hex[1]);
      $reader = new XMLReader();
      $reader->open($loc);
      $readers[] = $reader;
    }
  }

  // If the file has a header row, skip it.
  if (!$no_header) {
    tpps_xlsx_get_rows($readers, $strings);
  }

  // Iterate through file.
  $count = 0;
  while (($rows = tpps_xlsx_get_rows($readers, $strings, TRUE, $columns))) {
    if (!empty($max_rows) and $count >= $max_rows) {
      break;
    }
    $count++;

    $values = array();
    $key_hex = $first_left;
    foreach ($readers as $idx => $read) {
      $row = $rows[$idx];
      if (!empty($row)) {
        ksort($row);
      }
      $hex = $dims[$idx][0];
      while (base_convert($hex, 16, 10) <= base_convert($dims[$idx][1], 16, 10)) {
        $key = pack('H*', $key_hex);
        $row_key = pack('H*', $hex);
        if (empty($columns) or array_search($key, $columns) !== FALSE) {
          $values[$key] = isset($row[$row_key]) ? trim($row[$row_key]) : NULL;
        }
        $hex = tpps_increment_hex($hex);
        $key_hex = tpps_increment_hex($key_hex);
      }
    }
    yield $values;
  }

  // Close readers.
  foreach ($readers as $reader) {
    $reader->close();
  }
  tpps_rmdir($dir);
}

/**
 * This function is s generator for flat files.
 *
 * If the 'max_rows' option is set, the generator stop after max_rows items are
 * yielded. If the 'columns' option is set, then each item yielded by the
 * generator will contain only the specified columns from the row.
 *
 * @param string $location
 *   The location of the file on the server.
 * @param array $options
 *   Additional options detailing how to parse the file.
 *   Keys are:
 *     'no_header' - bool Should header be skipped or not.
 *     'columns' - array List of columns to process. ['A', 'B', 'C'].
 *     'max_rows' - int How many rows must be returned.
 *
 * @return Generator|array
 *   Yields each row of the file (or columns of a row, if applicable).
 */
function tpps_flat_generator($location, array $options = array()) {
  $original = ini_get('auto_detect_line_endings');
  if (!$original) {
    ini_set('auto_detect_line_endings', TRUE);
  }

  $no_header = $options['no_header'] ?? FALSE;
  $columns = $options['columns'] ?? NULL;
  $max_rows = $options['max_rows'] ?? NULL;

  $delim = ',';
  $longest = tpps_flat_width($location, $delim);
  $handle = gzopen($location, 'r');

  if (!$no_header) {
    gzgets($handle);
  }
  $count = 0;

  while (($line = gzgets($handle))) {
    $vals = str_getcsv($line, $delim);
    if (!empty($max_rows) and $count >= $max_rows) {
      break;
    }
    $count++;

    $values = array();

    if (empty($columns)) {
      $hex = unpack('H*', 'A')[1];
      for ($i = 0; $i < $longest; $i++) {
        $key = pack('H*', $hex);
        $values[$key] = isset($vals[$i]) ? trim($vals[$i]) : NULL;
        if (isset($values[$key]) and !check_plain($values[$key])) {
          $values[$key] = trim(mb_convert_encoding($values[$key], "UTF-8", "Windows-1252"));
        }
        $hex = tpps_increment_hex($hex);
      }
      yield $values;
      continue;
    }

    foreach ($columns as $column) {
      $values[$column] = $vals[tpps_convert_colname($column)] ?? NULL;
    }

    yield $values;
  }

  fclose($handle);
  ini_set('auto_detect_line_endings', $original);
}

/**
 * Removes managed by Drupal file.
 *
 * @param int $fid
 *   Managed File Id.
 *
 * @return bool
 *   Returns TRUE for success,
 *   FALSE in the event of an error,
 *   or an array if the file is being used by any modules.
 */
function tpps_file_remove($fid) {
  if ($fid && $file = file_load($fid)) {
    file_usage_delete($file, 'tpps', 'tpps_project');
    if ($result = file_delete($file)) {
      watchdog('tpps', 'Removed file: <pre>@file</pre>',
       ['@file' => print_r($file, 1)]
      );
    }
    return $result;
  }
  return FALSE;
}

/**
 * Adds file usage if necessary.
 *
 * Adds file usage only once to allow file correct deletion.
 *
 * @param object $file
 *   File Object. See file_load($fid).
 * @param string $accession
 *   Stury accession (TGDRxxxx) but integer accession number (xxxx)
 *   also could be used.
 */
function tpps_file_add_usage($file, $accession) {
  $file_usage = file_usage_list($file);
  $file_id = str_replace('TGDR', '', $accession);
  if (empty($file_usage['tpps']['tpps_project'][$file_id] ?? NULL)) {
    file_usage_add($file, 'tpps', 'tpps_project', $file_id);
  }
}

/**
 * Gets list of column names in file (header).
 *
 * Note: We use the same function as a file iterator callback because it's
 * very simple and there is no need to have a separate function just to return
 * received parameter.
 *
 * Difference from tpps_file_headers():
 * 1. Not used special keys for columns like 'A', 'B' and etc.
 * 2. Not used extra function as File Iterrator callback.
 * 3. Returns simple (not indexed) array (not array of arrays).
 *
 * @param mixed $data
 *   When called 1st time $data will be File Id.
 *   When called as a file iterator it will be an array $row.
 * @param array $options
 *   Options for File Iteraror function.
 *
 * @return array
 *   Returns list of column name in file.
 */
function tpps_file_get_header($data, array &$options = []) {
  if (empty($options) && !is_array($data)) {
    // This is 1st call and $data is a File Id.
    $options = [
      'no_header' => TRUE,
      'max_rows' => 1,
    ];
    tpps_file_iterator($data, 'tpps_file_get_header', $options);
    return $options['header'] ?? NULL;
  }
  else {
    // Function was called as file iterator callback it $data is $row.
    $options['header'] = $data;
  }
}
