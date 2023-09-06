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

/**
 * DOI Field AJAX-callback.
 *
 * Note: Only curation form has DOI fields.
 */
function tpps_ajax_doi_callback(array &$form, array $form_state) {
  $is_tppsc = (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');
  if ($is_tppsc) {


    if (!empty($value = $form['publication']['doi_continer']['doi']['#value'])) {
      // Check the tpps_submissionos.
      // @TODO Search for doi value instead of getting all the data.
      // It's a huge amount of data.
      $tpps_submissions = chado_query("SELECT * FROM public.tpps_submission;");
      $found_doi = FALSE;
      $found_doi_accession = "";


      // @TODO Fill fields.
      foreach ($tpps_submissions as $submission_row) {
        $submission_state = unserialize($submission_row->submission_state);
        if (strtolower($value) == strtolower($submission_state['saved_values']['1']['doi'])) {
          $found_doi = TRUE;
          $found_doi_accession = $submission_row->accession;
          break;
        }
        // dpm($submission_state['saved_values']['1']['doi']);
      }
      if ($found_doi) {
        form_set_error('doi', "WARNING: DOI is already used by " . $found_doi_accession);
        $form['publication']['extra']['message']['#markup'] =
          // @TODO Use CSS rules.
          "<div style='text-align: right; color: "
          . "red;'>WARNING: DOI is already used by " . $found_doi_accession
          . "</div>";
      }
    }
    return $form['publication']['extra'];
  }
}
