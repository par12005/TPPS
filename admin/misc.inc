<?php

/**
 * @file
 * TPPS Misc Settings Form.
 */

/**
 * TPPS Misc Settings Form.
 */
function tpps_misc_settings_form(array $form, array &$form_state) {
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form['general'] = [
    '#type' => 'fieldset',
    '#title' => t('General Settings'),
    '#collapsible' => TRUE,
  ];
  $variable_name = 'tpps_watchdog_cleanup_filter';
  $filter_example = [
    [
      'uid' => '0',
      'type' => 'php',
      'message' => 'Cleaned up @count released locks',
      'variables' => [
        'Invalid argument supplied for foreach()',
        '248',
        '/var/www/Drupal/sites/all/modules/file_entity/file_entity.file.inc',
      ],
      'severity' => WATCHDOG_WARNING,
    ],
  ];
  $form['general']['tpps_watchdog_cleanup'] = [
    '#type' => 'checkbox',
    '#title' => t('Remove spam watchdog messages'),
    '#default_value' => variable_get('tpps_watchdog_cleanup'),
    '#description' => t('When enabled predefined rules to remove watchdog-'
      . 'messages will be used. Default: unchecked. '
      . '<br />New rules could be added by code changes or using variable '
      . '"@variable_name" which value is array of arrays.'
      . 'When "variables" key is an array of strings then each must be found '
      . ' to remove watchdog record.'
      . '<br />Example: <pre>@example</pre>',
      [
        '@example' => var_export($filter_example, 1),
        '@variable_name' => $variable_name,
      ]
    ),
  ];
  $form['general']['tpps_watchdog_cleanup_filter'] = [
    '#markup' => '<h3>' . t('Current value of the variable "@variable_name":',
      ['@variable_name' => $variable_name]) . '</h3>'
    . '<pre>' . var_export(variable_get($variable_name, []), 1) . '</pre>',
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validation Regular Expressions.
  $form['validation_regex'] = [
    '#type' => 'fieldset',
    '#title' => t('Validation Regex'),
    '#collapsible' => TRUE,
    '#description' => t('To disable validation leave regex field empty.'),
  ];
  // WARNING:
  //
  // Be sure to update tpps_add_css_js() when new regex added.
  //
  // To add new item:
  // 1. Create new settings below.
  // 2. Add variable to tpps_install.inc files in hook_install()/uninstall().
  // 3. Update list of regexes in tpps_add_css_js().
  $form['validation_regex']['tpps_validation_regex_doi'] = [
    '#type' => 'textfield',
    '#title' => t('DOI'),
    '#description' => t('Regex to check if DOI format is valid.'),
    '#default_value' => variable_get(
      'tpps_validation_regex_doi',
      '/^10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+$/'
    ),
    '#description' => t('Default is: <br/>'
      . '/^10\.\d{4,9}[\-._;()\/:A-Za-z0-9]+$/'
      . '<br />Examples of valid DOI: !list',
      ['!list' => theme('item_list', ['items' => tpps_doi_get_examples_list()])]
    ),
  ];

  // @TODO Update regex. See
  // https://docs.google.com/document/d/1xd3BcZHQArdpVPa3z6mcu87iErS7d9jXn_pcoQ8wNXw/edit?tab=t.0
  // https://regex101.com/r/HTOtYn/2
  $form['validation_regex']['tpps_validation_regex_organism_name'] = [
    '#type' => 'textfield',
    '#title' => t('Organism Name'),
    '#description' => t('Regex to check if organism name is valid.'),
    '#default_value' => variable_get(
      'tpps_validation_regex_organism_name',
      '/^[^\s]+\s[^\s]+(\s[^\s]+)*$/'
    ),
    '#description' => t('Default is: <br/>'
      . '/^[^\s]+\s[^\s]+(\s[^\s]+)*$/'
      . '<br />Work in progress: <br />'
      . '/^[^\s]+\s[^\s]+(\s[subsp.|spp.|subspecies|var.|varieta|variety|f.|forma|form]+\s[^\s]+)*$/'
    ),
  ];
  $form['validation_regex']['tpps_validation_regex_single_space'] = [
    '#type' => 'textfield',
    '#title' => t('Single Space'),
    '#description' => t('Regex to check if string contains only one space.'),
    '#default_value' => variable_get(
      'tpps_validation_regex_single_space',
      '/^[^\s]*\s[^\s]*$/'
    ),
    '#description' => t('Default is: <br/>'
      . '/^[^\s]*\s[^\s]*$/'
    ),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // NCBI API Settings.
  $form['ncbi_settings'] = [
    '#type' => 'fieldset',
    '#title' => t('NCBI API Settings'),
    '#collapsible' => TRUE,
  ];
  $form['ncbi_settings']['tpps_ncbi_api_delay'] = [
    '#type' => 'textfield',
    '#title' => t('NCBI API Request Delay, milliseconds'),
    '#default_value' => variable_get('tpps_ncbi_api_delay'),
    '#description' => t('Value must be positive integer. This delay will '
      . 'dynamically increase every fail request up to 5 times.'
      . '<br />Recommended values is: 100 milliseconds.'
      . '<br /><a href="@url">NCBI API has limit: 3 requests per second</a>.'
      . 'But seems actual limit is a little bit higher (maybe 10 rps).',
      [
        '@url' => 'https://support.nlm.nih.gov/kbArticle/?pn=KA-05318'
      ]
    ),
    '#element_validate' => ['element_validate_integer_positive'],
  ];
  $form['ncbi_settings']['tpps_ncbi_api_max_attempt_number'] = [
    '#type' => 'textfield',
    '#title' => t('Max number of attempts when NCBI API Request fails'),
    '#default_value' => variable_get('tpps_ncbi_api_max_attempt_number'),
    '#element_validate' => ['element_validate_integer_positive'],
  ];
  $form['ncbi_settings']['tpps_ncbi_allow_fix_missing_data'] = [
    '#type' => 'checkbox',
    '#title' => t('Allows to fix missing NCBI Taxonomy data'),
    '#default_value' => variable_get('tpps_ncbi_allow_fix_missing_data'),
    '#description' => t('When tab "Species" on study details page loads and '
      . 'NCBI Taxomony data is missing then request NCBI server again and '
      . 'store data into correct chado-tables.'),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Theme Settings.
  $form['theme_settings'] = [
    '#type' => 'fieldset',
    '#title' => t('Theme Settings'),
    '#collapsible' => TRUE,
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
  return system_settings_form($form);
}
