<?php

/**
 * @file
 * Shows list of Submissions with files of certain type.
 */

/**
 * Menu callback. Lists submissions with files of certain type.
 */
function tpps_report_submission_file_type() {
  $type = arg(6);
  if (empty($type)) {
    // Show list of types when nothing yet selected.
    $path = 'admin/reports/tpps/submission/file/type/';
    $list = [];
    foreach (FileField::getClassList() as $class_name) {
      $list[] = l(
        constant($class_name . '::TITLE'),
        $path . constant($class_name . '::DOM_NAME')
      );
    }
    return theme('item_list', ['items' => $list]);
  }
  else {
    drupal_set_title(t('Submissions File Type "@type"', ['@type' => $type]));
    $file_name = str_replace('.inc', '',
      Report::getFilePath('all', ['submission', 'file'])
    );
    module_load_include('inc', 'tpps', $file_name);
    return tpps_report_submission_file($type);
  }
}
