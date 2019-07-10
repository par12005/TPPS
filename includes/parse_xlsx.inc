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

      $v = (string) $cell->v;

      if (isset($cell['t']) && $cell['t'] == 's') {
        $s = array();
        $si = $strings->si[(int) $v];
        $si->registerXPathNamespace('n', 'http://schemas.openxmlformats.org/spreadsheetml/2006/main');

        foreach ($si->xpath('.//n:t') as $t) {
          $s[] = (string) $t;
        }

        $v = implode($s);
      }
      $arr[] = $v;
    }

    if (!isset($headers)) {
      $headers = $arr;
      $content['headers'] = $headers;
    }
    else {
      $count++;
      $values = array_pad($arr, count($headers), '');
      $row = array_combine($headers, $values);
      $content[] = $sequence ? $values : $row;
      if ($max_rows > 0 and $count >= $max_rows) {
        break;
      }
    }
  }

  @unlink($dir);
  @unlink($inputFile);

  return $content;

}
