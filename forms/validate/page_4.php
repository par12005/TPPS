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
 * if function tpps_file_validate_columns().
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

  if (form_get_errors() and !$form_state['rebuild']) {
    $form_state['rebuild'] = TRUE;
    $new_form = drupal_rebuild_form('tpps_main', $form_state, $form);

    for ($i = 1; $i <= $organism_number; $i++) {
      if (isset($new_form["organism-$i"]['phenotype']['metadata']['upload'])) {
        $form["organism-$i"]['phenotype']['metadata']['upload']
          = $new_form["organism-$i"]['phenotype']['metadata']['upload'];
        $form["organism-$i"]['phenotype']['metadata']['upload']['#id']
          = "edit-organism-$i-phenotype-metadata-upload";
      }
      if (isset($new_form["organism-$i"]['phenotype']['metadata']['columns'])) {
        $form["organism-$i"]['phenotype']['metadata']['columns']
          = $new_form["organism-$i"]['phenotype']['metadata']['columns'];
        $form["organism-$i"]['phenotype']['metadata']['columns']['#id']
          = "edit-organism-$i-phenotype-metadata-columns";
      }

      if (isset($form["organism-$i"]['phenotype']['file'])) {
        foreach (['upload', 'columns'] as $field) {
          $form["organism-$i"]['phenotype']['file'][$field]
            = $new_form["organism-$i"]['phenotype']['file'][$field];
          $form["organism-$i"]['phenotype']['file'][$field]['#id']
            = "edit-organism-$i-phenotype-file-$field";
        }
      }

      foreach (['snps-assay'] as $type) {
        foreach (['upload', 'columns'] as $field) {
          if (
            isset($form["organism-$i"]['genotype'][$snps_fieldset][$type][$field])
            && isset($new_form["organism-$i"]['genotype'][$snps_fieldset][$type][$field])
          ) {
            $form["organism-$i"]['genotype'][$snps_fieldset][$type][$field]
              = $new_form["organism-$i"]['genotype'][$snps_fieldset][$type][$field];
            $form["organism-$i"]['genotype'][$snps_fieldset][$type][$field]['#id']
              = "edit-organism-$i-genotype-snps-{$type}-{$field}";
          }
        }
      }

      // Note: this field will be relocated later.
      $other_fieldset = 'other';
      foreach (['other'] as $type) {
        foreach (['upload', 'columns'] as $field) {
          if (
            isset($form["organism-$i"]['genotype'][$other_fieldset][$type][$field])
            && isset($new_form["organism-$i"]['genotype'][$other_fieldset][$type][$field])
          ) {
            $form["organism-$i"]['genotype'][$other_fieldset][$type][$field]
              = $new_form["organism-$i"]['genotype'][$other_fieldset][$type][$field];
            $form["organism-$i"]['genotype'][$other_fieldset][$type][$field]['#id']
              = "edit-organism-$i-genotype-${other_fieldset}-{$type}-{$field}";
          }
        }
      }
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
      // Clear manually added metadata to show correct data at edit page.
      if (!empty($phenotype['phenotypes-meta']['number'])) {
        for ($i = 1; $i <= $phenotype_number; $i++) {
          unset($phenotype['phenotypes-meta'][$i]);
        }
        $phenotype['phenotypes-meta']['number'] = 0;
      }
      if (empty($phenotype['metadata'])) {
        // $phenotype['metadata'] holds File Id of Phenotype Metadata file.
        // @TODO Use:
        // tpps_form_error_required($form_state, [$id, 'phenotype', 'metadata']);
        form_set_error("$id][phenotype][metadata",
          t('Phenotype Metadata File: field is required.')
        );
      }

      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Metadata file was used.
      else {
        // Check if number of Phenotypes matches number of phenotypes in file.
        // Note: number of phenotypes not equal to number of columns.
        if (!form_get_errors() && !empty($phenotype_file)) {
          $file_header = tpps_file_get_header($phenotype_file);
          $metadata_file_len = tpps_file_len($phenotype_meta);
          if (
            is_array($file_header)
            && $file_phenotypes_count = count($file_header)
          ) {
            if ($metadata_file_len != ($file_phenotypes_count - 1)) {
              $message = t('Number of phenotypes in Phenotype Metadata file '
                . ' NOT matches number of columns in Phenotype file.'
                . '<br />Number of phenotypes in Metadata File: <strong>@count</strong>.'
                . '<br />Number of phenotypes in Phenotype File: <strong>@file_count</strong>.',
                [
                  '@count' => $metadata_file_len,
                  '@file_count' => ($file_phenotypes_count - 1),
                ]
              );
              form_set_error("$id][phenotype][file", $message);
            }
          }
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        $required_groups = [
          'Phenotype Id' => ['id' => [1]],
          'Attribute' => ['attr' => [2]],
          'Description' => ['desc' => [3]],
          'Unit' => ['unit' => [4]],
          'Structure' => ['structure' => [5]],
        ];
        $file_element = $form[$id]['phenotype']['metadata'];
        $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
        if (!form_get_errors()) {
          // Get phenotype name column.
          $phenotype_name_col = $groups['Phenotype Id']['1'];
          // Preserve file if it is valid.
          tpps_preserve_valid_file(
            $form_state,
            $form_state['values'][$id]['phenotype']['metadata'],
            $org_num,
            "Phenotype_Metadata"
          );
        }
      }
      // Do not allow empty units in metadata file.
      if ($groups = $phenotype['metadata-groups'] ?? NULL) {
        $columns = [
          'name' => $groups['Phenotype Id']['1'],
          'attr' => $groups['Attribute']['2'],
          'desc' => $groups['Description']['3'],
          'unit' => $groups['Unit']['4'],
        ];
        $meta_options = [
          'meta_columns' => $columns,
          'id' => $id,
        ];
        tpps_file_iterator($phenotype_meta, 'tpps_unit_validate_metafile', $meta_options);
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
        // Validate if metafile matches phenotype file using columns.
        if ($file_header) {
          // $form_state['values'] has no value but $form_state['input']
          // has is so we will use it.
          $phenotype_no_header = $form_state['input'][$id]['phenotype']['file']['no-header'] ?? NULL;
          // Check if checkbox 'My file has no header row' wasn't checked.
          if ($phenotype_no_header) {
            // @TODO Check if phenotype file could be without header?
            drupal_set_message(
              t('Phenotype file has no header in 1st row so validation was NOT'
              . ' completed. Please check if Phenotype Names in both files are'
              . ' the same.'),
              'warning'
            );
          }
          else {
            $phenotype_metadata_fid = $phenotype['metadata'];
            // Check if list of phenotype names from metafile matches column names
            // in phenotype file.
            // $file_header contains list of phenotype file column names.
            $options = [
              'columns' => [$columns['name']],
              // List of phenotype file column names.
              'phenotypes' => $file_header,
              // $id is a string (not integer). Example: 'organism-1'.
              'organism_name' => $id,
              'column_name' => $columns['name'],
            ];
            tpps_file_iterator(
              $phenotype_metadata_fid,
              'tpps_validate_metafile_phenotype_names',
              $options
            );
          }
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      }
      else {
        form_set_error("$id][phenotype][metadata",
          t("Phenotype Metadata File: No groups found.")
        );
      }
    }
    // Manually added Phenotype Metadata. File wasn't used.
    else {
      // $phenotype['metadata'] could have Phenotype Metadata File Id if user
      // first uploaded file and then decided to manually add phenotypes.
      // We need to remove this file if it exists.
      if (!empty($phenotype['metadata'])) {
        // Remove already uploaded file.
        $file = tpps_file_load(($phenotype['metadata'] ?? ''));
        file_delete($file);
        // Clear metadatafile field.
        unset($phenotype['metadata']);
      }
      // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      // Check number of phenotypes.
      // Note: number of phenotypes not equal to number of columns.
      if (!form_get_errors() && !empty($phenotype_file)) {
        $file_header = tpps_file_get_header($phenotype_file);
        if (
          is_array($file_header)
          && $file_phenotypes_count = count($file_header)
        ) {
          if ($phenotype_number != ($file_phenotypes_count - 1)) {
            $message = t('Number of phenotypes NOT matches number of columns '
                . 'in Phenotype File.'
                . '<br />Number of manually added phenotypes: <strong>@count</strong>.'
                . '<br />Number of phenotypes in Phenotype File: <strong>@file_count</strong>.',
                [
                  '@count' => $phenotype_number,
                  '@file_count' => ($file_phenotypes_count - 1),
                ]
              );
            form_set_error("$id][phenotype][file", $message);
          }
        }
      }
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
      $required_groups = [
        'Tree Identifier' => ['id' => [1]],
        'Phenotype Data' => ['phenotype-data' => [0]],
      ];
      if ($phenotype['format'] != 0) {
        $required_groups = [
          'Tree Identifier' => ['id' => [1]],
          'Phenotype Name/Identifier' => ['phenotype-name' => [2]],
          'Phenotype Value(s)' => ['val' => [3]],
        ];
      }

      $file_element = $form[$id]['phenotype']['file'];
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

      if (!form_get_errors()) {
        $phenotype_file_tree_col = $groups['Tree Identifier']['1'];
        $phenotype_names = array();
        if ($phenotype['format'] == 0) {
          // If there is only one column with data then it will be a string
          // but we need an array.
          $phenotype_file_name_cols = is_array($groups['Phenotype Data']['0'])
            ? $groups['Phenotype Data']['0'] : [$groups['Phenotype Data']['0']];
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
          $species_index = empty($page3['tree-accession']['check']) ? 'species-1' : "species-$org_num";
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

          $missing_trees = tpps_compare_files(
            $form_state['values'][$id]['phenotype']['file'],
            $tree_accession_file,
            $phenotype_file_tree_col,
            $id_col_accession_name,
            $phenotype_no_header,
            $acc_no_header
          );
          if ($missing_trees !== array()) {
            $tree_id_str = implode(', ', $missing_trees);
            form_set_error("$id][phenotype][file",
              "Phenotype file: We detected Plant Identifiers that were not "
              . "in your Plant Accession file. Please either remove these "
              . "plants from your Phenotype file, or add them to your Plant "
              . "Accession file. The Plant Identifiers we found were: $tree_id_str"
            );
          }
        }
      }

      // Preserve file if it is valid.
      tpps_preserve_valid_file(
        $form_state,
        $form_state['values'][$id]['phenotype']['file'],
        $org_num,
        "Phenotype_Data"
      );
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Iso/Mass Spectrometry.
  if ($iso_check) {
    if (empty($phenotype['iso'])) {
      form_set_error("$id][phenotype][iso",
        t("Phenotype Isotope/Mass Spectrometry File: field is required.")
      );
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
        form_set_error("$id][phenotype][iso", t("Mass spectrometry/Isotope file: "
          . "some columns in the file you provided are missing or "
          . "have duplicate header values. Please either enter valid header "
          . "values for those columns or remove those columns, then reupload your file."));
      }
    }

    if (!form_get_errors()) {
      $species_index = empty($page3['tree-accession']['check'])
        ? 'species-1' : "species-$org_num";
      $tree_accession_file = $page3['tree-accession'][$species_index]['file'];
      $id_col_accession_name = $page3['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

      $acc_no_header = $page3['tree-accession'][$species_index]['file-no-header'];
      $missing_trees = tpps_compare_files($phenotype['iso'], $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);

      if ($missing_trees !== []) {
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
 *   $form_state['values']["organism-$i"]['genotype'].
 * @param int $org_num
 *   The id of the organism being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_genotype(array &$genotype, $org_num, array $form, array &$form_state) {
  $id = "organism-$org_num";

  $snps_fieldset = 'SNPs';
  $other_fieldset = 'other';
  $snps = $genotype[$snps_fieldset] ?? NULL;
  $ref_genome = $snps['ref-genome'] ?? NULL;

  $does_study_include_snp_data
    = $genotype['does_study_include_snp_data'] ?? NULL;
  $does_study_include_ssr_cpssr_data
    = $genotype['does_study_include_ssr_cpssr_data'] ?? NULL;
  $does_study_include_other_genotypic_data
    = $genotype['does_study_include_other_genotypic_data'] ?? NULL;

  $genotyping_type = $genotype[$snps_fieldset]['genotyping-type'] ?? NULL;
  // WARNING: 'maker-type' is array because multiple values could be selected.
  $marker_type = $genotype['marker-type'] ?? NULL;
  $file_type = $genotype[$snps_fieldset]['file-type'] ?? NULL;
  // File fields:
  $vcf = $genotype[$snps_fieldset]['vcf'] ?? 0;
  $snps_assay = $genotype[$snps_fieldset]['snps-assay'] ?? 0;
  $assay_design = $genotype[$snps_fieldset]['assay-design'] ?? 0;
  $assoc_file = $genotype[$snps_fieldset]['snps-association'] ?? 0;

  $other_fieldset = 'other';
  $other_file = $genotype[$other_fieldset]['other'] ?? 0;
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
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if ($does_study_include_snp_data == 'yes') {
    if ($is_step2_genotype) {
      if ($genotype[$snps_fieldset]['upload_snp_association'] == 'Yes') {
        tpps_is_required_field_empty($form_state,
          [$id, 'genotype', $snps_fielset, 'snps-association-type']
        );
        tpps_is_required_field_empty($form_state,
          [$id, 'genotype', $snps_fieldset, 'snps-association-tool']
        );
      }
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
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate SSR/cpSSR fieldset fields.
  $ssrs_fieldset = 'ssrs_cpssrs';
  if ($does_study_include_ssr_cpssr_data == 'yes') {
    // 'Define SSRs/cpSSRs Type' field.
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

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Validate 'Other' fieldset fields.
  if ($does_study_include_other_genotypic_data == 'yes') {
    tpps_is_required_field_empty($form_state,
      [$id, 'genotype', $other_fieldset, 'other-marker']
    );
    tpps_is_required_field_empty($form_state,
      [$id, 'genotype', $other_fieldset, 'other']
    );
  }
  // [/VS]
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  $accession = $form_state['accession'] ?? NULL;
  $submission = new Submission($accession);
  $loaded_state = $submission->state;
  if (!empty($loaded_state['vcf_replace'])) {
    foreach ($loaded_state['vcf_replace'] as $org_num => $fid) {
      $file = file_load($fid ?? '');
      if ($file) {
        if ($file->filesize == 0) {
          form_set_error("$org_num][genotype][$snps_fieldset][local_vcf",
            t('Local VCF File: File is empty.')
          );
        }
        else {
          $form_state['values'][$id]['genotype'][$snps_fieldset]['vcf'] = $fid;
          $vcf = $fid;
          $form_state['values'][$id]['genotype'][$snps_fieldset]['local_vcf_check'] = NULL;
          $form_state['values'][$id]['genotype'][$snps_fieldset]['local_vcf'] = NULL;
        }
      }
      else {
        // @TODO Should be $id instead of $org_num here?
        form_set_error("$org_num][genotype][$snps_fieldset][local_vcf",
          t("Local VCF File: File could not be loaded properly.")
        );
      }
    }
  }

  // Validate 'Remote/cluster VCF' field.
  if (
    $does_study_include_snp_data == 'yes'
    && $file_type == TPPS_GENOTYPING_FILE_TYPE_VCF
    && $genotype[$snps_fieldset]['vcf_file-location'] == 'remote'
    && trim($genotype[$snps_fieldset]['local_vcf']) == ''
    && !$vcf
  ) {
    tpps_form_error_required($form_state,
      [$id, 'genotype', $snps_fieldset, 'local_vcf']
    );
  }

  // Validate 'uploaded VCF' field.
  if (
    $does_study_include_snp_data == 'yes'
    && $file_type == TPPS_GENOTYPING_FILE_TYPE_VCF
    && $genotype[$snps_fieldset]['vcf_file-location'] == 'local'
    && !$vcf
    //&& trim($form_state['values'][$id]['genotype'][$snps_fieldset]['local_vcf']) == ''
  ) {
    tpps_form_error_required($form_state,
      [$id, 'genotype', $snps_fieldset, 'vcf']
    );
  }
  elseif (
    $does_study_include_snp_data == 'yes'
    && $file_type == TPPS_GENOTYPING_FILE_TYPE_VCF
  ) {
    if (
      !empty($assembly) && !form_get_errors()
      && in_array($ref_genome, ['manual', 'manual2', 'url'])
    ) {
      if (trim($genotype[$snps_fieldset]['local_vcf']) != '') {
        $local_vcf_path = trim($genotype[$snps_fieldset]['local_vcf']);
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
    if (
      empty($loaded_state['vcf_replace'])
      && trim($form_state['values']["organism-$org_num"]['genotype'][$snps_fieldset]['local_vcf']) != ''
      && ($loaded_state['vcf_validated'] ?? NULL) !== TRUE
    ) {
      form_set_error(
        "$id][genotype][$snps_fieldset][local_vcf",
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
      if ($vcf) {
        $vcf_file = file_load($vcf);
      }
      if (trim($genotype[$snps_fieldset]['local_vcf']) != '') {
        $location = trim($genotype[$snps_fieldset]['local_vcf']);
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

  if (
    $does_study_include_snp_data == 'yes'
    && (
      $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
      || $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY
    )
  ) {
    if (!$snps_assay) {
      tpps_form_error_required($form_state,
        [$id, 'genotype', $snps_fieldset, 'snps-assay']
      );
    }
    else {
      $headers = tpps_file_headers($snps_assay);
      $id_col_name = key($headers);
      while (($k = array_search(NULL, $headers))) {
        $message = t('Following header column is Null which needs to be fixed. '
          . '%data', ['%data' => $k]);
        drupal_set_message($message, 'error');
        unset($headers[$k]);
      }
      $num_columns = tpps_file_width($snps_assay) - 1;
      $num_unique_columns = count(array_unique($headers)) - 1;
      if ($num_unique_columns != $num_columns) {
        $duplicates = array_diff_assoc($headers, array_unique($headers));
        if (!empty($duplicates)) {
          drupal_set_message(
            t('Following header values are duplicate in provided snp file. %data',
            array('%data' => implode(',', $duplicates))),
          'error'
          );
        }
        form_set_error("$id][genotype][$snps_fieldset][snps-assay",
          t("SNPs Assay file: some columns in the file you provided are "
            . "missing or have duplicate header values. Please either enter "
            . "valid header values for those columns or remove those columns, "
            . "then reupload your file."
          )
        );
      }

      // #86782z4xu Skip this check if we reuse files from existing study.
      // @todo Minor. Maybe better to get new list of trees and use it in
      // all other checks to be sure we have the same list in other files.
      if (!form_get_errors() && empty($page3['existing_trees'])) {
        $acc_no_header = $page3['tree-accession'][$species_index]['file-no-header'];
        $missing_trees = tpps_compare_files(
          $snps_assay,
          $tree_accession_file,
          $id_col_name,
          $id_col_accession_name,
          FALSE,
          $acc_no_header
        );
        if ($missing_trees !== []) {
          form_set_error("$id][genotype][$snps_fieldset][snps-assay",
            t(
              "SNPs Assay file: We detected Plant Identifiers that were "
              . "not in your Plant Accession file. Please either remove these "
              . "plants from your Genotype file, or add them to your "
              . "Plant Accession file. "
              . "The Plant Identifiers we found were: @tree_id_str",
              ['@tree_id_str' => implode(', ', $missing_trees)]
            )
          );
        }
      }
      // Preserve file if it is valid.
      tpps_preserve_valid_file($form_state, $snps_assay, $org_num, "Genotype_SNPs_Assay");

      if (!form_get_errors()) {
        if (
          $does_study_include_snp_data == 'yes'
          && $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
          && $assoc_file
        ) {
          $required_groups = [
            'SNP ID' => ['id' => [1]],
            'Scaffold' => ['scaffold' => [2]],
            'Position' => ['position' => [3]],
            'Allele' => ['allele' => [4]],
            'Associated Trait' => ['trait' => [5]],
            'Confidence Value' => ['confidence' => [6]],
          ];
          $file_element = $form[$id]['genotype'][$snps_fieldset]['snps-association'];
          $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

          if (!form_get_errors()) {
            // Check that SNP IDs match Genotype Assay.
            $snps_id_col = $groups['SNP ID'][1];
            $assoc_no_header = $genotype[$snps_fieldset]['snps-association-no-header'] ?? FALSE;

            $assay_snps = tpps_file_headers($snps_assay);
            unset($assay_snps[key($assay_snps)]);
            $assoc_snps = tpps_parse_file_column($assoc_file, $snps_id_col, $assoc_no_header);
            $missing_snps = array_diff($assoc_snps, $assay_snps);

            if ($missing_snps !== array()) {
              $snps_id_str = implode(', ', $missing_snps);
              form_set_error("$id][genotype][$snps_fieldset][snps-association",
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
              form_set_error("$id][genotype][$snps_fieldset][snps-association",
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
                form_set_error("$id][genotype][$snps_fieldset][snps-association",
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

          tpps_is_required_field_empty($form_state,
            [$id, 'genotype', $snps_fieldset, 'snps-association-type']
          );
          tpps_is_required_field_empty($form_state,
            [$id, 'genotype', $snps_fieldset, 'snps-association-tool']
          );
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // SNPs Population Structure file.
          if (
            $genotype[$snps_fieldset]['upload_snp_population'] == 'Yes'
            && !tpps_is_required_field_empty($form_state,
              [$id, 'genotype', $snps_fieldset, 'snps-pop-struct']
            )
          ) {
            // Preserve file if it is valid.
            tpps_preserve_valid_file(
              $form_state,
              $genotype[$snps_fieldset]['snps-pop-struct'],
              $org_num,
              'SNPs_Population_Structure'
            );
          }
          // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
          // SNPs Kinship File.
          if ($genotype[$snps_fieldset]['upload_snp_kinship'] == 'Yes'
            && !tpps_is_required_field_empty(
              $form_state, [$id, 'genotype', $snps_fieldset, 'snps-kinship'])
          ) {
            // Preserve file if it is valid.
            tpps_preserve_valid_file(
              $form_state,
              $genotype[$snps_fieldset]['snps-kinship'],
              $org_num,
              'SNPs_Kinship'
            );
          }
        }
      }
    }
  }
  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Assay Design File.
  if (
    $does_study_include_snp_data == 'yes'
    && (
      $file_type == TPPS_GENOTYPING_FILE_TYPE_SNP_ASSAY_FILE_AND_ASSAY_DESIGN_FILE
      || $genotyping_type == TPPS_GENOTYPING_TYPE_GENOTYPING_ASSAY
    )
  ) {
    $file_field_name = 'assay-design';
    $file_field_value = drupal_array_get_nested_value(
      $form_state, ['values', $id, 'genotype', $snps_fieldset, $file_field_name]
    );
    if ($file_field_value) {
      // Preserve file if it is valid.
      tpps_preserve_valid_file(
        $form_state,
        $genotype[$snps_fieldset][$file_field_name],
        $org_num,
        'Genotype_Assay_Design'
      );
    }
  }

  // ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  // Files / Other.
  $other_fieldset = 'other';
  if (
    $does_study_include_other_genotypic_data == 'yes'
    && !tpps_is_required_field_empty($form_state,
      [$id, 'genotype', $other_fieldset, 'other']
    )
  ) {
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
      $acc_no_header = $page3['tree-accession'][$species_index]['file-no-header'];
      $other_no_header = $genotype[$other_fieldset]['other-no-header'] ?? FALSE;
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
  // [/VS]
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
 * Check if 'unit' column has empty values.
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_unit_validate_metafile($row, array &$options = []) {
  $columns = $options['meta_columns'];
  if (empty($row[$columns['unit']])) {
    form_set_error(
      $options['id'] . '][phenotype][metadata',
      t('Phenotype Metadata File: Empty unit not allowed.')
      . '<br />Row: ' . implode(', ', $row)
    );
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
