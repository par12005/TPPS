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
  $form['titles'] = [
    '#type' => 'fieldset',
    '#title' => t('Reports Titles'),
  ];
  $form['titles']['tpps_report_imported_studies_title'] = [
    '#type' => 'textfield',
    '#title' => t('Missing DOI Report Title'),
    '#default_value' => variable_get(
      'tpps_report_imported_studies_title',
      'Imported Studies'
    ),
  ];
  $form['titles']['tpps_report_missing_doi_title'] = [
    '#type' => 'textfield',
    '#title' => t('Missing DOI Report Title'),
    '#default_value' => variable_get(
      'tpps_report_missing_doi_title',
      'Missing DOI'
    ),
  ];
  $form['titles']['tpps_report_missing_files_title'] = [
    '#type' => 'textfield',
    '#title' => t('Missing Files Report Title'),
    '#default_value' => variable_get(
      'tpps_report_missing_files_title',
      'List of missing files'
    ),
  ];
  $form['titles']['tpps_report_no_synonym_title'] = [
    '#type' => 'textfield',
    '#title' => t('No Synonyms Report Title'),
    '#default_value' => variable_get(
      'tpps_report_no_synonym_title',
      'List of phenotypes without Synonyms'
    ),
  ];
  $form['titles']['tpps_report_unit_warning_title'] = [
    '#type' => 'textfield',
    '#title' => t('Unit Warning Report Title'),
    '#default_value' => variable_get(
      'tpps_report_unit_warning_title',
      'Unit Warning (list of phenotypes which unit differs from Synonym)'
    ),
  ];
  $form['titles']['tpps_report_order_family_not_exist_title'] = [
    '#type' => 'textfield',
    '#title' => t('Order/Family not exist Report Title'),
    '#default_value' => variable_get(
      'tpps_report_order_family_not_exist_title',
      'Order/Family not exist'
    ),
  ];
  $form['titles']['tpps_report_study_missing_files_title'] = [
    '#type' => 'textfield',
    '#title' => t('Study Missing Files Report Title'),
    '#default_value' => variable_get(
      'tpps_report_study_missing_files_title',
      'Study Missing Files'
    ),
  ];

  return system_settings_form($form);
}
