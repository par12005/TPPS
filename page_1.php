<?php
function page_1_create_form(&$form, $form_state){
    
    if (isset($form_state['saved_values']['Hellopage'])){
        $values = $form_state['saved_values']['Hellopage'];
    }
    else{
        $values = array();
    }
    
    function user_info(&$form, $values){

        $form['primaryAuthor'] = array(
          '#type' => 'textfield',
          '#title' => t('Primary Author:'),
          '#autocomplete_path' => 'author/autocomplete',
          '#default_value' => isset($values['primaryAuthor']) ? $values['primaryAuthor'] : NULL,
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('First Author of the publication')
          )
        );
        
        $form['organization'] = array(
          '#type' => 'textfield',
          '#title' => t('Organization:'),
          '#autocomplete_path' => 'organization/autocomplete',
          '#default_value' => isset($values['organization']) ? $values['organization'] : NULL,
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('Organization of the Primary Author')
          )
        );
        
        return $form;
    }
    
    function publication(&$form, $values, $form_state){
        
        function year(&$form, $values, $form_state){
            
            if (isset($form_state['values']['publication']['status']) and $form_state['values']['publication']['status'] != '0'){
                $pub_status = $form_state['values']['publication']['status'];
            }
            elseif (isset($form_state['saved_values']['Hellopage']['publication']['status']) and $form_state['saved_values']['Hellopage']['publication']['status'] != '0'){
                $pub_status = $form_state['saved_values']['Hellopage']['publication']['status'];
            }
            
            if (isset($pub_status) and $pub_status != 'Published'){
                $yearArr = array(0 => '- Select -');
                for ($i = 2015; $i <= 2018; $i++) {
                    $yearArr[$i] = "$i";
                }
            }
            elseif (isset($pub_status)){
                $yearArr = array(0 => '- Select -');
                for ($i = 1990; $i <= 2018; $i++) {
                    $yearArr[$i] = "$i";
                }
            }
            else {
                $yearArr = array(0 => '- Select -');
            }
            
            $form['publication']['year'] = array(
              '#type' => 'select',
              '#title' => t('Year of Publication'),
              '#options' => $yearArr,
              '#default_value' => isset($values['publication']['year']) ? $values['publication']['year'] : 0,
              '#states' => array(
                'invisible' => array(
                  ':input[name="publication[status]"]' => array('value' => '0')
                )
              ),
              '#prefix' => '<div id="pubyear">',
              '#suffix' => '</div>'
            );
            
            return $form;
        }
        
        function secondary_authors(&$form, $values, $form_state){
            
            $file_upload_location = 'public://tpps_authors';
            
            $form['publication']['secondaryAuthors'] = array(
              '#type' => 'fieldset',
              '#states' => array(
                'invisible' => array(
                  ':input[name="publication[status]"]' => array('value' => '0')
                ),
              )
            );
            
            $form['publication']['secondaryAuthors']['add'] = array(
              '#type' => 'button',
              '#title' => t('Add Secondary Author'),
              '#button_type' => 'button',
              '#value' => t('Add Secondary Author')
            );
            
            $form['publication']['secondaryAuthors']['remove'] = array(
              '#type' => 'button',
              '#title' => t('Remove Secondary Author'),
              '#button_type' => 'button',
              '#value' => t('Remove Secondary Author')
            );
            
            $form['publication']['secondaryAuthors']['number'] = array(
              '#type' => 'textfield',
              '#default_value' => isset($values['publication']['secondaryAuthors']['number']) ? $values['publication']['secondaryAuthors']['number'] : '0',
            );
            
            for ($i = 1; $i <= 30; $i++){

                $form['publication']['secondaryAuthors'][$i] = array(
                  '#type' => 'textfield',
                  '#title' => t("Secondary Author $i:"),
                  '#autocomplete_path' => 'author/autocomplete',
                  '#default_value' => isset($values['publication']['secondaryAuthors'][$i]) ? $values['publication']['secondaryAuthors'][$i] : NULL,
                );
            }
            
            $form['publication']['secondaryAuthors']['check'] = array(
              '#type' => 'checkbox',
              '#title' => t('I have >30 Secondary Authors'),
              '#default_value' => isset($values['publication']['secondaryAuthors']['check']) ? $values['publication']['secondaryAuthors']['check'] : NULL,
              '#attributes' => array(
                'data-toggle' => array('tooltip'),
                'data-placement' => array('left'),
                'title' => array('Upload a file instead')
              )
            );
            
            $form['publication']['secondaryAuthors']['file'] = array(
              '#type' => 'managed_file',
              '#title' => t('Secondary Authors file: please upload a spreadsheet with columns for last name, first name, and middle initial of each author, in any order'),
              '#upload_location' => "$file_upload_location",
              '#upload_validators' => array(
                'file_validate_extensions' => array('txt csv xlsx')
              ),
              '#default_value' => isset($values['publication']['secondaryAuthors']['file']) ? $values['publication']['secondaryAuthors']['file'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="publication[secondaryAuthors][check]"]' => array('checked' => TRUE)
                )
              ),
              '#tree' => TRUE,
            );
            
            $form['publication']['secondaryAuthors']['no-header'] = array(
              '#type' => 'checkbox',
              '#title' => t('My file has no header row'),
              '#default_value' => isset($values['publication']['secondaryAuthors']['no-header']) ? $values['publication']['secondaryAuthors']['no-header'] : NULL,
              '#ajax' => array(
                'wrapper' => 'header-wrapper',
                'callback' => 'authors_header_callback',
              ),
              '#states' => array(
                'visible' => array(
                  ':input[name="publication[secondaryAuthors][check]"]' => array('checked' => TRUE)
                )
              ),
            );
            
            $form['publication']['secondaryAuthors']['file']['columns'] = array(
              '#type' => 'fieldset',
              '#title' => t('<h2>Define Data</h2>'),
              '#states' => array(
                'invisible' => array(
                  ':input[name="publication_secondaryAuthors_file_upload_button"]' => array('value' => 'Upload')
                )
              ),
              '#description' => 'Please define which columns hold the required data: Last Name, First Name, and Middle Initial',
              '#prefix' => '<div id="header-wrapper">',
              '#suffix' => '</div>'
            );
            
            $file = 0;
            if (isset($form_state['values']['publication']['secondaryAuthors']['file']) and $form_state['values']['publication']['secondaryAuthors']['file'] != 0){
                $file = $form_state['values']['publication']['secondaryAuthors']['file'];
            }
            elseif (isset($form_state['saved_values']['Hellopage']['publication']['secondaryAuthors']['file']) and $form_state['saved_values']['Hellopage']['publication']['secondaryAuthors']['file'] != 0){
                $file = $form_state['saved_values']['Hellopage']['publication']['secondaryAuthors']['file'];
            }
            
            if ($file != 0){
                if (($file = file_load($file))){
                    $file_name = $file->uri;
                    
                    $location = drupal_realpath("$file_name");
                    $content = parse_xlsx($location);
                    $no_header = FALSE;

                    if (isset($form_state['complete form']['publication']['secondaryAuthors']['no-header']['#value']) and $form_state['complete form']['publication']['secondaryAuthors']['no-header']['#value'] == 1){
                        tpps_content_no_header($content);
                        $no_header = TRUE;
                    }
                    elseif (!isset($form_state['complete form']['publication']['secondaryAuthors']['no-header']['#value']) and isset($values['publication']['secondaryAuthors']['no-header']) and $values['publication']['secondaryAuthors']['no-header'] == 1){
                        tpps_content_no_header($content);
                        $no_header = TRUE;
                    }

                    $column_options = array(
                      'N/A',
                      'First Name',
                      'Last Name',
                      'Middle Initial'
                    );

                    $first = TRUE;

                    foreach ($content['headers'] as $item){
                        $form['publication']['secondaryAuthors']['file']['columns'][$item] = array(
                          '#type' => 'select',
                          '#title' => t($item),
                          '#options' => $column_options,
                          '#default_value' => isset($values['publication']['secondaryAuthors']['file-columns'][$item]) ? $values['publication']['secondaryAuthors']['file-columns'][$item] : 0,
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
                            $form['publication']['secondaryAuthors']['file']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $form['publication']['secondaryAuthors']['file']['columns'][$item]['#prefix'];
                        }

                        if ($no_header){
                            $form['publication']['secondaryAuthors']['file']['columns'][$item]['#title'] = '';
                            $form['publication']['secondaryAuthors']['file']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
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

                    $form['publication']['secondaryAuthors']['file']['columns'][$item]['#suffix'] .= $display;
                }
            }
            
            return $form;
        }
        
        $form['publication'] = array(
          '#type' => 'fieldset',
          '#title' => t('<h2>Publication Information:</h2>'),
          '#tree' => true,
        );

        $form['publication']['status'] = array(
          '#type' => 'select',
          '#title' => t('Publication Status:'),
          '#options' => array(
            0 => t('- Select -'),
            'In Preparation or Submitted' => t('In Preparation or Submitted'),
            'In Press' => t('In Press'),
            'Published' => t('Published'),
          ),
          '#ajax' => array(
            'callback' => 'page_1_pub_status',
            'wrapper' => 'pubyear'
          ),
          '#default_value' => isset($values['publication']['status']) ? $values['publication']['status'] : 0,
        );
        
        secondary_authors($form, $values, $form_state);
        
        year($form, $values, $form_state);

        $form['publication']['title'] = array(
          '#type' => 'textfield',
          '#title' => t('Title of Publication:'),
          '#default_value' => isset($values['publication']['title']) ? $values['publication']['title'] : NULL,
        );

        $form['publication']['abstract'] = array(
          '#type' => 'textarea',
          '#title' => t('Abstract:'),
          '#default_value' => isset($values['publication']['abstract']) ? $values['publication']['abstract'] : NULL,
        );

        $form['publication']['journal'] = array(
          '#type' => 'textfield',
          '#title' => t('Journal:'),
          '#autocomplete_path' => 'journal/autocomplete',
          '#default_value' => isset($values['publication']['journal']) ? $values['publication']['journal'] : NULL,
        );
        
        return $form;
    }
    
    function organism(&$form, $values){
        
        $form['organism'] = array(
          '#type' => 'fieldset',
          '#tree' => TRUE,
          '#title' => t('<h2>Organism information:</h2>'),
          '#description' => t('Up to 5 organisms per submission.'),
        );
        
        $form['organism']['add'] = array(
          '#type' => 'button',
          '#title' => t('Add Organism'),
          '#button_type' => 'button',
          '#value' => t('Add Organism')
        );

        $form['organism']['remove'] = array(
          '#type' => 'button',
          '#title' => t('Remove Organism'),
          '#button_type' => 'button',
          '#value' => t('Remove Organism')
        );

        $form['organism']['number'] = array(
          '#type' => 'textfield',
          '#default_value' => isset($values['organism']['number']) ? $values['organism']['number'] : '1',
        );
    
        for($i = 1; $i <= 5; $i++){
            
            $form['organism']["$i"] = array(
              '#type' => 'fieldset',
              //'#title' => t("Tree Species $i:"),
            );
            
            $form['organism']["$i"]['species'] = array(
              '#type' => 'textfield',
              '#title' => t("Species $i:"),
              '#autocomplete_path' => "species/autocomplete",
              '#default_value' => isset($values['organism']["$i"]['species']) ? $values['organism']["$i"]['species'] : NULL,
              '#attributes' => array(
                'data-toggle' => array('tooltip'),
                'data-placement' => array('left'),
                'title' => array('If your species is not in the autocomplete list, don\'t worry about it! We will create a new organism entry in the database for you.')
              )
            );
        }
        
        return $form;
    }
    
    user_info($form, $values);
    
    publication($form, $values, $form_state);
    
    organism($form, $values);
    
    /*
    $form['keywords'] = array(
      '#type' => 'textfield',
      '#title' => t('Keywords'),
      '#description' => t('Please enter keywords separated by commmas'),
    );
     */
    
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

function page_1_pub_status($form, $form_state){
    return $form['publication']['year'];
}

function authors_header_callback($form, $form_state){
    return $form['publication']['secondaryAuthors']['file']['columns'];
}

function page_1_validate_form(&$form, &$form_state){
    //for testing only.
    /*foreach($form_state['values'] as $key => $value){
        print_r($key . " => " . $value . ";<br>");
    }*/
    
    if ($form_state['submitted'] == '1'){
        
        $form_values = $form_state['values'];
        $primary_author = $form_values['primaryAuthor'];
        $organization = $form_values['organization'];
        $publication_status = $form_values['publication']['status'];
        $secondary_authors_number = $form_values['publication']['secondaryAuthors']['number'];
        $secondary_authors_array = array_slice($form_values['publication']['secondaryAuthors'], 3, 30, true);
        $secondary_authors_file = $form_values['publication']['secondaryAuthors']['file'];
        $secondary_authors_check = $form_values['publication']['secondaryAuthors']['check'];
        $year = $form_values['publication']['year'];
        $publication_title = $form_values['publication']['title'];
        $publication_abstract = $form_values['publication']['abstract'];
        $publication_journal = $form_values['publication']['journal'];
        $organism = $form_values['organism'];
        $organism_number = $form_values['organism']['number'];
        
        function validate_secondary_authors($secondary_authors_file, $form, &$form_state){
            if ($secondary_authors_file != ""){
                
                $required_columns = array(
                  '1' => 'First Name',
                  '2' => 'Last Name',
                );
                
                $form_state['values']['publication']['secondaryAuthors']['file-columns'] = array();
                //dpm($form['publication']['secondaryAuthors']['file']['columns']);
                
                foreach ($form['publication']['secondaryAuthors']['file']['columns'] as $req => $val){
                    if ($req[0] != '#'){
                        $form_state['values']['publication']['secondaryAuthors']['file-columns'][$req] = $form['publication']['secondaryAuthors']['file']['columns'][$req]['#value'];

                        $col_val = $form_state['values']['publication']['secondaryAuthors']['file-columns'][$req];
                        if ($col_val != '0'){
                            $required_columns[$col_val] = NULL;
                        }
                    }
                }
                
                foreach ($required_columns as $item){
                    if ($item != NULL){
                        form_set_error("publication][secondaryAuthors][file][columns][$item", "Secondary Authors file: Please specify a column that holds $item.");
                    }
                }
            }
            else{
                form_set_error('publication][secondaryAuthors][file', 'Secondary Authors file: field is required.');
            }
        }
        
        if ($primary_author == ''){
            form_set_error('primaryAuthor', 'Primary Author: field is required.');
        }
        
        if ($organization == ''){
            form_set_error('organization', 'Organization: field is required.');
        }
        
        if ($publication_status == '0'){
            form_set_error('publication][status', 'Publication Status: field is required.');
        }
        else{
            
            if ($secondary_authors_number > 0 and $secondary_authors_check == '0'){
                for($i = 1; $i <= $secondary_authors_number; $i++){
                    if ($secondary_authors_array[$i] == ''){
                        form_set_error("publication][secondaryAuthors][$i", "Secondary Author $i: field is required.");
                    }
                }
            }
            elseif ($secondary_authors_check == '1'){
                validate_secondary_authors($secondary_authors_file, $form, $form_state);
            }

            if ($year == '0'){
                form_set_error('publication][year', 'Year of Publication: field is required.');
            }

            if ($publication_title == ''){
                form_set_error('publication][title', 'Title of Publication: field is required.');
            }

            if ($publication_abstract == ''){
                form_set_error('publication][abstract', 'Abstract: field is required.');
            }

            if ($publication_journal == ''){
                form_set_error('publication][journal', 'Journal: field is required.');
            }
        }
        
        for ($i = 1; $i <= $organism_number; $i++){
            $name = $organism[$i]['species'];
            
            if ($name == ''){
                form_set_error("organism[$i][species", "Tree Species $i: field is required.");
            }
            else{
                $name = explode(" ", $name);
                $genus = $name[0];
                $species = implode(" ", array_slice($name, 1));
                $name = implode(" ", $name);
                $empty_pattern = '/^ *$/';
                $correct_pattern = '/^[A-Z|a-z|.| ]+$/';
                if (!isset($genus) or !isset($species) or preg_match($empty_pattern, $genus) or preg_match($empty_pattern, $species) or !preg_match($correct_pattern, $genus) or !preg_match($correct_pattern, $species)){
                    form_set_error("organism[$i][species", check_plain("Tree Species $i: please provide both genus and species in the form \"<genus> <species>\""));
                }
            }
        }
    }
}

function page_1_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'secondPage';
}
