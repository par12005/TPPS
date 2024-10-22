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
  // List of variable's names for search purpuse.
  // 'tpps_report_missing_doi_title',
  // 'tpps_report_order_family_not_exist_title',
  // 'tpps_report_no_synonym_title',
  // 'tpps_report_unit_warning_title',
  // 'tpps_report_submissions_all_files_title',
  // 'tpps_report_submissions_without_files_title',
  // 'tpps_report_submissions_missing_files_title',
  // 'tpps_report_missing_files_title',
  // 'tpps_report_imported_studies_title',
  // 'tpps_report_variable_list_title',
  // 'tpps_report_organism_list_title',
  // 'tpps_report_new_organisms_title',
  // and etc.
  foreach (tpps_report_get_list() as $name => $report) {
    $title = variable_get('tpps_report_' . $name . '_title',
      str_replace('_', ' ', $name)
    );
    $form['titles']['tpps_report_' . $name . '_title'] = [
      '#type' => 'textfield',
      '#title' => t("'@title' Report Title", ['@title' => $title]),
      '#default_value' => $title,
    ];
  }

  return system_settings_form($form);
}
