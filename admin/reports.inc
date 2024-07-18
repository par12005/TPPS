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
  // Titles.
  $form['titles'] = [
    '#type' => 'fieldset',
    '#title' => t('Reports Titles'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  //$form['titles']['tpps_report_imported_studies_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t('Missing DOI Report Title'),
  //  '#default_value' => variable_get(
  //    'tpps_report_imported_studies_title',
  //    'Imported Studies'
  //  ),
  //];
  //$form['titles']['tpps_report_missing_doi_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t('Missing DOI Report Title'),
  //  '#default_value' => variable_get(
  //    'tpps_report_missing_doi_title',
  //    'Missing DOI'
  //  ),
  //];
  //$form['titles']['tpps_report_missing_files_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t('Files missing in filesystem Report Title'),
  //  '#default_value' => variable_get(
  //    'tpps_report_missing_files_title',
  //    'Files missing in filesystem'
  //  ),
  //];
  //$form['titles']['tpps_report_no_synonym_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t('No Synonyms Report Title'),
  //  '#default_value' => variable_get(
  //    'tpps_report_no_synonym_title',
  //    'List of phenotypes without Synonyms'
  //  ),
  //];
  //$form['titles']['tpps_report_unit_warning_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t('Unit Warning Report Title'),
  //  '#default_value' => variable_get(
  //    'tpps_report_unit_warning_title',
  //    'Unit Warning (list of phenotypes which unit differs from Synonym)'
  //  ),
  //];
  //$form['titles']['tpps_report_order_family_not_exist_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t('Order/Family not exist Report Title'),
  //  '#default_value' => variable_get(
  //    'tpps_report_order_family_not_exist_title',
  //    'Order/Family not exist'
  //  ),
  //];
  //$form['titles']['tpps_report_submissions_missing_files_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t("Submission's Missing Files Report Title"),
  //  '#default_value' => variable_get(
  //    'tpps_report_submissions_missing_files_title',
  //    "Submission's Missing Files"
  //  ),
  //];
  //$form['titles']['tpps_report_submissions_without_files_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t('Submissions without Files Report Title'),
  //  '#default_value' => variable_get(
  //    'tpps_report_submissions_without_files_title',
  //    'Submissions without Files'
  //  ),
  //];
  //$form['titles']['tpps_report_submissions_all_files_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t("Submission's All Files Report Title"),
  //  '#default_value' => variable_get(
  //    'tpps_report_submissions_all_files_title',
  //    "Submission's All Files"
  //  ),
  //];
  //$form['titles']['tpps_report_variable_list_title'] = [
  //  '#type' => 'textfield',
  //  '#title' => t("'TPPS module variables' Report Title"),
  //  '#default_value' => variable_get('tpps_report_variable_list_title',
  //    'TPPS module variables'
  //  ),
  //];
  dpm('sdfsd');
  foreach (tpps_report_get_list() as $name => $report) {
    $title = variable_get('tpps_report_' . $name . '_title',
      str_replace('_', ' ', $name)
    );
    $form['titles']['tpps_report_' . $name . '_title'] = [
      '#type' => 'textfield',
      '#title' => t("'@default_title' Report Title", ['@title' => $title]),
      '#default_value' => $title,
    ];
  }

  return system_settings_form($form);
}
