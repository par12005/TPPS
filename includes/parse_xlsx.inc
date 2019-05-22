<?php

/**
 * @file
 */

/**
 *
 */
function tpps_parse_xlsx($location) {

  // Modified from https://gist.github.com/searbe/3284011
  $content = array();
  $dir = drupal_realpath("public://tpps_xlsx");

  $zip = new ZipArchive();
  $zip->open($location);
  $zip->extractTo($dir);

  $strings = simplexml_load_file($dir . '/xl/sharedStrings.xml');
  $sheet = simplexml_load_file($dir . '/xl/worksheets/sheet1.xml');

  $xlrows = $sheet->sheetData->row;

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
      $values = array_pad($arr, count($headers), '');
      $row = array_combine($headers, $values);
      $content[] = $row;
    }
  }

  @unlink($dir);
  @unlink($inputFile);

  return $content;

}
