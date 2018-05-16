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
      '#tree' => TRUE
    );
	
    $form['tree-accession']['file']['columns'] = array(
      '#type' => 'fieldset',
      '#title' => t('Columns'),
      '#states' => array(
        'invisible' => array(
          ':input[name="tree-accession_file_upload_button"]' => array('value' => 'Upload')
        )
      )
    );
    
    $file = 0;
    if (isset($form_state['values']['tree-accession']['file']) and $form_state['values']['tree-accession']['file'] != 0){
        $file = $form_state['values']['tree-accession']['file'];
    }
    elseif (isset($form_state['saved_values']['thirdPage']['tree-accession']['file']) and $form_state['saved_values']['thirdPage']['tree-accession']['file'] != 0){
        $file = $form_state['saved_values']['thirdPage']['tree-accession']['file'];
    }
    
    if ($file != 0){
        $file = file_load($file);
        $file_name = explode('//', $file->uri);
        $file_name = $file_name[1];

        //vm
        //$location = "/var/www/html/Drupal/sites/default/files/$file_name";
        //dev site
        $location = "/var/www/Drupal/sites/default/files/$file_name";
        $content = parse_xlsx($location);

        $required_columns = array(
          'Tree Identifier',
          'Country',
          'Region',
          'Latitude',
          'Longitude'
        );

        $options_arr = $content['headers'];
        $options_arr['- Select -'] = '- Select -';

        foreach ($required_columns as $req){
            $form['tree-accession']['file']['columns'][$req] = array(
              '#type' => 'select',
              '#title' => t($req),
              '#options' => $options_arr,
              '#default_value' => isset($values['tree-accession']['file-columns'][$req]) ? $values['tree-accession']['file-columns'][$req] : '- Select -',
            );
        }

        // display sample data
        $display = "";
        $display .= "<div><table><tbody>";
        $display .= "<tr>";
        foreach ($content['headers'] as $item){
            $display .= "<th>$item</th>";
        }
        $display .= "</tr>";
        for ($i = 0; $i < 3; $i++){
            if (isset($content[$i])){
                $display .= "<tr>";
                foreach ($content['headers'] as $item){
                    $display .= "<th>{$content[$i][$item]}</th>";
                }
                $display .= "</tr>";
            }
        }
        $display .= "</tbody></table></div>";

        $form['tree-accession']['file']['columns']['#suffix'] = $display;

    }
    
    if ($species_number > 1){
        $form['tree-accession']['check'] = array(
          '#type' => 'checkbox',
          '#title' => t('I would like to upload a separate tree accession file for each species.'),
          '#default_value' => isset($values['tree-accession']['check']) ? $values['tree-accession']['check'] : NULL,
        );

        for ($i = 1; $i <= $species_number; $i++){
            $name = $form_state['saved_values']['Hellopage']['organism']["$i"]['species'];
            
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
              '#default_value' => isset($values['tree-accession']["species-$i"]['file']) ? $values['tree-accession']["species-$i"]['file'] : NULL,
              '#description' => $file_description,
              '#tree' => TRUE
            );
            
            $form['tree-accession']["species-$i"]['file']['columns'] = array(
              '#type' => 'fieldset',
              '#title' => t('Columns'),
              '#states' => array(
                'invisible' => array(
                  ':input[name="tree-accession_species-' . $i . '_file_upload_button"]' => array('value' => 'Upload')
                )
              )
            );

            $file = 0;
            if (isset($form_state['values']['tree-accession']["species-$i"]['file']) and $form_state['values']['tree-accession']["species-$i"]['file'] != 0){
                $file = $form_state['values']['tree-accession']["species-$i"]['file'];
            }
            elseif (isset($form_state['saved_values']['thirdPage']['tree-accession']["species-$i"]['file']) and $form_state['saved_values']['thirdPage']['tree-accession']["species-$i"]['file'] != 0){
                $file = $form_state['saved_values']['thirdPage']['tree-accession']["species-$i"]['file'];
            }
            
            if ($file != 0){
                $file = file_load($file);
                $file_name = explode('//', $file->uri);
                $file_name = $file_name[1];

                //vm
                //$location = "/var/www/html/Drupal/sites/default/files/$file_name";
                //dev site
                $location = "/var/www/Drupal/sites/default/files/$file_name";
                $content = parse_xlsx($location);

                $required_columns = array(
                  'Tree Identifier',
                  'Country',
                  'Region',
                  'Latitude',
                  'Longitude'
                );

                $options_arr = $content['headers'];
                $options_arr['- Select -'] = '- Select -';

                foreach ($required_columns as $req){
                    $form['tree-accession']["species-$i"]['file']['columns'][$req] = array(
                      '#type' => 'select',
                      '#title' => t($req),
                      '#options' => $options_arr,
                      '#default_value' => isset($values['tree-accession']["species-$i"]['file-columns'][$req]) ? $values['tree-accession']["species-$i"]['file-columns'][$req] : '- Select -',
                    );
                }

                // display sample data
                $display = "";
                $display .= "<div><table><tbody>";
                $display .= "<tr>";
                foreach ($content['headers'] as $item){
                    $display .= "<th>$item</th>";
                }
                $display .= "</tr>";
                for ($j = 0; $j < 3; $j++){
                    if (isset($content[$j])){
                        $display .= "<tr>";
                        foreach ($content['headers'] as $item){
                            $display .= "<th>{$content[$j][$item]}</th>";
                        }
                        $display .= "</tr>";
                    }
                }
                $display .= "</tbody></table></div>";

                $form['tree-accession']["species-$i"]['file']['columns']['#suffix'] = $display;

            }	
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
    if ($form_state['submitted'] == '1'){
        
        $species_number = $form_state['saved_values']['Hellopage']['organism']['number'];
        if ($species_number == 1 or $form_state['values']['tree-accession']['check'] == '0'){
            if ($form_state['values']['tree-accession']['file'] != ""){

                $required_columns = array(
                  'Tree Identifier',
                  'Country',
                  'Region',
                  'Latitude',
                  'Longitude'
                );

                $form_state['values']['tree-accession']['file-columns'] = array();

                foreach ($required_columns as $req){
                    $form_state['values']['tree-accession']['file-columns'][$req] = $form['tree-accession']['file']['columns'][$req]['#value'];

                    $col_val = $form_state['values']['tree-accession']['file-columns'][$req];
                    if ($col_val == '- Select -'){
                        form_set_error("tree-accession][file][columns][$req", "$req: please select the appropriate column.");
                    }
                }
            }
            else{
                form_set_error('tree-accession][file', 'Tree Accession file: field is required.');
            }
        }
        else {
            
            $required_columns = array(
              'Tree Identifier',
              'Country',
              'Region',
              'Latitude',
              'Longitude'
            );

            for ($i = 1; $i <= $species_number; $i++){
                if ($form_state['values']['tree-accession']["species-$i"]['file'] != ""){

                    $form_state['values']['tree-accession']["species-$i"]['file-columns'] = array();

                    foreach ($required_columns as $req){
                        $form_state['values']['tree-accession']["species-$i"]['file-columns'][$req] = $form['tree-accession']["species-$i"]['file']['columns'][$req]['#value'];

                        $col_val = $form_state['values']['tree-accession']["species-$i"]['file-columns'][$req];
                        if ($col_val == '- Select -'){
                            form_set_error("tree-accession][species-$i][file][columns][$req", "$req: please select the appropriate column.");
                        }
                    }
                }
                else{
                    form_set_error("tree-accession][species-$i][file", "Species $i Tree Accession file: field is required.");
                }
            }
        }
    }
    
}

function page_3_submit_form(&$form, &$form_state){
    
}