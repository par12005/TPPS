<?php

/**
 * @file
 * Removes parts of the TPPS Submission.
 */

module_load_include('inc', 'tpps', 'pages/submission/list');

/**
 * Menu callback. TPPS Submission Remove Form.
 */
function tpps_submission_remove_form(array $form, array &$form_state, $accession = '') {
  tpps_submission_list_validate_accession($accession);
  $submission = new Submission($accession);
  $message = t(
    'Are you sure you want to remove Submission @accession?',
    ['@accession' => $accession]
  );
  $cancel_redirect = 'tpps/submission';
  $description = t("Submission @accession and it's files will be removed.",
      ['@accession' => $accession]
    ) . '<br />' . l(
        t('Submistion @accession view page', ['@accession' => $accession]),
        'tpps/submission/' . $accession . '/view'
      );
  $confirm_button_text = t('Remove');
  $cancel_button_text = t('Cancel');
  // @TODO Get accession from URL (if it's safe).
  $form['tpps_submission_accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];

  $form = confirm_form(
    $form,
    $message,
    $cancel_redirect,
    $description,
    $confirm_button_text,
    $cancel_button_text
  );
  $form['#attributes']['class'][] = 'tpps-submission';
  // Removement options.
  // @TODO Check each option and show status.
  $form['options'] = [
    '#type' => 'fieldset',
    '#title' => t('Removement options'),
    '#collapsible' => TRUE,
  ];
  $remove_options = tpps_submission_get_remove_options($accession);
  foreach ($remove_options as $name => $item) {

    // @TODO Disable empty parts.
    $status = NULL;
    if (!empty($item['status_method'])) {
      if (method_exists('Submission', $item['status_method'])) {
        $status = call_user_func([$submission, $item['status_method']]);
      }
    }
    $status_message = ' [' . ($status ? '<strong>NOT empty</strong>' : 'Empty') . ']';
    $form['options'][$name] = [
      '#type' => 'checkbox',
      '#title' => $item['title'] . (is_null($status) ? '' : $status_message),
      '#default_value' => $item['default_value'],
      '#description' => ($item['description'] ?? NULL),
    ];
    if (!is_null($status)) {
      $form['options'][$name]['#default_value'] = $status;
      if ($status === FALSE) {
        $form['options'][$name]['#disabled'] = TRUE;
      }
    }
  }
  tpps_add_css_js('theme', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_remove_form_submit(array $form, array &$form_state) {
  $accession = $form_state['values']['tpps_submission_accession'] ?? NULL;
  tpps_submission_list_validate_accession($accession);
  // Get removement options.
  $options = [];
  $remove_options = tpps_submission_get_remove_options();
  foreach (array_keys($remove_options) as $name) {
    $options[$name] = (bool) $form_state['values'][$name];
  }
  // Remove study.
  tpps_submission_load($accession)->remove($options);
  drupal_goto('tpps/submission/' . $accession . '/remove');
}

/**
 * Gets list of removement options.
 *
 * @return array
 *   Returns array to build form and set default values.
 */
function tpps_submission_get_remove_options($accession = NULL) {
  // Options.
  $options = [
    'clearTppsSubmissionTable' => [
      'default_value' => TRUE,
      'title' => t('Remove record in "tpps_submission" table.'),
      'status_method' => 'hasDbRecord',
      'description' => t('Remove records from "public.tpps_submission" and "chado.dbxref" tables.')
      . theme('item_list', [
        'items' => [
          l(t('View record in "chado.dbxref" table.'),
            'admin/reports/db/chado.dbxref',
            ['query' => ['chado.dbxref.accession' => $accession]]
          ),
          l(t('View record in "tpps_submission" table.'),
            'admin/reports/db/public.tpps_submission',
            ['query' => ['public.tpps_submission.accession' => $accession]]
          ),
        ]
      ]),
    ],
    'removeImportRecord' => [
      'default_value' => TRUE,
      'title' => t('Remove record from the imported submission list'),
      'status_method' => 'wasImported',
      'description' => t('Remove record from the list of imported submissions.')
      . theme('item_list', [
        'items' => [
          l(t('View "Imported Studies" report.'),
            'admin/reports/tpps/submission/imported'
          ),
          l(t('View variable which holds list of imported submissions.'),
            'admin/reports/db/public.variable',
            [
              'query' => [
                'public.variable.name' => 'tpps_imported_studies_list'
              ],
            ]
          ),
        ]
      ]),
    ],
    'clearDB' => [
      'default_value' => TRUE,
      // @TODO Convert into 'checkboxes' and show list of tables which checked
      // by default.
      'title' => t('Clear DB tables.'),
      'description' => t('Remove records from many DB tables.'),
    ],
    'removeFiles' => [
      'default_value' => FALSE,
      'title' => t('Remove files.'),
      'status_method' => 'hasFiles',
      'description' => t('Remove only files not used by other studies.')
      . theme('item_list', [
        'items' => [
          l(t("View Submission's File report."),
            'admin/reports/tpps/submission/file/all/' . $accession
          ),
        ]
      ]),
    ],
  ];
  return $options;
}
