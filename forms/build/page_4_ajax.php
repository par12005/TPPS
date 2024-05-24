<?php

/**
 * @file
 * Defines ajax functions necessary for the fourth page of the form.
 *
 * @TODO Use single function but send required element in request.
 */

/**
 * Ajax callback for BioProject field.
 *
 * Indicates the element to be updated when the NCBI BioProject ID field is
 * changed.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_ajax_bioproject_callback(array &$form, array $form_state) {

  $id = $form_state['triggering_element']['#parents'][0];

  return $form[$id]['genotype']['tripal_eutils'];
}

/**
 * Ajax callback for phenotype fieldset.
 *
 * Indicates the element to be updated when the add or remove phenotype buttons
 * are clicked.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_update_phenotype(array $form, array &$form_state) {
  $id = $form_state['triggering_element']['#parents'][0];

  return $form[$id]['phenotype'];
}

/**
 * Ajax callback for phenotype meta fieldset.
 *
 * Indicates the element to be updated when changes are made in the manual
 * phenotype metadata section.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_update_phenotype_meta(array $form, array &$form_state) {
  $id = $form_state['triggering_element']['#parents'][0];
  $phenotype_num = $form_state['triggering_element']['#parents'][3];
  return $form[$id]['phenotype']['phenotypes-meta'][$phenotype_num];
}

/**
 * Ajax callback for phenotype file format.
 *
 * Indicates the element to be updated when the format option of the phenotype
 * file has been changed.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_phenotype_file_format_callback(array $form, array &$form_state) {
  $id = $form_state['triggering_element']['#parents'][0];

  return $form[$id]['phenotype']['file'];
}

/**
 * Indicate the managed_file element to be updated.
 *
 * This function is called after a no_header element is changed, triggering an
 * update of the managed_file element.
 *
 * @param array $form
 *   The form that needs to be updated.
 * @param array $form_state
 *   The state of the form that needs to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_page_4_file_dynamic(array $form, array &$form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  array_pop($parents);

  $element = drupal_array_get_nested_value($form, $parents);
  // $element contains 'Other Information' fieldset which is parent.
  return $element;
}
