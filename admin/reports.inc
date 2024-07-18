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
