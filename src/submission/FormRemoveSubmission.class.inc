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
    $accession = $form_state['values']['accession'];
    TppsForm::validateAccession($accession);
    $removement_options = FormRemoveSubmission::getOptions($accession);
    // Get removement options.
    $selected_options = [];
    $formState = new FormState($form_state);
    foreach ($removement_options as $name => $item) {
      $fieldset = $formState->saved_values([$name]);
      if (!empty($fieldset)) {
        // Fieldset wasn't empty.
        if (!empty($fieldset['force'])) {
          $selected_options[$name] =
            '<div class="tpps-submission-removement-force">'
            . $item['title']
            . ': ' . t('<strong>force</strong> removement.')
            . $item['force']['description']
            . '</div>';
        }
        elseif (!empty($fieldset['remove'])) {
          $selected_options[$name] = $item['title']
            . ': ' . t('removement.');
        }
        else {
          // Both (remove and force) wasn't selected.
        }
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
    $form['accession'] = [
      '#type' => 'hidden',
      '#value' => $accession,
    ];
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
            '#prefix' => '<div class="tpps-submission-removement-force">',
            '#title' => t('Force'),
            '#suffix' => '</div>',
            '#description' => ($item['force']['description'] ?? NULL),
            '#states' => [
              'visible' => [
                ':input[name="' . $name . '[remove]"]' => ['checked' => TRUE],
              ],
            ],
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
    $form['accession'] = [
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
    $options['clearDataBase'] = [
      'default_value' => FALSE,
      // @TODO check if study was processed.
      // 'status_method' => 'hasFiles',
      // @TODO Convert into 'checkboxes' and show list of tables which checked
      // by default.
      'title' => t('DB tables'),
      'description' => t('Remove records from DB tables.'),
      'info' => [
        'title' => t("Tables to be cleared"),
        'content' => theme('item_list',
          ['items' => $submission->getTableListToClearDataBase()]
        ),
      ],
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Files.
    // Submission::removeFiles().
    $force_removement_description = [
      t('Remove files EVEN if they are used by other studies.'),
    ];
    if (is_live_site()) {
      // Dev-site should not remove local VCF files at all.
      $force_removement_description[] =
        t('Remove local VCF files but ONLY if they are not used by other studies.');
    }
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
      'description' => t('Remove files linked to submission.')
        . '<br />' . t('Note: If any file removed then data loaded to '
        . 'database will be cleared too.'),
      'remove' => [
        'description' => t('Remove only files not used by other studies, modules.')
      ],
      'force' => [
        'description' => theme('item_list',
          ['items' => $force_removement_description]
        ),
      ],
    ];

    // Show file status and link to usage page.
    // @TODO Minor. Implement ability to select files for removement.
    $file_list = SubmissionCache::get($accession)['file']['grouped_list'] ?? [];
    // Structure of the $file_list:
    // ['file']['grouped_list'][$group][$organism_label][$field_name] => $fid.
    foreach ($file_list as $group_name => $group_list) {
      if (empty($group_list)) {
        unset($file_list[$group_name]);
        continue;
      }
      foreach ($group_list as $organism_name => $organism_list) {
        if (empty($organism_list)) {
          unset($file_list[$group_name][$organism_name]);
          continue;
        }
        foreach ($organism_list as $field_name => $fid) {
          if ($file = tpps_file_load($fid)) {
            $usage = TppsFile::getUsage($file, $accession);
            $tooltip = t('Could NOT be removed');
            $class = 'strikethrough';
            if ($usage == 1) {
              $tooltip = t('Could be removed');
              $class = '';
            }
            TppsArray::set(
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
    if (!empty($file_list)) {
      $options['removeFiles']['info'] = [
        'title' => t("Submission's Files"),
        'content' => TppsArray::print($file_list),
      ];
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    return $options;
  }

  /**
   * Implements hook_form_submit().
   */
  public static function submit(array $form, array &$form_state) {

    // @TODO When any file removed - clear db tables (all loaded data).
    if (!empty($form_state['saved_values']['confirmed'])) {
      $formState = new FormState($form_state);
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Confirmed removement.
      $accession = $formState->saved_values(['accession']);
      if (empty($accession)) {
        drupal_set_message(t("Empty submission's accession."));
        drupal_goto('tpps/submission');
      }
      TppsForm::validateAccession($accession);
      $submission = new Submission($accession);
      foreach ($formState->saved_values() as $method_name => $sub_elements) {
        if (empty($sub_elements['remove'])) {
          continue;
        }
        // Remove.
        if (method_exists($submission, $method_name)) {
          $force = $sub_elements['force'];
          $submission->$method_name($force);
        }
      }
      // Show new status. Not all parts could be removed so it makes sense.
      drupal_goto('tpps/submission/' . $accession . '/remove');
    }
    else {
      // Not yet confirmed.
      $form_state['saved_values'] = $form_state['values'];
      $form_state['saved_values']['confirmed'] = TRUE;
      // Rebuild form to show confirmation page.
      $form_state['rebuild'] = TRUE;
    }
//dpm(__METHOD__);
  }

  /**
   * Implements hook_form_validate().
   */
  public static function validate(array &$form, array &$form_state) {
    $accession = $form_state['values']['accession'] ?? NULL;
    TppsForm::validateAccession($accession);

    // @TODO When Form State removed - remove all (redirect to 'purge').

    //if (!empty($result)) {
    //  $message = t("A tag already exists with this name. Please select a "
    //    . "different name or use the existing tag.");
    //  form_set_error('name', $message);
    //}
  }

}
