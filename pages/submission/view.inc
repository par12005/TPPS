<?php

/**
 * @file
 * TPPS Submission View.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. Shows a single submission data.
 */
function tpps_submission_view_form(array $form, array $form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);

  if (empty($state = tpps_submission_get_state($accession))) {
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
  module_load_include('inc', 'tpps', 'includes/submissions');
  $import_meta = tpps_submission_get_import_meta($state);
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
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Show the data!
  $interface = tpps_submission_interface_load($accession, RESET_CACHE) ?? [];

  dpm(
    'Real Interface Version: ' .
    (isset($interface['saved_values'][1]['publication']['publication_doi']) ? 2 : 1)
  );
  dpm(
    'Real State Version: ' .
    (isset($state['saved_values'][1]['publication']['publication_doi']) ? 2 : 1)
  );

  $dumps = [
    'Submission Interface' => tpps_array_dump($interface),
    'Form State' => tpps_array_dump($state),
  ];
  foreach ($dumps as $type => $data) {
    $form[$type] = [
      '#type' => 'fieldset',
      '#title' => t(ucfirst($type)),
      '#collapsed' => FALSE,
      '#collapsible' => TRUE,
    ];
    foreach ($data as $key => $item) {
      $form[$type][$key . '_container'] = [
        '#type' => 'fieldset',
        '#title' => ($item['title'] ?? $key)
          . ($item['count'] ? ' (' . $item['count'] . ')' : ''),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        'plain_items' => ['#markup' => ($item['data'] ?? NULL)],
      ];
    }
  }

  //tpps_submission_list_add_buttons($form);
  // Autofocus disabled because it's more important to scroll then compare.
  // tpps_form_autofocus($form, 'tpps_submission_accession_2');
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
