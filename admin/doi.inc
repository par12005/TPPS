<?php

/**
 * @file
 * DOI Lookup settings.
 *
 * See classes:
 * - /src/Doi.class.inc.
 */

/**
 * DOI Lookup Settings Form.
 */
function tpps_doi_settings_form(array $form, array &$form_state) {
  $form['info'] = [
    '#markup' => theme('item_list', [
      'title' => t('Useful links'),
      'items' => [
        l(t('Report "DOI Duplicates"'), 'admin/reports/tpps/doi/duplicates'),
        l(t('Report "Missing DOI"'), 'admin/reports/tpps/doi/missing'),
      ]
    ]),
  ];
  $form['tpps_doi_lookup_method'] = [
    '#type' => 'select',
    '#title' => t('Lookup method'),
    '#options' => [
      'gemini' => t('Gemini'),
      'google_scholar' => t('Google Scholar Scraper'),
    ],
    '#default_value' => variable_get('tpps_doi_lookup_method'),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Google Scholar settings.
  //
  // Resources:
  // Class: /src/GoogleScholar.class.inc.
  // DB table: public.tpps_google_scholar.
  //
  // @TODO Minor. Create report for db table.
  $form['google_scholar'] = [
    '#type' => 'fieldset',
    '#title' => t('Google Scholar Scraper'),
    '#collapsible' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="tpps_doi_lookup_method"]' => ['value' => 'google_scholar'],
      ],
    ],
  ];
  $form['google_scholar']['info'] = [
    '#markup' => theme('item_list', [
      'items' => [
        'DB table: public.tpps_google_scholar',
      ],
    ]),
  ];
  $form['google_scholar']['tpps_doi_google_scholar_debug_mode'] = [
    '#type' => 'checkbox',
    '#title' => t('Debug Mode'),
    '#default_value' => variable_get('tpps_doi_google_scholar_debug_mode'),
    '#description' => t('When enabled no actual requests to Google Scholar '
      . 'will be performed.'),
  ];
  $form['google_scholar']['tpps_doi_google_scholar_delay'] = [
    '#type' => 'textfield',
    '#title' => t('Mininal delay between requests (in seconds)'),
    '#default_value' => variable_get(
      'tpps_doi_google_scholar_delay', GoogleScholar::DEFAULT_DELAY
    ),
    '#description' => t('Introduce random delays (e.g., 2-10 seconds) '
      . 'between requests to avoid rate-limiting.'),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // DOI / Gemini.
  $form['gemini'] = [
    '#type' => 'fieldset',
    '#title' => t('Gemini'),
    '#collapsible' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="tpps_doi_lookup_method"]' => ['value' => 'gemini'],
      ],
    ],
    '#description' => t('Using Gemini AI allows to avoid risks caused by scrapping.')
    . '<br />' . t('Be sure to set API Key at !setting_page.',
      [
        '!setting_page' => l(
          t('Gemini Settings page'),
          'admin/config/tpps/gemini'
        ),
      ]
    )
  ];
  $form['gemini']['info'] = [
    '#markup' => theme('item_list', [
      'items' => [
        l(t('Gemini: System instructions and other configurations'),
          'https://ai.google.dev/gemini-api/docs/text-generation#system-instructions'
        ),
        l(t('Gemini: GenerationConfig'),
          'https://ai.google.dev/api/generate-content#v1beta.GenerationConfig'
        ),
        l(t('Gemini: Response Schema'),
          'https://ai.google.dev/api/caching#Schema'
        ),
        l(t('JSON text generation guide'),
          'https://ai.google.dev/gemini-api/docs/structured-output?example=recipe'
        ),
      ]
    ]),
  ];
  $form['gemini']['tpps_doi_gemini_system_instruction'] = [
    '#type' => 'textarea',
    '#title' => t('Gemini System Instruction'),
    '#description' => t('Format: plain text. You can guide the behavior of '
      . 'Gemini models with system instructions.') . '<br />'
      . l(t('Gemini: System instructions and other configurations'),
        'https://ai.google.dev/gemini-api/docs/text-generation#system-instructions'
    ),
    '#default_value' => variable_get('tpps_doi_gemini_system_instruction', ''),
  ];
  $form['gemini']['tpps_doi_gemini_generation_config'] = [
    '#type' => 'textarea',
    '#title' => t('Gemini Generation Config Object'),
    '#description' => t('Format: JSON. See !link.', [
      '!link' => l(t('Gemini: GenerationConfig'),
        'https://ai.google.dev/api/generate-content#v1beta.GenerationConfig'
      )
    ]),
    '#default_value' => variable_get('tpps_doi_gemini_generation_config', ''),
  ];

  return system_settings_form($form);
}
