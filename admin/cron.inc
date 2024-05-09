<?php

/**
 * @file
 * Settings for cron jobs.
 */

/**
 * Menu callback. Shows settings form.
 */
function tpps_cron_settings_form(array $form, array &$form_state) {
  $form['notes'] = [
    '#markup' => t("This should be a valid crontab schedule string. If "
      . "you don't know what that is or would like more information, see !link."
      . "<br />Please also note that if you change this value, you will need "
      . "to clear your site's cache in order for the change to take effect.",
      ['!link' => l(t('here'), 'http://man7.org/linux/man-pages/man5/crontab.5.html')]
    ),
  ];
  $form['tpps_latest_job_status_slack_updates_job_frequency'] = [
    '#type' => 'textfield',
    '#title' => t('TPPS Jobs Slack Updates'),
    '#default_value' => variable_get(
      'tpps_latest_job_status_slack_updates_job_frequency',
      '*/15 * * * *'
    ),
  ];
  $form['tpps_refresh_views_job_frequency'] = [
    '#type' => 'textfield',
    '#title' => t('TPPS Refresh Views Cron Job Frequency'),
    '#default_value' => variable_get('tpps_refresh_views_job_frequency',
      '0 */12 * * *'
    ),
  ];
  $form['tpps_refresh_plusgeno_view_frequency'] = [
    '#type' => 'textfield',
    '#title' => t('Refresh TPPS PlusGeno View (TPPS directory listing)'),
    '#default_value' => variable_get('tpps_refresh_plusgeno_view_frequency',
      '0 0 * * SAT'
    ),
  ];
  $form['tpps_delayed_submissions_job_frequency'] = [
    '#type' => 'textfield',
    '#title' => t('TPPS Delayed Submissions Cron Job Frequency'),
    '#default_value' => variable_get('tpps_delayed_submissions_job_frequency',
      '0 */6 * * *'
    ),
  ];
  $form['tpps_cron_rule_ref_genomes_recreate'] = [
    '#type' => 'textfield',
    '#title' => t('Recreate table "chado.tpps_ref_genomes"'),
    '#default_value' => variable_get('tpps_cron_rule_ref_genomes_recreate',
      '0 0 */3 * *'
    ),
  ];

  return system_settings_form($form);
}
