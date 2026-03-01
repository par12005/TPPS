<?php

/**
 * @file
 * TPPS Submission Clone Form.
 *
 * @TODO Minor. Implement ability to set accession number. Currently there is
 * no ability to specify accession. Only next one could be used
 * or existing one overritten.
 * @TODO Minor. Set Submission::STATUS_APPROVED status and do necessary changes.
 * Just setting Submission::STATUS_APPROVED status don't allow to run Tripal
 * Job just after import. Required more work with code.
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

module_load_include('inc', 'tpps', 'includes/imported_studies_list');
module_load_include('inc', 'tpps', 'pages/submission/import');

// @TOdO Set status 'Approved'.

/**
 * TPPS Submission Import Form.
 */
function tpps_doi_lookup_form(array $form, array &$form_state, $doi1 = '', $doi2 = '') {
  if (!empty($doi2)) {
    $doi = $doi1 . '/' . $doi2;
  }
  else {
    $doi = variable_get('tpps_doi_lookup_question');
  }

  $form['tpps_doi_lookup_question'] = [
    '#type' => 'textarea',
    '#title' => t('Question'),
    '#default_value' => $doi,
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
  $form['tpps_doi_lookup_generation_config'] = [
    '#type' => 'textarea',
    '#title' => t('Generation Config Object'),
    '#description' => t('Format: JSON. See !link.', [
      '!link' => l(t('Gemini: GenerationConfig'),
        'https://ai.google.dev/api/generate-content#v1beta.GenerationConfig'
      )
    ]),
    '#default_value' => variable_get('tpps_doi_lookup_generation_config', ''),
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
  if (empty($values['tpps_doi_lookup_question'])) {
    return;
  }

  $gemini = new Gemini();
  if (!empty($values['tpps_doi_lookup_system_instruction'])) {
    $gemini->systemInstruction = $values['tpps_doi_lookup_system_instruction'];
  }
  //if (!empty($values['tpps_doi_lookup_generation_config'])) {
  //  $gemini->generationConfig = $values['tpps_doi_lookup_generation_config'];
  //}
  //else {
    $gemini->generationConfig = [
      "responseMimeType" => "application/json",
      "responseJsonSchema" => [
        "type" => "object",
        "properties" => [
          "title" => (object) ["type" => "string"],
          "journal" => (object) ["type" => "string"],
          "authors" => (object) ["type" => "string"],
          "date" => (object) ["type" => "string"],
          "abstract" => (object) ["type" => "string"],
        ],
        "required" => ["title", "authors", "date", "abstract", "journal"],
      ],
    ];
  //}


  // Note:
  // Disabled because requiests failed and doesn't actually required.
  //$gemini->tools = [["google_search" => (object) []]];
  $response = $gemini->ask(
    $values['tpps_doi_lookup_question']
  );
  dpm(var_export($response, 1));
}
