<?php

/**
 * @file
 * TPPS DOI Lookup Form.
 */

// @TODO Update 'source' column in DB table to use DoiSerp::KEY.
//
//
//
/**
 * TPPS DOI Lookup Form.
 */
function tpps_doi_lookup_form(array $form, array &$form_state, $doi1 = '', $doi2 = '') {
  $doi = '';
  if (!empty($doi2)) {
    $doi = $doi1 . '/' . $doi2;
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // DOI Field.
  $description = '';
  // Build list of DOI-related reports.
  if (variable_get('tpps_doi_lookup_show_report_links', FALSE)) {
    $description .= theme('item_list', [
      'items' => [
        l('https://treegenesdb.org/admin/reports/tpps/doi/duplicates',
          'https://treegenesdb.org/admin/reports/tpps/doi/duplicates'),
        l('https://tgwebdev.cam.uchc.edu/admin/reports/tpps/doi/submission/all',
          'https://tgwebdev.cam.uchc.edu/admin/reports/tpps/doi/submission/all'
        ),
      ],
    ]);
  }
  // Build list of suggestions.
  if (variable_get('tpps_doi_lookup_show_suggestions', TRUE)) {
    $options = ['attributes' => ['class' => ['tpps-suggestion']]];
    $doi_list = db_select(PublicationDOI::TABLE, 't')
      ->fields('t', ['doi'])
      ->groupBy('doi')
      ->execute()
      ->fetchCol('doi');
    foreach ($doi_list as $value) {
      $suggestions[] = l($value, '', $options);
    }
    $description .= 'Parsed DOI:' . theme('item_list', ['items' => $suggestions]);
  }
  // Create field.
  $form['tpps_doi_lookup_doi'] = [
    '#type' => 'textfield',
    '#title' => t('DOI'),
    '#default_value' => $doi,
    '#description' => $description,
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // JS Settings.
  $proxy = variable_get('tpps_doi_lookup_proxy');
  $js_settings = [
    'doi_lookup' => [
      // Selector for the textfield with DOI.
      'field' => '#edit-tpps-doi-lookup-doi',
      // Selector for the iframe which holds Google Scholar SERP.
      'proxy' => $proxy,
      // List of the SERP Provider KEYs.
      'ajax_get_publication_data_path' => '/ajax/tpps/get_publication_data',
      'publication_data' => [],
    ],
  ];
  // Check if any SERP providers are enabled.
  // List of existing providers.
  $serp_provider_class_list = ['GoogleScholar', 'PubMed'];

  // List of enabled providers.
  $serp_providers = [];
  foreach ($serp_provider_class_list as $class_name) {
    $name = constant($class_name . '::KEY');
    $provider = new $class_name($doi);
    $serp_providers[$name] = $provider;
    // Publication data per provider.
    if (!empty($publication_data = PublicationDOI::load($doi, $name))) {
      $js_settings['doi_lookup']['publication_data'][$name][$doi]
        = $publication_data;
    }
    // List of existing SERP Providers.
    $js_settings['doi_lookup']['serp_providers'][] = $name;
    // Metadata of the providers allowed for scraping.
    $js_settings['doi_lookup'][$name] = [
      'scrape_allowed' => $provider->scrapeAllowed,
      'iframe' => 'iframe#' . $name,
      'dump_container' => '#' . $name . '_dump_container',
      'endpoint' => $provider->getEndpointURL(),
      'query' => $provider->getQuery(),
      'query_param' => constant($provider->getClassName() . '::QUERY_PARAM'),
      // Delay between requests of the same client.
      'delay' => $provider->delay,
      'ajax_save_path' => '/ajax/tpps/save_' . $name . '_serp',
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $url = '';
    if (!empty($doi) && empty($publication_data)) {
      // Note: PubMed replaces '/' in DOI with space.
      $fixed_provider_url = str_replace('%252F', '/', $provider->buildUrl());
      $url = $proxy . '/' . $fixed_provider_url;
    }
    // 'google_scholar_serp',
    // 'pubmed_serp'.
    $form[$name . '_serp'] = [
      '#type' => 'fieldset',
      '#title' => $provider->getClassName(),
      '#collapsed' => FALSE,
      '#collapsible' => TRUE,
      // 'google_scholar_dump_container',
      // 'pubmed_dump_container'.
      'data' => [
        '#markup' => '<div class="dump_container" id="' . $name
          . '_dump_container"></div>
          <iframe id="' . $name . '" class="serp" src="' . $url . '"
          target="' . $name . '">' . $url . '</iframe>',
      ],
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add JS settings.
  $form['#attached']['js'][] = [
    'type' => 'setting',
    'data' => ['tpps' => $js_settings],
    'scope' => 'footer',
    'group' => JS_THEME,
    'weight' => 5,
  ];
  // Add JS-logic.
  tpps_add_css_js('doi_lookup', $form);

  tpps_add_css_js('suggestion', $form);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TppsForm::autofocus($form, ['tpps_doi_lookup_doi']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('main', $form);

  return $form;
}
