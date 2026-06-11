<?php

/**
 * @file
 * TPPS DOI Lookup Form.
 */

// @TODO Check if 'source' column in DB table uses DoiSerp::KEY.

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
        l('Proxy Server Status', 'corsanywhere'),
        l('DOI Lookup Settings', 'admin/config/tpps/doi'),
        l('DOI Duplicates report at live site',
          'https://treegenesdb.org/admin/reports/tpps/doi/duplicates'),
        l('All DOI report at live site',
          'https://tgwebdev.cam.uchc.edu/admin/reports/tpps/doi/submission/all'
        ),
      ],
    ]);
  }
  $options = PublicationDOI::buildOptions();
  $module_exists = module_exists('select_or_other');
  if (!$module_exists) {
    // Build list of suggestions.
    if (variable_get('tpps_doi_lookup_show_suggestions')) {
      $params = ['attributes' => ['class' => ['tpps-suggestion']]];
      // Build list of other DOIs.
      foreach ($options[PublicationDOI::PD_KEY] as $value) {
        $suggestions[] = l($value, '', $params);
      }
      foreach ($options[PublicationDOI::NOPD_KEY] as $value) {
        // Not yet processed.
        $other_doi_list[] = l($value, '', $params);
      }
      $description .= theme('item_list', [
        'title' => t(PublicationDOI::PD_KEY),
        'items' => $suggestions,
      ]) . theme('item_list', [
        'title' => t(PublicationDOI::NOPD_KEY),
        'items' => $other_doi_list,
      ]);
    }
  }

  if ($module_exists) {
    // Create main DOI field.
    $form['tpps_doi_lookup_doi'] = [
      '#type' => 'select_or_other',
      '#title' => t('DOI'),
      '#default_value' => $doi,
      '#description' => $description,
      '#options' => $options,
      '#attributes' => ['class' => ['tpps_doi_lookup_doi']],
    ];
  }
  else {
    // Create main DOI field.
    $form['tpps_doi_lookup_doi'] = [
      '#type' => 'textfield',
      '#title' => t('DOI'),
      '#default_value' => $doi,
      '#description' => $description,
      '#attributes' => ['class' => ['tpps_doi_lookup_doi']],
    ];
    tpps_add_css_js('suggestion', $form);
  }

  // Container to show complete combined publication data.
  $form['publication_data'] = [
    '#markup' => '<div class="dump_container" id="dump_container"></div>',
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // JS Settings.
  $proxy = variable_get('tpps_doi_lookup_proxy');
  $js_settings = [
    'doi_lookup' => [
      // Selector for the textfield with DOI.
      'field' => '.tpps_doi_lookup_doi',
      'proxy' => $proxy,
      // List of the SERP Provider KEYs.
      'ajax_get_publication_data_path' => '/ajax/tpps/getPublicationData',
      'ajax_callback' => 'getPublicationData',
      'publication_data' => [],
      'dump_container' => '#dump_container',
    ],
  ];
  // Check if any SERP providers are enabled.
  // List of existing providers.
  $serp_provider_class_list = ['GoogleScholar', 'PubMed'];

  // List of enabled providers.
  $serp_providers = [];
  foreach ($serp_provider_class_list as $class_name) {
    $provider = new $class_name($doi);
    $serp_providers[$class_name] = $provider;
    // Publication data per provider.
    if (!empty($publication_data = PublicationDOI::load($doi, $class_name))) {
      $js_settings['doi_lookup']['publication_data'][$class_name][$doi]
        = $publication_data;
    }
    // List of existing SERP Providers.
    $js_settings['doi_lookup']['serp_providers'][] = $class_name;
    // Metadata of the providers allowed for scraping.
    $js_settings['doi_lookup'][$class_name] = [
      'scrape_allowed' => $provider->scrapeAllowed,
      'dump_container' => '#' . $class_name . '_dump_container',
      'endpoint' => $provider->getEndpointUrl(),
      'query' => $provider->getQuery(),
      'query_param' => constant($provider->getClassName() . '::QUERY_PARAM'),
      // Delay between requests of the same client.
      'delay' => $provider->delay,
      // Note: path doesn't mean anything. More important is 'ajax_callback'.
      'ajax_save_path' => '/ajax/tpps/saveSerp',
      'ajax_callback' => 'saveSerp',
      'not_found_token' => $provider->getNotFoundString(),
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $url = '';
    if (!empty($doi) && empty($publication_data)) {
      // Note: PubMed replaces '/' in DOI with space.
      $fixed_provider_url = str_replace('%252F', '/', $provider->buildUrl());
      $url = $proxy . '/' . $fixed_provider_url;
    }
    // 'GoogleScholar_serp',
    // 'PubMed_serp'.
    $form[$class_name . '_serp'] = [
      '#type' => 'fieldset',
      '#title' => $provider->getClassName(),
      '#collapsed' => FALSE,
      '#collapsible' => TRUE,
      // '#GoogleScholar_dump_container',
      // '#PubMed_dump_container'.
      'data' => [
        '#markup' => '<div class="dump_container" id="' . $class_name . '_dump_container"></div>',
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TppsForm::autofocus($form, ['tpps_doi_lookup_doi']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('main', $form);

  return $form;
}
