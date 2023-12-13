<?php

/**
 * @file
 * Defines functions used to display complete and incomplete TPPS submissions.
 */

/**
 * This function returns a flattened and filtered form state.
 *
 * This is useful for any functions that display all of the detailed contents of
 * a TPPS Submission.
 *
 * @param array $state
 *   The state of the submission form.
 *
 * @return array
 *   The flattened state array.
 */
function tpps_flatten(array $state) {
  $data = isset($state['saved_values']) ? $state['saved_values'] : NULL;

  if (!$data) {
    return array();
  }
  unset($data['frontpage']);
  unset($data['form_build_id']);

  $page_1 = &$data[TPPS_PAGE_1];
  $pub = &$page_1['publication'];
  $org = &$page_1['organism'];

  if (isset($page_1['#cleared']) and $page_1['#cleared']) {
    $data['#page_1_cleared'] = TRUE;
  }
  else {
    $data['#page_1_cleared'] = FALSE;
  }
  $data['Project ID'] = $state['ids']['project_id'] ?? NULL;
  $data['#study_photo'] = $page_1['photo'] ?? NULL;
  // @todo [VS] Change machine names of DOI fields.
  $data['Publication DOI'] = $page_1['doi'] ?? NULL;
  $data['Dataset DOI'] = $page_1['dataset_doi'] ?? NULL;

  $data['Publication Title'] = isset($pub['title']) ? $pub['title'] : NULL;
  $data['Primary Author'] = isset($page_1['primaryAuthor']) ? $page_1['primaryAuthor'] : NULL;
  $data['Publication Status'] = isset($pub['status']) ? $pub['status'] : NULL;
  $data['Publication Year'] = isset($pub['year']) ? $pub['year'] : NULL;
  $data['Publication Abstract'] = isset($pub['abstract']) ? $pub['abstract'] : NULL;
  $data['Publication Journal'] = isset($pub['journal']) ? $pub['journal'] : NULL;

  if (isset($pub['secondaryAuthors']['number'])) {
    $data['#number_secondary_authors'] = $pub['secondaryAuthors']['number'];
    for ($i = 1; $i <= $data['#number_secondary_authors']; $i++) {
      if (isset($pub['secondaryAuthors']["$i"]) and $pub['secondaryAuthors']["$i"] != '') {
        $data["Secondary Author $i"] = $pub['secondaryAuthors']["$i"];
      }
    }
  }

  if (isset($org['number'])) {
    $data['#number_organisms'] = $org['number'];
    for ($i = 1; $i <= $data['#number_organisms']; $i++) {
      if (isset($org["$i"]['name']) and $org["$i"]['name'] != '') {
        $data["Species $i"] = $org["$i"]['name'];
      }
    }
  }
  unset($data[TPPS_PAGE_1]);

  $page_2 = &$data[TPPS_PAGE_2];

  if (isset($page_2['#cleared']) and $page_2['#cleared']) {
    $data['#page_2_cleared'] = TRUE;
  }
  else {
    $data['#page_2_cleared'] = FALSE;
  }
  $data['Experiment Starting Month'] = !empty($page_2['StartingDate']['month']) ? $page_2['StartingDate']['month'] : NULL;
  $data['Experiment Starting Year'] = !empty($page_2['StartingDate']['year']) ? $page_2['StartingDate']['year'] : NULL;
  $data['Experiment Ending Month'] = !empty($page_2['EndingDate']['month']) ? $page_2['EndingDate']['month'] : NULL;
  $data['Experiment Ending Year'] = !empty($page_2['EndingDate']['year']) ? $page_2['EndingDate']['year'] : NULL;
  if (!empty($page_2['study_location'])) {
    // Legacy support for old TPPS submissions.
    $legacy_loc = &$page_2['study_location'];
    $data['#location_type'] = (isset($legacy_loc['type']) and $legacy_loc['type']) ? $legacy_loc['type'] : NULL;

    if ($data['#location_type'] == '2' and isset($legacy_loc['custom'])) {
      $data['Study Location'] = $legacy_loc['custom'];
    }
    elseif ($data['#location_type'] and isset($legacy_loc['coordinates'])) {
      $data['Study Location'] = $legacy_loc['coordinates'];
    }
  }

  if (isset($page_2['data_type']) and $page_2['data_type']) {
    $data['Data Type'] = $page_2['data_type'];
  }

  if (isset($page_2['study_type']) and $page_2['study_type']) {
    $study_type_options = array(
      1 => 'Natural Population (Landscape)',
      2 => 'Growth Chamber',
      3 => 'Greenhouse',
      4 => 'Experimental/Common Garden',
      5 => 'Plantation',
    );
    if (isset($study_type_options[$page_2['study_type']])) {
      $data['Study Type'] = $study_type_options[$page_2['study_type']];
    }

    if (!empty($page_2['study_info']['season'])) {
      $data['Seasons Assessed'] = implode($page_2['study_info']['season']);
    }

    if (!empty($page_2['study_info']['assessions'])) {
      $data['Average number of assessions'] = $page_2['study_info']['assessions'];
    }

    $data['Average High Temperature'] = isset($page_2['study_info']['temp']['high']) ? $page_2['study_info']['temp']['high'] : NULL;
    $data['Average Low Temperature'] = isset($page_2['study_info']['temp']['low']) ? $page_2['study_info']['temp']['low'] : NULL;

    $types = array(
      'co2' => 'CO2',
      'humidity' => 'Air Humidity',
      'light' => 'Light Intensity',
      'salinity' => 'Salinity',
    );

    foreach ($types as $type => $label) {
      if (!empty($page_2['study_info'][$type])) {
        $set = $page_2['study_info'][$type];
        if (!empty($set['option'])) {
          $data["$label Control"] = ($set['option'] == '1') ? 'Controlled' : 'Uncontrolled';
          if ($set['option'] == '1' and !empty($set['controlled'])) {
            $data["$label Value"] = $set['controlled'];
          }
          elseif (!empty($set['uncontrolled'])) {
            $data["$label Value"] = $set['uncontrolled'];
          }
        }
      }
    }

    if (!empty($page_2['study_info']['rooting']['option'])) {
      $data['Rooting Type'] = $page_2['study_info']['rooting']['option'];

      if ($data['Rooting Type'] == 'Soil' and !empty($page_2['study_info']['rooting']['soil']['type'])) {
        $soil = &$page_2['study_info']['rooting']['soil'];
        $data['Soil Type'] = $soil['type'];

        if ($data['Soil Type'] == 'Other') {
          $data['Custom Soil Type'] = $soil['other'];
        }

        $data['Soil Container Type'] = isset($soil['container']) ? $soil['container'] : NULL;
      }
    }

    if (!empty($page_2['study_info']['rooting']['ph']['option'])) {
      $set = $page_2['study_info']['rooting']['ph'];
      $data['pH Control'] = ($set['option'] == '1') ? 'Controlled' : 'Uncontrolled';

      if ($set['option'] == '1' and !empty($set['controlled'])) {
        $data["pH Value"] = $set['controlled'];
      }
      elseif (!empty($set['uncontrolled'])) {
        $data["pH Value"] = $set['uncontrolled'];
      }
    }

    if (!empty($page_2['study_info']['rooting']['treatment'])) {
      $treatment = $page_2['study_info']['rooting']['treatment'];
      $selected = FALSE;
      $description = FALSE;

      foreach ($treatment as $field => $value) {
        if (!$description) {
          $description = TRUE;
          $selected = ($value) ? $field : FALSE;
          continue;
        }
        elseif ($selected and !empty($value)) {
          $data["Treatment: $selected description"] = $value;
        }
        $description = FALSE;
      }
    }

    if (!empty($page_2['study_info']['irrigation']) and !empty($page_2['study_info']['irrigation']['option'])) {
      $data['Irrigation Type'] = $page_2['study_info']['irrigation']['option'];
      if ($data['Irrigation Type'] == 'Other' and !empty($page_2['study_info']['irrigation']['other'])) {
        $data['Custom Irrigation Type'] = $page_2['study_info']['irrigation']['other'];
      }
    }

    if (!empty($page_2['study_info']['biotic_env']['option'])) {
      $biotic_envs = array();
      foreach ($page_2['study_info']['biotic_env']['option'] as $key => $val) {
        if ($val) {
          $biotic_envs[] = $key;
        }
      }

      if (!empty($biotic_envs)) {
        $data['Biotic Environment'] = implode(', ', $biotic_envs);
        if (in_array('Other', $biotic_envs) and !empty($page_2['study_info']['biotic_env']['other'])) {
          $data['Custom Biotic Environment'] = $page_2['study_info']['biotic_env']['other'];
        }
      }
    }

    if (!empty($page_2['study_info']['treatment']['check'])) {
      $treatment = &$page_2['study_info']['treatment'];
      $selected = FALSE;
      $description = FALSE;

      foreach ($treatment as $field => $value) {
        if ($field != 'check') {
          if (!$description) {
            $description = TRUE;
            $selected = ($value) ? $field : FALSE;
            continue;
          }
          elseif ($selected and !empty($value)) {
            $data["Treatment: $selected description"] = $value;
          }
          $description = FALSE;
        }
      }
    }
  }
  unset($data[TPPS_PAGE_2]);

  $page_3 = &$data[TPPS_PAGE_3];

  if (isset($page_3['#cleared']) and $page_3['#cleared']) {
    $data['#page_3_cleared'] = TRUE;
  }
  else {
    $data['#page_3_cleared'] = FALSE;
  }

  $data['#skip_loc'] = FALSE;
  $loc = &$page_3['study_location'];
  $data['#location_type'] = (isset($loc['type']) and $loc['type']) ? $loc['type'] : NULL;

  if ($data['#location_type'] == '2' and isset($loc['custom'])) {
    $data['Study Location'] = $loc['custom'];
  }
  elseif ($data['#location_type'] and isset($loc['coordinates'])) {
    $data['Study Location'] = $loc['coordinates'];
  }
  $page_3 = &$data[TPPS_PAGE_3]['tree-accession'];

  if (!empty($data['#number_organisms'])) {
    for ($i = 1; $i <= $data['#number_organisms']; $i++) {
      if (!empty($page_3["species-$i"]['file'])) {
        $organism_name = $data["Species $i"] . ' Accession File';
        if ($data['#number_organisms'] > 1 and empty($page_3['check'])) {
          $organism_name = 'Plant Accession File';
        }

        $data[$organism_name] = $page_3["species-$i"]['file'];
        $loc_type = $page_3["species-$i"]['file-groups']['Location (latitude/longitude or country/state or population group)']['#type'] ?? NULL;
        if (!empty($loc_type) and ($loc_type == "gps" or (array_search('gps', $loc_type) !== FALSE))) {
          $fid = $page_3["species-$i"]['file'];
          drupal_add_js(array(
            'tpps' => array(
              'accession_files' => array(
                $fid => array(
                  'no_header' => $page_3["species-$i"]['file-no-header'] ?? '0',
                  'id_col' => $page_3["species-$i"]['file-groups']['Tree Id'][1],
                  'lat_col' => $page_3["species-$i"]['file-groups']['Location (latitude/longitude or country/state or population group)'][4],
                  'long_col' => $page_3["species-$i"]['file-groups']['Location (latitude/longitude or country/state or population group)'][5],
                ),
              ),
            ),
          ), 'setting');
        }
      }

      if (empty($page_3['check'])) {
        break;
      }
    }
  }

  $data["Total number of plants"] = $state['stats']['tree_count'] ?? NULL;
  unset($data[TPPS_PAGE_3]);

  $page_4 = &$data[TPPS_PAGE_4];

  if (isset($page_4['#cleared']) and $page_4['#cleared']) {
    $data['#page_4_cleared'] = TRUE;
  }
  else {
    $data['#page_4_cleared'] = FALSE;
  }
  if (!empty($data['#number_organisms'])) {
    for ($i = 1; $i <= $data['#number_organisms']; $i++) {
      if (isset($page_4["organism-$i"])) {
        $org = &$page_4["organism-$i"];
        $org_name = isset($data["Species $i"]) ? $data["Species $i"] : NULL;

        if (isset($org['phenotype'])) {
          $pheno = &$org['phenotype'];

          if (!empty($pheno['iso-check'])) {
            $data["$org_name Phenotype Isotope/Mass Spectrometry File"] = $pheno['iso'];
          }
          else {
            if (isset($pheno['phenotypes-meta']['number'])) {
              $data['#number_phenotypes'] = $pheno['phenotypes-meta']['number'];
              $pheno_meta = &$pheno['phenotypes-meta'];
              for ($j = 1; $j <= $data['#number_phenotypes']; $j++) {
                $fields = array(
                  'name' => 'Name',
                  'attribute' => 'Attribute',
                  'description' => 'Description',
                  'unit' => 'Unit',
                );
                if (isset($pheno_meta["$j"]['struct-check']) and $pheno_meta["$j"]['struct-check']) {
                  $fields['structure'] = 'Structure';
                }
                // [VS] #8669rmrw5
                //if (isset($pheno_meta["$j"]['val-check']) and $pheno_meta["$j"]['val-check']) {
                //  $fields['min'] = 'Minimum Value';
                //  $fields['max'] = 'Maximum Value';
                //}
                // [/VS] #8669rmrw5

                foreach ($fields as $field => $field_string) {
                  if (isset($pheno_meta["$j"][$field]) and $pheno_meta["$j"][$field] != '') {
                    if ($field == 'unit') {
                      $value = $pheno_meta[$j][$field];
                      $value = tpps_unit_get_name($value);
                    }
                    else {
                      $value = $pheno_meta[$j][$field];
                    }
                    $data["$org_name Phenotype $j $field_string"] = $value;
                  }
                }
              }
            }

            if (isset($pheno['check']) and $pheno['check'] and isset($pheno['metadata']) and $pheno['metadata']) {
              $data["$org_name Phenotype Metadata File"] = $pheno['metadata'];
            }

            if (isset($pheno['file']) and $pheno['file']) {
              $data["$org_name Phenotype File"] = $pheno['file'];
            }
            $data["$org_name Phenotypes Assessed"] = isset($pheno['phenotype_count']) ? $pheno['phenotype_count'] : NULL;
          }

          // Rish comments: 21 March 2023
          // Based on the example of TGDR465, there is a single ISO file that can be uploaded
          // I *think* the headers should match each phenotype name that is added manually
          // from the previous part of page 4
          if (isset($pheno['file']) and $pheno['file']) {
            $data["$org_name Phenotype File"] = $pheno['file'];
          }
        }

        if (isset($org['genotype'])) {
          $geno = &$org['genotype'];

          $marker_types = array();
          foreach ($geno['marker-type'] as $key => $val) {
            if ($val) {
              $marker_types[] = $key;
            }
          }
          $data["$org_name Genotype Marker Types"] = isset($marker_types) ? implode(', ', $marker_types) : NULL;

          if (isset($geno['marker-type']['SNPs']) and $geno['marker-type']['SNPs']) {
            $snp = $geno['SNPs'];
            if (isset($snp['genotyping-design']) and $snp['genotyping-design']) {
              $geno_design_options = array(
                1 => 'GBS',
                2 => 'Targeted Capture',
                3 => 'Whole Genome Resequencing',
                4 => 'RNA-Seq',
                5 => 'Genotyping Array',
              );
              if (isset($geno_design_options[$snp['genotyping-design']])) {
                $data["$org_name Genotyping Design"] = $geno_design_options[$snp['genotyping-design']];
              }

              if (isset($data["$org_name Genotyping Design"]) and $data["$org_name Genotyping Design"] == 'GBS' and isset($snp['GBS']) and $snp['GBS']) {
                $gbs_options = array(
                  1 => 'RADSeq',
                  2 => 'ddRAD-Seq',
                  3 => 'NextRAD',
                  4 => 'RAPTURE',
                  5 => 'Other',
                );
                if (isset($gbs_options[$snp['GBS']])) {
                  $data["$org_name GBS Type"] = $gbs_options[$snp['GBS']];
                }

                if (isset($data["$org_name GBS Type"]) and $data["$org_name GBS Type"] == 'Other' and isset($snp['GBS-other']) and $snp['GBS-other'] != '') {
                  $data["$org_name Custom GBS Type"] = $snp['GBS-other'];
                }
              }

              if (isset($data["$org_name Genotyping Design"]) and $data["$org_name Genotyping Design"] == 'Targeted Capture' and isset($snp['targeted-capture']) and $snp['targeted-capture']) {
                $targeted_capture_options = array(
                  1 => 'Exome Capture',
                  2 => 'Other',
                );
                if (isset($targeted_capture_options[$snp['targeted-capture']])) {
                  $data["$org_name Targeted Capture Type"] = $targeted_capture_options[$snp['targeted-capture']];
                }

                if (isset($data["$org_name Targeted Capture Type"]) and $data["$org_name Targeted Capture Type"] == 'Other' and isset($snp['targeted-capture-other']) and $snp['targeted-capture-other'] != '') {
                  $data["$org_name Custom Targeted Capture Type"] = $snp['targeted-capture-other'];
                }
              }
            }
          }
          if (isset($geno['marker-type']['SSRs/cpSSRs']) and $geno['marker-type']['SSRs/cpSSRs'] and isset($geno['SSRs/cpSSRs']) and $geno['SSRs/cpSSRs'] != '') {
            $data["$org_name SSRs/cpSSRs Type"] = $geno['SSRs/cpSSRs'];
          }
          if (isset($geno['marker-type']['Other']) and $geno['marker-type']['Other'] and isset($geno['other-marker']) and $geno['other-marker'] != '') {
            $data["$org_name Other Marker Type"] = $geno['other-marker'];
          }

          if (isset($geno['ref-genome']) and $geno['ref-genome'] === 'bio' and isset($geno['BioProject-id']) and $geno['BioProject-id'] != '') {
            $data["$org_name BioProject Id"] = $geno['BioProject-id'];

            if (isset($geno['assembly-auto']) and gettype($geno['assembly-auto']) === 'array') {
              $count = 1;
              foreach ($geno['assembly-auto'] as $id => $check) {
                if (isset($check) and $check) {
                  $data["$org_name Linked Accession $count"] = $id;
                  $count++;
                }
              }
            }
          }
          elseif (isset($geno['ref-genome']) and ($geno['ref-genome'] === 'manual' or $geno['ref-genome'] === 'manual2') and isset($geno['tripal_fasta']['file']['file_upload']) and isset($geno['tripal_fasta']['file']['file_upload_existing']) and ($geno['tripal_fasta']['file']['file_upload'] or $geno['tripal_fasta']['file']['file_upload_existing'])) {
            if ($geno['tripal_fasta']['file']['file_upload']) {
              $data["$org_name Uploaded Reference File"] = $geno['tripal_fasta']['file']['file_upload'];
            }
            if ($geno['tripal_fasta']['file']['file_upload_existing']) {
              $data["$org_name Reference File"] = $geno['tripal_fasta']['file']['file_upload_existing'];
            }
          }
          elseif (isset($geno['ref-genome']) and $geno['ref-genome'] === 'url' and isset($geno['tripal_fasta']['file']['file_remote']) and $geno['tripal_fasta']['file']['file_remote']) {
            $data["$org_name Website of reference file"] = $geno['tripal_fasta']['file']['file_remote'];
          }
          elseif (isset($geno['ref-genome']) and $geno['ref-genome']) {
            $data["$org_name Reference Genome"] = $geno['ref-genome'];
          }

          if (!empty($geno['files']['snps-assay'])) {
            $data["$org_name SNP Genotype Assay File"] = $geno['files']['snps-assay'];
          }

          if (!empty($geno['files']['assay-design'])) {
            $data["$org_name Assay Design File"] = $geno['files']['assay-design'];
          }

          if (!empty($geno['files']['ssrs'])) {
            $data["$org_name SSRs Genotype File"] = $geno['files']['ssrs'];
          }

          if (!empty($geno['files']['ssrs_extra'])) {
            $data["$org_name cpSSRs Additional Genotype File"] = $geno['files']['ssrs_extra'];
          }

          //if (!empty($geno['files']['indels'])) {
          //  $data["$org_name Indel Genotype File"] = $geno['files']['indels'];
          //}

          if (!empty($geno['files']['other'])) {
            $data["$org_name Other Marker Genotype File"] = $geno['files']['other'];
          }

          if (!empty($geno['files']['local_vcf'])) {
            $data["$org_name VCF File (cluster)"] = $geno['files']['local_vcf'];
          }

          if ($geno['files']['vcf'] > 0) {
            $file = file_load($geno['files']['vcf']);
            $file_url = file_create_url($file->uri);
            $data["$org_name VCF File (uploaded)"] = "<a href='$file_url' target='blank'>$file->filename</a>";
          }
        }

        if (isset($org['environment'])) {
          $env = &$org['environment'];

          if (variable_get('tpps_cartogratree_env', FALSE)) {
            $data["CartograPlant Layers"] = array();
            foreach ($env['env_layers'] as $layer => $layer_id) {
              if (!empty($layer_id)) {
                $data["CartograPlant Layers"][] = $layer;
              }
              if (!empty($layer_id) and !empty($env['env_params'][$layer])) {
                $params = array();
                foreach ($env['env_params'][$layer] as $param => $param_id) {
                  if (!empty($param_id)) {
                    $params[] = $param;
                  }
                }
                $data["CartograPlant Layer: $layer Parameters"] = implode(', ', $params);
              }
            }
            if (!empty($data["CartograPlant Layers"])) {
              $data["CartograPlant Layers"] = implode(', ', $data["CartograPlant Layers"]);
            }
            else {
              unset($data["CartograPlant Layers"]);
            }
          }
        }
      }
    }
  }
  unset($data[TPPS_PAGE_4]);

  $sum_page = &$data['summarypage'];
  if (!empty($sum_page['comments'])) {
    $data["Additional Comments"] = $sum_page['comments'];
  }
  for ($i = 1; $i <= 10; $i++) {
    if (!empty($sum_page['files'][$i])) {
      $data["Supplemental File $i"] = $sum_page['files'][$i];
    }
  }
  if (!empty($sum_page['tree_pictures'])) {
    foreach ($sum_page['tree_pictures'] as $name => $fid) {
      if (substr($name, -4) == '_url' or substr($name, -12) == '_attribution' or substr($name, -8) == '_license') {
        continue;
      }
      if (!$fid) {
        $data["$name species photo"] = t('None provided');
      }
      else {
        $data["$name species photo"] = $fid;
      }
    }
  }
  unset($data['summarypage']);

  $data['#dbxref_id'] = isset($state['dbxref_id']) ? $state['dbxref_id'] : NULL;
  $data['#accession'] = isset($state['accession']) ? $state['accession'] : NULL;
  $data['#status'] = isset($state['status']) ? $state['status'] : NULL;

  foreach ($data as $key => $val) {
    if (!isset($val)) {
      unset($data[$key]);
    }
  }

  return $data;
}

/**
 * Returns a formatted HTML table based on a TPPS Submission form state.
 *
 * @param array $state
 *   The form_state of the TPPS Submission form.
 *
 * @return string
 *   The TPPS Submission formatted in an HTML table.
 */
function tpps_table_display(array &$state, array $options = array()) {

  $flat = tpps_flatten($state);
  $map_api_key = variable_get('tpps_maps_api_key', NULL);

  $rows = array();
  $content = "";
  foreach ($flat as $key => $val) {
    if ($key[0] != '#') {
      // [VS] Added extra space which must not be visible at site but
      // will divide key and it's value in CLI.
      $key_str = filter_xss(check_plain($key)) . ' ';
      $val_str = filter_xss(check_plain($val));

      if (preg_match('/File/', $key) and ($file = file_load($val) === FALSE) and stripos($key_str, 'VCF File (cluster)') === FALSE) {
        // FILE NOT FOUND
        $val_str .= ' <span style="font-size: 32px;">🆘</span><span>(File missing)</span>';
      }
      if (preg_match('/File/', $key) and ($file = file_load($val))) {

        // Create links to files.
        $file_name = $file->filename;

        $file_url = check_plain(file_create_url($file->uri));
        $val_str = "<a href='$file_url' target='blank'>$file_name</a>";
        if (!empty($state['revised_files'][$val]) and ($revised_file = file_load($state['revised_files'][$val]))) {
          $val = $state['revised_files'][$val];
          $file_url = check_plain(file_create_url($revised_file->uri));
          $file_name = $revised_file->filename;
          $val_str = "<a href='$file_url' target='blank'>$file_name</a> ( Revised from $val_str )";
        }

        // @TODO [VS] Use Drupal Form API instead of HTML in PHP.
        $val_str .= "<br><input id=\"fid_$val\" class=\"preview_button btn form-button form-submit\" type=\"button\" value=\"Preview file\"></input>";
        if (tpps_access('administer tpps module')) {
          $val_str .= "<br><input id=\"fid_$val\" class=\"preview_full_button btn form-button form-submit\" type=\"button\" value=\"Show full file\"></input>";
        }
        if (preg_match('/^(.*)Accession File/', $key, $matches)) {
          if (!$flat['#skip_loc']) {
            $fid = $file->fid;
            $wrapper_id = "{$fid}_map_wrapper";
            $button_id = "{$fid}_map_button";
            $val_str .= "<div id=\"$wrapper_id\"></div>"
              . "<input id=\"$button_id\" type=\"button\" class=\"btn form-button form-submit\" "
              . "value=\"Click here to view plants on map!\"></input>";

            drupal_add_js(array(
              'tpps' => array(
                'map_buttons' => array(
                  $fid => array(
                    'wrapper' => $wrapper_id,
                    'button' => $button_id,
                    'fid' => $fid,
                  ),
                ),
              ),
            ), 'setting');
          }
          else {
            $val_str .= "<br>This submission was submitted without validating location information. As a result, we did not import location information for these plants.";
          }
        }

        if (!empty($options['files']['revision_destination'])) {
          $val_str .= "<div id=\"file_{$file->fid}_options_dest\"></div>";
        }
      }
      elseif (preg_match('/species photo/', $key) and (file_load($val)) and ($file_name = file_load($val)->filename)) {
        $file_url = check_plain(file_create_url(file_load($val)->uri));
        $val_str = "<img src='$file_url' width='100%'>";
        $val_str .= "<br><a href='$file_url' target='blank'>$file_name</a>";
      }
      elseif (preg_match('/Month/', $key)) {
        // Skip experiment start/end month.
        continue;
      }
      elseif (preg_match('/^(.*)Year/', $key, $matches)) {
        // Combine experiment start/end month + year.
        if (isset($flat[$matches[1] . 'Month'])) {
          $key_str = $matches[1] . 'Date';
          $val_str = "{$flat[$matches[1] . 'Month']} $val_str";
        }
        else {
          continue;
        }
      }
      elseif (preg_match('/Study Location/', $key) and isset($flat['#location_type']) and $flat['#location_type']) {
        // Add map capability to location.
        if ($flat['#location_type'] == '2') {
          $query = $val_str;
        }
        elseif ($flat['#location_type'] != '2' and ($standard = tpps_standard_coord($val_str))) {
          $query = $standard;
        }
        $val_str = "<a id=\"sidebar_map\" style=\"cursor:pointer\">$val_str</a>";
        if (isset($query)) {
          $map = "<iframe frameborder=\\\"0\\\" style=\\\"border:0;width:100%;height:100%\\\" src=\\\"https://www.google.com/maps?q=$query&output=embed&key=AIzaSyDkeQ6KN6HEBxrIoiSCrCHFhIbipycqouY&z=5\\\" allowfullscreen> </iframe></div>";
          $js = "jQuery(document).ready(function ($) {
            jQuery(\"#sidebar_map\").click(function() {
              jQuery(\"#sidebar_map_wrapper\").remove();
              jQuery(\"#sidebar_map\").after( \"<br><div id=\\\"sidebar_map_wrapper\\\">$map</div>\" );
            });
          });";
          drupal_add_js($js, 'inline');
        }
      }
      elseif ($key == 'Publication Abstract' and strlen($val_str) > 50) {
        $val_str = substr($val_str, 0, 47) . '...';
      }
      elseif (in_array($key, ['Publication DOI', 'Dataset DOI'])) {
        $link = preg_match('/https?:\/\//', $val_str) ? $val_str : "https://doi.org/$val_str";
        $val_str = "<a href=\"$link\" target=\"blank\">$val_str</a>";
      }
      elseif (!empty($options['skip_phenotypes']) and preg_match('/Phenotype/', $key)) {
        continue;
      }

      $rows[] = array($key_str, $val_str);
    }
    elseif (preg_match('/^#(.*)accession_(.*)/', $key, $matches)) {
      $val_str = filter_xss(check_plain($val));
      $parts = explode(" ", substr($matches[0], 1));
      $item_id = implode("_", $parts);
      $content .= "<div id=\"$item_id\" style=\"display:none;\">$val_str</div>";
    }
  }
  if (!empty($map_api_key)) {
    $content .= "<script src=\"https://developers.google.com/maps/documentation/javascript/examples/markerclusterer/markerclusterer.js\"></script><script src=\"https://maps.googleapis.com/maps/api/js?key=$map_api_key&callback=initMap\"
    async defer></script>";
  }

  $headers = array(
    'Field',
    'Value',
  );

  $vars = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_table_display',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );

  $photo = "";
  if (!empty($flat['#study_photo'])) {
    $url = check_plain(file_create_url(file_load($flat['#study_photo'])->uri));
    $photo = "<img src=\"$url\" style=\"display: block; margin-left: auto; margin-right: auto;\"></img>";
  }

  $display = $photo . theme('table', $vars) . $content;

  return $display;
}

/**
 * Creates a details page for a completed TPPS Submission.
 *
 * @global string $base_url
 *   The base url of the site.
 *
 * @return string
 *   The rendered page.
 */
function tpps_completed_display($accession = NULL) {

  global $base_url;
  global $user;
  $display = "";
  $link = "$base_url/tpps/details/$accession";

  $old_form_state = tpps_load_submission($accession);
  $values = $old_form_state['saved_values'];

  // @TODO Move CSS to /css/tpps.css. Check if scope must be 'header'.
  $css = "
  .tgdr_complete_submission {
    background: #ffffff;
    border: 1px solid #cccccc;
    border-radius: 4px;
    padding: 0px 10px;
  }
  ";
  drupal_add_css($css, 'inline');

  $display .= "<label for='tpps_$accession'>{$values[TPPS_PAGE_1]['publication']['title']}</label><p>You can view the TPPS Details page with completed and approved TPPS and TGDR data <a href=\"$link\">here</a></p>";
  if ($values[TPPS_PAGE_1]['publication']['status'] != 'Published' and $user->uid == tpps_load_submission($accession, FALSE)->uid) {
    $display .= "<a href=\"$base_url/tpps/$accession/edit-publication\">Edit publication information</a>";
  }

  $display .= tpps_table_display($old_form_state);
  tpps_add_css_js('main');
  return render($display);
}

/**
 * This function populates the TPPS Submissions user tab.
 *
 * @param int $uid
 *   The id of the user.
 *
 * @global string $base_url
 *   The base URL of the site.
 *
 * @return string
 *   The HTML of the TPPS user tab.
 */
function tpps_tab_create($uid) {

  // @TODO [VS] Minor. Replace $user with $account. $user is a current user.
  $user = user_load($uid);
  if (empty($user)) {
    drupal_goto('user');
  }

  global $base_url;
  $params = drupal_get_query_parameters();

  if (isset($params['accession']) && ($params['action'] ?? NULL) === 'DELETE') {
    tpps_delete_submission($params['accession']);
    drupal_goto("user/{$uid}/tpps");
  }

  $states = tpps_load_submission_multiple([
    'status' => 'Incomplete',
    'uid' => $user->uid,
  ]);

  $rows = array();
  foreach ($states as $state) {
    $prefix = !empty($state['tpps_type']) ? $state['tpps_type'] : 'tpps';
    $page_1 = &$state['saved_values'][TPPS_PAGE_1];
    $title = $abstract = $accession_cell = NULL;

    if (isset($page_1['publication']['title'])) {
      $title = "<a href='$base_url/$prefix/{$state['accession']}' style='text-decoration: underline;'>" . check_plain($page_1['publication']['title']) . "</a>";
    }
    else {
      $title = "<a href='$base_url/$prefix/{$state['accession']}' style='text-decoration: underline;'>No Title</a>";
    }

    if (isset($page_1['publication']['abstract'])) {
      $abstract = check_plain($page_1['publication']['abstract']);
      if (strlen($abstract) > 53) {
        $abstract = substr($abstract, 0, 50) . "...";
      }
    }

    if (isset($state['accession'])) {
      $accession_cell = "<input type='submit' id='delete-{$state['accession']}' onclick='";
      $name = isset($page_1['publication']['title']) ? ' titled ' . $page_1['publication']['title'] : '';
      $name = str_replace("'", "", $name); // remove single quotes
      $name = str_replace('"', "", $name); // remove double quotes
      // @TODO It's not safe to use only JS confirmation.
      $accession_cell .= "if (confirm(\"Attention! \\n\\nAre you sure you want to delete your submission{$name}?\\nThis action cannot be undone, and all of your data will be lost!\")){window.location.href = \"$base_url/tpps-submission/{$state['accession']}/delete\"}";
      $accession_cell .= "' name='op' class='form-button' value='Delete'>";
    }

    $rows[] = array(
      $title,
      isset($page_1['primaryAuthor']) ? $page_1['primaryAuthor'] : "",
      isset($page_1['organism']['number']) ? $page_1['organism']['number'] : "",
      isset($abstract) ? $abstract : "",
      isset($page_1['publication']['journal']) ? $page_1['publication']['journal'] : "",
      isset($accession_cell) ? $accession_cell : "",
    );
  }

  $headers = array(
    'Title',
    'Primary Author',
    'Number of Species',
    'Abstract',
    'Journal',
  );

  $vars = array(
    'header' => $headers,
    'rows' => $rows,
    'attributes' => array(
      'class' => array('view'),
      'id' => 'tpps_incomplete_table',
    ),
    'caption' => '',
    'colgroups' => NULL,
    'sticky' => FALSE,
    'empty' => '',
  );

  $final = isset($state) ? "<div class='tpps_table'><label for='tpps_incomplete_table'>Incomplete Submissions</label>" . theme('table', $vars) . '</div>' : "";
  $state = NULL;

  $states = tpps_load_submission_multiple(array(
    'status' => array('Pending Approval', 'Approved'),
    'uid' => $user->uid,
  ));

  $rows = array();
  foreach ($states as $state) {
    $page_1 = &$state['saved_values'][TPPS_PAGE_1];
    $title = $abstract = NULL;

    if (isset($page_1['publication']['title'])) {
      $title = "<a href='$base_url/completed-submission/{$state['accession']}' style='text-decoration: underline;'>" . check_plain($page_1['publication']['title']) . "</a>";
    }
    else {
      $title = "<a href='$base_url/completed-submission/{$state['accession']}' style='text-decoration: underline;'>No Title</a>";
    }

    if (isset($page_1['publication']['abstract'])) {
      $abstract = check_plain($page_1['publication']['abstract']);
      if (strlen($abstract) > 103) {
        $abstract = substr($abstract, 0, 100) . "...";
      }
    }

    $rows[] = array(
      isset($title) ? $title : "",
      isset($page_1['primaryAuthor']) ? $page_1['primaryAuthor'] : "",
      isset($page_1['organism']['number']) ? $page_1['organism']['number'] : "",
      isset($abstract) ? $abstract : "",
      isset($page_1['publication']['journal']) ? $page_1['publication']['journal'] : "",
      isset($state['status']) ? $state['status'] : "",
    );
  }

  $headers[] = 'Status';
  $vars['header'] = $headers;
  $vars['rows'] = $rows;
  $vars['attributes']['id'] = 'tpps_complete_table';

  $final .= isset($state) ? "<div class='tpps_table'><label for='tpps_complete_table'>Complete Submissions</label>" . theme('table', $vars) . '</div>' : "";

  if (empty($final)) {
    $final .= "<h2>Could not find any Submissions</h2>";
    $final .= "<p>We could not find any incomplete TPPS Submissions or TPPS Submissions pending approval. If you would like to start a new TPPS Submission, you can start <a href=$base_url/tpps>here.</a></p>";
  }

  tpps_add_css_js('main');
  return $final;
}

/**
 * Creates and prints an HTML table of the first 3 lines of a file.
 *
 * This function techinically does not return a value, but will print() the
 * rendered table of preview values to the page.
 */
function tpps_preview_file() {
  $params = drupal_get_query_parameters($_POST);
  $no_header = FALSE;
  $fid = $params['fid'] ?? NULL;
  if (empty($fid)) {
    return;
  }
  $rows = $params['rows'] ?? 3;
  $file = file_load($fid);
  $location = tpps_get_location($file->uri);
  $extension = tpps_get_path_extension($location);

  if ($extension == 'vcf') {
    $display = "";
    $count = 0;
    $vcf_content = gzopen($location, 'r');
    while (($vcf_line = gzgets($vcf_content)) !== FALSE) {
      $display .= $vcf_line . "<br>";
      $count++;
      if ($count > $rows) {
        break;
      }
    }
    print(render($display));
    return;
  }

  if ((file_load($fid))) {
    $content = tpps_parse_file($fid, $rows, !empty($no_header));
    $rows = array();
    for ($i = 0; $i < count($content) - 1; $i++) {
      $rows[] = $content[$i];
    }
    $vars = array(
      'header' => $content['headers'],
      'rows' => $rows,
      'attributes' => array(
        'class' => array(
          'view',
          "preview_$fid",
        ),
        'id' => 'tpps_table_display',
      ),
      'caption' => '',
      'colgroups' => NULL,
      'sticky' => FALSE,
      'empty' => '',
    );
    $display = render(theme('table', $vars));
    print($display);
    return;
  }
}

/**
 * Returns an entity link based on an id and a name.
 *
 * @param int $id
 *   The record id in CHADO.
 * @param string $name
 *   The display text of the link.
 * @param string $label
 *   The label of the bundle.
 *
 * @return string
 *   The constructed link, or the provided name if no entity is found.
 */
function tpps_entity_link($id, $name, $label) {
  global $base_url;
  $bundle = tripal_load_bundle_entity(array('label' => $label));
  if (empty($bundle)) {
    return $name;
  }
  $entity_id = chado_get_record_entity_by_bundle($bundle, $id);
  if (!empty($entity_id)) {
    return "<a href=\"$base_url/bio_data/$entity_id\">$name</a>";
  }
  return $name;
}

/**
 * Displays tags with proper styling.
 *
 * @param array $tags
 *   The array of tags to display.
 *
 * @return string
 *   HTML string displaying all of the tags.
 */
function tpps_show_tags(array $tags) {
  $display = "";
  foreach ($tags as $tag) {
    $color = !empty($tag['color']) ? $tag['color'] : 'white';
    $display .= "<span class=\"tag\" style=\"background-color:$color\"><span class=\"tag-text\">{$tag['name']}</span></span>";
  }
  return $display;
}
