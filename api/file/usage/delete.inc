<?php

/**
 * @file
 * Deleted usage from TPPS Submission File.
 */

/**
 * Menu callback. Removes usage from file.
 *
 * @param object $file
 *   Fully loaded managed file object.
 * @param string $accession
 *   Study accession. E.g., 'TGDR1040'.
 * @param int $count
 *   (optional) The number of references to delete from the object.
 *   Defaults to 1. 0 may be specified to delete all references to the
 *   file within a specific object.
 */
function tpps_api_file_usage_delete($file, $accession, $count = NULL) {
  $count = $count ?? arg(6) ?? 1;
  TppsFile::deleteUsage($file, $accession, $count);
  $message = 'Removed all usage for file #@fid for submission "@accession".';
  if ($count) {
    $message = 'Removed @count usage for file #@fid for submission "@accession".';
  }
  return [
    'message' => t($message,
      ['@count' => $count, '@fid' => $file->fid, '@accession' => $accession]
    ),
  ];
}
