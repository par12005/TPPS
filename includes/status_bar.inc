<?php

/**
 * @file
 * Defines function to display the TPPS Status Bar.
 */

/**
 * Adds the TPPS Status Bar to the top of the form.
 *
 * The status bar allows users to see where they are in the TPPS Submission form
 * steps.
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

  $form['#prefix'] = '<style>
    .tgdr_form_status div {
      display: inline-block;
      padding: 3px;
      margin-right: 10px;
    }
    .tgdr_form_status div img {
      vertical-align: middle;
      margin-bottom: 4px;
      margin-right: 2px;
    }

    .tgdr_form_status {
      padding: 5px;
      background-color: #ffffff;
      border: 1px solid #cccccc;
      border-radius: 4px;
      margin-bottom: 10px;
    }

    .btn-primary[name=op]{
      margin: 10px 10px 10px 0px;
    }

    a.fieldset-title {
      display: contents;
    }
  </style>';
  $form['#prefix'] .= '<div class="tgdr_form_status">';

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
