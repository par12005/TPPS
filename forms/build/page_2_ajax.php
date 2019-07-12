<?php

/**
 * @file
 * Defines the ajax functions necessary for the second page of the form.
 */

/**
 * Ajax callback for the ending year field.
 *
 * This function indicates the element to be updated after the starting year
 * field has been changed.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function ajax_date_year_callback(array &$form, array $form_state) {
  return $form['EndingDate']['year'];
}

/**
 * Ajax callback for the ending month field.
 *
 * This function indicates the element to be updated after the starting month or
 * the ending year field has been changed.
 *
 * @param array $form
 *   The form to be updated.
 * @param array $form_state
 *   The state of the form to be updated.
 *
 * @return array
 *   The element in the form to be updated.
 */
function ajax_date_month_callback(array &$form, array $form_state) {
  return $form['EndingDate']['month'];
}

/**
 * Ajax callback for study type.
 *
 * This function updates the study_info fieldset when the study type dropdown
 * menu element is changed.
 *
 * @param array $form
 *   The form being updated.
 * @param array $form_state
 *   The state of the form being updated.
 *
 * @return array
 *   The part of the form to be updated.
 */
function study_type_callback(array &$form, array $form_state) {
  return $form['study_info'];
}
