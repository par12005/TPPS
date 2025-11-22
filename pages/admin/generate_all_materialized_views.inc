<?php

/**
 * @file
 * Generates all materialized views.
 */

module_load_include('inc', 'tpps', 'includes/common');

/**
 * Menu callback for Generates all materialized views.
 */
function tpps_manage_generate_all_materialized_views() {
  global $user;
  module_load_include('php', 'tpps', 'forms/submit/submit_all');

  $includes = [];
  $includes[] = module_load_include('php', 'tpps', 'forms/submit/submit_all');
  $includes[] = module_load_include('inc', 'tpps', 'includes/file_parsing');
  $args = [];
  $jid = tripal_add_job("Generate materialized views for all studies",
    'tpps', 'tpps_generate_all_genotype_materialized_views',
    $args, $user->uid, 10, $includes, TRUE
  );

  drupal_set_message(
    t('A job has been created to (re)generate materialized views for all studies')
  );
  drupal_goto('tpps-admin-panel');
}
