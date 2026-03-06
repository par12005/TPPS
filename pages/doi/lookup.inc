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
  $url = $gs->buildUrl();

  // @todo Check if $url and $proxy exists.
  $proxy = variable_get('tpps_doi_lookup_proxy');
  $url = $proxy . $gs->buildUrl();

  dpm($url);

  // @todo Hide iframe.
  // @todo Get it dynamically when DOI field updated.
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

  $form['#submit'][] = 'tpps_doi_lookup_form_submit';
  TppsForm::autofocus($form, ['tpps_doi_lookup_question']);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);

  return system_settings_form($form);
}

/**
 * Submission Clone Form submitter.
 */
function tpps_doi_lookup_form_submit(array $form, array &$form_state) {
  //dpm(print_r($form_state, 1));

  $values = &$form_state['values'];
  if (empty($values['tpps_doi_lookup_doi'])) {
    return;
  }

  $gemini = new Gemini();
  if (!empty($values['tpps_doi_lookup_system_instruction'])) {
    $gemini->systemInstruction = $values['tpps_doi_lookup_system_instruction'];
  }


  // Answer in JSON format.
  $gemini->generationConfig = [
    "responseMimeType" => "application/json",
    "responseJsonSchema" => [
      "type" => "object",
      "properties" => [
        "doi" => (object) ["type" => "string"],
        "title" => (object) ["type" => "string"],
        "journal" => (object) ["type" => "string"],
        "date" => (object) ["type" => "string"],
        "authors" => (object) ["type" => "string"],
        "abstract" => (object) ["type" => "string"],
      ],
      "required" => ["title", "authors", "date", "abstract", "journal"],
    ],
  ];

  // @TODO Try to allow internet search to get better results.
  // Note:
  // Disabled because requiests failed and doesn't actually required.
  //$gemini->tools = [["google_search" => (object) []]];

  $gemini->ask(
    $values['tpps_doi_lookup_doi']
  );
  $fields = json_decode($gemini->answer, TRUE);

  if ($fields === NULL || $fields === FALSE) {
    // An error occurred during decoding.
    $error_code = json_last_error();
    $error_message = json_last_error_msg();
    throw new Exception("JSON decoding error: [{$error_code}] {$error_message}");
  }

  // Store to DB.
  $fields['source'] = 'gemini';
  // @TODO Remove this debug code.
  $fields['doi'] = $fields['doi']
    ?? $values['tpps_doi_lookup_doi']
    ?? '10.1002\/ece3.4216';
  // Check if record exists.
  $is_record_exists = (bool) db_select('tpps_doi', 't')
    ->fields('t', ['doi'])
    ->condition('doi', $fields['doi'])
    ->condition('source', $fields['source'])
    ->range(0, 1)
    ->execute()
    ->fetchField();

  if ($is_record_exists) {
    drupal_set_message('Record already exists. DOI: ' . $fields['doi']);
  }
  else {
    drupal_set_message('Created new record. DOI: ' . $fields['doi']);
    db_insert('tpps_doi')->fields($fields)->execute();
  }
}
