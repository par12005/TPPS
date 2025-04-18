<?php

/**
 * @file
 * Add usage to TPPS Submission File.
 */

/**
 * Menu callback. Adds usage to file.
 *
 * @param object $file
 *   Fully loaded managed file object.
 * @param string $accession
 *   Study accession. E.g., 'TGDR1040'.
 * @param int $count
 *   (optional) The number of references to add to the object. Defaults to 1.
 */
function tpps_api_file_usage_add($file, $accession, $count = NULL) {
  $count = $count ?? arg(6) ?? 1;
  $id = str_replace('TGDR', '', $accession);
  file_usage_add($file, 'tpps', 'tpps_project', $id, $count);
  $message = 'Added @count usage for file #@fid and id "@id".';
  return [
    'message' => t($message,
      ['@count' => $count, '@fid' => $file->fid, '@id' => $id]
    ),
  ];
}
