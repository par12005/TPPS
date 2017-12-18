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
        );
        
        $form['organization'] = array(
          '#type' => 'textfield',
          '#title' => t('Organization:'),
          '#autocomplete_path' => 'organization/autocomplete',
          '#default_value' => isset($values['organization']) ? $values['organization'] : NULL,
        );
        
        return $form;
    }
    
    function publication(&$form, $values){
        
        function year(&$form, $values){
            
            $yearArrSubmitted = Array();
            $yearArrSubmitted[0] = '- Select -';
            for ($i = 2015; $i <= 2017; $i++) {
                $yearArrSubmitted[$i] = "$i";
            }

            $yearArrInPress = Array();
            $yearArrInPress[0] = '- Select -';
            for ($i = 2015; $i <= 2017; $i++) {
                $yearArrInPress[$i] = "$i";
            }

            $yearArrPublished = Array();
            $yearArrPublished[0] = '- Select -';
            for ($i = 1990; $i <= 2017; $i++) {
                $yearArrPublished[$i] = "$i";
            }

            $form['publication']['yearSubmitted'] = array(
              '#type' => 'select',
              '#title' => t('Year of Publication'),
              '#options' => $yearArrSubmitted,
              '#default_value' => isset($values['publication']['yearSubmitted']) ? $values['publication']['yearSubmitted'] : 0,
              '#states' => array(
                'visible' => array(
                  ':input[name="publication[status]"]' => array('value' => '2')
                ),
                'required' => array(
                  ':input[name="publication[status]"]' => array('value' => '2')
                )
              )
            );

            $form['publication']['yearInPress'] = array(
              '#type' => 'select',
              '#title' => t('Year of Publication'),
              '#options' => $yearArrInPress,
              '#default_value' => isset($values['publication']['yearInPress']) ? $values['publication']['yearInPress'] : 0,
              '#states' => array(
                'visible' => array(
                  ':input[name="publication[status]"]' => array('value' => '3')
                ),
                'required' => array(
                  ':input[name="publication[status]"]' => array('value' => '3')
                )
              )
            );

            $form['publication']['yearPublished'] = array(
              '#type' => 'select',
              '#title' => t('Year of Publication'),
              '#options' => $yearArrPublished,
              '#default_value' => isset($values['publication']['yearPublished']) ? $values['publication']['yearPublished'] : 0,
              '#states' => array(
                'visible' => array(
                  ':input[name="publication[status]"]' => array('value' => '4')
                ),
                'required' => array(
                  ':input[name="publication[status]"]' => array('value' => '4')
                )
              )
            );
            
            return $form;
        }
        
        function secondary_authors(&$form, $values){
            
            $form['publication']['secondaryAuthors'] = array(
              '#type' => 'fieldset',
              '#title' => t('Secondary Authors:'),
              '#states' => array(
                'visible' => array(
                  array(
                    array(':input[name="publication[status]"]' => array('value' => '2')),
                    'or',
                    array(':input[name="publication[status]"]' => array('value' => '3')),
                    'or',
                    array(':input[name="publication[status]"]' => array('value' => '4')),
                  )
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
            );
            
            $form['publication']['secondaryAuthors']['file'] = array(
              '#type' => 'file',
              '#title' => t('Please upload a file containing the names of all of your authors.'),
              '#states' => array(
                'visible' => array(
                  ':input[name="publication[secondaryAuthors][check]"]' => array('checked' => TRUE)
                )
              )
            );
            
            return $form;
        }
        
        $form['publication'] = array(
          '#type' => 'fieldset',
          '#title' => t('Publication Information:'),
          '#tree' => true
        );

        $form['publication']['status'] = array(
          '#type' => 'select',
          '#title' => t('Publication Status:'),
          '#options' => array(
            0 => t('- Select -'),
            2 => t('In Preparation or Submitted'),
            3 => t('In press'),
            4 => t('Published'),
          ),
          '#default_value' => isset($values['publication']['status']) ? $values['publication']['status'] : 0,
          '#required' => true,
        );
        
        secondary_authors($form, $values);
        
        year($form, $values);

        $form['publication']['title'] = array(
          '#type' => 'textfield',
          '#title' => t('Title of Publication:'),
          '#default_value' => isset($values['publication']['title']) ? $values['publication']['title'] : NULL,
          '#states' => array(
            'visible' => array(
              array(
                array(':input[name="publication[status]"]' => array('value' => '2')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '3')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '4')),
              )
            ),
            'required' => array(
              array(
                array(':input[name="publication[status]"]' => array('value' => '2')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '3')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '4')),
              )
            )
          )
        );

        $form['publication']['abstract'] = array(
          '#type' => 'textarea',
          '#title' => t('Abstract:'),
          '#default_value' => isset($values['publication']['abstract']) ? $values['publication']['abstract'] : NULL,
          '#states' => array(
            'visible' => array(
              array(
                array(':input[name="publication[status]"]' => array('value' => '2')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '3')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '4')),
              )
            ),
            'required' => array(
              array(
                array(':input[name="publication[status]"]' => array('value' => '2')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '3')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '4')),
              )
            )
          )
        );

        $form['publication']['journal'] = array(
          '#type' => 'textfield',
          '#title' => t('Journal:'),
          '#autocomplete_path' => 'journal/autocomplete',
          '#default_value' => isset($values['publication']['journal']) ? $values['publication']['journal'] : NULL,
          '#states' => array(
            'visible' => array(
              array(
                array(':input[name="publication[status]"]' => array('value' => '2')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '3')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '4')),
              )
            ),
            'required' => array(
              array(
                array(':input[name="publication[status]"]' => array('value' => '2')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '3')),
                'or',
                array(':input[name="publication[status]"]' => array('value' => '4')),
              )
            )
          )
        );
        
        return $form;
    }
    
    function organism(&$form, $values, $form_state){
        
        $form['organism'] = array(
          '#type' => 'fieldset',
          '#tree' => TRUE,
          '#title' => t('Organism information:'),
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
              '#title' => t("Organism $i:"),
            );
            
            $form['organism']["$i"]['species'] = array(
              '#type' => 'textfield',
              '#title' => t('Species:'),
              '#autocomplete_path' => "species/autocomplete",
              '#default_value' => isset($values['organism']["$i"]['species']) ? $values['organism']["$i"]['species'] : NULL,
            );
            
            $form['organism']["$i"]['custom-species'] = array(
              '#type' => 'textfield',
              '#default_value' => isset($values['organism']["$i"]['custom-species']) ? $values['organism']["$i"]['custom-species'] : "TRUE",
            );
        }
        
        return $form;
    }
    
    user_info($form, $values);
    
    publication($form, $values);
    
    organism($form, $values, $form_state);
    
    /*
    $form['keywords'] = array(
      '#type' => 'textfield',
      '#title' => t('Keywords'),
      '#description' => t('Please enter keywords separated by commmas'),
    );
     */
    
    $form['Next'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );
    
    //drupal_add_js(drupal_get_path('module', 'custom_module') . "/custom_module.js");

    /*
     * This is instantiating a user token.  It will be verified
     * in the JS file and they will not be given acess to data 
     * returned by AJAX if their token is invalid (if not logged
     * in).
     */
    global $user;
    $newToken = drupal_get_token("my secret value" . $user->uid);
    drupal_add_js("var myToken='$newToken'", "inline");
    
    return $form;
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
        $year_submitted = $form_values['publication']['yearSubmitted'];
        $year_in_press = $form_values['publication']['yearInPress'];
        $year_published = $form_values['publication']['yearPublished'];
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
        
        if ($publication_status == 0){
            form_set_error('publication][status', 'Publication Status: field is required.');
        }
        elseif($publication_status == 2 or $publication_status == 3 or $publication_status == 4){
            
            if ($secondary_authors_number > 0 and $secondary_authors_check == '0'){
                for($i = 1; $i <= $secondary_authors_number; $i++){
                    if ($secondary_authors_array[$i] == ''){
                        form_set_error("publication][secondaryAuthors][$i", "Secondary Author $i: field is required.");
                    }
                }
            }

            if ($publication_status == 2 and $year_submitted == 0){
                form_set_error('publication][yearSubmitted', 'Year of Publication: field is required.');
            }
            elseif ($publication_status == 3 and $year_in_press == 0){
                form_set_error('publication][yearInPress', 'Year of Publication: field is required.');
            }
            elseif ($publication_status == 4 and $year_published == 0){
                form_set_error('publication][yearPublished', 'Year of Publication: field is required.');
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
        
        $indexed_species = db_select('chado.organism', 'organism')
            ->fields('organism', array('genus', 'species'))
            ->orderBy('genus')
            ->orderBy('species')
            ->execute();
        
        for ($i = 1; $i <= $organism_number; $i++){
            $species = $organism[$i]['species'];
            $form_state['values']['organism']["$i"]['custom-species'] = "TRUE";
            
            if ($species == ''){
                form_set_error("organism[$i][species", "Tree Species $i: field is required.");
            }
            else{
                foreach ($indexed_species as $row){
                    if ($species === ($row->genus . " " . $row->species)){
                        $form_state['values']['organism']["$i"]['custom-species'] = "FALSE";
                        break;
                    }
                }
            }
        }
    }
}

function page_1_submit_form(&$form, &$form_state){
    /*//    Values in the fields need to be submitted to a junk table in chado schema
//    $form_state['values'] for secondary author textfield is blank?
    $primaryAuthor = $form_state['values']['primaryAuthor'];
    $orginization = $form_state['values']['organization'];
    $publicationStatus = $form_state['values']['publicationStatus'];
    $journal = $form_state['values']['journal'];
    $title = $form_state['values']['title'];
    $year = $form_state['values']['year'];
    $abstract = $form_state['values']['abstract'];
    
    $submitArr = array($primaryAuthor, $orginization, $publicationStatus, $journal, $title, $year, $abstract);
    for ($i = 0; $i <= 10; $i++){
        if ($form_state['values']['secondaryAuthorForm' . $i] == 'Other'){
//            send $form_state['values']['secondaryAuthorCustomForm' . $i] to temporary table
        }
        
        else{
//            send $form_state['values']['secondaryAuthorForm' . $i] to temporary table
        }
        
        if ($form_state['values']['species' . $i] == 'Other'){
//            send $form_state['values']['customSpecies' . $i] to temporary table            
        }
        
        else{
//            send $form_state['values']['species' . $i] to temporary table
        }
    }

//     foreach($submitArr as $r) {
//        db_query('INSERT INTO chado.custom_table_version 2 (custom_field) '
//            . 'VALUES (' . $r 
//            . ' )');
//    }
//    
//     $authorResults = db_query('SELECT contact.contact_id, contact.name FROM chado.contact WHERE contact.type_id = 71 ORDER BY contact.name LIMIT 200');
//    
//    
//    
//    $rawKeywords = $form_state['values']['keywords']; 
//    $keyWords = explode(',', $rawKeywords);      
    
//    $secondaryAuthorArr = array();
//    
//    }*/
    

    $form_state['redirect'] = 'secondPage';
}
