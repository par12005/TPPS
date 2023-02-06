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

  $image_path = drupal_get_path('module', 'tpps') . '/images/';
  $prefix_text = "<div><figure style=\"text-align:center;\"><img style=\"max-height:100%;max-width:100%;\" src=\"{$image_path}TPPS-1.png\"></figure>";
  $prefix_text .= "<div id=\"landing-buttons\">";
  $prefix_text .= "<a href=\"https://tpps.readthedocs.io/en/latest/\" target=\"blank\" class=\"landing-button\"><button type=\"button\" class=\"btn btn-primary\">TPPS Documentation</button></a>";
  $prefix_text .= "<a href=\"$base_url/tpps/details\" target=\"blank\" class=\"landing-button\"><button type=\"button\" class=\"btn btn-primary\">TPPS Studies</button></a>";
  if (module_exists('cartogratree')) {
    $prefix_text .= "<a href=\"$base_url/ct\" target=\"blank\" class=\"landing-button\"><button type=\"button\" class=\"btn btn-primary\">CartograPlant</button></a>";
  }
  $prefix_text .= "</div></div>";


  if(!isset($user->mail)) {
    $prefix_text .= "<div style='text-align: center'>To begin submitting your data, please ensure that you're logged in to access upload features on this page.</div>";
    $prefix_text .= "<div style='text-align: center'>If you do not have an account <a style='color: #e2b448;' href='/user/register'>register one here</a> or <a style='color: #e2b448' href='/user/login'>click here to login</a></div>";
  }

  $form['description'] = array(
    '#markup' => $prefix_text,
  );


  if (isset($user->mail)) {
    // Logged in.
    $options_arr = array();
    $options_arr['new'] = 'Create new TPPS Submission';

    $states = tpps_load_submission_multiple(array(
      'status' => 'Incomplete',
      'uid' => $user->uid,
    ));

    foreach ($states as $state) {
      if (empty($state['tpps_type']) or $state['tpps_type'] != 'tppsc') {
        if ($state != NULL and isset($state['saved_values'][TPPS_PAGE_1]['publication']['title'])) {
          $title = ($state['saved_values'][TPPS_PAGE_1]['publication']['title'] != NULL) ? $state['saved_values'][TPPS_PAGE_1]['publication']['title'] : "No Title";
          $tgdr_id = $state['accession'];
          $options_arr["$tgdr_id"] = "$title";
        }
        else {
          if (isset($state) and !isset($state['saved_values'][TPPS_PAGE_1])) {
            tpps_delete_submission($state['accession'], FALSE);
          }
        }
      }
    }

    if (count($options_arr) > 1) {
      // Has submissions.
      $form['accession'] = array(
        '#type' => 'select',
        '#title' => t('Would you like to load an old TPPS submission, or create a new one?'),
        '#options' => $options_arr,
        '#default_value' => isset($form_state['saved_values']['frontpage']['accession']) ? $form_state['saved_values']['frontpage']['accession'] : 'new',
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

  if (isset($user->mail)) {
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Submit Data'),
    );
  }

  return $form;
}
