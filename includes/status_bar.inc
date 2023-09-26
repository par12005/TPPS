<?php

/**
 * @file
 * Defines function to display the TPPS Status Bar.
 */

/**
 * Adds the Status Bar to the top of the TPPSc form.
 *
 * The status bar allows users to see where they are in the TPPS Submission form
 * steps.
 * TPPSc Status Bar differs from status bar for TPPS:
 *  - no icons
 *  - active links
 *  - itme's labels.
 *
 * @param array $form
 *   The form being populated.
 * @param array $form_state
 *   The state of the form being populated.
 *
 * @global string $base_url
 *   The base URL of the site.
 *
 * @TODO [VS] Minor. Rename to 'navigation bar' because it doesn't show
 * status now but allows to open form steps.
 */
function tppsc_status_bar(array &$form, array $form_state) {
  $current_page = $form_state['stage'] ?? 1;

  // @TODO Each stage has '#cleared' option which indicates that form step
  // passed validation when it's set and value is TRUE. Remove for TPPSc.
  $form['#prefix'] = '<div class="tgdr_form_status">';
  $page_list = [
    '1' => t('Publication Information'),
    '2' => t('Experimental Design'),
    '3' => t('Data'),
    '4' => t('Review and Submit'),
  ];
  foreach ($page_list as $number => $label) {
    $options['query'] = ['page' => $number];
    if ($current_page == $number) {
      $options['attributes']['class'] = ['current_page'];
    }
    $form['#prefix'] .= '<div>' . l($label, 'tppsc', $options) . '</div>';
  }
  $form['#prefix'] .= '</div>';
}

/**
 * Adds the TPPS Status Bar to the top of the form.
 *
 * The status bar allows users to see where they are in the TPPS Submission form
 * steps.
 *
 * WARNING:
 * Only this function uses icons.
 *
 * @param array $form
 *   The form being populated.
 * @param array $form_state
 *   The state of the form being populated.
 *
 * @global string $base_url
 *   The base URL of the site.
 */
function tpps_status_bar(array &$form, array $form_state) {

  global $base_url;

  $form['#prefix'] = '<div class="tgdr_form_status">';

  /**** Page 1 ****/
  if (isset($form_state['saved_values'][TPPS_PAGE_1]) and $form_state['stage'] != TPPS_PAGE_1) {
    $image_fname = 'message-16-ok.png';
  }
  else {
    $image_fname = 'message-16-warning.png';
  }
  $form['#prefix'] .= "<div><img src='$base_url/misc/$image_fname'> Author and Species Information</div>";

  /**** Page 2 ****/
  if ($form_state['stage'] != TPPS_PAGE_2 and !isset($form_state['saved_values'][TPPS_PAGE_2]['#cleared'])) {
    $image_fname = 'message-16-error.png';
  }
  elseif ($form_state['stage'] != TPPS_PAGE_2 and $form_state['saved_values'][TPPS_PAGE_2]['#cleared'] == TRUE) {
    $image_fname = 'message-16-ok.png';
  }
  else {
    $image_fname = 'message-16-warning.png';
  }
  $form['#prefix'] .= "<div><img src='$base_url/misc/$image_fname'> Experimental Conditions</div>";

  /**** Page 3 ****/
  if ($form_state['stage'] != TPPS_PAGE_3 and !isset($form_state['saved_values'][TPPS_PAGE_3]['#cleared'])) {
    $image_fname = 'message-16-error.png';
  }
  elseif ($form_state['stage'] != TPPS_PAGE_3 and $form_state['saved_values'][TPPS_PAGE_3]['#cleared'] == TRUE) {
    $image_fname = 'message-16-ok.png';
  }
  else {
    $image_fname = 'message-16-warning.png';
  }
  $form['#prefix'] .= "<div><img src='$base_url/misc/$image_fname'> Plant Accession</div>";

  /**** Page 4 ****/
  if ($form_state['stage'] != TPPS_PAGE_4 and !isset($form_state['saved_values'][TPPS_PAGE_4]['#cleared'])) {
    $image_fname = 'message-16-error.png';
  }
  elseif ($form_state['stage'] != TPPS_PAGE_4 and $form_state['saved_values'][TPPS_PAGE_4]['#cleared'] == TRUE) {
    $image_fname = 'message-16-ok.png';
  }
  else {
    $image_fname = 'message-16-warning.png';
  }
  $form['#prefix'] .= "<div><img src='$base_url/misc/$image_fname'> Submit Data</div>";

  $form['#prefix'] .= '</div>';
}
