<?php

/**
 * @file
 * Google Scholar settings.
 *
 * Resources:
 * Class: /src/GoogleScholar.class.inc.
 * DB table: public.tpps_google_scholar.
 *
 * @TODO Minor. Create report for db table.
 */

/**
 * Google Scholar Settings Form.
 */
function tpps_google_scholar_settings_form(array $form, array &$form_state) {
  $form['tpps_google_scholar_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('Debug Mode'),
    '#default_value' => variable_get('tpps_google_scholar_debug_mode'),
    '#description' => t('When enabled no actual requests to Google Scholar '
      . 'will be performed.'),
  ];
  $form['tpps_google_scholar_delay'] = [
    '#type' => 'textfield',
    '#title' => t('Mininal delay between requests (in seconds)'),
    '#default_value' => variable_get(
      'tpps_google_scholar_delay', GoogleScholar::DEFAULT_DELAY
    ),
    '#description' => t('Introduce random delays (e.g., 2-10 seconds) '
      . 'between requests to avoid rate-limiting.'),
  ];
  $form['info'] = [
    '#markup' => theme('item_list', [
      'items' => [
        'DB table: public.tpps_google_scholar',
      ],
    ]),
  ];

  return system_settings_form($form);
}
