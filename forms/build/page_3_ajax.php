<?php

/**
 * @file
 */

/**
 *
 */
function tpps_accession_pop_group(&$form, $form_state) {
  if ($form_state['saved_values'][TPPS_PAGE_1]['organism']['number'] > 1 and !empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
    $species_id = $form_state['triggering_element']['#parents'][1];
    return $form['tree-accession'][$species_id]['pop-group'];
  }
  else {
    return $form['tree-accession']['pop-group'];
  }
}
