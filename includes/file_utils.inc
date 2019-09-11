<?php

/**
 * @file
 * Defines useful functions for file management
 */

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

      $add = 1;
      $new = "";
      for ($i = strlen($key) - 1; $i >= 0; $i--) {
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
      $hex = $new;
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
      $values[$key] = isset($vals[$key]) ? $vals[$key] : NULL;

      $add = 1;
      $new = "";
      for ($i = strlen($key) - 1; $i >= 0; $i--) {
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
      $hex = $new;
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
  @unlink($inputFile);

  return $content;

}

/**
 * 
 */
function tpps_rename_file($fid, $new_name, array $options = array()) {
  if (!array_key_exists('preserve', $options)) {
    $options['preserve'] = FALSE;
  }

  if (!empty($fid) and ($file = file_load($fid))) {
    if (preg_match('/^(.*\/)(.*)$/', $new_name, $matches)) {
      $new_filename = $new_name;
    }
    else {
      preg_match('/^(.*\/).*$/', $file->uri, $matches);
      $new_filename = $matches[1] . $new_name;
    }

    if ($options['preserve']) {
      return file_copy($file, $new_filename);
    }
    return file_move($file, $new_filename);
  }
  throw new Exception("Error: could not open file");
}
