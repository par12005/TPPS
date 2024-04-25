<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. Shows a single submission data.
 */
function tpps_submission_view_form(array $form, array $form_state, $accession = '', $type = '') {
  tpps_submission_list_validate_accession($accession);
  $submission = new Submission($accession);

  if ($submission->doesNotExist()) {
    drupal_set_message(t('Submission "<strong>@accession</strong>" not found. '
      . 'Please choose from the list below.',
      ['@accession' => $accession]), 'error'
    );
    drupal_goto('tpps/submission');
  }

  $form['tpps_submission_current_accession'] = [
    '#markup' => '<h3>' . t('Current Accession: @accession',
    ['@accession' => $accession]) . '</h3>',
  ];
  $form['tpps_submission_accession_1'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Import data.
  //module_load_include('inc', 'tpps', 'includes/submissions');
  $import_meta = $submission->getImportMeta();
  if (!empty($import_meta)) {
    tpps_form_add_report($form, [
      'import_metadata' => [
        'title' => t('Import Metadata'),
        'data' => theme('item_list', [
          'items' => [
            t('Remote Accession: !url', ['!url' => $import_meta['accession_link']]),
            t('Import Date: @date.', ['@date' => $import_meta['full_date']]),
            t('Site !site.',
              ['!site' => l($import_meta['site'], $import_meta['site'])]
            ),
            t('Submission Form Version: @version.',
            ['@version' => $import_meta[TPPS_SUBMISSION_FORM_VERSION_KEY]]),
          ],
        ]),
      ],
    ]);
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (empty($type)) {
    $form['tpps_submission_accession_2'] = [
      '#type' => 'select',
      '#title' => t('Accession to compare with'),
      '#options' => tpps_submission_get_accession_list(),
    ];
    $form['actions'] = ['#type' => 'container'];
    $form['actions']['compare'] = [
      '#type' => 'submit',
      '#value' => t('Compare'),
      '#submit' => ['tpps_submission_view_form_submit'],
    ];
    drupal_set_message('Real Shared State Version: ' .
      (isset($submission->sharedState['saved_values'][1]['publication']['publication_doi']) ? 2 : 1)
    );
    drupal_set_message('Real Form State Version: ' .
      (isset($submission->state['saved_values'][1]['publication']['publication_doi']) ? 2 : 1)
    );
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show the data!

  if ($type == 'state') {
    $dumps = ['state'];
  }
  elseif (in_array($type, ['shared_state', 'sharedState'])) {
    $dumps = ['sharedState'];
  }
  else {
    $dumps = ['sharedState', 'state'];
  }

  foreach ($dumps as $data_type) {
    // Huge: 5M, Normal: 25K.
    $max_len = variable_get('tpps_dump_max_len_' . $data_type, 100000);
    $state = $submission->$data_type;
    // Remove huge element which added by Tripal Job.
    if (!empty($state['data'])) {
      drupal_set_message('Huge element "data" was cut off.');
      unset($state['data']);
    }
    // State and Shared State fieldsets.
    $form[$data_type] = [
      '#type' => 'fieldset',
      '#title' => t(ucfirst($data_type)),
      '#collapsed' => FALSE,
    ];

    $data_len = strlen(json_encode($state));
    if ($data_len > $max_len) {
      drupal_set_message(t('@data_type length: $len.',
        ['@data_type' => $data_type, '@len' => $data_len])
      );
      $form[$data_type]['container'] = [
        // Need to set manually text color because theme 'dawn' set
        // grey for PRE elements.
        '#markup' => '<pre style="color: white;">'
          . var_export($state, 1) . '</pre>'
      ];
    }
    else {
      $data = tpps_array_dump($state);
      foreach ($data as $key => $item) {
        $form[$data_type][$key . '_container'] = [
          '#type' => 'fieldset',
          '#title' => ($item['title'] ?? $key)
            . ($item['count'] ? ' (' . $item['count'] . ')' : ''),
          '#collapsible' => TRUE,
          // 'saved_values' item always open.
          '#collapsed' => ($key == 'saved_values') ? FALSE : TRUE,
          'plain_items' => ['#markup' => ($item['data'] ?? NULL)],
        ];
      }
    }
  }

  //tpps_submission_list_add_buttons($form);
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_view_form_submit(array $form, array &$form_state) {
  $button = $form_state['triggering_element']['#value'];
  $accession_1 = ($form_state['values']['tpps_submission_accession_1'] ?? '');
  if ($button == t('Compare')) {
    $accession_2 = ($form_state['values']['tpps_submission_accession_2'] ?? '');
    $path = 'tpps/submission/' . $accession_1 . '/compare/' . $accession_2;
    drupal_goto($path);
  }
}
