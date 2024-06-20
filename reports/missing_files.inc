<?php

/**
 * @file
 * Generates list of the imported studies.
 */

/**
 * Menu callback. Shows list of files missing if filesystem.
 *
 * Files from 'file_managed' table will be checked.
 */
function tpps_admin_missing_files_report() {

  $start = arg(4) ?? 0;
  $end = arg(5) ?? NULL;

  $query = db_select('file_managed', 't')
    ->fields('t', ['fid', 'uri'])
    ->orderBy('fid', 'DESC');
  if ($end) {
    drupal_set_message(t('Shown items from @start to @end.',
      ['@start' => $start, '@end' => $end]
    ));
    $query->range($start, $end);
  }
  $files = $query->execute()->fetchAllKeyed();
  drupal_set_message(
    t('Processed @number items.', ['@number' => count($files)])
  );

  $rows = [];
  foreach ($files as $fid => $uri) {
    $path = drupal_realpath($uri);
    if (!file_exists($path)) {
      $rows[] = [$fid, $path];
    }
  }

  return theme('table', [
    'header' => [t('File Id'), t('Path')],
    'rows' => $rows,
  ]);
}
