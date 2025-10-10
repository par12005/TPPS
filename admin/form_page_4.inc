<?php

/**
 * @file
 * TPPS Page 4 Settings Form.
 */

/**
 * Generates settings form.
 */
function tpps_form_page_4_settings_form(array $form, array &$form_state) {
  $setting_list = [
    'tpps_page_4_cache_synonym_list' => [
      'title' => t('Cache list of Synonyms'),
      'description' => t(
        'Default is <strong>unchecked</strong>.<br />'
        . "When checked list of synonyms will be cached using custom bin 'tpps'."
      ),
    ],
    'tpps_page_4_cache_unit_list' => [
      'title' => t('Cache lists of Units'),
      'description' => t(
        'Default is <strong>unchecked</strong>.<br />'
        . "When checked list of units per synonym will be cached using "
        . "custom bin 'tpps'."
      ),
    ],
    'tpps_page_4_update_ploidy_description' => [
      'title' => t('Update Ploidy Description'),
      'description' => t(
        'Default is <strong>checked</strong>.'
        . "<br />When checked description of the 'Ploidy' field will be "
        . " updated according to current selection."
      ),
    ],
    'tpps_page_4_use_alphabetic_list' => [
      'title' => t('Use Alphabetic Lists'),
      'description' => t(
        'Default is <strong>checked</strong>.<br />When checked lists will be '
        . 'splitted by letter to improve readability. Could be applied to '
        . 'synonym and unit lists. Example: <pre>@unit_list</pre>',
        ['@unit_list' => print_r(tpps_unit_get_list('1'), 1)]
      ),
    ],
    'tpps_page_4_env_check_always_visible' => [
      'title' => t('Field "Is environmental phenotype" is always visible'),
      'description' => t(
        'Default is <strong>checked</strong>.<br />'
        . "When checked this checkbox will be always visible. When unchecked it "
        . "will be only visible when 'I can't find my phenotype' option of the "
        . "'Phenotype/Synonym' field was selected."
      ),
    ],
    'tpps_page_4_env_check_ajax_update' => [
      'title' => t('Field "Is environmental phenotype" will update form.'),
      'description' => t(
        'Default is <strong>checked</strong>.<br />'
        . "When checked and 'Is environmental phenotype' checkbox is checked then "
        . " it reduces number of options in the 'Structure' dropdown list when "
        . "'Is environmental phenotype' checkbox selected."
      ),
    ],
    'tpps_page_4_env_check_ajax_update' => [
      'title' => t('Field "Is environmental phenotype" will update form.'),
      'description' => t(
        'Default is <strong>checked</strong>.<br />'
        . "When checked and 'Is environmental phenotype' checkbox is checked "
        . "then it reduces number of options in the 'Structure' dropdown list "
        . "when 'Is environmental phenotype' checkbox selected."
      ),
    ],
    'tpps_page_4_phenotype_metadata_validate_synonyms' => [
      'title' => t('Validate synonyms in phenotype metadata file.'),
      'description' => t(
        'Default is <strong>checked</strong>.<br />'
        . "When checked each phenotype in metadata file will be matched "
        . "with exisitng synonyms using 'attribute' column and submission "
        . "will be blocked if any of them doesn't have matching synonym."
      ),
    ],
  ];
  foreach ($setting_list as $key => $item) {
    $form[$key] = [
      '#type' => 'checkbox',
      '#title' => $item['title'],
      '#default_value' => variable_get($key),
      '#description' => $item['description'],
    ];
  }

  $form['#submit'][] = 'tpps_form_page_4_settings_form_submit';
  return system_settings_form($form);
}

/**
 * Form submitter.
 */
function tpps_form_page_4_settings_form_submit(array $form, array &$form_state) {
  // Clear cache on change only.
  if (
    ($form_state['values']['tpps_page_4_cache_synonym_list'] ?? NULL)
      != variable_get('tpps_page_4_cache_synonym_list', FALSE)
    || ($form_state['values']['tpps_page_4_cache_unit_list'] ?? NULL)
      != variable_get('tpps_page_4_cache_unit_list', FALSE)
  ) {
    $cache_bin = TPPS_CACHE_BIN ?? 'cache';
    // Clear only unit/synonym lists.
    tpps_cache_clear_all(['tpps_synonym_list', 'tpps_unit_list']);
  }
}
