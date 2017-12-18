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
              '#title' => t('Description'),
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
              '#title' => t('Phenotype Type:'),
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
          '#type' => 'file',
          '#title' => t('Please upload a file containing information about all of your phenotypes'),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
            )
          )
        );
        
        return $fields;
    }
    
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    
    for ($i = 1; $i <= $organism_number; $i++){
        
        if (!isset($form_state['saved_values']['Hellopage']['organism']["$i"]['species']['other']['check'])){
            $name = $form_state['saved_values']['Hellopage']['organism']["$i"]['genus'] . ' ' . $form_state['saved_values']['Hellopage']['organism']["$i"]['species'];
        }
        else{
            $name = $form_state['saved_values']['Hellopage']['organism']["$i"]['species']['other']['textGenus'] . ' ' . $form_state['saved_values']['Hellopage']['organism']["$i"]['species']['other']['textSpecies'];
        }
        
        $form["organism-$i"] = array(
          '#type' => 'fieldset',
          '#title' => t($name . ":"),
          '#tree' => TRUE,
        );
        
        $form["organism-$i"]['tree-accession'] = array(
          '#type' => 'managed_file',
          '#title' => t("Please provide a file with information regarding the accession of the $name trees used in this study:"),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('txt csv vcf'),
          ),
          '#default_value' => isset($values["organism-$i"]['tree-accession']) ? $values["organism-$i"]['tree-accession'] : NULL
        );
        
        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
            $form["organism-$i"]['phenotype'] = phenotype($form, $values, "organism-$i");
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
            $form["organism-$i"]['genotype'] = array(
              '#type' => 'fieldset',
              '#title' => t('Genotype Information:')
            );
            
            $form["organism-$i"]['genotype']['name'] = array(
              '#type' => 'textfield',
              '#title' => t('Genotype name'),
            );
        }
    }
    
    /*
    
    function genotype(&$form, $values){
        
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
          '#title' => t('Define Genotyping Design:'),
          '#options' => array(
            0 => '- Select -',
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
            0 => '- Select -',
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
            0 => '- Select -',
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
            0 => '- Select -',
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
    
    tree_access($form, $values);
    
    phenotype($form, $values);
    
    genotype($form, $values);*/
    
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
    
    //form_set_error('submit', 'validation error');
    
    function validate_phenotype($phenotype){
        $phenotype_number = $phenotype['number'];
        
        print_r($phenotype_number);
    }
    
    function validate_genotype($genotype){
        
    }
    
    function accession_validate_csv($organism, $id){
        $tree_accession_file = file(file_load($organism['tree-accession'])->uri);
        $tree_accession_file = explode("\r", $tree_accession_file[0]);
        $columns = $tree_accession_file[0];
        
        if (!preg_match('/(i|I)dentifier/', $columns)){
            form_set_error($id . "][tree-accession", "Tree Accession Identifier: Column is required in tree accession file.");
        }
        
        if (!preg_match('/(l|L)ocation/', $columns)){
            form_set_error($id . "][tree-accession", "Tree Accession Location: Column is required in tree accession file.");
        }
    }
    
    function accession_validate_vcf($organism, $id){
        $tree_accession_file = file(file_load($organism['tree-accession'])->uri);
        $first_line = $tree_accession_file[0];
        if ($first_line != "##fileformat=VCFv4.0\n"){
            form_set_error($id . "][tree-accession", "File upload: file is not correct vcf format.");
        }
    }
    
    $form_values = $form_state['values'];
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    
    for ($i = 1; $i <= $organism_number; $i++){
        $organism = $form_values["organism-$i"];
        $file_type = file_load($organism['tree-accession'])->filemime;
        
        if ($file_type == 'text/csv'){
            accession_validate_csv($organism, "organism-$i");
        }
        elseif($file_type == 'text/x-vcard'){
            accession_validate_vcf($organism, "organism-$i");
        }
        
        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
            $phenotype = $organism['phenotype'];
            validate_phenotype($phenotype);
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
            $genotype = $organism['genotype'];
            validate_genotype($genotype);
        }
    }
    
    if (empty(form_get_errors())){
        form_set_error('submit', 'validation success');
    }
}

function page_3_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'thirdPage';
}
