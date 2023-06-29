<?php

/**
 * @file
 * Defines the data integrity checks for the fourth page of the form.
 */

/**
 * Defines the data integrity checks for the fourth page of the form.
 *
 * @param array $form
 *   The form that is being validated.
 * @param array $form_state
 *   The state of the form that is being validated.
 */
function tpps_page_4_validate_form(array &$form, array &$form_state) {
  if ($form_state['submitted'] == '1') {
    unset($form_state['file_info'][TPPS_PAGE_4]);

    $form_values = $form_state['values'];
    $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];

    for ($i = 1; $i <= $organism_number; $i++) {
      $organism = $form_values["organism-$i"];

      if ($i > 1 and isset($organism['phenotype-repeat-check']) and $organism['phenotype-repeat-check'] == '1') {
        unset($form_state['values']["organism-$i"]['phenotype']);
      }
      if (isset($form_state['values']["organism-$i"]['phenotype'])) {
        tpps_validate_phenotype($form_state['values']["organism-$i"]['phenotype'], $i, $form, $form_state);
      }

      if ($i > 1 and isset($organism['genotype-repeat-check']) and $organism['genotype-repeat-check'] == '1') {
        unset($form_state['values']["organism-$i"]['genotype']);
      }
      if (isset($form_state['values']["organism-$i"]['genotype'])) {
        tpps_validate_genotype($form_state['values']["organism-$i"]['genotype'], $i, $form, $form_state);
      }

      if ($i > 1 and isset($organism['environment-repeat-check']) and $organism['environment-repeat-check'] == '1') {
        unset($form_state['values']["organism-$i"]['environment']);
      }
      if (isset($form_state['values']["organism-$i"]['environment'])) {
        tpps_validate_environment($form_state['values']["organism-$i"]['environment'], "organism-$i");
      }
    }

    if (form_get_errors() and !$form_state['rebuild']) {
      $form_state['rebuild'] = TRUE;
      $new_form = drupal_rebuild_form('tpps_main', $form_state, $form);

      for ($i = 1; $i <= $organism_number; $i++) {

        if (isset($new_form["organism-$i"]['phenotype']['metadata']['upload'])) {
          $form["organism-$i"]['phenotype']['metadata']['upload'] = $new_form["organism-$i"]['phenotype']['metadata']['upload'];
          $form["organism-$i"]['phenotype']['metadata']['upload']['#id'] = "edit-organism-$i-phenotype-metadata-upload";
        }
        if (isset($new_form["organism-$i"]['phenotype']['metadata']['columns'])) {
          $form["organism-$i"]['phenotype']['metadata']['columns'] = $new_form["organism-$i"]['phenotype']['metadata']['columns'];
          $form["organism-$i"]['phenotype']['metadata']['columns']['#id'] = "edit-organism-$i-phenotype-metadata-columns";
        }

        if (isset($form["organism-$i"]['phenotype']['file'])) {
          $form["organism-$i"]['phenotype']['file']['upload'] = $new_form["organism-$i"]['phenotype']['file']['upload'];
          $form["organism-$i"]['phenotype']['file']['columns'] = $new_form["organism-$i"]['phenotype']['file']['columns'];
          $form["organism-$i"]['phenotype']['file']['upload']['#id'] = "edit-organism-$i-phenotype-file-upload";
          $form["organism-$i"]['phenotype']['file']['columns']['#id'] = "edit-organism-$i-phenotype-file-columns";
        }

        $file_types = array(
          'snps-assay',
          'other',
        );

        $field_types = array(
          'upload',
          'columns',
        );

        foreach ($file_types as $type) {
          foreach ($field_types as $field) {
            if (isset($form["organism-$i"]['genotype']['files'][$type][$field]) and isset($new_form["organism-$i"]['genotype']['files'][$type][$field])) {
              $form["organism-$i"]['genotype']['files'][$type][$field] = $new_form["organism-$i"]['genotype']['files'][$type][$field];
              $form["organism-$i"]['genotype']['files'][$type][$field]['#id'] = "edit-organism-$i-genotype-files-{$type}-{$field}";
            }
          }
        }
      }
    }
  }
}

/**
 * Validates the phenotype section of the fourth page of the form.
 *
 * @param array $phenotype
 *   The form_state values of the phenotype fieldset for organism $id.
 * @param string $org_num
 *   The id of the organism fieldset being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_phenotype(array $phenotype, $org_num, array $form, array &$form_state) {
  $normal_check = $phenotype['normal-check'];
  $iso_check = $phenotype['iso-check'];
  $id = "organism-$org_num";
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];

  if (empty($normal_check) and empty($iso_check)) {
    form_set_error("$id][phenotype][normal-check", t("Please choose at least one category of phenotypes to upload"));
  }

  if ($normal_check) {
    $phenotype_number = $phenotype['phenotypes-meta']['number'];
    $phenotype_check = $phenotype['check'];
    $phenotype_meta = $phenotype['metadata'];
    $phenotype_file = $phenotype['file'];

    if ($phenotype_check == '1' and empty($phenotype_meta)) {
      form_set_error("$id][phenotype][metadata", t("Phenotype Metadata File: field is required."));
    }
    if ($phenotype_check == '1' and !empty($phenotype_meta)) {
      $required_groups = array(
        'Phenotype Id' => array(
          'id' => array(1),
        ),
        'Attribute' => array(
          'attr' => array(2),
        ),
        'Description' => array(
          'desc' => array(3),
        ),
        'Units' => array(
          'units' => array(4),
        ),
        'Structure' => array(
          'structure' => array(5),
        ),
      );

      $file_element = $form[$id]['phenotype']['metadata'];
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
      if (!form_get_errors()) {
        // Get phenotype name column.
        $phenotype_name_col = $groups['Phenotype Id']['1'];

        // Preserve file if it is valid.
        tpps_preserve_valid_file($form_state, $form_state['values'][$id]['phenotype']['metadata'], $org_num, "Phenotype_Metadata");
      }
    }

    for ($i = 1; $i <= $phenotype_number; $i++) {
      $current_phenotype = $phenotype['phenotypes-meta']["$i"];
      $name = $current_phenotype['name'];
      $description = $current_phenotype['description'];
      $units = $current_phenotype['units'];

      if ($name == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][name", "Phenotype $i Name: field is required.");
      }

      if (!$current_phenotype['attribute']) {
        form_set_error("$id][phenotype][phenotypes-meta][$i][attribute", "Phenotype $i Attribute: field is required.");
      }

      if ($current_phenotype['attribute'] == 'other' and $current_phenotype['attr-other'] == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][attr-other", "Phenotype $i Custom Attribute: field is required.");
      }

      if ($description == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][description", "Phenotype $i Description: field is required.");
      }

      if ($units == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][units", "Phenotype $i Units: field is required.");
      }
      elseif ($units == 'other' and $current_phenotype['unit-other'] == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][unit-other", "Phenotype $i Custom Unit: field is required.");
      }

      if ($current_phenotype['structure'] == 'other' and $current_phenotype['struct-other'] == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][struct-other", "Phenotype $i Custom Structure: field is required.");
      }

      if (($current_phenotype['val-check'] or $current_phenotype['bin-check'] or $current_phenotype['units'] == tpps_load_cvterm('boolean')->cvterm_id) and $current_phenotype['min'] == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][min", "Phenotype $i Minimum Value: field is required.");
      }

      if (($current_phenotype['val-check'] or $current_phenotype['bin-check'] or $current_phenotype['units'] == tpps_load_cvterm('boolean')->cvterm_id) and $current_phenotype['max'] == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][max", "Phenotype $i Maximum Value: field is required.");
      }
    }

    if ($phenotype['time']['time-check']) {
      foreach ($phenotype['time']['time_phenotypes'] as $key => $val) {
        if (!$val) {
          unset($form_state['values'][$id]['phenotype']['time']['time_phenotypes'][$key]);
          unset($form_state['values'][$id]['phenotype']['time']['time_values'][$key]);
        }
      }
      if (empty($form_state['values'][$id]['phenotype']['time']['time_phenotypes'])) {
        form_set_error("$id][phenotype][time][time_phenotypes", t("Time-based Phenotypes: field is required."));
      }
    }

    if (empty($phenotype_file)) {
      form_set_error("$id][phenotype][file", t("Phenotypes: field is required."));
    }
    if (!empty($phenotype_file)) {
      $required_groups = array(
        'Tree Identifier' => array(
          'id' => array(1),
        ),
        'Phenotype Data' => array(
          'phenotype-data' => array(0),
        ),
      );
      if ($phenotype['format'] != 0) {
        $required_groups = array(
          'Tree Identifier' => array(
            'id' => array(1),
          ),
          'Phenotype Name/Identifier' => array(
            'phenotype-name' => array(2),
          ),
          'Phenotype Value(s)' => array(
            'val' => array(3),
          ),
        );
      }

      $file_element = $form[$id]['phenotype']['file'];
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

      if (!form_get_errors()) {
        $phenotype_file_tree_col = $groups['Tree Identifier']['1'];
        $phenotype_names = array();
        if ($phenotype['format'] == 0) {
          $phenotype_file_name_cols = $groups['Phenotype Data']['0'];
          $headers = tpps_file_headers($phenotype_file, !empty($phenotype['file-no-header']));
          foreach ($phenotype_file_name_cols as $column_index) {
            $phenotype_names[] = $headers[$column_index];
          }
        }
        if ($phenotype['format'] != 0) {
          $phenotype_file_name_col = $groups['Phenotype Name/Identifier']['2'];
          $phenotype_names = tpps_parse_file_column($phenotype_file, $phenotype_file_name_col);
        }

        $phenotype_meta_names = array();
        if (isset($phenotype_name_col)) {
          $phenotype_meta_names = tpps_parse_file_column($phenotype_meta, $phenotype_name_col);
        }

        for ($i = 1; $i <= $phenotype_number; $i++) {
          $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
        }
        $missing_phenotypes = array_diff($phenotype_names, $phenotype_meta_names);
        if (!empty($missing_phenotypes)) {
          $phenotype_id_str = implode(', ', $missing_phenotypes);
          form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype Metadata file. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype Metadata file. The phenotypes we detected with missing definitions were: $phenotype_id_str");
        }

        if (isset($phenotype_file_tree_col)) {
          $species_index = empty($thirdpage['tree-accession']['check']) ? 'species-1' : "species-$org_num";
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file'];
          $column_vals = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-columns'];

          foreach ($column_vals as $col => $val) {
            if ($val == '1') {
              $id_col_accession_name = $col;
              break;
            }
          }
          $acc_no_header = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-no-header'];
          $phenotype_no_header = $form_state['values'][$id]['phenotype']['file-no-header'];
          $missing_trees = tpps_compare_files($form_state['values'][$id]['phenotype']['file'], $tree_accession_file, $phenotype_file_tree_col, $id_col_accession_name, $phenotype_no_header, $acc_no_header);
          if ($missing_trees !== array()) {
            $tree_id_str = implode(', ', $missing_trees);
            form_set_error("$id][phenotype][file", "Phenotype file: We detected Plant Identifiers that were not in your Plant Accession file. Please either remove these plants from your Phenotype file, or add them to your Plant Accession file. The Plant Identifiers we found were: $tree_id_str");
          }
        }
      }

      // Preserve file if it is valid.
      tpps_preserve_valid_file($form_state, $form_state['values'][$id]['phenotype']['file'], $org_num, "Phenotype_Data");
    }
  }

  if ($iso_check) {
    if (empty($phenotype['iso'])) {
      form_set_error("$id][phenotype][iso", t("Phenotype Isotope/Mass Spectrometry File: field is required."));
    }

    if (!form_get_errors()) {
      $headers = tpps_file_headers($phenotype['iso']);
      $id_col_name = key($headers);
      while (($k = array_search(NULL, $headers))) {
        unset($headers[$k]);
      }
      $num_columns = tpps_file_width($phenotype['iso']) - 1;
      $num_unique_columns = count(array_unique($headers)) - 1;

      if ($num_unique_columns != $num_columns) {
        form_set_error("$id][phenotype][iso", t("Mass spectrometry/Isotope file: some columns in the file you provided are missing or have duplicate header values. Please either enter valid header values for those columns or remove those columns, then reupload your file."));
      }
    }

    if (!form_get_errors()) {
      $species_index = empty($thirdpage['tree-accession']['check']) ? 'species-1' : "species-$org_num";
      $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file'];
      $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

      $acc_no_header = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-no-header'];
      $missing_trees = tpps_compare_files($phenotype['iso'], $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);

      if ($missing_trees !== array()) {
        $tree_id_str = implode(', ', $missing_trees);
        form_set_error("$id][phenotype][iso", "Mass spectrometry/Isotope file: We detected Plant Identifiers that were not in your Plant Accession file. Please either remove these plants from your file, or add them to your Plant Accession file. The Plant Identifiers we found were: $tree_id_str");
      }
    }

    // Preserve file if it is valid.
    tpps_preserve_valid_file($form_state, $phenotype['iso'], $org_num, "Phenotype_Data");
  }
}

/**
 * Validates the genotype section of the fourth page of the form.
 *
 * @param array $genotype
 *   The form_state values of the genotype fieldset for organism $id.
 * @param string $org_num
 *   The id of the organism fieldset being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_genotype(array $genotype, $org_num, array $form, array &$form_state) {
  $id = "organism-$org_num";
  $snps = $genotype['SNPs'];
  $ref_genome = $genotype['ref-genome'];
  $genotyping_type = $genotype['files']['genotyping-type'];
  $file_type = $genotype['files']['file-type'];
  $vcf = isset($genotype['files']['vcf']) ? $genotype['files']['vcf'] : 0;
  $snps_assay = isset($genotype['files']['snps-assay']) ? $genotype['files']['snps-assay'] : 0;
  $assoc_file = $genotype['files']['snps-association'] ?? 0;
  $other_file = isset($genotype['files']['other']) ? $genotype['files']['other'] : 0;
  $thirdpage = $form_state['saved_values'][TPPS_PAGE_3];

  // [VS]
  $is_step2_genotype = in_array(
    $form_state['saved_values'][TPPS_PAGE_2]['data_type'],
    [
      'Genotype x Environment',
      'Genotype x Phenotype x Environment',
      'Genotype x Phenotype',
    ]
  );
  // [/VS]
  $species_index = empty($thirdpage['tree-accession']['check']) ? 'species-1' : "species-$org_num";
  $tree_accession_file = $thirdpage['tree-accession'][$species_index]['file'];
  $id_col_accession_name = $thirdpage['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

  if (!$ref_genome) {
    tpps_form_error_required($form_state,
      [$id, 'genotype', 'ref-genome']
    );
  }
  elseif ($ref_genome === 'bio') {
    tpps_check_required(
      $form_state, [$id, 'genotype', 'tripal_eutils', 'accession']
    );
    $connection = new \EUtils();
    try {
      $connection->setPreview();
      $parsed = $connection->get(
        $genotype['tripal_eutils']['db'],
        $genotype['tripal_eutils']['accession']
      );
      foreach ($_SESSION['messages']['status'] as $key => $message) {
        if ($message == '<pre>biosample</pre>') {
          unset($_SESSION['messages']['status'][$key]);
          if (empty($_SESSION['messages']['status'])) {
            unset($_SESSION['messages']['status']);
          }
          break;
        }
      }
      $form_state['values']['parsed'] = $parsed;
    }
    catch (\Exception $e) {
      form_set_error("$id][genotype][tripal_eutils][accession", $e->getMessage());
    }
  }

  elseif (in_array($ref_genome, ['url', 'manual', 'manual2'])) {
    $class = 'FASTAImporter';
    tripal_load_include_importer_class($class);
    $fasta_vals = $genotype['tripal_fasta'];

    $file_upload = isset($fasta_vals['file']['file_upload'])
      ? trim($fasta_vals['file']['file_upload']) : 0;
    $file_existing = isset($fasta_vals['file']['file_upload_existing'])
      ? trim($fasta_vals['file']['file_upload_existing']) : 0;
    $file_remote = isset($fasta_vals['file']['file_remote'])
      ? trim($fasta_vals['file']['file_remote']) : 0;
    $db_id = trim($fasta_vals['db']['db_id']);
    $re_accession = trim($fasta_vals['db']['re_accession']);
    $analysis_id = trim($fasta_vals['analysis_id']);
    $seqtype = trim($fasta_vals['seqtype']);

    if (!$file_upload and !$file_existing and !$file_remote) {
      tpps_form_error_required($form_state,
        [$id, 'genotype', 'tripal_fasta', 'file']
      );
    }

    if ($db_id and !$re_accession) {
      tpps_form_error_required($form_state,
        [$id, 'genotype', 'tripal_fasta', 'additional', 're_accession']
      );
    }
    if ($re_accession and !$db_id) {
      tpps_form_error_required($form_state,
        [$id, 'genotype', 'tripal_fasta', 'additional', 'db_id']
      );
    }

    if (!$analysis_id) {
      tpps_form_error_required($form_state,
        [$id, 'genotype', 'tripal_fasta', 'analysis_id']
      );
    }
    if (!$seqtype) {
      tpps_form_error_required($form_state,
        [$id, 'genotype', 'tripal_fasta', 'seqtype']
      );
    }

    if (!form_get_errors()) {
      $assembly = $file_existing ? $file_existing : ($file_upload ? $file_upload : $file_remote);
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // [VS]
  tpps_check_required($form_state, [$id, 'genotype', 'marker-type']);
  // Note: 'maker-type' is array (allows multiple values).
  if (isset($genotype['marker-type']['SNPs'])) {

    if ($is_step2_genotype) {
      $upload_snp_association = ('Yes' ==
        $form_state['values'][$id]['genotype']['files']['upload_snp_association']
      );
      if ($upload_snp_association) {
        tpps_check_required(
          $form_state, [$id, 'genotype', 'files', 'snps-association']
        );
        tpps_check_required(
          $form_state, [$id, 'genotype', 'files', 'snps-association-type']
        );
        tpps_check_required(
          $form_state, [$id, 'genotype', 'files', 'snps-association-tool']
        );
      }
      tpps_check_required(
        $form_state, [$id, 'genotype', 'files', 'genotyping-type']
      );
      tpps_check_required(
        $form_state, [$id, 'genotype', 'files', 'snps-assay']
      );
      tpps_check_required(
        $form_state, [$id, 'genotype', 'files', 'assay-design']
      );
    }

    $condition = (
      tpps_check_required(
        $form_state, [$id, 'genotype', 'SNPs', 'genotyping-design']
      )
      && $snps['genotyping-design'] == '1'
    );
    if ($condition) {
      $condition = (
        tpps_check_required($form_state, [$id, 'genotype', 'SNPs', 'GBS'])
        // 5 = 'Genotyping Array'
        && $snps['GBS'] == '5'
      );
      if ($condition) {
        tpps_check_required($form_state, [$id, 'genotype', 'SNPs', 'GBS-other']);
      }
    }
    elseif ($snps['genotyping-design'] == '2') {
      $condition = (
        tpps_check_required(
          $form_state, [$id, 'genotype', 'SNPs', 'targeted-capture']
        )
        && $snps['targeted-capture'] == '2'
      );
      if ($condition) {
        tpps_check_required(
          $form_state, [$id, 'genotype', 'SNPs', 'targeted-capture-other']
        );
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (isset($genotype['marker-type']['SSRs/cpSSRs'])) {
    tpps_check_required($form_state, [$id, 'genotype', 'SSRs/cpSSRs']);
    if (in_array($genotype['SSRs/cpSSRs'], ['SSRs', 'Both SSRs and cpSSRs'])) {
      tpps_check_required($form_state, [$id, 'genotype', 'files', 'ploidy']);
      tpps_check_required($form_state, [$id, 'genotype', 'files', 'ssrs']);
    }
    if (in_array($genotype['SSRs/cpSSRs'], ['cpSSRs', 'Both SSRs and cpSSRs'])) {
      tpps_check_required(
        $form_state, [$id, 'genotype', 'files', 'extra-ploidy']
      );
      tpps_check_required($form_state, [$id, 'genotype', 'files', 'ssrs_extra']);
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (isset($genotype['marker-type']['Other'])) {
    tpps_check_required($form_state, [$id, 'genotype', 'other-marker']);
    tpps_check_required($form_state, [$id, 'genotype', 'files', 'other']);
  }
  // [/VS]
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if (preg_match('/^0+$/', implode('', $file_type))) {
    tpps_form_error_required($form_state,
      [$id, 'genotype', 'files', 'file-type']
    );
    return;
  }
  $loaded_state = tpps_load_submission($form_state['accession']);
  if (!empty($loaded_state['vcf_replace'])) {
    foreach ($loaded_state['vcf_replace'] as $org_num => $fid) {
      if (file_load($fid)) {
        $form_state['values'][$id]['genotype']['files']['vcf'] = $fid;
        $vcf = $fid;
        $form_state['values'][$id]['genotype']['files']['local_vcf_check'] = NULL;
        $form_state['values'][$id]['genotype']['files']['local_vcf'] = NULL;
      }
      if (!file_load($fid)) {
        form_set_error("$org_num][genotype][files][local_vcf",
          t("Local VCF File: File could not be loaded properly.")
        );
      }
    }
  }

  if (
    !empty($file_type['VCF'])
    and !$vcf
    and trim($form_state['values'][$id]['genotype']['files']['local_vcf']) == ''
  ) {
    tpps_form_error_required($form_state, [$id, 'genotype', 'files', 'vcf']);
  }

  elseif (!empty($file_type['VCF'])) {
    if (
      (
        $ref_genome === 'manual'
        or $ref_genome === 'manual2'
        or $ref_genome === 'url'
      )
      and isset($assembly)
      and $assembly
      and !form_get_errors()
    ) {
      if (trim($form_state['values']["organism-$org_num"]['genotype']['files']['local_vcf']) != '') {
        $local_vcf_path = trim($form_state['values']["organism-$org_num"]['genotype']['files']['local_vcf']);
        $vcf_content = gzopen($local_vcf_path, 'r');
      }
      else {
        $vcf_content = gzopen(file_load($vcf)->uri, 'r');
      }
      $assembly_content = gzopen(file_load($assembly)->uri, 'r');

      while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
        if ($vcf_line[0] != '#') {

          $vcf_values = explode("\t", $vcf_line);
          $scaffold_id = $vcf_values[0];
          $match = FALSE;

          while (($assembly_line = gzgets($assembly_content)) !== FALSE) {
            if ($assembly_line[0] != '>') {
              continue;
            }
            if (preg_match('/^(.*?)\s.*$/', $assembly_line, $matches)) {
              $assembly_scaffold = $matches[1];
            }
            if ($assembly_scaffold[0] == '>') {
              $assembly_scaffold = substr($assembly_scaffold, 1);
            }
            if ($assembly_scaffold == $scaffold_id) {
              $match = TRUE;
              break;
            }
          }
          if (!$match) {
            fclose($assembly_content);
            $assembly_content = gzopen(file_load($assembly)->uri, 'r');
            while (($assembly_line = gzgets($assembly_content)) !== FALSE) {
              if ($assembly_line[0] != '>') {
                continue;
              }
              if (preg_match('/^(.*?)\s.*$/', $assembly_line, $matches)) {
                $assembly_scaffold = $matches[1];
              }
              if ($assembly_scaffold[0] == '>') {
                $assembly_scaffold = substr($assembly_scaffold, 1);
              }
              if ($assembly_scaffold == $scaffold_id) {
                $match = TRUE;
                break;
              }
            }
          }

          if (!$match) {
            form_set_error("$id][genotype][files][vcf",
              t("VCF File: scaffold @scaffold_id not found in assembly file(s)",
              array('@scaffold_id' => $scaffold_id))
            );
          }
        }
      }

    }
    if (
      empty($loaded_state['vcf_replace'])
      && trim($form_state['values']["organism-$org_num"]['genotype']['files']['local_vcf']) != ''
      && $loaded_state['vcf_validated'] !== TRUE
    ) {
      form_set_error(
        "$org_num][genotype][files][local_vcf",
        t("Local VCF File: File needs to be pre-validated. "
        . "Please click on Pre-validate my VCF files button at the bottom.")
      );
    }

    if (
      !empty($loaded_state['vcf_validated'])
      and $loaded_state['vcf_validated'] === TRUE
      and empty($loaded_state['vcf_val_errors'])
    ) {
      drupal_set_message(t('VCF files pre-validated. Skipping VCF file validation'));
    }
    elseif (!form_get_errors()) {
      $accession_ids = tpps_parse_file_column($tree_accession_file, $id_col_accession_name);
      $vcf_file = file_load($vcf);
      if (trim($form_state['values']["organism-$org_num"]['genotype']['files']['local_vcf']) != '') {
        $location = trim($form_state['values']["organism-$org_num"]['genotype']['files']['local_vcf']);
      }
      else {
        $location = tpps_get_location($vcf_file->uri);
      }
      $vcf_content = gzopen($location, 'r');
      $stocks = array();
      while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
        if (preg_match('/#CHROM/', $vcf_line)) {
          $vcf_line = explode("\t", $vcf_line);
          for ($j = 9; $j < count($vcf_line); $j++) {
            $stocks[] = trim($vcf_line[$j]);
          }
          break;
        }
      }

      if (count($stocks) == 0) {
        form_set_error(
          "$id][genotype][files][vcf",
          t("Genotype VCF File: unable to parse Plant Identifiers. "
          . "The format of your VCF file must be invalid")
        );
      }

      if (count($stocks) != 0) {
        $missing_plants = array();
        foreach ($stocks as $stock_id) {
          if (array_search($stock_id, $accession_ids) === FALSE) {
            $missing_plants[] = $stock_id;
          }
        }
        if (count($missing_plants) > 0) {
          $missing_plants = implode(', ', $missing_plants);
          form_set_error(
            "$id][genotype][files][vcf",
            t("Genotype VCF File: We found Plant Identifiers in your VCF file "
            . "that were not present in your accession file. "
            . "Please either add these plants to your accession file or "
            . "remove them from your VCF file. The missing plants are: "
            . "@missing_plants.", ['@missing_plants' => $missing_plants])
          );
        }
      }

      if (!form_get_errors()) {
        $form_state['values'][$id]['genotype']['files']['vcf_genotype_count'] = tpps_file_len($vcf);
      }
    }

    // Preserve file if it is valid.
    tpps_preserve_valid_file($form_state, $vcf, $org_num, "Genotype_VCF");
  }

  if (!empty($file_type['SNPs Genotype Assay']) and !$snps_assay) {
    tpps_form_error_required($form_state,
      [$id, 'genotype', 'files', 'snps-assay']
    );
  }
  elseif (!empty($file_type['SNPs Genotype Assay'])) {
    $headers = tpps_file_headers($snps_assay);
    $id_col_name = key($headers);
    while (($k = array_search(NULL, $headers))) {
      drupal_set_message(t('Following header column is Null which needs to be fixed. %data', array('%data' => $k)), 'error');
      unset($headers[$k]);
    }
    $num_columns = tpps_file_width($snps_assay) - 1;
    $num_unique_columns = count(array_unique($headers)) - 1;
    if ($num_unique_columns != $num_columns) {
      $duplicates = array_diff_assoc($headers, array_unique($headers));
      if (!empty($duplicates)) {
        drupal_set_message(t('Following header values are duplicate in provided snp file. %data', array('%data' => implode(',', $duplicates))), 'error');
      }
      form_set_error("$id][genotype][files][snps-assay", t("SNPs Assay file: some columns in the file you provided are missing or have duplicate header values. Please either enter valid header values for those columns or remove those columns, then reupload your file."));
    }

    if (!form_get_errors()) {
      $acc_no_header = $thirdpage['tree-accession'][$species_index]['file-no-header'];
      $missing_trees = tpps_compare_files($snps_assay, $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);
      if ($missing_trees !== array()) {
        $tree_id_str = implode(', ', $missing_trees);
        form_set_error("$id][genotype][files][snps-assay", t("SNPs Assay file: We detected Plant Identifiers that were not in your Plant Accession file. Please either remove these plants from your Genotype file, or add them to your Plant Accession file. The Plant Identifiers we found were: @tree_id_str", array('@tree_id_str' => $tree_id_str)));
      }
    }

    // Preserve file if it is valid.
    tpps_preserve_valid_file($form_state, $snps_assay, $org_num, "Genotype_SNPs_Assay");

    if (!form_get_errors()) {
      if (!empty($file_type['SNPs Associations']) and !$assoc_file) {
        form_set_error("$id][genotype][files][snps-association", t("SNPs Associations file: field is required."));
      }
      elseif (!empty($file_type['SNPs Associations'])) {
        $required_groups = array(
          'SNP ID' => array(
            'id' => array(1),
          ),
          'Scaffold' => array(
            'scaffold' => array(2),
          ),
          'Position' => array(
            'position' => array(3),
          ),
          'Allele' => array(
            'allele' => array(4),
          ),
          'Associated Trait' => array(
            'trait' => array(5),
          ),
          'Confidence Value' => array(
            'confidence' => array(6),
          ),
        );

        $file_element = $form[$id]['genotype']['files']['snps-association'];
        $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

        if (!form_get_errors()) {
          // Check that SNP IDs match Genotype Assay.
          $snps_id_col = $groups['SNP ID'][1];
          $assoc_no_header = $genotype['files']['snps-association-no-header'] ?? FALSE;

          $assay_snps = tpps_file_headers($snps_assay);
          unset($assay_snps[key($assay_snps)]);
          $assoc_snps = tpps_parse_file_column($assoc_file, $snps_id_col, $assoc_no_header);
          $missing_snps = array_diff($assoc_snps, $assay_snps);

          if ($missing_snps !== array()) {
            $snps_id_str = implode(', ', $missing_snps);
            form_set_error("$id][genotype][files][snps-association",
              t("SNP Association File: We detected SNP IDs that were not in "
              . "your Genotype Assay. Please either remove these SNPs from "
              . "your Association file, or add them to your Genotype Assay. "
              . "The SNP Identifiers we found were: @snps_id_str",
              ['@snps_id_str' => $snps_id_str]));
          }

          // Check that Phenotype names match phenotype metadata section.
          $trait_id_col = $groups['Associated Trait'][5];
          $association_phenotypes = tpps_parse_file_column($assoc_file, $trait_id_col, $assoc_no_header);

          $phenotype = $form_state['values'][$id]['phenotype'];
          $phenotype_meta = $phenotype['metadata'];
          $phenotype_number = $phenotype['phenotypes-meta']['number'];

          $phenotype_meta_names = array();
          $phenotype_name_col = $form_state['values'][$id]['phenotype']['metadata-groups']['Phenotype Id']['1'] ?? NULL;
          if (isset($phenotype_name_col)) {
            $phenotype_meta_names = tpps_parse_file_column($phenotype_meta, $phenotype_name_col);
          }

          for ($i = 1; $i <= $phenotype_number; $i++) {
            $phenotype_meta_names[] = $phenotype['phenotypes-meta'][$i]['name'];
          }

          $missing_phenotypes = array_diff($association_phenotypes, $phenotype_meta_names);
          if ($missing_phenotypes !== array()) {
            $phenotype_names_str = implode(', ', $missing_phenotypes);
            form_set_error("$id][genotype][files][snps-association",
              t("SNP Association File: We detected Associated Traits that were "
              . "not specified in the Phenotype Metadata Section. Please "
              . "either remove these Traits from your Association file, "
              . "or add them to your Phenotype Metadata section. The Trait "
              . "names we foud were: $phenotype_names_str")
            );
          }

          // Check that position values are correctly formatted.
          $position_col = $groups['Position'][3];
          $positions = tpps_parse_file_column($assoc_file, $position_col, $assoc_no_header);
          foreach ($positions as $position) {
            if (!preg_match('/^(\d+):(\d+)$/', $position)) {
              form_set_error("$id][genotype][files][snps-association",
                t('SNP Association File: We detected SNP positions that do '
                . 'not match the required format. '
                . 'The correct format is: "start:stop".')
              );
              break;
            }
          }
        }

        // Preserve file if it is valid.
        tpps_preserve_valid_file($form_state, $assoc_file, $org_num, "SNPs_Association");

        if (empty($genotype['files']['snps-association-type'])) {
          form_set_error("$id][genotype][files][snps-association-type",
            t("SNPs Association Type: field is required."));
        }

        if (empty($genotype['files']['snps-association-tool'])) {
          form_set_error("$id][genotype][files][snps-association-tool",
            t("SNPs Association Tool: field is required."));
        }

        if (!empty($genotype['files']['snps-pop-struct'])) {
          // Preserve file if it is valid.
          tpps_preserve_valid_file(
            $form_state,
            $genotype['files']['snps-pop-struct'],
            $org_num,
            "SNPs_Population_Structure"
          );
        }

        if (!empty($genotype['files']['snps-kinship'])) {
          // Preserve file if it is valid.
          tpps_preserve_valid_file(
            $form_state,
            $genotype['files']['snps-kinship'],
            $org_num,
            "SNPs_Kinship"
          );
        }
      }
    }
  }

  // [VS]
  elseif (!empty($genotyping_type['Genotyping Assay'])) {
    $file_field_name = 'assay-design';
    if (!$genotype['files'][$file_field_name]) {
      form_set_error("$id][genotype][files][$file_field_name",
        t("Assay Design file: field is required.")
      );
    }
    else {
      // Preserve file if it is valid.
      tpps_preserve_valid_file($form_state, $genotype['files'][$file_field_name],
        $org_num, "Genotype_Assay_Design");
    }
  }
  // [/VS]
  if (
    !empty($file_type['SSRs/cpSSRs Genotype Spreadsheet'])
    && !$genotype['files']['ssrs']
  ) {
    form_set_error("$id][genotype][files][ssrs]",
      t("SSRs/cpSSRs Spreadsheet: field is required."));
  }
  elseif (
    !empty($file_type['SSRs/cpSSRs Genotype Spreadsheet'])
    && !empty($genotype['files']['ploidy'])
  ) {
    $headers = tpps_file_headers($genotype['files']['ssrs']);
    $form_state['values']["organism-$org_num"]['genotype']['files']['ssrs-empty'] = $form["organism-$org_num"]['genotype']['files']['ssrs']['#value']['empty'];
    $genotype['files']['ssrs-empty'] = $form_state['values']["organism-$org_num"]['genotype']['files']['ssrs-empty'];
    $id_col_name = key($headers);
    while (($k = array_search(NULL, $headers))) {
      unset($headers[$k]);
    }
    $num_columns = tpps_file_width($genotype['files']['ssrs']) - 1;
    $num_unique_columns = count(array_unique($headers)) - 1;

    tpps_ssr_valid_ploidy($genotype['files']['ploidy'], $num_columns,
      $num_unique_columns, "$id][genotype][files][ssrs");

    // @todo Field 'ssr-extra-check' must be replaced with check if
    // Define SSRs/cpSSRs Type:
    // cpSSR or Both SSRs and cpSSRs.
    if (!empty($genotype['files']['ssr-extra-check'])) {
      if (empty($genotype['files']['extra-ssr-type'])) {
        form_set_error("$id][genotype][files][extra-ssr-type",
          t("Define Additional SSRs/cpSSRs Type: field is required.")
        );
      }

      if (!$genotype['files']['ssrs_extra']) {
        form_set_error("$id][genotype][files][ssrs_extra]", t("SSRs/cpSSRs Additional Spreadsheet: field is required."));
      }
      elseif (!empty($genotype['files']['extra-ploidy'])) {
        $headers = tpps_file_headers($genotype['files']['ssrs_extra']);
        $id_col_name = key($headers);
        while (($k = array_search(NULL, $headers))) {
          unset($headers[$k]);
        }
        $num_columns = tpps_file_width($genotype['files']['ssrs_extra']) - 1;
        $num_unique_columns = count(array_unique($headers)) - 1;

        tpps_ssr_valid_ploidy($genotype['files']['extra-ploidy'], $num_columns, $num_unique_columns, "$id][genotype][files][ssrs_extra");
      }
    }

    if (!form_get_errors()) {
      $acc_no_header = $thirdpage['tree-accession'][$species_index]['file-no-header'];
      $missing_trees = tpps_compare_files($genotype['files']['ssrs'], $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);

      if ($missing_trees !== array()) {
        $tree_id_str = implode(', ', $missing_trees);
        form_set_error("$id][genotype][files][ssrs", t("SSRs/cpSSRs Genotype Spreadsheet: We detected Plant Identifiers that were not in your Plant Accession file. Please either remove these plants from your Genotype file, or add them to your Plant Accession file. The Plant Identifiers we found were: @tree_id_str", array('@tree_id_str' => $tree_id_str)));
      }
    }

    if (!form_get_errors()) {
      $options = array(
        'empty' => $genotype['files']['ssrs-empty'] ?? NULL,
        'org_num' => $org_num,
      );
      tpps_file_iterator($genotype['files']['ssrs'], 'tpps_ssr_valid_values', $options);
    }

    // Preserve file if it is valid.
    tpps_preserve_valid_file($form_state, $genotype['files']['ssrs'], $org_num, "Genotype_SSR_Spreadsheet");
    if (!empty($genotype['files']['ssrs_extra'])) {
      tpps_preserve_valid_file($form_state, $genotype['files']['ssrs_extra'], $org_num, "Genotype_SSR_Additional_Spreadsheet");
    }
  }

  if (!empty($file_type['Indel Genotype Spreadsheet']) and !$genotype['files']['indels']) {
    form_set_error("$id][genotype][files][indels]", t("Indel Genotype Spreadsheet: field is required."));
  }
  elseif (!empty($file_type['Indel Genotype Spreadsheet'])) {
    $indel_fid = $genotype['files']['indels'];
    $headers = tpps_file_headers($indel_fid);
    $id_col_name = key($headers);
    while (($k = array_search(NULL, $headers))) {
      unset($headers[$k]);
    }
    $num_columns = tpps_file_width($indel_fid) - 1;
    $num_unique_columns = count(array_unique($headers)) - 1;

    if ($num_unique_columns != $num_columns) {
      form_set_error("$id][genotype][files][indels", t("Indel Genotype Spreadsheet: some columns in the file you provided are missing or have duplicate header values. Please either enter valid header values for those columns or remove those columns, then reupload your file."));
    }

    if (!form_get_errors()) {
      $acc_no_header = $thirdpage['tree-accession'][$species_index]['file-no-header'];
      $missing_trees = tpps_compare_files($indel_fid, $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);

      if ($missing_trees !== array()) {
        $tree_id_str = implode(', ', $missing_trees);
        form_set_error("$id][genotype][files][indels", t("Indel Genotype Spreadsheet: We detected Plant Identifiers that were not in your Plant Accession file. Please either remove these plants from your Genotype file, or add them to your Plant Accession file. The Plant Identifiers we found were: @tree_id_str", array('@tree_id_str' => $tree_id_str)));
      }
    }

    // Preserve file if it is valid.
    tpps_preserve_valid_file($form_state, $indel_fid, $org_num, "Genotype_Indel_Assay");
  }

  if (!empty($file_type['Other Marker Genotype Spreadsheet']) and !$genotype['files']['other']) {
    form_set_error("$id][genotype][files][other]", t("Other Marker Spreadsheet: field is required."));
  }
  elseif (!empty($file_type['Other Marker Genotype Spreadsheet'])) {
    if (array_key_exists('columns', $form[$id]['genotype']['files']['other'])) {
      $required_groups = array(
        'Tree Id' => array(
          'id' => array(1),
        ),
        'Genotype Data' => array(
          'data' => array(0),
        ),
      );

      $file_element = $form[$id]['genotype']['files']['other'];
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
      // Get Plant Id column name.
      if (!form_get_errors()) {
        $id_col_genotype_name = $groups['Tree Id']['1'];
      }
    }
    if (!array_key_exists('columns', $form[$id]['genotype']['files']['other'])) {
      $headers = tpps_file_headers($genotype['files']['other']);
      if (!form_get_errors()) {
        $id_col_genotype_name = key($headers);
      }
    }

    if (!form_get_errors()) {
      $acc_no_header = $thirdpage['tree-accession'][$species_index]['file-no-header'];
      $other_no_header = $genotype['files']['other-no-header'] ?? FALSE;
      $missing_trees = tpps_compare_files($other_file, $tree_accession_file, $id_col_genotype_name, $id_col_accession_name, $other_no_header, $acc_no_header);

      if ($missing_trees !== array()) {
        $tree_id_str = implode(', ', $missing_trees);
        form_set_error("$id][genotype][files][other", "Other Marker Genotype Spreadsheet: We detected Plant Identifiers that were not in your Plant Accession file. Please either remove these plants from your Genotype file, or add them to your Plant Accession file. The Plant Identifiers we found were: $tree_id_str");
      }
    }

    // Preserve file if it is valid.
    tpps_preserve_valid_file($form_state, $other_file, $org_num, "Genotype_Other_Marker_Spreadsheet");
  }
}

/**
 * Validates the environment section of the fourth page of the form.
 *
 * @param array $environment
 *   The form_state values of the environment fieldset for organism $id.
 * @param string $id
 *   The id of the organism fieldset being validated.
 */
function tpps_validate_environment(array &$environment, $id) {
  // Using cartograplant environment layers.
  $group_check = FALSE;
  $new_layers = array();
  foreach ($environment['env_layers_groups'] as $group_name => $group_id) {
    if (!empty($group_id)) {
      $group_check = TRUE;
      if ($group_name == 'WorldClim v.2 (WorldClim)') {
        $subgroups_query = db_select('cartogratree_layers', 'l')
          ->distinct()
          ->fields('l', array('subgroup_id'))
          ->condition('group_id', $group_id)
          ->execute();
        while (($subgroup = $subgroups_query->fetchObject())) {
          $subgroup_title = db_select('cartogratree_subgroups', 's')
            ->fields('s', array('subgroup_name'))
            ->condition('subgroup_id', $subgroup->subgroup_id)
            ->execute()
            ->fetchObject()->subgroup_name;
          if (!empty($environment['env_layers'][$subgroup_title])) {
            $new_layers[$subgroup_title] = $environment['env_layers'][$subgroup_title];
          }
        }
      }
      if ($group_name != 'WorldClim v.2 (WorldClim)') {
        $layer_query = db_select('cartogratree_layers', 'l')
          ->fields('l', array('title'))
          ->condition('group_id', $group_id)
          ->execute();
        while (($layer = $layer_query->fetchObject())) {
          if (!empty($environment['env_layers'][$layer->title])) {
            $new_layers[$layer->title] = $environment['env_layers'][$layer->title];
          }
        }
      }
    }
  }

  if (!empty($environment['env_layers']['other'])) {
    if (empty($environment['env_layers']['other_db'])) {
      form_set_error("$id][environment][env_layers][other_db", t('CartograPlant other environmental layer DB: field is required.'));
    }

    if (empty($environment['env_layers']['other_name'])) {
      form_set_error("$id][environment][env_layers][other_name", t('CartograPlant other environmental layer name: field is required.'));
    }

    if (empty($environment['env_layers']['other_params'])) {
      form_set_error("$id][environment][env_layers][other_params", t('CartograPlant other environmental layer parameters: field is required.'));
    }

    if (!form_get_errors()) {
      $new_layers['other'] = 'other';
      $new_layers['other_db'] = $environment['env_layers']['other_db'];
      $new_layers['other_name'] = $environment['env_layers']['other_name'];
    }
  }

  $environment['env_layers'] = $new_layers;

  if (!$group_check) {
    form_set_error("$id][environment][env_layers_groups", t('CartograPlant environmental layers groups: field is required.'));
  }
  elseif (empty($new_layers)) {
    form_set_error("$id][environment][env_layers", t('CartograPlant environmental layers: field is required.'));
  }
}

/**
 * This function processes a single row of a plant accession file.
 *
 * This function validates that the values in the provided SSR file are all
 * either non-negative or equal to the NA value. This function is meant to be
 * used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_ssr_valid_values($row, array &$options) {
  $id = array_shift($row);
  foreach ($row as $value) {
    if ($value < 0 and $value !== $options['empty']) {
      form_set_error("{$options['org_num']}-genotype-files-ssrs-{$id}", "SSRs Spreadsheet file: Some non-empty values are negative for plant \"{$id}\".");
      break;
    }
  }
}

/**
 * This function validates column counts for an SSR file based on ploidy.
 *
 * Form errors are thrown if the column counts are invalid for the specified
 * ploidy.
 *
 * @param string $ploidy
 *   The ploidy we are checking.
 * @param int $num_columns
 *   The total column count.
 * @param int $num_unique_columns
 *   The unique column count.
 * @param string $name
 *   The field name for use in form_set_error().
 */
function tpps_ssr_valid_ploidy($ploidy, $num_columns, $num_unique_columns, $name) {
  switch ($ploidy) {
    case 'Haploid':
      if ($num_unique_columns != $num_columns) {
        form_set_error($name, t("SSRs/cpSSRs Genotype Spreadsheet: some columns in the file you provided are missing or have duplicate header values. Please either enter header values for those columns or remove those columns, then reupload your file."));
      }
      break;

    case 'Diploid':
      if ($num_unique_columns != $num_columns and $num_columns / $num_unique_columns !== 2) {
        form_set_error($name, t("SSRs/cpSSRs Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file."));
      }
      elseif ($num_unique_columns == $num_columns and $num_columns % 2 !== 0) {
        form_set_error($name, t("SSRs/cpSSRs Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file."));
      }
      break;

    case 'Polyploid':
      if ($num_columns % $num_unique_columns !== 0) {
        form_set_error($name, t("SSRs/cpSSRs Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file."));
      }
      break;

    default:
      break;
  }
}

/**
 * Check if required field is not empty.
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $parents
 *   Path to field. For example:
 *   ['organism-1', 'genotype', 'files', 'marker-type'].
 *
 * @return bool
 *   Returns TRUE if required field is not empty and FALSE other was.
 */
function tpps_check_required(array $form_state, array $parents) {
  $value = drupal_array_get_nested_value($form_state['values'], $parents);
  if (empty($value)) {
    tpps_form_error_required($form_state, $parents);
    return FALSE;
  }
  return TRUE;
}

/**
 * Shows form_validation error message that field is required.
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $parents
 *   Path to field. For example:
 *   ['organism-1', 'genotype', 'files', 'marker-type'].
 */
function tpps_form_error_required(array $form_state, array $parents) {
  tpps_form_error($form_state, $parents, 'field is required.');
}

/**
 * Shows form validation error message.
 *
 * @param array $form_state
 *   Drupal Form API array with submitted values.
 * @param array $parents
 *   Path to field. For example:
 *   ['organism-1', 'genotype', 'files', 'marker-type'].
 * @param string $message
 *   Error message text.
 */
function tpps_form_error(array $form_state, array $parents, $message) {
  $field = drupal_array_get_nested_value($form_state['complete form'], $parents);
  if (!empty($field['#title'])) {
    $title = strtok($field['#title'], ':');
    form_set_error(
      implode('][', $parents),
      // @todo Should field name and message be separated?
      t('@title: @message', ['@title' => $title, '@message' => $message])
    );
  }
  else {
    watchdog('tpps', "Field didn't pass validation but it's missing at form."
      . '@parents', ['@parents' => implode(' > ', $parents)], WATCHDOG_ERROR
    );
  }
}
