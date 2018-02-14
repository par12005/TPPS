<?php
function page_3_create_form(&$form, $form_state){
    
    if (isset($form_state['saved_values']['thirdPage'])){
        $values = $form_state['saved_values']['thirdPage'];
    }
    else{
        $values = array();
    }
    
    $form['tree-accession'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );
    
    $file_description = 'Columns with information describing the Identifier of the tree and the location of the tree are required.';
    $species_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    
    if ($form_state['saved_values']['secondPage']['studyType'] == '4'){
        $file_description .= ' Location columns should describe the location of the source tree for the Common Garden.';
    }
    
    $form['tree-accession']['file'] = array(
      '#type' => 'managed_file',
      '#title' => t("Please provide a file with information regarding the accession of the trees used in this study:"),
      '#upload_location' => 'public://',
      '#upload_validators' => array(
        'file_validate_extensions' => array('txt csv xlsx'),
      ),
      '#default_value' => isset($values['tree-accession']['file']) ? $values['tree-accession']['file'] : NULL,
      '#description' => $file_description,
      '#states' => ($species_number > 1) ? (array(
        'visible' => array(
          ':input[name="tree-accession[check]"]' => array('checked' => FALSE),
        )
      )) : NULL,
    );

    $form['tree-accession']['columns'] = array(
      '#type' => 'textfield',
      '#title' => t('Please provide the order of the columns in the file above, separated by commas.'),
      '#default_value' => isset($values['tree-accession']['columns']) ? $values['tree-accession']['columns'] : NULL,
      '#states' => ($species_number > 1) ? (array(
        'visible' => array(
          ':input[name="tree-accession[check]"]' => array('checked' => FALSE),
        )
      )) : NULL,
    );
    
    if ($species_number > 1){
        $form['tree-accession']['check'] = array(
          '#type' => 'checkbox',
          '#title' => t('I would like to upload a separate tree accession file for each species.'),
          '#default_value' => isset($values['tree-accession']['check']) ? $values['tree-accession']['check'] : NULL,
        );

        for ($i = 1; $i <= $species_number; $i++){
            $name = $form_state['saved_values']['Hellopage']['organism']["$species_number"]['species'];
            
            $form['tree-accession']["species-$i"] = array(
              '#type' => 'fieldset',
              '#title' => t("Tree Accession information for $name trees:"),
              '#states' => array(
                'visible' => array(
                  ':input[name="tree-accession[check]"]' => array('checked' => TRUE),
                )
              )
            );
            
            $form['tree-accession']["species-$i"]['file'] = array(
              '#type' => 'managed_file',
              '#title' => t("Please provide a file with information regarding the accession of the $name trees used in this study:"),
              '#upload_location' => 'public://',
              '#upload_validators' => array(
                'file_validate_extensions' => array('txt csv xlsx'),
              ),
              '#default_value' => isset($values['tree-accession']["speices-$i-file"]) ? $values['tree-accession']["speices-$i-file"] : NULL,
              '#description' => $file_description
            );
            
            $form['tree-accession']["species-$i"]['columns'] = array(
              '#type' => 'textfield',
              '#title' => t('Please provide the order of the columns in the file above, separated by columns.'),
              '#default_value' => isset($values['tree-accession']["species-$i"]['columns']) ? $values['tree-accession']["species-$i"]['columns'] : NULL,
              
            );
        }
    }
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
    );
    
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );
    
    return $form;
}

function page_3_validate_form(&$form, &$form_state){
    
    /*function validate_accession($accession, $provided_columns){
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
    $tree_accession = $form_values['tree-accession'];
    $tree_accession_columns = $form_values['tree-accession-columns'];
    
    if ($tree_accession == ''){
        form_set_error("tree-accession", 'Tree Accesison File: field is required.');
    }
    else{
        validate_accession($tree_accession, $tree_accession_columns);
    }*/
    
}

function page_3_submit_form(&$form, &$form_state){
    
}