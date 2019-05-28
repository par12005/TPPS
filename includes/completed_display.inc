<?php

/**
 * @file
 */

/**
 *
 */
function tpps_completed_display($form, $form_state) {

  global $user;
  global $base_url;
  $params = drupal_get_query_parameters();
  $accession = $params['id'];
  $link = "$base_url/tpps_details/accession/$accession";

  $old_form_state = tpps_load_submission($accession);
  $values = $old_form_state['saved_values'];

  $css = "
  .tgdr_complete_submission {
    background: #ffffff;
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 0px 10px;
  }		
  ";
  drupal_add_css($css, 'inline');

  $display = "<label for='tpps_$accession'>{$values[TPPS_PAGE_1]['publication']['title']}</label><p>You can view the TPPS Details page with completed and approved TPPS and TGDR data <a href=\"$link\">here</a></p>";

  $display .= tpps_table_display($old_form_state);

  $form['a'] = array(
    '#type' => 'hidden',
    '#suffix' => $display,
  );

  return $form;
}
