<?php

/**
 * @file
 * Shows ubmission's version changes or list of versions.
 */

/**
 * Menu callback. Shows ubmission's version changes or list of versions.
 */
function tpps_report_submission_version() {
  $version = arg(5);

  if (empty($version)) {
    // Show list of types when nothing yet selected.
    $path = 'admin/reports/tpps/submission/version/';
    // @todo Get list of versions dynamically.
    $version_list = [1, 2, 3];
    foreach ($version_list as $n) {
      $list[] = l(t('Version @number', ['@number' => $n]), $path . $n);
    }
    return theme('item_list', ['items' => $list]);
  }
  else {
    $parents = array_slice(arg(), 6);
    if ($version == 1) {
      $title = t(
        'Migration actions to get Version @version',
        ['@version' => $version]
      );
    }
    else {
      $title = t(
        'Version changes from version @prev_version to version @version',
        ['@version' => $version, '@prev_version' => $version - 1]
      );
    }
    $title .= ' [' . implode('/', $parents) . ']';
    drupal_set_title($title);
    if ($version == 1) {
      return t("It's initial version and there is no previous version.");
    }
    else {
      module_load_include('inc', 'tpps', 'includes/submission_form_version_changes');
      $list = array_keys(SubmissionCache::getAll());
      $latest_accession = reset($list);
      $submission = new Submission($latest_accession);
      $changes = tpps_submission_form_version_changes($submission->state, $version);
      if (!empty($parents)) {
        $changes = TppsArray::get($changes, $parents);
      }
      return TppsArray::print($changes);
    }
  }
}
