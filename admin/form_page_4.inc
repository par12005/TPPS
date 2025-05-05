<?php

/**
 * @file
 * TPPS Page 4 Settings Form.
 */

/**
 * Generates settings form.
 */
function tpps_form_page_4_settings_form(array $form, array &$form_state) {
  $form['tpps_page_4_cache_synonym_list'] = [
    '#type' => 'checkbox',
    '#title' => t('Cache list of Synonyms'),
    '#default_value' => variable_get('tpps_page_4_cache_synonym_list', FALSE),
    '#description' => t(
      'Default is <strong>unchecked</strong>.<br />'
      . "When checked list of synonyms will be cached using custom bin 'tpps'."
    ),
  ];
  $form['tpps_page_4_cache_unit_list'] = [
    '#type' => 'checkbox',
    '#title' => t('Cache lists of Units'),
    '#default_value' => variable_get('tpps_page_4_cache_unit_list', FALSE),
    '#description' => t(
      'Default is <strong>unchecked</strong>.<br />'
      . "When checked list of units per synonym will be cached using "
      . "custom bin 'tpps'."
    ),
  ];
  $form['tpps_page_4_update_ploidy_description'] = [
    '#type' => 'checkbox',
    '#title' => t('Update Ploidy Description'),
    '#default_value' => variable_get('tpps_page_4_update_ploidy_description', TRUE),
    '#description' => t(
      'Default is <strong>checked</strong>.'
      . "<br />When checked description of the 'Ploidy' field will be "
      . " updated according to current selection."
    ),
  ];
  $form['tpps_page_4_use_alphabetic_list'] = [
    '#type' => 'checkbox',
    '#title' => t('Use Alphabetic Lists'),
    '#default_value' => variable_get('tpps_page_4_use_alphabetic_list', TRUE),
    '#description' => t(
      'Default is <strong>checked</strong>.<br />When checked lists will be '
      . 'splitted by letter to improve readability. Could be applied to '
      . 'synonym and unit lists. Example: <pre>@unit_list</pre>',
      ['@unit_list' => print_r(tpps_unit_get_list('1'), 1)]
    ),
  ];
  $form['tpps_page_4_env_check_always_visible'] = [
    '#type' => 'checkbox',
    '#title' => t('Field "Is environmental phenotype" is always visible'),
    '#default_value' => variable_get('tpps_page_4_env_check_always_visible'),
    '#description' => t(
      'Default is <strong>checked</strong>.<br />'
      . "When checked this checkbox will be always visible. When unchecked it "
      . "will be only visible when 'I can't find my phenotype' option of the "
      . "'Phenotype/Synonym' field was selected."
    ),
  ];
  $form['tpps_page_4_env_check_ajax_update'] = [
    '#type' => 'checkbox',
    '#title' => t('Field "Is environmental phenotype" will update form.'),
    '#default_value' => variable_get('tpps_page_4_env_check_ajax_update'),
    '#description' => t(
      'Default is <strong>checked</strong>.<br />'
      . "When checked and 'Is environmental phenotype' checkbox is checked then "
      . " it reduces number of options in the 'Structure' dropdown list when "
      . "'Is environmental phenotype' checkbox selected."
    ),
  ];

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
    // @TODO Minor. Clear only unit/synonym lists.
    cache_clear_all('*', $cache_bin, TRUE);
  }
}
