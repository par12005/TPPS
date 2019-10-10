<?php

/**
 * @file
 * Defines the ajax functions necessary for the third page of the form.
 */

/**
 * Ajax callback for the population group fieldset.
 *
 * This function indicates the element to be updated when a population group
 * option is selected for a column in an accession file.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The part of the form to be updated.
 */
function tpps_accession_pop_group(array &$form, array $form_state) {
  $species_id = $form_state['triggering_element']['#parents'][1];
  return $form['tree-accession'][$species_id]['pop-group'];
}

/**
 * Ajax callback for the map button field.
 *
 * This function indicates that the map button field needs to be updated. This
 * happens when the user selects a coordinate projection or custom option from
 * the coordinate projection field.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_study_location_map_ajax(array $form, array $form_state) {
  return $form['study_location']['map-button'];
}

/**
 *
 */
function tpps_accession_multi_file(array &$form, array $form_state) {
  return $form['tree-accession'];
}
