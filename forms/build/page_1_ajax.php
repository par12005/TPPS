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
function tpps_pub_status(array $form, array $form_state) {
  return $form['publication']['year'];
}

/**
 * Ajax callback for the organism fieldset.
 *
 * Indicates the element to be updated when the add or remove organism buttons
 * are clicked.
 *
 * @param array $form
 *   The form that needs to be updated.
 * @param array $form_state
 *   The state of the form that needs to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_organism_callback(array $form, array &$form_state) {
  return $form['organism'];
}
