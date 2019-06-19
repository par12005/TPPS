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

  if (isset($user->mail)) {
    // Logged in.
    $options_arr = array();
    $options_arr['new'] = 'Create new TPPS Submission';

    $states = tpps_load_submission_multiple(array('status' => 'Incomplete'));

    foreach ($states as $state) {
      if ($state != NULL and isset($state['saved_values'][TPPS_PAGE_1]['publication']['title'])) {
        $title = ($state['saved_values'][TPPS_PAGE_1]['publication']['title'] != NULL) ? $state['saved_values'][TPPS_PAGE_1]['publication']['title'] : "No Title";
        $tgdr_id = $state['accession'];
        $options_arr["$tgdr_id"] = "$title";
      }
      else {
        if (isset($state) and !isset($state['saved_values'][TPPS_PAGE_1])) {
          tpps_delete_submission($state['accession']);
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

  $form['Next'] = array(
    '#type' => 'submit',
    '#value' => t('Continue to TPPS'),
  );

  $image_path = drupal_get_path('module', 'tpps') . '/images/';
  $prefix_text = "<div>Welcome to TPPS!<br><br>The Tripal PopGen Submit (TPPS) workflow provides researchers with a streamlined submission interface for studies resulting from any combination of genotype, phenotype, and environmental data for georeferenced forest trees.<br>";
  $prefix_text .= "<figure><img style=\"max-height:100%;max-width:100%;\" src=\"{$image_path}TPPS_front_diagram.png\"></figure>";
  $prefix_text .= "TPPS has documentation to assist users with the process of creating a submission, which can be accessed <a target=\"blank\" href=\"https://tpps.readthedocs.io/en/latest/user.html\">here</a>.<br><br></div>";

  if (isset($form['accession'])) {
    $form['accession']['#prefix'] = $prefix_text;
  }
  else {
    $form['Next']['#prefix'] = $prefix_text;
  }

  return $form;
}
