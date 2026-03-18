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
  $publication_data = PublicationDOI::load($doi);

  $gs = new GoogleScholar($doi);
  $pubmed = new PubMed($doi);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add JS.
  $js_settings = [
    'doi_lookup' => [
      // Selector for the textfield with DOI.
      'field' => '#edit-tpps-doi-lookup-doi',
      // Selector for the iframe which holds Google Scholar SERP.
      // 'doi' => $doi,
      'proxy' => variable_get('tpps_doi_lookup_proxy'),
      'google_scholar' => [
        'iframe' => 'iframe#google-scholar',
        'endpoint' => $gs->getEndpointURL(),
        'query' => $gs->getQuery(),
        // Delay between requests of the same client.
        'delay' => $gs->delay,
        'ajax_save_path' => '/ajax/tpps/save_google_scholar_serp',
      ],
      'pubmed' => [
        'iframe' => 'iframe#pubmed',
        'endpoint' => $pubmed->getEndpointURL(),
        'query' => $pubmed->getQuery(),
        // Delay between requests of the same client.
        'delay' => $pubmed->delay,
        'ajax_save_path' => '/ajax/tpps/save_pubmed_serp',
      ],
      'ajax_get_path' => '/ajax/tpps/get_publication_data',
      'publication_data' => [$doi => $publication_data],

// @TODO Remove debug code.
      // Makes proxy URL broken to avoid Google Scholar ban.
      //'debug' => 'debug',
// End of the debug code.
    ],
  ];
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
    ->execute()
    ->fetchCol('doi');
  $options = ['attributes' => ['class' => ['tpps-suggestion']]];
  foreach ($doi_list as $value) {
    $items[] = l($value, '', $options);
  }
  tpps_add_css_js('suggestion', $form);

  // DOI Field.
  $form['tpps_doi_lookup_doi'] = [
    '#type' => 'textfield',
    '#title' => t('DOI'),
    '#default_value' => $doi,
    '#description' => 'Parsed DOI:' . theme('item_list', ['items' => $items]),
  ];

  // @todo Move to CSS.
  // @todo If there is a SERP for this DOI it will be loaded.
  // Check PublicationDOI::TABLE for the data first.

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $gs_url = '';
  $pubmed_url = '';
  if (!empty($doi) && empty($publication_data)) {
    $gs_url = variable_get('tpps_doi_lookup_proxy')
      . '/' . str_replace('%252F', '/', $gs->buildUrl());
    $pubmed_url = variable_get('tpps_doi_lookup_proxy')
      . '/' . str_replace('%252F', ' ', $pubmed->buildUrl());
  }
  // @todo Add setting to allow/block serp provider.
  $form['google_scholar_serp'] = [
    '#markup' => '<div id="google_scholar_dump_container"></div>
      <iframe id="google-scholar" target="google_scholar"
        src="' . $gs_url . '">' . $gs_url . '</iframe>',
  ];
  $form['pubmed_serp'] = [
    '#markup' => '<div id="pubmed_dump_container"></div>
      <iframe id="pubmed" target="pubmed"
        src="' . $pubmed_url . '">' . $pubmed_url . '</iframe>',
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TppsForm::autofocus($form, ['tpps_doi_lookup_doi']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);

  return $form;
}
