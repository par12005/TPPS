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
function page_4_validate_form(array &$form, array &$form_state) {

  if ($form_state['submitted'] == '1') {

    $form_values = $form_state['values'];
    $organism_number = $form_state['saved_values'][TPPS_PAGE_1]['organism']['number'];

    for ($i = 1; $i <= $organism_number; $i++) {
      $organism = $form_values["organism-$i"];

      if ($i > 1 and isset($organism['phenotype-repeat-check']) and $organism['phenotype-repeat-check'] == '1') {
        unset($form_state['values']["organism-$i"]['phenotype']);
      }
      if (isset($form_state['values']["organism-$i"]['phenotype'])) {
        validate_phenotype($form_state['values']["organism-$i"]['phenotype'], "organism-$i", $form, $form_state);
      }

      if ($i > 1 and isset($organism['genotype-repeat-check']) and $organism['genotype-repeat-check'] == '1') {
        unset($form_state['values']["organism-$i"]['genotype']);
      }
      if (isset($form_state['values']["organism-$i"]['genotype'])) {
        validate_genotype($form_state['values']["organism-$i"]['genotype'], "organism-$i", $form, $form_state);
      }

      if ($i > 1 and isset($organism['environment-repeat-check']) and $organism['environment-repeat-check'] == '1') {
        unset($form_state['values']["organism-$i"]['environment']);
      }
      if (isset($form_state['values']["organism-$i"]['environment'])) {
        $env = &$form_state['values']["organism-$i"]['environment'];
        validate_environment($env, "organism-$i");
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

        if (isset($form["organism-$i"]['genotype']['files']['snps-assay']['upload']) and isset($new_form["organism-$i"]['genotype']['files']['snps-assay']['upload'])) {
          $form["organism-$i"]['genotype']['files']['snps-assay']['upload'] = $new_form["organism-$i"]['genotype']['files']['snps-assay']['upload'];
          $form["organism-$i"]['genotype']['files']['snps-assay']['upload']['#id'] = "edit-organism-$i-genotype-files-snps-assay-upload";
        }
        if (isset($form["organism-$i"]['genotype']['files']['snps-assay']['columns']) and isset($new_form["organism-$i"]['genotype']['files']['snps-assay']['columns'])) {
          $form["organism-$i"]['genotype']['files']['snps-assay']['columns'] = $new_form["organism-$i"]['genotype']['files']['snps-assay']['columns'];
          $form["organism-$i"]['genotype']['files']['snps-assay']['columns']['#id'] = "edit-organism-$i-genotype-files-snps-assay-columns";
        }

        if (isset($form["organism-$i"]['genotype']['files']['other']['upload']) and isset($new_form["organism-$i"]['genotype']['files']['other']['upload'])) {
          $form["organism-$i"]['genotype']['files']['other']['upload'] = $new_form["organism-$i"]['genotype']['files']['other']['upload'];
          $form["organism-$i"]['genotype']['files']['other']['upload']['#id'] = "edit-organism-$i-genotype-files-other-upload";
        }
        if (isset($form["organism-$i"]['genotype']['files']['other']['columns']) and isset($new_form["organism-$i"]['genotype']['files']['other']['columns'])) {
          $form["organism-$i"]['genotype']['files']['other']['columns'] = $new_form["organism-$i"]['genotype']['files']['other']['columns'];
          $form["organism-$i"]['genotype']['files']['other']['columns']['#id'] = "edit-organism-$i-genotype-files-other-columns";
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
 * @param string $id
 *   The id of the organism fieldset being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function validate_phenotype(array $phenotype, $id, array $form, array &$form_state) {
  $phenotype_number = $phenotype['number'];
  $phenotype_check = $phenotype['check'];
  $phenotype_meta = $phenotype['metadata'];
  $phenotype_file = $phenotype['file'];

  if ($phenotype_check == '1') {
    if ($phenotype_meta == '') {
      form_set_error("$id][phenotype][metadata", "Phenotype Metadata File: field is required.");
    }
    else {
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
      );

      $file_element = $form[$id]['phenotype']['metadata'];
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

      if (!form_get_errors()) {
        // Get phenotype name column.
        $phenotype_name_col = $groups['Phenotype Id']['1'];

        // Preserve file if it is valid.
        $file = file_load($form_state['values'][$id]['phenotype']['metadata']);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }
    }
  }

  for ($i = 1; $i <= $phenotype_number; $i++) {
    $current_phenotype = $phenotype['phenotypes-meta']["$i"];
    $name = $current_phenotype['name'];
    $attribute = $current_phenotype['attribute'];
    $description = $current_phenotype['description'];
    $units = $current_phenotype['units'];

    if ($name == '') {
      form_set_error("$id][phenotype][phenotypes-meta][$i][name", "Phenotype $i Name: field is required.");
    }

    if ($attribute == '') {
      form_set_error("$id][phenotype][phenotypes-meta][$i][attribute", "Phenotype $i Attribute: field is required.");
    }

    if ($description == '') {
      form_set_error("$id][phenotype][phenotypes-meta][$i][description", "Phenotype $i Description: field is required.");
    }

    if ($units == '') {
      form_set_error("$id][phenotype][phenotypes-meta][$i][units", "Phenotype $i Units: field is required.");
    }

    if ($current_phenotype['struct-check'] == '1' and $current_phenotype['structure'] == '') {
      form_set_error("$id][phenotype][phenotypes-meta][$i][structure", "Phenotype $i Structure: field is required.");
    }

    if ($current_phenotype['val-check'] == '1' and $current_phenotype['min'] == '') {
      form_set_error("$id][phenotype][phenotypes-meta][$i][min", "Phenotype $i Minimum Value: field is required.");
    }

    if ($current_phenotype['val-check'] == '1' and $current_phenotype['max'] == '') {
      form_set_error("$id][phenotype][phenotypes-meta][$i][max", "Phenotype $i Maximum Value: field is required.");
    }
  }

  if ($phenotype_file == '') {
    form_set_error("$id][phenotype][file", "Phenotypes: field is required.");
  }
  else {
    if ($phenotype['format'] == 0) {
      $required_groups = array(
        'Tree Identifier' => array(
          'id' => array(1),
        ),
        'Phenotype Data' => array(
          'phenotype-data' => array(0),
        ),
      );
    }
    else {
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
      // Preserve file if it is valid.
      $file = file_load($form_state['values'][$id]['phenotype']['file']);
      $phenotype_file_location = drupal_realpath($file->uri);
      file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

      if ($phenotype['format'] == 0) {
        $phenotype_file_tree_col = $groups['Tree Identifier']['1'];
        $phenotype_file_name_cols = $groups['Phenotype Data']['0'];
        $content = tpps_parse_xlsx($phenotype_file_location, 1);

        if (isset($phenotype_file_name_cols) and isset($phenotype_name_col)) {
          $meta_content = tpps_parse_xlsx(drupal_realpath(file_load($phenotype_meta)->uri));
          $missing_phenotypes = array();
          foreach ($phenotype_file_name_cols as $key) {
            $name = $content['headers'][$key];
            $exists = FALSE;
            for ($i = 0; $i < count($meta_content) - 1; $i++) {
              if (strtolower($meta_content[$i][$phenotype_name_col]) == strtolower($name)) {
                $exists = TRUE;
                break;
              }
            }

            if (!$exists) {
              for ($i = 1; $i <= $phenotype_number; $i++) {
                if (strtolower($phenotype['phenotypes-meta'][$i]['name']) == strtolower($name)) {
                  $exists = TRUE;
                  break;
                }
              }
            }

            if (!$exists) {
              $missing_phenotypes[] = $name;
            }
          }
          if ($missing_phenotypes !== array()) {
            $phenotype_id_str = implode(', ', $missing_phenotypes);
            form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype Metadata file. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype Metadata file. The phenotypes we detected with missing definitions were: $phenotype_id_str");
          }
          // dpm($meta_content[0][$phenotype_name_col]);.
        }
        elseif (isset($phenotype_file_name_cols)) {
          $missing_phenotypes = array();
          foreach ($phenotype_file_name_cols as $key) {
            $name = $content['headers'][$key];
            $exists = FALSE;
            for ($i = 1; $i <= $phenotype_number; $i++) {
              if (strtolower($phenotype['phenotypes-meta'][$i]['name']) == strtolower($name)) {
                $exists = TRUE;
                break;
              }
            }

            if (!$exists) {
              $missing_phenotypes[] = $name;
            }
          }
          if ($missing_phenotypes !== array()) {
            $phenotype_id_str = implode(', ', $missing_phenotypes);
            form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype Metadata file. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype Metadata file. The phenotypes we detected with missing definitions were: $phenotype_id_str");
          }
        }
      }
      else {
        // Get column names.
        $phenotype_file_tree_col = $groups['Tree Identifier']['1'];
        $phenotype_file_name_col = $groups['Phenotype Name/Identifier']['2'];

        if (isset($phenotype_file_name_col) and isset($phenotype_name_col)) {
          $missing_phenotypes = tpps_compare_files($phenotype_file, $phenotype_meta, $phenotype_file_name_col, $phenotype_name_col);

          for ($i = 0; $i < count($missing_phenotypes); $i++) {
            $missing_phenotypes[$i];
            for ($j = 1; $j <= $phenotype_number; $j++) {
              if (strtolower($phenotype['phenotypes-meta'][$j]['name']) == strtolower($missing_phenotypes[$i])) {
                unset($missing_phenotypes[$i]);
                break;
              }
            }
          }

          if ($missing_phenotypes !== array()) {
            $phenotype_id_str = implode(', ', $missing_phenotypes);
            form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype Metadata file. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype Metadata file. The phenotypes we detected with missing definitions were: $phenotype_id_str");
          }
        }
        elseif (isset($phenotype_file_name_col)) {
          $content = tpps_parse_xlsx($phenotype_file_location);

          $missing_phenotypes = array();
          for ($i = 0; $i < count($content) - 1; $i++) {
            $used_phenotype = $content[$i][$phenotype_file_name_col];
            $defined = FALSE;
            for ($j = 1; $j <= $phenotype_number; $j++) {
              if (strtolower($phenotype['phenotypes-meta'][$j]['name']) == strtolower($used_phenotype)) {
                $defined = TRUE;
                break;
              }
            }
            if (!$defined) {
              array_push($missing_phenotypes, $used_phenotype);
            }
          }

          if ($missing_phenotypes !== array()) {
            $phenotype_id_str = implode(', ', $missing_phenotypes);
            form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype definitions. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype definitions. The phenotypes we detected with missing definitions were: $phenotype_id_str");
          }
        }
      }

      if (isset($phenotype_file_tree_col)) {
        if ($form_state['saved_values'][TPPS_PAGE_1]['organism']['number'] == 1 or $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'] == '0') {
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file'];
          $column_vals = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file-columns'];
        }
        else {
          $num = substr($id, 9);
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$num"]['file'];
          $column_vals = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$num"]['file-columns'];
        }

        foreach ($column_vals as $col => $val) {
          if ($val == '1') {
            $id_col_accession_name = $col;
            break;
          }
        }

        $missing_trees = tpps_compare_files($form_state['values'][$id]['phenotype']['file'], $tree_accession_file, $phenotype_file_tree_col, $id_col_accession_name);

        if ($missing_trees !== array()) {
          $tree_id_str = implode(', ', $missing_trees);
          form_set_error("$id][phenotype][file", "Phenotype file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Phenotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
        }
      }

      if (!form_get_errors()) {
        $rows = count(tpps_parse_xlsx(drupal_realpath($file->uri))) - 1 + !empty($phenotype['file-no-header']);
        if ($phenotype['format'] == 0) {
          $form_state['values']["$id"]['phenotype']['phenotype_count'] = $rows * count($phenotype_file_name_cols);
        }
        else {
          $form_state['values']["$id"]['phenotype']['phenotype_count'] = $rows;
        }
      }
    }
  }
}

/**
 * Validates the genotype section of the fourth page of the form.
 *
 * @param array $genotype
 *   The form_state values of the genotype fieldset for organism $id.
 * @param string $id
 *   The id of the organism fieldset being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function validate_genotype(array $genotype, $id, array $form, array &$form_state) {
  $snps = $genotype['SNPs'];
  $ref_genome = $genotype['ref-genome'];
  $file_type = $genotype['files']['file-type'];
  $vcf = isset($genotype['files']['vcf']) ? $genotype['files']['vcf'] : 0;
  $snps_assay = isset($genotype['files']['snps-assay']) ? $genotype['files']['snps-assay'] : 0;
  $other_file = isset($genotype['files']['other']) ? $genotype['files']['other'] : 0;

  if (!$ref_genome) {
    form_set_error("$id][genotype][ref-genome", "Reference Genome: field is required.");
  }
  elseif ($ref_genome === 'bio') {
    if (!$genotype['BioProject-id']) {
      form_set_error("$id][genotype][Bioproject-id", 'BioProject Id: field is required.');
    }
    else {
      $assembly_auto_check = '';

      foreach ($genotype['assembly-auto'] as $item) {
        $assembly_auto_check += $item;
      }

      if (preg_match('/^0*$/', $assembly_auto_check)) {
        form_set_error("$id][genotype][assembly-auto", 'Assembly file(s): field is required.');
      }
    }
  }
  elseif ($ref_genome === 'url' or $ref_genome === 'manual' or $ref_genome === 'manual2') {

    $class = 'FASTAImporter';
    tripal_load_include_importer_class($class);
    $fasta_vals = $genotype['tripal_fasta'];

    $file_upload = isset($fasta_vals['file']['file_upload']) ? trim($fasta_vals['file']['file_upload']) : 0;
    $file_existing = isset($fasta_vals['file']['file_upload_existing']) ? trim($fasta_vals['file']['file_upload_existing']) : 0;
    $file_remote = isset($fasta_vals['file']['file_remote']) ? trim($fasta_vals['file']['file_remote']) : 0;
    $db_id = trim($fasta_vals['db']['db_id']);
    $re_accession = trim($fasta_vals['db']['re_accession']);
    $analysis_id = trim($fasta_vals['analysis_id']);
    $seqtype = trim($fasta_vals['seqtype']);

    if (!$file_upload and !$file_existing and !$file_remote) {
      form_set_error("$id][genotype][tripal_fasta][file", "Assembly file: field is required.");
    }

    $re_name = '^(.*?)\s.*$';

    if ($db_id and !$re_accession) {
      form_set_error("$id][genotype][tripal_fasta][additional][re_accession", 'Accession regular expression: field is required.');
    }
    if ($re_accession and !$db_id) {
      form_set_error("$id][genotype][tripal_fasta][additional][db_id", 'External Database: field is required.');
    }

    if (!$analysis_id) {
      form_set_error("$id][genotype][tripal_fasta][analysis_id", 'Analysis: field is required.');
    }
    if (!$seqtype) {
      form_set_error("$id][genotype][tripal_fasta][seqtype", 'Sequence Type: field is required.');
    }

    // dpm($class::$file_required);
    // dpm($fasta_vals);
    // form_set_error("Submit", 'error');.
    if (!form_get_errors()) {
      $assembly = $file_existing ? $file_existing : ($file_upload ? $file_upload : $file_remote);
    }
  }

  if (implode('', $genotype['marker-type']) === '000') {
    form_set_error("$id][genotype][marker-type", "Genotype Marker Type: field is required.");
  }
  elseif ($genotype['marker-type']['SNPs']) {
    if (!$snps['genotyping-design']) {
      form_set_error("$id][genotype][SNPs][genotyping-design", "Genotyping Design: field is required.");
    }
    elseif ($snps['genotyping-design'] == '1') {
      if (!$snps['GBS']) {
        form_set_error("$id][genotype][SNPs][GBS", "GBS Type: field is required.");
      }
      elseif ($snps['GBS'] == '5' and !$snps['GBS-other']) {
        form_set_error("$id][genotype][SNPs][GBS=other", "Custom GBS Type: field is required.");
      }
    }
    elseif ($snps['genotyping-design'] == '2') {
      if (!$snps['targeted-capture']) {
        form_set_error("$id][genotype][SNPs][targeted-capture", "Targeted Capture: field is required.");
      }
      elseif ($snps['targeted-capture'] == '2' and !$snps['targeted-capture-other']) {
        form_set_error("$id][genotype][SNPs][targeted-capture-other", "Custom Targeted Capture: field is required.");
      }
    }
  }
  elseif ($genotype['marker-type']['SSRs/cpSSRs'] and empty($genotype['SSRs/cpSSRs'])) {
    form_set_error("$id][genotype][SSRs/cpSSRs", "SSRs/cpSSRs: field is required.");
  }
  elseif ($genotype['marker-type']['SSRs/cpSSRs'] and empty($genotype['files']['ploidy'])) {
    form_set_error("$id][genotype][files][ploidy", "Ploidy: field is required.");
  }
  elseif ($genotype['marker-type']['Other'] and empty($genotype['other-marker'])) {
    form_set_error("$id][genotype][other-marker", "Other Genotype marker: field is required.");
  }

  if (empty($file_type['VCF']) and empty($file_type['SNPs Genotype Assay']) and empty($file_type['SSRs/cpSSRs Genotype Spreadsheet']) and empty($file_type['Other Marker Genotype Spreadsheet'])) {
    form_set_error("$id][genotype][files][file-type", "Genotype File Type: field is required.");
  }
  else {
    if (!empty($file_type['VCF']) and !$vcf) {
      form_set_error("$id][genotype][files][vcf", "Genotype VCF File: field is required.");
    }
    elseif (!empty($file_type['VCF'])) {
      if (($ref_genome === 'manual' or $ref_genome === 'manual2' or $ref_genome === 'url') and isset($assembly) and $assembly and !form_get_errors()) {
        $vcf_content = fopen(file_load($vcf)->uri, 'r');
        $assembly_content = fopen(file_load($assembly)->uri, 'r');

        while (($vcf_line = fgets($vcf_content)) !== FALSE) {
          if ($vcf_line[0] != '#') {

            $vcf_values = explode("\t", $vcf_line);
            $scaffold_id = $vcf_values[0];
            $match = FALSE;

            while (($assembly_line = fgets($assembly_content)) !== FALSE) {
              if ($assembly_line[0] != '>') {
                continue;
              }
              else {
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
              fclose($assembly_content);
              $assembly_content = fopen(file_load($assembly)->uri, 'r');
              while (($assembly_line = fgets($assembly_content)) !== FALSE) {
                if ($assembly_line[0] != '>') {
                  continue;
                }
                else {
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
            }

            if (!$match) {
              form_set_error("$id][genotype][files][vcf", "VCF File: scaffold $scaffold_id not found in assembly file(s)");
            }
          }
        }

      }

      if (!form_get_errors()) {
        $vcf_content = fopen(file_load($vcf)->uri, 'r');
        $count = 0;
        while (($vcf_line = fgets($vcf_content)) !== FALSE) {
          if ($vcf_line[0] != '#') {
            $count++;
          }
        }
        $form_state['values'][$id]['genotype']['files']['vcf_genotype_count'] = $count;
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($vcf);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }
    }

    if (!empty($file_type['SNPs Genotype Assay']) and !$snps_assay) {
      form_set_error("$id][genotype][files][snps-assay", "SNPs Assay file: field is required.");
    }
    elseif (!empty($file_type['SNPs Genotype Assay'])) {
      $required_groups = array(
        'Tree Id' => array(
          'id' => array(1),
        ),
        'SNP Data' => array(
          'data' => array(2),
        ),
      );

      $file_element = $form[$id]['genotype']['files']['snps-assay'];
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

      if (!form_get_errors()) {
        // Get Tree Id column name.
        $id_col_genotype_name = $groups['Tree Id']['1'];

        if ($form_state['saved_values'][TPPS_PAGE_1]['organism']['number'] == 1 or $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'] == '0') {
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file'];
          $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file-groups']['Tree Id']['1'];
        }
        else {
          $num = substr($id, 9);
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$num"]['file'];
          $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$num"]['file-groups']['Tree Id']['1'];
        }

        $missing_trees = tpps_compare_files($snps_assay, $tree_accession_file, $id_col_genotype_name, $id_col_accession_name);

        if ($missing_trees !== array()) {
          $tree_id_str = implode(', ', $missing_trees);
          form_set_error("$id][genotype][files][snps-assay", "SNPs Assay file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
        }
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($snps_assay);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }
    }

    if (!empty($file_type['Assay Design']) and !$genotype['files']['assay-design']) {
      form_set_error("$id][genotype][files][assay-design", "Assay Design file: field is required.");
    }
    elseif (!empty($file_type['Assay Design']) and !form_get_errors()) {
      // Preserve file if it is valid.
      $file = file_load($genotype['files']['assay-design']);
      file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
    }

    if (!empty($file_type['SSRs/cpSSRs Genotype Spreadsheet']) and !$genotype['files']['ssrs']) {
      form_set_error("$id][genotype][files][ssrs]", "SSRs/cpSSRs Spreadsheet: field is required.");
    }
    elseif (!empty($file_type['SSRs/cpSSRs Genotype Spreadsheet']) and !empty($genotype['files']['ploidy'])) {
      $ssrs_file = file_load($genotype['files']['ssrs']);
      $location = drupal_realpath($ssrs_file->uri);
      $content = tpps_parse_xlsx($location, 1);
      $id_col_name = key($content['headers']);
      while (($k = array_search(NULL, $content['headers']))){
        unset($content['headers'][$k]);
      }
      $num_columns = count($content[0]) - 1;
      $num_unique_columns = count(array_unique($content['headers'])) - 1;
      
      if ($num_unique_columns != $num_columns) {
        switch ($genotype['files']['ploidy']) {
          case 'Haploid':
            form_set_error("$id][genotype][files][ssrs", "SSRs/cpSSRs Genotype Spreadsheet: some columns in the file you provided are missing or have duplicate header values. Please either enter header values for those columns or remove those columns, then reupload your file.");
            break;
          
          case 'Diploid':
            if ($num_columns / $num_unique_columns !== 2) {
              form_set_error("$id][genotype][files][ssrs", "SSRs/cpSSRs Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file.");
            }
            break;

          case 'Polyploid':
            if ($num_columns % $num_unique_columns !== 0) {
              form_set_error("$id][genotype][files][ssrs", "SSRs/cpSSRs Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file.");
            }
            break;

          default:
            break;
        }
      }
      
      if (!form_get_errors()) {
        if ($form_state['saved_values'][TPPS_PAGE_1]['organism']['number'] == 1 or $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'] == '0') {
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file'];
          $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file-groups']['Tree Id']['1'];
        }
        else {
          $num = substr($id, 9);
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$num"]['file'];
          $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$num"]['file-groups']['Tree Id']['1'];
        }

        $missing_trees = tpps_compare_files($ssrs_file->fid, $tree_accession_file, $id_col_name, $id_col_accession_name);

        if ($missing_trees !== array()) {
          $tree_id_str = implode(', ', $missing_trees);
          form_set_error("$id][genotype][files][ssrs", "SSRs/cpSSRs Genotype Spreadsheet: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
        }
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($genotype['files']['ssrs']);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }
    }

    if (!empty($file_type['Other Marker Genotype Spreadsheet']) and !$genotype['files']['other']) {
      form_set_error("$id][genotype][files][other]", "Other Marker Spreadsheet: field is required.");
    }
    elseif (!empty($file_type['Other Marker Genotype Spreadsheet'])) {
      $required_groups = array(
        'Tree Id' => array(
          'id' => array(1),
        ),
        'Genotype Data' => array(
          'data' => array(2),
        ),
      );
      
      $file_element = $form[$id]['genotype']['files']['other'];
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);
      
      if (!form_get_errors()) {
        // Get Tree Id column name.
        $id_col_genotype_name = $groups['Tree Id']['1'];

        if ($form_state['saved_values'][TPPS_PAGE_1]['organism']['number'] == 1 or $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'] == '0') {
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file'];
          $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file-groups']['Tree Id']['1'];
        }
        else {
          $num = substr($id, 9);
          $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$num"]['file'];
          $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$num"]['file-groups']['Tree Id']['1'];
        }

        $missing_trees = tpps_compare_files($other_file, $tree_accession_file, $id_col_genotype_name, $id_col_accession_name);

        if ($missing_trees !== array()) {
          $tree_id_str = implode(', ', $missing_trees);
          form_set_error("$id][genotype][files][other", "Other Marker Genotype Spreadsheet: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
        }
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($other_file);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      }
    }
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
function validate_environment(array &$environment, $id) {
  if ($environment['use_layers']) {
    // Using cartogratree environment layers.
    $group_check = '';
    $new_layers = array();
    foreach ($environment['env_layers_groups'] as $group_id) {
      if (!empty($group_id)) {
        $group_check .= "1";
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
      else {
        $group_check .= "0";
      }
    }
    $environment['env_layers'] = $new_layers;

    if (preg_match('/^0+$/', $group_check)) {
      form_set_error("$id][environment][env_layers_groups", 'Cartogratree environmental layers groups: field is required.');
    }
    elseif (empty($new_layers)) {
      form_set_error("$id][environment][env_layers", 'CartograTree environmental layers: field is required.');
    }
  }

  if ($environment['env_manual_check']) {
    $env_number = $environment['number'];
    for ($i = 1; $i <= $env_number; $i++) {
      $current_env = $environment['env_manual']["$i"];
      $name = $current_env['name'];
      $desc = $current_env['description'];
      $unit = $current_env['units'];
      $val = $current_env['value'];

      if (empty($name)) {
        form_set_error("$id][environment][env_manual][$i][name", "Environment Data $i Name: field is required.");
      }
      if (empty($desc)) {
        form_set_error("$id][environment][env_manual][$i][description", "Environment Data $i Description: field is required.");
      }
      if (empty($unit)) {
        form_set_error("$id][environment][env_manual][$i][units", "Environment Data $i Units: field is required.");
      }
      if (empty($val)) {
        form_set_error("$id][environment][env_manual][$i][value", "Environment Data $i Value: field is required.");
      }
    }
  }

  if (empty($environment['env_manual_check']) and empty($environment['use_layers'])) {
    form_set_error("$id][environment", 'Environment: field is required.');
  }
}
