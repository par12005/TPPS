<?php

/**
 * @file
 * Define the TPPS landing page.
 *
 * This page can be accessed even by anonymous users. If the user is logged in,
 * they are able to select an existing TPPS Submission, or create a new one.
 */

/**
 * Creates the landing page and its form.
 *
 * @param array $form
 *   The form being created.
 * @param array $form_state
 *   The state of the form being created.
 *
 * @global string $base_url
 *   The base url of the site.
 * @global stdClass $user
 *   The user trying to access the form.
 *
 * @return array
 *   The form for the TPPS landing page.
 */
function tpps_front_create_form(array &$form, array $form_state) {
  global $base_url;
  global $user;
  $is_tppsc = (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if ($is_tppsc) {
    if (user_is_logged_in()) {
      // Logged in.
      $options = [
        'new' => 'Create new TPPSC Submission',
        'placeholder1' =>
          '------------------------ YOUR / INCOMPLETE -------------------------',
      ]
      + tpps_submission_get_accession_list([
        ['status', 'Incomplete'],
        ['uid', $user->uid],
      ]);
      if (variable_get('tpps_front_show_pending_status_mine', FALSE)) {
        $options = $options + [
          'placeholder2' =>
          '--------------------- YOUR / PENDING APPROVAL ----------------------',
        ]
        + tpps_submission_get_accession_list([
          ['status', 'Pending Approval'],
          ['uid', $user->uid],
        ]);
      }
      if (variable_get('tpps_front_show_others_studies', TRUE)) {
        $options = $options + [
          'placeholder3' =>
          '----------------------- OTHERS / INCOMPLETE ------------------------',
        ]
        + tpps_submission_get_accession_list([
          ['status', 'Incomplete'],
          ['uid', $user->uid, '<>'],
        ]);
      }
      if (variable_get('tpps_front_show_pending_status_others', FALSE)) {
        $options = $options + [
          'placeholder4' =>
          '--------------------- OTHERS / PENDING APPROVAL --------------------',
        ]
        + tpps_submission_get_accession_list([
          ['status', 'Pending Approval'],
          ['uid', $user->uid, '<>'],
        ]);
      }

      if (count($options) > 1) {
        $form['accession'] = [
          '#type' => 'select',
          '#title' => t('Would you like to load an old TPPSC submission, '
            . 'or create a new one?'),
          '#options' => $options,
          '#default_value' => $form_state['saved_values']['frontpage']['accession'] ?? 'new',
        ];
      }
      $form['use_old_tgdr'] = [
        '#type' => 'checkbox',
        '#title' => t('I would like to use an existing TGDR number'),
        '#default_value' => FALSE,
      ];
      $form['old_tgdr'] = [
        '#type' => 'select',
        '#title' => t('Existing TGDR number'),
        '#options' => tpps_submission_get_tgdr_number_list(TRUE),
        '#description' => t('<div class="error">WARNING: Using this TGDR '
          . 'number will clear all data associated with this study!</div>'),
        '#states' => [
          'visible' => [
            ':input[name="use_old_tgdr"]' => ['checked' => TRUE],
          ],
        ],
      ];
    }
    $form['Next'] = ['#type' => 'submit', '#value' => t('Continue to TPPSC')];

    $prefix_text = "<div>Welcome to TPPSC!<br><br>"
      . "If you would like to submit your data, you can click the button "
      . "'Continue to TPPSC' below!<br><br></div>";

    if (isset($form['accession'])) {
      $form['accession']['#prefix'] = $prefix_text;
    }
    else {
      $form['Next']['#prefix'] = $prefix_text;
    }

  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // TPPS Form.
  else {
    // @TODO [VS] Move HTML code to template or theme function.
    $prefix_text = '<div><figure style="text-align:center;"><img '
        . 'style="max-height:100%;max-width:100%;" src="/' . TPPS_IMAGES_PATH
        . 'TPPS-1_1118px.jpg"></figure>'
      . '<div id="landing-buttons">'
      . '<a href="https://tpps.readthedocs.io/en/latest/" target="blank" '
        . 'class="landing-button"><button type="button" class="btn '
        . 'btn-primary">TPPS Documentation</button></a>'
        . '<a href="' . $base_url . '/tpps/details" target="blank" '
          . 'class="landing-button"><button type="button" '
          . 'class="btn btn-primary">TPPS Studies</button></a>';

    if (module_exists('cartogratree')) {
      $prefix_text .= "<a href=\"$base_url/ct\" target=\"blank\" class=\"landing-button\"><button type=\"button\" class=\"btn btn-primary\">CartograPlant</button></a>";
    }

    $prefix_text .= "</div></div>";

    if (user_is_anonymous()) {
      $prefix_text .= "<div style='text-align: center'>To begin submitting your data, please ensure that you're logged in to access upload features on this page.</div>";
      $prefix_text .= "<div style='text-align: center'>If you do not have an account <a style='color: #e2b448;' href='/user/register'>register one here</a> or <a style='color: #e2b448' href='/user/login'>click here to login</a></div>";
    }

    $form['description'] = ['#markup' => $prefix_text];
    // @TODO Check what anonymous users will see.
    if (user_is_logged_in()) {
      $options_arr = ['new' => 'Create new TPPSC Submission']
        + tpps_submission_get_accession_list([
          ['status', 'Incomplete', '='],
          ['uid', $user->uid, '='],
        ]);
      if (count($options_arr) > 1) {
        $form['accession'] = [
          '#type' => 'select',
          '#title' => t('Would you like to load an old TPPS submission, '
            . 'or create a new one?'),
          '#options' => $options_arr,
          '#default_value' =>
          $form_state['saved_values']['frontpage']['accession'] ?? 'new',
        ];
      }
      if (tpps_access('administer tpps module')) {
        $form['custom_accession_check'] = [
          '#type' => 'checkbox',
          '#title' => t('I would like to use a custom accession number'),
          '#description' => t('Specify a custom accession number. '
            . 'This feature is available only to users with administrative '
            . 'access, and is generally not required or recommended.'
          ),
        ];
        // This field allows to specify any accession number except existing
        // numbers. This is hard to guess and there is no autocompletion.
        $form['custom_accession'] = [
          '#type' => 'textfield',
          '#title' => t('Custom Accession number'),
          '#states' => [
            'visible' => [
              ':input[name="custom_accession_check"]' => ['checked' => TRUE],
            ],
          ],
          '#description' => t('Use this field to specify a custom accession '
            . 'number. Must be of the format TGDR###'
          ),
        ];
      }
      $form['Next'] = [
        '#type' => 'submit',
        '#value' => t('Submit Data'),
      ];
    }
  }
  tpps_add_css_js('main', $form);
  return $form;
}
