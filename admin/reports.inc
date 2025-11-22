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
  // 'tpps_report_missing_files_title'.
  // 'tpps_report_variable_list_title'.
  // 'tpps_report_synonym_missing_title'.
  // 'tpps_report_synonym_unit_warning_title'.
  // 'tpps_report_organism_list_title'.
  // 'tpps_report_organism_new_title'.
  // 'tpps_report_organism_order_family_not_exist_title'.
  // 'tpps_report_doi_duplicates_title'.
  // 'tpps_report_doi_missing_title'.
  // 'tpps_report_zenodo_deposition_title'.
  // 'tpps_report_submission_imported_title'.
  // 'tpps_report_submission_zenodo_title'.
  // 'tpps_report_submission_file_all_title',
  // 'tpps_report_submission_file_none_title'.
  // 'tpps_report_submission_file_missing_title'.
  // 'tpps_report_submission_file_type_title'.
  // 'tpps_report_submission_version_title'.
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
