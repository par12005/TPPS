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
 * @return array
 *   The populated form.
 *
 * @TODO Do not return but update $form.
 */
function tpps_genotype_subform(array $chest) {
  $form = isset($chest['form']) ? $form = &$chest['form'] : [];
  $form_state = isset($chest['form_state']) ? $form_state = &$chest['form_state'] : [];
  if (!isset($chest['i'])) {
    return [];
  }
  $i = $chest['i'];
  $organism_name = 'organism-' . $i;

  // Get necessary data.
  $page1_values = $form_state['saved_values'][TPPS_PAGE_1] ?? [];
  $page4_values = $form_state['saved_values'][TPPS_PAGE_4] ?? [];
  $organism_number = $page1_values['organism']['number'];
  $marker_parents = [$organism_name, 'genotype', 'marker-type'];
  $genotype_marker_type = array_keys(
    tpps_get_ajax_value($form_state, $marker_parents, [])
  );
  $genotype_dir = variable_get(
    'tpps_' . $chest['type'] . '_files_dir',
    'tpps_' . $chest['type']
  );
  $fields = &$form[$organism_name][$chest['type']];
  $fields = [
    '#type' => 'fieldset',
    '#title' => t('Genotype Information:'),
    '#collapsible' => TRUE,
    '#weight' => 0,
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $chest['parents'] = [$organism_name, $chest['type']];
  // Only for 1st organism.
  if ($i == 1) {
    tpps_form_add_yesno_field(array_merge($chest,
      [
        'field_name' => 'are_genotype_markers_identical',
        '#title' => t('Are your genotype markers identical accross species?'),
        '#default_value' => (($organism_number == 1) ? 'yes' : 0),
      ]
    ));
    // Next 3 questions must be shown/hidden only for 1st organism.
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
        'field_name' => $field_name,
        '#title' => t('Does your study include @marker_name data?',
          [
            '@marker_name' => ($marker_name == 'Other'
            // Just a fast workaround. Sorry :(.
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
    $organism_name, 'genotype', 'files', 'genotyping-type'
  ];
  $file_type_parents = [$organism_name, 'genotype', 'files', 'file-type'];
  // Value is a string because mutiple values not allowed.
  $genotyping_type_check = tpps_get_ajax_value($form_state, $genotyping_type_parents);
  $file_type_value = tpps_get_ajax_value($form_state, $file_type_parents);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Note: Marker Type allows multiple values to be selected.
  if (in_array('SNPs', $genotype_marker_type)) {
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
        '#ajax' => [
          'callback' => 'tpps_genotype_files_callback',
          'wrapper' => "$organism_name-genotype-files",
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
        'wrapper' => "$organism_name-genotype-files",
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
        'wrapper' => "$organism_name-genotype-files",
        'effect' => 'slide',
      ],
      '#states' => [
        'visible' => [
          ':input[name="' . $organism_name . '[genotype][files][genotyping-type]"]'
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
      if (empty(tpps_add_file_selector($form_state, $fields, $organism_name, $title, ''))) {
        // Add file upload field if file selector wasn't checked.
        tpps_genotype_build_file_field($fields, [
          'form_state' => $form_state,
          'id' => $organism_name,
          'file_field_name' => $file_field_name,
          'title' => $title,
          'description' => t('Please provide a spreadsheet with columns '
            . 'for the Plant ID of genotypes used in this study'
            . '<br />The format of this file is very important! '
            . '<br />The first column of your file should contain plant '
            . 'identifiers which match the plant identifiers you provided '
            . 'in your plant accession file, and all of the remaining '
            . 'columns should contain SNP data.'),
          'upload_location' => 'public://' . $genotype_dir,
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
          'upload_location' => 'public://' . $genotype_dir,
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
        'id' => $organism_name,
        'file_field_name' => $file_field_name,
        'title' => $title,
        'upload_location' => 'public://' . $genotype_dir,
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
        'id' => $organism_name,
        'file_field_name' => $file_field_name,
        'title' => $title,
        'upload_location' => 'public://' . $genotype_dir,
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
            '#default_value' => $page4_values[$organism_name]['genotype']['files'][$file_field_name]['empty'] ?? 'NA',
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
        'id' => $organism_name,
        'file_field_name' => 'snps-pop-struct',
        // @todo [VS] Replace with 'required' with default value 'TRUE'.
        'optional' => TRUE,
        'title' => t('SNPs Population Structure File'),
        'upload_location' => 'public://' . $genotype_dir,
      ]);
      // SNPs Kinship File.
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $organism_name,
        'file_field_name' => 'snps-kinship',
        'optional' => TRUE,
        'title' => t('SNPs Kinship File'),
        'upload_location' => 'public://' . $genotype_dir,
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
        'wrapper' => "$organism_name-genotype-files",
        'effect' => 'slide',
      ],
      '#default_value' => tpps_get_ajax_value($form_state,
        [$organism_name, 'genotype', 'files', 'ploidy'], 'haploid'
      ),
    ];
    // SSRs.
    if ($ssrs_cpssrs_value != 'cpSSRs') {
      // 'SSRs' or 'Both SSRs and cpSSRs'.
      $file_field_name = 'ssrs';
      $title = t('SSRs Spreadsheet');
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $organism_name,
        'file_field_name' => $file_field_name,
        'title' => $title,
        'upload_location' => 'public://' . $genotype_dir,
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
      tpps_genotype_build_file_field($fields, [
        'form_state' => $form_state,
        'id' => $organism_name,
        'file_field_name' => $file_field_name,
        'title' => $title,
        'upload_location' => 'public://' . $genotype_dir,
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
      'id' => $organism_name,
      'file_field_name' => $file_field_name,
      'title' => $title,
      'upload_location' => 'public://' . $genotype_dir,
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
      'id' => $organism_name,
      'file_field_name' => $file_field_name,
      'title' => $title,
      'upload_location' => 'public://' . $genotype_dir,
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
  }
  else {
    tpps_build_disabled_file_field($fields, $file_field_name);
  }
  tpps_add_css_js('page_4_genotype', $form);
  return $fields;
}
