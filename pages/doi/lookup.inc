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
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add JS.
  $js_settings = [
    'doi_lookup' => [
      // Selector for the textfield with DOI.
      'field' => '#edit-tpps-doi-lookup-doi',
      // Selector for the iframe which holds Google Scholar SERP.
      'iframe' => 'iframe#google-scholar',
      // 'doi' => $doi,
      'proxy' => variable_get('tpps_doi_lookup_proxy'),
      'endpoint' => $gs->getEndpointURL(),
      'query' => $gs->getQuery(),
      // Delay between requests of the same client.
      'delay' => $gs->delay,
      'ajax_get_path' => '/ajax/tpps/get_publication_data',
      'ajax_save_path' => '/ajax/tpps/save_google_scholar_serp',
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
  // @TODO Move to CSS.
  $form['tpps_doi_lookup_result'] = [
    '#markup' => '<div id="dumpContainer"></div>',
  ];

  // @TODO If there is a SERP for this DOI it will be loaded.
  // Check PublicationDOI::TABLE for the data first.

  $debug_mode = 0;
  $show_iframe = FALSE;
  if (empty($doi)) {
    $url = '';
  }
  else {
    $url = '';
    if (empty($publication_data)) {
      $url = variable_get('tpps_doi_lookup_proxy')
        . '/' . str_replace('%252F', '/', $gs->buildUrl());
    }
  }

  $form['google_scholar_serp'] = [
    '#markup' => '<iframe id="google-scholar" target="google_scholar"
     src="' . $url . '">' . $url . '</iframe>',
  ];

  TppsForm::autofocus($form, ['tpps_doi_lookup_doi']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);

  return $form;
  // We only store DOI into variable. Other work must be done using AJAX.
  return system_settings_form($form);
}
