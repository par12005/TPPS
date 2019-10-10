<?php

/**
 * @file
 * Defines useful functions for file management.
 */

/**
 * This function parses a file and returns an arary of its content.
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
function tpps_parse_file($fid, $max_rows = 0, $no_header = FALSE) {
  $file = file_load($fid);
  $location = drupal_realpath($file->uri);
  $extension = tpps_get_path_extension($location);
  switch ($extension) {
    case 'xlsx':
      return tpps_parse_xlsx($location, $max_rows, $no_header);
    case 'txt':
    case 'csv':
      return tpps_parse_flat($location, $max_rows, $no_header);
    default:
      throw new Exception('Unrecognized file type');
  }
}

/**
 * This function parses a .csv file and returns an array of its content.
 *
 * @param string $location
 *   The path on the server to the file being parsed.
 * @param int $max_rows
 *   The maximum number of rows to read from the file.
 *
 * @return array
 *   An array containing the content headers and content body.
 */
function tpps_parse_flat($location, $max_rows = 0, $no_header = FALSE) {
  $original = ini_get('auto_detect_line_endings');
  if (!$original) {
    ini_set('auto_detect_line_endings', TRUE);
  }

  $content = array();
  $delim = ',';
  $longest = tpps_longest_line($location, $delim);
  $handle = fopen($location, 'r');
  $count = 0;

  if ($no_header) {
    $hex = unpack('H*', 'A')[1];
    for ($i = 0; $i < $longest; $i++) {
      $key = pack('H*', $hex);
      $content['headers'][$key] = $i;
      $hex = tpps_increment_hex($hex);
    }
  }

  while (($line = fgets($handle))) {
    $hex = unpack('H*', 'A')[1];
    $vals = explode($delim, $line);
    $values = array();
    for ($i = 0; $i < $longest; $i++) {
      $key = pack('H*', $hex);
      $values[$key] = $vals[$i] ?? NULL;
      if (!check_plain($values[$key])) {
        $values[$key] = mb_convert_encoding($values[$key], "UTF-8", "Windows-1252");
      }
      $hex = tpps_increment_hex($hex);
    }

    if (!isset($content['headers'])) {
      $content['headers'] = $values;
      continue;
    }

    $count++;
    $content[] = $values;
    if ($max_rows > 0 and $count >= $max_rows) {
      break;
    }
  }

  fclose($handle);
  ini_set('auto_detect_line_endings', $original);
  return $content;
}

/**
 * This function parses an .xlsx file and returns an array of its content.
 *
 * @param string $location
 *   The path on the server to the file being parsed.
 * @param int $max_rows
 *   The maximum number of rows to read from the file.
 *
 * @return array
 *   An array containing the content headers and content body.
 */
function tpps_parse_xlsx($location, $max_rows = 0, $no_header = FALSE) {

  // Modified from https://gist.github.com/searbe/3284011
  $content = array();
  $dir = drupal_realpath("public://tpps_xlsx");

  $zip = new ZipArchive();
  $zip->open($location);
  $zip->extractTo($dir);

  $strings = simplexml_load_file($dir . '/xl/sharedStrings.xml');
  $sheet = simplexml_load_file($dir . '/xl/worksheets/sheet1.xml');

  $xlrows = $sheet->sheetData->row;

  // Get width of the data in the file.
  preg_match('/([A-Z]+)[0-9]+:([A-Z]+)[0-9]+/', $sheet->dimension['ref'], $matches);
  $left_hex = unpack('H*', $matches[1]);
  $hex = $left_hex[1];
  $right_hex = unpack('H*', $matches[2]);

  if ($no_header) {
    while (base_convert($hex, 16, 10) <= base_convert($right_hex[1], 16, 10)) {
      $key = pack('H*', $hex);
      $content['headers'][$key] = base_convert($hex, 16, 10) - base_convert($left_hex[1], 16, 10);
      $hex = tpps_increment_hex($hex);
    }
  }

  $count = 0;
  foreach ($xlrows as $xlrow) {
    $vals = array();

    foreach ($xlrow->c as $cell) {

      $v = isset($cell->v) ? (float) $cell->v : NULL;
      if (!isset($v)) {
        continue;
      }

      if (isset($cell['t']) && $cell['t'] == 's') {
        $v = (string) $strings->si[(int) $v]->t;
      }

      preg_match('/([A-Z]+)([0-9]+)/', (string) $cell['r'], $matches);
      $vals[$matches[1]] = $v;
    }

    ksort($vals);
    $values = array();
    $hex = $left_hex[1];
    while (base_convert($hex, 16, 10) <= base_convert($right_hex[1], 16, 10)) {
      $key = pack('H*', $hex);
      $values[$key] = $vals[$key] ?? NULL;
      $hex = tpps_increment_hex($hex);
    }

    if (!isset($content['headers'])) {
      $content['headers'] = $values;
    }
    else {
      $count++;
      $content[] = $values;
      if ($max_rows > 0 and $count >= $max_rows) {
        break;
      }
    }
  }

  @unlink($dir);

  return $content;

}

/**
 *
 */
function tpps_file_len($fid) {
  $file = file_load($fid);
  $location = drupal_realpath($file->uri);
  $extension = tpps_get_path_extension($location);
  switch ($extension) {
    case 'xlsx':
      return tpps_xlsx_len($location);
    case 'txt':
    case 'csv':
      return tpps_flat_len($location);
    default:
      throw new Exception('Unrecognized file type');
  }
}

/**
 *
 */
function tpps_flat_len($location) {
  $original = ini_get('auto_detect_line_endings');
  if (!$original) {
    ini_set('auto_detect_line_endings', TRUE);
  }

  $cont = 0;
  $handle = fopen($location, 'r');
  while (fgets($handle)) {
    $count++;
  }

  fclose($handle);
  ini_set('auto_detect_line_endings', $original);
  return $count;
}

/**
 *
 */
function tpps_xlsx_len($location) {
  $count = 0;
  $dir = drupal_realpath("public://tpps_xlsx");

  $zip = new ZipArchive();
  $zip->open($location);
  $zip->extractTo($dir);

  $sheet = simplexml_load_file($dir . '/xl/worksheets/sheet1.xml');
  $xlrows = $sheet->sheetData->row;

  @unlink($dir);

  return count($xlrows);
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
function tpps_longest_line($location, $delim = ',') {
  $longest = 0;
  $handle = fopen($location, 'r');

  while (($line = fgets($handle))) {
    $longest = max($longest, count(explode($delim, $line)));
  }

  fclose($handle);
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
 *  echo pack('H*', tpps_increment_hex(unpack('H*', 'ZZZ')[1]));  // AAAA
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
    if ($i == 0 and $add) {
      $new = '41' . $new;
    }
  }
  return $new;
}

/**
 * 
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
      return file_copy($file, $new_filename);
    }
    return file_move($file, $new_filename);
  }
  throw new Exception("Error: could not open file");
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
function tpps_compare_files($fid_1, $fid_2, $file_1_id_name, $file_2_id_name) {
  $file_1_content = tpps_parse_file($fid_1);
  $file_2_content = tpps_parse_file($fid_2);

  if (gettype($file_1_id_name) == 'integer') {
    $file_1_content[] = $file_1_content['headers'];
  }
  if (gettype($file_2_id_name) == 'integer') {
    $file_2_content[] = $file_2_content['headers'];
  }

  $missing_ids = array();

  for ($i = 0; $i < count($file_1_content) - 1; $i++) {
    $row = $file_1_content[$i];

    $needle_id = $row[$file_1_id_name];
    $found = FALSE;

    for ($j = 0; $j < count($file_2_content) - 1; $j++) {
      if ($needle_id === $file_2_content[$j][$file_2_id_name]) {
        $found = TRUE;
        break;
      }
    }

    if (!$found) {
      array_push($missing_ids, $needle_id);
    }
  }

  return $missing_ids;
}

/**
 * This function validates that the file contains all of the required groups.
 *
 * A "required group" is a column type that must be present within the file for
 * the file to be considered valid. For instance, a required group for a tree
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
  $cols = $file_element['#value']['columns'];

  $parents = $file_element['#parents'];
  $new_end_columns = end($parents) . "-columns";
  $new_end_no_header = end($parents) . "-no-header";
  $new_end_empty = end($parents) . "-empty";
  $group_path = array_pop($parents) . "-groups";
  $values = &$form_state['values'];
  foreach ($parents as $item) {
    $values = &$values[$item];
  }
  // Initialize form column values in form state.
  $values[$new_end_columns] = array();
  // Hold onto the location of the columns in form state.
  $state_column_values = &$values[$new_end_columns];
  $values[$new_end_no_header] = isset($file_element['#value']['no-header']) ? $file_element['#value']['no-header'] : NULL;
  $values[$new_end_empty] = isset($file_element['#value']['empty']) ? $file_element['#value']['empty'] : NULL;

  $title_parts = explode(':', $file_element['#title']);
  $error_prompt = $title_parts[0];

  $groups = array();
  $required_groups_flat = array();
  foreach ($required_groups as $group => $combinations) {
    $groups[$group] = array();
    $required_groups_flat[$group] = array();
    foreach ($combinations as $name => $combination) {
      $required_groups_flat[$group] = array_merge($required_groups_flat[$group], $combination);
    }
  }

  // dpm($required_groups_flat);
  // dpm($cols);
  // dpm($state_column_values);
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
  // dpm($groups);
  foreach ($required_groups as $group => $combinations) {
    $group_valid = FALSE;
    $groups[$group]['#type'] = array();
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
      form_set_error($file_element['#name'] . "[columns][$group", "$error_prompt: Please specify a column or columns that hold $group.");
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
 */
function tpps_save_file_columns(array &$form, array &$form_state) {

  if ($form_state['stage'] == TPPS_PAGE_3) {
    for ($i = 1; $i <= $form_state['stats']['species_count']; $i++) {
      if (($file = file_load($form_state['values']['tree-accession']["species-$i"]['file'])) and $form_state['values']['tree-accession']["species-$i"]['file'] != 0) {
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }

      $form_state['values']['tree-accession']["species-$i"]['file-columns'] = array();

      if (isset($form['tree-accession']["species-$i"]['file']['#value']['columns'])) {
        foreach ($form['tree-accession']["species-$i"]['file']['#value']['columns'] as $col => $val) {
          if ($col[0] != '#') {
            $form_state['values']['tree-accession']["species-$i"]['file-columns'][$col] = $form['tree-accession']["species-$i"]['file']['#value']['columns'][$col];
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

        if (!empty($form_state['values']["organism-$i"]['genotype']['files']['snps-assay']) and ($file = file_load($form_state['values']["organism-$i"]['genotype']['files']['snps-assay']))) {
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

          $form_state['values']["organism-$i"]['genotype']['files']['snps-assay-columns'] = array();

          if (isset($form["organism-$i"]['genotype']['files']['snps-assay']['#value']['columns'])) {
            foreach ($form["organism-$i"]['genotype']['files']['snps-assay']['#value']['columns'] as $col => $val) {
              if ($col[0] != '#') {
                $form_state['values']["organism-$i"]['genotype']['files']['snps-assay-columns'][$col] = $form["organism-$i"]['genotype']['files']['snps-assay']['#value']['columns'][$col];
              }
            }
          }
        }

        if (!empty($form_state['values']["organism-$i"]['genotype']['files']['other']) and ($file = file_load($form_state['values']["organism-$i"]['genotype']['files']['other']))) {
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

          $form_state['values']["organism-$i"]['genotype']['files']['other-columns'] = array();

          if (isset($form["organism-$i"]['genotype']['files']['other']['#value']['columns'])) {
            foreach ($form["organism-$i"]['genotype']['files']['other']['#value']['columns'] as $col => $val) {
              if ($col[0] != '#') {
                $form_state['values']["organism-$i"]['genotype']['files']['other-columns'][$col] = $form["organism-$i"]['genotype']['files']['other']['#value']['columns'][$col];
              }
            }
          }
        }
      }
    }

    if (isset($form['organism-1']['phenotype'])) {
      for ($i = 1; $i <= $form_state['saved_values'][TPPS_PAGE_1]['organism']['number']; $i++) {
        if (!isset($form["organism-$i"]['phenotype']['iso'])) {
          if (($file = file_load($form_state['values']["organism-$i"]['phenotype']['file'])) and $form_state['values']["organism-$i"]['phenotype']['file'] != 0) {
            file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          }

          $form_state['values']["organism-$i"]['phenotype']['file-columns'] = array();

          if (isset($form["organism-$i"]['phenotype']['file']['#value']['columns'])) {
            foreach ($form["organism-$i"]['phenotype']['file']['#value']['columns'] as $col => $val) {
              if ($col[0] != '#') {
                $form_state['values']["organism-$i"]['phenotype']['file-columns'][$col] = $form["organism-$i"]['phenotype']['file']['#value']['columns'][$col];
              }
            }
          }

          if ($form_state['values']["organism-$i"]['phenotype']['check'] != '0') {
            if (($file = file_load($form_state['values']["organism-$i"]['phenotype']['metadata'])) and $form_state['values']["organism-$i"]['phenotype']['metadata'] != 0) {
              file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
            }

            $form_state['values']["organism-$i"]['phenotype']['metadata-columns'] = array();

            if (isset($form["organism-$i"]['phenotype']['metadata']['#value']['columns'])) {
              foreach ($form["organism-$i"]['phenotype']['metadata']['#value']['columns'] as $col => $val) {
                if ($col[0] != '#') {
                  $form_state['values']["organism-$i"]['phenotype']['metadata-columns'][$col] = $form["organism-$i"]['phenotype']['metadata']['#value']['columns'][$col];
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
  return $matches[1];
}
