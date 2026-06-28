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
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // CORS Proxy.
  $form['cors_proxy'] = [
    '#type' => 'fieldset',
    '#title' => t('CORS Proxy'),
    '#description' => t('CORS Proxy server settings.'),
  ];
  $form['cors_proxy']['tpps_doi_lookup_proxy'] = [
    '#type' => 'textfield',
    '#title' => t('CORS Proxy Server URL'),
    '#default_value' => variable_get('tpps_doi_lookup_proxy'),
    '#description' => t('We use <a href="@cors_anywhere_url">CORS Anywhere</a> '
      . 'which is a NodeJS proxy which adds CORS headers to the proxied '
      . 'request. So Google Scholar SERP could be shown at site in iframe.',
      ['@cors_anywhere_url' => 'https://github.com/Rob--W/cors-anywhere']
    )
    . theme('item_list',
      ['items' => ['https://tgwebdev.cam.uchc.edu/corsanywhere']]
    ),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // UI.
  $form['ui'] = [
    '#type' => 'fieldset',
    '#title' => t('User Interface settings'),
  ];
  $form['ui']['tpps_doi_lookup_show_report_links'] = [
    '#type' => 'checkbox',
    '#title' => t('Show DOI Report links'),
    '#default_value' => variable_get('tpps_doi_lookup_show_report_links'),
    '#description' => t('When enabled list of DOI-related reports at '
      . 'live and dev-sites will be shown under DOI input field.'),
  ];
  $form['ui']['tpps_doi_lookup_show_suggestions'] = [
    '#type' => 'checkbox',
    '#title' => t('Suggest DOI'),
    '#default_value' => variable_get('tpps_doi_lookup_show_suggestions'),
    '#description' => t('When enabled list of DOI will be shown under DOI '
      . 'input field. Click on this DOI will insert it into field and '
      . 'starts processing'),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // API.
  $form['api'] = [
    '#type' => 'fieldset',
    '#title' => t('API'),
    '#description' => t('APIs which provide DOI Publication data.'),
  ];
  $form['api']['tpps_doi_lookup_allowed_api_list'] = [
    '#type' => 'checkboxes',
    '#title' => t('Use API to get DOI publication data'),
    '#default_value' => variable_get('tpps_doi_lookup_allowed_api_list'),
    // Note:: class name => class name.
    '#options' => array_combine(
      PublicationDOI::getApiList(),
      PublicationDOI::getApiList()
    ),
    '#description' => t("Some services doesn't have records for DOI, some "
      . "have not full data so more APIs is better but it will take more time. "
      . "<br />Note: API requests are still faster than parsing SERP. "
    ),
  ];
  $form['api']['tpps_doi_lookup_force_api'] = [
    '#type' => 'checkbox',
    '#title' => t('Force API requests'),
    '#default_value' => variable_get('tpps_doi_lookup_force_api'),
    '#description' => t('Default: unchecked.'
    . '<br /> When checked makes request to API to get missing data even if '
    . 'DOI Publication data is complete from other sources. Since API requests '
    . "usually processed very quickly it's safe enough to have it checked."
    . 'Uncheck to avoid exceeding the API request limit.'),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // SERP Provider's settings.
  $form['serp'] = [
    '#type' => 'fieldset',
    '#title' => t('SERP Providers'),
    '#description' => t('Online databases which shows DOI Publication data '
    . ' at Search Engine Result Pages.'),
  ];
  // Classes:
  // - /src/GoogleScholar.class.inc,
  // - /src/PubMed.class.inc.
  // DB tables to save SERP data:
  // - 'tpps_google_scholar',
  // - 'tpps_pubmed'.
  $serp_provider_list = ['GoogleScholar', 'PubMed'];
  foreach ($serp_provider_list as $class_name) {
    $name = constant($class_name . '::KEY');
    $human_name = ucwords(str_replace('_', ' ', $name));
    $params = ['@serp_provider' => $human_name];
    $form['serp'][$name] = [
      '#type' => 'fieldset',
      '#title' => t('@serp_provider', $params),
      '#collapsible' => TRUE,
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      'info' => [
        '#markup' => theme('item_list', [
          'items' => ['DB table: public.tpps_' . $name],
        ]),
      ],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // 'tpps_doi_google_scholar_scrape_allowed',
      // 'tpps_doi_pubmed_scrape_allowed',
      // 'tpps_doi_dryad_scrape_allowed',
      'tpps_doi_' . $name . '_scrape_allowed' => [
        '#type' => 'checkbox',
        '#title' => t('Allow scraping'),
        '#default_value' => variable_get('tpps_doi_' . $name . '_scrape_allowed'),
        '#description' => t('When enabled search page results will be '
          . 'used to scrape data.'),
      ],
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      'tpps_doi_' . $name . '_scrape_container' => [
        '#type' => 'container',
        '#states' => [
          'visible' => [
            ':input[name="tpps_doi_' . $name . '_scrape_allowed"]' => ['checked' => TRUE],
          ],
        ],
        // 'tpps_doi_google_scholar_unique_url',
        // 'tpps_doi_pubmed_unique_url',
        // 'tpps_doi_dryad_unique_url',
        'tpps_doi_' . $name . '_unique_url' => [
          '#type' => 'checkbox',
          '#title' => t('Make URL unique'),
          '#default_value' => variable_get('tpps_doi_' . $name . '_unique_url'),
          '#description' => t('Default is unchecked. When checked then '
            . 'unique parameter will be added to the Endpoint URL to make '
            . 'it unique and disable caching.'),
        ],
        // 'tpps_doi_google_scholar_serp_path',
        // 'tpps_doi_pubmed_serp_path',
        // 'tpps_doi_dryad_serp_path',
        'tpps_doi_' . $name . '_serp_path' => [
          '#type' => 'textfield',
          '#title' => t('Relative path to the @serp_provider SERP ', $params),
          '#default_value' => variable_get('tpps_doi_' . $name . '_serp_path'),
          '#description' => t('This path must be allowed by CORS Anywhere '
            . 'Proxy server and will be used to build Endpoint URL.'
          )
          . theme('item_list', ['items' => [constant($class_name . '::SERP_PATH')]]),
        ],
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // @TODO Use this delay for AJAX-requests from client to backend.
        'tpps_doi_' . $name . '_delay' => [
          '#type' => 'textfield',
          '#title' => t('Mininal delay between requests (in seconds)'),
          '#default_value' => variable_get(
            'tpps_doi_' . $name . '_delay',
            constant($class_name . '::DEFAULT_DELAY')
          ),
          '#description' => t('Introduce random delays (e.g., 2-10 seconds) '
            . 'between requests to avoid rate-limiting.'),
        ],
        // 'tpps_doi_google_scholar_not_found_token',
        // 'tpps_doi_pubmed_not_found_token',
        // 'tpps_doi_dryad_not_found_token',
        'tpps_doi_' . $name . '_not_found_token' => [
          '#type' => 'textfield',
          '#title' => t('Not found Token'),
          '#default_value' => variable_get(
            'tpps_doi_' . $name . '_not_found_token',
            constant($class_name . '::NOT_FOUND_TOKEN')
          ),
          '#description' => t('String which appears on page when publication '
            . "data not found and page shouldn't be processed."
          ),
        ],
      ],
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // DOI Lookup / Dryad.
  $form['dryad'] = [
    '#type' => 'fieldset',
    '#title' => t('Dryad'),
    '#collapsible' => TRUE,
    '#description' => t('Dryad service has an API.'),
  ];
  $form['dryad']['tpps_doi_dryad_delay'] = [
    '#type' => 'textfield',
    '#title' => t('Mininal delay between requests (in seconds)'),
    '#default_value' => variable_get('tpps_doi_dryad_delay'),
    '#description' => t('Introduce random delays (e.g., 1-5 seconds) '
      . 'between requests to avoid rate-limiting.'),
  ];
  $form['dryad']['tpps_doi_dryad_endpoint_path'] = [
    '#type' => 'textfield',
    '#title' => t('Relative Path to the API Endpoint'),
    '#default_value' => variable_get('tpps_doi_dryad_endpoint_path'),
    '#description' => t('Note: no leading and trailing slashes.')
      . '<br />'
      . t('Default value: @path.', ['@path' => Dryad::ENDPOINT_PATH]),
  ];
  $form['dryad']['tpps_doi_dryad_unique_url'] = [
    '#type' => 'checkbox',
    '#title' => t('Make URL unique'),
    '#default_value' => variable_get('tpps_doi_dryad_unique_url'),
    '#description' => t('Default is unchecked. When checked then '
      . 'unique parameter will be added to the Endpoint URL to make '
      . 'it unique and disable caching.'),
  ];


  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // DOI / Gemini.
  $form['gemini'] = [
    '#type' => 'fieldset',
    '#title' => t('Gemini'),
    '#collapsible' => TRUE,
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
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $form['help'] = [
    '#type' => 'fieldset',
    '#title' => t('Help'),
  ];
  $form['help']['info'] = [
    '#markup' => theme('item_list', [
      'title' => t('Useful links'),
      'items' => [
        l(t('Report "DOI Duplicates"'), 'admin/reports/tpps/doi/duplicates'),
        l(t('Report "Missing DOI"'), 'admin/reports/tpps/doi/missing'),
        l(t('Report "DOI Duplicates"'), 'admin/reports/tpps/doi/duplicates'),
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
      ],
    ]),
  ];

  return system_settings_form($form);
}
