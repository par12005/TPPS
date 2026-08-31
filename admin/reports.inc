<?php

/**
 * @file
 * Settings for cron jobs.
 */

/**
 * Menu callback. Shows settings form.
 *
 * Reports settings.
 */
function tpps_reports_settings_form(array $form, array &$form_state) {
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // General.
  $form['general'] = [
    '#type' => 'fieldset',
    '#title' => t('General Report Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['general']['tpps_report_items_per_page'] = [
    '#type' => 'textfield',
    '#title' => t('Number of items per page'),
    '#default_value' => variable_get('tpps_report_items_per_page', 20),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Report titles.
  $form['titles'] = [
    '#type' => 'fieldset',
    '#title' => t('Reports Titles'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  // For searching purposes list of dynamically generated variables.
  // 'tpps_report_variable_list_title'.
  // File:
  // 'tpps_report_file_missing_title'.
  // 'tpps_report_file_not_used_title'.
  // 'tpps_report_file_orphan_title'.
  //
  // Synonym:
  // 'tpps_report_synonym_missing_title'.
  // 'tpps_report_synonym_unit_warning_title'.
  //
  // Organism:
  // 'tpps_report_organism_list_title'.
  // 'tpps_report_organism_all_title'.
  // 'tpps_report_organism_genus_title'.
  // 'tpps_report_organism_species_title'.
  // 'tpps_report_organism_new_title'.
  // 'tpps_report_organism_order_family_not_exist_title'.
  // 'tpps_report_organism_taxonomy_title'.
  // 'tpps_report_organism_synonym_list_title'.
  // 'tpps_report_organism_split'.
  //
  // DOI:
  // 'tpps_report_doi_duplicates_title'.
  // 'tpps_report_doi_missing_title'.
  //
  // DOI / Submission.
  // 'tpps_report_doi_submission_publication_title'.
  // 'tpps_report_doi_submission_dataset_title'.
  // 'tpps_report_doi_submission_all_title'.
  //
  // DOI / Publication Data.
  // 'tpps_report_doi_publication_data_all_title'.
  // 'tpps_report_doi_publication_data_pubmed_title'.
  // 'tpps_report_doi_publication_data_pubmed_central_title'.
  // 'tpps_report_doi_publication_data_google_scholar_title'.
  // 'tpps_report_doi_publication_data_semantic_scholar_title'.
  // 'tpps_report_doi_publication_data_crossref_title'.
  // 'tpps_report_doi_publication_data_dryad_title'.
  //
  // Zenodo:
  // 'tpps_report_zenodo_deposition_title',
  //
  // Submission:
  // 'tpps_report_submission_imported_title',
  // 'tpps_report_submission_zenodo_title',
  // 'tpps_report_submission_version_title',
  //
  // Submission/File:
  // 'tpps_report_submission_file_all_title',
  // 'tpps_report_submission_file_none_title',
  // 'tpps_report_submission_file_missing_title',
  // 'tpps_report_submission_file_type_title',
  //
  //  Rest API.
  // 'tpps_report_rest_api_request_list_title',
  // 'tpps_report_rest_api_request_body_title',
  // 'tpps_report_rest_api_response_list_title',
  // 'tpps_report_rest_api_response_body_title',
  //
  // NCBI API.
  // 'tpps_report_organism_ncbi_all_title',
  // 'tpps_report_organism_ncbi_missing_title',
  // GBIF API.
  // 'tpps_report_organism_gbif_all_title',
  // 'tpps_report_organism_gbif_missing_title'.
  // 'tpps_report_organism_taxonomy_title'.
  module_load_install('tpps');
  foreach (tpps_get_variable_list() as $name => $report) {
    if (
      strpos($name, 'tpps_report_') === FALSE
      || strpos($name, '_title') === FALSE
    ) {
      continue;
    }
    $default_title = ucwords(str_replace('_', ' ',
      str_replace('tpps_report_', '', str_replace('_title', '', $name))
    ));
    $title = variable_get($name, $default_title);
    $form['titles'][$name] = [
      '#type' => 'textfield',
      '#title' => t("'@title' Report Title", ['@title' => $title]),
      '#default_value' => $title,
    ];
  }

  return system_settings_form($form);
}
