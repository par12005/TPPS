<?php

/**
 * @file
 * Page 4 Genotype related functions.
 */

/**
 * Creates fields describing the genotype data for the submission.
 *
 * @param array $chest
 *   Accosiative array with metadata.
 *
 *   Per study data:
 *   'form' array
 *     Reference to the Drupal Form Array.
 *   'form_state' array
 *     Drupal Form State Array.
 *   'page1_values' => &$form_state['saved_values'][TPPS_PAGE_1] ?? [],
 *   'page2_values' => &$form_state['saved_values'][TPPS_PAGE_2] ?? [],
 *   'page3_values' => &$form_state['saved_values'][TPPS_PAGE_3] ?? [],
 *   'page4_values' => &$form_state['saved_values'][TPPS_PAGE_4] ?? [],
 *
 *   Per organism data:
 *   'organism_id' int 1
 *     Organism number (or Id). E.g., 1. See 'organism_number'.
 *   'type' string
 *     Machine name of the data type. E.g., 'genotype'.
 *   'type_name' string
 *      Human readable data type name. E.g., 'Genotype'.
 *
 * @return array
 *   The populated form.
 *
 * @TODO Do not return because we update $form.
 */
function tpps_genotype_subform(array $chest) {
  if (!isset($chest['organism_id']) || !isset($chest['type'])) {
    return [];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Map from the $chest.
  $form = &$chest['form'];
  $form_state = &$chest['form_state'];
  $i = $chest['organism_id'];
  $organism_count = $chest['organism_count'] ?? 1;
  $type = $chest['type'] ?? '';
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Get "treasure" from the $chest.
  $page4_values = &$form_state['saved_values'][TPPS_PAGE_4] ?? [];
  $organism_count = $chest['page1_values']['organism']['number'];
  $organism_name = 'organism-' . $i;
  $genotype_dir = variable_get(
    'tpps_' . $chest['type'] . '_files_dir',
    'tpps_' . $chest['type']
  );
  tpps_add_css_js('page_4_genotype', $form);

  $fields = &$form[$organism_name][$chest['type']];
  $fields = [
    '#type' => 'fieldset',
    '#title' => t('Genotype Information:'),
    '#collapsible' => TRUE,
    '#weight' => 0,
  ];

  $marker_parents = [$organism_name, 'genotype', 'marker-type'];
  $genotype_marker_type = array_keys(
    tpps_get_ajax_value($form_state, $marker_parents, [])
  );
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Only for 1st organism.
  if ($i == 1) {
    tpps_form_add_yesno_field(array_merge($chest,
      [
        'parents' => [$organism_name, $type],
        'field_name' => 'are_genotype_markers_identical',
        '#title' => t('Are your genotype markers identical accross species?'),
        '#default_value' => (($organism_count == 1) ? 'yes' : 0),
      ]
    ));
    // Оnly for non-first questions. Next 3 questions are dependent on 1st one.
    $chest['#states'] = [
      'invisible' => [
        ':input[name="organism-1[genotype][are_genotype_markers_identical]"]' => ['value' => 0],
      ],
    ];
  }
  $marker_type_field_list = [
    // Field HTML name -> Marker Name.
    'does_study_include_snp_data' => 'SNPs',
    'does_study_include_ssr_cpssr_data' => 'SSRs/cpSSRs',
    'does_study_include_other_data' => 'Other',
  ];
  $form['#attached']['js'][] = [
    'type' => 'setting',
    'data' => ['tpps' => ['markerTypeFieldList' => $marker_type_field_list]],
    'scope' => 'footer',
  ];
  foreach ($marker_type_field_list as $field_name => $marker_name) {
    $default_value = (
      in_array($marker_name, $genotype_marker_type)
      ? 'yes' : (count($genotype_marker_type) ? 'no' : 0)
    );
    tpps_form_add_yesno_field(array_merge($chest,
      [
        'parents' => [$organism_name, $type],
        'field_name' => $field_name,
        '#title' => t('Does your study include @marker_name data?',
          [
            '@marker_name' => ($marker_name == 'Other'
            // Just a quick fix :(.
            ? strtolower($marker_name) : str_replace('s', '', $marker_name)),
          ]
        ),
        '#default_value' => $default_value,
      ]
    ));
  }
  unset($chest['parents']);
  unset($chest['#states']);

  // @TODO Hide not genotype information fieldset but organism fieldset.
  //'#states' => [
  //  'visible' => [
  //    ':input[name="are_genotype_markers_identical"]' => ['value' => 'no'],
  //  ],
  //],
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  tpps_page_4_marker_info($fields, $form_state, $organism_name);
  tpps_page_4_ref($fields, $form_state, $organism_name);

  // Get 'Define SSRs/cpSSRs Type' field value to show correct fields
  // which visiblity depends on value of this field.
  $ssrs_cpssrs_value = tpps_get_ajax_value(
    $form_state, [$organism_name, 'genotype', 'SSRs/cpSSRs'], 'SSRs'
  );

  $fields['files'] = [
    '#type' => 'fieldset',
    '#prefix' => "<div id='$organism_name-genotype-files'>",
    '#suffix' => '</div>',
    '#weight' => 10,
  ];

  $genotyping_type_parents = [
    $organism_name, 'genotype', 'SNPs', 'genotyping-type'
  ];
  $file_type_parents = [$organism_name, 'genotype', 'files', 'file-type'];
  // Value is a string because mutiple values not allowed.
  $genotyping_type_check = tpps_get_ajax_value($form_state, $genotyping_type_parents);
  $file_type_value = tpps_get_ajax_value($form_state, $file_type_parents);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Note: Marker Type allows multiple values to be selected.
  //if (in_array('SNPs', $genotype_marker_type)) {
  $upload_snp_association = tpps_get_ajax_value(
    $form_state, [$organism_name, 'genotype', 'files', 'upload_snp_association'], 'Yes'
  );
  if (tpps_is_genotype_data_type($form_state)) {
    $fields['files']['upload_snp_association'] = [
      '#type' => 'select',
      '#title' => t('Would you like to upload a SNP association file?'),
      '#options' => [
        'Yes' => t('Yes'),
        'No' => t('No'),
      ],
      '#default_value' => $upload_snp_association,
      // @TODO Use Drupal Form States.
      //'#ajax' => [
      //  'callback' => 'tpps_genotype_files_callback',
      //  'wrapper' => "$organism_name-genotype-files",
      //  'effect' => 'slide',
      //],
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Genotyping Type.
  $fields['files']['genotyping-type'] = [
    '#type' => 'select',
    '#title' => t('Genotyping Type: *'),
    '#options' => [
      'Genotyping Assay' => t('Genotyping Assay'),
      'Genotyping' => t('Genotyping'),
    ],
  ];
  tpps_form_relocate_field([
    'form' => &$fields,
    'current_parents' => ['files'],
    'field_name' => 'genotyping-type',
    'new_parents' => ['SNPs'],
    '#parents' => [$organism_name, 'genotype', 'files'],
    '#name' => 'organism-1[genotype][files][genotyping-type]',
  ]);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Genotype File Type.
  $fields['files']['file-type'] = [
    '#type' => 'select',
    '#title' => t('Genotyping file type: *'),
    '#options' => [
      'SNP Assay file and Assay design file' => t('SNP Assay file and Assay design file'),
      'VCF' => t('VCF'),
    ],
    '#states' => [
      'visible' => [
        ':input[name="' . $organism_name . '[genotype][files][genotyping-type]"]'
          => ['value' => 'Genotyping'],
      ],
    ],
  ];
  tpps_form_relocate_field([
    'form' => &$fields,
    'current_parents' => ['files'],
    'field_name' => 'file-type',
    'new_parents' => ['SNPs'],
    '#parents' => [$organism_name, 'genotype', 'files'],
    '#name' => 'organism-1[genotype][files][file-type]',
  ]);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // SNP Assay File.
  $title = t('SNP Assay File');
  $file_field_name = 'snps-assay';
  $states = [
    'visible' => [
      [
        ':input[name="' . $organism_name . '[genotype][genotyping-type]"]'
        => ['value' => 'Genotyping Assay'],
      ],
      'or',
      [
        ':input[name="' . $organism_name . '[genotype][files][file-type]"]'
        => ['value' => 'SNP Assay file and Assay design file'],
      ],
    ],
  ];

  // File upload field.
  tpps_form_build_file_field(array_merge($chest, [
    'parents' => [$organism_name, 'genotype', 'files'],
    'field_name' => $file_field_name,
    'title' => $title,
    'organism_name' => $organism_name,
    'type' => $chest['type'],
    // According to Meghan's mockup.
    // 'allow_file_reuse' => TRUE,
    'description' => t('Please provide a spreadsheet with columns '
      . 'for the Plant ID of genotypes used in this study'
      . '<br />The format of this file is very important! '
      . '<br />The first column of your file should contain plant '
      . 'identifiers which match the plant identifiers you provided '
      . 'in your plant accession file, and all of the remaining '
      . 'columns should contain SNP data.'),
    'use_fid' => TRUE,
    'states' => $states,
  ]));
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Assay Design File.
    $title = t('Assay Design File');
    $file_field_name = 'assay-design';
    $condition = (
      $genotyping_type_check == "Genotyping Assay"
      || $file_type_value == 'SNP Assay file and Assay design file'
    );
    if ($condition) {
      // Add file upload field.
      tpps_form_build_file_field([
        'form' => &$form,
        'form_state' => $form_state,
        'parents' => [$organism_name, 'genotype', 'files'],
        'field_name' => $file_field_name,
        'title' => $title,
        'organism_name' => $organism_name,
        'type' => $chest['type'],
      ]);
      $fields['files']['assay-citation'] = [
        '#type' => 'textfield',
        '#title' => t('Assay Design Citation (Optional):'),
        '#description' => t('If your assay design file is from a different '
          . 'paper, please include the citation for that paper here.'),
      ];
    }
    else {
      tpps_build_disabled_file_field($fields, $file_field_name);
    }

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // SNP Association File.
    if ($upload_snp_association == 'Yes') {
    //if ($genotyping_type_check == "Genotyping Assay") {
      $file_field_name = 'snps-association';
      $title = t('SNP Association File');
      tpps_form_build_file_field([
        'form' => &$form,
        'form_state' => $form_state,
        'parents' => [$organism_name, 'genotype', 'files'],
        'field_name' => $file_field_name,
        'title' => $title,
        'organism_name' => $organism_name,
        'type' => $chest['type'],
        'description' => t('Please upload a spreadsheet file containing '
          . 'SNPs Association data. When your file is uploaded, you will '
          . 'be shown a table with your column header names, several '
          . 'drop-downs, and the first few rows of your file. You will be '
          . 'asked to define the data type for each column, using the '
          . 'drop-downs provided to you. If a column data type does not '
          . 'fit any of the options in the drop-down menu, you may set that '
          . 'drop-down menu to "N/A". Your file must contain columns with '
          . 'the SNP ID, Scaffold, Position (formatted like "start:stop"), '
          . 'Allele (formatted like "major:minor"), Associated Trait Name '
          . '(must match a phenotype from the above section), and '
          . 'Confidence Value. Optionally, you can also specify a Gene ID '
          . '(which should match the gene reference) and '
          . 'a SNP Annotation (non synonymous, coding, etc).'),
        '#tree' => TRUE,
      ]);


      //dpm(
      //  tpps_array_get_value(
      //    $chest,
      //    ['page4_values', $organism_name, 'genotype', 'files', $file_field_name, 'empty'],
      //  'NA'
      //));
      return;


      $fields['files'][$file_field_name] = array_merge(
        $fields['files'][$file_field_name],
        [
          'empty' => [
            '#default_value' => tpps_array_get_value(
              $chest['page4_values'],
              [$organism_name, 'genotype', 'files', $file_field_name, 'empty'],
              'NA'
            ),
          ],
          'columns' => [
            '#description' => t('Please define which columns hold the '
              . 'required data: SNP ID, Scaffold, Position, Allele, '
              . 'Associated Trait, Confidence Value.'),
          ],
          'columns-options' => [
            '#type' => 'hidden',
            '#value' => [
              'N/A',
              'SNP ID',
              'Scaffold',
              'Position',
              'Allele',
              'Associated Trait',
              'Confidence Value',
              'Gene ID',
              'Annotation',
            ],
            'no-header' => [],
          ],
        ]
      );

      $fields['files']['snps-association-type'] = [
        '#type' => 'select',
        '#title' => t('Confidence Value Type: *'),
        '#options' => [
          0 => t('- Select -'),
          'P value' => t('P value'),
          'Genomic Inflation Factor (GIF)' => t('Genomic Inflation Factor (GIF)'),
          'P-adjusted (FDR) / Q value' => t('P-adjusted (FDR) / Q value'),
          'P-adjusted (FWE)' => t('P-adjusted (FWE)'),
          'P-adjusted (Bonferroni)' => t('P-adjusted (Bonferroni)'),
        ],
      ];

      $fields['files']['snps-association-tool'] = [
        '#type' => 'select',
        '#title' => t('Association Analysis Tool: *'),
        '#options' => [
          0 => t('- Select -'),
          'GEMMA' => t('GEMMA'),
          'EMMAX' => t('EMMAX'),
          'Plink' => t('Plink'),
          'Tassel' => t('Tassel'),
          'Sambada' => t('Sambada'),
          'Bayenv' => t('Bayenv'),
          'BayeScan' => t('BayeScan'),
          'LFMM' => t('LFMM'),
        ],
      ];

      // SNPs Population Structure File.
      tpps_form_build_file_field([
        'form' => &$form,
        'form_state' => $form_state,
        'parents' => [$organism_name, 'genotype', 'files'],
        'organism_name' => $organism_name,
        'type' => $chest['type'],
        'field_name' => 'snps-pop-struct',
        // @todo [VS] Replace with 'required' with default value 'TRUE'.
        'optional' => TRUE,
        'title' => t('SNPs Population Structure File'),
      ]);
      // SNPs Kinship File.
      tpps_form_build_file_field([
        'form' => &$form,
        'form_state' => $form_state,
        'parents' => [$organism_name, 'genotype', 'files'],
        'organism_name' => $organism_name,
        'type' => $chest['type'],
        'field_name' => 'snps-kinship',
        'optional' => TRUE,
        'title' => t('SNPs Kinship File'),
      ]);
    }
    else {
      $file_field_list = ['snps-association', 'snps-pop-struct', 'snps-kinship'];
      foreach ($file_field_list as $file_field_name) {
        tpps_build_disabled_file_field($fields, $file_field_name);
      }
    }
  //}
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //if (in_array('SSRs/cpSSRs', $genotype_marker_type)) {
    $fields['files']['ploidy'] = [
      '#type' => 'select',
      '#title' => t('SSR Ploidy: *'),
      '#options' => [
        'Haploid' => t('Haploid'),
        'Diploid' => t('Diploid'),
        'Polyploid' => t('Polyploid'),
      ],
      // Note:
      // SSRs / cpSSRs Spreadsheet fields are loaded via AJAX to have updated
      // description. See function tpps_genotype_update_description().
      // This could be done in browser on client side using JS
      // but for now it was left as is.
        // @TODO Use Drupal Form States.
      //'#ajax' => [
      //  'callback' => 'tpps_genotype_files_callback',
      //  'wrapper' => "$organism_name-genotype-files",
      //  'effect' => 'slide',
      //],
      '#default_value' => tpps_get_ajax_value($form_state,
        [$organism_name, 'genotype', 'files', 'ploidy'], 'haploid'
      ),
    ];
    // SSRs.
    if ($ssrs_cpssrs_value != 'cpSSRs') {
      // 'SSRs' or 'Both SSRs and cpSSRs'.
      $file_field_name = 'ssrs';
      $title = t('SSRs Spreadsheet');
      tpps_form_build_file_field([
        'form' => &$form,
        'form_state' => $form_state,
        'parents' => [$organism_name, 'genotype', 'files'],
        'field_name' => $file_field_name,
        'title' => $title,
        'organism_name' => $organism_name,
        'type' => $chest['type'],
        'description' => t('Please upload a spreadsheet containing your '
          . 'SSRs data. The format of this file is very important! TPPS will '
          . 'parse your file based on the ploidy you have selected above. '
          . 'For any ploidy, TPPS will assume that the first column of your '
          . 'file is the column that holds the Plant Identifier that matches '
          . 'your accession file.'),
        // Add extra text field for empty field value.
        'empty_field_value' => tpps_get_empty_field_value(
          $form_state, $organism_name, $file_field_name
        ),
        'use_fid' => TRUE,
      ]);
      tpps_genotype_update_description($fields, [
        'id' => $organism_name,
        'form_state' => $form_state,
        'source_field_name' => 'ploidy',
        'target_field_name' => $file_field_name,
      ]);
    }
    else {
      tpps_build_disabled_file_field($fields, 'ssrs');
    }
    // End of 'SSRs' field.
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // 'cpSSRs'.
    if ($ssrs_cpssrs_value != 'SSRs') {
      $file_field_name = 'ssrs_extra';
      $title = t('cpSSRs Spreadsheet');
      tpps_form_build_file_field([
        'field_name' => $file_field_name,
        'title' => $title,
        'form' => &$form,
        'form_state' => $form_state,
        'organism_name' => $organism_name,
        'type' => $chest['type'],
        // Note:
        // Difference from form 'ssrs' field: 'cpSSRs' (2nd line).
        'description' => t('Please upload a spreadsheet containing your '
          . 'cpSSRs data. The format of this file is very important! TPPS will '
          . 'parse your file based on the ploidy you have selected above. '
          . 'For any ploidy, TPPS will assume that the first column of your '
          . 'file is the column that holds the Plant Identifier that matches '
          . 'your accession file.'),
        // Add extra text field for empty field value.
        'empty_field_value' => tpps_get_empty_field_value(
          $form_state, $organism_name, $file_field_name
        ),
        'use_fid' => TRUE,
      ]);
      tpps_genotype_update_description($fields, [
        'id' => $organism_name,
        'form_state' => $form_state,
        'source_field_name' => 'ploidy',
        'target_field_name' => $file_field_name,
      ]);
    }
    else {
      tpps_build_disabled_file_field($fields, 'ssrs_extra');
    }
    // End of 'cpSSR' field.
    //
    //
    // @TODO Move disabled fields upper.
  //}
  //else {
  //  $file_field_list = ['ssrs', 'ssrs_extra'];
  //  foreach ($file_field_list as $file_field_name) {
  //    tpps_build_disabled_file_field($fields, $file_field_name);
  //  }
  //}

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //if (in_array('Other', $genotype_marker_type)) {
    $fields['other-marker'] = [
      '#type' => 'textfield',
      '#title' => t('Other marker type: *'),
    ];
    $title = t('Other spreadsheet: '
      . '<br />please provide a spreadsheet with columns for the Plant ID '
      . 'of genotypes used in this study');
    $file_field_name = 'other';
    $description = t('Please upload a spreadsheet file containing '
      . 'Genotype data. When your file is uploaded, you will be shown '
      . 'a table with your column header names, several drop-downs, '
      . 'and the first few rows of your file. You will be asked to define '
      . 'the data type for each column, using the drop-downs provided to you. '
      . 'If a column data type does not fit any of the options in the '
      . 'drop-down menu, you may set that drop-down menu to "N/A". '
      . 'Your file must contain one column with the Plant Identifier.');
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [$organism_name, 'genotype', 'files'],
      'field_name' => $file_field_name,
      'title' => $title,
      'organism_name' => $organism_name,
      'type' => $chest['type'],
      'description' => $description,
      'empty_field_value' => tpps_get_empty_field_value(
        $form_state, $organism_name, $file_field_name
      ),
    ]);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Other Columns.
    $default_dynamic = !empty($page4_values[$organism_name]['genotype']['files']['other-columns']);
    $fields['files']['other']['dynamic'] = [
      '#type' => 'checkbox',
      '#title' => t('This file needs dynamic dropdown options for column data type specification'),
      '#ajax' => [
        'wrapper' => "edit-$organism_name-genotype-files-other-ajax-wrapper",
        'callback' => 'tpps_page_4_file_dynamic',
        'effect' => 'slide',
      ],
      '#default_value' => $default_dynamic,
    ];
    $dynamic = tpps_get_ajax_value($form_state,
      [$organism_name, 'genotype', 'files', 'other', 'dynamic'],
      $default_dynamic,
      'other'
    );

    if ($dynamic) {
      $fields['files']['other']['columns'] = [
        '#description' => t('Please define which columns hold the required data: '
          . '<br />Plant Identifier, Genotype Data'
        ),
      ];
      $fields['files']['other']['columns-options'] = [
        '#type' => 'hidden',
        '#value' => ['Genotype Data', 'Plant Identifier', 'N/A'],
      ];
    }
    $fields['files']['other']['no-header'] = [];

    // @TODO Move disabled fields.
  //}
  //else {
  //  tpps_build_disabled_file_field($fields, 'other');
  //}

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Genotype VCF File.
  $title = t('Genotype VCF File');
  $file_field_name = 'vcf';
  //if (
  //  $genotyping_type_check == 'Genotyping'
  //  && $file_type_value == 'VCF'
  //) {
  tpps_form_build_file_field([
    'form' => &$form,
    'form_state' => $form_state,
    'parents' => [$organism_name, 'genotype', 'files'],
    'field_name' => $file_field_name,
    'title' => $title,
    'organism_name' => $organism_name,
    'type' => $chest['type'],
    'description' => '',
    'extensions' => ['gz tar zip'],
  ]);

    // @todo This field must be shown for admins/curators only but condition
    // didn't work correctly and was commented out.
    //if (
    //  isset($form_state['tpps_type'])
    //  && $form_state['tpps_type'] == 'tppsc'
    //) {
      tpps_add_dropdown_file_selector($fields, [
        'form_state' => $form_state,
        'file_field_name' => $file_field_name,
        'id' => $organism_name,
      ]);
  //}
  //
  //
  // @TODO Move upper.
  //}
  //else {
  //  tpps_build_disabled_file_field($fields, $file_field_name);
  //}

  return $fields;
}

/**
 * Creates fields describing the genotype markers used in the submission.
 *
 * @param array $fields
 *   The form element being populated.
 * @param array $form_state
 *   Drupal Form API array.
 * @param string $id
 *   The id of the organism fieldset being populated.
 */
function tpps_page_4_marker_info(array &$fields, array $form_state, $id) {
  $fields['marker-type'] = [
    '#type' => 'select',
    '#multiple' => TRUE,
    '#title' => t('Marker Type: *'),
    '#options' => [
      'SNPs' => t('SNPs'),
      'SSRs/cpSSRs' => t('SSRs/cpSSRs'),
      'Other' => t('Other'),
    ],
    // @TODO Use #states.
    //'#ajax' => [
    //  'callback' => 'tpps_genotype_files_callback',
    //  'wrapper' => "$id-genotype-files",
    //  'effect' => 'slide',
    //],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // SNPs
  $fields['SNPs'] = [
    '#type' => 'fieldset',
    '#title' => t('SNPs Information:'),
    '#collapsible' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][marker-type]"]' => ['value' => 'SNPs'],
      ],
    ],

  ];
  $fields['SNPs']['genotyping-design'] = [
    '#type' => 'select',
    '#title' => t('Define Experimental Design: *'),
    '#options' => [
      0 => t('- Select -'),
      1 => t('GBS'),
      2 => t('Targeted Capture'),
      3 => t('Whole Genome Resequencing'),
      4 => t('RNA-Seq'),
      5 => t('Genotyping Array'),
    ],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // GBS Type.
  $fields['SNPs']['GBS'] = [
    '#type' => 'select',
    '#title' => t('GBS Type: *'),
    '#options' => [
      0 => t('- Select -'),
      1 => t('RADSeq'),
      2 => t('ddRAD-Seq'),
      3 => t('NextRAD'),
      4 => t('RAPTURE'),
      5 => t('Other'),
    ],
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => ['value' => '1'],
      ],
    ],
  ];
  $fields['SNPs']['GBS-other'] = [
    '#type' => 'textfield',
    '#title' => t('Other GBS Type: *'),
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][SNPs][GBS]"]' => ['value' => '5'],
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => ['value' => '1'],
      ],
    ],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Targeted Capture Type.
  $fields['SNPs']['targeted-capture'] = [
    '#type' => 'select',
    '#title' => t('Targeted Capture Type: *'),
    '#options' => [
      0 => t('- Select -'),
      1 => t('Exome Capture'),
      2 => t('Other'),
    ],
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => ['value' => '2'],
      ],
    ],
  ];

  $fields['SNPs']['targeted-capture-other'] = [
    '#type' => 'textfield',
    '#title' => t('Other Targeted Capture: *'),
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][SNPs][targeted-capture]"]' => ['value' => '2'],
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => ['value' => '2'],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Field 'Define SSRs/cpSSRs Type'.
  // @TODO Minor. Better to rename field to avoid '/' in name
  // and make it more meaningful.
  $fields['SSRs/cpSSRs'] = [
    '#type' => 'select',
    '#title' => t('Define SSRs/cpSSRs Type: *'),
    '#options' => [
      'SSRs' => t('SSRs'), // Original from Peter
      'cpSSRs' => t('cpSSRs'), // Original from Peter
      'Both SSRs and cpSSRs' => t('Both SSRs and cpSSRs'),
    ],
    // Fields 'SSRs' and 'cpSSRs' are switched good on already loaded page
    // but when page loaded first time or changed Ploidy (which updates
    // form using AJAX) then both fields are shown which is not correct.
    // #TODO use #states.
    //'#ajax' => [
    //  'callback' => 'tpps_genotype_files_callback',
    //  'wrapper' => "$id-genotype-files",
    //  'effect' => 'slide',
    //],
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][marker-type]"]'
        => ['value' => 'SSRs/cpSSRs'],
      ],
    ],
  ];
}

/**
 * Creates fields for the user to specify a reference genome.
 *
 * @param array $fields
 *   The form element being populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @global stdClass $user
 *   The user accessing the form.
 */
function tpps_page_4_ref(array &$fields, array &$form_state, $id) {
  global $user;
  $uid = $user->uid;

  $options = array(
    'key' => 'filename',
    'recurse' => FALSE,
  );

  $genome_dir = variable_get('tpps_local_genome_dir', NULL);

  if ($genome_dir) {
    $existing_genomes = array();
    $results = file_scan_directory($genome_dir, '/^([A-Z][a-z]{3})$/', $options);
    $code_cvterm = tpps_load_cvterm('organism 4 letter code')->cvterm_id;
    foreach ($results as $key => $value) {
      $org_id_query = chado_select_record('organismprop', array('organism_id'), array(
        'value' => $key,
        'type_id' => $code_cvterm,
      ));

      if (!empty($org_id_query)) {
        $org_query = chado_select_record('organism', array('genus', 'species'), array(
          'organism_id' => current($org_id_query)->organism_id,
        ));
        $result = current($org_query);

        $versions = file_scan_directory("$genome_dir/$key", '/^v([0-9]|.)+$/', $options);
        foreach ($versions as $item) {
          $opt_string = $result->genus . " " . $result->species . " " . $item->filename;
          $existing_genomes[$opt_string] = $opt_string;
        }
      }
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Field 'Reference Assembly'.
  // Perform a database lookup as well using new query from Emily Grau (6/6/2023).
  // @TODO Move this to constant or (better) to settings page.
  $time_expire_period = 3 * 24 * 60 * 60;
  $time_genome_query_results_time = variable_get('tpps_genome_query_results_time', 0);
  if (REQUEST_TIME > ($time_genome_query_results_time + $time_expire_period)) {
    chado_query("DROP TABLE IF EXISTS chado.tpps_ref_genomes;", []);
    chado_query("CREATE TABLE chado.tpps_ref_genomes AS (
      select distinct a.name, a.analysis_id, a.programversion, o.genus||' '||o.species as species from chado.analysis a
      join chado.analysisfeature af on a.analysis_id = af.analysis_id
      join chado.feature f on af.feature_id = f.feature_id
      join chado.organism o on f.organism_id = o.organism_id
      where f.type_id in (379,595,597,825,1245) AND a.name LIKE '% v%'
    )", []);
    variable_set('tpps_genome_query_results_time', REQUEST_TIME);
  }
  $genome_query_results = chado_query("select * FROM chado.tpps_ref_genomes;", []);
  foreach ($genome_query_results as $genome_query_row) {
    $genome_query_row->name = str_ireplace(' genome', '', $genome_query_row->name);
    $genome_query_row->name = str_ireplace(' assembly', '', $genome_query_row->name);
    $existing_genomes[$genome_query_row->name] = $genome_query_row->name;
  }
  ksort($existing_genomes);
  $ref_genome_arr = array_merge(['0' => '- Select -'], $existing_genomes, [
    // @todo Use t() for option's names. Check if they are used by other code.
    "url" => 'I can provide a URL to the website of my reference file(s)',
    "bio" => 'I can provide a GenBank accession number (BioProject, WGS, TSA) '
      . 'and select assembly file(s) from a list',
    "manual" => 'I can upload my own reference genome file',
    "manual2" => 'I can upload my own reference transcriptome file',
    "none" => 'I am unable to provide a reference assembly',
  ]);
  $fields['ref-genome'] = [
    '#type' => 'select',
    '#title' => t('Reference Assembly used: *'),
    '#options' => $ref_genome_arr,
  ];
  tpps_form_relocate_field([
    'form' => &$fields,
    'current_parents' => [],
    'field_name' => 'ref-genome',
    'new_parents' => ['SNPs'],
    '#parents' => [$id, 'genotype'],
  ]);
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  require_once drupal_get_path('module', 'tripal') . '/includes/tripal.importer.inc';
  $class = 'EutilsImporter';
  tripal_load_include_importer_class($class);
  $eutils = tripal_get_importer_form(array(), $form_state, $class);
  $eutils['#type'] = 'fieldset';
  $eutils['#title'] = 'Tripal Eutils BioProject Loader';
  $eutils['#states'] = [
    'visible' => [
      ':input[name="' . $id . '[genotype][ref-genome]"]' => ['value' => 'bio'],
    ],
  ];
  $eutils['accession']['#description'] = t('Valid examples: 12384, 394253, 66853, PRJNA185471');
  $eutils['db'] = array(
    '#type' => 'hidden',
    '#value' => 'bioproject',
  );
  unset($eutils['options']);
  $eutils['options']['linked_records'] = array(
    '#type' => 'hidden',
    '#value' => 1,
  );
  $eutils['callback']['#ajax'] = array(
    'callback' => 'tpps_ajax_bioproject_callback',
    'wrapper' => "$id-tripal-eutils",
    'effect' => 'slide',
  );
  $eutils['#prefix'] = "<div id=\"$id-tripal-eutils\">";
  $eutils['#suffix'] = '</div>';

  if (!empty($form_state['values'][$id]['genotype']['tripal_eutils'])) {
    $eutils_vals = $form_state['values'][$id]['genotype']['tripal_eutils'];
    if (!empty($eutils_vals['accession']) and !empty($eutils_vals['db'])) {
      $connection = new \EUtils();
      try {
        $connection->setPreview(TRUE);
        $eutils['data'] = $connection->get($eutils_vals['db'], $eutils_vals['accession']);
        foreach ($_SESSION['messages']['status'] as $key => $message) {
          if ($message == '<pre>biosample</pre>') {
            unset($_SESSION['messages']['status'][$key]);
            if (empty($_SESSION['messages']['status'])) {
              unset($_SESSION['messages']['status']);
            }
            break;
          }
        }
      }
      catch (\Exception $e) {
        tripal_set_message($e->getMessage(), TRIPAL_ERROR);
      }
    }
  }
  unset($eutils['button']);
  unset($eutils['instructions']);
  $fields['tripal_eutils'] = $eutils;

  $class = 'FASTAImporter';
  tripal_load_include_importer_class($class);
  $tripal_upload_location = "public://tripal/users/$uid";

  $fasta = tripal_get_importer_form(array(), $form_state, $class);
  $fasta['#type'] = 'fieldset';
  $fasta['#title'] = 'Tripal FASTA Loader';
  $fasta['#states'] = array(
    'visible' => array(
    array(
      [':input[name="' . $id . '[genotype][ref-genome]"]' => ['value' => 'url']],
      'or',
      [':input[name="' . $id . '[genotype][ref-genome]"]' => ['value' => 'manual']],
      'or',
      [':input[name="' . $id . '[genotype][ref-genome]"]' => ['value' => 'manual2']],
    ),
    ),
  );

  unset($fasta['file']['file_local']);
  unset($fasta['organism_id']);
  unset($fasta['method']);
  unset($fasta['match_type']);
  $db = $fasta['additional']['db'];
  unset($fasta['additional']);
  $fasta['db'] = $db;
  $fasta['db']['#collapsible'] = TRUE;
  unset($fasta['button']);

  $upload = [
    '#type' => 'managed_file',
    '#title' => '',
    '#description' => t('Remember to click the "Upload" button below to send '
      . 'your file to the server. <br />This interface is capable of uploading '
      . 'very large files. <br />If you are disconnected you can return, '
      . 'reload the file and it will resume where it left off. <br />Once the '
      . 'file is uploaded the "Upload Progress" will indicate "Complete". '
      . '<br />If the file is already present on the server then the status '
      . 'will quickly update to "Complete".'),
    '#upload_validators' => [
      'file_validate_extensions' => [implode(' ', $class::$file_types)],
    ],
    '#upload_location' => $tripal_upload_location,
  ];

  $fasta['file']['file_upload'] = $upload;
  $fasta['analysis_id']['#required'] = $fasta['seqtype']['#required'] = FALSE;
  $fasta['file']['file_upload']['#states']
    = $fasta['file']['file_upload_existing']['#states'] = [
      'visible' => [
        [
          [':input[name="' . $id . '[genotype][ref-genome]"]' => ['value' => 'manual']],
          'or',
          [':input[name="' . $id . '[genotype][ref-genome]"]' => ['value' => 'manual2']],
        ],
      ],
    ];
  $fasta['file']['file_remote']['#states'] = [
    'visible' => [
      ':input[name="' . $id . '[genotype][ref-genome]"]' => ['value' => 'url'],
    ],
  ];

  $fields['tripal_fasta'] = $fasta;
}
