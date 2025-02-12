<?php

/**
 * @file
 * Defines the data integrity checks for the fourth page of the form.
 */

module_load_include('inc', 'tpps', 'includes/form');

/**
 * Defines the data integrity checks for the fourth page of the form.
 *
 * Note: File size validation will be done in includes/file_utils.inc
 * in tpps_file_validate_columns().
 *
 * @param array $form
 *   The form that is being validated.
 * @param array $form_state
 *   The state of the form that is being validated.
 */
function tpps_page_4_validate_form(array &$form, array &$form_state) {
  if ($form_state['submitted'] != '1') {
    return;
  }
  $snps_fieldset = 'SNPs';
  $other_fieldset = 'other';
  unset($form_state['file_info'][TPPS_PAGE_4]);

  $form_values = $form_state['values'];
  $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];

  for ($i = 1; $i <= $organism_number; $i++) {
    $organism = &$form_state['values']["organism-$i"] ?? NULL;
    // Note: 1st item skipped because there is a checkbox which allows to
    // skip validation of non-first items so only them must be checked.
    //
    // Check if validation functions exists.
    $study_type_list = [];
    foreach (['phenotype', 'genotype', 'environment'] as $item) {
      if (!function_exists('tpps_validate_' . $item)) {
        // Dynamically built function names are:
        // tpps_validate_phenotype(),
        // tpps_validate_genotype(),
        // tpps_validate_environment().
        watchdog('tpps', 'Validation function for @study_type not found.',
          ['@study_type' => $item], WATCHDOG_ERROR
        );
        continue;
      }
      $study_type_list[] = $item;
    }

    foreach ($study_type_list as $item) {
      if ($i > 1) {
        if (($organism[$item . '-repeat-check'] ?? NULL) == '1') {
          // phenotype-repeat-check,
          // genotype-repeat-check,
          // environment-repeat-check.
          unset($organism[$item]);
        }
      }
      if (!empty($organism[$item])) {
        call_user_func_array(
          'tpps_validate_' . $item,
          [&$organism[$item], $i, $form, &$form_state]
        );
      }
    }
  }

  if (form_get_errors() && !$form_state['rebuild']) {
    $form_state['rebuild'] = TRUE;
    for ($i = 1; $i <= $organism_number; $i++) {
      tpps_validate_restore_file_field_on_form_rebuild($form, $form_state,
        ['organism-' . $i, 'phenotype', 'metadata']
      );
      tpps_validate_restore_file_field_on_form_rebuild($form, $form_state,
        ['organism-' . $i, 'phenotype', 'file']
      );
      tpps_validate_restore_file_field_on_form_rebuild($form, $form_state,
        ['organism-' . $i, 'genotype', $snps_fieldset, 'snps-assay']
      );
      // Note: this field will be relocated later.
      tpps_validate_restore_file_field_on_form_rebuild($form, $form_state,
        ['organism-' . $i, 'genotype', $other_fieldset, 'other']
      );
    }
  }

  // Validation passed and form is going to be submitted.
  // We shouldn't remove any files until validation passed.
  if (!form_get_errors()) {
    // We are removing genotype files here to allow on user to get exactly
    // the same form as was submitted and rmeove files only when they
    // definitly not needed.
    for ($i = 1; $i <= $organism_number; $i++) {
      $genotype = &$form_state['values']["organism-$i"]['genotype'];
      $genotyping_type = $genotype[$snps_fieldset]['genotyping-type'] ?? NULL;
      $file_type = $genotype[$snps_fieldset]['file-type'] ?? NULL;
      $does_study_include_snp_data
        = $genotype['does_study_include_snp_data'] ?? NULL;
      if (
        $does_study_include_snp_data == 'yes'
        && $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING
        && $file_type == TPPS_GENOTYPING_FILE_TYPE_VCF
      ) {
        // Remove files which was uploaded before settings on form was changed
        // and those files became useless but already uploaded to server.
        if (tpps_file_remove($genotype[$snps_fieldset]['snps-assay'])) {
          $genotype[$snps_fieldset]['snps-assay'] = 0;
        }
        if (tpps_file_remove($genotype[$snps_fieldset]['assay-design'])) {
          $genotype[$snps_fieldset]['assay-design'] = 0;
        }
      }
      else {



// @TODO Check why fields are missing on Page4 submit.
        //if (tpps_file_remove($genotype[$snps_fieldset]['vcf'])) {
        //  $genotype[$snps_fieldset]['vcf'] = 0;
        //}
      }

// @TODO Check why fields are missing on Page4 submit.

      // Remove SSR/cpSSR files which was uploaded but not in use.
      // $ssrs_fieldset = 'ssrs_cpssrs';
      //if (!empty($genotype['marker-type']['SSRs/cpSSRs'])) {
      //  if ($genotype['SSRs/cpSSRs'] == 'cpSSRs') {
      //    if (tpps_file_remove($genotype[$ssrs_fieldset]['ssrs'])) {
      //      $genotype[$ssrs_fieldset]['ssrs'] = 0;
      //    }
      //  }
      //  if (($genotype['SSRs/cpSSRs'] ?? NULL) == 'SSRs') {
      //    if (tpps_file_remove($genotype[$ssrs_fieldset]['ssrs_extra'])) {
      //      $genotype[$ssrs_fieldset]['ssrs_extra'] = 0;
      //    }
      //  }
      //}
    }
  }
}

/**
 * Validates the phenotype section of the fourth page of the form.
 *
 * @param array $phenotype
 *   The form_state values of the phenotype fieldset for organism $id.
 * @param int $org_num
 *   The id of the organism being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_phenotype(array &$phenotype, $org_num, array $form, array &$form_state) {
  $normal_check = $phenotype['normal-check'] ?? NULL;
  $iso_check = $phenotype['iso-check'] ?? NULL;
  $id = "organism-$org_num";
  $page3 = $form_state['saved_values'][TPPS_PAGE_3] ?? NULL;

  // Uncomment to block form submission and test validation.
  // form_set_error('DEBUG', 'Remove debug code.');

  if (empty($normal_check) && empty($iso_check)) {
    form_set_error("$id][phenotype][normal-check",
      t('Please choose at least one category of phenotypes to upload')
    );
  }

  if ($normal_check) {
    $phenotype_number = $phenotype['phenotypes-meta']['number'];
    // File Id of metadata file.
    $phenotype_meta = $phenotype['metadata'];
    $phenotype_file = $phenotype['file'];

    if (empty($phenotype_file)) {
      form_set_error("$id][phenotype][file",
        t('Phenotype File: field is required.')
      );
    }
    // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Phenotype Metafile was used.
    $is_metadata_file = (bool) ($phenotype['check'] && !empty($phenotype_file));
    if ($is_metadata_file) {
      module_load_include('inc', 'tpps', 'src/PhenotypeMeta.class');
      PhenotypeMeta::validate($org_num, $form, $form_state);
    }
    // Manually added Phenotype Metadata. File wasn't used.
    else {
      // @TODO Move to PhenotypeManual::validate().
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      for ($i = 1; $i <= $phenotype_number; $i++) {
        $current_phenotype = &$phenotype['phenotypes-meta']["$i"];
        // [VS] Synonym form.
        if (!empty($current_phenotype['synonym_id'])) {
          $synonym_name = $current_phenotype['synonym_name'];
          $synonym_description = $current_phenotype['synonym_description'];
          if ($synonym_name == '') {
            form_set_error("$id][phenotype][phenotypes-meta][$i][synonym_name",
              "Phenotype $i Name: field is required.");
          }
          if ($synonym_description == '') {
            form_set_error("$id][phenotype][phenotypes-meta][$i][synonym_description",
              "Phenotype $i Description: field is required.");
          }
          if (!empty($current_phenotype['synonym_id'])) {
            // Restore only if there is Synonym Id.
            tpps_synonym_restore_values($current_phenotype);
          }
        }

        // [/VS]
        // Main form.
        $name = $current_phenotype['name'];
        $description = $current_phenotype['description'];
        if ($name == '') {
          form_set_error(
            $id . '][phenotype][phenotypes-meta][' . $i . '][name',
            t(
              'Phenotype @phenotype_id Name: field is required.',
              ['@phenotype_id' => $i]
            )
          );
        }
        if ($description == '') {
          form_set_error(
            $id . '][phenotype][phenotypes-meta][' . $i . '][description',
            t('Phenotype @phenotype_id Description: field is required.',
              ['@phenotype_id' => $i]
            )
          );
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Check if phenotype name matches column names in Phenotype File.
        if (!form_get_errors() && !empty($phenotype_file)) {
          $file_header = tpps_file_get_header($phenotype_file);
          if (!empty($file_header) && !in_array(($name ?? NULL), $file_header)) {
            $message = t('Phenotype @phenotype_id Name: Name '
              . '"<strong>@phenotype_name</strong>" do not match any column name '
              . 'in Phenotype File.<br />Columns in file are: @column_list.',
              [
                '@phenotype_id' => $i,
                '@phenotype_name' => $name,
                '@column_list' => implode(', ', $file_header),
              ]
            );
            form_set_error("$id][phenotype][phenotypes-meta][$i][name", $message);
          }
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Validate 'Attribute'.
        if (!$current_phenotype['attribute']) {
          form_set_error("$id][phenotype][phenotypes-meta][$i][attribute",
            "Phenotype $i Attribute: field is required.");
        }
        $condition = (
          $current_phenotype['attribute'] == 'other'
          && $current_phenotype['attr-other'] == ''
        );
        if ($condition) {
          form_set_error("$id][phenotype][phenotypes-meta][$i][attr-other",
            "Phenotype $i Custom Attribute: field is required.");
        }
        // [VS]
        $unit = $current_phenotype['unit'];
        if ($unit == '') {
          form_set_error("$id][phenotype][phenotypes-meta][$i][unit",
            "Phenotype $i Unit: field is required.");
        }
        elseif ($unit == 'other') {
          if ($current_phenotype['unit-other'] == '') {
            form_set_error("$id][phenotype][phenotypes-meta][$i][unit-other",
              "Phenotype $i Custom Unit: field is required.");
          }
          else {
            // Create a record in 'Unit Warning' table for Custom Unit.
            db_merge('tpps_phenotype_unit_warning')
              ->key(['study_name' => $form_state['accession']])
              ->fields(['study_name' => $form_state['accession']])
              ->execute();
          }
        }
        // [/VS]

        $condition = (
          $current_phenotype['structure'] == 'other'
          && $current_phenotype['struct-other'] == ''
        );
        if ($condition) {
          form_set_error("$id][phenotype][phenotypes-meta][$i][struct-other",
            "Phenotype $i Custom Structure: field is required.");
        }
      }
    }

    if ($phenotype['time']['time-check']) {
      $time = &$form_state['values'][$id]['phenotype']['time'];
      foreach ($phenotype['time']['time_phenotypes'] as $key => $val) {
        if (!$val) {
          unset($time['time_phenotypes'][$key]);
          unset($time['time_values'][$key]);
        }
      }
      if (empty($time['time_phenotypes'])) {
        form_set_error("$id][phenotype][time][time_phenotypes",
          t("Time-based Phenotypes: field is required.")
        );
      }
    }

    if (!empty($phenotype_file)) {
      PhenotypeData::validateNormal($org_num, $form, $form_state);
    }
    if (!form_get_errors()) {
      tpps_preserve_valid_file($form_state,
        //Phenotype data file id.
        $phenotype['file'], $org_num, "Phenotype_Data");
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Iso/Mass Spectrometry.
  if ($iso_check) {
    PhenotypeData::validateIso($org_num, $form, $form_state);
    if (!form_get_errors()) {
      // Preserve file if it is valid.
      tpps_preserve_valid_file($form_state, $phenotype['iso'], $org_num, "Phenotype_Data");
    }
  }
}

/**
 * Validates the genotype section of the fourth page of the form.
 *
 * @param array $genotype
 *   The form_state values of the genotype fieldset for organism $id.
 *   $form_state['values']["organism-$i"]['genotype'].
 * @param int $org_num
 *   The id of the organism being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_genotype(array &$genotype, $org_num, array $form, array &$form_state) {
  tpps_validate_genotype_snps($genotype, $org_num, $form, $form_state);
  // Validate SSR/cpSSR fieldset fields.
  tpps_validate_genotype_ssr($genotype, $org_num, $form, $form_state);
  // Validate 'Other' fieldset fields.
  tpps_validate_genotype_other($genotype, $org_num, $form, $form_state);

  $snps_fieldset = 'SNPs';
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Note: This check was outside of the SNPs fieldset check. Maybe it's by
  // mistake but lets leave it as is.
  $accession = $form_state['accession'] ?? NULL;
  $submission = new Submission($accession);
  if (!empty($submission->state['vcf_replace'])) {
    foreach ($submission->state['vcf_replace'] as $org_num => $fid) {
      $file = file_load($fid ?? '');
      if ($file) {
        if ($file->filesize == 0) {
          form_set_error("$org_num][genotype][$snps_fieldset][local_vcf",
            t('Local VCF File: File is empty.')
          );
        }
        else {
          $form_state['values'][$org_num]['genotype'][$snps_fieldset]['vcf'] = $fid;
          $vcf = $fid;
          $form_state['values'][$org_num]['genotype'][$snps_fieldset]['local_vcf'] = NULL;
        }
      }
      else {
        // @TODO Should be $id instead of $org_num here?
        form_set_error("$orn_num][genotype][$snps_fieldset][local_vcf",
          t("Local VCF File: File could not be loaded properly.")
        );
      }
    }
  }
}

/**
 * Validates the genotype/SNPs section of the fourth page of the form.
 *
 * @param array $genotype
 *   The form_state values of the genotype fieldset for organism $id.
 *   $form_state['values']["organism-$i"]['genotype'].
 * @param int $org_num
 *   The id of the organism being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_genotype_snps(array &$genotype, $org_num, array $form, array &$form_state) {
  if (($genotype['does_study_include_snp_data'] ?? NULL) == "no") {
    return;
  }
  $id = "organism-$org_num";

  $snps_fieldset = 'SNPs';
  $snps = $genotype[$snps_fieldset] ?? NULL;
  $ref_genome = $snps['ref-genome'] ?? NULL;

  $genotyping_type = $snps['genotyping-type'] ?? NULL;
  // WARNING: 'maker-type' is array because multiple values could be selected.
  $marker_type = $genotype['marker-type'] ?? NULL;
  $file_type = $snps['file-type'] ?? NULL;
  // File fields:
  $vcf = $snps['vcf'] ?? 0;
  $snps_assay = $snps['snps-assay'] ?? 0;
  $page3 = $form_state['saved_values'][TPPS_PAGE_3];

  // [VS]
  $is_step2_genotype = in_array(
    $form_state['saved_values'][TPPS_PAGE_2]['data_type'],
    [
      'Genotype x Environment',
      'Genotype x Phenotype x Environment',
      'Genotype x Phenotype',
    ]
  );
  $species_index = empty($page3['tree-accession']['check']) ? 'species-1' : "species-$org_num";
  $tree_accession_file = $page3['tree-accession'][$species_index]['file'];
  $id_col_accession_name = $page3['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate 'Reference Assembly used' field.
  // This field must be shown on any value of 'Marker Type' field.
  if (!$ref_genome) {
    tpps_form_error_required($form_state,
      [$id, 'genotype', 'SNPs', 'ref-genome']
    );
  }
  elseif ($ref_genome === 'bio') {
    tpps_is_required_field_empty(
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
  // End of 'Reference Assembly used' field validation.
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if ($is_step2_genotype) {
    tpps_is_required_field_empty($form_state,
      [$id, 'genotype', $snps_fieldset, 'genotyping-type']
    );
  }

  if (!tpps_is_required_field_empty($form_state,
    [$id, 'genotype', $snps_fieldset, 'genotyping-design'])
  ) {
    if ($snps['genotyping-design'] == TPPS_GENOTYPING_DESIGN_GBS) {
      $condition = (
        !tpps_is_required_field_empty(
          $form_state, [$id, 'genotype', $snps_fieldset, 'GBS']
        )
        // 5 = 'Other'
        && $snps['GBS'] == '5'
      );
      if ($condition) {
        !tpps_is_required_field_empty(
          $form_state, [$id, 'genotype', $snps_fieldset, 'GBS-other']
        );
      }
    }
    elseif ($snps['genotyping-design'] == TPPS_GENOTYPING_DESIGN_TARGETED_CAPTURE) {
      $condition = (
        !tpps_is_required_field_empty(
          $form_state, [$id, 'genotype', $snps_fieldset, 'targeted-capture']
        )
        && $snps['targeted-capture'] == '2'
      );
      if ($condition) {
        !tpps_is_required_field_empty($form_state,
          [$id, 'genotype', $snps_fieldset, 'targeted-capture-other']
        );
      }
    }
  }

  // Validate 'uploaded VCF' field.
  if ($file_type == TPPS_GENOTYPING_FILE_TYPE_VCF) {
    // Validate 'Remote/cluster VCF' field.
    if (
      $snps['vcf_file-location'] == 'remote'
      && trim($snps['local_vcf']) == ''
      && !$vcf
    ) {
      tpps_form_error_required($form_state,
        [$id, 'genotype', $snps_fieldset, 'local_vcf']
      );
    }
    if (!$vcf && $snps['vcf_file-location'] == 'local') {
      // && trim($form_state['values'][$id]['genotype'][$snps_fieldset]['local_vcf']) == ''
      tpps_form_error_required($form_state,
        [$id, 'genotype', $snps_fieldset, 'vcf']
      );
    }
    else {
      if (
        !empty($assembly) && !form_get_errors()
        && in_array($ref_genome, ['manual', 'manual2', 'url'])
      ) {
        if (trim($snps['local_vcf']) != '') {
          $local_vcf_path = trim($snps['local_vcf']);
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
              form_set_error("$id][genotype][$snps_fieldset][vcf",
                t("VCF File: scaffold @scaffold_id not found in assembly file(s)",
                array('@scaffold_id' => $scaffold_id))
              );
            }
          }
        }

      }
      // @TODO Minor. Find better way to get study accession. Maybe it should
      // be pass as function's argument?
      $accession = $form_state['accession'];
      $submission = new Submission($accession);

    //dpm( $submission->state['vcf_replace'], 1);
    //dpm( ($snps['local_vcf']), 2);
    //dpm( $submission->state['vcf_validated'], 3);


      // VCF pre-validation is missing.
      if (
        empty($submission->state['vcf_replace'])
        && trim($snps['local_vcf']) != ''
        && ($submission->state['vcf_validated'] ?? NULL) !== TRUE
      ) {
        form_set_error(
          "$id][genotype][$snps_fieldset][local_vcf",
          t("Local VCF File: File needs to be pre-validated. "
          . "Please click on Pre-validate my VCF files button at the bottom.")
        );
      }
      // Pre-validation failed and there are an errors.
      if (
        !empty($submission->state['vcf_validated'])
        && $submission->state['vcf_validated'] === TRUE
        && empty($submission->state['vcf_val_errors'])
      ) {
        drupal_set_message(t('VCF files pre-validated. Skipping VCF file validation'));
      }
      // No pre-validation errors and no other form validation errors.
      elseif (!form_get_errors()) {
        $accession_ids = tpps_parse_file_column($tree_accession_file, $id_col_accession_name);
        if ($vcf) {
          $vcf_file = file_load($vcf);
        }
        if (trim($snps['local_vcf']) != '') {
          $location = trim($snps['local_vcf']);
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
            "$id][genotype][$snps_fieldset][vcf",
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
              "$id][genotype][$snps_fieldset][vcf",
              t("Genotype VCF File: We found Plant Identifiers in your VCF file "
              . "that were not present in your accession file. "
              . "Please either add these plants to your accession file or "
              . "remove them from your VCF file. The missing plants are: "
              . "@missing_plants.", ['@missing_plants' => $missing_plants])
            );
          }
        }

        if (!form_get_errors()) {
          // WARNING: this is the only one use of this field in code.
          // This filed name could be dynamically build so it was left as is
          // and 'files' wasn't replaced with 'SNPs'.
          $genotype['files']['vcf_genotype_count'] = tpps_file_len($vcf);
        }
      }

      // Preserve file if it is valid.
      tpps_preserve_valid_file($form_state, $vcf, $org_num, "Genotype_VCF");
    }
  }

  SnpAssay::validate($org_num, $form, $form_state);
  SnpAssociation::validate($org_num, $form, $form_state);
  // AssayDesign file requires 'SNP Association' and 'SNP Assay' files.
  AssayDesign::validate($org_num, $form, $form_state);
  SnpsPopulationStructure::validate($org_num, $form, $form_state);
  SnpsKinship::validate($org_num, $form, $form_state);
}

/**
 * Validates the environment section of the fourth page of the form.
 *
 * @param array $environment
 *   The form_state values of the environment fieldset for organism $id.
 * @param int $org_num
 *   The id of the organism being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_environment(array &$environment, $org_num, array $form, array &$form_state) {
  $id = "organism-$org_num";
  // Using cartograplant environment layers.
  $group_check = FALSE;
  $new_layers = array();
  foreach (($environment['env_layers_groups'] ?? []) as $group_name => $group_id) {
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
      form_set_error("$id][environment][env_layers][other_db",
        t('CartograPlant other environmental layer DB: field is required.')
      );
    }

    if (empty($environment['env_layers']['other_name'])) {
      form_set_error("$id][environment][env_layers][other_name",
        t('CartograPlant other environmental layer name: field is required.')
      );
    }

    if (empty($environment['env_layers']['other_params'])) {
      form_set_error("$id][environment][env_layers][other_params",
        t('CartograPlant other environmental layer parameters: field is required.')
      );
    }

    if (!form_get_errors()) {
      $new_layers['other'] = 'other';
      $new_layers['other_db'] = $environment['env_layers']['other_db'];
      $new_layers['other_name'] = $environment['env_layers']['other_name'];
    }
  }

  $environment['env_layers'] = $new_layers;

  if (!$group_check) {
    form_set_error("$id][environment][env_layers_groups",
      t('CartograPlant environmental layers groups: field is required.')
    );
  }
  elseif (empty($new_layers)) {
    form_set_error("$id][environment][env_layers",
      t('CartograPlant environmental layers: field is required.')
    );
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
  $ssrs_fieldset = 'ssrs_cpssrs';
  foreach ($row as $value) {
    if ($value < 0 and $value !== $options['empty']) {
      form_set_error(
        "{$options['org_num']}-genotype-$ssrs_fieldset-ssrs-{$id}",
        'SSRs Spreadsheet file: Some non-empty values are negative for plant "'
        . $id . '".'
      );
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
 * @param int $org_num
 *   Ordinal number of organism.
 * @param string $field_name
 *   Ploidy field name.
 */
function tpps_ssr_valid_ploidy($ploidy, $num_columns, $num_unique_columns, $org_num, $field_name) {
  $ssrs_fieldset = 'ssrs_cpssrs';
  $id = "organism-$org_num";

  if ($field_name == 'ssrs') {
    $title = 'SSRs Genotype Spreadsheet';
  }
  elseif ($field_name == 'ssrs_extra') {
    $title = 'cpSSRs Genotype Spreadsheet';
  }
  switch ($ploidy) {
    case 'Haploid':
      if ($num_unique_columns != $num_columns) {
        form_set_error("$id][genotype][$ssrs_fieldset][$field_name",
          t("@title: some columns in the file you provided are missing or "
          . "have duplicate header values. Please either enter header "
          . "values for those columns or remove those columns, "
          . "then reupload your file.",
          ['@title' => $title]
        ));
      }
      break;

    case 'Diploid':
      if (
        $num_unique_columns != $num_columns
        and $num_columns / $num_unique_columns !== 2
      ) {
        form_set_error("$id][genotype][$ssrs_fieldset][$field_name",
          t("@title: There is either an invalid number of columns in your file, "
          . "or some of your columns are missing values. "
          . "Please review and reupload your file.",
          ['@title' => $title]
        ));
      }
      elseif (
        $num_unique_columns == $num_columns
        and $num_columns % 2 !== 0
      ) {
        form_set_error("$id][genotype][$ssrs_fieldset][$field_name",
          t("@title: There is either an invalid number of columns in your file, "
          . "or some of your columns are missing values. "
          . "Please review and reupload your file.",
          ['@title' => $title]
        ));
      }
      break;

    case 'Polyploid':
      if ($num_columns % $num_unique_columns !== 0) {
        form_set_error("$id][genotype][$ssrs_fieldset][$field_name",
          t("@title: There is either an invalid number of columns in your file, "
          . "or some of your columns are missing values. "
          . "Please review and reupload your file.",
          ['@title' => $title]
        ));
      }
      break;

    default:
      break;
  }
}

/**
 * Validate SSRs and cpSSRs fields.
 *
 * @param array $form_state
 *   Drupal Form API array.
 * @param int $org_num
 *   Ordinal number of organism.
 * @param string $field_name
 *   Field name. For example: 'ssrs', 'ssrs_extra'.
 *
 * @TODO Find better name for function.
 */
function tpps_validate_ssr(array &$form_state, $org_num, $field_name) {
  $id = 'organism-' . $org_num;
  $genotype = $form_state['values'][$id]['genotype'];
  $page3 = $form_state['saved_values'][TPPS_PAGE_3];
  $species_index = empty($page3['tree-accession']['check']) ? 'species-1' : "species-$org_num";
  $tree_accession_file = $page3['tree-accession'][$species_index]['file'];
  $id_col_accession_name = $page3['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

  $ploidy_field_name = 'ploidy';
  if ($field_name == 'ssrs') {
    $prefix = 'Genotype_SSR_Spreadsheet';
  }
  elseif ($field_name == 'ssrs_extra') {
    $prefix = 'Genotype_SSR_Additional_Spreadsheet';
  }

  $ssrs_fieldset = 'ssrs_cpssrs';
  $condition = (
    !tpps_is_required_field_empty($form_state,
      [$id, 'genotype', $ssrs_fieldset, $ploidy_field_name]
    )
    && !tpps_is_required_field_empty($form_state,
      [$id, 'genotype', $ssrs_fieldset, $field_name]
    )
  );
  if ($condition) {
    // Required fields are not empty.
    $headers = tpps_file_headers($genotype[$ssrs_fieldset][$field_name]);
    $id_col_name = key($headers);
    while (($k = array_search(NULL, $headers))) {
      unset($headers[$k]);
    }

    if (isset($genotype[$ssrs_fieldset][$ploidy_field_name])) {
      tpps_ssr_valid_ploidy(
        $genotype[$ssrs_fieldset][$ploidy_field_name],
        // Number of columns.
        (tpps_file_width($genotype[$ssrs_fieldset][$field_name]) - 1),
        // Number of unique columns.
        (count(array_unique($headers)) - 1),
        $org_num,
        $field_name
      );
    }
    // Check missing trees.
    if (!form_get_errors()) {
      // @TODO Reuse SnpAssay::validateTrees();
      $missing_trees = tpps_compare_files(
        $genotype[$ssrs_fieldset][$field_name],
        $tree_accession_file,
        $id_col_name,
        $id_col_accession_name,
        FALSE,
        $page3['tree-accession'][$species_index]['file-no-header']
      );

      if ($missing_trees !== []) {
        $tree_id_str = implode(', ', $missing_trees);
        form_set_error("$id][genotype][$ssrs_fieldset][$field_name", t(
          "SSRs/cpSSRs Genotype Spreadsheet: "
          . "We detected Plant Identifiers that were not in your "
          . "Plant Accession file. Please either remove these plants from "
          . "your Genotype file, or add them to your Plant Accession file. "
          . "The Plant Identifiers we found were: @tree_id_str",
          ['@tree_id_str' => $tree_id_str]
        ));
      }
    }
    if (!form_get_errors()) {
      $options = [
        'empty' => $genotype[$ssrs_fieldset][$field_name . '-empty'] ?? NULL,
        'org_num' => $org_num,
      ];
      tpps_file_iterator(
        $genotype[$ssrs_fieldset][$field_name],
        'tpps_ssr_valid_values',
        $options
      );
      // Preserve file if it is valid.
      tpps_preserve_valid_file(
        $form_state,
        $genotype[$ssrs_fieldset][$field_name],
        $org_num,
        $prefix
      );
    }
  }
}

/**
 * Checks if names in Phenotype Metadata File and Phenotype File matches.
 *
 * File Iterator callback for Phenotype Metadata File.
 * WARNING:
 * Only first error message will be shown because form_set_error() do not use
 * arrays for messages but stores only singe error message.
 *
 * @param mixed $row
 *   Single row from Phenotype File.
 * @param array $options
 *   Options with the keys:
 *   - 'phenotypes' - list of Phenotype names in Phenotype Metadata File.
 *   - 'column_name' - column name (for example 'A') which contains Phenotype
 *     Names in Phenotype File.
 *   - 'organism_name' - Organism Name used in HTML Forms. Eg., 'organism-1'.
 */
function tpps_validate_metafile_phenotype_names($row, array $options = []) {
  $file_header = $options['phenotypes'];
  $column_name = $options['column_name'];
  if (!in_array($row[$column_name], $file_header)) {
    $organism_name = $options['organism_name'];
    // @TODO Minor. Get Organism Name for this message.
    $message = t('Phenotype Metadata File for organism '
      . '#<strong>@organism_id</strong> : Phenotype name '
      . '"<strong>@phenotype_name</strong>" from metafile was NOT found in '
      . 'Phenotype File.<br />Phenotype File columns: @column_list.',
      [
        '@organism_id' => str_replace('organism-', '', $organism_name),
        '@phenotype_name' => $row[$column_name],
        '@column_list' => implode(', ', $file_header),
      ]
    );
    form_set_error($organism_name . '][phenotype][metadata', $message);
  }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Genotype sub-sections.

/**
 * Validates the genotype/other section of the 4th page of the form.
 *
 * @param array $genotype
 *   The form_state values of the genotype fieldset for organism $id.
 *   $form_state['values']["organism-$i"]['genotype'].
 * @param int $org_num
 *   The id of the organism being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_genotype_other(array &$genotype, $org_num, array $form, array &$form_state) {
  if (($genotype['does_study_include_other_genotypic_data'] ?? NULL) == 'no') {
    return;
  }
  $other_fieldset = 'other';
  $id = "organism-$org_num";

  if (tpps_is_required_field_empty(
    $form_state, [$id, 'genotype', $other_fieldset, 'other']
  )) {
    // Validation failed and no need to do extra checks.
    return;
  }
  $other_file = $genotype[$other_fieldset]['other'] ?? 0;
  $page3_values = $form_state['saved_values'][TPPS_PAGE_3];
  $species_index = empty($page3_values['tree-accession']['check'])
    ? 'species-1' : "species-$org_num";
  $tree_accession_file = $page3_values['tree-accession'][$species_index]['file'];
  $id_col_accession_name = $page3_values['tree-accession'][$species_index]
    ['file-groups']['Tree Id']['1'];

  // ? [VS] Should $form_state be used instead of $form here?
  if (array_key_exists('columns', $form[$id]['genotype'][$other_fieldset]['other'])) {
    $required_groups = [
      'Tree Id' => ['id' => [1]],
      'Genotype Data' => ['data' => [0]],
    ];
    $file_element = $form[$id]['genotype'][$other_fieldset]['other'];
    $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
    // Get Plant Id column name.
    if (!form_get_errors()) {
      $id_col_genotype_name = $groups['Tree Id']['1'];
    }
  }
  if (!array_key_exists('columns', $form[$id]['genotype'][$other_fieldset]['other'])) {
    $headers = tpps_file_headers($genotype[$other_fieldset]['other']);
    if (!form_get_errors()) {
      $id_col_genotype_name = key($headers);
    }
  }

  if (!form_get_errors()) {
    // @TODO Reuse SnpAssay::validateTrees();
    $acc_no_header = $page3_values['tree-accession'][$species_index]['file-no-header'];
    $other_no_header = $genotype[$other_fieldset]['other']['no-header'] ?? FALSE;
    $missing_trees = tpps_compare_files(
      $other_file,
      $tree_accession_file,
      $id_col_genotype_name,
      $id_col_accession_name,
      $other_no_header,
      $acc_no_header
    );
    if ($missing_trees !== []) {
      $tree_id_str = implode(', ', $missing_trees);
      form_set_error("$id][genotype][$other_fieldset][other",
        "Other Marker Genotype Spreadsheet: "
        . "We detected Plant Identifiers that were not in your "
        . "Plant Accession file. Please either remove these plants "
        . "from your Genotype file, or add them to your Plant Accession "
        . "file. The Plant Identifiers we found were: $tree_id_str"
      );
    }
  }
  // Preserve file if it is valid.
  tpps_preserve_valid_file(
    $form_state,
    $other_file,
    $org_num,
    'Genotype_Other_Marker_Spreadsheet'
  );
}

/**
 * Validates the genotype/ssr section of the fourth page of the form.
 *
 * @param array $genotype
 *   The form_state values of the genotype fieldset for organism $id.
 *   $form_state['values']["organism-$i"]['genotype'].
 * @param int $org_num
 *   The id of the organism being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_genotype_ssr(array &$genotype, $org_num, array $form, array &$form_state) {
  $id = "organism-$org_num";

  if (($genotype['does_study_include_ssr_cpssr_data'] ?? NULL) == "yes") {
    $ssrs_fieldset = 'ssrs_cpssrs';
    $field_name = 'SSRs/cpSSRs';
    tpps_is_required_field_empty($form_state,
      [$id, 'genotype', $ssrs_fieldset, $field_name]
    );
    $field_value = drupal_array_get_nested_value($genotype,
      [$ssrs_fieldset, $field_name]
    );

    if (in_array($field_value, ['cpSSRs', 'Both SSRs and cpSSRs'])) {
      tpps_validate_ssr($form_state, $org_num, 'ssrs_extra');
    }
    if (in_array($field_value, ['SSRs', 'Both SSRs and cpSSRs'])) {
      tpps_validate_ssr($form_state, $org_num, 'ssrs');
    }
  }
}

/**
 * Restores 'upload' and 'columns' sub-elements for file fields.
 *
 * Restores value of 'upload' sub-element (which is file id) and it's DOM id.
 * Restores 'columns' sub-element value.
 *
 * @param array $form
 *   Drupal Form API array.
 * @param array $form_state
 *   Drupal Form API State array.
 * @param array $parents
 *   File field parents.
 */
function tpps_validate_restore_file_field_on_form_rebuild(array &$form, array &$form_state, array $parents) {
  $new_form = &drupal_static(__FUNCTION__);
  if (empty($new_form)) {
    $new_form = drupal_rebuild_form('tpps_main', $form_state);
  }

  // @todo Minor. Use static caching for $new_form and create it inside function.
  $debug_mode = FALSE;
  $key_exists = NULL;
  $new_key_exists = NULL;
  $element = &drupal_array_get_nested_value($form, $parents, $key_exists);
  $new_element = &drupal_array_get_nested_value($new_form, $parents, $new_key_exists);
  if (!$key_exists || empty($element) || !$new_key_exists || empty($new_element)) {
    return;
  }
  foreach (['upload', 'columns'] as $field) {
    if (isset($element[$field]) && isset($new_element[$field])) {
      if ($debug_mode) {
        $diff = array_diff(
          array_map('serialize', $element[$field]),
          array_map('serialize', $new_element[$field])
        );
        dpm(print_r(array_map('unserialize', $diff), 1), $field . ' diff (before)');
      }
      // 'upload' will get attached JS setting which has element's id and
      // allowed file extensions. That's why DOM id of the element must be
      // restored to original to do not have suffixes like '--2' and so on.
      // 'columns' will get huge array with column's data types.
      $element[$field] = $new_element[$field];
      // Changes DOM id of the fieldset for 'columns' and id of the file-field.
      // On rebuild those fields gets extra suffix like '--2'.
      $element[$field]['#id']
        = 'edit-' . implode('-', array_merge($parents, [$field]));
    }
  }
}
