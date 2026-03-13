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
  else {
    //$doi = variable_get('tpps_doi_lookup_doi');
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
  $doi_list = [
    '10.1002/ajb2.1504',
    '10.1186/s12870-023-04428-z',
    '10.1186/s12862-016-0810-1',
    '10.1038/s41437-024-00684-3',
    '10.5061/dryad.269126c',
    '10.3732/apps.1300025',
    '10.1038/sdata.2015.6',
    '10.5061/dryad.1mt1h',
    '10.1002/ece3.4216',
  ];
  $options = ['attributes' => ['class' => ['tpps-suggestion']]];
  foreach ($doi_list as $value) {
    $items[] = l($value, '', $options);
  }
  $form['tpps_doi_lookup_doi'] = [
    '#type' => 'textfield',
    '#title' => t('DOI'),
    '#default_value' => $doi,
    '#description' => 'Examples:' . theme('item_list', ['items' => $items]),
  ];

  tpps_add_css_js('suggestion', $form);

  $form['tpps_doi_lookup_result'] = [
    '#markup' => '<div style="border: 1px solid white; display: none;" id="dumpContainer"></div>',
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
    '#markup' => '<iframe style="'
      . ($show_iframe ? 'margin: 30px 0 0 0;' : 'display: none')
      . '" src="' . $url . '" width="100%" height="300px"
      name="google_scholar" target="google_scholar"
      id="google-scholar">' . $url . '</iframe>',
  ];

  TppsForm::autofocus($form, ['tpps_doi_lookup_doi']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);

  return $form;
  // We only store DOI into variable. Other work must be done using AJAX.
  return system_settings_form($form);
}
