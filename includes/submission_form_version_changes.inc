<?php

/**
 * @file
 * Manages Submission Form Version.
 */

/**
 * How to use?
 * module_load_include('inc', 'tpps', 'includes/submission_form_version_changes');
 */

/**
 * Gets specific version or all version's changes.
 *
 * Note:
 * Version 1 just converts $form_state into Submission Interface by
 * removing elements useless for pipeline processing.
 * Next versions are about change order of fields at form and could be used
 * to update old versions of $form_state to latest version.
 *
 * @param int $version
 *   Version of the Submission Form.
 *   This version will be used to create Submission Interface.
 *
 * @return mixed
 *   Returns list of changes for particular version or NULL if there is no
 *   changes for given version number.
 *   Returns all version's changes.
 */
function tpps_submission_form_version_changes(array $form_state, $version = NULL) {
  //dpm(print_r(debug_backtrace(!DEBUG_BACKTRACE_PROVIDE_OBJECT | DEBUG_BACKTRACE_IGNORE_ARGS, 2), 1));
  $version_change_list = [
    // Version 1 is a initial version.
    // Submission Interface is a version 1 without some unnecessary elements.
    // See Submission:purifySharedState();
    '1' => [],
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // #86ayw8a7k Genotype Logic changes.
    // Define how to get Submission Form Version #2.
    '2' => [
      // v1 -----> v2.
      'move' => [
        // Page 1. 'DOI' field.
        'publication_doi' => [
          // v1.
          'source' => ['saved_values', TPPS_PAGE_1, 'doi'],
          // v2.
          'target' => [
            'saved_values', TPPS_PAGE_1, 'publication', 'publication_doi',
          ],
        ],
        'dataset_doi' => [
          'source' => ['saved_values', TPPS_PAGE_1, 'dataset_doi'],
          'target' => [
            'saved_values', TPPS_PAGE_1, 'publication', 'dataset_doi',
          ],
        ],
      ],
    ],
  ];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Version 2. Dynamyc changes.
  // #86ayw8a7k Relocate genotype fields.
  if (!empty($form_state['saved_values'][TPPS_PAGE_1])) {
    $page1_values = $form_state['saved_values'][TPPS_PAGE_1];
    $organism_number = $page1_values['organism']['number'] ?? 1;
    // List of relocated genotype fields from 'files' to 'SNPs' fieldset.
    // ['files'] -> ['SNPs'].
    $snps_fieldset = 'SNPs';
    $files_to_snps_fields = [
      'genotyping-type',
      'file-type',
      'vcf',
      // @TODO process.
      // 'vcf_file-location',
      'snps-assay',
      'assay-design',
      'snps-association',
      'snps-association-tool',
      'snps-association-type',
      'snps-pop-struct',
      'snps-kinship',
      'assay-citation',
      // Yes/No field.
      'upload_snp_association',
    ];
    $to_other_fieldset = [
      // Textfield (must be select).
      ['files', 'other-marker'], // Select.
      ['files', 'other'], // Managed file.
      ['files', 'other-columns'],
      // Other file-field subfields. Those fields commented out and not shown
      // at form right now.
      //['files', 'other', 'dynamic'],
      //['files', 'other', 'columns'],
      //['files', 'other', 'columns-options'],
      //['files', 'other', 'no-header'],
    ];

    $ssrs_fieldset = 'ssrs_cpssrs';
    $to_ssr_fieldset = [
      // Dropdown menu.
      ['SSRs/cpSSRs'],
      // Managed files.
      ['files', 'ploidy'],
      ['files', 'ssrs'],
      ['files', 'ssrs_extra'],
    ];
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    for ($i = 1; $i <= $organism_number; $i++) {
      $genotype_parents = [
        'saved_values', TPPS_PAGE_4, 'organism-' . $i, 'genotype',
      ];
      // 'ref-genome' field.
      $field_name = 'ref-genome';
      $version_change_list['2']['move'][] = [
        'source' => array_merge($genotype_parents, [$field_name]),
        'target' => array_merge(
          $genotype_parents,
          [$snps_fieldset, $field_name]
        ),
      ];
      // 'files' to 'SNPs' fields.
      foreach ($files_to_snps_fields as $field_name) {
        $version_change_list['2']['move'][] = [
          'source' => array_merge($genotype_parents, ['files', $field_name]),
          'target' => array_merge($genotype_parents, ['SNPs', $field_name]),
        ];
      }
      // Move to fieldset 'SSRs/cpSSRs Information'.
      // 'source' new location, 'target' - original location.
      foreach ($to_ssr_fieldset as $item) {
        $version_change_list['2']['move'][] = [
          'source' => array_merge($genotype_parents, $item),
          'target' => array_merge(
            $genotype_parents,
            [$ssrs_fieldset, end($item)]
          ),
        ];
      }
      // ['genotype'] -> ['genotype', 'other']
      foreach ($to_other_fieldset as $item) {
        $version_change_list['2']['move'][] = [
          'source' => array_merge($genotype_parents, $item),
          'target' => array_merge($genotype_parents, ['other', end($item)]),
        ];
      }
    }
  }
  // End of Verion 2 Dynamic Changes.

  return ($version ? ($version_change_list[$version] ?? NULL) : $version_change_list);
}
