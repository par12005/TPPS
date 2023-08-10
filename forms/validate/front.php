<?php

/**
 * @file
 * Defines the data integrity checks for the form landing page.
 */

/**
 * Defines the data integrity checks for the form landing page.
 *
 * @param array $form
 *   The form that is being validated.
 * @param array $form_state
 *   The state of the form that is being validated.
 */
function tpps_front_page_validate_form(array &$form, array &$form_state) {
  if ($form_state['submitted']) {
    if (tpps_access('administer tpps module') and !empty($form_state['values']['custom_accession_check'])) {
      if (!preg_match('/^TGDR\d{3,}$/', $form_state['values']['custom_accession'])) {
        form_set_error('custom_accession', "The accession number {$form_state['values']['custom_accession']} is invalid.");
      }
      else {
        $result = chado_select_record('dbxref', array('accession'), array(
          'accession' => $form_state['values']['custom_accession'],
        ), array(
          'limit' => 1,
        ));
        if (!empty($result)) {
          form_set_error('custom_accession', "The accession number {$form_state['values']['custom_accession']} is already in use.");
        }
      }
    }
  }
}
