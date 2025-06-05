<?php

/**
 * @file
 * Contains 'ZenodoDepositWidget' class.
 *
 * @TODO Convert action-links into form-buttons to use only widget's code.
 */

/**
 * Manages Zenodo Deposition Widget.
 *
 * Widget has an action-buttons and Zenodo Depositon object dump or links
 * to Zenodo Page and to submission page on TPPS Admin Panel.
 *
 * @package tpps
 * @author Vlad Savitsky <vlad.savitsky@gmail.com>
 */
class ZenodoDepositWidget {

  const NAME = 'zenodo_deposit_widget';

  /**
   * Adds Zenodo Deposit widget to the form.
   *
   * Widget has an action-buttons and dump of the Zenodo Depositon.
   *
   * Note:
   * 1. Passed by reference $form will be altered.
   * 2. Action-buttons could be shown/hidden and content could be short/full.
   *
   * Usage example:
   * $context = [
   *   'form' => &$form,
   *   'accession' => $accession,
   *   'zenodo_deposit_page_path' => $path . '/' . $accession . '/zenodo',
   *   'zenodo_files_page_path' => $path . '/' . $accession . '/zenodo/files',
   * ];
   * ZenodoDepositWidget::add($context);
   *
   * @param array $context
   *   Data which is required to build widget.
   *   Required keys are:
   *   'form' - array. Drupal Form array.
   *   'accession' - string. TPPS Submission accession. E.g., 'TGDRxxxx'.
   *
   * @return array
   *   Returns Zenodo Deposition array or NULL if deposition wasn't found.
   */
  public static function add(array $context) {
    global $user;

    $debug_mode = $context['debug_mode'] ?? FALSE;
    $accession = $context['accession'];
    if (empty($accession)) {
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
    $widget = &$form[$widget_name];
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
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get Zenodo Deposition.
    if (
      empty($context['deposition'])
      || !ZenodoDeposit::isDeposition($context['deposition'])
    ) {
      // Use Zenodo API.
      if (empty($accession)) {
        drupal_set_message(
          t('Empty accession. Please choose one from the list below.'),
          'error'
        );
        if ($context['zenodo_deposit_page_path']) {
          drupal_goto($context['zenodo_deposit_page_path']);
        }
        return FALSE;
      }
      // tpps_submission_list_validate_accession($accession);
      $submission = $context['submission'] = new Submission($accession);
      // @TODO Minor. Check if submission exists.
      $deposit_id = $context['deposit_id'] = $submission->getZenodoDepositId();
      if ($deposit_id) {
        try {
          $data['deposit_id'] = $deposit_id;
          if (variable_get('tpps_zenodo_host_greedy_search', FALSE)) {
            ZenodoHost::search($data);
          }
          $context['deposition'] = ZenodoDeposit::retrieve($data);
        }
        catch (Exception $e) {
          if (in_array($e->getCode(), [403, 404])) {
            $deposition = NULL;
            $message = t("Submission has Deposit Id (#<strong>@deposit_id"
                . "</strong>) but it <strong>wasn't found</strong> on neither Live Host nor "
                . "Sandbox Host.<br />Suggested solutions: !option_list",
              [
                '@deposit_id' => $deposit_id,
                '!option_list' => theme('item_list', [
                  'items' => [
                    'Create new Deposition to overwrite existing Deposit Id.',
                    'Create and remove to unset Deposit Id.',
                  ],
                ]),
              ]
            );
            drupal_set_message($message, 'error');
          }
        }
      }
    }
    $deposition = $context['deposition'] ?? NULL;
    $widget = array_merge($widget, static::view($context));

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Choose which buttons must be shown.
    // Note: in 'Debug Mode' all the buttons will be shown.
    if (empty($deposition)) {
      $action_list = ['create'];
    }
    else {
      switch ($deposition['state']) {
        // New Deposition.
        case 'unsubmitted':
          if (!$deposition['submitted']) {
            $action_list[] = 'update';
            $action_list[] = 'delete';
            // 'Discard' action rolls back to previous version but for not yet
            // published deposition it works as 'Delete' and it's better to
            // disable it because it's frustrating.
            //$action_list[] = 'discard';
            $action_list[] = 'publish';
          }
          break;

        // Deposition metadata can be updated.
        // If deposition is also unsubmitted (see submitted) files can be updated as well.
        case 'inprogress':
          if ($deposition['submitted']) {
            $action_list[] = 'update';
            $action_list[] = 'newversion';
            $action_list[] = 'discard';
            $action_list[] = 'publish';
          }
          // @todo Add files button.
          break;

        // Deposition has been published.
        case 'done':
          if ($deposition['submitted']) {
            $action_list[] = 'newversion';
            $action_list[] = 'edit';
          }
          break;

        case 'error':
          $message = t('Deposition is in an error state - contact Zenodo support.');
          throw new Exception($message);
      }
    }
    if ($debug_mode) {
      $action_list = [
        'create',
        'update',
        'delete',
        'newversion',
        'discard',
        'edit',
        'publish',
      ];
    }
    if (static::isActionAllowed($user)) {
      // Add action-buttons in own container.
      $widget['actions'] = ['#type' => 'container'];
      foreach ($action_list as $key) {
        $widget['actions'][$key] = [
          '#type' => 'submit',
          '#name' => $key,
          '#value' => t(ucfirst(str_replace('_', ' ', $key))),
          // @todo Minor. Do we need also validator?
          '#validate' => ['ZenodoDepositWidget::validate'],
          '#submit' => ['ZenodoDepositWidget::submit'],
        ];
      }
    }
    // Add Navigation buttons.
    $form['actions'] = $form['actions'] ?? ['#type' => 'container'];
    $action = 'files';
    $form['actions'][$action] = [
      '#type' => 'submit',
      '#name' => $action,
      '#value' => t('Files'),
      // Move button to the right. Huge positive integer number must be used.
      '#weight' => 12345,
      '#validate' => ['ZenodoDepositWidget::validate'],
      '#submit' => ['ZenodoDepositWidget::submit'],
    ];
    return $deposition ?? NULL;
  }

  /**
   * Validator for buttons on the Zenodo Deposit Widget.
   *
   * See ZenodoDepositWidget::add().
   *
   * @param array $form
   *   Drupal Form array.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function validate(array $form, array $form_state) {
    $widget = $form_state['values'][static::NAME];

    $accession = $widget['accession'] ?? '';
    if (empty($accession)) {
      form_set_error('hidden accession field',
        t('Submission accession is required.')
      );
    }
  }

  /**
   * Submitter for the buttons on Zenodo Deposit Widget.
   *
   * See ZenodoDepositWidget::add().
   *
   * @param array $form
   *   Drupal Form array.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function submit(array $form, array $form_state) {
    $action = $form_state['triggering_element']['#name'];
    $widget_name = static::NAME;
    $widget = $form_state['values'][$widget_name];
    $files_page_path = $widget['zenodo_files_page_path'] ?? NULL;

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Files has a subpage.
    if ($action == 'files' && !empty($files_page_path)) {
      //drupal_get_messages();
      if ($files_page_path) {
        drupal_goto($files_page_path);
      }
      $form_state['rebuild'] = TRUE;
    }



    $zenodo_page_path = $widget['zenodo_deposit_page_path'] ?? NULL;
    $accession = $widget['accession'] ?? '';

    $data = [];
    $data['accession'] = $accession;

    $submission = new Submission($accession);
    $deposit_id = $submission->getZenodoDepositId();
    $path = $form_state[$widget_name]['path'] ?? 'tpps/submission';
    // Validate Deposit Id here intead of orm validator because it will be used
    // here and full load of the Submission object takes a lot of time.
    if (empty($deposit_id)) {
      if (!in_array($action, ['create', 'list', 'files'])) {
        $message = t('Submission has no Zenodo Deposit Id. '
          . 'Create Zenodo Deposition first.');
        drupal_set_message($message, 'error');
        if ($zenodo_page_path) {
          drupal_goto($zenodo_page_path);
        }
      }
    }
    else {
      $data['deposit_id'] = $deposit_id;
    }
    // Validate $action.
    $class = 'ZenodoDeposit';
    // ZenodoDeposit::create().
    // ZenodoDeposit::delete().
    // ZenodoDeposit::update().
    // ZenodoDeposit::newVersion().
    // ZenodoDeposit::edit().
    // ZenodoDeposit::update().
    // ZenodoDeposit::discard().
    $method_name = $class . '::' . $action;
    if ($debug_mode ?? FALSE) {
      dpm('Method: ' . $method_name);
    }
    if (is_callable($method_name, TRUE)) {
      // Get response.
      try {
        call_user_func($method_name, $data);
      }
      catch (Exception $e) {
        $message = '@code@message';
        if ($e->getCode() == '1002') {
          $message .= '<br />Please allow overwriting Dataset DOI on [!settings_page_link].';
        }
        $params = [
          '@code' => $e->getCode() ? $e->getCode() . ' - ' : '',
          '@message' => $e->getMessage(),
          '!settings_page_link' => l(t('Zenodo Settings Page'), 'admin/config/tpps/zenodo'),
        ];
      }
    }
    else {
      $message = 'Attempt to call non-existing Zenodo API action: @name.';
      $params = ['@name' => $method_name];
    }
    if (!empty($message)) {
      drupal_set_message(t($message, $params), 'error');
    }
    if ($zenodo_page_path) {
      drupal_goto($zenodo_page_path);
    }
  }

  /**
   * Checks if user could peform actions with deposition.
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

  /**
   * Checks if user must see shorter view of the Deposit dump.
   *
   * @param object $account
   *   Drupal user account.
   * @param object $submission
   *   TPPS Submission.
   *
   * @return bool
   *   Returns TRUE if all data could be shown.
   *   Returns FALSE when shorter version of deposition (only metadata)
   *   must be shown.
   */
  public static function isFullAccess($account, $submission = NULL) {
    $permission = FALSE;
    if (
      user_access('view own zenodo deposits', $account)
      && ($submission->uid ?? FALSE) == $account->uid
    ) {
      $permission = TRUE;
    }
    return $permission;
  }

  /**
   * Print deposition view.
   *
   * @param array $context
   *   Metadata.
   *
   * @return array
   *   Returns form element which shows deposition dump.
   */
  public static function view(array $context) {
    global $user;
    // Output element.
    $element = [];
    $debug_mode = $context['debug_mode'] ?? FALSE;

    if (empty($accession = $context['accession'])) {
      return $element;
    }

    if (empty($deposit_id = $context['deposit_id'])) {
      $element['message'] = [
        '#markup' => theme('item_list', [
          'items' => [
            ZenodoHost::getStatus(),
            t("Submission doesn't have Zenodo Deposition."),
          ],
        ]),
      ];
      return $element;
    }

    $submission = $context['submission'];
    $deposition = $context['deposition'] ?? NULL;
    if (empty($deposition)) {
      return $element;
    }
    $element[$deposit_id] = [
      '#type' => 'fieldset',
      '#title' => t('Zenodo Deposit Id: @id', ['@id' => $deposit_id]),
    ];
    $output = '';
    // Show Deposition Dump.
    if (static::isFullAccess($user, $submission)) {
      // For admins show any Zenodo response.
      $output = $deposition;
      if (is_array($deposition)) {
        $output = ZenodoDeposit::printDeposition($deposition);
      }
    }
    else {
      // Show shorter version of Deposition for regular users.
      if (
        is_array($deposition)
        && !empty($deposition['metadata'])
        // && $deposition['state'] == 'done'
      ) {
        // Zenodo Deposit was created but not yet published.
        // Meghan: Let's only allow regular users to see Zenodo
        // pages for which state == 'done'.
        $content = $deposition['metadata'];
        $content['created'] = str_replace('T', ' ',
          ($deposition['created'] ?? '')
        );
        $content['modified'] = str_replace('T', ' ',
          ($deposition['modified'] ?? '')
        );
        $output = tpps_array_print_list($content);
      }
    }
    $element[$deposit_id]['dump'] = [
      '#markup' => ZenodoHost::getStatus() . '<br />' . $output,
    ];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // @todo Move under $element[$deposit_id]. Action buttons must be updated.
    // Currently we can't show multiple widgets on form because of this issue.
    $element['id'] = [
      '#type' => 'hidden',
      '#value' => $deposit_id ?? NULL,
    ];
    return $element;
  }

}
