<?php

/**
 * @file
 * Page 4 Genotype related functions.
 */

/**
 * Creates fields describing the genotype data for the submission.
 *
 * @param array $form
 *   The form to be populated.
 * @param array $form_state
 *   The state of the form to be populated.
 * @param array $values
 *   The form_state values of the form to be populated.
 * @param string $id
 *   The id of the organism fieldset being populated.
 *
 * @return array
 *   The populated form.
 */
function tpps_genotype(array &$form, array &$form_state, array $values, $id) {
  // [VS]
  $genotype_upload_location = 'public://' . variable_get('tpps_genotype_files_dir', 'tpps_genotype');
  $fields = array(
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">Genotype Information:</div>'),
    '#collapsible' => TRUE,
  );
  tpps_page_4_marker_info($fields, $form_state, $id);
  tpps_page_4_ref($fields, $form_state, $id);

  $marker_parents = [$id, 'genotype', 'marker-type'];
  $genotype_marker_type = array_keys(
    tpps_get_ajax_value($form_state, $marker_parents, [])
  );
  // Get 'Define SSRs/cpSSRs Type' field value to show correct fields
  // which visiblity depends on value of this field.
  $ssrs_cpssrs_value = tpps_get_ajax_value(
    $form_state, [$id, 'genotype', 'SSRs/cpSSRs'], 'SSRs'
  );

  $fields['files'] = [
    '#type' => 'fieldset',
    '#prefix' => "<div id='$id-genotype-files'>",
    '#suffix' => '</div>',
    '#weight' => 10,
  ];

  $genotyping_type_parents = [$id, 'genotype', 'files', 'genotyping-type'];
  $file_type_parents = [$id, 'genotype', 'files', 'file-type'];
  // Value is a string because mutiple values not allowed.
  $genotyping_type_check = tpps_get_ajax_value($form_state, $genotyping_type_parents);
  $file_type_value = tpps_get_ajax_value($form_state, $file_type_parents);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Note: Marker Type allows multiple values to be selected.
  if (in_array('SNPs', $genotype_marker_type)) {
    $is_step2_genotype = in_array(
      $form_state['saved_values'][TPPS_PAGE_2]['data_type'],
      [
        'Genotype x Environment',
        'Genotype x Phenotype x Environment',
        'Genotype x Phenotype',
      ]
    );
    $upload_snp_association = tpps_get_ajax_value(
      $form_state, [$id, 'genotype', 'files', 'upload_snp_association'], 'Yes'
    );
    if ($is_step2_genotype) {
      $fields['files']['upload_snp_association'] = [
        '#type' => 'select',
        '#title' => t('Would you like to upload a SNP association file?'),
        '#options' => [
          'Yes' => t('Yes'),
          'No' => t('No'),
        ],
        '#default_value' => $upload_snp_association,
        '#ajax' => [
          'callback' => 'tpps_genotype_files_callback',
          'wrapper' => "$id-genotype-files",
          'effect' => 'slide',
        ],
      ];
    }
    $fields['files']['genotyping-type'] = [
      '#type' => 'select',
      '#title' => t('Genotyping Type: *'),
      '#options' => [
        'Genotyping Assay' => t('Genotyping Assay'),
        'Genotyping' => t('Genotyping'),
      ],
      '#ajax' => [
        'callback' => 'tpps_genotype_files_callback',
        'wrapper' => "$id-genotype-files",
        'effect' => 'slide',
      ],
    ];

    // Genotype File Type.
    $fields['files']['file-type'] = [
      '#type' => 'select',
      '#title' => t('Genotyping file type: *'),
      '#options' => [
        'SNP Assay file and Assay design file'
          => t('SNP Assay file and Assay design file'),
        'VCF' => t('VCF'),
      ],
      '#ajax' => [
        'callback' => 'tpps_genotype_files_callback',
        'wrapper' => "$id-genotype-files",
        'effect' => 'slide',
      ],
      '#states' => [
        'visible' => [
          ':input[name="' . $id . '[genotype][files][genotyping-type]"]'
          => ['value' => 'Genotyping'],
        ],
      ],
    ];

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // SNP Assay File.
    $title = t('SNP Assay File');
    $file_field_name = 'snps-assay';
    $condition = (
      $genotyping_type_check == 'Genotyping Assay'
      || $file_type_value == 'SNP Assay file and Assay design file'
    );
    if ($condition) {
      if (empty(tpps_add_file_selector($form_state, $fields, $id, $title, ''))) {
        // Add file upload field if file selector wasn't checked.
        tpps_genotype_build_file_field($fields, [
          'form_state' => $form_state,
          'id' => $id,
          'file_field_name' => $file_field_name,
          'title' => $title,
          'description' => t('Please provide a spreadsheet with columns '
            . 'for the Plant ID of genotypes used in this study'
            . '<br />The format of this file is very important! '
            . '<br />The first column of your file should contain plant '
            . 'identifiers which match the plant identifiers you provided '
            . 'in your plant accession file, and all of the remaining '
            . 'columns should contain SNP data.'),
          'upload_location' => "$genotype_upload_location",
          'use_fid' => TRUE,
        ]);
      }
      else {
        // Add autocomplete field.
        $fields['files'][$file_field_name] = [
          '#type' => 'textfield',
          '#title' => t($title . ': please select an already existing '
            . 'spreadsheet with columns for the Plant ID of genotypes '
            . 'used in this study: *'),
          '#upload_location' => "$genotype_upload_location",
          '#autocomplete_path' => 'snp-assay-file/upload',
          '#description' => t("Please select an already existing spreadsheet "
            . "file containing SNP Genotype Assay data. The format of this "
            . "file is very important! The first column of your file should "
            . "contain plant identifiers which match the plant identifiers "
            . "you provided in your plant accession file, and all of the "
            . "remaining columns should contain SNP data."),
        ];
      }
    }
    else {
      tpps_build_disabled_file_field($fields, $file_field_name);
    }

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
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $id,
        'file_field_name' => $file_field_name,
        'title' => $title,
        'upload_location' => "$genotype_upload_location",
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
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $id,
        'file_field_name' => $file_field_name,
        'title' => $title,
        'upload_location' => "$genotype_upload_location",
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
      $fields['files'][$file_field_name] = array_merge(
        $fields['files'][$file_field_name],
        [
          'empty' => [
            '#default_value' => $values[$id]['genotype']['files'][$file_field_name]['empty'] ?? 'NA',
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
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $id,
        'file_field_name' => 'snps-pop-struct',
        // @todo [VS] Replace with 'required' with default value 'TRUE'.
        'optional' => TRUE,
        'title' => t('SNPs Population Structure File'),
        'upload_location' => "$genotype_upload_location",
      ]);
      // SNPs Kinship File.
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $id,
        'file_field_name' => 'snps-kinship',
        'optional' => TRUE,
        'title' => t('SNPs Kinship File'),
        'upload_location' => "$genotype_upload_location",
      ]);
    }
    else {
      $file_field_list = ['snps-association', 'snps-pop-struct', 'snps-kinship'];
      foreach ($file_field_list as $file_field_name) {
        tpps_build_disabled_file_field($fields, $file_field_name);
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (in_array('SSRs/cpSSRs', $genotype_marker_type)) {
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
      '#ajax' => [
        'callback' => 'tpps_genotype_files_callback',
        'wrapper' => "$id-genotype-files",
        'effect' => 'slide',
      ],
      '#default_value' => tpps_get_ajax_value($form_state,
        [$id, 'genotype', 'files', 'ploidy'], 'haploid'
      ),
    ];
    // SSRs.
    if ($ssrs_cpssrs_value != 'cpSSRs') {
      // 'SSRs' or 'Both SSRs and cpSSRs'.
      $file_field_name = 'ssrs';
      $title = t('SSRs Spreadsheet');
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $id,
        'file_field_name' => $file_field_name,
        'title' => $title,
        'upload_location' => "$genotype_upload_location",
        'description' => t('Please upload a spreadsheet containing your '
          . 'SSRs data. The format of this file is very important! TPPS will '
          . 'parse your file based on the ploidy you have selected above. '
          . 'For any ploidy, TPPS will assume that the first column of your '
          . 'file is the column that holds the Plant Identifier that matches '
          . 'your accession file.'),
        // Add extra text field for empty field value.
        'empty_field_value' => tpps_get_empty_field_value(
          $form_state, $id, $file_field_name
        ),
        'use_fid' => TRUE,
      ]);
      tpps_genotype_update_description($fields, [
        'id' => $id,
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
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $id,
        'file_field_name' => $file_field_name,
        'title' => $title,
        'upload_location' => "$genotype_upload_location",
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
          $form_state, $id, $file_field_name
        ),
        'use_fid' => TRUE,
      ]);
      tpps_genotype_update_description($fields, [
        'id' => $id,
        'form_state' => $form_state,
        'source_field_name' => 'ploidy',
        'target_field_name' => $file_field_name,
      ]);
    }
    else {
      tpps_build_disabled_file_field($fields, 'ssrs_extra');
    }
    // End of 'cpSSR' field.
  }
  else {
    $file_field_list = ['ssrs', 'ssrs_extra'];
    foreach ($file_field_list as $file_field_name) {
      tpps_build_disabled_file_field($fields, $file_field_name);
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (in_array('Other', $genotype_marker_type)) {
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
    tpps_genotype_build_file_field($fields, [
      'form_state' => $form_state,
      'id' => $id,
      'file_field_name' => $file_field_name,
      'title' => $title,
      'upload_location' => "$genotype_upload_location",
      'description' => $description,
      'empty_field_value' => tpps_get_empty_field_value(
        $form_state, $id, $file_field_name
      ),
    ]);

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Other Columns.
    $default_dynamic = !empty($values[$id]['genotype']['files']['other-columns']);
    $fields['files']['other']['dynamic'] = [
      '#type' => 'checkbox',
      '#title' => t('This file needs dynamic dropdown options for column data type specification'),
      '#ajax' => [
        'wrapper' => "edit-$id-genotype-files-other-ajax-wrapper",
        'callback' => 'tpps_page_4_file_dynamic',
        'effect' => 'slide',
      ],
      '#default_value' => $default_dynamic,
    ];
    $dynamic = tpps_get_ajax_value($form_state,
      [$id, 'genotype', 'files', 'other', 'dynamic'],
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
  }
  else {
    tpps_build_disabled_file_field($fields, 'other');
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Genotype VCF File.
  $title = t('Genotype VCF File');
  $file_field_name = 'vcf';
  if (
    $genotyping_type_check == 'Genotyping'
    && $file_type_value == 'VCF'
  ) {
    tpps_genotype_build_file_field($fields, [
      'form_state' => $form_state,
      'id' => $id,
      'file_field_name' => $file_field_name,
      'title' => $title,
      'upload_location' => "$genotype_upload_location",
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
        'id' => $id,
      ]);
    //}
  }
  else {
    tpps_build_disabled_file_field($fields, $file_field_name);
  }
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
    '#ajax' => [
      'callback' => 'tpps_genotype_files_callback',
      'wrapper' => "$id-genotype-files",
      'effect' => 'slide',
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // SNPs
  $fields['SNPs'] = [
    '#type' => 'fieldset',
    '#title' => t('<div class="fieldset-title">SNPs Information:</div>'),
    '#collapsible' => TRUE,
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
  $fields['SNPs']['GBS'] = array(
    '#type' => 'select',
    '#title' => t('GBS Type: *'),
    '#options' => array(
      0 => t('- Select -'),
      1 => t('RADSeq'),
      2 => t('ddRAD-Seq'),
      3 => t('NextRAD'),
      4 => t('RAPTURE'),
      5 => t('Other'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1'),
      ),
    ),
  );
  $fields['SNPs']['GBS-other'] = array(
    '#type' => 'textfield',
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][GBS]"]' => array('value' => '5'),
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1'),
      ),
    ),
  );

  $fields['SNPs']['targeted-capture'] = array(
    '#type' => 'select',
    '#title' => t('Targeted Capture Type: *'),
    '#options' => [
      0 => t('- Select -'),
      1 => t('Exome Capture'),
      2 => t('Other'),
    ],
    '#states' => array(
      'visible' => array(
        ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2'),
      ),
    ),
  );

  // [VS]
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
    '#ajax' => [
      'callback' => 'tpps_genotype_files_callback',
      'wrapper' => "$id-genotype-files",
      'effect' => 'slide',
    ],
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
  $ref_genome_arr = array();
  $ref_genome_arr[0] = '- Select -';

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

  // Perform a database lookup as well using new query from Emily Grau (6/6/2023).
  $time_now = time();
  $time_expire_period = 3 * 24 * 60 * 60;
  $time_genome_query_results_time = cache_get('tpps_genome_query_results_time', 'tpps');
  if ($time_now > ($time_genome_query_results_time + $time_expire_period)) {
    chado_query("DROP TABLE IF EXISTS chado.tpps_ref_genomes;", []);
    chado_query("CREATE TABLE chado.tpps_ref_genomes AS (
      select distinct a.name, a.analysis_id, a.programversion, o.genus||' '||o.species as species from chado.analysis a
      join chado.analysisfeature af on a.analysis_id = af.analysis_id
      join chado.feature f on af.feature_id = f.feature_id
      join chado.organism o on f.organism_id = o.organism_id
      where f.type_id in (379,595,597,825,1245,422) AND a.name LIKE '% v%'
    )", []);
    cache_set('tpps_genome_query_results_time', $time_now, 'tpps');
  }
  $genome_query_results = chado_query("select * FROM chado.tpps_ref_genomes;", []);
  foreach ($genome_query_results as $genome_query_row) {
    $genome_query_row->name = str_ireplace(' genome', '', $genome_query_row->name);
    $genome_query_row->name = str_ireplace(' assembly', '', $genome_query_row->name);
    $existing_genomes[$genome_query_row->name] = $genome_query_row->name;
  }
  ksort($existing_genomes);
  $ref_genome_arr += $existing_genomes;

  $ref_genome_arr["url"] = 'I can provide a URL to the website of my reference file(s)';
  $ref_genome_arr["bio"] = 'I can provide a GenBank accession number (BioProject, WGS, TSA) and select assembly file(s) from a list';
  $ref_genome_arr["manual"] = 'I can upload my own reference genome file';
  $ref_genome_arr["manual2"] = 'I can upload my own reference transcriptome file';
  $ref_genome_arr["none"] = 'I am unable to provide a reference assembly';

  $fields['ref-genome'] = [
    '#type' => 'select',
    '#title' => t('Reference Assembly used: *'),
    '#options' => $ref_genome_arr,
  ];

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

/**
 * Adds checkbox to select existing file or upload new one.
 *
 * @return mixed
 *   Returns value of file field.
 */
function tpps_add_dropdown_file_selector(array &$fields, array $meta) {
  extract($meta);
  $hostname = tpps_get_hostname();
  $fields['files'][$file_field_name . '_file-location'] = [
    '#type' => 'select',
    '#title' => t('VCF File Location'),
    '#options' => [
      'local' => t('My VCF File is stored locally'),
      'remote' => t('My VCF File is stored at @hostname',
        ['@hostname' => $hostname]),
    ],
    '#weight' => 90,
  ];
  $fields['files'][$file_field_name]['#states'] = [
    'visible' => [
      ':input[name="' . $id . '[genotype][files]['
        . $file_field_name . '_file-location]"]' => ['value' => 'local'],
    ],
  ];
  $fields['files'][$file_field_name]['#weight'] = 100;
  $fields['files']['local_' . $file_field_name] = [
    '#type' => 'textfield',
    '#title' => t('Path to VCF File at @hostname: *',
      ['@hostname' => $hostname]
    ),
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][files]['
          . $file_field_name . '_file-location]"]' => ['value' => 'remote'],
      ],
    ],
    '#description' => t('Please provide the full path to your vcf file '
      . 'stored on @hostname', ['@hostname' => $hostname]
    ),
    '#weight' => 100,
  ];
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Helper functions.

/**
 * Adds checkbox to select existing file or upload new one.
 *
 * @param array $form_state
 *   Submitted Form API values.
 * @param array $fields
 *   Form API array.
 * @param string $id
 *   Organism Id. Example: 'organism-1'.
 * @param string $title
 *   Human readable name of the related file upload field
 *   (including trailing word 'File').
 * @param string $key
 *   Unique key to avoid field's duplication.
 *
 * @return mixed
 *   Returns value of file field.
 */
function tpps_add_file_selector(array $form_state, array &$fields, $id, $title, $key) {
  $name = ($key ? $key . '_' : '') . 'file-selector';
  $fields['files'][$name] = [
    '#type' => 'checkbox',
    '#title' => t('Reference Existing @title', ['@title' => $title]),
    '#ajax' => [
      'callback' => 'tpps_genotype_files_type_change_callback',
      'wrapper' => "$id-genotype-files",
      'effect' => 'slide',
    ],
  ];
  return tpps_get_ajax_value($form_state, [$id, 'genotype', 'files', $name]);
}

/**
 * Builds Genotype file field.
 *
 * WARNING: Genotype ONLY!
 *
 * @param array $fields
 *   Drupal Form API array with fields.
 * @param array $meta
 *   File field metadata. Example:
 *
 *   tpps_genotype_build_file_field($fields, [
 *     'form_state' => $form_state,
 *     'id' => $id,
 *     'file_field_name' => $file_field_name,
 *     'optional' => TRUE,
 *     'title' => $title,
 *     'upload_location' => "$genotype_upload_location",
 *     'description' => $description,
 *     'extensions' => $extensions, // Default: ['csv tsv xlsx']
 *     'states' => $states, // Default is ''.
 *     'empty_field_value' => 'NA',
 *     // Element 'extra_elements' allow to add any not expected form elements.
 *     'extra_elements' => [],
 *     'use_fid' => FALSE, // Default is FALSE. See below.
 *   ]);
 *
 *   List of fields which used 'fid'-related code:
 *     'snps-assay',
 *     'ssrs',
 *     'ssrs_extra'.
 *
 *   List of fields which not used 'fid'-related code:
 *     'assay-design',
 *     'snps-pop-struct',
 *     'snps-association',
 *     'snps-kinship',
 *     'indels', // was removed.
 *     'vcf',
 *     'other'.
 */
function tpps_genotype_build_file_field(array &$fields, array $meta) {
  extract($meta);
  // When enabled field's machine name will be shown in field's decription.
  $debug_mode = FALSE;
  $fields['files'][$file_field_name] = [
    '#type' => 'managed_file',
    '#title' => $title . (!empty($optional) ? ':' : ': *'),
    '#upload_location' => $upload_location,
    '#upload_validators' => [
      'file_validate_extensions' => $extensions ?? ['csv tsv'],
    ],
    '#description' => ($description ?? '')
    . ($debug_mode ? '<br/>Field name: <strong>' . $file_field_name . '</strong>' : ''),
    '#tree' => TRUE,
    '#states' => $states ?? '',
  ];
  // Add extra text field for empty field value. Default is FALSE.
  if (!empty($empty_field_value)) {
    $fields['files'][$file_field_name]['empty'] = ['#default_value' => $empty_field_value];
  }
  if (!empty($extra_elements)) {
    $fields['files'][$file_field_name] = array_merge(
      $fields['files'][$file_field_name], $extra_elements
    );
  }

  // Some fields have used this piece of code before.
  // To use: $meta['use_fid'] = TRUE; Default is FALSE.
  // @TODO [VS] Check if this code could be removed. It's not used on form
  // generation and when validation failed.
  if (!empty($use_fid)) {
    if (isset($fields['files'][$file_field_name]['#value']['fid'])) {
      $fields['files'][$file_field_name]['#default_value']
        = $fields['files'][$file_field_name]['#value']['fid'];
    }
  }

  // Most of fields have used this code so only 2 must be excluded.
  if (!in_array($file_field_name, ['snps-association', 'other'])) {
    // Field 'snps-association' excluded because it didn't have this code.
    if (isset($fields['files'][$file_field_name]['#value'])) {
      $fields['files'][$file_field_name]['#default_value']
        = $fields['files'][$file_field_name]['#value'];
    }
    // @TODO Check file size not zero.
    if (
      !empty($fields['files'][$file_field_name]['#default_value'])
      && ($file = file_load($fields['files'][$file_field_name]['#default_value']))
    ) {
      // Stop using the file so it can be deleted if the user clicks 'remove'.
      if (variable_get('tpps_genotype_file_usage_delete', TRUE)) {
        // Study Id is a number in 'TGDRXXX'.
        $study_id = substr($form_state['accession'], 4);
        file_usage_delete($file, 'tpps', 'tpps_project', $study_id);
      }
    }
  }
}

/**
 * Generates disabled managed field.
 *
 * When file already was uploaded.
 *
 * @param array $fields
 *   Drupal Form API array with fields.
 * @param string $file_field_name
 *   Name of the managed file field.
 */
function tpps_build_disabled_file_field(array &$fields, $file_field_name) {
  $fields['files'][$file_field_name] = [
    '#type' => 'managed_file',
    '#tree' => TRUE,
    '#access' => FALSE,
  ];
}


/**
 * Updates description of related file field on 'Ploidy' field value change.
 *
 * @param array $fields
 *   Drupal Form API array with Genotype form.
 * @param array $meta
 *   Metadata for function. Associative array with keys:
 *     'id', 'form_state', 'source_field_name', 'target_field_name'.
 *   Example:
 *   tpps_genotype_update_description($fields, [
 *     'id' => $id, // Organism Number.
 *     // Drupal Form API $form_state.
 *     'form_state' => $form_state,
 *     // Source field name. Usually selectbox.
 *     'source_field_name' => 'ploidy',
 *     // Genotype file field name which must be updated.
 *     'file_field_name' => 'ssrs',
 *   ]); //.
 */
function tpps_genotype_update_description(array &$fields, array $meta) {
  $ploidy = tpps_get_ajax_value($meta['form_state'], [
    $meta['id'],
    'genotype',
    'files',
    $meta['source_field_name'],
  ]);

  switch ($ploidy) {
    case 'Haploid':
      $fields['files'][$meta['target_field_name']]['#description']
        .= '<br/>For haploid, TPPS assumes that each remaining column in the '
        . 'spreadsheet is a marker.';
      break;

    case 'Diploid':
      $fields['files'][$meta['target_field_name']]['#description']
        .= '<br />For diploid, TPPS will assume that pairs of columns together '
        . 'are describing an individual marker, so the second and third '
        . 'columns would be the first marker, the fourth and fifth columns '
        . 'would be the second marker, etc.';
      break;

    case 'Polyploid':
      $fields['files'][$meta['target_field_name']]['#description']
        .= '<br />For polyploid, TPPS will read columns until it arrives at a '
        . 'non-empty column with a different name from the last.';
      break;

    default:
      break;
  }
}

/**
 * Gets value of the empty field.
 *
 * @param array $form_state
 *   Drupal Form API state.
 * @param int $id
 *   Organism Id.
 * @param string $file_field_name
 *   Machine file upload field name.
 *
 * @return string
 *   Returns value of empty fields in file. Usually it will be 'NA'.
 */
function tpps_get_empty_field_value(array $form_state, $id, $file_field_name) {
  return $form_state['saved_values'][TPPS_PAGE_4]["organism-$id"]['genotype']['files'][$file_field_name]['other'] ?? 'NA';
}
