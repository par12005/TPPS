<?php

/**
 * @file
 */

/**
 *
 */
function tpps_flatten($state) {
  $data = isset($state['saved_values']) ? $state['saved_values'] : NULL;

  if (!$data) {
    return;
  }
  unset($data['frontpage']);
  unset($data['form_build_id']);

  $page_1 = &$data[PAGE_1];
  $pub = &$page_1['publication'];
  $org = &$page_1['organism'];

  if (isset($page_1['#cleared']) and $page_1['#cleared']) {
    $data['#page_1_cleared'] = TRUE;
  }
  else {
    $data['#page_1_cleared'] = FALSE;
  }
  $data['Publication Title'] = isset($pub['title']) ? $pub['title'] : NULL;
  $data['Primary Author'] = isset($page_1['primaryAuthor']) ? $page_1['primaryAuthor'] : NULL;
  $data['Organization'] = isset($page_1['organization']) ? $page_1['organization'] : NULL;
  $data['Publication Status'] = isset($pub['status']) ? $pub['status'] : NULL;
  $data['Publication Year'] = isset($pub['year']) ? $pub['year'] : NULL;
  $data['Publication Abstract'] = isset($pub['abstract']) ? $pub['abstract'] : NULL;
  $data['Publication Journal'] = isset($pub['journal']) ? $pub['journal'] : NULL;

  if (isset($pub['secondaryAuthors']['number'])) {
    $data['#number_secondary_authors'] = $pub['secondaryAuthors']['number'];
    for ($i = 1; $i < $data['#number_secondary_authors']; $i++) {
      if (isset($pub['secondaryAuthors']["$i"]) and $pub['secondaryAuthors']["$i"] != '') {
        $data["Secondary Author $i"] = $pub['secondaryAuthors']["$i"];
      }
    }
    if ($pub['secondaryAuthors']['check'] and $pub['secondaryAuthors']['file']) {
      $data["Secondary Authors File"] = $pub['secondaryAuthors']['file'];
      $data['#secondary_authors_file_no_header'] = isset($pub['secondaryAuthors']['file-no-header']) ? $pub['secondaryAuthors']['file-no-header'] : NULL;
    }
  }

  if (isset($org['number'])) {
    $data['#number_organisms'] = $org['number'];
    for ($i = 1; $i <= $data['#number_organisms']; $i++) {
      if (isset($org["$i"]) and $org["$i"] != '') {
        $data["Species $i"] = $org["$i"];
      }
    }
  }
  unset($data[PAGE_1]);

  $page_2 = &$data[PAGE_2];
  $loc = &$page_2['studyLocation'];

  if (isset($page_2['#cleared']) and $page_2['#cleared']) {
    $data['#page_2_cleared'] = TRUE;
  }
  else {
    $data['#page_2_cleared'] = FALSE;
  }
  $data['Experiment Starting Month'] = isset($page_2['StartingDate']['month']) ? $page_2['StartingDate']['month'] : NULL;
  $data['Experiment Starting Year'] = isset($page_2['StartingDate']['year']) ? $page_2['StartingDate']['year'] : NULL;
  $data['Experiment Ending Month'] = isset($page_2['EndingDate']['month']) ? $page_2['EndingDate']['month'] : NULL;
  $data['Experiment Ending Year'] = isset($page_2['EndingDate']['year']) ? $page_2['EndingDate']['year'] : NULL;
  $data['#location_type'] = (isset($loc['type']) and $loc['type']) ? $loc['type'] : NULL;

  if ($data['#location_type'] == '2' and isset($loc['custom'])) {
    $data['Study Location'] = $loc['custom'];
  }
  elseif ($data['#location_type'] and isset($loc['coordinates'])) {
    $data['Study Location'] = $loc['coordinates'];
  }

  if (isset($page_2['dataType']) and $page_2['dataType']) {
    $data['Data Type'] = $page_2['dataType'];
  }

  if (isset($page_2['studyType']) and $page_2['studyType']) {
    $study_type_options = array(
      1 => 'Natural Population (Landscape)',
      2 => 'Growth Chamber',
      3 => 'Greenhouse',
      4 => 'Experimental/Common Garden',
      5 => 'Plantation',
    );
    if (isset($study_type_options[$page_2['studyType']])) {
      $data['Study Type'] = $study_type_options[$page_2['studyType']];
    }

    switch ($page_2['studyType']) {
      case 1:
        if (isset($page_2['naturalPopulation'])) {
          $nat_pop = &$page_2['naturalPopulation'];
          $data['Seasons Assessed'] = '';
          foreach ($nat_pop['season'] as $key => $item) {
            if (isset($item) and $item != '0') {
              $data['Seasons Assessed'] .= $item . ' ';
            }
          }
          if (isset($nat_pop['assessions']) and $nat_pop['assessions']) {
            $data['Average number of assessions'] = $nat_pop['assessions'];
          }
        }
        break;

      case 2:
        if (isset($page_2['growthChamber'])) {
          $growth = &$page_2['growthChamber'];
          $root = &$growth['rooting'];
          $treatment = &$root['treatment'];

          if (isset($growth['co2Control']['option']) and $growth['co2Control']['option'] and $growth['co2Control']['option'] == '1') {
            $data['CO2 Control'] = 'Controlled';
            $data['CO2 Value'] = isset($growth['co2Control']['controlled']) ? $growth['co2Control']['controlled'] : NULL;
          }
          elseif (isset($growth['co2Control']['option']) and $growth['co2Control']['option']) {
            $data['CO2 Control'] = 'Uncontrolled';
            $data['CO2 Value'] = isset($growth['co2Control']['uncontrolled']) ? $growth['co2Control']['uncontrolled'] : NULL;
          }

          if (isset($growth['humidityControl']['option']) and $growth['humidityControl']['option'] and $growth['humidityControl']['option'] == '1') {
            $data['Humidity Control'] = 'Controlled';
            $data['Humidity Value'] = isset($growth['humidityControl']['controlled']) ? $growth['humidityControl']['controlled'] : NULL;
          }
          elseif (isset($growth['humidityControl']['option']) and $growth['humidityControl']['option']) {
            $data['Humidity Control'] = 'Uncontrolled';
            $data['Humidity Value'] = isset($growth['humidityControl']['uncontrolled']) ? $growth['humidityControl']['uncontrolled'] : NULL;
          }

          if (isset($growth['lightControl']['option']) and $growth['lightControl']['option'] and $growth['lightControl']['option'] == '1') {
            $data['Light Intensity Control'] = 'Controlled';
            $data['Light Intensity Value'] = isset($growth['lightControl']['controlled']) ? $growth['lightControl']['controlled'] : NULL;
          }
          elseif (isset($growth['lightControl']['option']) and $growth['lightControl']['option']) {
            $data['Light Intensity Control'] = 'Uncontrolled';
            $data['Light Intensity Value'] = isset($growth['lightControl']['uncontrolled']) ? $growth['lightControl']['uncontrolled'] : NULL;
          }

          $data['Average High Temperature'] = isset($growth['temp']['high']) ? $growth['temp']['high'] : NULL;
          $data['Average Low Temperature'] = isset($growth['temp']['low']) ? $growth['temp']['low'] : NULL;

          if (isset($root['option'])) {
            $rooting_options = array(
              1 => 'Aeroponics',
              2 => 'Hydroponics',
              3 => 'Soil',
            );
            if (isset($rooting_options[$growth['rooting']['option']])) {
              $data['Rooting Type'] = $rooting_options[$growth['rooting']['option']];
            }

            if (isset($data['Rooting Type']) and $data['Rooting Type'] == 'Soil') {
              $soil_options = array(
                1 => 'Sand',
                2 => 'Peat',
                3 => 'Clay',
                4 => 'Mixed',
                5 => 'Other',
              );
              if (isset($soil_options[$root['soil']['type']])) {
                $data['Soil Type'] = $soil_options[$root['soil']['type']];
              }

              if (isset($data['Soil Type']) and $data['Soil Type'] == 'Other') {
                $data['Custom Soil Type'] = $root['soil']['other'];
              }

              $data['Soil Container Type'] = isset($root['soil']['container']) ? $root['soil']['container'] : NULL;
            }
          }

          if (isset($root['ph']['option']) and $root['ph']['option']) {
            $data['pH Control'] = 'Controlled';
            $data['pH Value'] = isset($root['ph']['controlled']) ? $root['ph']['controlled'] : NULL;
          }
          else {
            $data['pH Control'] = 'Uncontrolled';
            $data['pH Value'] = isset($root['ph']['uncontrolled']) ? $root['ph']['uncontrolled'] : NULL;
          }

          if (isset($treatment)) {
            $selected = FALSE;
            $description = FALSE;

            foreach ($treatment as $field => $value) {
              if (!$description) {
                $description = TRUE;
                $selected = $value;
                continue;
              }
              elseif ($selected == '1') {
                $treatment_name = substr($field, 0, -12);
                $data["Treatment: $treatment_name description"] = isset($value) ? $value : NULL;
              }
              $description = FALSE;
            }
          }
        }
        break;

      case 3:
        if (isset($page_2['greenhouse'])) {
          $green = &$page_2['greenhouse'];
          $root = &$green['rooting'];
          $treatment = &$root['treatment'];

          if (isset($green['humidityControl']['option']) and $green['humidityControl']['option'] and $green['humidityControl']['option'] == '1') {
            $data['Humidity Control'] = 'Controlled';
            $data['Humidity Value'] = isset($green['humidityControl']['controlled']) ? $green['humidityControl']['controlled'] : NULL;
          }
          elseif (isset($green['humidityControl']['option']) and $green['humidityControl']['option']) {
            $data['Humidity Control'] = 'Uncontrolled';
          }

          if (isset($green['lightControl']['option']) and $green['lightControl']['option'] and $green['lightControl']['option'] == '1') {
            $data['Light Intensity Control'] = 'Controlled';
            $data['Light Intensity Value'] = isset($green['lightControl']['controlled']) ? $green['lightControl']['controlled'] : NULL;
          }
          elseif (isset($green['lightControl']['option']) and $green['lightControl']['option']) {
            $data['Light Intensity Control'] = 'Uncontrolled';
          }

          $data['Average High Temperature'] = isset($green['temp']['high']) ? $green['temp']['high'] : NULL;
          $data['Average Low Temperature'] = isset($green['temp']['low']) ? $green['temp']['low'] : NULL;

          if (isset($root['option'])) {
            $rooting_options = array(
              1 => 'Aeroponics',
              2 => 'Hydroponics',
              3 => 'Soil',
            );
            if (isset($rooting_options[$green['rooting']['option']])) {
              $data['Rooting Type'] = $rooting_options[$green['rooting']['option']];
            }

            if (isset($data['Rooting Type']) and $data['Rooting Type'] == 'Soil') {
              $soil_options = array(
                1 => 'Sand',
                2 => 'Peat',
                3 => 'Clay',
                4 => 'Mixed',
                5 => 'Other',
              );
              if (isset($soil_options[$root['soil']['type']])) {
                $data['Soil Type'] = $soil_options[$root['soil']['type']];
              }

              if (isset($data['Soil Type']) and $data['Soil Type'] == 'Other') {
                $data['Custom Soil Type'] = $root['soil']['other'];
              }

              $data['Soil Container Type'] = isset($root['soil']['container']) ? $root['soil']['container'] : NULL;
            }
          }

          if (isset($root['ph']['option']) and $root['ph']['option']) {
            $data['pH Control'] = 'Controlled';
            $data['pH Value'] = isset($root['ph']['controlled']) ? $root['ph']['controlled'] : NULL;
          }
          else {
            $data['pH Control'] = 'Uncontrolled';
          }

          if (isset($treatment)) {
            $selected = FALSE;
            $description = FALSE;

            foreach ($treatment as $field => $value) {
              if (!$description) {
                $description = TRUE;
                $selected = $value;
                continue;
              }
              elseif ($selected == '1') {
                $treatment_name = substr($field, 0, -12);
                $data["Treatment: $treatment_name description"] = isset($value) ? $value : NULL;
              }
              $description = FALSE;
            }
          }
        }
        break;

      case 4:
        if (isset($page_2['commonGarden'])) {
          $garden = &$page_2['commonGarden'];

          if (isset($garden['irrigation']['option']) and $garden['irrigation']['option']) {
            $irrigation_options = array(
              1 => 'Irrigation from top',
              2 => 'Irrigation from bottom',
              3 => 'Drip Irrigation',
              4 => 'Other',
              5 => 'No Irrigation',
            );
            if (isset($irrigation_options[$garden['irrigation']['option']])) {
              $data['Irrigation Type'] = $irrigation_options[$garden['irrigation']['option']];
            }

            if (isset($data['Irrigation Type']) and $data['Irrigation Type'] == 'Other') {
              $data['Custom Irrigation Type'] = $garden['irrigation']['other'];
            }
          }

          if (isset($garden['salinity']['option']) and $garden['salinity']['option'] and $garden['salinity']['option'] == '1') {
            $data['Salinity Control'] = 'Controlled';
            $data['Salinity Value'] = isset($garden['salinity']['controlled']) ? $garden['salinity']['controlled'] : NULL;
          }
          elseif (isset($garden['salinity']['option']) and $garden['salinity']['option']) {
            $data['Salinity Control'] = 'Uncontrolled';
            $data['Salinity Value'] = isset($garden['salinity']['uncontrolled']) ? $garden['salinity']['uncontrolled'] : NULL;
          }

          $biotic_envs = array();
          foreach ($garden['bioticEnv']['option'] as $key => $val) {
            if ($val) {
              $biotic_envs[] = $key;
            }
          }
          $data['Biotic Environment'] = (isset($biotic_envs) and count($biotic_envs) > 1) ? implode(', ', $biotic_envs) : NULL;

          if (isset($garden['bioticEnv']['option']['Other']) and $garden['bioticEnv']['option']['Other'] and isset($garden['bioticEnv']['other']) and $garden['bioticEnv']['other'] != '') {
            $data['Custom Biotic Environment'] = $garden['bioticEnv']['other'];
          }

          $seasons = array();
          foreach ($garden['season'] as $key => $item) {
            if (isset($item) and $item != '0') {
              $seasons[] = $item;
            }
          }
          $data['Seasons Assessed'] = (isset($seasons) and count($seasons) > 1) ? implode(', ', $seasons) : NULL;

          if (isset($garden['treatment']['check']) and $garden['treatment']['check']) {
            $treatment = &$garden['treatment'];
            $selected = FALSE;
            $description = FALSE;

            foreach ($treatment as $field => $value) {
              if ($field != 'check') {
                if (!$description) {
                  $description = TRUE;
                  $selected = $value;
                  continue;
                }
                elseif (isset($selected) and $selected == '1') {
                  $treatment_name = substr($field, 0, -12);
                  $data["Treatment: $treatment_name description"] = $value;
                }
                $description = FALSE;
              }
            }
          }
        }
        break;

      case 5:
        if (isset($page_2['plantation'])) {
          $plant = &$page_2['naturalPopulation'];
          $data['Seasons Assessed'] = '';
          foreach ($plant['season'] as $key => $item) {
            if (isset($item) and $item != '0') {
              $data['Seasons Assessed'] .= $item . ' ';
            }
          }

          if (isset($plant['assessions']) and $plant['assessions']) {
            $data['Average number of assessions'] = $plant['assessions'];
          }

          if (isset($plant['treatment']['check']) and $plant['treatment']['check']) {
            $treatment = &$plant['treatment'];
            $selected = FALSE;
            $description = FALSE;

            foreach ($treatment as $field => $value) {
              if ($field != 'check') {
                if (!$description) {
                  $description = TRUE;
                  $selected = $value;
                  continue;
                }
                elseif (isset($selected) and $selected == '1') {
                  $treatment_name = substr($field, 0, -12);
                  $data["Treatment: $treatment_name description"] = $value;
                }
                $description = FALSE;
              }
            }
          }
        }
        break;

      default:
        break;
    }
  }
  unset($data[PAGE_2]);

  $page_3 = &$data[PAGE_3];

  if (isset($page_3['#cleared']) and $page_3['#cleared']) {
    $data['#page_3_cleared'] = TRUE;
  }
  else {
    $data['#page_3_cleared'] = FALSE;
  }
  $page_3 = &$data[PAGE_3]['tree-accession'];
  if (isset($page_3['check']) and $page_3['check']) {
    for ($i = 1; $i <= $data['#number_organisms']; $i++) {
      if (isset($page_3["species-$i"]['file']) and $page_3["species-$i"]['file']) {
        $organism_name = $data["Species $i"];
        $data["$organism_name Accession File"] = $page_3["species-$i"]['file'];
        if (!empty($page_3["species-$i"]['file-groups']['Location (latitude/longitude or country/state)']['#type']) and $page_3["species-$i"]['file-groups']['Location (latitude/longitude or country/state)']['#type'] == "gps") {
          $data["#{$organism_name}_accession_no_header"] = isset($page_3["species-$i"]['file-no-header']) ? $page_3["species-$i"]['file-no-header'] : '0';
          $data["#{$organism_name}_accession_id_col"] = $page_3["species-$i"]['file-groups']['Tree Id'][1];
          $data["#{$organism_name}_accession_lat_col"] = $page_3["species-$i"]['file-groups']['Location (latitude/longitude or country/state)'][4];
          $data["#{$organism_name}_accession_long_col"] = $page_3["species-$i"]['file-groups']['Location (latitude/longitude or country/state)'][5];
        }
      }
    }
  }
  elseif (isset($page_3['file']) and $page_3['file']) {
    $data['Tree Accession File'] = $page_3['file'];
    if (!empty($page_3['file-groups']['Location (latitude/longitude or country/state)']['#type']) and $page_3['file-groups']['Location (latitude/longitude or country/state)']['#type'] == "gps") {
      $data["#accession_no_header"] = isset($page_3['file-no-header']) ? $page_3['file-no-header'] : '0';
      $data["#accession_id_col"] = $page_3['file-groups']['Tree Id'][1];
      $data["#accession_lat_col"] = $page_3['file-groups']['Location (latitude/longitude or country/state)'][4];
      $data["#accession_long_col"] = $page_3['file-groups']['Location (latitude/longitude or country/state)'][5];
    }
  }
  unset($data[PAGE_3]);

  $page_4 = &$data[PAGE_4];

  if (isset($page_4['#cleared']) and $page_4['#cleared']) {
    $data['#page_4_cleared'] = TRUE;
  }
  else {
    $data['#page_4_cleared'] = FALSE;
  }
  for ($i = 1; $i <= $data['#number_organisms']; $i++) {
    if (isset($page_4["organism-$i"])) {
      $org = &$page_4["organism-$i"];
      $org_name = isset($data["Species $i"]) ? $data["Species $i"] : NULL;

      if (isset($org['phenotype'])) {
        $pheno = &$org['phenotype'];

        if (isset($pheno['number'])) {
          $data['#number_phenotypes'] = $pheno['number'];
          for ($j = 1; $j < $data['#number_phenotypes']; $j++) {
            $fields = array(
              'name' => 'Name',
              'attribute' => 'Attribute',
              'description' => 'Description',
              'units' => 'Units',
            );
            if (isset($pheno["$j"]['struct-check']) and $pheno["$j"]['struct-check']) {
              $fields['structure'] = 'Structure';
            }
            if (isset($pheno["$j"]['val-check']) and $pheno["$j"]['val-check']) {
              $fields['min'] = 'Minimum Value';
              $fields['max'] = 'Maximum Value';
            }

            foreach ($fields as $field => $field_string) {
              if (isset($pheno["$j"][$field]) and $pheno["$j"][$field] != '') {
                $data["$org_name Phenotype $j $field_string"] = $pheno[$j][$field];
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
      }

      if (isset($org['genotype'])) {
        $geno = &$org['genotype'];
        $file_type = &$geno['file-type'];

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

        if (isset($file_type['Genotype Assay']) and $file_type['Genotype Assay'] and isset($geno['file']) and $geno['file']) {
          $data["$org_name Genotype Assay File"] = $geno['file'];
        }

        if (isset($file_type['Assay Design']) and $file_type['Assay Design'] and isset($geno['assay-design']) and $geno['assay-design']) {
          $data["$org_name Genotype Assay Design File"] = $geno['assay-design'];
        }

        if (isset($file_type['VCF']) and $file_type['VCF'] and isset($geno['vcf']) and $geno['vcf']) {
          $data["$org_name Genotype VCF File"] = $geno['vcf'];
        }
      }

      if (isset($org['environment'])) {
        $env = &$org['environment'];

        if (variable_get('tpps_cartogratree_env', FALSE) and !empty($env['use_layers'])) {
          foreach ($env['env_layers'] as $layer => $layer_id) {
            if (!empty($layer_id) and !empty($env['env_params'][$layer])) {
              $params = array();
              foreach ($env['env_params'][$layer] as $param => $param_id) {
                if (!empty($param_id)) {
                  $params[] = $param;
                }
              }
              $data["Cartogratree Layer: $layer Parameters"] = implode(', ', $params);
            }
          }
        }

        if (!empty($env['env_manual_check'])) {
          if (!empty($env['number'])) {
            $data['#number_manual_env'] = $env['number'];
            for ($j = 1; $j <= $data['#number_manual_env']; $j++) {
              $fields = array(
                'name' => 'Name',
                'description' => 'Description',
                'units' => 'Units',
                'value' => 'Value',
              );

              foreach ($fields as $field => $field_string) {
                if (!empty($env['env_manual']["$j"][$field])) {
                  $data["$org_name Environment Value $j $field_string"] = $env['env_manual'][$j][$field];
                }
              }
            }
          }
        }
      }
    }
  }
  unset($data[PAGE_4]);

  $sum_page = &$data['summarypage'];
  if (!empty($sum_page['comments'])) {
    $data["Additional Comments"] = $sum_page['comments'];
  }
  for ($i = 1; $i <= 10; $i++) {
    if (!empty($sum_page['files'][$i])) {
      $data["Supplemental File $i"] = $sum_page['files'][$i];
    }
  }
  unset($data['summarypage']);

  $data['#dbxref_id'] = isset($state['dbxref_id']) ? $state['dbxref_id'] : NULL;
  $data['#accession'] = isset($state['accession']) ? $state['accession'] : NULL;
  $data['#status'] = isset($state['status']) ? $state['status'] : NULL;

  foreach ($data as $key => $val) {
    if (empty($val)) {
      unset($data[$key]);
    }
  }

  return $data;
}
