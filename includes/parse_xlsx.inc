<?php

/**
 * @file
 * Defines a function used to parse excel files for their content.
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
function tpps_parse_xlsx($location, $max_rows = 0, $sequence = FALSE) {

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
  $left = base_convert($left_hex[1], 16, 10);
  $right_hex = unpack('H*', $matches[2]);
  $right = base_convert($right_hex[1], 16, 10);
  
  $count = 0;
  foreach ($xlrows as $xlrow) {
    $vals = $arr = array();

    foreach ($xlrow->c as $cell) {

      $v = isset($cell->v) ? (int) $cell->v : NULL;
      if (!isset($v)) {
        continue;
      }
      
      if (isset($cell['t']) && $cell['t'] == 's') {
        $v = (string) $strings->si[(int) $v]->t;
      }
      
      preg_match('/([A-Z]+)([0-9]+)/', (string) $cell['r'], $matches);
      $vals[$matches[1]] = $v;
      $arr[] = $v;
    }

    ksort($vals);
    $values = array();
    for ($i = $left; $i <= $right; $i++) {
      $key = pack('H*', base_convert($i, 10, 16));
      if (isset($content['headers'])) {
        $index = $sequence ? $key : $content['headers'][$i - $left];
        $values[$index] = isset($vals[$key]) ? $vals[$key] : '';
      }
      else {
        $index = $sequence ? $key : $i - $left;
        $values[$index] = isset($vals[$key]) ? $vals[$key] : $key;
      }
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
