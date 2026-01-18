<?php

/**
 * @file
 * Contains 'FormRemoveSubmission' class.
 *
 * WARNING: form builder must be added in tpps_forms().
 */

/**
 * Manages TPPS Submission Removement Form.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class FormRemoveSubmission {

  /**
   * Builds form. Form callback.
   *
   * @param array $form
   *   The form object to be populated.
   * @param array $form_state
   *   The state of the form object to be populated.
   * @param string $accession
   *   Submission accession. E.g., 'TGDRxxx'.
   *
   * @return array
   *   The populated form object.
   */
  public static function build(array $form, array &$form_state, $accession = '') {
    if (isset($form_state['saved_values']['confirmed'])) {
      FormRemoveSubmission::buildConfirmationForm($form, $form_state);
    }
    else {
      FormRemoveSubmission::buildSelectionForm($form, $accession);
    }
    $form['#attributes']['class'][] = 'tpps-submission';
    tpps_add_css_js('theme', $form);
    tpps_add_css_js('main', $form);
    $form['actions']['cancel']['#attributes']['class'][] = 'btn form-button';


    //$form['submit'] = ['#type' => 'submit', '#value' => t('Submit')];
    //$form['#attributes']['class'][] = 'tpps-submission-tag-form';
    //$form['#attributes']['class'][] = 'tpps';
    $form['accession'] = ['#type' => 'hidden', '#value' => $accession];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Custom.
    $form['#validate'][] = 'FormRemoveSubmission::validate';
    $form['#submit'][] = 'FormRemoveSubmission::submit';
    return $form;
  }

  /**
   * Builds confirmation form.
   *
   * @param array $form
   *   Drupal Form array.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function buildConfirmationForm(array &$form, array &$form_state) {
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
    $removement_options = FormRemoveSubmission::getOptions($accession);
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
   * Builds confirmation form.
   *
   * @param array $form
   *   Drupal Form array.
   * @param string $accession
   *   Submission accession. E.g., 'TGDR1223'.
   */
  public static function buildSelectionForm(array &$form, $accession) {
    if (empty($accession)) {
      drupal_set_message(t('Empty accession.'), 'warning');
      drupal_goto('tpps/submission');
    }
    $submission = new Submission($accession);
    $removement_options = FormRemoveSubmission::getOptions($accession);
    $form['#attributes']['class'][] = 'tpps';

    foreach ($removement_options as $name => $item) {
      $is_empty = FALSE;
      if (isset($item['status_method'])) {
        $is_empty = (bool) $item['status_method'];
        if (method_exists('Submission', $item['status_method'])) {
          $is_empty = empty(call_user_func([$submission, $item['status_method']]));
        }
      }
      $form[$name] = [
        '#type' => 'fieldset',
        '#title' => '<span class="' . ($is_empty ? 'strikethrough' : '') . '">'
          . $item['title'] . '</span>',
        '#tree' => TRUE,
        '#collapsible' => TRUE,
        '#collapsed' => $is_empty,
        '#description' => ($item['description'] ?? NULL),
      ];
      // Useful links.
      if (!empty($item['links'])) {
        $form[$name]['links'] = [
          '#markup' => theme('item_list', ['items' => $item['links']]),
        ];
      }
      // Disable if empty.
      if ($is_empty) {
        // Note: disabled whole fieldset.
        $form[$name]['#disabled'] = TRUE;
        $form[$name]['#default_value'] = FALSE;
      }
      // Extra info.
      if (!empty($item['info']['title'])) {
        $form[$name]['info'] = [
          '#title' => $item['info']['title'],
          '#type' => 'fieldset',
          '#tree' => FALSE,
          '#collapsible' => TRUE,
          'content' => ['#markup' => ($item['info']['content'] ?? '')],
        ];
      }
      if (!$is_empty) {
        $form[$name]['remove'] = [
          '#type' => 'checkbox',
          '#title' => t('Remove'),
          '#default_value' => $item['default_value'],
          '#description' => ($item['remove']['description'] ?? NULL),
        ];
        if (!empty($item['force'])) {
          $form[$name]['force'] = [
            '#type' => 'checkbox',
            '#title' => t('Force'),
            '#description' => ($item['force']['description'] ?? NULL),
          ];
        }
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
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
   * Gets list of removement options.
   *
   * @return array
   *   Returns array to build form and set default values.
   */
  public static function getOptions($accession) {
    if (empty($accession)) {
      return [];
    }
    $submission = new Submission($accession);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Submission object.
    //
    // @TODO Check if Submission Cache is updated.
    // @TODO When selected use all other options too.
    // @TODO REmove this option. Use 'purge' instead because there is no
    // sence to remove record and leave other parts.
    $options['removeSubmissionTable'] = [
      'default_value' => FALSE,
      'title' => t('Form State'),
      'status_method' => 'hasDbRecord',
      'description' => t('Remove Form State records stored in table '
        . '"public.tpps_submission".'),
      'links' => [
        l(t('View record in "chado.dbxref" table.'),
          'admin/reports/db/chado.dbxref',
          ['query' => ['chado.dbxref.accession' => $accession]]
        ),
        l(t('View record in "tpps_submission" table.'),
          'admin/reports/db/public.tpps_submission',
          ['query' => ['public.tpps_submission.accession' => $accession]]
        ),
      ],
      'remove' => [
        'description' => t("Removement of the Form State record (and Shared "
          . "State) means that study can't be found and processed. "
          . "But records in other DB tables and files will be left."),
      ],
      'force' => [
        'description' => t('Remove Form State, all the files and any records '
        . 'in other DB tables.')
        . '<br />'
        . t('WARNING: This is full removement.')
        . '<br />'
        . t('Note: Submission Purge does the same.'),
      ],
    ];

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    //$options['removeDbxrefTable'] = [
    //  'default_value' => FALSE,
    //  'title' => t('Form State'),
    //  'status_method' => 'hasDbRecord',
    //  'description' => t('Remove Form State records from "public.tpps_submission" and "chado.dbxref" tables.')
    //  . theme('item_list', [
    //    'items' => [
    //      l(t('View record in "chado.dbxref" table.'),
    //        'admin/reports/db/chado.dbxref',
    //        ['query' => ['chado.dbxref.accession' => $accession]]
    //      ),
    //      l(t('View record in "tpps_submission" table.'),
    //        'admin/reports/db/public.tpps_submission',
    //        ['query' => ['public.tpps_submission.accession' => $accession]]
    //      ),
    //    ]
    //  ]),
    //];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // DB Tables.
    // @TODO Get code from submit all.
    $options['clearDB'] = [
      'default_value' => FALSE,
      // @TODO Convert into 'checkboxes' and show list of tables which checked
      // by default.
      'title' => t('DB tables'),
      'description' => t('Remove records from DB tables.'),
    ];


    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Zenodo Deposit.
    if (!empty($deposit_id = $submission->getZenodoDepositId())) {
      $options['removeZenodoDeposit'] = [
        'default_value' => FALSE,
        'title' => t('Zenodo Deposit (and files)'),
        'status_method' => 'hasZenodoDepositId',
        'description' => t("Removes unpublished Zenodo Deposit and it's files.")
          . '<br />' . ZenodoHost::getStatus(),
      ];
      try {
        $deposit = ZenodoDeposit::retrieve([
          'deposit_id' => $deposit_id,
        ]);
        if (empty($deposit['submitted'])) {
          // Not yet published.
          $options['removeZenodoDeposit']['status_method'] = FALSE;
          $options['removeZenodoDeposit']['description'] .= '<br />'
            . t("Deposit status: not yet published and could be removed.");
        }
        else {
          // Published.
          $options['removeZenodoDeposit']['status_method'] = TRUE;
          $options['removeZenodoDeposit']['description'] .= '<br />'
            . t("Deposit status: was published and can't be removed.");
        }
      }
      catch (Exception $e) {
        $options['removeZenodoDeposit']['description'] .= '<br />'
          . t("Deposit status: doesn't exist on Zenodo server but submission has "
          . "Zenodo Deposition Id (which probably outdated).");
      }
      $options['removeZenodoDeposit']['links'] = [
        l(
          t('Zenodo Deposit #@deposit_id', ['@deposit_id' => $deposit_id]),
          'tpps/submission/' . $accession . '/zenodo'
        ),
        l(
          t('Zenodo Deposit #@deposit_id Files', ['@deposit_id' => $deposit_id]),
          'tpps/submission/' . $accession . '/zenodo/files'
        ),
        l(t('"Zenodo Submissions" report'), 'admin/reports/tpps/submission/zenodo'),
        l(t('"Zenodo Depositions" report'), 'admin/reports/tpps/zenodo/deposition'),
      ];
    }


    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Imported Submission Metadata.
    $options['removeImportRecord'] = [
      'default_value' => FALSE,
      'title' => t('Record in the list of the imported submissions'),
      'status_method' => 'wasImported',
      'remove' => [
        'description' => t('Remove record from the list of the submissions '
        . 'which was imported. This list is store in Drupal variable with name '
        . '"tpps_imported_studies_list".'),
      ],
      'description' => t('Removes record from the list of imported submissions.'),
      'links' => [
        l(t('View "Imported Studies" report.'),
          'admin/reports/tpps/submission/imported'
        ),
        l(t('View variable which holds list of imported submissions.'),
          'admin/reports/db/public.variable',
          ['query' => ['public.variable.name' => 'tpps_imported_studies_list']]
        ),
      ],
    ];


    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Import Meta-data.
    $options['removeImportMeta'] = [
      'default_value' => FALSE,
      'title' => t('Import Meta-data'),
      'status_method' => 'hasImportMeta',
      'description' => t("Remove Import Meta-data from Submission's Form State"),
      'links' => [
        l(t('View "Imported Studies" report.'), 'admin/reports/tpps/submission/imported'),
        l(t('Submission Cache in JSON format'), 'api/submission/' . $accession . '/cache'),
        l(t("Submission's State View page"), 'tpps/submission/' . $accession . '/view'),
      ],
    ];
    if ($submission->hasImportMeta()) {
      $options['removeImportMeta']['info'] = [
        'content' => TppsArray::print($submission->getImportMeta()),
        'title' => t('Import Meta data'),
      ];
    }


    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Submission Cache.
    $options['removeSubmissionCache'] = [
      'default_value' => FALSE,
      'title' => t("Submission Cache"),
      'status_method' => 'isCached',
      'description' => t("Remove the record from the 'cache_tpps_submission' table."),
      'links' => [
        l(t("View JSON Submission Cache"), 'api/submission/' . $accession . '/cache'),
        l(t('View content of the "public.cache_tpps_submission" table.'),
          'admin/reports/db/public.cache_tpps_submission'
        ),
      ],
      'remove' => [
        'description' => t("Note: If submission won't be fully removed this "
          . "cache will be updated automatically when cached submittion will "
          . "be requested."),
      ],
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Files.
    // Submission::removeFiles().
    $options['removeFiles'] = [
      'default_value' => FALSE,
      'title' => t('Files'),
      'status_method' => 'hasFiles',
      'links' => [
        l(t("View 'File Diagnostics' page"),
          'tpps-admin-panel/file-diagnostics/' . $accession
        ),
        l(t("'Submission's File' report"),
          'admin/reports/tpps/submission/file/all/' . $accession
        ),
      ],
      'description' => t('Remove files linked to submission.'),
      'remove' => [
        'description' => t('Remove only files not used by other studies, modules.')
      ],
      'force' => [
        'description' => t('Remove files which are in use by other modules and '
          . 'files locally stored on server (huge VCF-files).')
          . '<br />'
          . t('Note: Files which has usage greater then 1 are used by other '
          . ' modules, studies. By default those file will not be removed.'),
      ],
    ];

    // Show file status and link to usage page.
    // @TODO Minor. Implement ability to select files for removement.
    $file_list = SubmissionCache::get($accession)['file']['grouped_list'] ?? [];
    // Structure of the $file_list:
    // ['file']['grouped_list'][$group][$organism_label][$field_name] => $fid.
    foreach ($file_list as $group_name => $group_list) {
      foreach ($group_list as $organism_name => $organism_list) {
        foreach ($organism_list as $field_name => $fid) {
          if ($file = tpps_file_load($fid)) {
            $usage = TppsFile::getUsage($file, $accession);
            $tooltip = t('Could NOT be removed');
            $class = 'strikethrough';
            if ($usage == 1) {
              $tooltip = t('Could be removed');
              $class = '';
            }
            TppsArray::setValue(
              $file_list,
              [$group_name, $organism_name, $field_name],
                theme('file_id',
                  [
                    'fid' => $fid,
                    'attributes' => [
                      'title' => $tooltip,
                      'class' => [$class],
                    ],
                  ]
                )
                . ' ' . theme('file_size',
                  ['size' => $file_size = $file->filesize]
                )
                . ' ' . theme('file_action_list',
                   ['accession' => $accession, 'fid' => $fid, 'tiny' => FALSE]
                )
            );
          }
        }
      }
    }
    $options['removeFiles']['info'] = [
      'title' => t("Submission's Files"),
      'content' => TppsArray::print($file_list),
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Tags.
    $links = [
      l(t("Manage tags of this submission"), 'tpps-admin-panel/' . $accession),
      l(t("List of all existing tags"), 'tpps/tag'),
    ];
    if (!is_live_site()) {
      $links[] = l(
        t("Table 'public.tpps_tag' (tags definitions)"),
        'admin/reports/db/public.tpps_tag'
      );
      $links[] = l(
        t("Table 'public.tpps_submission_tag' (relation between tag and submission)"),
        'admin/reports/db/tpps_submission_tag'
      );
    }
    $options['removeAllTags'] = [
      'default_value' => FALSE,
      'title' => t("Submission Tags"),
      'status_method' => 'hasTags',
      'description' => t('Remove tags associated with study.'),
      'links' => $links,
      'remove' => [
        'description' => t('Removes tags associated with study except system tags.')
          . '<br />'
          . t('Note: There are system tag (not custom) which will not be removed.')
          . '<br />'
          . t('System tags are: "TPPS" (#1) and "TPPSc" (#2).'),
      ],
      'force' => [
        'description' => t('Removes all tags (including system tags) associated '
        . 'with study.'),
      ],
    ];
    if ($submission->hasTags()) {
      $options['removeAllTags']['info'] = [
        'title' => t("Submission's Tags"),
        'content' => theme('submission_tag_list',
          ['tags' => $submission->getTagIdList()]
        ),
      ];
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    return $options;
  }

  /**
   * Implements hook_form_submit().
   */
  public static function submit(array $form, array &$form_state) {
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
   * Implements hook_form_validate().
   */
  public static function validate(array &$form, array &$form_state) {
    //if (!empty($result)) {
    //  $message = t("A tag already exists with this name. Please select a "
    //    . "different name or use the existing tag.");
    //  form_set_error('name', $message);
    //}
  }

}
