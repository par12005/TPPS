<?php

/**
 * @file
 * Contains 'ZenodoWidget' class.
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
class ZenodoWidget {

  /**
   * Adds Zenodo Deposit widget to the form.
   *
   * Widget has an action-buttons and Zenodo Depositon object dump or links
   * to Zenodo Page.
   *
   * Usage example:
   * // Zenodo Deposition Widget.
   * $context = [
   *   'form' => &$form,
   *   'accession' => $accession,
   *   'zenodo_page_path' => $path . '/' . $accession . '/zenodo',
   *   'files_page_path' => $path . '/' . $accession . '/zenodo/files',
   *   'show_deposition_dump' => FALSE,
   *   'show_deposition_links' => FALSE,
   * ];
   * ZenodoDeposit::addWidget($context);
   *
   * @param array $context
   *   Data which is required to build widget.
   *   Required keys are:
   *   'form' - array. Drupal Form array.
   *   'accession' - string. TPPS Submission accession. E.g., 'TGDRxxxx'.
   *
   * @return array
   *   Returns Zenodo Depostion array or NULL if deposition wasn't found.
   */
  public static function add(array $context) {
    $debug_mode = $context['debug_mode'] ?? FALSE;
    // Defines if links from Zenodo Deposition Object be shown.
    $show_deposition_links = $context['show_deposition_links'] ?? FALSE;

    if (empty($accession = $context['accession'])) {
      drupal_set_message(
        t('Empty study accession. Please choose one from the list below.'),
        'error'
      );
      return FALSE;
    }
    $form = &$context['form'] ?? NULL;
    if (is_null($form)) {
      drupal_set_message(t('Zenodo Deposition Widget requres form.'), 'error');
      return FALSE;
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Widget container and hidden fields.
    $form['zenodo_deposition'] = $form['zenodo_deposition'] ?? [
      '#type' => 'container',
      '#tree' => TRUE,
      // Store Submission Accession and Zenodo Deposit Id.
      'accession' => ['#type' => 'hidden', '#value' => $accession],
      'zenodo_page_path' => [
        '#type' => 'hidden',
        '#value' => $context['zenodo_page_path'] ?? NULL,
      ],
      'files_page_path' => [
        '#type' => 'hidden',
        '#value' => $context['files_page_path'] ?? NULL,
      ],
    ];
    $zenodo_widget = &$form['zenodo_deposition'];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Get Zenodo Deposition.
    if (
      !empty($context['$deposition'])
      && ZenodoDeposit::isDeposition($context['deposition'])
    ) {
      // Zenodo Deposition was already loaded.
      $deposition = $context['$deposition'];
    }
    else {
      // Use Zenodo API.
      if (empty($accession)) {
        drupal_set_message(
          t('Empty acession. Please choose one from the list below.'),
          'error'
        );
        if ($context['zenodo_page_path']) {
          drupal_goto($context['zenodo_page_path']);
        }
      }
      // tpps_submission_list_validate_accession($accession);
      $submission = new Submission($accession);

      // Check if submission exists.
      $deposit_id = $submission->getZenodoDepositId();

      // Get Zenodo Deposition.
      if ($deposit_id) {
        $zenodo_widget[$deposit_id] = [
          '#type' => 'fieldset',
          '#title' => t('Zenodo Deposit Id: @id', ['@id' => $deposit_id]),
        ];
        $data['deposit_id'] = $deposit_id;
        try {
          if (variable_get('tpps_zenodo_host_greedy_search', FALSE)) {
            ZenodoHost::search($data);
          }
          $deposition = ZenodoDeposit::retrieve($data);
          if (!empty($context['show_deposition_dump'])) {
            // Show Deposition Dump.
            $output = $deposition;
            if (is_array($deposition)) {
              $output = ZenodoDeposit::printDeposition($deposition);
            }
            $zenodo_widget[$deposit_id]['dump'] = [
              '#markup' => ZenodoHost::getStatus() . '<br />' . $output,
            ];
          }
          else {
            // Only a few links.
            $items = [
              ZenodoHost::getStatus(),
              l(t('Zenodo Deposition'),
                'tpps/submission/' . $accession . '/zenodo'
              ),
              l(t('Zenodo Deposition Files'),
                'tpps/submission/' . $accession . '/zenodo/files'
              ),
            ];
            if ($show_deposition_links ?? FALSE) {
              $zenodo_links = [];
              foreach ($deposition['links'] as $name => $url) {
                if (in_array($name, ['html', 'badge', 'latest_draft_html'])) {
                  $zenodo_links[] = l($name, $url);
                }
                else {
                  $zenodo_links[] = l($name, Zenodo::addAccessToken($url));
                }
              }
              $items = array_merge($items, $zenodo_links);
            }
            $zenodo_widget[$deposit_id]['links'] = [
              '#markup' => theme('item_list', ['items' => $items]),
            ];
          }
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
          // @todo Idea: Switch to other host and retrieve deposition again.
          $zenodo_widget[$deposit_id]['dump'] = [
            '#markup' => t('No Depostion found.'),
          ];
        }
      }
      else {
        $zenodo_widget['message'] = [
          '#markup' => theme('item_list', [
            'items' => [
              ZenodoHost::getStatus(),
              t("Submission doesn't have Zenodo Deposition."),
            ],
          ]),
        ];
      }
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    $zenodo_widget['id'] = [
      '#type' => 'hidden',
      '#value' => $deposit_id ?? NULL,
    ];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Show buttons.
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

    // Add action-buttons in own container.
    $zenodo_widget['actions'] = ['#type' => 'container'];
    foreach ($action_list as $key) {
      $zenodo_widget['actions'][$key] = [
        '#type' => 'submit',
        '#name' => $key,
        '#value' => t(ucfirst(str_replace('_', ' ', $key))),
        // @todo Minor. Do we need also validator?
        '#validate' => ['ZenodoWidget::validate'],
        '#submit' => ['ZenodoWidget::submit'],
      ];
    }
    return $deposition ?? NULL;
  }

  /**
   * Validator for buttons on the Zenodo Deposit Widget.
   *
   * See ZenodoWidget::add().
   *
   * @param array $form
   *   Drupal Form array.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function validate(array $form, array $form_state) {
    $zenodo_widget = $form_state['values']['zenodo_deposition'];

    $accession = $zenodo_widget['accession'] ?? '';
    if (empty($accession)) {
      form_set_error('hidden accession field',
        t('Submission accession is required.')
      );
    }
  }

  /**
   * Submitter for the buttons on Zenodo Deposit Widget.
   *
   * See ZenodoWidget::add().
   *
   * @param array $form
   *   Drupal Form array.
   * @param array $form_state
   *   Drupal Form State array.
   */
  public static function submit(array $form, array $form_state) {
    $zenodo_widget = $form_state['values']['zenodo_deposition'];

    $files_page_path = $zenodo_widget['files_page_path'] ?? NULL;
    $zenodo_page_path = $zenodo_widget['zenodo_page_path'] ?? NULL;
    $accession = $zenodo_widget['accession'] ?? '';
    $action = $form_state['triggering_element']['#name'];

    $data = [];
    $data['accession'] = $accession;

    $submission = new Submission($accession);
    $deposit_id = $submission->getZenodoDepositId();
    $path = $form_state['zenodo_deposition']['path'] ?? 'tpps/submission';
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
    // Files has a subpage.
    if ($action == 'files' && !empty($files_page_path)) {
      drupal_get_messages();
      if ($files_page_path) {
        drupal_goto($files_page_path);
      }
    }
    // Validate $action.
    $class = 'ZenodoDeposit';
    // ZenodoDeposit::create().
    // ZenodoDeposit::delete().
    // ZenodoDeposit::update().
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

}
