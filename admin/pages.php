<?php

/**
 * @file
 * File diagnostics page.
 */

/**
 * Menu callback. Shows list of files used by study.
 *
 * This function will check study submission state from database
 * find the file ids and also check the managed tables to see what is
 * missing. This will thus detect old files.
 *
 * @param string $accession
 *   Study accession in format 'TGDRxxxx'.
 *
 * @return string
 *   Returns rendered list of files.
 */
function tpps_admin_files_diagnostics_page($accession = NULL) {
  // Exclude 'submission_interface' form results.
  $fields = implode(', ', array_diff(
    array_keys(drupal_get_schema('tpps_submission')['fields']),
    ['submission_interface']
  ));
  $results = chado_query(
    "SELECT $fields FROM tpps_submission WHERE accession = :accession",
    [':accession' => $accession]
  );
  $serialized_data = "";
  foreach ($results as $row) {
    $serialized_data = unserialize($row->submission_state);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $project_file_ids = [];
  // Get file_ids from project_id.
  $project_id = $serialized_data['ids']['project_id'];
  $results = chado_query(
    'SELECT * FROM tpps_project_file_managed WHERE project_id = :project_id',
    [':project_id' => $project_id]
  );
  foreach ($results as $row) {
    array_push($project_file_ids, $row->fid);
  }
  sort($project_file_ids);
  $saved_values = $serialized_data['saved_values'];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $file_ids = [];
  $organism_count = $saved_values['1']['organism']['number'];
  for ($j = 1; $j <= $organism_count; $j++) {
    $parents = [
      // Page 3. Accession file.
      [TPPS_PAGE_3, 'tree-accession', 'species-' . $j, 'file'],
      // Page 4. Phenotype files.
      [TPPS_PAGE_4, 'organism-' . $j, 'phenotype', 'file'],
      [TPPS_PAGE_4, 'organism-' . $j, 'phenotype', 'metadata'],
      // Page 4. Genotype files.
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'snps-assay'],
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'snps-association'],
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'vcf'],
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'ssrs'],
      [TPPS_PAGE_4, 'organism-' . $j, 'genotype', 'files', 'ssrs_extra'],
    ];
    foreach ($parents as $parent) {
      if ($value = drupal_array_get_nested_value($saved_values, $parent)) {
        array_push($file_ids, $value);
      }
    }
  }
  sort($file_ids);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // History/State files.
  $overall_file_ids = $serialized_data['files'] ?? [];
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
      if ($file = file_load($fid)) {
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
