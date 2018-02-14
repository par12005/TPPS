<?php
function page_4_create_form(&$form, $form_state){
    
    if (isset($form_state['saved_values']['fourthPage'])){
        $values = $form_state['saved_values']['fourthPage'];
    }
    else{
        $values = array();
    }
    
    function phenotype(&$form, $values, $id){
        
        $fields = array(
          '#type' => 'fieldset',
          '#title' => t('Phenotype Information:')
        );
        
        $phenotype_number = isset($values[$id]['phenotype']['number']) ? $values[$id]['phenotype']['number'] : 1;
        
        $fields['add'] = array(
          '#type' => 'button',
          '#title' => t('Add Phenotype'),
          '#button_type' => 'button',
          '#value' => t('Add Phenotype'),
        );
        
        $fields['remove'] = array(
          '#type' => 'button',
          '#title' => t('Remove Phenotype'),
          '#button_type' => 'button',
          '#value' => t('Remove Phenotype'),
        );
        
        $fields['number'] = array(
          '#type' => 'textfield',
          '#default_value' => $phenotype_number,
        );
        
        for ($i = 1; $i <= 20; $i++){
            
            $fields["$i"] = array(
              '#type' => 'fieldset',
              '#title' => t("Phenotype $i:"),
            );
            
            $fields["$i"]['name'] = array(
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Name:"),
              '#autocomplete_path' => 'phenotype/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['name']) ? $values[$id]['phenotype']["$i"]['name'] : NULL,
            );
            
            $fields["$i"]['environment-check'] = array(
              '#type' => 'checkbox',
              '#title' => t('This is environmental data about the study'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment-check']) ? $values[$id]['phenotype']["$i"]['environment-check'] : NULL,
            );
            
            $fields["$i"]['environment'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][environment-check]"]' => array('checked' => TRUE)
                )
              )
            );
            
            $fields["$i"]['environment']['description'] = array(
              '#type' => 'textarea',
              '#title' => t("Phenotype $i Description:"),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment']['description']) ? $values[$id]['phenotype']["$i"]['environment']['description'] : NULL,
            );
            
            $fields["$i"]['environment']['units'] = array(
              '#type' => 'select',
              '#title' => t("Phenotype $i Units:"),
              '#options' => array(
                0 => '- Select -',
                1 => 'mm', 
                2 => 'cm',
                3 => 'm', 
                4 => 'Degrees Celsius',
                5 => 'Degrees Fahrenheit',
                6 => 'Other'
              ),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment']['units']) ? $values[$id]['phenotype']["$i"]['environment']['units'] : NULL,
            );
            
            $fields["$i"]['environment']['units-other'] = array(
              '#type' => 'textfield',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment']['units-other']) ? $values[$id]['phenotype']["$i"]['environment']['units-other'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][environment][units]' => array('value' => '6')
                )
              )
            );
            
            $fields["$i"]['non-environment'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][environment-check]"]' => array('checked' => FALSE)
                )
              )
            );
            
            $fields["$i"]['non-environment']['type'] = array(
              '#type' => 'select',
              '#title' => t("Phenotype $i Type:"),
              '#options' => array(
                0 => '- Select -',
                1 => 'Binary',
                2 => 'Quantitative',
                3 => 'Qualitative'
              ),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['type']) ? $values[$id]['phenotype']["$i"]['non-environment']['type'] : 0,
            );
            
            $fields["$i"]['non-environment']['binary'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][type]"]' => array('value' => '1')
                )
              )
            );
            
            $fields["$i"]['non-environment']['binary'][1] = array(
              '#type' => 'textfield',
              '#title' => t('Type 1:'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['binary'][1]) ? $values[$id]['phenotype']["$i"]['non-environment']['binary'][1] : NULL,
            );
            
            $fields["$i"]['non-environment']['binary'][2] = array(
              '#type' => 'textfield',
              '#title' => t('Type 2:'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['binary'][2]) ? $values[$id]['phenotype']["$i"]['non-environment']['binary'][2] : NULL,
            );
            
            $fields["$i"]['non-environment']['quantitative'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][type]"]' => array('value' => '2')
                )
              )
            );
            
            $fields["$i"]['non-environment']['quantitative']['min'] = array(
              '#type' => 'textfield',
              '#title' => t('Minimum'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['quantitative']['min']) ? $values[$id]['phenotype']["$i"]['non-environment']['quantitative']['min'] : NULL,
            );
            
            $fields["$i"]['non-environment']['quantitative']['max'] = array(
              '#type' => 'textfield',
              '#title' => t('Maximum'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['quantitative']['max']) ? $values[$id]['phenotype']["$i"]['non-environment']['quantitative']['max'] : NULL,
            );
            
            $fields["$i"]['non-environment']['description'] = array(
              '#type' => 'textarea',
              '#title' => t("Phenotype $i Description:"),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['description']) ? $values[$id]['phenotype']["$i"]['non-environment']['description'] : NULL,
            );
            
            $fields["$i"]['non-environment']['units'] = array(
              '#type' => 'select',
              '#title' => t("Phenotype $i Units:"),
              '#options' => array(
                0 => '- Select -',
                1 => 'mm', 
                2 => 'cm',
                3 => 'm', 
                4 => 'Degrees Celsius',
                5 => 'Degrees Fahrenheit',
                6 => 'Other'
              ),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['units']) ? $values[$id]['phenotype']["$i"]['non-environment']['units'] : NULL,
            );
            
            $fields["$i"]['non-environment']['units-other'] = array(
              '#type' => 'textfield',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['units-other']) ? $values[$id]['phenotype']["$i"]['non-environment']['units-other'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][units]' => array('value' => '6')
                )
              )
            );
            
            $fields["$i"]['non-environment']['structure'] = array(
              '#type' => 'textfield',
              '#title' => t('Plant Structure:'),
              '#autocomplete_path' => 'structure/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['structure']) ? $values[$id]['phenotype']["$i"]['non-environment']['structure'] : NULL,
            );
            
            $fields["$i"]['non-environment']['structure-check'] = array(
              '#type' => 'checkbox',
              '#title' => t('None of the autocomplete terms meet my needs'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['structure-check']) ? $values[$id]['phenotype']["$i"]['non-environment']['structure-check'] : NULL,
            );
            
            $fields["$i"]['non-environment']['structure-definition'] = array(
              '#type' => 'textfield',
              '#title' => t('Structure Term Definition:'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['structure-definition']) ? $values[$id]['phenotype']["$i"]['non-environment']['structure-definition'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][structure-check]"]' => array('checked' => TRUE)
                )
              )
            );
            
            $fields["$i"]['non-environment']['developmental'] = array(
              '#type' => 'textfield',
              '#title' => t('Plant Developmental Stage:'),
              '#autocomplete_path' => 'developmental/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['developmental']) ? $values[$id]['phenotype']["$i"]['non-environment']['developmental'] : NULL,
            );
            
            $fields["$i"]['non-environment']['developmental-check'] = array(
              '#type' => 'checkbox',
              '#title' => t('None of the autocomplete terms meet my needs'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['developmental-check']) ? $values[$id]['phenotype']["$i"]['non-environment']['developmental-check'] : NULL,
            );
            
            $fields["$i"]['non-environment']['developmental-definition'] = array(
              '#type' => 'textfield',
              '#title' => t('Developmental Stage Term Definition:'),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['developmental-definition']) ? $values[$id]['phenotype']["$i"]['non-environment']['developmental-definition'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][developmental-check]"]' => array('checked' => TRUE)
                )
              )
            );
        }
        
        $fields['check'] = array(
          '#type' => 'checkbox',
          '#title' => t('I have >20 Phenotypes'),
          '#default_value' => isset($values[$id]['phenotype']['check']) ? $values[$id]['phenotype']['check'] : NULL,
        );
        
        $fields['file'] = array(
          '#type' => 'managed_file',
          '#title' => t('Please upload a file containing information about all of your phenotypes'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('csv tsv xlsx')
          ),
          '#default_value' => isset($values[$id]['phenotype']['file']) ? $values[$id]['phenotype']['file'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
            )
          )
        );
        
        return $fields;
    }
    
    function genotype(&$form, $values, $id){
        
        $fields = array(
          '#type' => 'fieldset',
          '#title' => t('Genotype Information:')
        );
        
        $fields['BioProject-id'] = array(
          '#type' => 'textfield',
          '#title' => t('BioProject Accession Number:'),
          '#default_value' => isset($values[$id]['genotype']['BioProject-id']) ? $values[$id]['genotype']['BioProject-id'] : NULL,
          '#ajax' => array(
            'callback' => 'ajax_bioproject_callback',
            'wrapper' => "$id-assembly-auto",
          ),
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $id . '[genotype][assembly-check]"]' => array('checked' => TRUE)
            )
          )
        );
        
        $fields['assembly-user'] = array(
          '#type' => 'managed_file',
          '#title' => t('Assembly Files: (WGS/TSA)'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('fsa_nt')
          ),
          '#default_value' => isset($values[$id]['genotype']['assembly-user']) ? $values[$id]['genotype']['assembly-user'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[genotype][assembly-check]"]' => array('checked' => TRUE)
            )
          )
        );
        
        $fields['assembly-auto'] = array(
          '#type' => 'checkboxes',
          '#title' => t('Waiting for BioProject accession number...'),
          '#options' => array(),
          '#prefix' => "<div id='$id-assembly-auto'>",
          '#suffix' => '</div>',
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $id . '[genotype][assembly-check]"]' => array('checked' => TRUE)
            )
          )
        );
        
        $fields['assembly-check'] = array(
          '#type' => 'checkbox',
          '#title' => t('My assembly file is not in this list'),
          '#default_value' => isset($values[$id]['genotype']['assembly-check']) ? $values[$id]['genotype']['assembly-check'] : NULL,
        );
        
        $fields['SNPs'] = array(
          '#type' => 'managed_file',
          '#title' => t('SNP Files:'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('vcf')
          ),
          '#default_value' => isset($values[$id]['genotype']['SNPs']) ? $values[$id]['genotype']['SNPs'] : NULL,
        );
        
        return $fields;
    }
    
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    
    for ($i = 1; $i <= $organism_number; $i++){
        
        $name = $form_state['saved_values']['Hellopage']['organism']["$i"]['species'];
        
        $form["organism-$i"] = array(
          '#type' => 'fieldset',
          '#title' => t($name . ":"),
          '#tree' => TRUE,
        );
        
        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
            $form["organism-$i"]['phenotype'] = phenotype($form, $values, "organism-$i");
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
            $form["organism-$i"]['genotype'] = genotype($form, $values, "organism-$i");
        }
    }
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
    );
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit')
    );

    drupal_add_js(drupal_get_path('module', 'custom_module') . "/custom_module.js");
    
    return $form;
}

function ajax_bioproject_callback(&$form, $form_state){
    
    $id = $form_state['triggering_element']['#parents'][0];
    $bio_id = substr($form_state['values']["$id"]['genotype']['BioProject-id'], 5);
    
    $options = array();
    $url = "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?dbfrom=bioproject&db=nuccore&id=" . $bio_id;
    $response_xml_data = file_get_contents($url);
    $data = simplexml_load_string($response_xml_data)->children()->children()->LinkSetDb->children();

    foreach ($data->Link as $link){
        array_push($options, $link->Id->__tostring());
    }

    $form["$id"]['genotype']['assembly-auto'] = array(
      '#type' => 'fieldset',
      '#title' => 'Select all that apply:',
      '#tree' => TRUE,
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $id . '[genotype][assembly-check]"]' => array('checked' => TRUE)
        )
      ),
      '#prefix' => "<div id='$id-assembly-auto'>",
      '#suffix' => '</div>',
      '#description' => 'If this list needs to be refreshed, please refresh the page and re-enter the BioProject ID.'
    );

    foreach ($options as $item){
        $form["$id"]['genotype']['assembly-auto']["$item"] = array(
          '#type' => 'checkbox',
          '#title' => t("$item"),
          '#default_value' => isset($form_state['saved_values']['fourthPage']["$id"]['genotype']['assembly-auto']["$item"]) ? $form_state['saved_values']['fourthPage']["$id"]['genotype']['assembly-auto']["$item"] : NULL,
        );
    }
    
    return $form["$id"]['genotype']['assembly-auto'];
    
}

function page_4_validate_form(&$form, &$form_state){
    
    function validate_phenotype($phenotype, $id){
        $phenotype_number = $phenotype['number'];
        $phenotype_check = $phenotype['check'];
        $phenotype_file = $phenotype['file'];
        
        if ($phenotype_check == '1'){
            if ($phenotype_file == ''){
                form_set_error("$id][file", "Phenotype File: field is required");
            }
            else{
                //validate phenotype file
                $file = file(file_load($phenotype_file)->uri);
                $file_type = file_load($phenotype_file)->filemime;
            }
        }
        else{
            for($i = 1; $i <= $phenotype_number; $i++){
                $current_phenotype = $phenotype[$i];
                $name = $current_phenotype['name'];
                $environment_check = $current_phenotype['environment-check'];

                if ($name == ''){
                    form_set_error("$id][$i][name", "Phenotype $i Name: field is required.");
                }

                if ($environment_check == '1'){
                    $description = $current_phenotype['environment']['description'];
                    $units = $current_phenotype['environment']['units'];

                    if ($description == ''){
                        form_set_error("$id][$i][environment][description", "Phenotype $i Description: field is required.");
                    }

                    if ($units == ''){
                        form_set_error("$id][$i][environment][units", "Phenotype $i Units: field is required.");
                    }
                }
                else{
                    $type = $current_phenotype['non-environment']['type'];
                    $binary_1 = $current_phenotype['non-environment']['binary'][1];
                    $binary_2 = $current_phenotype['non-environment']['binary'][2];
                    $min = $current_phenotype['non-environment']['quantitative']['min'];
                    $max = $current_phenotype['non-environment']['quantitative']['max'];
                    $description = $current_phenotype['non-environment']['description'];
                    $units = $current_phenotype['non-environment']['units'];
                    $structure = $current_phenotype['non-environment']['structure'];
                    $structure_check = $current_phenotype['non-environment']['structure-check'];
                    $structure_definition = $current_phenotype['non-environment']['structure-definition'];
                    $developmental = $current_phenotype['non-environment']['developmental'];
                    $developmental_check = $current_phenotype['non-environment']['developmental-check'];
                    $developmental_definition = $current_phenotype['non-environment']['developmental-definition'];

                    if ($type == '0'){
                        form_set_error("$id][$i][non-environment][type", "Phenotype $i Type: field is required.");
                    }
                    elseif ($type == '1'){
                        if ($binary_1 == ''){
                            form_set_error("$id][$i][non-environment][binary][1", "Phenotype $i Binary Type 1: field is required.");
                        }
                        if ($binary_2 == ''){
                            form_set_error("$id][$i][non-environment][binary][2", "Phenotype $i Binary Type 2: field is required.");
                        }
                    }
                    elseif($type == '2'){
                        if ($min == ''){
                            form_set_error("$id][$i][non-environment][quantitative][min", "Phenotype $i Minimum: field is required.");
                        }
                        if ($max == ''){
                            form_set_error("$id][$i][non-environment][quantitative][max", "Phenotype $i Maximum: field is required.");
                        }
                    }

                    if ($description == ''){
                        form_set_error("$id][$i][non-environment][description", "Phenotype $i Description: field is required.");
                    }

                    if ($units == ''){
                        form_set_error("$id][$i][non-environment][units", "Phenotype $i Units: field is required.");
                    }

                    if ($structure == ''){
                        form_set_error("$id][$i][non-environment][structure", "Phenotype $i Plant Structure: field is required.");
                    }
                    else{
                        $indexed_structure = db_select('chado.phenotype_structure_cvterm', 'phenotype_structure_cvterm')
                            ->fields('phenotype_structure_cvterm', array('name'))
                            ->execute();

                        $custom_structure = TRUE;

                        foreach($indexed_structure as $item){
                            if ($item == $structure){
                                $custom_structure = FALSE;
                                break;
                            }
                        }

                        if ($structure_check == '1' or ($custom_structure)){
                            if ($structure_definition == ''){
                                form_set_error("$id][$i][non-environment][structure-definition", "Phenotype $i Plant Structure Definition: field is required.");
                            }
                        }
                    }

                    if ($developmental == ''){
                        form_set_error("$id][$i][non-environment][developmental", "Phenotype $i Developmental Stage: field is required.");
                    }
                    else{
                        $indexed_developmental = db_select('chado.phenotype_cvterm', 'phenotype_cvterm')
                            ->fields('phenotype_cvterm', array('name'))
                            ->execute();

                        $custom_developmental = TRUE;

                        foreach($indexed_developmental as $item){
                            if ($item == $developmental){
                                $custom_developmental = FALSE;
                                break;
                            }
                        }

                        if ($developmental_check == '1' or ($custom_developmental)){
                            if ($developmental_definition == ''){
                                form_set_error("$id][$i][non-environment][developmental-definition", "Phenotype $i Developmental Stage Definition: field is required.");
                            }
                        }
                    }
                }
            }
        }
    }
    
    function validate_genotype($genotype, $id){
        $assembly = $genotype['assembly'];
        
        if ($assembly == ''){
            form_set_error("$id][assembly", "Assembly File: field is required.");
        }
        else{
            /*$file = file(file_load($assembly)->uri);
            $headers_assembly = array();
            
            foreach($file as $line){
                if ($line[0] == '>'){
                    $items = explode('|', $line);
                    array_push($headers_assembly, $items[1]);
                }
            }
            
            print_r($headers_assembly);*/
        }
        
    }
    
    $form_values = $form_state['values'];
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    
    for ($i = 1; $i <= $organism_number; $i++){
        $organism = $form_values["organism-$i"];
        
        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
            $phenotype = $organism['phenotype'];
            validate_phenotype($phenotype, "organism-$i][phenotype");
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
            $genotype = $organism['genotype'];
            //validate_genotype($genotype, "organism-$i][phenotype");
        }
    }
    
    /*if (empty(form_get_errors())){
        form_set_error('submit', 'validation success');
    }*/
}

function page_4_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'fourthPage';
}
