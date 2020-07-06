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
 * @param string $org_num
 *   The id of the organism fieldset being validated.
 * @param array $form
 *   The form being validated.
 * @param array $form_state
 *   The state of the form being validated.
 */
function tpps_validate_phenotype(array $phenotype, $org_num, array $form, array &$form_state) {
  $iso_check = $phenotype['iso-check'];
  $id = "organism-$org_num";

  if (empty($iso_check)) {
    $phenotype_number = $phenotype['phenotypes-meta']['number'];
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
          $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Phenotype_Metadata_$org_num";
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

      if (($current_phenotype['val-check'] or $current_phenotype['bin-check']) and $current_phenotype['min'] == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][min", "Phenotype $i Minimum Value: field is required.");
      }

      if (($current_phenotype['val-check'] or $current_phenotype['bin-check']) and $current_phenotype['max'] == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][max", "Phenotype $i Maximum Value: field is required.");
      }

      if ($current_phenotype['time-check'] == '1' and $current_phenotype['time'] == '') {
        form_set_error("$id][phenotype][phenotypes-meta][$i][time", "Phenotype $i Time: field is required.");
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
        $phenotype_file_tree_col = $groups['Tree Identifier']['1'];
        $phenotype_names = array();
        if ($phenotype['format'] == 0) {
          $phenotype_file_name_cols = $groups['Phenotype Data']['0'];
          $headers = tpps_file_headers($phenotype_file, !empty($phenotype['file-no-header']));
          foreach ($phenotype_file_name_cols as $column_index) {
            $phenotype_names[] = $headers[$column_index];
          }
        }
        else {
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
          $species_index = "species-$org_num";
          if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
            $species_index = "species-1";
          }
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
            form_set_error("$id][phenotype][file", "Phenotype file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Phenotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
          }
        }
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($form_state['values'][$id]['phenotype']['file']);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Phenotype_Data_$org_num";
      }

    }
  }
  else {
    $headers = tpps_file_headers($phenotype['iso']);
    $id_col_name = key($headers);
    while (($k = array_search(NULL, $headers))) {
      unset($headers[$k]);
    }
    $num_columns = tpps_file_width($phenotype['iso']) - 1;
    $num_unique_columns = count(array_unique($headers)) - 1;

    if ($num_unique_columns != $num_columns) {
      form_set_error("$id][phenotype][iso", "Mass spectrometry/Isotope file: some columns in the file you provided are missing or have duplicate header values. Please either enter valid header values for those columns or remove those columns, then reupload your file.");
    }

    if (!form_get_errors()) {
      $species_index = "species-$org_num";
      if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
        $species_index = "species-1";
      }
      $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file'];
      $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

      $acc_no_header = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-no-header'];
      $missing_trees = tpps_compare_files($phenotype['iso'], $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);

      if ($missing_trees !== array()) {
        $tree_id_str = implode(', ', $missing_trees);
        form_set_error("$id][phenotype][iso", "Mass spectrometry/Isotope file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
      }
    }

    if (!form_get_errors()) {
      // Preserve file if it is valid.
      $file = file_load($phenotype['iso']);
      file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Phenotype_Data_$org_num";
    }
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
  $file_type = $genotype['files']['file-type'];
  $vcf = isset($genotype['files']['vcf']) ? $genotype['files']['vcf'] : 0;
  $snps_assay = isset($genotype['files']['snps-assay']) ? $genotype['files']['snps-assay'] : 0;
  $other_file = isset($genotype['files']['other']) ? $genotype['files']['other'] : 0;

  if (!$ref_genome) {
    form_set_error("$id][genotype][ref-genome", "Reference Genome: field is required.");
  }
  elseif ($ref_genome === 'bio') {
    if (!$genotype['tripal_eutils']['accession']) {
      form_set_error("$id][genotype][tripal_eutils][accession", 'NCBI Accession Number: field is required.');
    }
    $connection = new \EUtils();
    try {
      $connection->setPreview();
      $parsed = $connection->get($genotype['tripal_eutils']['db'], $genotype['tripal_eutils']['accession']);
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
    /*if (!$genotype['BioProject-id']) {
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
    }*/
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

  if (preg_match('/^0+$/', implode('', $file_type))) {
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
        $form_state['values'][$id]['genotype']['files']['vcf_genotype_count'] = tpps_file_len($vcf);
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($vcf);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Genotype_VCF_$org_num";
      }
    }

    if (!empty($file_type['SNPs Genotype Assay']) and !$snps_assay) {
      form_set_error("$id][genotype][files][snps-assay", "SNPs Assay file: field is required.");
    }
    elseif (!empty($file_type['SNPs Genotype Assay'])) {
      $headers = tpps_file_headers($snps_assay);
      $id_col_name = key($headers);
      while (($k = array_search(NULL, $headers))) {
        unset($headers[$k]);
      }
      $num_columns = tpps_file_width($snps_assay) - 1;
      $num_unique_columns = count(array_unique($headers)) - 1;

      if ($num_unique_columns != $num_columns) {
        form_set_error("$id][genotype][files][snps-assay", "SNPs Assay file: some columns in the file you provided are missing or have duplicate header values. Please either enter valid header values for those columns or remove those columns, then reupload your file.");
      }

      if (!form_get_errors()) {
        if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
          $species_index = 'species-1';
        }
        else {
          $num = substr($id, 9);
          $species_index = "species-$num";
        }
        $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file'];
        $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

        $acc_no_header = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-no-header'];
        $missing_trees = tpps_compare_files($snps_assay, $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);

        if ($missing_trees !== array()) {
          $tree_id_str = implode(', ', $missing_trees);
          form_set_error("$id][genotype][files][snps-assay", "SNPs Assay file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
        }
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($snps_assay);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Genotype_SNPs_Assay_$org_num";
      }
    }

    if (!empty($file_type['Assay Design']) and !$genotype['files']['assay-design']) {
      form_set_error("$id][genotype][files][assay-design", "Assay Design file: field is required.");
    }
    elseif (!empty($file_type['Assay Design']) and !form_get_errors()) {
      // Preserve file if it is valid.
      $file = file_load($genotype['files']['assay-design']);
      file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
      $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Genotype_Assay_Design_$org_num";
    }

    if (!empty($file_type['SSRs/cpSSRs Genotype Spreadsheet']) and !$genotype['files']['ssrs']) {
      form_set_error("$id][genotype][files][ssrs]", "SSRs/cpSSRs Spreadsheet: field is required.");
    }
    elseif (!empty($file_type['SSRs/cpSSRs Genotype Spreadsheet']) and !empty($genotype['files']['ploidy'])) {
      $headers = tpps_file_headers($genotype['files']['ssrs']);
      $id_col_name = key($headers);
      while (($k = array_search(NULL, $headers))) {
        unset($headers[$k]);
      }
      $num_columns = tpps_file_width($genotype['files']['ssrs']) - 1;
      $num_unique_columns = count(array_unique($headers)) - 1;

      switch ($genotype['files']['ploidy']) {
        case 'Haploid':
          if ($num_unique_columns != $num_columns) {
            form_set_error("$id][genotype][files][ssrs", "SSRs/cpSSRs Genotype Spreadsheet: some columns in the file you provided are missing or have duplicate header values. Please either enter header values for those columns or remove those columns, then reupload your file.");
          }
          break;

        case 'Diploid':
          if ($num_unique_columns != $num_columns and $num_columns / $num_unique_columns !== 2) {
            form_set_error("$id][genotype][files][ssrs", "SSRs/cpSSRs Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file.");
          }
          elseif ($num_unique_columns == $num_columns and $num_columns % 2 !== 0) {
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

      if (!empty($genotype['files']['ssr-extra-check'])) {
        if (empty($genotype['files']['extra-ssr-type'])) {
          form_set_error("$id][genotype][files][extra-ssr-type", "Define Additional SSRs/cpSSRs Type: field is required.");
        }

        if (!$genotype['files']['ssrs_extra']) {
          form_set_error("$id][genotype][files][ssrs_extra]", "SSRs/cpSSRs Additional Spreadsheet: field is required.");
        }
        elseif (!empty($genotype['files']['extra-ploidy'])) {
          $headers = tpps_file_headers($genotype['files']['ssrs_extra']);
          $id_col_name = key($headers);
          while (($k = array_search(NULL, $headers))) {
            unset($headers[$k]);
          }
          $num_columns = tpps_file_width($genotype['files']['ssrs_extra']) - 1;
          $num_unique_columns = count(array_unique($headers)) - 1;

          switch ($genotype['files']['extra-ploidy']) {
            case 'Haploid':
              if ($num_unique_columns != $num_columns) {
                form_set_error("$id][genotype][files][ssrs_extra", "SSRs/cpSSRs Additional Genotype Spreadsheet: some columns in the file you provided are missing or have duplicate header values. Please either enter header values for those columns or remove those columns, then reupload your file.");
              }
              break;

            case 'Diploid':
              if ($num_unique_columns != $num_columns and $num_columns / $num_unique_columns !== 2) {
                form_set_error("$id][genotype][files][ssrs_extra", "SSRs/cpSSRs Additional Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file.");
              }
              elseif ($num_unique_columns == $num_columns and $num_columns % 2 !== 0) {
                form_set_error("$id][genotype][files][ssrs_extra", "SSRs/cpSSRs Additional Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file.");
              }
              break;

            case 'Polyploid':
              if ($num_columns % $num_unique_columns !== 0) {
                form_set_error("$id][genotype][files][ssrs_extra", "SSRs/cpSSRs Additional Genotype Spreadsheet: There is either an invalid number of columns in your file, or some of your columns are missing values. Please review and reupload your file.");
              }
              break;

            default:
              break;
          }
        }
      }

      if (!form_get_errors()) {
        $species_index = "species-$org_num";
        if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
          $species_index = "species-1";
        }
        $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file'];
        $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

        $acc_no_header = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-no-header'];
        $missing_trees = tpps_compare_files($genotype['files']['ssrs'], $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);

        if ($missing_trees !== array()) {
          $tree_id_str = implode(', ', $missing_trees);
          form_set_error("$id][genotype][files][ssrs", "SSRs/cpSSRs Genotype Spreadsheet: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
        }
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($genotype['files']['ssrs']);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Genotype_SSR_Spreadsheet_$org_num";
        if (!empty($genotype['files']['ssrs_extra'])) {
          $file = file_load($genotype['files']['ssrs_extra']);
          file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
          $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Genotype_SSR_Additional_Spreadsheet_$org_num";
        }
      }
    }

    if (!empty($file_type['Indel Genotype Spreadsheet']) and !$genotype['files']['indels']) {
      form_set_error("$id][genotype][files][indels]", "Indel Genotype Spreadsheet: field is required.");
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
        form_set_error("$id][genotype][files][indels", "Indel Genotype Spreadsheet: some columns in the file you provided are missing or have duplicate header values. Please either enter valid header values for those columns or remove those columns, then reupload your file.");
      }

      if (!form_get_errors()) {
        if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
          $species_index = 'species-1';
        }
        else {
          $num = substr($id, 9);
          $species_index = "species-$num";
        }
        $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file'];
        $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

        $acc_no_header = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-no-header'];
        $missing_trees = tpps_compare_files($indel_fid, $tree_accession_file, $id_col_name, $id_col_accession_name, FALSE, $acc_no_header);

        if ($missing_trees !== array()) {
          $tree_id_str = implode(', ', $missing_trees);
          form_set_error("$id][genotype][files][indels", "Indel Genotype Spreadsheet: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
        }
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($indel_fid);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Genotype_Indel_Assay_$org_num";
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
          'data' => array(0),
        ),
      );

      $file_element = $form[$id]['genotype']['files']['other'];
      $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

      if (!form_get_errors()) {
        // Get Tree Id column name.
        $id_col_genotype_name = $groups['Tree Id']['1'];
        $species_index = "species-$org_num";
        if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
          $species_index = "species-1";
        }
        $tree_accession_file = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file'];
        $id_col_accession_name = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-groups']['Tree Id']['1'];

        $acc_no_header = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index]['file-no-header'];
        $other_no_header = $genotype['files']['other-no-header'];
        $missing_trees = tpps_compare_files($other_file, $tree_accession_file, $id_col_genotype_name, $id_col_accession_name, $other_no_header, $acc_no_header);

        if ($missing_trees !== array()) {
          $tree_id_str = implode(', ', $missing_trees);
          form_set_error("$id][genotype][files][other", "Other Marker Genotype Spreadsheet: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
        }
      }

      if (!form_get_errors()) {
        // Preserve file if it is valid.
        $file = file_load($other_file);
        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
        $form_state['file_info'][TPPS_PAGE_4][$file->fid] = "Genotype_Other_Marker_Spreadsheet_$org_num";
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
function tpps_validate_environment(array &$environment, $id) {
  // Using cartogratree environment layers.
  $group_check = '';
  $new_layers = array();
  foreach ($environment['env_layers_groups'] as $group_name => $group_id) {
    if (!empty($group_id)) {
      $group_check .= "1";
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
      else {
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
    else {
      $group_check .= "0";
    }
  }

  if (!empty($environment['env_layers']['other'])) {
    if (empty($environment['env_layers']['other_db'])) {
      form_set_error("$id][environment][env_layers][other_db", 'Cartogratree other environmental layer DB: field is required.');
    }

    if (empty($environment['env_layers']['other_name'])) {
      form_set_error("$id][environment][env_layers][other_name", 'Cartogratree other environmental layer name: field is required.');
    }

    if (!form_get_errors()) {
      $new_layers['other'] = 'other';
      $new_layers['other_db'] = $environment['env_layers']['other_db'];
      $new_layers['other_name'] = $environment['env_layers']['other_name'];
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
