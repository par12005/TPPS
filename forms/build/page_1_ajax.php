<?php

/**
 * @file
 * Defines the ajax functions necessary for the first page of the form.
 */

/**
 * Ajax callback for the publication status field.
 *
 * This function indicates the element to be updated after the publication
 * status field has been changed.
 *
 * @param array $form
 *   The form that needs to be updated.
 * @param array $form_state
 *   The state of the form that needs to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function page_1_pub_status(array $form, array $form_state) {
  return $form['publication']['year'];
}

function tpps_organism_callback($form, &$form_state) {
  return $form['organism'];
}
