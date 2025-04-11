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
   */
  public static function add(array $context) {
    // Defines if links from Zenodo Deposition Object be shown.
    $show_deposition_links = $context['show_deposition_links'] ?? FALSE;

    if (empty($accession = $context['accession'])) {
      drupal_set_message(
        t('Empty study accession. Please choose one from the list below.'),
        'error'
      );
      return;
    }
    $form = &$context['form'] ?? NULL;
    if (is_null($form)) {
      drupal_set_message(t('Zenodo Deposition Widget requres form.'), 'error');
      return;
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
            ZenodoDeposit::searchHost($data);
          }
          $deposition = ZenodoDeposit::retrieve($data);
          if (!empty($context['show_deposition_dump'])) {
            // Show Deposition Dump.
            $output = $deposition;
            if (is_array($deposition)) {
              $output = ZenodoDeposit::printDeposition($deposition);
            }
            $zenodo_widget[$deposit_id]['dump'] = [
              '#markup' => ZenodoDeposit::getHostStatus() . '<br />' . $output,
            ];
          }
          else {
            // Only a few links.
            $items = [
              ZenodoDeposit::getHostStatus(),
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
            $message = t('Submission has Deposit Id (#@deposit_id) but it'
              . ' was NOT found on any Zenodo Host. Options: !option_list',
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
              ZenodoDeposit::getHostStatus(),
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
    $action_list = ['create'];
    if (!empty($deposition)) {
      $action_list = [
        'update', 'delete',
        'publish', 'newversion',
        // @todo Minor. Buttons which must be shown in certain conditions.
        // 'discard', 'edit',
      ];
    }
    // Add action-buttons in own container.
    $zenodo_widget['actions'] = ['#type' => 'container'];
    foreach ($action_list as $key) {
      // Show only 'Create' button if there is no Zenodo Deposition and
      // don't show 'Create' Button when Depositon exists.
      if (
        !empty($deposition['id'])
        // Hide 'Edit' button for not yet submitted Depositions.
        && $key == 'edit' && empty($deposition['submitted'])
      ) {
        continue;
      }
      $zenodo_widget['actions'][$key] = [
        '#type' => 'submit',
        '#name' => $key,
        '#value' => t(ucfirst(str_replace('_', ' ', $key))),
        // @todo Minor. Do we need also validator?
        '#validate' => ['ZenodoWidget::validate'],
        '#submit' => ['ZenodoWidget::submit'],
      ];
    }
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
    $method_name = $class . '::' . $action;
    if (!is_callable($method_name, TRUE)) {
      drupal_set_message(
        t(
          'Attempt to call non-existing Zenodo API action: @name.',
          ['@name' => $method_name]
        ), 'error'
      );
      if ($zenodo_page_path) {
        drupal_goto($zenodo_page_path);
      }
    }
    // Get response.
    call_user_func($method_name, $data);
  }

}
