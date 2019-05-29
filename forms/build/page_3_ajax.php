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
  if ($form_state['saved_values'][TPPS_PAGE_1]['organism']['number'] > 1 and !empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
    $species_id = $form_state['triggering_element']['#parents'][1];
    return $form['tree-accession'][$species_id]['pop-group'];
  }
  else {
    return $form['tree-accession']['pop-group'];
  }
}
