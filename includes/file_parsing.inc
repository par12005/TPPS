<?php

/**
 * @file
 * Defines function to parse genotype and phenotype files from the submission.
 */

require_once 'get_env_data.inc';

/**
 * Parses genotype and phenotype files and creates records in the database.
 *
 * This function is only called from Tripal Jobs because of the size of genotype
 * and phenotype files and the number of records needed to properly store and
 * integrate the data.
 *
 * @param string $accession
 *   The accession number of the completed submission.
 */
function tpps_file_parsing($accession) {
  $form_state = tpps_load_submission($accession);

  $fourthpage = $form_state['saved_values'][TPPS_PAGE_4];
  $firstpage = $form_state['saved_values'][TPPS_PAGE_1];
  $organism_number = $firstpage['organism']['number'];
  $file_rank = &$form_state['file_rank'];
  $record_group = variable_get('tpps_record_group', 10000);
  $project_id = $form_state['ids']['project_id'];
  $species_codes = array();
  for ($i = 1; $i <= $organism_number; $i++) {
    $species_codes[$form_state['ids']['organism_ids'][$i]] = current(chado_select_record('organismprop', array('value'), array(
      'type_id' => array(
        'name' => 'organism 4 letter code',
      ),
      'organism_id' => $form_state['ids']['organism_ids'][$i],
    ), array(
      'limit' => 1,
    )))->value;
  }

  for ($i = 1; $i <= $organism_number; $i++) {
    $parts = explode(" ", $firstpage['organism'][$i]);
    $genus = $parts[0];
    $species = implode(" ", array_slice($parts, 1));

    if (isset($parts[2]) and ($parts[2] == 'var.' or $parts[2] == 'subsp.')) {
      $infra = implode(" ", array_slice($parts, 2));
    }
    else {
      $infra = NULL;
    }

    if (isset($fourthpage["organism-$i"]['phenotype'])) {
      $phenotype = $fourthpage["organism-$i"]['phenotype'];

      // Get appropriate cvterms.
      $phenotype_cvterms = array(
        'time' => tpps_load_cvterm('time')->cvterm_id,
        'desc' => tpps_load_cvterm('description')->cvterm_id,
        'unit' => tpps_load_cvterm('unit')->cvterm_id,
        'min' => tpps_load_cvterm('minimum')->cvterm_id,
        'max' => tpps_load_cvterm('maximum')->cvterm_id,
      );

      $records = array(
        'phenotype' => array(),
        'phenotypeprop' => array(),
        'stock_phenotype' => array(),
      );
      $phenotype_count = 0;

      $options = array(
        'records' => $records,
        'cvterms' => $phenotype_cvterms,
        'accession' => $form_state['accession'],
        'tree_info' => $form_state['tree_info'],
        'suffix' => 0,
        'phenotype_count' => $phenotype_count,
      );

      if (empty($phenotype['iso-check'])) {
        $phenotype_number = $phenotype['phenotypes-meta']['number'];
        $phenotypes_meta = array();

        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($phenotype['file'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;

        // Populate $phenotypes_meta with manually entered metadata.
        for ($j = 1; $j <= $phenotype_number; $j++) {
          $name = strtolower($phenotype['phenotypes-meta'][$j]['name']);
          $phenotypes_meta[$name] = array();
          $phenotypes_meta[$name]['attr'] = $phenotype['phenotypes-meta'][$j]['attribute'];
          $phenotypes_meta[$name]['desc'] = $phenotype['phenotypes-meta'][$j]['description'];
          $phenotypes_meta[$name]['unit'] = $phenotype['phenotypes-meta'][$j]['units'];
          if ($phenotype['phenotypes-meta'][$j]['struct-check'] == '1') {
            $phenotypes_meta[$name]['struct'] = $phenotype['phenotypes-meta'][$j]['structure'];
          }
          if (!empty($phenotype['phenotypes-meta'][$j]['val-check']) or !empty($phenotype['phenotypes-meta'][$j]['bin-check'])) {
            $phenotypes_meta[$name]['min'] = $phenotype['phenotypes-meta'][$j]['min'];
            $phenotypes_meta[$name]['max'] = $phenotype['phenotypes-meta'][$j]['max'];
          }
          if ($phenotype['phenotypes-meta'][$j]['time-check'] == '1') {
            $phenotypes_meta[$name]['time'] = $phenotype['phenotypes-meta'][$j]['time'];
          }
        }

        if ($phenotype['check'] == '1') {
          $phenotype_meta_file_id = tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'schema',
              ),
              'name' => 'url',
              'is_obsolete' => 0,
            ),
            'value' => file_create_url(file_load($phenotype['metadata'])->uri),
            'rank' => $file_rank,
          ));
          $file_rank++;

          // Get metadata column values.
          $groups = $phenotype['metadata-groups'];
          $column_vals = $phenotype['metadata-columns'];
          $struct = array_search('5', $column_vals);
          $min = array_search('6', $column_vals);
          $max = array_search('7', $column_vals);
          $columns = array(
            'name' => $groups['Phenotype Id']['1'],
            'attr' => $groups['Attribute']['2'],
            'desc' => $groups['Description']['3'],
            'unit' => $groups['Units']['4'],
            'struct' => !empty($struct) ? $struct : NULL,
            'min' => !empty($min) ? $min : NULL,
            'max' => !empty($max) ? $max : NULL,
          );

          $meta_options = array(
            'no_header' => $phenotype['metadata-no-header'],
            'meta_columns' => $columns,
            'meta' => &$phenotypes_meta,
          );

          tpps_file_iterator($phenotype['metadata'], 'tpps_process_phenotype_meta', $meta_options);
        }

        tpps_refine_phenotype_meta($phenotypes_meta);

        // Get metadata header values.
        $groups = $phenotype['file-groups'];
        $column_vals = $phenotype['file-columns'];
        $time_index = ($phenotype['format'] == 0) ? '2' : '4';
        $clone_index = ($phenotype['format'] == 0) ? '3' : '5';
        $time = array_search($time_index, $column_vals);
        $clone = array_search($clone_index, $column_vals);
        $meta_headers = array(
          'name' => $groups['Phenotype Name/Identifier']['2'] ?? NULL,
          'value' => $groups['Phenotype Value(s)']['3'] ?? NULL,
          'time' => !empty($time) ? $time : NULL,
          'clone' => !empty($clone) ? $clone : NULL,
        );

        // Get data header values.
        if ($phenotype['format'] == 0) {
          $file_headers = tpps_file_headers($phenotype['file'], $phenotype['file-no-header']);
          $data_columns = array();
          foreach ($groups['Phenotype Data']['0'] as $col) {
            $data_columns[$col] = $file_headers[$col];
          }
          unset($file_headers);
        }

        $options['no_header'] = $phenotype['file-no-header'];
        $options['tree_id'] = $groups['Tree Identifier']['1'];
        $options['meta_headers'] = $meta_headers;
        $options['data_columns'] = $data_columns ?? NULL;
        $options['meta'] = $phenotypes_meta;
        $options['file_empty'] = $phenotype['file-empty'];

        tpps_file_iterator($phenotype['file'], 'tpps_process_phenotype_data', $options);
      }
      else {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($phenotype['iso'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;

        $options['iso'] = TRUE;
        $options['records'] = $records;
        $options['cvterms'] = $phenotype_cvterms;
        $options['file_headers'] = tpps_file_headers($phenotype['iso']);
        $options['meta'] = array(
          'desc' => "Mass Spectrometry",
          'unit' => "intensity (arbitrary units)",
          'attr_id' => tpps_load_cvterm('intensity')->cvterm_id,
        );

        tpps_file_iterator($phenotype['iso'], 'tpps_process_phenotype_data', $options);
      }
      tpps_chado_insert_multi($options['records']);
      unset($options['records']);
    }

    if (isset($fourthpage["organism-$i"]['genotype'])) {
      $genotype = $fourthpage["organism-$i"]['genotype'];
      $genotype_count = 0;
      $genotype_total = 0;
      $seq_var_cvterm = tpps_load_cvterm('sequence_variant')->cvterm_id;
      $overrides = array(
        'genotype_call' => array(
          'variant' => array(
            'table' => 'feature',
            'columns' => array(
              'variant_id' => 'feature_id',
            ),
          ),
          'marker' => array(
            'table' => 'feature',
            'columns' => array(
              'marker_id' => 'feature_id',
            ),
          ),
        ),
      );

      $records = array(
        'feature' => array(),
        'genotype' => array(),
        'genotype_call' => array(),
        'stock_genotype' => array(),
      );

      $multi_insert_options = array(
        'fk_overrides' => $overrides,
        'entities' => array(
          'label' => 'Genotype',
          'table' => 'genotype',
        ),
      );

      $options = array(
        'records' => $records,
        'tree_info' => $form_state['tree_info'],
        'species_codes' => $species_codes,
        'genotype_count' => $genotype_count,
        'genotype_total' => &$genotype_total,
        'project_id' => $project_id,
        'seq_var_cvterm' => $seq_var_cvterm,
        'overrides' => $overrides,
        'multi_insert' => $multi_insert_options,
      );

      /*if ($genotype['ref-genome'] == 'bio') {

        $bioproject_id = tpps_chado_insert_record('dbxref', array(
          'db_id' => array(
            'name' => 'NCBI BioProject',
          ),
          'accession' => $genotype['BioProject-id'],
        ));

        $project_dbxref_id = tpps_chado_insert_record('project_dbxref', array(
          'project_id' => $project_id,
          'dbxref_id' => $bioproject_id,
        ));

        $bioproject_assembly_file_ids = array();
        foreach ($genotype['assembly-auto'] as $key => $val) {
          if ($val == '1') {
            array_push($bioproject_assembly_file_ids, tpps_chado_insert_record('projectprop', array(
              'project_id' => $project_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'schema',
                ),
                'name' => 'url',
                'is_obsolete' => 0,
              ),
              'value' => "https://www.ncbi.nlm.nih.gov/nuccore/$key",
              'rank' => $file_rank,
            )));
            $file_rank++;
          }
        }
      }
      else*/
      if ($genotype['ref-genome'] == 'manual' or $genotype['ref-genome'] == 'manual2' or $genotype['ref-genome'] == 'url') {
        if ($genotype['tripal_fasta']['file_upload']) {
          // Uploaded new file.
          $assembly_user = $genotype['tripal_fasta']['file_upload'];
          $assembly_user_id = tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'schema',
              ),
              'name' => 'url',
              'is_obsolete' => 0,
            ),
            'value' => file_create_url(file_load($assembly_user)->uri),
            'rank' => $file_rank,
          ));
          $file_rank++;
        }
        if ($genotype['tripal_fasta']['file_upload_existing']) {
          // Uploaded existing file.
          $assembly_user = $genotype['tripal_fasta']['file_upload_existing'];
          $assembly_user_id = tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'schema',
              ),
              'name' => 'url',
              'is_obsolete' => 0,
            ),
            'value' => file_create_url(file_load($assembly_user)->uri),
            'rank' => $file_rank,
          ));
          $file_rank++;
        }
        if ($genotype['tripal_fasta']['file_remote']) {
          // Provided url to file.
          $assembly_user = $genotype['tripal_fasta']['file_remote'];
          $assembly_user_id = tpps_chado_insert_record('projectprop', array(
            'project_id' => $project_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'schema',
              ),
              'name' => 'url',
              'is_obsolete' => 0,
            ),
            'value' => $assembly_user,
            'rank' => $file_rank,
          ));
          $file_rank++;
        }
      }
      elseif ($genotype['ref-genome'] != 'none') {
        $reference_genome_id = tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'sequence',
            ),
            'name' => 'reference_genome',
            'is_obsolete' => 0,
          ),
          'value' => $genotype['ref-genome'],
        ));
      }

      if (!empty($genotype['files']['file-type']['SNPs Genotype Assay'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($genotype['files']['snps-assay'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;

        $options['type'] = 'snp';
        $options['headers'] = tpps_file_headers($genotype['files']['snps-assay']);
        $options['marker'] = 'SNP';
        $options['type_cvterm'] = tpps_load_cvterm('snp')->cvterm_id;

        tpps_file_iterator($genotype['files']['snps-assay'], 'tpps_process_genotype_spreadsheet', $options);

        tpps_chado_insert_multi($options['records'], $multi_insert_options);
        unset($options['records']);
        $genotype_total += $genotype_count;
        $genotype_count = 0;
      }

      if (!empty($genotype['files']['file-type']['Assay Design']) and $genotype['marker-type']['SNPs']) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($genotype['files']['assay-design'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;
      }

      if (!empty($genotype['files']['file-type']['SSRs/cpSSRs Genotype Spreadsheet'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($genotype['files']['ssrs'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;

        $options['type'] = 'ssrs';
        $options['headers'] = tpps_ssrs_headers($genotype['files']['ssrs'], $genotype['files']['ploidy']);
        $options['marker'] = $genotype['SSRs/cpSSRs'];
        $options['type_cvterm'] = tpps_load_cvterm('ssr')->cvterm_id;

        tpps_file_iterator($genotype['files']['ssrs'], 'tpps_process_genotype_spreadsheet', $options);

        tpps_chado_insert_multi($options['records'], $multi_insert_options);
        unset($options['records']);
        $genotype_count = 0;
      }

      if (!empty($genotype['files']['file-type']['Other Marker Genotype Spreadsheet'])) {
        tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($genotype['files']['other'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;

        $groups = $genotype['files']['other-groups'];

        $options['type'] = 'other';
        $options['headers'] = tpps_other_marker_headers($genotype['files']['other'], $groups['Genotype Data'][0]);
        $options['marker'] = $genotype['other-marker'];
        $options['type_cvterm'] = tpps_load_cvterm('genetic_marker')->cvterm_id;
        $options['tree_id'] = $groups['Tree Id'][1];

        tpps_file_iterator($genotype['files']['other'], 'tpps_process_genotype_spreadsheet', $options);

        tpps_chado_insert_multi($options['records'], $multi_insert_options);
        unset($options['records']);
        $genotype_count = 0;
      }

      if (!empty($genotype['files']['file-type']['VCF'])) {
        // TODO: we probably want to use tpps_file_iterator to parse vcf files.

        $genotype_vcf_id = tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($genotype['files']['vcf'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;

        $marker = 'SNP';

        $records['genotypeprop'] = array();

        $snp_cvterm = tpps_load_cvterm('snp')->cvterm_id;
        $format_cvterm = tpps_load_cvterm('format')->cvterm_id;
        $qual_cvterm = tpps_load_cvterm('quality_value')->cvterm_id;
        $filter_cvterm = tpps_load_cvterm('filter')->cvterm_id;
        $freq_cvterm = tpps_load_cvterm('allelic_frequency')->cvterm_id;
        $depth_cvterm = tpps_load_cvterm('read_depth')->cvterm_id;
        $n_sample_cvterm = tpps_load_cvterm('number_samples')->cvterm_id;

        $vcf_file = file_load($genotype['files']['vcf']);
        $location = drupal_realpath($vcf_file->uri);
        $vcf_content = fopen($location, 'r');
        $stocks = array();
        $format = "";
        $current_id = $form_state['ids']['organism_ids'][$i];
        $species_code = $species_codes[$current_id];

        // dpm('start: ' . date('r'));.
        while (($vcf_line = fgets($vcf_content)) !== FALSE) {
          if ($vcf_line[0] != '#') {
            $genotype_count++;
            $vcf_line = explode("\t", $vcf_line);
            $scaffold_id = &$vcf_line[0];
            $position = &$vcf_line[1];
            $marker_name = &$vcf_line[2];
            $ref = &$vcf_line[3];
            $alt = &$vcf_line[4];
            $qual = &$vcf_line[5];
            $filter = &$vcf_line[6];
            $info = &$vcf_line[7];

            if (empty($variant_name) or $variant_name == '.') {
              $variant_name = "{$scaffold_id}{$position}$ref:$alt";
            }
            $marker_name = $variant_name . $marker;
            $description = "$ref:$alt";
            $genotype_name = "$marker-$species_code-$scaffold_id-$position";
            $genotype_desc = "$marker-$species_code-$scaffold_id-$position-$description";

            $records['feature'][$marker_name] = array(
              'organism_id' => $current_id,
              'uniquename' => $marker_name,
              'type_id' => $seq_var_cvterm,
            );

            $records['feature'][$variant_name] = array(
              'organism_id' => $current_id,
              'uniquename' => $variant_name,
              'type_id' => $seq_var_cvterm,
            );

            $records['genotype'][$genotype_desc] = array(
              'name' => $genotype_name,
              'uniquename' => $genotype_desc,
              'description' => $description,
              'type_id' => $snp_cvterm,
            );

            if ($format != "") {
              $records['genotypeprop']["$genotype_desc-format"] = array(
                'type_id' => $format_cvterm,
                'value' => $format,
                '#fk' => array(
                  'genotype' => $genotype_desc,
                ),
              );
            }

            for ($j = 9; $j < count($vcf_line); $j++) {
              $records['genotype_call']["{$stocks[$j - 9]}-$genotype_name"] = array(
                'project_id' => $project_id,
                'stock_id' => $stocks[$j - 9],
                '#fk' => array(
                  'genotype' => $genotype_desc,
                  'variant' => $variant_name,
                  'marker' => $marker_name,
                ),
              );

              $records['stock_genotype']["{$stocks[$j - 9]}-$genotype_name"] = array(
                'stock_id' => $stocks[$j - 9],
                '#fk' => array(
                  'genotype' => $genotype_desc,
                ),
              );
            }

            // Quality score.
            $records['genotypeprop']["$genotype_desc-qual"] = array(
              'type_id' => $qual_cvterm,
              'value' => $qual,
              '#fk' => array(
                'genotype' => $genotype_desc,
              ),
            );

            // filter: pass/fail.
            $records['genotypeprop']["$genotype_desc-filter"] = array(
              'type_id' => $filter_cvterm,
              'value' => ($filter == '.') ? "P" : "NP",
              '#fk' => array(
                'genotype' => $genotype_desc,
              ),
            );

            // Break up info column.
            $info_vals = explode(";", $info);
            foreach ($info_vals as $key => $val) {
              $parts = explode("=", $val);
              unset($info_vals[$key]);
              $info_vals[$parts[0]] = isset($parts[1]) ? $parts[1] : '';
            }

            // Allele frequency, assuming that the info code for allele
            // frequency is 'AF'.
            if (isset($info_vals['AF']) and $info_vals['AF'] != '') {
              $records['genotypeprop']["$genotype_desc-freq"] = array(
                'type_id' => $freq_cvterm,
                'value' => $info_vals['AF'],
                '#fk' => array(
                  'genotype' => $genotype_desc,
                ),
              );
            }

            // Depth coverage, assuming that the info code for depth coverage is
            // 'DP'.
            if (isset($info_vals['DP']) and $info_vals['DP'] != '') {
              $records['genotypeprop']["$genotype_desc-depth"] = array(
                'type_id' => $depth_cvterm,
                'value' => $info_vals['DP'],
                '#fk' => array(
                  'genotype' => $genotype_desc,
                ),
              );
            }

            // Number of samples, assuming that the info code for number of
            // samples is 'NS'.
            if (isset($info_vals['NS']) and $info_vals['NS'] != '') {
              $records['genotypeprop']["$genotype_desc-n_sample"] = array(
                'type_id' => $n_sample_cvterm,
                'value' => $info_vals['NS'],
                '#fk' => array(
                  'genotype' => $genotype_desc,
                ),
              );
            }
            // Tripal Job has issues when all submissions are made at the same
            // time, so break them up into groups of 10,000 genotypes along with
            // their relevant genotypeprops.
            if ($genotype_count > $record_group) {
              $genotype_count = 0;
              tpps_chado_insert_multi($records, $multi_insert_options);
              $records = array(
                'feature' => array(),
                'genotype' => array(),
                'genotype_call' => array(),
                'genotypeprop' => array(),
                'stock_genotype' => array(),
              );
              $genotype_count = 0;
            }
          }
          elseif (preg_match('/##FORMAT=/', $vcf_line)) {
            $format .= substr($vcf_line, 9, -1);
          }
          elseif (preg_match('/#CHROM/', $vcf_line)) {
            $vcf_line = explode("\t", $vcf_line);
            for ($j = 9; $j < count($vcf_line); $j++) {
              $stocks[] = $form_state['tree_info'][trim($vcf_line[$j])]['stock_id'];
            }
          }
        }
        // Insert the last set of values.
        tpps_chado_insert_multi($records, $multi_insert_options);
        unset($records);
        $genotype_count = 0;
        // dpm('done: ' . date('r'));.
      }
    }

    if (isset($fourthpage["organism-$i"]['environment'])) {
      $environment = $fourthpage["organism-$i"]['environment'];
      $env_layers_check = isset($environment['use_layers']) ? $environment['use_layers'] : FALSE;
      $env_layers = isset($environment['env_layers']) ? $environment['env_layers'] : FALSE;
      $env_params = isset($environment['env_params']) ? $environment['env_params'] : FALSE;
      $env_number = $environment['env_manual']['number'];
      $env_count = 0;

      $species_index = "species-$i";
      if (empty($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check'])) {
        $species_index = "species-1";
      }
      $tree_accession = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession'][$species_index];
      $id_col = $tree_accession['file-groups']['Tree Id'][1];

      $env_cvterm = tpps_load_cvterm('environment')->cvterm_id;

      if ($env_layers_check and db_table_exists('cartogratree_layers') and db_table_exists('cartogratree_fields')) {
        $layers_params = array();
        $records = array(
          'phenotype' => array(),
          'phenotype_cvterm' => array(),
          'stock_phenotype' => array(),
        );

        foreach ($env_layers as $layer_name => $layer_id) {
          if (!empty($layer_id) and !empty($env_params[$layer_name])) {
            $layers_params[$layer_id] = array();
            $params = $env_params[$layer_name];
            foreach ($params as $param_name => $param_id) {
              if (!empty($param_id)) {
                $layers_params[$layer_id][$param_id] = $param_name;
              }
            }
          }
          elseif (!empty($layer_id) and preg_match('/worldclim_subgroup_(.+)/', $layer_id, $matches)) {
            $subgroup_id = $matches[1];
            $layers = db_select('cartogratree_layers', 'l')
              ->fields('l', array('layer_id'))
              ->condition('subgroup_id', $subgroup_id)
              ->execute();
            while (($layer = $layers->fetchObject())) {
              $params = db_select('cartogratree_fields', 'f')
                ->fields('f', array('field_id', 'display_name'))
                ->condition('layer_id', $layer->layer_id)
                ->execute();
              while (($param = $params->fetchObject())) {
                $layers_params[$layer->layer_id][$param->field_id] = $param->display_name;
              }
            }
          }
        }

        $options = array(
          'no_header' => !empty($tree_accession['file-no-header']),
          'records' => $records,
          'tree_id' => $tree_accession['file-groups']['Tree Id'][1],
          'accession' => $form_state['accession'],
          'tree_info' => $form_state['tree_info'],
          'layers_params' => $layers_params,
          'env_count' => &$env_count,
          'env_cvterm' => $env_cvterm,
          'suffix' => 0,
        );

        tpps_file_iterator($tree_accession['file'], 'tpps_process_environment_layers', $options);

        tpps_chado_insert_multi($options['records']);
        unset($options['records']);
        $env_count = 0;
      }

      $env_meta = array();

      for ($j = 1; $j <= $env_number; $j++) {
        $current_env = $environment['env_manual'][$j];
        $env_meta[] = array(
          'name' => $current_env['name'],
          'desc' => $current_env['description'],
          'unit' => $current_env['units'],
          'val' => $current_env['value'],
        );
      }

      $records = array(
        'phenotype' => array(),
        'stock_phenotype' => array(),
        'phenotypeprop' => array(),
      );

      $options = array(
        'no_header' => !empty($tree_accession['file-no-header']),
        'accession' => $form_state['accession'],
        'records' => $records,
        'env_meta' => $env_meta,
        'env_count' => $env_count,
        'suffix' => 0,
        'tree_info' => $form_state['tree_info'],
        'tree_id' => $tree_accession['file-groups']['Tree Id'][1],
        'env_cvterm' => $env_cvterm,
        'desc_id' => tpps_load_cvterm('description')->cvterm_id,
        'unit_id' => tpps_load_cvterm('unit')->cvterm_id,
      );

      tpps_file_iterator($tree_accession['file'], 'tpps_process_environment_manual', $options);

      tpps_chado_insert_multi($options['records']);
      unset($options['records']);
    }
  }

  tpps_update_submission($form_state);
}

/**
 * This function will process a row from a phenotype metadata file.
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_phenotype_meta($row, array &$options = array()) {
  $columns = $options['meta_columns'];
  $meta = &$options['meta'];

  $name = strtolower($row[$columns['name']]);
  $meta[$name] = array();
  $meta[$name]['attr'] = $row[$columns['attr']];
  $meta[$name]['desc'] = $row[$columns['desc']];
  $meta[$name]['unit'] = $row[$columns['unit']];
  if (!empty($columns['struct']) and isset($row[$columns['struct']]) and $row[$columns['struct']] != '') {
    $meta[$name]['struct'] = $row[$columns['struct']];
  }
  if (!empty($columns['min']) and isset($row[$columns['min']]) and $row[$columns['min']] != '') {
    $meta[$name]['min'] = $row[$columns['min']];
  }
  if (!empty($columns['max']) and isset($row[$columns['max']]) and $row[$columns['max']] != '') {
    $meta[$name]['max'] = $row[$columns['max']];
  }
}

/**
 * This function will further refine existing phenotype metadata.
 *
 * The function mostly just adds cvterm ids where applicable.
 *
 * @param array $meta
 *   The existing metadata array.
 */
function tpps_refine_phenotype_meta(array &$meta) {
  $cvt_cache = array();
  $local_cv = chado_get_cv(array('name' => 'local'));
  $local_db = variable_get('tpps_local_db');
  foreach ($meta as $name => $data) {
    if (!empty($cvt_cache[$data['attr']])) {
      $meta[$name]['attr_id'] = $cvt_cache[$data['attr']];
    }
    else {
      $attr = chado_select_record('cvterm', array('cvterm_id'), array(
        'name' => array(
          'data' => $data['attr'],
          'op' => 'LIKE',
        ),
      ), array(
        'limit' => 1,
      ));
      $meta[$name]['attr_id'] = current($attr)->cvterm_id ?? NULL;

      if (empty($meta[$name]['attr_id'])) {
        $meta[$name]['attr_id'] = chado_insert_cvterm(array(
          'id' => "{$local_db->name}:{$data['attr']}",
          'name' => $data['attr'],
          'definition' => '',
          'cv_name' => $local_cv->name,
        ))->cvterm_id;
      }
      $cvt_cache[$data['attr']] = $meta[$name]['attr_id'];
    }

    if (!empty($data['struct'])) {
      if (!empty($cvt_cache[$data['struct']])) {
        $meta[$name]['struct_id'] = $cvt_cache[$data['struct']];
      }
      else {
        $obs = chado_select_record('cvterm', array('cvterm_id'), array(
          'name' => array(
            'data' => $data['struct'],
            'op' => 'LIKE',
          ),
        ), array(
          'limit' => 1,
        ));
        $meta[$name]['struct_id'] = current($obs)->cvterm_id ?? NULL;

        if (empty($meta[$name]['struct_id'])) {
          $meta[$name]['struct_id'] = chado_insert_cvterm(array(
            'id' => "{$local_db->name}:{$data['struct']}",
            'name' => $data['struct'],
            'definition' => '',
            'cv_name' => $local_cv->name,
          ))->cvterm_id;
        }
        $cvt_cache[$data['struct']] = $meta[$name]['struct_id'];
      }
    }
    else {
      $meta[$name]['struct_id'] = NULL;
    }
  }
}

/**
 * This function will process a row from a phenotype data file.
 *
 * This function is used for standard phenotypes of both phenotype formats, as
 * well as phenotype isotope files. The functionality is slightly different
 * based on the type of phenotype file being processed (set in the options
 * array). This function is meant to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_phenotype_data($row, array &$options = array()) {
  $iso = $options['iso'] ?? FALSE;
  $records = &$options['records'];
  $meta_headers = $options['meta_headers'] ?? NULL;
  $file_headers = $options['file_headers'] ?? NULL;
  $cvterms = $options['cvterms'];
  $meta = $options['meta'];
  $empty = $options['file-empty'] ?? NULL;
  $accession = $options['accession'];
  $suffix = &$options['suffix'];
  $tree_info = &$options['tree_info'];
  $phenotype_count = &$options['phenotype_count'];
  $record_group = variable_get('tpps_record_group', 10000);

  if (!$iso) {
    if (isset($meta_headers['name']) and (isset($meta_headers['value']))) {
      $id = $row[$meta_headers['value']];
      $values = array($id => $row[$meta_headers['name']]);
    }

    if (!empty($options['data_columns'])) {
      $values = $options['data_columns'];
    }

    $tree_id = $row[$options['tree_id']];
    $clone_col = $meta_headers['clone'] ?? NULL;
    if (isset($clone_col) and !empty($row[$clone_col]) and $row[$clone_col] !== $empty) {
      $tree_id .= "-" . $row[$clone_col];
    }
  }
  else {
    foreach ($row as $id => $value) {
      if (empty($tree_id)) {
        $tree_id = $value;
        continue;
      }
      $values[$id] = $file_headers[$id];
    }
  }

  foreach ($values as $id => $name) {
    $attr_id = $iso ? $meta['attr_id'] : $meta[strtolower($name)]['attr_id'];
    $value = $row[$id];
    $phenotype_name = "$accession-$tree_id-$name-$suffix";

    $records['phenotype'][$phenotype_name] = array(
      'uniquename' => $phenotype_name,
      'name' => $name,
      'attr_id' => $attr_id,
      'observable_id' => $meta[strtolower($name)]['struct_id'] ?? NULL,
      'value' => $value,
    );

    $records['stock_phenotype'][$phenotype_name] = array(
      'stock_id' => $tree_info[$tree_id]['stock_id'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    if (isset($meta[strtolower($name)]['time'])) {
      $records['phenotypeprop']["$phenotype_name-time"] = array(
        'type_id' => $cvterms['time'],
        'value' => $meta[strtolower($name)]['time'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
    }
    elseif (isset($meta_headers['time'])) {
      $records['phenotypeprop']["$phenotype_name-time"] = array(
        'type_id' => $cvterms['time'],
        'value' => $row[$meta_headers['time']],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
    }

    $records['phenotypeprop']["$phenotype_name-desc"] = array(
      'type_id' => $cvterms['desc'],
      'value' => $iso ? $meta['desc'] : $meta[strtolower($name)]['desc'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    $records['phenotypeprop']["$phenotype_name-unit"] = array(
      'type_id' => $cvterms['unit'],
      'value' => $iso ? $meta['unit'] : $meta[strtolower($name)]['unit'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    if (isset($meta[strtolower($name)]['min'])) {
      $records['phenotypeprop']["$phenotype_name-min"] = array(
        'type_id' => $cvterms['min'],
        'value' => $meta[strtolower($name)]['min'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
    }

    if (isset($meta[strtolower($name)]['max'])) {
      $records['phenotypeprop']["$phenotype_name-max"] = array(
        'type_id' => $cvterms['max'],
        'value' => $meta[strtolower($name)]['max'],
        '#fk' => array(
          'phenotype' => $phenotype_name,
        ),
      );
    }

    if ($phenotype_count > $record_group) {
      tpps_chado_insert_multi($records);
      $records = array(
        'phenotype' => array(),
        'phenotypeprop' => array(),
        'stock_phenotype' => array(),
      );
      $phenotype_count = 0;
    }

    $phenotype_count++;
  }
  $suffix++;
}

/**
 * This function processes a single row of a genotype spreadsheet.
 *
 * This function is used for SNP assay files, SSR spreadsheets, and other
 * marker type spreadsheets. The functionality is slightly different based on
 * the type of marker being processed (this is set in the options array). This
 * function is meant to be used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_genotype_spreadsheet($row, array &$options = array()) {
  $type = $options['type'];
  $records = &$options['records'];
  $headers = $options['headers'];
  $tree_info = &$options['tree_info'];
  $species_codes = $options['species_codes'];
  $genotype_count = &$options['genotype_count'];
  $genotype_total = &$options['genotype_total'];
  $project_id = $options['project_id'];
  $marker = $options['marker'];
  $type_cvterm = $options['type_cvterm'];
  $seq_var_cvterm = $options['seq_var_cvterm'];
  $multi_insert_options = $options['multi_insert'];
  $record_group = variable_get('tpps_record_group', 10000);
  $stock_id = NULL;
  if ($type == 'other') {
    $val = $row[$options['tree_id']];
    $stock_id = $tree_info[trim($val)]['stock_id'];
    $current_id = $tree_info[trim($val)]['organism_id'];
    $species_code = $species_codes[$current_id];
  }
  foreach ($row as $key => $val) {
    if (empty($headers[$key])) {
      continue;
    }

    if (!isset($stock_id)) {
      $stock_id = $tree_info[trim($val)]['stock_id'];
      $current_id = $tree_info[trim($val)]['organism_id'];
      $species_code = $species_codes[$current_id];
      continue;
    }
    $genotype_count++;

    if ($type == 'ssrs' and ($val === 0 or $val === "0")) {
      $val = "NA";
    }

    $variant_name = $headers[$key];
    $marker_name = $variant_name . $marker;
    $genotype_name = "$marker-$variant_name-$species_code-$val";

    $records['feature'][$marker_name] = array(
      'organism_id' => $current_id,
      'uniquename' => $marker_name,
      'type_id' => $seq_var_cvterm,
    );

    $records['feature'][$variant_name] = array(
      'organism_id' => $current_id,
      'uniquename' => $variant_name,
      'type_id' => $seq_var_cvterm,
    );

    $records['genotype'][$genotype_name] = array(
      'name' => $genotype_name,
      'uniquename' => $genotype_name,
      'description' => $val,
      'type_id' => $type_cvterm,
    );

    $records['genotype_call']["$stock_id-$genotype_name"] = array(
      'project_id' => $project_id,
      'stock_id' => $stock_id,
      '#fk' => array(
        'genotype' => $genotype_name,
        'variant' => $variant_name,
        'marker' => $marker_name,
      ),
    );

    $records['stock_genotype']["$stock_id-$genotype_name"] = array(
      'stock_id' => $stock_id,
      '#fk' => array(
        'genotype' => $genotype_name,
      ),
    );

    if ($genotype_count >= $record_group) {
      tpps_chado_insert_multi($records, $multi_insert_options);
      $records = array(
        'feature' => array(),
        'genotype' => array(),
        'genotype_call' => array(),
        'stock_genotype' => array(),
      );
      $genotype_total += $genotype_count;
      $genotype_count = 0;
    }
  }
}

/**
 * This function formats headers for a microsatellite spreadsheet.
 *
 * SSR/cpSSR spreadsheets will often have blank or duplicate headers, depending
 * on the ploidy of the organism they are meant for. This file standardizes the
 * headers for the spreadsheet so that they can be used with the
 * tpps_process_genotype_spreadsheet() function.
 *
 * @param int $fid
 *   The Drupal managed file id of the file.
 * @param string $ploidy
 *   The ploidy of the organism, as indicated by the user.
 *
 * @return array
 *   The array of standardized headers for the spreadsheet.
 */
function tpps_ssrs_headers($fid, $ploidy) {
  $headers = tpps_file_headers($fid);
  if ($ploidy == 'Haploid') {
    return $headers;
  }
  $row_len = count($headers);
  $results = $headers;

  while (($k = array_search(NULL, $results))) {
    unset($results[$k]);
  }

  $marker_num = 0;
  $first = TRUE;
  reset($headers);
  $num_headers = count($results);
  $num_unique_headers = count(array_unique($results));

  foreach ($headers as $key => $val) {
    next($headers);
    $next_key = key($headers);
    if ($first) {
      $first = FALSE;
      continue;
    }

    switch ($ploidy) {
      case 'Diploid':
        if ($num_headers == ($row_len + 1) / 2) {
          // Every other marker column name is left blank.
          if (array_key_exists($key, $results)) {
            $last = $results[$key];
            $results[$key] .= "_A";
            break;
          }
          $results[$key] = $last . "_B";
          break;
        }
        
        if ($num_headers == $row_len) {
          // All of the marker column names are filled out.
          if ($num_headers != $num_unique_headers) {
            // The marker column names are duplicates, need to append
            // _A and _B.
            if ($results[$key] == $results[$next_key]) {
              $results[$key] .= "_A";
              break;
            }
            $results[$key] .= "_B";
          }
        }
        break;

      case 'Polyploid':
        if ($num_headers == $row_len) {
          // All of the marker column names are filled out.
          if ($num_unique_headers != $num_headers) {
            // The marker column names are duplicates, need to append
            // _1, _2, up to X ploidy.
            // The total number of headers divided by the number of
            // unique headers should be equal to the ploidy.
            $ploidy_suffix = ($marker_num % ($num_headers - 1 / $num_unique_headers - 1)) + 1;
            $results[$key] .= "_$ploidy_suffix";
          }
          $marker_num++;
          break;
        }
        $ploidy_suffix = ($marker_num % ($row_len - 1 / $num_headers - 1)) + 1;
        if (array_key_exists($key, $results)) {
          $last = $results[$key];
          $results[$key] .= "_$ploidy_suffix";
        }
        else {
          $results[$key] = "{$last}_$ploidy_suffix";
        }
        $marker_num++;
        break;

      default:
        break;
    }
  }

  return $results;
}

/**
 * This function formats headers for the "other" type genotype markers.
 *
 * The headers for the "other" genotype marker types are set by the users, so
 * we need to return the names of the headers they have indicated, rather than
 * the values provided in the file-groups array.
 *
 * @param int $fid
 *   The Drupal managed file id of the file.
 * @param array $cols
 *   An array of columns indicating which of the columns contain genotype data.
 *
 * @return array
 *   The array of standardized headers for the spreadsheet.
 */
function tpps_other_marker_headers($fid, array $cols) {
  $headers = tpps_file_headers($fid);
  $results = array();
  foreach ($cols as $col) {
    $results[$col] = $headers[$col];
  }
  return $results;
}

/**
 * This function processes a single row of a tree accession file.
 *
 * This function populates the db with environmental data provided through
 * CartograTree layers. This function is meant to be used with
 * tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_environment_layers($row, array &$options = array()) {
  $id_col = $options['tree_id'];
  $records = &$options['records'];
  $tree_info = &$options['tree_info'];
  $layers_params = $options['layers_params'];
  $env_count = &$options['env_count'];
  $accession = $options['accession'];
  $suffix = &$options['suffix'];
  $env_cvterm = $options['env_cvterm'];
  $record_group = variable_get('tpps_record_group', 10000);

  $tree_id = $row[$id_col];
  $stock_id = $tree_info[$tree_id]['stock_id'];

  $gps_query = chado_select_record('stockprop', array('value'), array(
    'stock_id' => $stock_id,
    'type_id' => array(
      'name' => 'gps_latitude',
    ),
  ), array(
    'limit' => 1,
  ));
  $lat = current($gps_query)->value;

  $gps_query = chado_select_record('stockprop', array('value'), array(
    'stock_id' => $stock_id,
    'type_id' => array(
      'name' => 'gps_longitude',
    ),
  ), array(
    'limit' => 1,
  ));
  $long = current($gps_query)->value;

  foreach ($layers_params as $layer_id => $params) {
    $layer_query = db_select('cartogratree_layers', 'l')
      ->fields('l', array('title'))
      ->condition('layer_id', $layer_id)
      ->execute();

    $layer_name = $layer_query->fetchObject()->title;

    foreach ($params as $param_id => $param) {
      $param_query = db_select('cartogratree_fields', 'f')
        ->fields('f', array('field_name'))
        ->condition('field_id', $param_id)
        ->execute();

      $param_name = $param_query->fetchObject()->field_name;
      $phenotype_name = "$accession-$tree_id-$layer_name-$param_name-$suffix";

      $value = tpps_get_environmental_layer_data($layer_id, $lat, $long, $param_name);
      $type = variable_get("tpps_param_{$param_id}_type", 'attr_id');

      if ($type == 'attr_id') {
        $records['phenotype'][$phenotype_name] = array(
          'uniquename' => $phenotype_name,
          'name' => $param_name,
          'attr_id' => $env_cvterm,
          'value' => $value,
        );

        $records['stock_phenotype'][$phenotype_name] = array(
          'stock_id' => $stock_id,
          '#fk' => array(
            'phenotype' => $phenotype_name,
          ),
        );
      }
      else {
        $records['phenotype'][$phenotype_name] = array(
          'uniquename' => $phenotype_name,
          'name' => "$param_name",
          'value' => "$value",
        );

        $records['phenotype_cvterm'][$phenotype_name] = array(
          'cvterm_id' => $env_cvterm,
          '#fk' => array(
            'phenotype' => $phenotype_name,
          ),
        );

        $records['stock_phenotype'][$phenotype_name] = array(
          'stock_id' => $stock_id,
          '#fk' => array(
            'phenotype' => $phenotype_name,
          ),
        );
      }

      $env_count++;
      if ($env_count >= $record_group) {
        tpps_chado_insert_multi($records);
        $records = array(
          'phenotype' => array(),
          'phenotype_cvterm' => array(),
          'stock_phenotype' => array(),
        );
        $env_count = 0;
      }
    }
  }
  $suffix++;
}

/**
 * This function processes a single row of a tree accession file.
 *
 * This function populates the db with environmental data provided manually by
 * the user (as opposed to CartograTree layers). This function is meant to be
 * used with tpps_file_iterator().
 *
 * @param mixed $row
 *   The item yielded by the TPPS file generator.
 * @param array $options
 *   Additional options set when calling tpps_file_iterator().
 */
function tpps_process_environment_manual($row, array &$options = array()) {
  $records = &$options['records'];
  $accession = $options['accession'];
  $id_col = $options['tree_id'];
  $suffix = &$options['suffix'];
  $env_meta = $options['env_meta'];
  $env_count = &$options['env_count'];
  $desc_id = $options['desc_id'];
  $unit_id = $options['unit_id'];
  $env_cvterm = $options['env_cvterm'];
  $tree_info = &$options['tree_info'];
  $record_group = variable_get('tpps_record_group', 10000);

  $tree_id = $row[$id_col];
  foreach ($env_meta as $current_env) {
    $name = $current_env['name'];
    $desc = $current_env['desc'];
    $unit = $current_env['unit'];
    $val = $current_env['val'];
    $phenotype_name = "$accession-$tree_id-$name-$suffix";

    $records['phenotype'][$phenotype_name] = array(
      'uniquename' => $phenotype_name,
      'name' => $name,
      'attr_id' => $env_cvterm,
      'value' => $val,
    );

    $records['stock_phenotype'][$phenotype_name] = array(
      'stock_id' => $tree_info[$tree_id]['stock_id'],
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    $records['phenotypeprop']["$phenotype_name-desc"] = array(
      'type_id' => $desc_id,
      'value' => $desc,
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    $records['phenotypeprop']["$phenotype_name-unit"] = array(
      'type_id' => $unit_id,
      'value' => $unit,
      '#fk' => array(
        'phenotype' => $phenotype_name,
      ),
    );

    $env_count++;
    if ($env_count >= $record_group) {
      tpps_chado_insert_multi($records);
      $records = array(
        'phenotype' => array(),
        'stock_phenotype' => array(),
        'phenotypeprop' => array(),
      );
      $env_count = 0;
    }
  }
  $suffix++;
}
