<?php

/**
 * @file
 * File diagnostics page.
 */

/**
 * Menu callback. Shows list of files used by study.
 *
 * WARNING: Submission Shared State used.
 *
 * This function will check study submission state from database
 * find the file ids and also check the managed tables to see what is
 * missing. This will thus detect old files.
 * Menu path:
 * /tpps-admin-panel/file-diagnostics/TGDR%%%
 *
 * @param string $accession
 *   Study accession in format 'TGDRxxxx'.
 *
 * @return string
 *   Returns rendered list of files.
 */
function tpps_admin_files_diagnostics_page($accession = NULL) {
  if (empty($accession)) {
    drupal_set_message(t('Empty accession.'), 'error');
    return '';
  }
  $submission = new Submission($accession);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $project_file_ids = [];
  // Get file_ids from project_id.
  $project_id = $submission->sharedState['ids']['project_id'] ?? NULL;
  $results = chado_query(
    'SELECT * FROM tpps_project_file_managed WHERE project_id = :project_id',
    [':project_id' => $project_id]
  );
  // @TODO Minor. Use Drupal DB API to get result in an array. fetchCol();
  foreach ($results as $row) {
    array_push($project_file_ids, $row->fid);
  }
  sort($project_file_ids);
  $saved_values = $submission->sharedState['saved_values'] ?? NULL;
  if (empty($saved_values)) {
    drupal_set_message(t('Empty "saved_values" in Submission Shared State.'));
    return ' ';
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $file_ids = [];
  for ($j = 1; $j <= $saved_values[TPPS_PAGE_1]['organism']['number']; $j++) {
    // WARNING: DO NOT UPDATE FIELDS!
    // It uses Submission Shared State (Form Version 1).
    $parents = [
      // Page 3. Accession file.
      [TPPS_PAGE_3, 'tree-accession', 'species-' . $j, 'file'],
      // Page 4. Phenotype files.
      [TPPS_PAGE_4, 'organism-' . $j, 'phenotype', 'file'],
      [TPPS_PAGE_4, 'organism-' . $j, 'phenotype', 'metadata'],
      // Page 4. Genotype files.
      // @TODO Minor. Add other files.
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'snps-assay'], // Shared State.
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'assay-design'],
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'snps-association'],
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'vcf'],
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'ssrs'],
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'ssrs_extra'],
      // Field Other spreadsheet wasn't there.
      //[TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'other'],
    ];
    foreach ($parents as $parent) {
      if ($value = drupal_array_get_nested_value($saved_values, $parent)) {
        array_push($file_ids, $value);
      }
    }
  }
  // Supplemental files at 'summarypage'.
  for ($i = 1; $i <= 10; $i++) {
    if (!empty($saved_values['summarypage']['files'][$i])) {
      $file_ids[] = $saved_values['summarypage']['files'][$i];
    }
  }

  sort($file_ids);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // History/State files.
  $overall_file_ids = $submission->sharedState['files'] ?? [];
  sort($overall_file_ids);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Output report.
  $file_lists = [
    'Project ID Managed' => $project_file_ids,
    'Submission state' => $file_ids,
    'History/State files' => $overall_file_ids,
  ];
  $options = ['attributes' => ['target' => '_blank']];
  foreach ($file_lists as $title => $fid_list) {
    $sublist = [];
    foreach ($fid_list as $fid) {
      if ($file = tpps_file_load($fid)) {
        $sublist[] = '<strong>' . $fid . '</strong> - '
          . l($file->filename, file_create_url($file->uri), $options);
      }
    }
    $markup = ($markup ?? '') . theme('item_list',
      ['title' => t($title), 'items' => $sublist, 'type' => 'ol']
    );
  }
  tpps_add_css_js('file_diagnostics');
  return '<div id="file-diagnostics">' . $markup . '</div>';
}
