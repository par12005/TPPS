<?php

/**
 * @file
 * Contains 'ZenodoFilesWidget' class.
 */

/**
 * Manages Zenodo Deposition Files Widget.
 *
 * Widget has an action-buttons and list of Zenodo Deposition files.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class ZenodoFilesWidget {

  // Name of the widget form element.
  // Used to get data in validator- and submitter-methods.
  // Note:
  // ZenodoDepositWidget uses different name so both could be used on the same form.
  const NAME = 'zenodo_files_widget';

  /**
   * Adds Zenodo Files widget to the form.
   *
   * Widget has an action-buttons and list of files.
   * Note:
   * 1. All users will see the same content. The difference in action-buttons.
   * 2. Passed by reference $form will be altered.
   *
   * Usage example:
   * $context = [
   *   'form' => &$form,
   *   'accession' => $accession,
   *   'zenodo_deposit_page_path' => $path . '/' . $accession . '/zenodo',
   *   'zenodo_files_page_path' => $path . '/' . $accession . '/zenodo/files',
   * ];
   * ZenodoFilesWidget::add($context);
   *
   * @param array $context
   *   Data which is required to build widget.
   *   Required keys are:
   *   'form' - array. Drupal Form array.
   *   'accession' - string. TPPS Submission accession. E.g., 'TGDRxxxx'.
   */
  public static function add(array $context) {
    global $user;

    $debug_mode = $context['debug_mode'] ?? FALSE;
    if (empty($accession = $context['accession'])) {
      drupal_set_message(
        t('Empty study accession. Please choose one from the list below.'),
        'error'
      );
      return FALSE;
    }
    $form = &$context['form'] ?? NULL;
    if (is_null($form)) {
      $message = t('Zenodo Deposition Widget requires the form array.');
      drupal_set_message($message, 'error');
      return FALSE;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Widget container and hidden fields.
    $widget_name = static::NAME;
    $form[$widget_name] = $form[$widget_name] ?? [
      '#type' => 'container',
      '#tree' => TRUE,
      // Store Submission Accession and Zenodo Deposit Id.
      'accession' => ['#type' => 'hidden', '#value' => $accession],
      'zenodo_deposit_page_path' => [
        '#type' => 'hidden',
        '#value' => $context['zenodo_deposit_page_path'] ?? NULL,
      ],
      'zenodo_files_page_path' => [
        '#type' => 'hidden',
        '#value' => $context['zenodo_files_page_path'] ?? NULL,
      ],
    ];
    $widget = &$form[$widget_name];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get content.
    $submission = new Submission($accession);
    $deposit_id = $submission->getZenodoDepositId();
    $widget['id'] = ['#type' => 'hidden', '#value' => $deposit_id ?? NULL];
    $widget['accession'] = ['#type' => 'hidden', '#value' => $accession];

    $data = ['deposit_id' => $deposit_id];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Validate.
    if (empty($deposit_id)) {
      $message = t('Submission has no Zenodo Deposit Id. Create Zenodo Deposition first.');
      drupal_set_message($message, 'error');
      drupal_goto($context['zenodo_deposit_page_path']);
    }
    if (!ZenodoHost::isValid($data)) {
      $message = t('Deposition was created using different Zenodo Host.');
      drupal_set_message($message, 'error');
      drupal_goto($context['zenodo_deposit_page_path']);
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Permissions.
    $actions_allowed = static::isActionAllowed($user);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Build output.
    $rows = $zenodo_file_list = $submission_files = [];
    $header = [
      t('Submission'),
      t('Zenodo'),
    ];
    $action_link_params = [
      'attributes' => [
        'class' => ['btn', 'form-submit', 'btn-primary'],
        'style' => 'display: inline-block;',
      ],
      'query' => [
        'destination' => $context['zenodo_files_page_path'],
      ],
    ];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Zenodo Files.
    //
    // Format:
    //  id (String, 36 characters ) 83fea13b-8a51-4b67-aa75-089570415401
    //  filename (String, 28 characters ) TGDR400_Phenotype_Data_1.csv
    //  filesize (Float) 12362
    //  checksum (String, 32 characters ) 4bd90201fbb420ae5c149a4af50a224b
    //  links (String, 218 characters ) <a href="https://zenodo.org/api/deposit/deposit...
    $response = ZenodoFile::list($data);
    if (!empty($response)) {
      foreach ($response as $item) {
        foreach ($item as $key => $value) {
          $links = '';
          if ($key == 'links') {
            foreach ($value as $name => $url) {
              $links[] = l($name, Zenodo::addAccessToken($url));
            }
            $value = implode(', ', $links);
          }
          $zenodo_file_list[$item['id']][$key] = $value;
        }
      }
    }
    // Deposition.
    $deposition = ZenodoDeposit::retrieve($data);
    if ($deposition['submitted']) {
      $message = t("Files can't be added or deleted because Zenodo Deposition "
        . "<strong>was published</strong>. <br />Create new version of this "
        . "Deposition to add new or delete existing files."
      );
      drupal_set_message($message, 'warning');
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Submission files.
    $missing_files = [];
    foreach (array_keys($submission->getFileIdList()) as $fid) {
      $create_url = 'tpps/submission/' . $accession . '/zenodo/files/' . $fid . '/create';
      if (empty($file = tpps_file_load($fid))) {
        $missing_files[] = $fid;
        continue;
      }
      // Make it first.
      $rows[$fid]['submission'] = '';
      $rows[$fid]['zenodo'] = '';
      $submission_file = [
        'fid' => $file->fid,
        'uid' => $file->uid,
        'filename' => $file->filename,
        'uri' => $file->uri,
        'filemime' => $file->filemime,
        'filesize' => $file->filesize,
        'status' => $file->status,
        'timestamp' => $file->timestamp,
        'md5' => md5(file_get_contents(tpps_realpath($file->uri))) ?? '',
      ];
      // Find relations between Submission and Zenodo files.
      if (!empty($zenodo_file_list)) {
        foreach ($zenodo_file_list as $zenodo_file) {
          if ($zenodo_file['checksum'] != $submission_file['md5']) {
            continue;
          }
          // Note: Filesize will be the same but filenames could differ.
          unset($zenodo_file['checksum']);
          $zenodo_file_path = 'tpps/submission/' . $accession . '/zenodo/files/' . $zenodo_file['id'];
          $delete_url = $zenodo_file_path . '/delete';
          $update_url = $zenodo_file_path . '/update';
          // 'Delete' button.
          if ($actions_allowed && empty($deposition['submitted'])) {
            $delete_button = l(t('Delete'), $delete_url, $action_link_params);
          }
          $rows[$fid]['zenodo'] = ($delete_button ?? '')
            // Zenodo File metadata.
            . tpps_array_print_list($zenodo_file);
          unset($zenodo_file_list[$zenodo_file['id']]);
        }
      }
      $update_url = 'tpps/submission/' . $accession . '/zenodo/files/' . $fid . '/update';
      unset($submission_file['md5']);

      // Buttons for the Submission side (left).
      $submission_button = '';
      if ($actions_allowed) {
        if (empty($rows[$fid]['zenodo'])) {
          // New files could be added to not yet published Depositions.
          if (!$deposition['submitted']) {
            $submission_button = l(t('Create'), $create_url, $action_link_params);
          }
        }
        else {
          // Files could be updated in published and not yet published Depositions.
          $submission_button = l(t('Update'), $update_url, $action_link_params);
        }
      }
      $rows[$fid]['submission'] = $submission_button
        . tpps_array_print_list($submission_file);
    }

    // Report about files missing in file system.
    if ($missing_files) {
      $message = t('Files listed below was NOT found in file system: <br />@list'
        . '<br />Probably they was removed by cron because of temporary status.'
        . '<br />Those files must be re-attached to study again.',
        ['@list' => implode(', ', $missing_files)]
      );
      drupal_set_message($message, 'error');
    }

    // Add Zenodo files which not match Submission Files.
    if (!empty($zenodo_file_list)) {
      foreach ($zenodo_file_list as $zenodo_file) {
        $rows[$zenodo_file['id']]['submission'] = '';
        if ($actions_allowed && empty($deposition['submitted'])) {
          $delete_url = 'tpps/submission/' . $accession . '/zenodo/files/' . $zenodo_file['id'] . '/delete';
          $delete_button = l(t('Delete'), $delete_url, $action_link_params);
        }
        $rows[$zenodo_file['id']]['zenodo'] = ($delete_button ?? '')
          . tpps_array_print_list($zenodo_file);
        unset($zenodo_file_list[$zenodo_file['id']]);
      }
    }
    // Output.
    $widget['table'] = [
      '#markup' => theme('table', [
        'header' => $header,
        'rows' => $rows,
        'attributes' => ['class' => ['tpps_table']],
      ]),
    ];
    // WARNING: This CSS-class adds theme and design.
    $context['form']['#attributes']['class'][] = 'tpps-submission';
    tpps_add_css_js('main', $widget);
    tpps_add_css_js('theme', $widget);

    // @TODO permissions.
    // @TODO File buttons are links (not form buttons).

    // Add back button.
    $widget['actions'] = ['#type' => 'container'];
    $action = 'back';
    $widget['actions'][$action] = [
      '#type' => 'submit',
      '#name' => $action,
      '#value' => t($action),
      '#validate' => ['ZenodoFilesWidget::validate'],
      '#submit' => ['ZenodoFilesWidget::submit'],
    ];
    if (empty($deposition['submitted'])) {
      $action = 'upload_all';
      $widget['actions'][$action] = [
        '#type' => 'submit',
        '#name' => $action,
        '#value' => t('Upload All'),
        '#validate' => ['ZenodoFilesWidget::validate'],
        '#submit' => ['ZenodoFilesWidget::submit'],
      ];
    }
  }

  /**
   * Validator for buttons of the Widget.
   *
   * @param array $form
   *   Drupal Form array.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function validate(array $form, array $form_state) {
    $widget_name = static::NAME;
    $widget = $form_state['values'][$widget_name];

    $accession = $widget['accession'] ?? '';
    if (empty($accession)) {
      form_set_error('hidden accession field',
        t('Submission accession is required.')
      );
    }
  }

  /**
   * Submitter for the buttons of the widget.
   *
   * @param array $form
   *   Drupal Form array.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function submit(array $form, array $form_state) {
    $widget_name = static::NAME;
    $widget = $form_state['values'][$widget_name];

    $files_page_path = $widget['zenodo_files_page_path'] ?? NULL;
    $zenodo_page_path = $widget['zenodo_deposit_page_path'] ?? NULL;
    $accession = $widget['accession'] ?? '';
    $action = $form_state['triggering_element']['#name'];

    if ($action == 'back') {
      drupal_get_messages();
      drupal_goto($zenodo_page_path);
    }
    // Upload all files.
    elseif ($action == 'upload_all') {
      $data = [];
      $data['accession'] = $accession;
      // Existing files will be overwritten and will got new ids.
      drupal_get_messages();
      $submission = new Submission($accession);
      if ($submission->doesNotExist()) {
        drupal_set_message(t('Submission @accession not found.',
          ['@accession' => $accession]
        ));
        drupal_goto($zenodo_page_path);
      }
      if (empty($submission->getZenodoDepositId())) {
        drupal_set_message(
          t('Submission "@accession" has empty Zenodo Deposit Id.',
          ['@accession' => $accession]
        ), 'error');
        drupal_goto($zenodo_page_path);
      }
      $submission->uploadAllFilesZenodo();
      $form_state['#rebuild'] = TRUE;
    }
  }

  /**
   * Checks if user could peform actions with files.
   *
   * @param object $account
   *   Drupal user account.
   * @param object $submission
   *   TPPS Submission.
   *
   * @return bool
   *   Returns TRUE if actions are allowed and FALSE otherwise.
   */
  public static function isActionAllowed($account, $submission = NULL) {
    $actions_allowed = FALSE;
    if (
      user_access('administer zenodo', $account)
      || $account->uid == 1
      || (
        user_access('view own zenodo deposits', $account)
        && ($submission->uid ?? FALSE) == $account->uid
      )
    ) {
      $actions_allowed = TRUE;
    }
    return $actions_allowed;
  }

}
