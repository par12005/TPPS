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
function ajax_bioproject_callback(array &$form, array $form_state) {

  $ajax_id = $form_state['triggering_element']['#parents'][0];

  return $form[$ajax_id]['genotype']['assembly-auto'];
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
function update_phenotype(array $form, array &$form_state) {
  $id = $form_state['triggering_element']['#parents'][0];

  return $form[$id]['phenotype'];
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
function phenotype_file_format_callback(array $form, array &$form_state) {
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
function update_environment(array $form, array &$form_state) {
  $id = $form_state['triggering_element']['#parents'][0];

  return $form[$id]['environment'];
}

/**
 * Ajax callback for SNPs marker type field.
 *
 * Returns the ajax commands to be executed when the SNPs marker type option is
 * changed.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   Ajax commands to be executed when the SNPs marker type option is changed.
 */
function snps_file_callback(array $form, array $form_state) {
  $id = $form_state['triggering_element']['#parents'][0];
  $commands = array();
  $commands[] = ajax_command_replace("#edit-$id-genotype-file-ajax-wrapper", drupal_render($form[$id]['genotype']['file']));
  if (!$form_state['complete form'][$id]['genotype']['file-type']['Genotype Assay']['#value']) {
    $commands[] = ajax_command_invoke(".form-item-$id-genotype-file", 'hide');
  }
  else {
    $commands[] = ajax_command_invoke(".form-item-$id-genotype-file", 'show');
  }

  return array('#type' => 'ajax', '#commands' => $commands);
}
