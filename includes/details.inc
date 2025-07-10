<?php

/**
 * @file
 * Defines functions to be used in the TPPS Details pages.
 */



/**
 * AJAX callback for the search form details_op element.
 *
 * @param array $form
 *   The form being updated.
 * @param array $form_state
 *   The state of the form being updated.
 *
 * @return array
 *   The part of the form to be updated.
 */
function tpps_details_form_callback(array $form, array $form_state) {
  return array(
    'details_type' => $form['details_type'],
    'details_op' => $form['details_op'],
    'details_value' => $form['details_value'],
  );
}
