<?php
function page_3_create_form(&$form, $form_state){
    
    if (isset($form_state['saved_values']['thirdPage'])){
        $values = $form_state['saved_values']['thirdPage'];
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
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Units:"),
              '#autocomplete_path' => 'units/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['environment']['units']) ? $values[$id]['phenotype']["$i"]['environment']['units'] : NULL,
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
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Units:"),
              '#autocomplete_path' => 'units/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['units']) ? $values[$id]['phenotype']["$i"]['non-environment']['units'] : NULL,
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
          '#title' => t('BioProject ID:'),
          '#default_value' => isset($values[$id]['genotype']['BioProject-id']) ? $values[$id]['genotype']['BioProject-id'] : NULL,
          /*'#ajax' => array(
            'event' => 'focusout',
            'callback' => 'get_assembly_GI_number',
          ),*/
        );
        
        $fields['assembly'] = array(
          '#type' => 'managed_file',
          '#title' => t('Assembly Files: (WGS/TSA)'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('fsa_nt')
          ),
          '#default_value' => isset($values[$id]['genotype']['assembly']) ? $values[$id]['genotype']['assembly'] : NULL,
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
    
    $form['tree-accession'] = array(
      '#type' => 'managed_file',
      '#title' => t("Please provide a file with information regarding the accession of the trees used in this study:"),
      '#upload_location' => 'public://',
      '#upload_validators' => array(
        'file_validate_extensions' => array('tsv csv xlsx'),
      ),
      '#default_value' => isset($values['tree-accession']) ? $values['tree-accession'] : NULL,
      '#description' => 'Columns with information describing the Identifier of the tree and the location of the tree are required.'
    );

    $form['tree-accession-columns'] = array(
      '#type' => 'textfield',
      '#title' => t('Please provide the order of the columns in the file above, separated by columns.'),
      '#default_value' => isset($values['tree-accession-columns']) ? $values['tree-accession-columns'] : NULL
    );
    
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

function page_3_validate_form(&$form, &$form_state){
    
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
    
    function validate_accession($accession, $provided_columns){
        $file = file(file_load($accession)->uri);
        $file_type = file_load($accession)->filemime;
        //$file = explode("\r", $file[0]);
        
        if ($file_type == 'text/csv'){
            $columns = explode("\r", $file[0]);
            $columns = explode(",", $columns[0]);
            $provided_columns = explode(",", $provided_columns);
            $id_omitted = TRUE;
            $location_omitted = TRUE;
            
            foreach($columns as $key => $col){
                $columns[$key] = trim($col);
                if (preg_match('/^(id|ID|Id|Identifier|identifier|IDENTIFIER)$/', $columns[$key]) == 1){
                    $id_omitted = FALSE;
                }
                elseif (preg_match('/^(location|Location|LOCATION)$/', $columns[$key]) == 1){
                    $location_omitted = FALSE;
                }
            }
            
            foreach($provided_columns as $key => $col){
                $provided_columns[$key] = trim($col);
            }
            
            if (array_diff($columns, $provided_columns) == array()){
                if ($id_omitted){
                    form_set_error("tree-accession", 'Tree Accession file: We were unable to find your "Identifier" column. Please resubmit your file with a column named "Identifier", with an identifier for each tree.');
                }
                if ($location_omitted){
                    form_set_error("tree-accession", 'Tree Accession file: We were unable to find your "Location" column. Please resubmit your file with a column named "Location", with the location of each tree.');
                }
            }
            else{
                form_set_error("tree-accession-columns", 'Tree Accession Columns: provided columns do not match file.');
            }
            
        }
        elseif ($file_type == 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'){
            $location = '/var/www/Drupal/sites/default/files/' . file_load($accession)->filename;
            
            $content = parse_xlsx($location);
            $columns = $content['headers'];
            $provided_columns = explode(",", $provided_columns);
            $id_omitted = TRUE;
            $location_omitted = TRUE;
            
            foreach($columns as $key => $col){
                $columns[$key] = trim($col);
                if (preg_match('/^(id|ID|Id|Identifier|identifier|IDENTIFIER)$/', $columns[$key]) == 1){
                    $id_omitted = FALSE;
                }
                elseif (preg_match('/^(location|Location|LOCATION)$/', $columns[$key]) == 1){
                    $location_omitted = FALSE;
                }
            }
            
            foreach($provided_columns as $key => $col){
                $provided_columns[$key] = trim($col);
            }
            
            if (array_diff($columns, $provided_columns) == array()){
                if ($id_omitted){
                    form_set_error("tree-accession", 'Tree Accession file: We were unable to find your "Identifier" column. Please resubmit your file with a column named "Identifier", with an identifier for each tree.');
                }
                if ($location_omitted){
                    form_set_error("tree-accession", 'Tree Accession file: We were unable to find your "Location" column. Please resubmit your file with a column named "Location", with the location of each tree.');
                }
            }
            else{
                form_set_error("tree-accession-columns", 'Tree Accession Columns: provided columns do not match file.');
            }
        }
        elseif ($file_type == 'text/plain'){
            $columns = explode("\r", $file[0]);
            $columns = explode("\t", $columns[0]);
            $provided_columns = explode(",", $provided_columns);
            $id_omitted = TRUE;
            $location_omitted = TRUE;
            
            foreach($columns as $key => $col){
                $columns[$key] = trim($col);
                if (preg_match('/^(id|ID|Id|Identifier|identifier|IDENTIFIER)$/', $columns[$key]) == 1){
                    $id_omitted = FALSE;
                }
                elseif (preg_match('/^(location|Location|LOCATION)$/', $columns[$key]) == 1){
                    $location_omitted = FALSE;
                }
            }
            
            foreach($provided_columns as $key => $col){
                $provided_columns[$key] = trim($col);
            }
            
            if (array_diff($columns, $provided_columns) == array()){
                if ($id_omitted){
                    form_set_error("tree-accession", 'Tree Accession file: We were unable to find your "Identifier" column. Please resubmit your file with a column named "Identifier", with an identifier for each tree.');
                }
                if ($location_omitted){
                    form_set_error("tree-accession", 'Tree Accession file: We were unable to find your "Location" column. Please resubmit your file with a column named "Location", with the location of each tree.');
                }
            }
            else{
                form_set_error("tree-accession-columns", 'Tree Accession Columns: provided columns do not match file.');
            }
            
        }
    }
    
    $form_values = $form_state['values'];
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    $tree_accession = $form_values['tree-accession'];
    $tree_accession_columns = $form_values['tree-accession-columns'];
    
    if ($tree_accession == ''){
        form_set_error("tree-accession", 'Tree Accesison File: field is required.');
    }
    else{
        validate_accession($tree_accession, $tree_accession_columns);
    }
    
    for ($i = 1; $i <= $organism_number; $i++){
        $organism = $form_values["organism-$i"];
        
        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
            $phenotype = $organism['phenotype'];
            validate_phenotype($phenotype, "organism-$i][phenotype");
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
            $genotype = $organism['genotype'];
            validate_genotype($genotype, "organism-$i][phenotype");
        }
    }
    
    /*if (empty(form_get_errors())){
        form_set_error('submit', 'validation success');
    }*/
}

function page_3_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'thirdPage';
}
