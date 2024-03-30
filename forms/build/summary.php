<?php

/**
 * @file
 * Creates the Submission Summary form page.
 */

/**
 * Creates the Submission Summary (last before full submit) form page.
 *
 * This function displays the data that the user has already submitted,
 * and also allows fields to add additional files to the submission
 * and provide optional comments.
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
  $summary_values = &$form_state['saved_values']['summarypage'] ?? [];
  // @TODO Update top navigation bar.
  $supplemental_upload_location = 'public://'
    . variable_get('tpps_supplemental_files_dir', 'tpps_supplemental');
  // When enabled TPPS uses Rachel's theme.
  //$form['#attributes']['class'][] = 'tpps-submission';
  tpps_add_css_js('theme', $form);

  $accession = tpps_form_get_accession($form_state);
  $submission = new Submission($accession);
  $submission->load();

  $form['table_display'] = [
    '#markup' => tpps_table_display($submission->sharedState),
  ];
  $form['comments'] = [
    '#type' => 'textarea',
    '#title' => t('If you have any additional comments about this submission '
    . 'you would like to include, please write them here:'),
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Files.
  $form['files'] = [
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Additional Files'),
    '#description' => t('If there are any additional files you would like '
      . 'to include with your submission, please upload up to 10 files here.'),
    '#collapsible' => TRUE,
  ];
  $form['files']['add'] = [
    '#type' => 'button',
    '#title' => t('Add File'),
    '#button_type' => 'button',
    '#value' => t('Add File'),
  ];
  $form['files']['remove'] = [
    '#type' => 'button',
    '#title' => t('Remove File'),
    '#button_type' => 'button',
    '#value' => t('Remove File'),
  ];
  $form['files']['number'] = [
    '#type' => 'hidden',
    '#default_value' => $summary_values['files']['number'] ?? '0',
  ];
  for ($i = 1; $i <= 10; $i++) {
    $form['files']["$i"] = [
      '#type' => 'managed_file',
      '#title' => t('Supplemental File @i', ['@i' => $i]),
      '#upload_validators' => [
      // These were all the relevant file types I could think of.
        'file_validate_extensions' => ['csv tsv xlsx txt pdf vcf '
          . 'doc docx xls ppt pptx fa fasta '
          . 'img png jpeg jpg zip gz fsa_nt html flat fsa ai '
        ],
      ],
      '#upload_location' => "$supplemental_upload_location",
    ];
  }
  $form['release'] = [
    '#type' => 'checkbox',
    '#title' => t('Release this data through the database immediately.'),
    '#default_value' => $summary_values['release'] ?? TRUE,
  ];
  $form['release-date'] = [
    '#type' => 'date',
    '#title' => t('Please select the release date for the dataset.'),
    '#default_value' => $summary_values['release-date'] ?? NULL,
    '#states' => [
      'visible' => [':input[name="release"]' => ['checked' => FALSE]],
    ],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Analysis.
  $analysis_options = [
    'diversity' => 'Diversity',
    'population_structure' => 'Population Structure',
    'association_genetics' => 'Association Genetics',
    'landscape_genomics' => 'Landscape Genomics',
    'phenotype_environment' => 'Phenotype-Environment',
  ];
  $form['analysis'] = [
    '#type' => 'fieldset',
    '#title' => t('Analysis'),
    '#tree' => TRUE,
  ];
  foreach ($analysis_options as $option => $label) {
    $form['analysis']["{$option}_check"] = [
      '#type' => 'checkbox',
      '#title' => $label,
    ];
    $form['analysis']["{$option}_file"] = [
      '#type' => 'managed_file',
      '#title' => $label . " file:",
      '#description' => t('Please upload the file associated with this analysis type'),
      '#upload_location' => 'public://' . variable_get('tpps_analysis_dir', 'tpps_analysis'),
      '#upload_validators' => [
        'file_validate_extensions' => [],
      ],
      '#states' => [
        'visible' => [
          ":input[name=\"analysis[{$option}_check]\"]" => ['checked' => TRUE],
        ],
      ],
    ];
    $form['analysis']["{$option}_file_description"] = [
      '#type' => 'textfield',
      '#title' => $label . " file description:",
      '#states' => [
        'visible' => [
          ":input[name=\"analysis[{$option}_check]\"]" => ['checked' => TRUE],
        ],
      ],
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $page1_values = &$form_state['saved_values'][TPPS_PAGE_1] ?? [];
  $org_number = $page1_values['organism']['number'];
  $new_species = array();
  for ($i = 1; $i <= $org_number; $i++) {
    $org = $page1_values['organism'][$i]['name'];
    $parts = explode(" ", $org);

    $org_record = chado_select_record('organism', ['organism_id'], [
      'genus' => $parts[0],
      'species' => implode(" ", array_slice($parts, 1)),
    ]);
    if (empty($org_record)) {
      $new_species[] = $org;
    }
  }
  $tree_pics_dir = variable_get('tpps_tree_pics_files_dir', NULL);
  if (!empty($new_species) && !empty($tree_pics_dir)) {
    $form['tree_pictures'] = [
      '#type' => 'fieldset',
      '#title' => t('The following plants are new in the database '
        . 'and will need pictures:'),
      '#tree' => TRUE,
    ];
    foreach ($new_species as $org) {
      $form['tree_pictures'][$org] = [
        '#type' => 'managed_file',
        '#title' => t('Picture for @org: (optional)', ['@org' => $org]),
        '#upload_location' => 'public://' . $tree_pics_dir,
        '#upload_validators' => [
          'file_validate_extensions' => ['jpeg jpg'],
        ],
        '#description' => t('Please upload a photo of the species in either '
          . '.jpeg or .jpg format'),
      ];
      if (db_table_exists('treepictures_metadata')) {
        $form['tree_pictures']["{$org}_url"] = [
          '#type' => 'textfield',
          '#title' => t('@org Picture source URL:', ['@org' => $org]),
          '#states' => [
            'invisible' => [
              ":input[name=\"tree_pictures[{$org}][fid]\"]" => ['value' => 0],
            ],
          ],
        ];
        $form['tree_pictures']["{$org}_attribution"] = [
          '#type' => 'textfield',
          '#title' => t('@org Picture Attribution:', ['@org' => $org]),
          '#states' => [
            'invisible' => [
              ":input[name=\"tree_pictures[{$org}][fid]\"]" => ['value' => 0],
            ],
          ],
        ];
        $form['tree_pictures']["{$org}_license"] = [
          '#type' => 'textfield',
          '#title' => t('@org Picture License:', ['@org' => $org]),
          '#states' => [
            'invisible' => [
              ":input[name=\"tree_pictures[{$org}][fid]\"]" => ['value' => 0],
            ],
          ],
        ];
      }
    }
  }
  tpps_form_add_buttons(['form' => &$form, 'page' => 'summary']);
  return $form;
}
