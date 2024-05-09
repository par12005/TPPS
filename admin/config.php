<?php

/**
 * @file
 * Defines the admin settings form at admin/config/content/tpps.
 *
 * @TODO Make each fieldset separate tab.
 */

/**
 * Creates the admin settings form and loads default settings.
 *
 * @param array $form
 *   The form being built.
 * @param array $form_state
 *   The state of the form being built.
 *
 * @return array
 *   The system settings form.
 */
function tpps_admin_settings(array $form, array &$form_state) {

  $authors = variable_get('tpps_author_files_dir', 'tpps_authors');
  $photos = variable_get('tpps_study_photo_files_dir', 'tpps_study_photos');
  $accession = variable_get('tpps_accession_files_dir', 'tpps_accession');
  $genotype = variable_get('tpps_genotype_files_dir', 'tpps_genotype');
  $phenotype = variable_get('tpps_phenotype_files_dir', 'tpps_phenotype');
  $cartogratree_env = variable_get('tpps_cartogratree_env', FALSE);
  $tpps_db_csv_directory = variable_get('tpps_db_csv_directory', '/tmp/tpps/');
  $tpps_db_directory_user = variable_get('tpps_db_directory_user', 'postgres');


  $form['tpps_db_csv_directory'] = array(
    '#type' => 'textfield',
    '#title' => 'Database CSV Directory',
    '#suffix' => '<div>If you have an external database server, you must provide a shared directory which the database has access to read import (CSV) generated files or study submissions will fail to run (default: /tmp/tpps/)</div>',
    '#default_value' => $tpps_db_csv_directory
  );

  $form['tpps_db_csv_directory_user'] = array(
    '#type' => 'textfield',
    '#title' => 'Database directory user',
    '#suffix' => '<div>The shared directory must be owned by the correct user in order for it to be read by the database process, this will try to set the directory permissions to the user your specify here (default: postgres)</div>',
    '#default_value' => $tpps_db_directory_user
  );

  $form['tpps_maps_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS Google Maps API Key'),
    '#default_value' => variable_get('tpps_maps_api_key', NULL),
  );

  $form['tpps_ncbi_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS NCBI EUtils API Key'),
    '#default_value' => variable_get('tpps_ncbi_api_key', NULL),
  );

  $form['tpps_geocode_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS OpenCage Geocoding API Key'),
    '#default_value' => variable_get('tpps_geocode_api_key', NULL),
  );

  $form['tpps_unpublished_days_threshold'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of days before an unpublished study gets highlighted in TPPS Admin panel'),
    '#default_value' => variable_get('tpps_unpublished_days_threshold', 180),
  );

  $form['tpps_gps_epsilon'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS GPS Epsilon'),
    '#default_value' => variable_get('tpps_gps_epsilon', .001),
    '#description' => t('This is the amount of error TPPS should allow for when trying to match plants. An epsilon value of 1 is around 100km, and an epsilon value of .001 is around 100 m.'),
  );

  $form['tpps_zenodo_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS Zenodo API Key'),
    '#default_value' => variable_get('tpps_zenodo_api_key', NULL),
  );

  $form['tpps_zenodo_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS Zenodo Prefix'),
    '#default_value' => variable_get('tpps_zenodo_prefix', ''),
    '#description' => t('For testing and development purposes. Set this field to "sandbox." to create dois in the Zenodo sandbox rather than the real site. Please keep in mind that you will need a separate API key for sandbox.zenodo.org.'),
  );

  $form['tpps_admin_email'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS Admin Email Address'),
    '#default_value' => variable_get('tpps_admin_email', ''),
  );

  $form['tpps_cartogratree_env'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use environmental layers from CartograPlant'),
    '#default_value' => $cartogratree_env,
    '#description' => t("If CartograPlant is installed, TPPS can add an optional field to the environment section for environment layers, using the data pulled in through CartograPlant."),
  );

  $form['tpps_latest_job_status_slack_updates_api_url'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS Latest Job Status Slack Updates API URL'),
    '#default_value' => variable_get('tpps_latest_job_status_slack_updates_api_url', NULL),
  );

  $form['tpps_latest_job_status_slack_updates_last_job_id'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS Latest Job Status Slack Updates Last Job ID'),
    '#default_value' => variable_get('tpps_latest_job_status_slack_updates_last_job_id', NULL),
  );

  if (module_exists('cartogratree') and db_table_exists('cartogratree_groups') and db_table_exists('cartogratree_layers')) {
    $form['tpps_ct_api_key'] = array(
      '#type' => 'textfield',
      '#title' => t('TPPS CartograPlant API Key'),
      '#default_value' => variable_get('tpps_ct_api_key', NULL),
    );

    $form['layer_groups'] = array(
      '#type' => 'fieldset',
      '#title' => 'CartograPlant Environmental Layer Groups:',
      '#description' => t('Please select which layer groups will contain environmental data that is relevant to TPPS. TPPS will use the selected groups to decide which layers to present as environmental options to the users.'),
      '#states' => array(
        'visible' => array(
          ':input[name="tpps_cartogratree_env"]' => array('checked' => TRUE),
        ),
      ),
    );

    $results = db_select('cartogratree_groups', 'g')
      ->fields('g', array('group_name', 'group_id'))
      ->execute();

    while (($result = $results->fetchObject())) {
      $form['layer_groups']["tpps_layer_group_{$result->group_id}"] = array(
        '#type' => 'checkbox',
        '#title' => $result->group_name,
        '#default_value' => variable_get("tpps_layer_group_{$result->group_id}", FALSE),
      );
    }
  }

  $form['tpps_record_group'] = array(
    '#type' => 'textfield',
    '#title' => t('TPPS Record max group'),
    '#default_value' => variable_get('tpps_record_group', 10000),
    '#description' => t('Some files are very large. TPPS tries to submit as many entries together as possible, in order to speed up the process of writing data to the database. However, very large size entries can cause errors within the Tripal Job daemon. This number is the maximum number of entries that may be submitted at once. Larger numbers will make the process faster, but are more likely to cause errors. Defaults to 10,000.'),
  );

  $form['tpps_local_genome_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('Reference Genome file directory:'),
    '#default_value' => variable_get('tpps_local_genome_dir', NULL),
    '#description' => t('The directory of local genome files on your web server. If left blank, tpps will skip the searching for local genomes step in the tpps genotype section. Local genome files should be organized according to the following structure: <br>[file directory]/[species code]/[version number]/[genome data] where: <br>&emsp;&emsp;[file directory] is the full path to the genome files provided above <br>&emsp;&emsp;[species code] is the 4-letter standard species code - this must match the species code entry in the "chado.organismprop" table<br>&emsp;&emsp;[version number] is the reference genome version, of the format "v#.#"<br>&emsp;&emsp;[genome data] is the actual reference genome files - these can be any format or structure<br>More information is available <a href="https://tpps.rtfd.io/en/latest/config.html" target="blank">here</a>.'),
  );

  $form['tpps_author_files_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('Author files:'),
    '#default_value' => $authors,
    '#description' => t("Currently points to @path.", array('@path' => drupal_realpath("public://$authors"))),
    '#prefix' => t('<h1>File Upload locations</h1>All file locations are relative to the "public://" file stream. Your current "public://" file stream points to "@path".<br><br>', array('@path' => drupal_realpath('public://'))),
  );

  $form['tpps_study_photo_files_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('Study photo files:'),
    '#default_value' => $photos,
    '#description' => t("Currently points to @path.", array('@path' => drupal_realpath("public://$photos"))),
  );

  $form['tpps_tree_pics_files_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('Plant Pictures directory:'),
    '#default_value' => variable_get('tpps_tree_pics_files_dir', NULL),
    '#description' => t("The directory of plant pictures on your web server. If you do not have any plant pictures on your web server, you can leave this field blank. Currently points to @path.", array('@path' => drupal_realpath("public://" . variable_get('tpps_tree_pics_files_dir', NULL)))),
  );

  $form['tpps_accession_files_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('Plant Accession files:'),
    '#default_value' => $accession,
    '#description' => t("Currently points to @path.", array('@path' => drupal_realpath("public://$accession"))),
  );

  $form['tpps_genotype_files_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('Genotype files:'),
    '#default_value' => $genotype,
    '#description' => t("Currently points to @path.", array('@path' => drupal_realpath("public://$genotype"))),
  );

  $form['tpps_phenotype_files_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('Phenotype files:'),
    '#default_value' => $phenotype,
    '#description' => t("Currently points to @path.", array('@path' => drupal_realpath("public://$phenotype"))),
  );

  $form['tpps_update_old_submissions'] = array(
    '#type' => 'checkbox',
    '#title' => t('Update Old TPPS Submissions'),
    '#description' => t('If you save configuration with this option enabled, TPPS will search for all older TPPS Submissions that are no longer compatible with newer versions of TPPS, and will make them compatible again. This works best after using the "tpps/update" tool from the "update_old_submissions" branch on the TPPS gitlab.'),
    '#default_value' => variable_get('tpps_update_old_submissions', NULL),
  );
  // [VS] #3v6kz7k
  // Custom Reports.
  $form['custom_reports'] = [
    '#type' => 'fieldset',
    '#title' => t('Custom Reports'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];
  $form['custom_reports']['tpps_report_no_synonym_title'] = [
    '#type' => 'textfield',
    '#title' => t('Title of "No synonym" report'),
    '#default_value' => variable_get('tpps_report_no_synonym_title'),
    '#description' => t('Used on admin panel page and as page title on report page.'),
  ];
  $form['custom_reports']['tpps_report_unit_warning_title'] = [
    '#type' => 'textfield',
    '#title' => t('Title of "Unit Warning" report'),
    '#default_value' => variable_get('tpps_report_unit_warning_title'),
    '#description' => t('Used on admin panel page and as page title on report page.'),
  ];
  $form['custom_reports']['tpps_report_order_family_not_exist_title'] = [
    '#type' => 'textfield',
    '#title' => t('Title of "Order/Family Not Exist" report'),
    '#default_value' => variable_get('tpps_report_order_family_not_exist_title'),
    '#description' => t('Used on admin panel page and as page title on report page.'),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Theme Settings.
  $form['theme_settings'] = [
    '#type' => 'fieldset',
    '#title' => t('Theme Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];
  // Pager fix.
  $form['theme_settings']['tpps_theme_fix_pager'] = [
    '#type' => 'checkbox',
    '#title' => t('Restore original position of "Next" button of pager'),
    '#default_value' => variable_get('tpps_theme_fix_pager'),
    '#description' => t("Theme 'Dawn' moves 'Next' button of pager to the "
      . "very right position (after 'Last' button. <br />When checked "
      . "original position (before 'Last' button) will be restored."),
  ];
  // [/VS]

  return system_settings_form($form);
}

/**
 * Implements hook_form_validate().
 *
 * Validates administrative TPPS settings.
 */
function tpps_admin_settings_validate($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if (substr($key, -10) == '_files_dir') {
      $location = "public://$value";
      if (!file_prepare_directory($location, FILE_CREATE_DIRECTORY)) {
        form_set_error("$key", t("Error: path must be valid and current user "
          . "must have permissions to access that path."));
      }
    }
    elseif ($key == 'tpps_admin_email') {
      if (!valid_email_address($value)) {
        form_set_error("$key", t("Error: please enter a valid email address."));
      }
    }
    elseif ($key == 'tpps_cartogratree_env') {
      if (!empty($value) and !module_exists('cartogratree')) {
        form_set_error("$key", t("Error: The CartograPlant module is not installed."));
      }
      elseif (
        !empty($value)
        && (
          !db_table_exists('cartogratree_groups')
          or !db_table_exists('cartogratree_layers')
          or !db_table_exists('cartogratree_fields')
        )
      ) {
        form_set_error("$key", t("Error: TPPS was unable to find the required "
          . "CartograPlant tables for environmental layers."));
      }
    }
    elseif ($key == 'tpps_zenodo_prefix') {
      if ($value and $value != 'sandbox.') {
        form_set_error("$key", t("Error: Zenodo Prefix must either be empty or 'sandbox.'"));
      }
    }
    elseif ($key == 'tpps_unpublished_days_threshold') {
      if (empty($value) or !preg_match('/^[0-9]+$/', $value)) {
        form_set_error("$key", t("Error: please enter a valid number of days"));
      }
    }
    elseif ($key == 'tpps_update_old_submissions' and !empty($value)) {
      $update = TRUE;
    }
  }

  if (!empty($update) and !form_get_errors()) {
    tpps_update_old_submissions();
  }
}

/**
 * Used to move old submissions to the new TPPS submission table.
 *
 * Older versions of TPPS stored submissions in the public.variable table, but
 * newer versions use the public.tpps_submission table. This function moves
 * previously completed submissions to the new table. It works best if the
 * form_states of the old submissions are already correctly formatted within the
 * public.variable table.
 */
function tpps_update_old_submissions() {
  $query = db_select('variable', 'v')
    ->fields('v')
    ->condition('name', db_like('tpps_complete_') . '%', 'LIKE')
    ->execute();

  while (($result = $query->fetchObject())) {
    $mail = substr($result->name, 14, -7);
    $user = user_load_by_mail($mail);
    $state = unserialize($result->value);
    if (!empty($user)) {
      $uid = $user->uid;
    }
    else {
      $uid = 21;
    }
    $accession = $state['accession'];
    $dbxref_id = $state['dbxref_id'];
    $status = $state['status'];
    db_insert('tpps_submission')
      ->fields(array(
        'uid' => $uid,
        'status' => $status,
        'accession' => $accession,
        'dbxref_id' => $dbxref_id,
        'submission_state' => serialize($state),
      ))
      ->execute();
    variable_del($result->name);
  }
}
