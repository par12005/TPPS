<?php

/**
 * @file
 * Creates the Submission Summary form page.
 */

/**
 * Creates the Submission Summary form page.
 *
 * This function displays the data that the user has already submitted, and also
 * allows fields to add additional files to the submission and provide optional
 * comments.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_summary_create_form(array &$form, array $form_state) {

  $supplemental_upload_location = 'public://' . variable_get('tpps_supplemental_files_dir', 'tpps_supplemental');

  $form['comments'] = array(
    '#type' => 'textarea',
    '#title' => 'If you have any additional comments about this submission you would like to include, please write them here:',
    '#prefix' => tpps_table_display($form_state),
  );

  $form['files'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('<div class="fieldset-title">Additional Files</div>'),
    '#description' => 'If there are any additional files you would like to include with your submission, please upload up to 10 files here.',
    '#collapsible' => TRUE,
  );

  $form['files']['add'] = array(
    '#type' => 'button',
    '#title' => t('Add File'),
    '#button_type' => 'button',
    '#value' => t('Add File'),
  );

  $form['files']['remove'] = array(
    '#type' => 'button',
    '#title' => t('Remove File'),
    '#button_type' => 'button',
    '#value' => t('Remove File'),
  );

  $form['files']['number'] = array(
    '#type' => 'hidden',
    '#default_value' => isset($form_state['saved_values']['summarypage']['files']['number']) ? $form_state['saved_values']['summarypage']['files']['number'] : '0',
  );

  for ($i = 1; $i <= 10; $i++) {

    $form['files']["$i"] = array(
      '#type' => 'managed_file',
      '#title' => t("Supplemental File @i", array('@i' => $i)),
      '#upload_validators' => array(
      // These were all the relevant file types I could think of.
        'file_validate_extensions' => array('csv tsv xlsx txt pdf vcf doc docx xls ppt pptx fa fasta img png jpeg jpg zip gz fsa_nt html flat fsa ai '),
      ),
      '#upload_location' => "$supplemental_upload_location",
    );
  }

  $form['release'] = array(
    '#type' => 'checkbox',
    '#title' => t('Release this data through the database immediately.'),
    '#default_value' => isset($form_state['saved_values']['summarypage']['release']) ? $form_state['saved_values']['summarypage']['release'] : TRUE,
  );

  $form['release-date'] = array(
    '#type' => 'date',
    '#title' => t('Please select the release date for the dataset.'),
    '#default_value' => isset($form_state['saved_values']['summarypage']['release-date']) ? $form_state['saved_values']['summarypage']['release-date'] : NULL,
    '#states' => array(
      'visible' => array(
        ':input[name="release"]' => array('checked' => FALSE),
      ),
    ),
  );

  $analysis_options = array(
    'diversity' => 'Diversity',
    'population_structure' => 'Population Structure',
    'association_genetics' => 'Association Genetics',
    'landscape_genomics' => 'Landscape Genomics',
    'phenotype_environment' => 'Phenotype-Environment',
  );

  $form['analysis'] = array(
    '#type' => 'fieldset',
    '#title' => t('Analysis'),
    '#tree' => TRUE,
  );

  foreach ($analysis_options as $option => $label) {
    $form['analysis']["{$option}_check"] = array(
      '#type' => 'checkbox',
      '#title' => $label,
    );

    $form['analysis']["{$option}_file"] = array(
      '#type' => 'managed_file',
      '#title' => $label . " file:",
      '#description' => t('Please upload the file associated with this analysis type'),
      '#upload_location' => 'public://' . variable_get('tpps_analysis_dir', 'tpps_analysis'),
      '#upload_validators' => array(
        'file_validate_extensions' => array(),
      ),
      '#states' => array(
        'visible' => array(
          ":input[name=\"analysis[{$option}_check]\"]" => array('checked' => TRUE),
        ),
      ),
    );

    $form['analysis']["{$option}_file_description"] = array(
      '#type' => 'textfield',
      '#title' => $label . " file description:",
      '#states' => array(
        'visible' => array(
          ":input[name=\"analysis[{$option}_check]\"]" => array('checked' => TRUE),
        ),
      ),
    );
  }

  $org_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];
  $new_species = array();
  for ($i = 1; $i <= $org_number; $i++) {
    $org = $form_state['saved_values'][TPPS_PAGE_1]['organism'][$i]['name'];
    $parts = explode(" ", $org);

    $org_record = chado_select_record('organism', array('organism_id'), array(
      'genus' => $parts[0],
      'species' => implode(" ", array_slice($parts, 1)),
    ));
    if (empty($org_record)) {
      $new_species[] = $org;
    }
  }

  if (!empty($new_species) and !empty(variable_get('tpps_tree_pics_files_dir', NULL))) {
    $form['tree_pictures'] = array(
      '#type' => 'fieldset',
      '#title' => t('The following plants are new in the database and will need pictures:'),
      '#tree' => TRUE,
    );

    foreach ($new_species as $org) {
      $form['tree_pictures'][$org] = array(
        '#type' => 'managed_file',
        '#title' => t('Picture for @org: (optional)', array('@org' => $org)),
        '#upload_location' => 'public://' . variable_get('tpps_tree_pics_files_dir'),
        '#upload_validators' => array(
          'file_validate_extensions' => array('jpeg jpg'),
        ),
        '#description' => t('Please upload a photo of the species in either .jpeg or .jpg format'),
      );

      if (db_table_exists('treepictures_metadata')) {
        $form['tree_pictures']["{$org}_url"] = array(
          '#type' => 'textfield',
          '#title' => t('@org Picture source URL:', array('@org' => $org)),
          '#states' => array(
            'invisible' => array(
              ":input[name=\"tree_pictures[{$org}][fid]\"]" => array('value' => 0),
            ),
          ),
        );

        $form['tree_pictures']["{$org}_attribution"] = array(
          '#type' => 'textfield',
          '#title' => t('@org Picture Attribution:', array('@org' => $org)),
          '#states' => array(
            'invisible' => array(
              ":input[name=\"tree_pictures[{$org}][fid]\"]" => array('value' => 0),
            ),
          ),
        );

        $form['tree_pictures']["{$org}_license"] = array(
          '#type' => 'textfield',
          '#title' => t('@org Picture License:', array('@org' => $org)),
          '#states' => array(
            'invisible' => array(
              ":input[name=\"tree_pictures[{$org}][fid]\"]" => array('value' => 0),
            ),
          ),
        );
      }
    }
  }

  $form['Back'] = array(
    '#type' => 'submit',
    '#value' => t('Back'),
  );

  $form['Next'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}
