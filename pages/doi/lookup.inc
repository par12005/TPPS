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

  $form['tpps_doi_lookup_doi'] = [
    '#type' => 'textfield',
    '#title' => t('DOI'),
    '#default_value' => $doi,
  ];

// @TODo show iframe if DOI is not empty.

  $gs = new GoogleScholar($doi);

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
    '#markup' => '<iframe src="' . $url . '" width="500px" height="300px"
      name="google_scholar" target="google_scholar"
      id="google-scholar">' . $url . '</iframe>',
  ];

  $form['tpps_doi_lookup_system_instruction'] = [
    '#type' => 'textarea',
    '#title' => t('System Instruction'),
    '#description' => t('Format: plain text. You can guide the behavior of '
      . 'Gemini models with system instructions.') . '<br />'
      . l(t('Gemini: System instructions and other configurations'),
        'https://ai.google.dev/gemini-api/docs/text-generation#system-instructions'
    ),
    '#default_value' => variable_get('tpps_doi_lookup_system_instruction', ''),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Add JS.
  $js_settings = [
    'doi_lookup' => [
      'field' => '#edit-tpps-doi-lookup-doi',
      'doi' => $doi,
      'proxy' => variable_get('tpps_doi_lookup_proxy'),
      'endpoint' => $gs->getEndpointURL(),
      'query' => $gs->getQuery(),
      // Delay between requests of the same client.
      'delay' => $gs->delay,
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
  $form['info'] = [
    '#markup' => theme('item_list', [
      'title' => t('Useful links'),
      'items' => [
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
      ]
    ]),
  ];

  TppsForm::autofocus($form, ['tpps_doi_lookup_question']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);

  // We only store DOI into variable. Other work must be done using AJAX.
  return system_settings_form($form);
}
