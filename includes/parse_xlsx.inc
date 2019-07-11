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

  $count = 0;
  foreach ($xlrows as $xlrow) {
    $arr = array();

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

    if (!isset($headers)) {
      $headers = $sequence ? $vals : $arr;
      $header_vals = $vals;
      $content['headers'] = $headers;
    }
    else {
      $count++;
      $values = array_pad($arr, count($headers), '');
      if (!$sequence and count($content) == 1 and count($headers) < count($values)) {
        $i = 0;
        foreach ($vals as $key => $val) {
            $headers[$i] = isset($header_vals[$key]) ? $header_vals[$key] : $key;
          $i++;
        }
        $content['headers'] = $headers;
      }
      $content[] = $sequence ? $vals : array_combine($headers, $values);
      if ($max_rows > 0 and $count >= $max_rows) {
        break;
      }
    }
  }

  @unlink($dir);
  @unlink($inputFile);

  return $content;

}
