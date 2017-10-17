<?php
function page_3_create_form(&$form){
    
    $form['dataType'] = array(
      '#type' => 'select',
      '#title' => t('Please select data type:'),
      '#options' => array(
        'Genotype x Phenotype',
        'Genotype',
        'Genotype x Phenotype x Environment',
        'Phenotype x Environment',
        'Genotype x Environment'
      ),
      '#required' => TRUE,
    );
    
    function tree_access(&$form){
        
        $form['tree-access'] = array(
          '#type' => 'fieldset',
          '#title' => t('Tree Accession Information:'),
          '#tree' => TRUE
        );

        $form['tree-access']['identifier'] = array(
          '#type' => 'textfield',
          '#title' => t('Identifier:'),
          '#required' => TRUE
        );

        $form['tree-access']['location']['type'] = array(
          '#type' => 'select',
          '#title' => t('Location Type:'),
          '#options' => array(
            0 => 'Please select a location type',
            1 => 'Latitude/Longitude (WGS 84)',
            3 => 'Latitude/Longitude (NAD 83)',
            4 => 'Latitude/Longitude (ETRS 89)',
            2 => 'Custom Location'
          ),
        );

        $form['tree-access']['location']['latitude'] = array(
          '#type' => 'textfield',
          '#title' => t('Latitude:'),
          '#states' => array(
            'visible' => array(
              array(
                array(':input[name="tree-access[location][type]"]' => array('value' => '1')),
                'or',
                array(':input[name="tree-access[location][type]"]' => array('value' => '3')),
                'or',
                array(':input[name="tree-access[location][type]"]' => array('value' => '4')),
              )
            ),
            'required' => array(
              array(
                array(':input[name="tree-access[location][type]"]' => array('value' => '1')),
                'or',
                array(':input[name="tree-access[location][type]"]' => array('value' => '3')),
                'or',
                array(':input[name="tree-access[location][type]"]' => array('value' => '4')),
              )
            )
          ),
        );

        $form['tree-access']['location']['longitude'] = array(
          '#type' => 'textfield',
          '#title' => t('Longitude:'),
          '#states' => array(
            'visible' => array(
              array(
                array(':input[name="tree-access[location][type]"]' => array('value' => '1')),
                'or',
                array(':input[name="tree-access[location][type]"]' => array('value' => '3')),
                'or',
                array(':input[name="tree-access[location][type]"]' => array('value' => '4')),
              )
            ),
            'required' => array(
              array(
                array(':input[name="tree-access[location][type]"]' => array('value' => '1')),
                'or',
                array(':input[name="tree-access[location][type]"]' => array('value' => '3')),
                'or',
                array(':input[name="tree-access[location][type]"]' => array('value' => '4')),
              )
            )
          ),
        );

        $form['tree-access']['location']['custom'] = array(
          '#type' => 'fieldset',
          '#states' => array(
            'visible' => array(
              ':input[name="tree-access[location][type]"]' => array('value' => '2')
            ),
          ),
        );
        
        $form['tree-access']['location']['country'] = array(
          '#type' => 'textfield',
          '#title' => t('Country:'),
          '#states' => array(
            'visible' => array(
              ':input[name="tree-access[location][type]"]' => array('value' => '2')
            ),
            'required' => array(
              ':input[name="tree-access[location][type]"]' => array('value' => '2')
            )
          )
        );
        
        $form['tree-access']['location']['region'] = array(
          '#type' => 'textfield',
          '#title' => t('State/Province/Region:'),
          '#states' => array(
            'invisible' => array(
              ':input[name="tree-access[location][country]"]' => array('value' => '')
            )
          )
        );

        $form['tree-access']['age'] = array(
          '#type' => 'textfield',
          '#title' => t('Age and development Stage'),
          '#required' => TRUE
        );

        $form['tree-access']['tissue'] = array(
          '#type' => 'select',
          '#title' => t('Tissue types'),
          '#options' => array(
            'Tissue 1',
            'Tissue 2'
          ),
        );

        $form['tree-access']['clone'] = array(
          '#type' => 'textfield',
          '#title' => t('Clone and or pedigree information')
        );
        
        return $form;
    }
    
    function phenotype(&$form){
        
        $num_pheno = array();
        
        for($i = 0; $i < 20; $i++){
            $num_pheno[$i] = $i + 1;
        }
        $num_pheno[20] = '>20';

        $form['phenotype'] = array(
          '#type' => 'fieldset',
          '#title' => t('Phenotype Information:'),
          '#tree' => TRUE,
          '#states' => array(
            'visible' => array(
              array(
                array(':input[name="dataType"]' => array('value' => '0')),
                'or',
                array(':input[name="dataType"]' => array('value' => '2')),
                'or',
                array(':input[name="dataType"]' => array('value' => '3')),
              )
            ),
            'required' => array(
              array(
                array(':input[name="dataType"]' => array('value' => '0')),
                'or',
                array(':input[name="dataType"]' => array('value' => '2')),
                'or',
                array(':input[name="dataType"]' => array('value' => '3')),
              )
            )
          )
        );

        $form['phenotype']['number'] = array(
          '#type' => 'select',
          '#title' => t('Number of Phenotypes'),
          '#options' => $num_pheno,
        );

        for($i = 0; $i < 20; $i++){

            $visible_values = array();
            array_push($visible_values, array(':input[name="phenotype[number]"]' => array('value' => "$i")));

            for ($j = $i + 1; $j < 20; $j++){
                array_push($visible_values, 'or');
                array_push($visible_values, array(':input[name="phenotype[number]"]' => array('value' => "$j")));
            }

            $phenotype_label = $i + 1;
            $form['phenotype'][$i] = array(
              '#type' => 'fieldset',
              '#title' => t("Phenotype $phenotype_label"),
              '#states' => array(
                'visible' => array($visible_values),
                'required' => array($visible_values)
              )
            );

            $form['phenotype'][$i]['name'] = array(
              '#type' => 'textfield',
              '#title' => t('Name'),
              '#states' => array(
                'visible' => array($visible_values),
                'required' => array($visible_values)
              )
            );

            $form['phenotype'][$i]['phenotype_or_environmental'] = array(
              '#type' => 'select',
              '#title' => t('Phenotype or Environmental Variable:'),
              '#options' => array(
                2 => t('Please identify phenotype or environmental variable'),
                0 => t('Phenotype'),
                1 => t('Environmental Variable'),
              ),
              '#default_value' => 2,
              '#states' => array(
                'visible' => array($visible_values),
                'required' => array($visible_values)
              )
            );

            $form['phenotype'][$i]['type'] = array(
              '#type' => 'select',
              '#title' => t('Type'),
              '#options' => array(
                0 => 'Please select a type',
                1 => 'Binary',
                2 => 'Quantitative',
                3 => 'Qualitative'
              ),
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                )
              )
            );

            $form['phenotype'][$i]['binary'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][type]"]' => array('value' => '1')
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][type]"]' => array('value' => '1')
                )
              ),
            );

            $form['phenotype'][$i]['binary'][1] = array(
              '#type' => 'textfield',
              '#title' => t('Type 1:'),
            );

            $form['phenotype'][$i]['binary'][2] = array(
              '#type' => 'textfield',
              '#title' => t('Type 2:'),
            );

            $form['phenotype'][$i]['quantitative'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][type]"]' => array('value' => '2')
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][type]"]' => array('value' => '2')
                )
              ),
            );

            $form['phenotype'][$i]['quantitative']['min'] = array(
              '#type' => 'textfield',
              '#title' => t('Minimum value:'),
            );

            $form['phenotype'][$i]['quantitative']['max'] = array(
              '#type' => 'textfield',
              '#title' => t('Maximum value:'),
            );

            $form['phenotype'][$i]['description'] = array(
              '#type' => 'textarea',
              '#title' => t('Please provide a short description of your phenotype:'),
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                )
              )
            );

            $form['phenotype'][$i]['units'] = array(
              '#type' => 'select',
              '#title' => t('Units:'),
              '#options' => array(
                0 => 'Please select a unit type',
                1 => 'mm',
                2 => 'cm',
                3 => 'm',
                4 => 'Degrees Celsius',
                5 => 'Degrees Fahrenheit',
                6 => 'Other'
              ),
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                )
              )
            );

            $form['phenotype'][$i]['units-other'] = array(
              '#type' => 'textfield',
              '#title' => t('Define Unit type'),
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][units]"]' => array('value' => '6')
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][units]"]' => array('value' => '6')
                )
              )
            );
            
            $form['phenotype'][$i]['structure'] = array(
              '#type' => 'textfield',
              '#title' => t('Plant Structure:'),
              '#description' => t('Please choose a term that describes the plant structure the phenotype was measured on.'),
              '#autocomplete_path' => 'structure/autocomplete',
              '#maxlength' => 128,
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                )
              )
            );
            
            $form['phenotype'][$i]['structure-custom'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                )
              )
            );
            
            $form['phenotype'][$i]['structure-custom']['check'] = array(
              '#type' => 'checkbox',
              '#title' => t('None of these terms meet my needs'),
              '#required' => false,
            );
            
            $form['phenotype'][$i]['structure-custom']['term'] = array(
              '#type' => 'textfield',
              '#title' => t('Please enter the name of your custom term'),
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][structure-custom][check]"]' => array('checked' => TRUE)
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][structure-custom][check]"]' => array('checked' => TRUE)
                ),
              )
            );
            
            $form['phenotype'][$i]['structure-custom']['definition'] = array(
              '#type' => 'textfield',
              '#title' => t('Please enter the definition of your custom term'),
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][structure-custom][check]"]' => array('checked' => TRUE)
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][structure-custom][check]"]' => array('checked' => TRUE)
                ),
              )
            );
            
            $form['phenotype'][$i]['developmental'] = array(
              '#type' => 'textfield',
              '#title' => t('Developmental Stage:'),
              '#description' => t('Please choose a term that describes the developmental stage the phenotype was measured on.'),
              '#autocomplete_path' => 'developmental/autocomplete',
              '#maxlength' => 128,
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                )
              )
            );
            
            $form['phenotype'][$i]['developmental-custom'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][phenotype_or_environmental]"]' => array('value' => '0')
                )
              )
            );
            
            $form['phenotype'][$i]['developmental-custom']['check'] = array(
              '#type' => 'checkbox',
              '#title' => t('None of these terms meet my needs'),
              '#required' => false
            );
            
            $form['phenotype'][$i]['developmental-custom']['term'] = array(
              '#type' => 'textfield',
              '#title' => t('Please enter the name of your custom term'),
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][developmental-custom][check]"]' => array('checked' => TRUE)
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][developmental-custom][check]"]' => array('checked' => TRUE)
                ),
              )
            );
            
            $form['phenotype'][$i]['developmental-custom']['definition'] = array(
              '#type' => 'textfield',
              '#title' => t('Please enter the definition of your custom term'),
              '#states' => array(
                'visible' => array(
                  ':input[name="phenotype[' . $i . '][developmental-custom][check]"]' => array('checked' => TRUE)
                ),
                'required' => array(
                  ':input[name="phenotype[' . $i . '][developmental-custom][check]"]' => array('checked' => TRUE)
                ),
              )
            );
        }

        $form['phenotype']['upload'] = array(
          '#type' => 'file',
          '#title' => t('Phenotype file'),
          '#title_display' => 'invisible',
          '#states' => array( 
            'visible' => array(
              ':input[name="phenotype[number]"]' => array('value' => '20')
            ),
            'required' => array(
              ':input[name="phenotype[number]"]' => array('value' => '20')
            )
          )
        );
        
        return $form;
    }
    
    function genotype(&$form){
        
        $form['genotype'] = array(
          '#type' => 'fieldset',
          '#title' => t('Genotype Information:'),
          '#tree' => TRUE,
          '#states' => array(
            'visible' => array(
              array(
                array(':input[name="dataType"]' => array('value' => '0')),
                'or',
                array(':input[name="dataType"]' => array('value' => '1')),
                'or',
                array(':input[name="dataType"]' => array('value' => '2')),
                'or',
                array(':input[name="dataType"]' => array('value' => '4')),
              )
            )
          )
        );
        
        $form['genotype']['marker-type'] = array(
          '#type' => 'checkboxes',
          '#title' => t('Marker Type (select all that apply):'),
          '#options' => drupal_map_assoc(array(
            t('SNPs'),
            t('SSRs/cpSSRs'),
            t('Other'),
          ))
        );

        $form['genotype']['SNPs'] = array(
          '#type' => 'fieldset',
          '#title' => t('SNPs Information:'),
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[marker-type][SNPs]"]' => array('checked' => true)
            )
          )
        );

        $form['genotype']['SNPs']['genotyping-design'] = array(
          '#type' => 'select',
          '#title' => t('Define Genotyping Design'),
          '#options' => array(
            0 => 'Please select a Genotyping Design',
            1 => 'GBS',
            2 => 'Targeted Capture',
            3 => 'Whole Genome Resequencing',
            4 => 'RNA-Seq',
            5 => 'Genotyping Array'
          )
        );

        $form['genotype']['SNPs']['GBS'] = array(
          '#type' => 'select',
          '#title' => t('GBS Type'),
          '#options' => array(
            0 => 'Please select a GBS type',
            1 => 'RADSeq',
            2 => 'ddRAD-Seq',
            3 => 'NextRAD',
            4 => 'RAPTURE',
            5 => 'Other'
          ),
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[SNPs][genotyping-design]"]' => array('value' => '1')
            )
          )
        );

        $form['genotype']['SNPs']['GBS-other'] = array(
          '#type' => 'textfield',
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[SNPs][GBS]"]' => array('value' => '5'),
              ':input[name="genotype[SNPs][genotyping-design]"]' => array('value' => '1')
            )
          )
        );

        $form['genotype']['SNPs']['targeted-capture'] = array(
          '#type' => 'select',
          '#title' => t('Targeted Capture Type'),
          '#options' => array(
            0 => 'Please select a Targeted Capture Type',
            1 => 'Exome Capture',
            2 => 'Other'
          ),
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[SNPs][genotyping-design]"]' => array('value' => '2')
            )
          )
        );

        $form['genotype']['SNPs']['targeted-capture-other'] = array(
          '#type' => 'textfield',
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[SNPs][targeted-capture]"]' => array('value' => '2'),
              ':input[name="genotype[SNPs][genotyping-design]"]' => array('value' => '2')
            )
          )
        );

        $form['genotype']['SNPs']['bioproject'] = array(
          '#type' => 'textfield',
          '#title' => 'BioProject ID:'
        );

        $form['genotype']['SNPs']['SNP-file-type'] = array(
          '#type' => 'select',
          '#title' => 'SNP File:',
          '#options' => array(
            0 => 'Please select an SNP file type',
            1 => '.VCF',
            2 => 'Spreadsheet',
          )
        );

        $form['genotype']['SNPs']['SNP-VCF'] = array(
          '#type' => 'file',
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[SNPs][SNP-file-type]"]' => array('value' => '1')
            )
          )
        );

        $form['genotype']['SNPs']['SNP-spreadsheet'] = array(
          '#type' => 'file',
          '#description' => t('Please provide a spreadsheet with the following columns:<br>'
              . '   SNP Name<br>   Type(indel/SNP)<br>   Location<br>   Quality Score<br>   Allele/Reference Allele<br><br>'
              . 'Optional columns:<br>'
              . '   Coverage Depth<br>   Mapping Quality Score<br>'),
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[SNPs][SNP-file-type]"]' => array('value' => '2')
            )
          )
        );

        $form['genotype']['SSRs/cpSSRs'] = array(
          '#type' => 'fieldset',
          '#title' => t('SSRs/cpSSRs Information:'),
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[marker-type][SSRs/cpSSRs]"]' => array('checked' => true)
            )
          )
        );

        $form['genotype']['SSRs/cpSSRs']['define-type'] = array(
          '#type' => 'textfield',
          '#title' => t('Define Type:'),
        );

        $form['genotype']['SSRs/cpSSRs']['load-file'] = array(
          '#type' => 'file',
          '#title' => t('Load File with primer information and marker for each individual genotype:'),
        );

        $form['genotype']['other'] = array(
          '#type' => 'fieldset',
          '#title' => t('Other Marker Type Information:'),
          '#states' => array(
            'visible' => array(
              ':input[name="genotype[marker-type][Other]"]' => array('checked' => true)
            )
          )
        );

        $form['genotype']['other']['define-type'] = array(
          '#type' => 'textfield',
          '#title' => t('Define Type:'),
        );

        $form['genotype']['other']['load-file'] = array(
          '#type' => 'file',
          '#title' => t('Load File:')
        );
        
        return $form;
    };
    
    tree_access($form);
    
    phenotype($form);
    
    genotype($form);
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#title' => t('Submit'),
      '#value' => t('Submit')
    );

    drupal_add_js(drupal_get_path('module', 'custom_module') . "/custom_module.js");
    
    return $form;
}

function page_3_validate_form(&$form, &$form_state){
    $form_values = $form_state['values'];
    $data_type = $form_values['dataType'];
    $location_type = $form_values['tree-access']['location']['type'];
    $latitude = $form_values['tree-access']['location']['latitude'];
    $longitude = $form_values['tree-access']['location']['longitude'];
    $custom_country = $form_values['tree-access']['location']['country'];
    $phenotype = $form_values['phenotype'];
    $genotype = $form_values['genotype'];
    
    function valid_phenotype($phenotype){
        $phenotype_number = $phenotype['number'];
        
        if ($phenotype_number == '20'){
            //same as file upload check in first page
        }
        else{
            for ($i = 1; $i <= $phenotype_number + 1; $i++){
                $phenotype_index = $i - 1;
                $current_phenotype = $phenotype[$phenotype_index];
                $name = $current_phenotype['name'];
                $phenotype_or_env = $current_phenotype['phenotype_or_environmental'];
                $type = $current_phenotype['type'];
                $binary_1 = $current_phenotype['binary'][1];
                $binary_2 = $current_phenotype['binary'][2];
                $quantitative_min = $current_phenotype['quantitative']['min'];
                $quantitative_max = $current_phenotype['quantitative']['max'];
                $description = $current_phenotype['description'];
                $units = $current_phenotype['units'];
                $custom_units = $current_phenotype['units-other'];
                $structure = $current_phenotype['structure'];
                $custom_structure_check = $current_phenotype['structure-custom']['check'];
                $custom_structure_term = $current_phenotype['structure-custom']['term'];
                $custom_structure_definition = $current_phenotype['structure-custom']['definition'];
                $developmental = $current_phenotype['developmental'];
                $custom_developmental_check = $current_phenotype['developmental-custom']['check'];
                $custom_developmental_term = $current_phenotype['developmental-custom']['term'];
                $custom_developmental_definition = $current_phenotype['developmental-custom']['definition'];
                
                if ($name == ''){
                    form_set_error("phenotype][$phenotype_index][name", "Phenotype $i Name: field is required.");
                }
                
                if ($phenotype_or_env == '2'){
                    form_set_error("phenotype][$phenotype_index][phenotype_or_environmental", "Phenotype $i Phenotype or Environmental Variable: field is required.");
                }
                elseif ($phenotype_or_env == '0'){
                    if ($type == '0'){
                        form_set_error("phenotype][$phenotype_index][type", "Phenotype $i Type: field is required.");
                    }
                    elseif ($type == '1'){
                        if ($binary_1 == ''){
                            form_set_error("phenotype][$phenotype_index][binary][1", "Phenotype $i Binary Type 1: field is required.");
                        }
                        
                        if ($binary_2 == ''){
                            form_set_error("phenotype][$phenotype_index][binary][2", "Phenotype $i Binary Type 2: field is required.");
                        }
                    }
                    elseif ($type == '2'){
                        if ($quantitative_min == ''){
                            form_set_error("phenotype][$phenotype_index][quantitative][min", "Phenotype $i Minimum Value: field is required.");
                        }
                        
                        if ($quantitative_max == ''){
                            form_set_error("phenotype][$phenotype_index][quantitative][max", "Phenotype $i Maximum Value: field is required.");
                        }
                    }
                    
                    if ($description == ''){
                        form_set_error("phenotype][$phenotype_index][description", "Phenotype $i Description: field is required.");
                    }
                    
                    if ($units == '0'){
                        form_set_error("phenotype][$phenotype_index][units", "Phenotype $i Units: field is required.");
                    }
                    elseif ($units == '6' and $custom_units == ''){
                        form_set_error("phenotype][$phenotype_index][units-other", "Phenotype $i Custom Units: field is required.");
                    }
                    
                    if ($custom_structure_check == '0' and $structure == ''){
                        form_set_error("phenotype][$phenotype_index][structure", "Phenotype $i Plant Structure: field is required.");
                    }
                    elseif ($custom_structure_check == '1'){
                        if ($custom_structure_term == ''){
                            form_set_error("phenotype][$phenotype_index][structure-custom][term", "Phenotype $i Custom Plant Structure Term: field is required.");
                        }
                        if ($custom_structure_definition == ''){
                            form_set_error("phenotype][$phenotype_index][structure-custom][definition", "Phenotype $i Custom Plant Structure Definition: field is required.");
                        }
                    }
                    
                    if ($custom_developmental_check == '0' and $developmental == ''){
                        form_set_error("phenotype][$phenotype_index][developmental", "Phenotype $i Developmental Stage: field is required.");
                    }
                    elseif ($custom_developmental_check == '1'){
                        if ($custom_developmental_term == ''){
                            form_set_error("phenotype][$phenotype_index][developmental-custom][term", "Phenotype $i Custom Developmental Stage Term: field is required.");
                        }
                        if ($custom_developmental_definition == ''){
                            form_set_error("phenotype][$phenotype_index][developmental-custom][definition", "Phenotype $i Custom Developmental Stage Definition: field is required.");
                        }
                    }
                }
            }
        }
    }
    
    function valid_genotype($genotype){
        $snps_check = $genotype['marker-type']['SNPs'];
        $ssrs_check = $genotype['marker-type']['SSRs/cpSSRs'];
        $custom_marker_check = $genotype['marker-type']['Other'];
        $marker_type = ($snps_check . $ssrs_check . $custom_marker_check);
        $snps = $genotype['SNPs'];
        $ssrs_type = $genotype['SSRs/cpSSRs']['define-type'];
        $custom_marker_type = $genotype['other']['define-type'];
        
        function valid_snps($snps){
            $design = $snps['genotyping-design'];
            $gbs = $snps['GBS'];
            $custom_gbs = $snps['GBS-other'];
            $targeted = $snps['targeted-capture'];
            $custom_targeted = $snps['targeted-capture-other'];
            $bio_id = $snps['bioproject'];
            $file_type = $snps['SNP-file-type'];
            
            if ($design == '0'){
                form_set_error('genotype][SNPs][genotyping-design', 'SNPs Genotyping Design: field is required.');
            }
            elseif($design == '1'){
                if ($gbs == '0'){
                    form_set_error('genotype][SNPs][GBS', 'GBS Type: field is required.');
                }
                elseif ($gbs == '5' and $custom_gbs == ''){
                    form_set_error('genotype][SNPs][GBS-other', 'Custom GBS Type: field is required.');
                }
            }
            elseif($design == '2'){
                if ($targeted == '0'){
                    form_set_error('genotype][SNPs][targeted-capture', 'Targeted Capture Type: field is required.');
                }
                elseif ($targeted == '2' and $custom_targeted == ''){
                    form_set_error('genotype][SNPs][targeted-capture-other', 'Custom Targeted Capture Type: field is required.');
                }
            }
            
            if ($bio_id == ''){
                form_set_error('genotype][SNPs][bioproject', 'BioProject ID: field is required.');
            }
            
            if ($file_type == '0'){
                form_set_error('genotype][SNPs][SNP-file-type', 'SNPs File: field is required.');
            }
        }
        
        if ($marker_type == '000'){
            form_set_error('genotype][marker-type', 'Marker Type: field is required.');
        }
        elseif ($snps_check != '0'){
            valid_snps($snps);
        }
        
        if ($ssrs_check != '0' and $ssrs_type == ''){
            form_set_error('genotype][SSRs/cpSSRs][define-type', 'SSRs/cpSSRs Type: field is required.');
        }
        
        if ($custom_marker_check != '0' and $custom_marker_type == ''){
            form_set_error('genotype][other][define-type', 'Custom Marker Type: field is required.');
        }
    }
    
    if ($location_type == '0'){
        form_set_error('tree-access][location][type', 'Loction Type: field is required.');
    }
    elseif($location_type == '1' or $location_type == '3' or $location_type == '4'){
        if ($latitude == ''){
            form_set_error('tree-access][location][latitude', 'Latitude: field is required.');
        }
        
        if ($longitude == ''){
            form_set_error('tree-access][location][longitude', 'Longitude: field is required.');
        }
    }
    else{
        if($custom_country == ''){
            form_set_error('tree-access][location][country', 'Country: field is required.');
        }
    }
    
    if ($data_type == '0' or $data_type == '2' or $data_type == '3'){
        valid_phenotype($phenotype);
    }
    
    if ($data_type == '0' or $data_type == '1' or $data_type == '2' or $data_type == '4'){
        valid_genotype($genotype);
    }
}

function page_3_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'fourthPage';
}
