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
          '#title' => t('Primary Author: *'),
          '#autocomplete_path' => 'author/autocomplete',
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('First Author of the publication')
          )
        );
        
        $form['organization'] = array(
          '#type' => 'textfield',
          '#title' => t('Organization: *'),
          '#autocomplete_path' => 'organization/autocomplete',
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
              '#title' => t('Year of Publication: *'),
              '#options' => $yearArr,
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
            
            $file_upload_location = 'public://' . variable_get('tpps_author_files_dir', 'tpps_authors');
            
            $form['publication']['secondaryAuthors'] = array(
              '#type' => 'fieldset',
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
                  '#title' => t("Secondary Author $i: *"),
                  '#autocomplete_path' => 'author/autocomplete',
                );
            }
            
            $form['publication']['secondaryAuthors']['check'] = array(
              '#type' => 'checkbox',
              '#title' => t('I have >30 Secondary Authors'),
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
              '#states' => array(
                'visible' => array(
                  ':input[name="publication[secondaryAuthors][check]"]' => array('checked' => TRUE)
                )
              ),
              '#tree' => TRUE,
            );
            
            $form['publication']['secondaryAuthors']['file']['empty'] = array(
              '#default_value' => isset($values['publication']['secondaryAuthors']['file']['empty']) ? $values['publication']['secondaryAuthors']['file']['empty'] : 'NA',
            );
            
            $form['publication']['secondaryAuthors']['file']['columns'] = array(
              '#description' => 'Please define which columns hold the required data: Last Name, First Name, and Middle Initial',
            );
            
            $column_options = array(
              'N/A',
              'First Name',
              'Last Name',
              'Middle Initial'
            );

            $form['publication']['secondaryAuthors']['file']['columns-options'] = array(
              '#type' => 'hidden',
              '#value' => $column_options,
            );
            
            $form['publication']['secondaryAuthors']['file']['no-header'] = array();
            
            return $form;
        }
        
        $form['publication'] = array(
          '#type' => 'fieldset',
          '#title' => t('<div class="fieldset-title">Publication Information:</div>'),
          '#tree' => TRUE,
          '#collapsible' => TRUE,
        );

        secondary_authors($form, $values, $form_state);
        
        $form['publication']['status'] = array(
          '#type' => 'select',
          '#title' => t('Publication Status: *'),
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
        );
        
        year($form, $values, $form_state);

        $form['publication']['title'] = array(
          '#type' => 'textfield',
          '#title' => t('Title of Publication: *'),
        );

        $form['publication']['abstract'] = array(
          '#type' => 'textarea',
          '#title' => t('Abstract: *'),
        );

        $form['publication']['journal'] = array(
          '#type' => 'textfield',
          '#title' => t('Journal: *'),
          '#autocomplete_path' => 'journal/autocomplete',
        );
        
        return $form;
    }
    
    function organism(&$form, $values){
        
        $form['organism'] = array(
          '#type' => 'fieldset',
          '#tree' => TRUE,
          '#title' => t('<div class="fieldset-title">Organism information:</div>'),
          '#description' => t('Up to 5 organisms per submission.'),
          '#collapsible' => TRUE
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
          '#type' => 'hidden',
          '#default_value' => isset($values['organism']['number']) ? $values['organism']['number'] : '1',
        );
    
        for($i = 1; $i <= 5; $i++){
            
            $form['organism']["$i"] = array(
              '#type' => 'textfield',
              '#title' => t("Species $i: *"),
              '#autocomplete_path' => "species/autocomplete",
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
      '#prefix' => '<div class="input-description">* : Required Field</div>',
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
        
        if ($primary_author == ''){
            form_set_error('primaryAuthor', 'Primary Author: field is required.');
        }
        
        if ($organization == ''){
            form_set_error('organization', 'Organization: field is required.');
        }
        
        if (!$publication_status){
            form_set_error('publication][status', 'Publication Status: field is required.');
        }
        
        if ($secondary_authors_number > 0 and !$secondary_authors_check){
            for($i = 1; $i <= $secondary_authors_number; $i++){
                if ($secondary_authors_array[$i] == ''){
                    form_set_error("publication][secondaryAuthors][$i", "Secondary Author $i: field is required.");
                }
            }
        }
        elseif ($secondary_authors_check){
            $file_element = $form_values['publication']['secondaryAuthors']['file'];

            if ($secondary_authors_file != ""){
                $required_groups = array(
                  'First Name' => array(
                    'first' => array(1),
                  ),
                  'Last Name' => array(
                    'last' => array(2),
                  ),
                );

                $file_element = $form['publication']['secondaryAuthors']['file'];
                $groups = tpps_file_validate_columns($form_state, $required_groups, $file_element);

                if (!form_get_errors()){
                    //preserve file if it is valid
                    $file = file_load($form_state['values']['publication']['secondaryAuthors']['file']);
                    file_usage_add($file, 'tpps', 'tpps_project', substr($form_state['accession'], 4));
                }
            }
            else{
                form_set_error('publication][secondaryAuthors][file', 'Secondary Authors file: field is required.');
            }
        }

        if (!$year){
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
        
        for ($i = 1; $i <= $organism_number; $i++){
            $name = $organism[$i];
            
            if ($name == ''){
                form_set_error("organism[$i", "Tree Species $i: field is required.");
            }
            else{
                $name = explode(" ", $name);
                $genus = $name[0];
                $species = implode(" ", array_slice($name, 1));
                $name = implode(" ", $name);
                $empty_pattern = '/^ *$/';
                $correct_pattern = '/^[A-Z|a-z|.| ]+$/';
                if (!isset($genus) or !isset($species) or preg_match($empty_pattern, $genus) or preg_match($empty_pattern, $species) or !preg_match($correct_pattern, $genus) or !preg_match($correct_pattern, $species)){
                    form_set_error("organism[$i", check_plain("Tree Species $i: please provide both genus and species in the form \"<genus> <species>\"."));
                }
            }
        }
    }
}

function page_1_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'secondPage';
}
