<?php

/**
 * @file
 * Defines ajax functions necessary for the fourth page of the form.
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

  $ajax_id = $form_state['triggering_element']['#parents'][0];

  return $form[$ajax_id]['genotype']['tripal_eutils'];
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

  return $form[$id]['phenotype']['phenotypes-meta'];
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
 * Ajax callback for environment fieldset.
 *
 * Indicates the element to be updated when the add or remove environmental data
 * buttons are clicked.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_update_environment(array $form, array &$form_state) {
  $id = $form_state['triggering_element']['#parents'][0];

  return $form[$id]['environment']['env_manual'];
}

/**
 * Ajax callback for genotype files fieldset.
 *
 * Indicates the element to be updated when the genotype marker types checkboxes
 * or the genotype file types checkboxes are updated.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function tpps_genotype_files_callback(array $form, array &$form_state) {
  $id = $form_state['triggering_element']['#parents'][0];

  return $form[$id]['genotype']['files'];
}
