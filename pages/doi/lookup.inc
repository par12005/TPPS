<?php

/**
 * @file
 * TPPS DOI Lookup Form.
 */

/**
 * TPPS DOI Lookup Form.
 */
function tpps_doi_lookup_form(array $form, array &$form_state, $doi1 = '', $doi2 = '') {
  $doi = '';
  if (!empty($doi2)) {
    $doi = $doi1 . '/' . $doi2;
  }

  $serp_providers = [
    PubMed::KEY => new PubMed($doi),
    GoogleScholar::KEY => new GoogleScholar($doi),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add JS settings.
  $js_settings = [
    'doi_lookup' => [
      // Selector for the textfield with DOI.
      'field' => '#edit-tpps-doi-lookup-doi',
      // Selector for the iframe which holds Google Scholar SERP.
      // 'doi' => $doi,
      'proxy' => variable_get('tpps_doi_lookup_proxy'),
      'serp_providers' => array_keys($serp_providers),
      'ajax_get_publication_data_path' => '/ajax/tpps/get_publication_data',
    ],
  ];
  foreach ($serp_providers as $name => $provider) {

// @TODO Update 'source' column in DB table to use DoiSerp::KEY.

    $publication_data[$name] = PublicationDOI::load($doi, $name);
    $js_settings['doi_lookup'][$name] = [
      'iframe' => 'iframe#' . $name,
      'dump_container' => '#' . $name . '_dump_container',
      'endpoint' => $provider->getEndpointURL(),
      'query' => $provider->getQuery(),
      'query_param' => constant($provider->getClassName() . '::QUERY_PARAM'),
      // Delay between requests of the same client.
      'delay' => $provider->delay,
      'ajax_save_path' => '/ajax/tpps/save_' . $name . '_serp',
      'publication_data' => [$doi => $publication_data],
    ];
  }
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
  // Build list of suggestions.
  $doi_list = db_select(PublicationDOI::TABLE, 't')
    ->fields('t', ['doi'])
    ->groupBy('doi')
    ->execute()
    ->fetchCol('doi');
  $options = ['attributes' => ['class' => ['tpps-suggestion']]];
  foreach ($doi_list as $value) {
    $items[] = l($value, '', $options);
  }
  tpps_add_css_js('suggestion', $form);

  // DOI Field.
  $description = '';
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
  if (variable_get('tpps_doi_lookup_show_suggestions', TRUE)) {
    $description .= 'Parsed DOI:' . theme('item_list', ['items' => $items]);
  }
  $form['tpps_doi_lookup_doi'] = [
    '#type' => 'textfield',
    '#title' => t('DOI'),
    '#default_value' => $doi,
    '#description' => $description,
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $url_list = [];
  if (!empty($doi) && empty($publication_data)) {
    $proxy = variable_get('tpps_doi_lookup_proxy');
    foreach ($serp_providers as $name => $provider) {
      if ($provider->scrapeAllowed) {
        // Note: PubMed replaces '/' in DOI with space.
        $fixed_provider_url = str_replace('%252F', '/', $provider->buildUrl());
        $url_list[$name] = $proxy . '/' . $fixed_provider_url;
      }
    }
  }
  foreach ($serp_providers as $name => $provider) {
    if (!$provider->scrapeAllowed) {
      continue;
    }
    $url = $url_list[$name] ?? '';
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
  TppsForm::autofocus($form, ['tpps_doi_lookup_doi']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('main', $form);

  return $form;
}
