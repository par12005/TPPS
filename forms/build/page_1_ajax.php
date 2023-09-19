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
function tpps_ajax_publication_status_change(array $form, array $form_state) {
  // Show extra fields when 'Published' status was selected.
  return $form['publication']['extra'];
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

/**
 * Ajax callback for the secondary authors fieldset.
 *
 * Indicates the element to be updated when the add or remove secondary authors
 * buttons are clicked.
 *
 * @param array $form
 *   The form that needs to be updated.
 * @param array $form_state
 *   The state of the form that needs to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_authors_callback(array $form, array &$form_state) {
  return $form['publication']['secondaryAuthors'];
}
