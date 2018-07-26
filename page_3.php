<?php
function page_3_create_form(&$form, &$form_state){
    
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
    $file_upload_location = 'public://' . variable_get('tpps_accession_files_dir', 'tpps_accession');
    
    if ($form_state['saved_values']['secondPage']['studyType'] == '4'){
        $file_description .= ' Location columns should describe the location of the source tree for the Common Garden.';
    }
    
    $form['tree-accession']['file'] = array(
      '#type' => 'managed_file',
      '#title' => t("Tree Accession File: please provide a spreadsheet with columns for the Tree ID and location of trees used in this study: *"),
      '#upload_location' => "$file_upload_location",
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
    
    $form['tree-accession']['no-header'] = array(
      '#type' => 'checkbox',
      '#title' => t('My file has no header row'),
      '#default_value' => isset($values['tree-accession']['no-header']) ? $values['tree-accession']['no-header'] : NULL,
      '#ajax' => array(
        'wrapper' => 'header-wrapper',
        'callback' => 'accession_header_callback',
      ),
      '#states' => ($species_number > 1) ? (array(
        'visible' => array(
          ':input[name="tree-accession[check]"]' => array('checked' => FALSE),
        )
      )) : NULL,
    );
    
    $form['tree-accession']['file']['columns'] = array(
      '#type' => 'fieldset',
      '#title' => t('<div class="fieldset-title">Define Data</div>'),
      '#states' => array(
        'invisible' => array(
          ':input[name="tree-accession_file_upload_button"]' => array('value' => 'Upload')
        )
      ),
      '#description' => 'Please define which columns hold the required data: Tree Identifier and Location',
      '#prefix' => '<div id="header-wrapper">',
      '#suffix' => '</div>',
      '#collapsible' => TRUE
    );
    
    $file = 0;
    if (isset($form_state['values']['tree-accession']['file']) and $form_state['values']['tree-accession']['file'] != 0){
        $file = $form_state['values']['tree-accession']['file'];
    }
    elseif (isset($form_state['saved_values']['thirdPage']['tree-accession']['file']) and $form_state['saved_values']['thirdPage']['tree-accession']['file'] != 0){
        $file = $form_state['saved_values']['thirdPage']['tree-accession']['file'];
    }
    
    //dpm($form_state);
    if ($file != 0){
        if (($file = file_load($file))){
            $file_name = $file->uri;
            
            //stop using the file so it can be deleted if the user clicks 'remove'
            file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
            
            $location = drupal_realpath("$file_name");
            $content = parse_xlsx($location);
            $no_header = FALSE;
            
            if (isset($form_state['complete form']['tree-accession']['no-header']['#value']) and $form_state['complete form']['tree-accession']['no-header']['#value'] == 1){
                tpps_content_no_header($content);
                $no_header = TRUE;
            }
            elseif (!isset($form_state['complete form']['tree-accession']['no-header']['#value']) and isset($values['tree-accession']['no-header']) and $values['tree-accession']['no-header'] == 1){
                tpps_content_no_header($content);
                $no_header = TRUE;
            }

            $column_options = array(
              '0' => 'N/A',
              '1' => 'Tree Identifier',
              '2' => 'Country',
              '3' => 'State',
              '8' => 'County',
              '9' => 'District',
              '4' => 'Latitude',
              '5' => 'Longitude',
              '6' => 'Genus',
              '7' => 'Species',
              '10' => 'Genus + Species'
            );

            $first = TRUE;

            foreach ($content['headers'] as $item){
                $form['tree-accession']['file']['columns'][$item] = array(
                  '#type' => 'select',
                  '#title' => t($item),
                  '#options' => $column_options,
                  '#default_value' => isset($values['tree-accession']['file-columns'][$item]) ? $values['tree-accession']['file-columns'][$item] : 0,
                  '#prefix' => "<td>",
                  '#suffix' => "</td>",
                  '#attributes' => array(
                    'data-toggle' => array('tooltip'),
                    'data-placement' => array('left'),
                    'title' => array("Select the type of data the '$item' column holds")
                  )
                );

                if ($first){
                    $first = FALSE;
                    $form['tree-accession']['file']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $form['tree-accession']['file']['columns'][$item]['#prefix'];
                }
                
                if ($no_header){
                    $form['tree-accession']['file']['columns'][$item]['#title'] = '';
                    $form['tree-accession']['file']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
                }
            }

            // display sample data
            $display = "</tr>";
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

            $form['tree-accession']['file']['columns'][$item]['#suffix'] .= $display;
            
            $first = TRUE;
            $states_arr = array();
            
            foreach ($content['headers'] as $item){
                if (!$first){
                    $states_arr[] = 'or';
                }
                else {
                    $first = FALSE;
                }
                $states_arr[] = array(':input[name="tree-accession[file][columns][' . $item . ']"]' => array('value' => '4'));
                $states_arr[] = 'or';
                $states_arr[] = array(':input[name="tree-accession[file][columns][' . $item . ']"]' => array('value' => '5'));
            }
            
            $form['tree-accession']['file']['coord-format'] = array(
              '#type' => 'select',
              '#title' => t('Coordinate Projection'),
              '#options' => array(
                'WGS 84',
                'NAD 83',
                'ETRS 89',
              ),
              '#default_value' => isset($values['tree-accession']['file']['coord-format']) ? $values['tree-accession']['file']['coord-format'] : 0,
              '#states' => array(
                'visible' => array(
                  $states_arr
                )
              ),
            );
        }
    }
    
    $form['tree-accession']['map-button'] = array(
      '#type' => 'button',
      '#title' => 'Click here to update map',
      '#value' => 'Click here to update map',
      '#button_type' => 'button',
      '#executes_submit_callback' => FALSE,
      '#ajax' => array(
        'callback' => 'page_3_multi_map',
        'wrapper' => 'multi_map',
      ),
      '#prefix' => '<div id="multi_map">',
      '#suffix' => '<div id="map_wrapper"></div></div>',
    );
    
    $form['tree-accession']['map-button']['#suffix'] .= '
    <script src="https://maps.googleapis.com/maps/api/js?key=AIzaSyDkeQ6KN6HEBxrIoiSCrCHFhIbipycqouY&callback=initMap"
    async defer></script>
    <style>
      #map_wrapper {
        height: 450px;
      }
    </style>';
    
    
    if ($species_number > 1){
        $form['tree-accession']['check'] = array(
          '#type' => 'checkbox',
          '#title' => t('I would like to upload a separate tree accession file for each species.'),
          '#default_value' => isset($values['tree-accession']['check']) ? $values['tree-accession']['check'] : NULL,
        );

        for ($i = 1; $i <= $species_number; $i++){
            $name = $form_state['saved_values']['Hellopage']['organism']["$i"];
            
            $form['tree-accession']["species-$i"] = array(
              '#type' => 'fieldset',
              '#title' => t("<div class=\"fieldset-title\">Tree Accession information for $name trees:</div>"),
              '#states' => array(
                'visible' => array(
                  ':input[name="tree-accession[check]"]' => array('checked' => TRUE),
                )
              ),
              '#collapsible' => TRUE,
            );
            
            $form['tree-accession']["species-$i"]['file'] = array(
              '#type' => 'managed_file',
              '#title' => t("$name Accession File: please provide a spreadsheet with columns for the Tree ID and location of the $name trees used in this study: *"),
              '#upload_location' => "$file_upload_location",
              '#upload_validators' => array(
                'file_validate_extensions' => array('txt csv xlsx'),
              ),
              '#default_value' => isset($values['tree-accession']["species-$i"]['file']) ? $values['tree-accession']["species-$i"]['file'] : NULL,
              '#description' => $file_description,
              '#tree' => TRUE
            );
            
            $form['tree-accession']["species-$i"]['no-header'] = array(
              '#type' => 'checkbox',
              '#title' => t('My file has no header row'),
              '#default_value' => isset($values['tree-accession']["species-$i"]['no-header']) ? $values['tree-accession']["species-$i"]['no-header'] : NULL,
              '#ajax' => array(
                'wrapper' => "header-$i-wrapper",
                'callback' => 'accession_header_callback',
              ),
            );
            
            $form['tree-accession']["species-$i"]['file']['columns'] = array(
              '#type' => 'fieldset',
              '#title' => t('<div class="fieldset-title">Define Data</div>'),
              '#states' => array(
                'invisible' => array(
                  ':input[name="tree-accession_species-' . $i . '_file_upload_button"]' => array('value' => 'Upload')
                )
              ),
              '#description' => 'Please define which columns hold the required data: Tree Identifier and Location',
              '#prefix' => "<div id=\"header-$i-wrapper\">",
              '#suffix' => '</div>',
              '#collapsible' => TRUE,
            );

            $file = 0;
            if (isset($form_state['values']['tree-accession']["species-$i"]['file']) and $form_state['values']['tree-accession']["species-$i"]['file'] != 0){
                $file = $form_state['values']['tree-accession']["species-$i"]['file'];
                $form_state['saved_values']['thirdPage']['tree-accession']["species-$i"]['file'] = $file;
            }
            elseif (isset($form_state['saved_values']['thirdPage']['tree-accession']["species-$i"]['file']) and $form_state['saved_values']['thirdPage']['tree-accession']["species-$i"]['file'] != 0){
                $file = $form_state['saved_values']['thirdPage']['tree-accession']["species-$i"]['file'];
            }
            
            if ($file != 0){
                if (($file = file_load($file))){
                    $file_name = $file->uri;
                    
                    //stop using the file so it can be deleted if the user clicks 'remove'
                    file_usage_delete($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));

                    $location = drupal_realpath("$file_name");
                    $content = parse_xlsx($location);
                    $no_header = FALSE;

                    if (isset($form_state['complete form']['tree-accession']["species-$i"]['no-header']['#value']) and $form_state['complete form']['tree-accession']["species-$i"]['no-header']['#value'] == 1){
                        tpps_content_no_header($content);
                        $no_header = TRUE;
                    }
                    elseif (!isset($form_state['complete form']['tree-accession']["species-$i"]['no-header']['#value']) and isset($values['tree-accession']["species-$i"]['no-header']) and $values['tree-accession']["species-$i"]['no-header'] == 1){
                        tpps_content_no_header($content);
                        $no_header = TRUE;
                    }

                    $column_options = array(
                      '0' => 'N/A',
                      '1' => 'Tree Identifier',
                      '2' => 'Country',
                      '3' => 'State',
                      '8' => 'County',
                      '9' => 'District',
                      '4' => 'Latitude',
                      '5' => 'Longitude',
                    );

                    $first = TRUE;

                    foreach ($content['headers'] as $item){
                        $form['tree-accession']["species-$i"]['file']['columns'][$item] = array(
                          '#type' => 'select',
                          '#title' => t($item),
                          '#options' => $column_options,
                          '#default_value' => isset($values['tree-accession']["species-$i"]['file-columns'][$item]) ? $values['tree-accession']["species-$i"]['file-columns'][$item] : 0,
                          '#prefix' => "<td>",
                          '#suffix' => "</td>",
                          '#attributes' => array(
                            'data-toggle' => array('tooltip'),
                            'data-placement' => array('left'),
                            'title' => array("Select the type of data the '$item' column holds")
                          )
                        );

                        if ($first){
                            $first = FALSE;
                            $form['tree-accession']["species-$i"]['file']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $form['tree-accession']["species-$i"]['file']['columns'][$item]['#prefix'];
                        }

                        if ($no_header){
                            $form['tree-accession']["species-$i"]['file']['columns'][$item]['#title'] = '';
                            $form['tree-accession']["species-$i"]['file']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
                        }
                    }

                    // display sample data
                    $display = "</tr>";
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

                    $form['tree-accession']["species-$i"]['file']['columns'][$item]['#suffix'] .= $display;
                }
            }	
        }
    }
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#prefix' => '<div class="input-description">* : Required Field</div>',
    );
    
    $form['Save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
    
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );
    
    return $form;
}

function accession_header_callback($form, $form_state){
    if (isset($form_state['values']['tree-accession']['check']) and $form_state['values']['tree-accession']['check']){
        return $form['tree-accession'][$form_state['triggering_element']['#parents'][1]]['file']['columns'];
    }
    else {
        return $form['tree-accession']['file']['columns'];
    }
}

function page_3_multi_map($form, $form_state){
    
    if (isset($form['tree-accession']['file']['#value']['fid']) and $form['tree-accession']['file']['#value']['fid'] != '0'){
        $file = $form_state['values']['tree-accession']['file']['fid'];
        $columns = $form_state['values']['tree-accession']['file']['columns'];
        
        foreach ($columns as $key => $val){
            if ($val == '1'){
                $id_col = $key;
            }
            if ($val == '4'){
                $lat_col = $key;
            }
            if ($val == '5'){
                $long_col = $key;
            }
        }
        
        if (!isset($lat_col) or !isset($long_col) or !isset($id_col)){
            $commands[] = ajax_command_invoke('#map_wrapper', 'hide');
            return array('#type' => 'ajax', '#commands' => $commands);
        }
        
        $standards = array();
        
        if (($file = file_load($file))){
            $file_name = $file->uri;

            $location = drupal_realpath("$file_name");
            $content = parse_xlsx($location);

            if (isset($form_state['values']['tree-accession']['no-header']) and $form_state['values']['tree-accession']['no-header'] == 1){
                tpps_content_no_header($content);
            }

            for ($i = 0; $i < count($content) - 1; $i++){
                if (($coord = tpps_standard_coord("{$content[$i][$lat_col]},{$content[$i][$long_col]}"))){
                    $pair = explode(',', $coord);
                    array_push($standards, array("{$content[$i][$id_col]}", $pair[0], $pair[1]));
                }
            }
            
        }
        
        $commands[] = ajax_command_invoke('#map_wrapper', 'updateMap', array($standards));
        
        return array('#type' => 'ajax', '#commands' => $commands);
    }
    else {
        $commands[] = ajax_command_invoke('#map_wrapper', 'hide');
        return array('#type' => 'ajax', '#commands' => $commands);
    }
}

function page_3_validate_form(&$form, &$form_state){
    if ($form_state['submitted'] == '1'){
        
        $species_number = $form_state['saved_values']['Hellopage']['organism']['number'];
        if ($species_number == 1 or $form_state['values']['tree-accession']['check'] == '0'){
            if ($form_state['values']['tree-accession']['file'] != ""){
                
                $required_groups = array(
                  'Tree Id' => array(
                    'id' => array(1),
                  ),
                  'Location (latitude/longitude or country/state)' => array(
                    'gps' => array(2, 3),
                    'approx' => array(4, 5),
                  ),
                  'Genus and Species' => array(
                    'separate' => array(6, 7),
                    'combined' => array(10),
                  ),
                );
                
                $file_element = $form['tree-accession']['file'];
                tpps_file_validate_columns($form_state, $required_groups, $file_element);
                
                if (!form_get_errors()){
                    //preserve file if it is valid
                    $file = file_load($form_state['values']['tree-accession']['file']);
                    file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
                }
            }
            else{
                form_set_error('tree-accession][file', 'Tree Accession file: field is required.');
            }
        }
        else {
            
            for ($i = 1; $i <= $species_number; $i++){
                if ($form_state['values']['tree-accession']["species-$i"]['file'] != ""){
                    
                    $required_groups = array(
                      'Tree Id' => array(
                        'id' => array(1),
                      ),
                      'Location (latitude/longitude or country/state)' => array(
                        'gps' => array(2, 3),
                        'approx' => array(4, 5),
                      )
                    );
                    
                    $file_element = $form['tree-accession']["species-$i"]['file'];
                    tpps_file_validate_columns($form_state, $required_groups, $file_element);
                    
                    if (!form_get_errors()){
                        //preserve file if it is valid
                        $file = file_load($form_state['values']['tree-accession']["species-$i"]['file']);
                        file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
                    }
                }
                else{
                    form_set_error("tree-accession][species-$i][file", "Species $i Tree Accession file: field is required.");
                }
            }
        }
        
        if (form_get_errors() and ($species_number == 1 or $form_state['values']['tree-accession']['check'] == '0')){
            $form_state['rebuild'] = TRUE;
            $new_form = drupal_rebuild_form('tpps_master', $form_state, $form);
            $form['tree-accession']['file']['upload'] = $new_form['tree-accession']['file']['upload'];
            $form['tree-accession']['file']['columns'] = $new_form['tree-accession']['file']['columns'];
            $form['tree-accession']['file']['upload']['#id'] = "edit-tree-accession-file-upload";
            $form['tree-accession']['file']['columns']['#id'] = "edit-tree-accession-file-columns";
        }
        elseif (form_get_errors()){
            $form_state['rebuild'] = TRUE;
            $new_form = drupal_rebuild_form('tpps_master', $form_state, $form);
            for ($i = 1; $i <= $species_number; $i++){
                $form['tree-accession']["species-$i"]['file']['upload'] = $new_form['tree-accession']["species-$i"]['file']['upload'];
                $form['tree-accession']["species-$i"]['file']['columns'] = $new_form['tree-accession']["species-$i"]['file']['columns'];
                $form['tree-accession']["species-$i"]['file']['upload']['#id'] = "edit-tree-accession-species-$i-file-upload";
                $form['tree-accession']["species-$i"]['file']['columns']['#id'] = "edit-tree-accession-species-$i-file-columns";
            }
        }
    }
    
}

function page_3_submit_form(&$form, &$form_state){
    
}