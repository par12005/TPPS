<?php

/**
 * @file
 * Page 4 Genotype related functions.
 *
 * Rules:
 * 1. Field's visibility is controlled by Drupal's State API and '#required'
 *    attribute must not be used because it broke validation and require extra
 *    processing.
 * 2. Validation step will check if field was visible and is required.
 *    So no need to do this at form building step.
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
  $snps_fieldset = 'SNPs';
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
    // Note: values will be used in JS for 'marker-type' field.
    'does_study_include_snp_data' => 'SNPs',
    'does_study_include_ssr_cpssr_data' => 'SSRs/cpSSRs',
    // Note: Should be used 'Other Genotypic' but left 'Other' to have
    // 'marker-type' field backward compatibility.
    'does_study_include_other_genotypic_data' => 'Other',
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
            ? t('other genotypic') : str_replace('s', '', $marker_name)),
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

  // This field is hidden but left to avoid changes in submit_all.php script.
  // @TODO remove this field and use new 3 fields.
  $fields['marker-type'] = [
    '#type' => 'select',
    '#multiple' => TRUE,
    '#title' => t('Marker Type: *'),
    '#options' => [
      'SNPs' => t('SNPs'),
      'SSRs/cpSSRs' => t('SSRs/cpSSRs'),
      'Other' => t('Other'),
    ],
    // This field is hidden but left for backward compatibility.
    '#prefix' => '<div class="tpps-hidden">',
    '#suffix' => '</div>',
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Fieldsets.
  // SNPs.
  $fields[$snps_fieldset] = [
    '#type' => 'fieldset',
    '#title' => t('SNPs Information:'),
    '#collapsible' => TRUE,
    // This element used to update whole fieldset by AJAX-request.
    '#prefix' => "<div id='$organism_name-genotype-$snps_fieldset'>",
    '#suffix' => '</div>',
    '#states' => [
      'visible' => [
        ':input[name="' . $organism_name . '[genotype]'
        . '[does_study_include_snp_data]"]' => ['value' => 'yes'],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // SNPs Fieldset's fields.
  tpps_page_4_marker_info($fields, $form_state, $organism_name);
  tpps_page_4_genotype_ssrs(array_merge($chest, [
    'organism_name' => $organism_name,
  ]));
  // Other.
  $other_fieldset = 'other';
  $fields[$other_fieldset] = [
    '#type' => 'fieldset',
    '#title' => t('Other Information:'),
    '#collapsible' => TRUE,
    // This element used to update whole fieldset by AJAX-request.
    '#prefix' => "<div id='$organism_name-genotype-$other_fieldset'>",
    '#suffix' => '</div>',
    '#states' => [
      'visible' => [
        ':input[name="' . $organism_name . '[genotype]'
        . '[does_study_include_other_genotypic_data]"]' => ['value' => 'yes'],
      ],
    ],
  ];
  tpps_page_4_ref($fields, $form_state, $organism_name);


  $genotyping_type_parents = [
    $organism_name, 'genotype', $snps_fieldset, 'genotyping-type'
  ];
  $file_type_parents = [$organism_name, 'genotype', $snps_fieldset, 'file-type'];
  // Value is a string because mutiple values not allowed.
  $genotyping_type_check = tpps_get_ajax_value($form_state, $genotyping_type_parents);
  $file_type_value = tpps_get_ajax_value($form_state, $file_type_parents);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Note: Marker Type allows multiple values to be selected.
  // Relocated in v2. ['genotype', 'files'] -> ['genotype', $snps_fieldset].
  $upload_snp_association = tpps_get_ajax_value(
    $form_state,
    [$organism_name, 'genotype', $snps_fieldset, 'upload_snp_association'],
    'Yes'
  );
  if (tpps_is_genotype_data_type($form_state)) {
    $fields[$snps_fieldset]['upload_snp_association'] = [
      '#type' => 'select',
      '#title' => t('Would you like to upload a SNP association file?'),
      '#options' => [
        'Yes' => t('Yes'),
        'No' => t('No'),
      ],
      '#default_value' => $upload_snp_association,
// @TODO Test if form really updates and new fields are shown.
      '#ajax' => [
        'callback' => 'tpps_genotype_snps_fieldset_callback',
        'wrapper' => "$organism_name-genotype-$snps_fieldset",
        'effect' => 'slide',
      ],
    ];
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Genotyping Type.
  // Field was relocated (v.2).
  // ['files'] -> [$snps_fieldset].
  $fields[$snps_fieldset]['genotyping-type'] = [
    '#type' => 'select',
    '#title' => t('Genotyping Type: *'),
    '#options' => [
      'Genotyping Assay' => t('Genotyping Assay'),
      'Genotyping' => t('Genotyping'),
    ],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Genotype File Type.
  // Field was relocated (v.2).
  // ['files'] -> [$snps_fieldset].
  $fields[$snps_fieldset]['file-type'] = [
    '#type' => 'select',
    '#title' => t('Genotyping file type: *'),
    '#options' => [
      'SNP Assay file and Assay design file' => t('SNP Assay file and Assay design file'),
      'VCF' => t('VCF'),
    ],
    '#states' => [
      'visible' => [
        ':input[name="' . $organism_name . '[genotype][' . $snps_fieldset
        . '][genotyping-type]"]' => ['value' => 'Genotyping'],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // VCF Location.
  $file_field_name = 'vcf';
  $is_tppsc = (($form_state['build_info']['form_id'] ?? 'tpps_main') == 'tppsc_main');
  if ($is_tppsc) {
    tpps_add_dropdown_file_selector(array_merge($chest, [
      'form' => &$fields,
      // Note: 'parents' not yet implemented.
      'parents' => ['files'],
      'file_field_name' => $file_field_name,
      'file_name' => t('VCF'),
      'organism_name' => $organism_name,
    ]));
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // VCF File.
  $title = t('VCF File');
  $file_field_name = 'vcf';
  $snps_fieldset = 'SNPs';
  // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
  tpps_form_build_file_field(array_merge($chest, [
    'parents' => [$organism_name, 'genotype', $snps_fieldset],
    'field_name' => $file_field_name,
    'title' => $title,
    'organism_name' => $organism_name,
    'type' => $chest['type'],
    'description' => '',
    'extensions' => ['gz tar zip'],
  ]));
  if ($is_tppsc) {
    tpps_array_set_value(
      $chest['form'],
      [$organism_name, 'genotype', $snps_fieldset, $file_field_name, '#states'],
      [
        'visible' => [
          ':input[name="' . $organism_name
            . '[genotype][' . $snps_fieldset . '][file-type]"]' => ['value' => 'VCF'],
          ':input[name="' . $organism_name . '[genotype][' . $snps_fieldset . ']['
            . $file_field_name . '_file-location]"]' => ['value' => 'local'],
        ],
      ]
    );
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // SNP Assay File.
  $title = t('SNP Assay File');
  $file_field_name = 'snps-assay';
  // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
  tpps_form_build_file_field(array_merge($chest, [
    'parents' => [$organism_name, 'genotype', $snps_fieldset],
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
    'states' => [
      'visible' => [
        [
          ':input[name="' . $organism_name . '[genotype][' . $snps_fieldset
            . '][genotyping-type]"]' => ['value' => 'Genotyping Assay'],
        ],
        'or',
        [
          ':input[name="' . $organism_name . '[genotype][' . $snps_fieldset
            . '][file-type]"]' => ['value' => 'SNP Assay file and Assay design file'],
        ],
      ],
    ],
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
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [$organism_name, 'genotype', $snps_fieldset],
      'field_name' => $file_field_name,
      'title' => $title,
      'organism_name' => $organism_name,
      'type' => $chest['type'],
    ]);
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    $fields[$snps_fieldset]['assay-citation'] = [
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
    $file_field_name = 'snps-association';
    $title = t('SNP Association File');
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [$organism_name, 'genotype', $snps_fieldset],
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

    $fields[$snps_fieldset][$file_field_name] = array_merge(
      $fields[$snps_fieldset][$file_field_name],
      [
        'empty' => [
          '#default_value' => tpps_array_get_value(
            $chest['page4_values'],
            [$organism_name, 'genotype', $snps_fieldset, $file_field_name, 'empty'])
            ?? 'NA',
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
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    $fields[$snps_fieldset]['snps-association-type'] = [
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

    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    $fields[$snps_fieldset]['snps-association-tool'] = [
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

    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // SNPs Population Structure File.
    $file_field_name = 'snps-pop-struct';
    $title = t('SNPs Population Structure File');
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [$organism_name, 'genotype', $snps_fieldset],
      'organism_name' => $organism_name,
      'type' => $chest['type'],
      'field_name' => $file_field_name,
      'title' => $title,
      // @todo [VS] Replace with 'required' with default value 'TRUE'.
      'optional' => TRUE,
    ]);
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // SNPs Kinship File.
    $file_field_name = 'snps-kinship';
    $title = t('SNPs Kinship File');
    // Field was relocated (v.2). ['files'] -> [$snps_fieldset].
    tpps_form_build_file_field([
      'form' => &$form,
      'form_state' => $form_state,
      'parents' => [$organism_name, 'genotype', $snps_fieldset],
      'organism_name' => $organism_name,
      'type' => $chest['type'],
      'field_name' => $file_field_name,
      'title' => $title,
      'optional' => TRUE,
    ]);
  }
  else {
    $file_field_list = ['snps-association', 'snps-pop-struct', 'snps-kinship'];
    foreach ($file_field_list as $file_field_name) {
      tpps_build_disabled_file_field($fields, $file_field_name);
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Other Fieldset.
  $other_fieldset = 'other';
  // Relocated in v2: [] -> ['other'].
  // v1: This field was a textfield.
  $fields[$other_fieldset]['other-marker'] = [
    '#type' => 'select',
    '#title' => t('Other marker type: *'),
    '#options' => [
      'AFLPs' => t('AFLPs'),
      'Indels' => t('Indels'),
    ],
  ];

  // Field was relocated (v.2). ['files'] -> ['other'].
  $file_field_name = 'other';
  $title = t('Other spreadsheet: '
    . '<br />please provide a spreadsheet with columns for the Plant ID '
    . 'of genotypes used in this study');
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
    'parents' => [$organism_name, 'genotype', $other_fieldset],
    'field_name' => $file_field_name,
    'title' => $title,
    'organism_name' => $organism_name,
    'type' => $chest['type'],
    'description' => $description,
    'empty_field_value' => TRUE,
  ]);

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Other Columns.
  // Field [$other_fieldset]['other']['dynamic'].
  $default_dynamic = !empty($page4_values[$organism_name]['genotype'][$other_fieldset]['other-columns']);
  // Field was relocated (v.2). ['files'] -> ['other'].
  $fields[$other_fieldset]['other']['dynamic'] = [
    '#type' => 'checkbox',
    '#title' => t('This file needs dynamic dropdown options for column data type specification'),
    '#ajax' => [
      // @TODO Check if this element exists on page.
      'wrapper' => "edit-$organism_name-genotype-$other_fieldset-other-ajax-wrapper",
      'callback' => 'tpps_page_4_file_dynamic',
      'effect' => 'slide',
    ],
    '#default_value' => $default_dynamic,
  ];
  $dynamic = tpps_get_ajax_value($form_state,
    [$organism_name, 'genotype', $other_fieldset, 'other', 'dynamic'],
    $default_dynamic,
    'other'
  );

  if ($dynamic) {
    $fields[$other_fieldset]['other']['columns'] = [
      '#description' => t('Please define which columns hold the required data: '
        . '<br />Plant Identifier, Genotype Data'
      ),
    ];
    $fields[$other_fieldset]['other']['columns-options'] = [
      '#type' => 'hidden',
      '#value' => ['Genotype Data', 'Plant Identifier', 'N/A'],
    ];
  }
  $fields[$other_fieldset]['other']['no-header'] = [];

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
  $snps_fieldset = 'SNPs';
  $fields[$snps_fieldset]['genotyping-design'] = [
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
  $fields[$snps_fieldset]['GBS'] = [
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
        ':input[name="' . $id . '[genotype][' . $snps_fieldset . '][genotyping-design]"]' =>
        ['value' => '1'],
      ],
    ],
  ];
  $fields[$snps_fieldset]['GBS-other'] = [
    '#type' => 'textfield',
    '#title' => t('Other GBS Type: *'),
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][' . $snps_fieldset . '][GBS]"]' => ['value' => '5'],
        ':input[name="' . $id . '[genotype][' . $snps_fieldset . '][genotyping-design]"]' =>
        ['value' => '1'],
      ],
    ],
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Targeted Capture Type.
  $fields[$snps_fieldset]['targeted-capture'] = [
    '#type' => 'select',
    '#title' => t('Targeted Capture Type: *'),
    '#options' => [
      0 => t('- Select -'),
      1 => t('Exome Capture'),
      2 => t('Other'),
    ],
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][' . $snps_fieldset . '][genotyping-design]"]' => ['value' => '2'],
      ],
    ],
  ];

  $fields[$snps_fieldset]['targeted-capture-other'] = [
    '#type' => 'textfield',
    '#title' => t('Other Targeted Capture: *'),
    '#states' => [
      'visible' => [
        ':input[name="' . $id . '[genotype][' . $snps_fieldset . '][targeted-capture]"]' => ['value' => '2'],
        ':input[name="' . $id . '[genotype][' . $snps_fieldset . '][genotyping-design]"]' => ['value' => '2'],
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

  $snps_fieldset = 'SNPs';
  $options = ['key' => 'filename', 'recurse' => FALSE];
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

  // Field was relocated (v.2).
  // 'source' => [$id, 'genotype', 'ref-genome'],
  // 'target' => [$id, 'genotype', $snps_fieldset, 'ref-genome'],
  $fields[$snps_fieldset]['ref-genome'] = [
    '#type' => 'select',
    '#title' => t('Reference Assembly used: *'),
    '#options' => $ref_genome_arr,
  ];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //require_once drupal_get_path('module', 'tripal') . '/includes/tripal.importer.inc';
  module_load_include('inc', 'tripal', '/includes/tripal.importer');
  $class = 'EutilsImporter';
  tripal_load_include_importer_class($class);
  $eutils = tripal_get_importer_form(array(), $form_state, $class);
  $eutils['#type'] = 'fieldset';
  $eutils['#title'] = 'Tripal Eutils BioProject Loader';
  $eutils['#states'] = [
    'visible' => [
      ':input[name="' . $id . '[genotype][' . $snps_fieldset . '][ref-genome]"]' => ['value' => 'bio'],
    ],
  ];
  $eutils['accession']['#description'] = t('Valid examples: 12384, 394253, 66853, PRJNA185471');
  $eutils['db'] = ['#type' => 'hidden', '#value' => 'bioproject'];
  unset($eutils['options']);
  $eutils['options']['linked_records'] = ['#type' => 'hidden', '#value' => 1];
  $eutils['callback']['#ajax'] = [
    'callback' => 'tpps_ajax_bioproject_callback',
    'wrapper' => "$id-tripal-eutils",
    'effect' => 'slide',
  ];
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
  $fasta['#states'] = [
    'visible' => [
      [
        [':input[name="' . $id . '[genotype][' . $snps_fieldset . '][ref-genome]"]' => ['value' => 'url']],
        'or',
        [':input[name="' . $id . '[genotype][' . $snps_fieldset . '][ref-genome]"]' => ['value' => 'manual']],
        'or',
        [':input[name="' . $id . '[genotype][' . $snps_fieldset . '][ref-genome]"]' => ['value' => 'manual2']],
      ],
    ],
  ];

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
          [':input[name="' . $id . '[genotype][' . $snps_fieldset . '][ref-genome]"]' => ['value' => 'manual']],
          'or',
          [':input[name="' . $id . '[genotype][' . $snps_fieldset . '][ref-genome]"]' => ['value' => 'manual2']],
        ],
      ],
    ];
  $fasta['file']['file_remote']['#states'] = [
    'visible' => [
      ':input[name="' . $id . '[genotype][' . $snps_fieldset . '][ref-genome]"]' => ['value' => 'url'],
    ],
  ];

  $fields['tripal_fasta'] = $fasta;
}

/**
 * Adds checkbox to select existing file or upload new one.
 *
 * @param array $chest
 *   Required data.
 *
 * @return mixed
 *   Returns value of file field.
 */
function tpps_add_dropdown_file_selector(array $chest) {
  if (!isset($chest['form']) || empty($chest['file_field_name'])
    || empty($chest['file_name']) || empty($chest['organism_name'])
  ) {
    return;
  }
  $snps_fieldset = 'SNPs';
  $form = &$chest['form'];
  $file_field_name = $chest['file_field_name'];
  $file_name = $chest['file_name'];
  $organism_name = $chest['organism_name'];
  // @TODO Use $parents instead of hardcoded 'files' element.
  $parents = $chest['parents'];
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  module_load_include('inc', 'tpps', 'includes/common');
  $params = [
    '@file_name' => $file_name,
    '@hostname' => tpps_get_hostname(),
  ];
  // Field was relocated (v.2).
  // ['saved_values', TPPS_PAGE_4, 'organism-' . $i, 'genotype', 'files'] =>
  // ['saved_values', TPPS_PAGE_4, 'organism-' . $i, 'genotype', $snps_fieldset].
  $form[$snps_fieldset][$file_field_name . '_file-location'] = [
    '#type' => 'select',
    '#title' => t('@file_name location', $params),
    '#options' => [
      'local' => t('My @file_name is stored locally', $params),
      'remote' => t('My @file_name is stored at @hostname', $params),
    ],
    '#default_value' => '',
    '#states' => [
      'visible' => [
        ':input[name="' . $organism_name . '[genotype][' . $snps_fieldset
          . '][file-type]"]' => ['value' => 'VCF'],
      ],
    ],
  ];
  // Field was relocated (v.2).
  // ['saved_values', TPPS_PAGE_4, 'organism-' . $i, 'genotype', 'files'] =>
  // ['saved_values', TPPS_PAGE_4, 'organism-' . $i, 'genotype', $snps_fieldset].
  $form[$snps_fieldset]['local_' . $file_field_name] = [
    '#type' => 'textfield',
    '#title' => t('Path to @file_name at @hostname: *', $params),
    '#states' => [
      'visible' => [
        ':input[name="' . $organism_name . '[genotype][' . $snps_fieldset . ']['
        . $file_field_name . '_file-location]"]' => ['value' => 'remote'],
      ],
    ],
    '#description' => t('Please provide the full path to your @file_name file '
      . 'stored on @hostname', $params),
  ];
}

/**
 * Adds SSRs/cpSSRs fields.
 *
 * @param array $chest
 *   Required metadata.
 */
function tpps_page_4_genotype_ssrs(array $chest) {
  $form = &$chest['form'];
  // @TODO Minor. Replace 'genotype' with $chest['type'].
  $organism_name = $chest['organism_name'];
  $fields = &$form[$organism_name][$chest['type']];
  // SSRs/cpSSRs.
  $ssrs_fieldset = 'ssrs_cpssrs';
  $fields[$ssrs_fieldset] = [
    '#type' => 'fieldset',
    '#title' => t('SSRs/cpSSRs Information:'),
    '#collapsible' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="' . $organism_name . '[genotype]'
        . '[does_study_include_ssr_cpssr_data]"]' => ['value' => 'yes'],
      ],
    ],
  ];
  // @TODO Minor. Better to rename field to avoid '/' in name
  // and make it more meaningful.
  $ssr_type_select = 'SSRs/cpSSRs';

  // Field was relocated (v.2).
  // ['saved_values', 4, "organism-$i", 'genotype', 'SSRs/cpSSRs'] =>
  // ['saved_values', 4, "organism-$i", 'genotype', $ssrs_fieldset, 'SSRs/cpSSRs'];
  $fields[$ssrs_fieldset][$ssr_type_select] = [
    '#type' => 'select',
    '#title' => t('Define SSRs/cpSSRs Type: *'),
    '#options' => [
      'SSRs' => t('SSRs'),
      'cpSSRs' => t('cpSSRs'),
      'Both SSRs and cpSSRs' => t('Both SSRs and cpSSRs'),
    ],
    '#default_value' => tpps_get_ajax_value(
      $chest['form_state'], [$organism_name, 'genotype', $ssr_type_select], 'SSRs'
    ),
    '#states' => [
      'visible' => [
        ':input[name="' . $organism_name . '[genotype]'
        . '[does_study_include_ssr_cpssr_data]"]' => ['value' => 'yes'],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Field was relocated (v.2).
  // ['saved_values', 4, "organism-$i", 'genotype', 'files', 'ploidy'] =>
  // ['saved_values', 4, "organism-$i", 'genotype', $ssrs_fieldset, 'ploidy'];
  $fields[$ssrs_fieldset]['ploidy'] = [
    '#type' => 'select',
    '#title' => t('SSR Ploidy: *'),
    '#options' => [
      'Haploid' => t('Haploid'),
      'Diploid' => t('Diploid'),
      'Polyploid' => t('Polyploid'),
    ],
    '#default_value' => tpps_get_ajax_value(
      $chest['form_state'], [$organism_name, 'genotype', 'files', 'ploidy'], 'Haploid'
    ),
  ];
  if (variable_get('tpps_page_4_update_ploidy_description', TRUE)) {
    // Allow 'Ploidy' field desctiption be update on the fly.
    $ploidy_description = [
      'Haploid' => t('For haploid, TPPS assumes that each remaining column '
        . 'in the spreadsheet is a marker.'),
      'Diploid' => t('For diploid, TPPS will assume that pairs of columns '
        . 'together are describing an individual marker, so the second and '
        . 'third columns would be the first marker, the fourth and fifth '
        . 'columns would be the second marker, etc.'),
      'Polyploid' => t('For polyploid, TPPS will read columns until it arrives '
        . 'at a non-empty column with a different name from the last.'),
    ];
    $form['#attached']['js'][] = [
      'type' => 'setting',
      'data' => [
        'tpps' => [
          'ssrsFieldset' => 'ssrs_cpssrs',
          'ploidyDescriptions' => $ploidy_description,
          'ssrFields' => ['ssrs', 'ssrs_extra'],
        ],
      ],
      'scope' => 'footer',
    ];
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // SSRs field.
  //
  $file_field_name = 'ssrs';
  $title = t('SSRs Spreadsheet');
  // Note: Description differs only single word '@type'.
  $ssr_field_description = 'Please upload a spreadsheet containing your '
    . '@type data. The format of this file is very important! TPPS will '
    . 'parse your file based on the ploidy you have selected above. '
    . 'For any ploidy, TPPS will assume that the first column of your '
    . 'file is the column that holds the Plant Identifier that matches '
    . 'your accession file.';

  // Field was relocated (v.2).
  //'source' => [$organism_name, 'genotype', 'files', 'ssrs'];
  //'target' => [$ssrs_fieldset, 'ssrs'];
  tpps_form_build_file_field(array_merge($chest, [
    'parents' => [$organism_name, 'genotype', $ssrs_fieldset],
    'field_name' => $file_field_name,
    'title' => $title,
    'organism_name' => $organism_name,
    'type' => $chest['type'],
    'description' => t($ssr_field_description, ['@type' => 'SSR']),
    // Add extra text field for empty field value.
    'empty_field_value' => TRUE,
    'show_extensions_in_description' => TRUE,
    'use_fid' => TRUE,
    // Visible when: 'SSRs' or 'Both SSRs and cpSSRs'.
    'states' => [
      'invisible' => [
        ':input[name="' . $organism_name . '[genotype][' . $ssr_type_select . ']"]'
        => ['value' => 'cpSSRs'],
      ],
    ],

    // no header checkbox.

    //'extra_elements' => [
    //      'columns' => [
    //        '#description' => t('Please define which columns hold the '
    //          . 'required data: SNP ID, Scaffold, Position, Allele, '
    //          . 'Associated Trait, Confidence Value.'),
    //      ],
    //      'columns-options' => [
    //        '#type' => 'hidden',
    //        '#value' => [
    //          'N/A',
    //          'SNP ID',
    //          'Scaffold',
    //          'Position',
    //          'Allele',
    //          'Associated Trait',
    //          'Confidence Value',
    //          'Gene ID',
    //          'Annotation',
    //        ],
    //        'no-header' => [],
    //      ],
    //],

  ]));
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // csSSR Field.
  $title = t('cpSSRs Spreadsheet');
  $file_field_name = 'ssrs_extra';
  // Field was relocated (v.2).
  // ['saved_values', 4, "organism-$i", 'genotype', 'files', 'ssrs_extra'] =>
  // ['saved_values', 4, "organism-$i", 'genotype', $ssrs_fieldset, 'ssrs_extra'];
  tpps_form_build_file_field(array_merge($chest, [
    'parents' => [$organism_name, 'genotype', $ssrs_fieldset],
    'field_name' => $file_field_name,
    'title' => $title,
    'organism_name' => $organism_name,
    'type' => $chest['type'],
    'description' => t($ssr_field_description, ['@type' => 'cpSSR']),
    // Add extra text field for empty field value.
    'empty_field_value' => TRUE,
    'show_extensions_in_description' => TRUE,
    'use_fid' => TRUE,
    // Visible when: 'cpSSRs' or 'Both SSRs and cpSSRs'.
    'states' => [
      'invisible' => [
        ':input[name="' . $organism_name . '[genotype][' . $ssr_type_select . ']"]'
        => ['value' => 'SSRs'],
      ],
    ],
  ]));
}
