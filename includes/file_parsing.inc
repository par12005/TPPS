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
  $genotype_group = variable_get('tpps_genotype_group', 10000);
  $project_id = $form_state['ids']['project_id'];

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
      $phenotype_number = $phenotype['number'];
      $phenotype_ids = array();
      $phenotypeprop_ids = array();
      $phenotypes_meta = array();
      $local_cv = chado_get_cv(array('name' => 'local'));
      $local_db = variable_get('tpps_local_db');

      $phenotype_file_id = tpps_chado_insert_record('projectprop', array(
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
      $phenotype_file = file_load($phenotype['file']);
      $file_rank++;

      for ($j = 1; $j <= $phenotype_number; $j++) {
        $name = strtolower($phenotype['phenotypes-meta'][$j]['name']);
        $phenotypes_meta[$name] = array();
        $phenotypes_meta[$name]['attr'] = $phenotype['phenotypes-meta'][$j]['attribute'];
        $phenotypes_meta[$name]['desc'] = $phenotype['phenotypes-meta'][$j]['description'];
        $phenotypes_meta[$name]['unit'] = $phenotype['phenotypes-meta'][$j]['units'];
        if ($phenotype['phenotypes-meta'][$j]['struct-check'] == '1') {
          $phenotypes_meta[$name]['struct'] = $phenotype['phenotypes-meta'][$j]['structure'];
        }
        if ($phenotype['phenotypes-meta'][$j]['val-check'] == '1') {
          $phenotypes_meta[$name]['min'] = $phenotype['phenotypes-meta'][$j]['min'];
          $phenotypes_meta[$name]['max'] = $phenotype['phenotypes-meta'][$j]['max'];
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
        $file = file_load($phenotype['metadata']);
        $location = drupal_realpath($file->uri);
        $content = tpps_parse_xlsx($location);
        $groups = $phenotype['metadata-groups'];
        $column_vals = $phenotype['metadata-columns'];

        $phenotype_name = $groups['Phenotype Id']['1'];
        $phenotype_attr = $groups['Attribute']['2'];
        $phenotype_desc = $groups['Description']['3'];
        $phenotype_unit = $groups['Units']['4'];

        foreach ($column_vals as $col => $val) {
          if ($val == '5') {
            $phenotype_struct = $col;
          }
          if ($val == '6') {
            $phenotype_min = $col;
          }
          if ($val == '7') {
            $phenotype_max = $col;
          }
        }

        for ($j = 0; $j < count($content) - 1; $j++) {
          $name = strtolower($content[$j][$phenotype_name]);
          $phenotypes_meta[$name] = array();
          $phenotypes_meta[$name]['attr'] = $content[$j][$phenotype_attr];
          $phenotypes_meta[$name]['desc'] = $content[$j][$phenotype_desc];
          $phenotypes_meta[$name]['unit'] = $content[$j][$phenotype_unit];
          if (isset($phenotype_struct) and isset($content[$j][$phenotype_struct]) and $content[$j][$phenotype_struct] != '') {
            $phenotypes_meta[$name]['struct'] = $content[$j][$phenotype_struct];
          }
          if (isset($phenotype_min) and isset($content[$j][$phenotype_min]) and $content[$j][$phenotype_min] != '') {
            $phenotypes_meta[$name]['min'] = $content[$j][$phenotype_min];
          }
          if (isset($phenotype_max) and isset($content[$j][$phenotype_max]) and $content[$j][$phenotype_max] != '') {
            $phenotypes_meta[$name]['max'] = $content[$j][$phenotype_max];
          }
        }

        $file->status = FILE_STATUS_PERMANENT;
        $file = file_save($file);
        $file_rank++;
      }

      $location = drupal_realpath($phenotype_file->uri);
      $content = tpps_parse_xlsx($location);
      $groups = $phenotype['file-groups'];

      $tree_id_col = $groups['Tree Identifier']['1'];
      $phenotype_name = $groups['Phenotype Name/Identifier']['2'];
      $phenotype_val = $groups['Phenotype Value(s)']['3'];

      for ($j = 0; $j < count($content) - 1; $j++) {
        $name = strtolower($content[$j][$phenotype_name]);
        $value = $content[$j][$phenotype_val];
        $stock_unique = $content[$j][$tree_id_col];

        $attr = db_select('chado.cvterm', 'c')
          ->fields('c', array('cvterm_id'))
          ->condition('c.name', db_like($phenotypes_meta[$name]['attr']), 'LIKE')
          ->execute()
          ->fetchAssoc();
        $attr_id = $attr['cvterm_id'];
        if (!isset($attr_id) or $attr_id == '') {
          $attr_id = chado_insert_cvterm(array(
            'id' => "{$local_db->name}:{$phenotypes_meta[$name]['attr']}",
            'name' => $phenotypes_meta[$name]['attr'],
            'definition' => '',
            'cv_name' => $local_cv->name,
          ));
        }

        if (isset($phenotypes_meta[$name]['struct'])) {
          $obs = db_select('chado.cvterm', 'c')
            ->fields('c', array('cvterm_id'))
            ->condition('c.name', db_like($phenotypes_meta[$name]['struct']), 'LIKE')
            ->execute()
            ->fetchAssoc();
          $observable_id = $obs['cvterm_id'];

          if (!isset($observable_id) or $observable_id == '') {
            $observable_id = chado_insert_cvterm(array(
              'id' => "{$local_db->name}:{$phenotypes_meta[$name]['struct']}",
              'name' => $phenotypes_meta[$name]['struct'],
              'definition' => '',
              'cv_name' => $local_cv->name,
            ));
          }
        }
        else {
          $observable_id = NULL;
        }

        $phenotype_ids[] = tpps_chado_insert_record('phenotype', array(
          'uniquename' => "$stock_unique-$name-$j",
          'name' => $name,
          'attr_id' => $attr_id,
          'observable_id' => $observable_id,
          'value' => $value,
        ));

        tpps_chado_insert_record('stock_phenotype', array(
          'stock_id' => $form_state['ids']['stock_ids'][$stock_unique],
          'phenotype_id' => end($phenotype_ids),
        ));

        $phenotypeprop_ids[] = tpps_chado_insert_record('phenotypeprop', array(
          'phenotype_id' => end($phenotype_ids),
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'description',
            'is_obsolete' => 0,
          ),
          'value' => $phenotypes_meta[$name]['desc'],
        ));

        $phenotypeprop_ids[] = tpps_chado_insert_record('phenotypeprop', array(
          'phenotype_id' => end($phenotype_ids),
          'type_id' => array(
            'cv_id' => array(
              'name' => 'uo',
            ),
            'name' => 'unit',
            'is_obsolete' => 0,
          ),
          'value' => $phenotypes_meta[$name]['unit'],
        ));

        if (isset($phenotypes_meta[$name]['min'])) {
          $phenotypeprop_ids[] = tpps_chado_insert_record('phenotypeprop', array(
            'phenotype_id' => end($phenotype_ids),
            'type_id' => array(
              'cv_id' => array(
                'name' => 'default_namespace',
              ),
              'name' => 'minimum parameter',
              'is_obsolete' => 0,
            ),
            'value' => $phenotypes_meta[$name]['min'],
          ));
        }

        if (isset($phenotypes_meta[$name]['max'])) {
          $phenotypeprop_ids[] = tpps_chado_insert_record('phenotypeprop', array(
            'phenotype_id' => end($phenotype_ids),
            'type_id' => array(
              'cv_id' => array(
                'name' => 'default_namespace',
              ),
              'name' => 'maximum parameter',
              'is_obsolete' => 0,
            ),
            'value' => $phenotypes_meta[$name]['max'],
          ));
        }
      }

      $phenotype_file->status = FILE_STATUS_PERMANENT;
      $phenotype_file = file_save($phenotype_file);
    }

    if (isset($fourthpage["organism-$i"]['genotype'])) {
      $genotype = $fourthpage["organism-$i"]['genotype'];
      $genotype_count = 0;

      if ($genotype['ref-genome'] == 'bio') {

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
      elseif ($genotype['ref-genome'] == 'manual' or $genotype['ref-genome'] == 'manual2' or $genotype['ref-genome'] == 'url') {
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
          $file = file_load($assembly_user);
          $file->status = FILE_STATUS_PERMANENT;
          $file = file_save($file);
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
          $file = file_load($assembly_user);
          $file->status = FILE_STATUS_PERMANENT;
          $file = file_save($file);
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

      if ($genotype['file-type']['Genotype Assay']) {
        $genotype_file_id = tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($genotype['file'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;

        $file = file_load($genotype['file']);
        $location = drupal_realpath($file->uri);
        $content = tpps_parse_xlsx($location);
        $groups = $genotype['file-groups'];
        $tree_id_col = $groups['Tree Id']['1'];

        // Process genotype spreadsheet/assay.
        if ($genotype['marker-type']['SNPs']) {
          // If SNPs marker, the file is a Genotype Assay.
          $genotype_cols = $groups['SNP Data']['2'];
        }
        else {
          // If not SNPs marker, the file is a Genotype spreadsheet, not assay.
          $genotype_cols = $groups['Genotype Data']['2'];
          $marker = ($genotype['marker-type']['SSRs/cpSSRs']) ? $genotype['SSRs/cpSSRs'] : $genotype['other-marker'];
        }

        foreach ($content as $key => $row) {
          if ($key != 'headers') {
            $tree_id = $row[$tree_id_col];

            $query = db_select('chado.stock', 's')
              ->fields('s', array('organism_id'))
              ->condition('uniquename', $tree_id)
              ->execute()
              ->fetchAssoc();
            $organism_id = $query['organism_id'];

            if (!isset($last_org_id) or $last_org_id !== $organism_id) {
              // If the last organism id is not present or is different from
              // this one, get a new species code. otherwise, keep the last one
              // to avoid some redundant queries.
              $and = db_and()
                ->condition('type_id', chado_get_cvterm(array('name' => 'organism 4 letter code'))->cvterm_id)
                ->condition('organism_id', "$organism_id");
              $species_code = db_select('chado.organismprop', 'o')
                ->fields('o', array('value'))
                ->condition($and)
                ->execute()
                ->fetchAssoc();
              $species_code = $species_code['value'];
            }

            $genotype_vals = array();
            foreach ($genotype_cols as $marker_id) {
              $genotype_count++;
              $marker_value = $row[$marker_id];

              if ($genotype_count > $genotype_group) {
                $genotype_count = 0;
                tpps_chado_insert_record('genotype', $genotype_vals);
                $genotype_vals = array();
              }

              if ($genotype['marker-type']['SNPs']) {
                $marker_type_id = chado_get_cvterm(array('name' => 'SNP', 'cv_id' => array('name' => 'sequence')))->cvterm_id;
              }
              elseif ($genotype['marker-type']['SSRs/cpSSRs']) {
                if ($genotype['SSRs/cpSSRs'] == 'cpSSR') {
                  $marker_type_id = chado_get_cvterm(array('name' => 'cpSSR'))->cvterm_id;
                }
                else {
                  $marker_type_id = chado_get_cvterm(array('name' => 'SSR'))->cvterm_id;
                }
              }
              else {
                // type_id for other genotype markers should just be 'genotype'.
                $marker_type_id = chado_get_cvterm(array('name' => 'genotype', 'cv_id' => array('name' => 'sequence')))->cvterm_id;
              }

              $genotype_vals[] = array(
                'name' => "$marker_id-$species_code-$marker_value",
                'uniquename' => "$marker_id-$species_code-$marker_value",
                'description' => $marker_value,
                'type_id' => "$marker_type_id",
              );
            }

            // Save the organism id for the next row.
            $last_org_id = $organism_id;
          }
        }
        tpps_chado_insert_record('genotype', $genotype_vals);
        unset($genotype_vals);
        $genotype_count = 0;

        $file->status = FILE_STATUS_PERMANENT;
        $file = file_save($file);
      }

      if ($genotype['file-type']['Assay Design'] and $genotype['marker-type']['SNPs']) {
        $genotype_file_id = tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($genotype['assay-design'])->uri),
          'rank' => $file_rank,
        ));
        $file = file_load($genotype['assay-design']);
        $file->status = FILE_STATUS_PERMANENT;
        $file = file_save($file);
        $file_rank++;
      }

      if ($genotype['file-type']['VCF']) {
        $genotype_vcf_id = tpps_chado_insert_record('projectprop', array(
          'project_id' => $project_id,
          'type_id' => array(
            'cv_id' => array(
              'name' => 'schema',
            ),
            'name' => 'url',
            'is_obsolete' => 0,
          ),
          'value' => file_create_url(file_load($genotype['vcf'])->uri),
          'rank' => $file_rank,
        ));
        $file_rank++;

        $current_id = $form_state['ids']['organism_ids'][$i];
        $and = db_and()
          ->condition('type_id', chado_get_cvterm(array('name' => 'organism 4 letter code'))->cvterm_id)
          ->condition('organism_id', "$current_id");
        $species_code = db_select('chado.organismprop', 'o')
          ->fields('o', array('value'))
          ->condition($and)
          ->execute()
          ->fetchAssoc();
        $species_code = $species_code['value'];
        $marker = 'SNP';

        $vcf_file = file_load($genotype['vcf']);
        $location = drupal_realpath($vcf_file->uri);
        $vcf_content = fopen($location, 'r');

        // Inserts are WAY faster when done as a group.
        $genotype_vals = array();
        $genotypeprop_vals = array();
        $genotype_call_vals = array();
        $feature_ids = array();
        $stocks = array();
        $format = "";
        $first = TRUE;

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

            if (empty($marker_name) or $marker_name == '.') {
              $marker_name = "{$scaffold_id}{$position}$ref:$alt";
            }
            $variant_name = $marker_name . $marker;

            if (!array_key_exists($marker_name, $feature_ids)) {
              $marker_id = tpps_chado_insert_record('feature', array(
                'organism_id' => $current_id,
                'uniquename' => $marker_name,
                'type_id' => array(
                  'cv_id' => array(
                    'name' => 'sequence',
                  ),
                  'name' => 'sequence_variant',
                  'is_obsolete' => 0,
                ),
              ));
              $feature_ids[$marker_name] = $marker_id;
            }
            else {
              $marker_id = $feature_ids[$marker_name];
            }

            if (!array_key_exists($variant_name, $feature_ids)) {
              $variant_id = tpps_chado_insert_record('feature', array(
                'organism_id' => $current_id,
                'uniquename' => $variant_name,
                'type_id' => array(
                  'cv_id' => array(
                    'name' => 'sequence',
                  ),
                  'name' => 'sequence_variant',
                  'is_obsolete' => 0,
                ),
              ));
              $feature_ids[$variant_name] = $variant_id;
            }
            else {
              $variant_id = $feature_ids[$variant_name];
            }

            $description = "$ref:$alt";

            // Tripal Job has issues when all submissions are made at the same
            // time, so break them up into groups of 10,000 genotypes along with
            // their relevant genotypeprops.
            if ($genotype_count > $genotype_group) {
              $genotype_count = 0;
              tpps_chado_insert_record('genotype', $genotype_vals, array('multi' => TRUE));
              $genotype_vals = array();
              tpps_chado_insert_record('genotype_call', $genotype_call_vals, array('multi' => TRUE));
              $genotype_call_vals = array();
              tpps_chado_insert_record('genotypeprop', $genotypeprop_vals, array('multi' => TRUE));
              $genotypeprop_vals = array();
            }

            $genotype_vals[] = array(
              'name' => "$marker-$species_code-$scaffold_id-$position",
              'uniquename' => "$marker-$species_code-$scaffold_id-$position-$description",
              'description' => $description,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'sequence',
                ),
                'name' => 'SNP',
                'is_obsolete' => 0,
              ),
            );

            if ($first) {
              $first = FALSE;
              $genotype_id = tpps_chado_insert_record('genotype', $genotype_vals[0]);
              $genotype_vals = array();
            }

            if ($format != "") {
              $genotypeprop_vals[] = array(
                'genotype_id' => $genotype_id,
                'type_id' => array(
                  'cv_id' => array(
                    'name' => 'default_namespace',
                  ),
                  'name' => 'format',
                  'is_obsolete' => 0,
                ),
                'value' => $format,
              );
            }

            for ($j = 9; $j < count($vcf_line); $j++) {
              $genotype_call_vals[] = array(
                'variant_id' => $variant_id,
                'marker_id' => $marker_id,
                'genotype_id' => $genotype_id,
                'project_id' => $project_id,
                'stock_id' => $stocks[$j - 9],
              );
            }

            // Quality score.
            $genotypeprop_vals[] = array(
              'genotype_id' => $genotype_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'sequence',
                ),
                'name' => 'quality_value',
                'is_obsolete' => 0,
              ),
              'value' => $qual,
            );

            // filter: pass/fail.
            $genotypeprop_vals[] = array(
              'genotype_id' => $genotype_id,
              'type_id' => array(
                'cv_id' => array(
                  'name' => 'operation',
                ),
                'name' => 'Sequence contamination filtering',
                'is_obsolete' => 0,
              ),
              'value' => ($filter == '.') ? "P" : "NP",
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
              $genotypeprop_vals[] = array(
                'genotype_id' => $genotype_id,
                'type_id' => array(
                  'cv_id' => array(
                    'name' => 'sequence',
                  ),
                  'name' => 'allelic_frequency',
                  'is_obsolete' => 0,
                ),
                'value' => $info_vals['AF'],
              );
            }

            // Depth coverage, assuming that the info code for depth coverage is
            // 'DP'.
            if (isset($info_vals['DP']) and $info_vals['DP'] != '') {
              $genotypeprop_vals[] = array(
                'genotype_id' => $genotype_id,
                'type_id' => array(
                  'cv_id' => array(
                    'name' => 'operation',
                  ),
                  'name' => 'Read depth analysis',
                  'is_obsolete' => 0,
                ),
                'value' => $info_vals['DP'],
              );
            }

            // Number of samples, assuming that the info code for number of
            // samples is 'NS'.
            if (isset($info_vals['NS']) and $info_vals['NS'] != '') {
              $genotypeprop_vals[] = array(
                'genotype_id' => $genotype_id,
                'type_id' => array(
                  'name' => 'number_samples',
                  'is_obsolete' => 0,
                ),
                'value' => $info_vals['NS'],
              );
            }
            $genotype_id++;
          }
          elseif (preg_match('/##FORMAT=/', $vcf_line)) {
            $format .= substr($vcf_line, 9, -1);
          }
          elseif (preg_match('/#CHROM/', $vcf_line)) {
            $vcf_line = explode("\t", $vcf_line);
            for ($j = 9; $j < count($vcf_line); $j++) {
              $stock_name = $vcf_line[$j];
              $values = array(
                'organism_id' => $current_id,
                'uniquename' => $form_state['accession'] . '-' . $stock_name,
                'type_id' => array(
                  'cv_id' => array(
                    'name' => 'obi',
                  ),
                  'name' => 'organism',
                  'is_obsolete' => 0,
                ),
              );
              $results = chado_select_record('stock', array('stock_id'), $values);
              $stocks[] = $form_state['ids']['stock_ids'][trim($vcf_line[$j])];
            }
          }
        }
        // Insert the last set of values.
        tpps_chado_insert_record('genotype', $genotype_vals, array('multi' => TRUE));
        tpps_chado_insert_record('genotype_call', $genotype_call_vals, array('multi' => TRUE));
        tpps_chado_insert_record('genotypeprop', $genotypeprop_vals, array('multi' => TRUE));
        unset($genotype_vals);
        unset($genotypeprop_vals);
        // dpm('done: ' . date('r'));.
        $vcf_file->status = FILE_STATUS_PERMANENT;
        $vcf_file = file_save($vcf_file);
      }
    }

    if (isset($fourthpage["organism-$i"]['environment'])) {
      $environment = $fourthpage["organism-$i"]['environment'];
      $env_layers_check = $environment['use_layers'];
      $env_layers = $environment['env_layers'];
      $env_params = $environment['env_params'];
      $env_number = $environment['number'];

      if ($organism_number == 1 or !$form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['check']) {
        $accession_file = file_load($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file']);
        $id_col = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']['file-groups']['Tree Id'][1];
      }
      else {
        $accession_file = file_load($form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['file']);
        $id_col = $form_state['saved_values'][TPPS_PAGE_3]['tree-accession']["species-$i"]['file-groups']['Tree Id'][1];
      }

      $location = drupal_realpath($accession_file->uri);
      $content = tpps_parse_xlsx($location);

      if ($env_layers_check) {
        $layers_params = array();

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
        }

        for ($j = 0; $j < count($content) - 1; $j++) {
          $tree_id = $content[$j][$id_col];
          $stock_query = db_select('chado.stock', 's')
            ->fields('s', array('stock_id'))
            ->condition('uniquename', $tree_id)
            ->execute();

          $stock_id = $stock_query->fetchObject()->stock_id;

          $and = db_and()
            ->condition('stock_id', $stock_id)
            ->condition('type_id', chado_get_cvterm(array('name' => 'gps_latitude'))->cvterm_id);
          $gps_query = db_select('chado.stockprop', 's')
            ->fields('s', array('value'))
            ->condition($and)
            ->execute();

          $lat = $gps_query->fetchObject()->value;

          $and = db_and()
            ->condition('stock_id', $stock_id)
            ->condition('type_id', chado_get_cvterm(array('name' => 'gps_longitude'))->cvterm_id);
          $gps_query = db_select('chado.stockprop', 's')
            ->fields('s', array('value'))
            ->condition($and)
            ->execute();

          $long = $gps_query->fetchObject()->value;

          if (db_table_exists('cartogratree_layers') and db_table_exists('cartogratree_fields')) {
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

                $value = tpps_get_environmental_layer_data($layer_id, $lat, $long, $param_name);
                $type = variable_get("tpps_param_{$param_id}_type");

                if ($type == 'attr_id') {
                  tpps_chado_insert_record('phenotype', array(
                    'uniquename' => "$tree_id-$layer_name-$param_name-$j",
                    'name' => "$param_name",
                    'attr_id' => array(
                      'cv_id' => array(
                        'name' => 'biomaterial_property',
                      ),
                      'name' => 'climate_environment',
                      'is_obsolete' => 0,
                    ),
                    'value' => "$value",
                  ));
                }
                else {
                  $phenotype_id = tpps_chado_insert_record('phenotype', array(
                    'uniquename' => "$tree_id-$layer_name-$param_name-$j",
                    'name' => "$param_name",
                    'value' => "$value",
                  ));

                  tpps_chado_insert_record('phenotype_cvterm', array(
                    'phenotype_id' => $phenotype_id,
                    'cvterm_id' => array(
                      'cv_id' => array(
                        'name' => 'biomaterial_property',
                      ),
                      'name' => 'climate_environment',
                      'is_obsolete' => 0,
                    ),
                  ));
                }
              }
            }
          }
        }
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

      for ($j = 0; $j < count($content) - 1; $j++) {
        $tree_id = $content[$j][$id_col];
        foreach ($env_meta as $current_env) {
          $name = $current_env['name'];
          $desc = $current_env['desc'];
          $unit = $current_env['unit'];
          $val = $current_env['val'];

          $env_id = tpps_chado_insert_record('phenotype', array(
            'uniquename' => "$tree_id-$name-$j",
            'name' => "$name",
            'attr_id' => array(
              'cv_id' => array(
                'name' => 'biomaterial_property',
              ),
              'name' => 'climate_environment',
              'is_obsolete' => 0,
            ),
            'value' => "$val",
          ));

          tpps_chado_insert_record('phenotypeprop', array(
            'phenotype_id' => $env_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'schema',
              ),
              'name' => 'description',
              'is_obsolete' => 0,
            ),
            'value' => $desc,
          ));

          tpps_chado_insert_record('phenotypeprop', array(
            'phenotype_id' => $env_id,
            'type_id' => array(
              'cv_id' => array(
                'name' => 'uo',
              ),
              'name' => 'unit',
              'is_obsolete' => 0,
            ),
            'value' => $unit,
          ));
        }
      }
    }
  }

  tpps_update_submission($form_state);
}
