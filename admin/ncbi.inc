<?php

/**
 * @file
 * NCBI API Settings Form.
 */

/**
 * NCBI API Settings Form.
 */
function tpps_ncbi_settings_form(array $form, array &$form_state) {
  $form['links'] = [
    '#markup' => theme('item_list', [
      'title' => t('Useful Links'),
      'items' => [
        l(t('NCBI Taxonomy Id Batch Updates'), 'admin/config/tpps/ncbi_batch'),
      ],
    ]),
  ];

  $form['tripal_eutils_ncbi_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('TPPS NCBI EUtils API Key'),
    // WARNING: We set veriable for another module here.
    '#default_value' => variable_get('tripal_eutils_ncbi_api_key'),
    '#description' => t('<strong>WARNING: Other module variable will be set.'
      . '</strong><br />This key used for NCBI API requests.'),
  ];
  $form['tpps_ncbi_api_delay'] = [
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
  $form['tpps_ncbi_api_max_attempt_number'] = [
    '#type' => 'textfield',
    '#title' => t('Max number of attempts when NCBI API Request fails'),
    '#default_value' => variable_get('tpps_ncbi_api_max_attempt_number'),
    '#element_validate' => ['element_validate_integer_positive'],
  ];
  return system_settings_form($form);
}
