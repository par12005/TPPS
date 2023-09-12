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
      $options_arr = [
        'new' => 'Create new TPPSC Submission',
        'placeholder1' => '------------------------- YOUR STUDIES -------------------------',
      ];
      $options_arr = $options_arr + tpps_submission_get_accession_list([
        ['status', 'Incomplete', '='],
        ['uid', $user->uid, '='],
      ]);
      $options_arr['placeholder2'] = '-------------------- OTHER USER STUDIES --------------------';
      $options_arr = $options_arr + tpps_submission_get_accession_list([
        ['status', 'Incomplete'],
        ['uid', $user->uid, '<>'],
      ]);

      if (count($options_arr) > 1) {
        // Has submissions.
        $form['accession'] = array(
          '#type' => 'select',
          '#title' => t('Would you like to load an old TPPSC submission, or create a new one?'),
          '#options' => $options_arr,
          '#default_value' => isset($form_state['saved_values']['frontpage']['accession']) ? $form_state['saved_values']['frontpage']['accession'] : 'new',
        );
      }

      $form['use_old_tgdr'] = array(
        '#type' => 'checkbox',
        '#title' => t('I would like to use an existing TGDR number'),
      );

      $tgdr_options = array('- Select -');

      // $tgdr_query = chado_query('SELECT dbxref_id, accession '
      //   . 'FROM chado.dbxref '
      //   . 'WHERE accession LIKE \'TGDR%\' '
      //     . 'AND accession NOT IN (SELECT accession FROM tpps_submission) '
      //   . 'ORDER BY accession;');

      $tgdr_query = chado_query('SELECT dbxref_id, accession '
        . 'FROM chado.dbxref '
        . 'WHERE accession LIKE \'TGDR%\' '
        . 'ORDER BY accession DESC;');

      foreach ($tgdr_query as $item) {
        $tgdr_options[$item->dbxref_id] = $item->accession;
      }

      $form['old_tgdr'] = array(
        '#type' => 'select',
        '#title' => t('Existing TGDR number'),
        '#options' => $tgdr_options,
        '#states' => array(
          'visible' => array(
            ':input[name="use_old_tgdr"]' => array('checked' => TRUE),
          ),
        ),
      );
    }

    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Continue to TPPSC'),
    );

    $prefix_text = "<div>Welcome to TPPSC!<br><br>"
      . "If you would like to submit your data, you can click the button 'Continue to TPPSC' below!<br><br>"
      . "</div>";

    if (isset($form['accession'])) {
      $form['accession']['#prefix'] = $prefix_text;
    }
    else {
      $form['Next']['#prefix'] = $prefix_text;
    }

    $module_path = drupal_get_path('module', 'tpps');
    $form['#attached']['js'][] = $module_path . TPPS_JS_PATH;
    $form['#attached']['css'][] = $module_path . TPPS_CSS_PATH;

  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // TPPS Form.
  else {
    $image_path = drupal_get_path('module', 'tpps') . '/images/';
    $prefix_text = "<div><figure style=\"text-align:center;\"><img style=\"max-height:100%;max-width:100%;\" src=\"{$image_path}TPPS-1_1118px.jpg\"></figure>";
    $prefix_text .= "<div id=\"landing-buttons\">";
    $prefix_text .= "<a href=\"https://tpps.readthedocs.io/en/latest/\" target=\"blank\" class=\"landing-button\"><button type=\"button\" class=\"btn btn-primary\">TPPS Documentation</button></a>";
    $prefix_text .= "<a href=\"$base_url/tpps/details\" target=\"blank\" class=\"landing-button\"><button type=\"button\" class=\"btn btn-primary\">TPPS Studies</button></a>";
    if (module_exists('cartogratree')) {
      $prefix_text .= "<a href=\"$base_url/ct\" target=\"blank\" class=\"landing-button\"><button type=\"button\" class=\"btn btn-primary\">CartograPlant</button></a>";
    }
    $prefix_text .= "</div></div>";

    if (user_is_anonymous()) {
      $prefix_text .= "<div style='text-align: center'>To begin submitting your data, please ensure that you're logged in to access upload features on this page.</div>";
      $prefix_text .= "<div style='text-align: center'>If you do not have an account <a style='color: #e2b448;' href='/user/register'>register one here</a> or <a style='color: #e2b448' href='/user/login'>click here to login</a></div>";
    }

    $form['description'] = array(
      '#markup' => $prefix_text,
    );

    // [VS] Probably it was attempt to check if use logged in. See user_is_logged_in().
    if (user_is_logged_in()) {
      // Logged in.
      $options_arr = ['new' => 'Create new TPPSC Submission']
        + tpps_submission_get_accession_list([
          ['status', 'Incomplete', '='],
          ['uid', $user->uid, '='],
        ]);

      if (count($options_arr) > 1) {
        // Has submissions.
        $form['accession'] = array(
          '#type' => 'select',
          '#title' => t('Would you like to load an old TPPS submission, or create a new one?'),
          '#options' => $options_arr,
          '#default_value' => $form_state['saved_values']['frontpage']['accession'] ?? 'new',
        );
      }
    }

    if (tpps_access('administer tpps module')) {
      $form['custom_accession_check'] = array(
        '#type' => 'checkbox',
        '#title' => t('I would like to use a custom accession number'),
        '#description' => t('Specify a custom accession number. This feature is available only to users with administrative access, and is generally not required or recommended.'),
      );

      $form['custom_accession'] = array(
        '#type' => 'textfield',
        '#title' => t('Custom Accession number'),
        '#states' => array(
          'visible' => array(
            ':input[name="custom_accession_check"]' => array('checked' => TRUE),
          ),
        ),
        '#description' => t('Use this field to specify a custom accession number. Must be of the format TGDR###'),
      );
    }

    if(user_is_logged_in()) {
      $form['Next'] = array(
        '#type' => 'submit',
        '#value' => t('Submit Data'),
      );
    }

  }
  return $form;
}
