<?php

/**
 * @file
 * Removes parts of the TPPS Submission.
 */

/**
 * Menu callback. TPPS Submission Remove Form.
 */
function tpps_submission_remove_form(array $form, array &$form_state, $accession = '') {
  if (isset($form_state['saved_values']['confirmed'])) {
    tpps_submission_remove_confirmation_form($form, $form_state);
  }
  else {
    tpps_submission_remove_selection_form($form, $accession);
  }
  $form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);
  tpps_add_css_js('main', $form);
  $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';
  return $form;
}

/**
 * Form submitter for 'tpps_form_page_1_settings_form' form.
 */
function tpps_submission_remove_form_submit(array $form, array &$form_state) {
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Confirmed removement.
  if (FormState::get($form_state, ['saved_values', 'confirmed'])) {
    $accession = FormState::get($form_state,
      ['saved_values', 'tpps_submission_accession']
    );
    if (empty($accession)) {
      drupal_set_message(t("Empty submission's accession."));
      drupal_goto('tpps/submission');
    }
    module_load_include('inc', 'tpps', 'pages/submission/list');
    tpps_submission_list_validate_accession($accession);
    $submission = new Submission($accession);
    $removement_options = tpps_submission_remove_get_options($accession);

    // Get removement options.
    $options = [];
    foreach (array_keys($removement_options) as $name) {
      $options[$name] = (bool) $form_state['saved_values'][$name];
    }
    // Remove study.
    if (1) {
      // Remove parts according to user selection.
      foreach (array_keys(array_filter($options)) as $method_name) {
        if (method_exists($submission, $method_name)) {
          $submission->$method_name();
        }
      }
    }
    else {
      dpm(var_export($options, 1));
    }
    // Show new status. Not all parts could be removed so it makes sense.
    drupal_goto('tpps/submission/' . $accession . '/remove');
  }
  else {
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Not yet confirmed.
    $accession = $form_state['values']['tpps_submission_accession'] ?? NULL;
    if (empty($accession)) {
      drupal_set_message(t("Empty accession."), 'warning');
      drupal_goto('tpps/submission');
    }
    if (FormState::getClickedButton($form_state) == 'purge') {
      drupal_goto('tpps/submission/' . $accession . '/purge');
    }
    $submission = new Submission($accession);
    $form_state['saved_values']['confirmed'] = TRUE;
    $values_to_store = array_keys(
      tpps_submission_remove_get_options($accession)
    );
    $values_to_store[] = 'tpps_submission_accession';
    foreach ($values_to_store as $name) {
      $form_state['saved_values'][$name] = FormState::get($form_state,
        ['values', $name]
      );
    }
    // Rebuild form to show confirmation page.
    $form_state['rebuild'] = TRUE;
  }
}

/**
 * Builds confirmation form.
 *
 * @param array $form
 *   Drupal Form array.
 * @param string $accession
 *   Submission accession. E.g., 'TGDR1223'.
 */
function tpps_submission_remove_selection_form(array &$form, $accession) {
  if (empty($accession)) {
    drupal_set_message(t('Empty accession.'), 'warning');
    drupal_goto('tpps/submission');
  }
  $submission = new Submission($accession);
  $removement_options = tpps_submission_remove_get_options($accession);
  $form['options'] = [
    '#type' => 'fieldset',
    '#title' => t('Removement options'),
    '#collapsible' => TRUE,
  ];

  foreach ($removement_options as $name => $item) {
    $is_empty = FALSE;
    if (isset($item['status_method'])) {
      $is_empty = (bool) $item['status_method'];
      if (method_exists('Submission', $item['status_method'])) {
        $is_empty = empty(call_user_func([$submission, $item['status_method']]));
      }
    }
    $form['options'][$name] = [
      '#type' => 'checkbox',
      '#title' => '<span class="' . ($is_empty ? 'strikethrough' : '') . '">'
        . $item['title'] . '</span>',
      '#default_value' => $item['default_value'],
      '#description' => ($item['description'] ?? NULL),
    ];
    if ($is_empty) {
      $form['options'][$name]['#disabled'] = TRUE;
      $form['options'][$name]['#default_value'] = FALSE;
    }
  }
  // Buttons.
  $form['actions'] = ['#type' => 'container'];
  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Remove'),
    '#name' => 'remove',
    '#attributes' => ['title' => t('Remove parts of the submission')],
  ];
  $form['actions']['button'] = [
    '#type' => 'submit',
    '#value' => t('Purge'),
    '#name' => 'purge',
    '#attributes' => ['title' => t('Totally remove submission')],
  ];
  $form['tpps_submission_accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
}

/**
 * Builds confirmation form.
 *
 * @param array $form
 *   Drupal Form array.
 * @param array $form_state
 *   Drupal Form State array.
 */
function tpps_submission_remove_confirmation_form(array &$form, array &$form_state) {
  $accession = $form_state['values']['tpps_submission_accession'];
  if (empty($accession)) {
    drupal_set_message(t('Empty accession.'), 'warning');
    drupal_goto('tpps/submission');
  }
  $form['tpps_submission_accession'] = [
    '#type' => 'hidden',
    '#value' => $accession,
  ];
  $submission = new Submission($accession);
  $removement_options = tpps_submission_remove_get_options($accession);
  // Get removement options.
  $selected_options = [];
  foreach ($removement_options as $name => $item) {
    if (!empty(FormState::get($form_state['saved_values'], [$name]))) {
      $selected_options[$name] = $item['title'];
    }
  }
  if (empty($selected_options)) {
    drupal_set_message(t("Nothing was selected to remove."), 'warning');
    drupal_goto('tpps/submission/' . $accession . '/remove');
  }
  $message = t(
    'Confirm removement of the Submission @accession?',
    ['@accession' => $accession]
  );
  $cancel_redirect = 'tpps/submission/' . $accession . '/remove';
  $description = t("Are you sure?");
  // Show list of the selected removement options.
  $description .= '<br />' . t("Removement options of the submission "
    . "@accession:", ['@accession' => $accession]);
  $description .= theme('item_list', [
    'items' => $selected_options,
  ]);

  $confirm_button_text = t('Remove');
  $cancel_button_text = t('Cancel');
  // @TODO Get accession from URL (if it's safe).

  $form = confirm_form(
    $form,
    $message,
    $cancel_redirect,
    $description,
    $confirm_button_text,
    $cancel_button_text
  );
}

/**
 * Gets list of removement options.
 *
 * @return array
 *   Returns array to build form and set default values.
 */
function tpps_submission_remove_get_options($accession) {
  if (empty($accession)) {
    return [];
  }
  $submission = new Submission($accession);
  $options = [
    'clearTppsSubmissionTable' => [
      'default_value' => FALSE,
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
      'default_value' => FALSE,
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

    # @todo Add remove metadata.
    # @TODO Zenodo Deposit and files.

    'clearDB' => [
      'default_value' => FALSE,
      // @TODO Convert into 'checkboxes' and show list of tables which checked
      // by default.
      'title' => t('Clear DB tables.'),
      'description' => t('Remove records from many DB tables.'),
    ],
  ];



  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Submission Cache.
  $options['removeSubmissionCache'] = [
    'default_value' => FALSE,
    'title' => t("Remove Submission's cache."),
    'status_method' => 'isCached',
    'description' => t("Remove record from the 'cache_tpps_submission' table. ")
      . t("Note: If submission won't be fully removed this cache will be "
      . "updated automatically when cached submittion will be requested")
      . theme('item_list', [
        'items' => [
          l(t("View JSON Submission Cache"), 'api/submission/' . $accession . '/cache'),
          l(t('View content of the "public.cache_tpps_submission" table.'),
            'admin/reports/db/public.cache_tpps_submission'
          ),
        ]
      ]),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Files.
  $options['removeFiles'] = [
    'default_value' => FALSE,
    'title' => t('Remove files.'),
    'status_method' => 'hasFiles',
    'description' => t('Remove only files not used by other studies.')
      . '<br />' . t('WARNING: Only files which not in use by other '
      . 'studies will be removed')
      . theme('item_list', [
        'items' => [
          l(t("View Submission's File report."),
            'admin/reports/tpps/submission/file/all/' . $accession
          ),
        ]
      ])
      . TppsArray::print(
        (SubmissionCache::get($accession)['file']['grouped_list'] ?? []),
        t("Submission's Files:")
      ),
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Tags.
  $options['removeAllTags'] = [
    'default_value' => FALSE,
    'title' => t("Remove all submission's tags."),
    'status_method' => 'hasTags',
    'description' => t('Removes tags associated with study.')
      . '<br />' . t('WARNING: All (!) tags (including system) will be removed.')
      . theme('item_list', [
        'items' => [
          l(t("Manage Submission's tags"), 'tpps-admin-panel/' . $accession),
          l(t("Tag's settings page"), 'tpps/tag'),
        ]
      ])
      . t("Submission's Tags:")
      . '<br />'
      . theme('submission_tag_list', ['tags' => $submission->getTagIdList()]),
  ];
  return $options;
}
