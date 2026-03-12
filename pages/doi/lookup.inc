<?php

/**
 * @file
 * TPPS DOI Lookup Form.
 */

//{
//  "responseMimeType": "application/json",
//  "responseJsonSchema": {
//    "type": "object",
//    "properties": {
//      "title": { "type": "string" },
//      "journal": { "type": "string" },
//      "authors": { "type": "string" },
//      "date": { "type": "string" },
//      "abstract": { "type": "string" }
//    },
//    "required": ["title", "authors", "date", "abstract", "journal"]
//  }
//}


/**
 * TPPS DOI Lookup Form.
 */
function tpps_doi_lookup_form(array $form, array &$form_state, $doi1 = '', $doi2 = '') {
  if (!empty($doi2)) {
    $doi = $doi1 . '/' . $doi2;
  }
  else {
    $doi = variable_get('tpps_doi_lookup_doi');
  }

  $gs = new GoogleScholar($doi);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add JS.
  $js_settings = [
    'doi_lookup' => [
      // Selector for the textfield with DOI.
      'field' => '#edit-tpps-doi-lookup-doi',
      // Selector for the iframe which holds Google Scholar SERP.
      'iframe' => 'iframe#google-scholar',
      'doi' => $doi,
      'proxy' => variable_get('tpps_doi_lookup_proxy'),
      'endpoint' => $gs->getEndpointURL(),
      'query' => $gs->getQuery(),
      // Delay between requests of the same client.
      'delay' => $gs->delay,
      'ajax_path' => '/ajax/tpps/get_publication_data',

// @TODO Remove debug code.
      // Makes proxy URL broken to avoid Google Scholar ban.
      'debug' => 'debug/',
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
  $form['tpps_doi_lookup_doi'] = [
    '#type' => 'textfield',
    '#title' => t('DOI'),
    '#default_value' => $doi,
  ];
  $form['tpps_doi_lookup_result'] = [
    '#markup' => '<div style="border: 1px solid white; display: none;" id="dumpContainer"></div>',
  ];

// @TODo show iframe if DOI is not empty.


  // @TODO Check if publication data already loaded in $gs.

  // @TODO If there is a SERP for this DOI it will be loaded.
  // Check PublicationDOI::TABLE for the data first.

  $url = $gs->buildUrl();

  // @todo Check if $url and $proxy exists.
  //$proxy = variable_get('tpps_doi_lookup_proxy');
  //$url = $proxy . $gs->buildUrl();

  //$url = 'https://tgwebdev.cam.uchc.edu/corsanywhere/https://142.250.120.104/scholar?hl=en&as_sdt=0%2C7&q=ok&btnG=';

// @TODO Remove debug code.

  $url = 'https://tgwebdev.cam.uchc.edu/devel/php';

  // @todo Hide iframe.
  $form['google_scholar_serp'] = [
    '#markup' => '<iframe  style="display: none;"
      src="' . $url . '" width="500px" height="300px"
      name="google_scholar" target="google_scholar"
      id="google-scholar">' . $url . '</iframe>',
  ];

  TppsForm::autofocus($form, ['tpps_doi_lookup_question']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);

  // We only store DOI into variable. Other work must be done using AJAX.
  return system_settings_form($form);
}
