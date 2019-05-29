<?php

/**
 * @file
 * Defines function to format content in the 'no_header' case.
 */

/**
 * Alters content from a file for a 'no_header' case.
 *
 * If the user indicates that their file has no header row, then the content
 * collected by tpps_parse_xlsx will need to be altered by creating a new row
 * for headers and moving the old headers into the main content.
 *
 * @param array $content
 *   The content to be altered.
 */
function tpps_content_no_header(array &$content) {
  $top_row = $content['headers'];
  $headers = array();
  for ($i = 0; $i < count($content['headers']); $i++) {
    $headers[$i] = $i;
  }
  $other_rows = array_slice($content, 1);

  foreach ($other_rows as $row_num => $row) {
    $new_row = array();
    for ($i = 0; $i < count($row); $i++) {
      $new_row[$i] = $row[$top_row[$i]];
    }
    $other_rows[$row_num] = $new_row;
  }
  array_unshift($other_rows, $top_row);

  $content = $other_rows;
  $content['headers'] = $headers;
}
