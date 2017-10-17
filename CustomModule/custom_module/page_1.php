<?php
function page_1_create_form(&$form){
        
    function user_info(&$form){

        $form['primaryAuthor'] = array(
          '#type' => 'textfield',
          '#title' => t('Primary Author:'),
          '#autocomplete_path' => 'author/autocomplete',
          '#required' => true
        );
        
        $form['organization'] = array(
          '#type' => 'textfield',
          '#title' => t('Organization:'),
          '#autocomplete_path' => 'organization/autocomplete',
          '#required' => true
        );
        
        return $form;
    }
    
    function publication(&$form){
        
        function year(&$form){
            
            $yearArrSubmitted = Array();
            $yearArrSubmitted[0] = 'Please select a year';
            for ($i = 2015; $i <= 2017; $i++) {
                $yearArrSubmitted[$i] = "$i";
            }

            $yearArrInPress = Array();
            $yearArrInPress[0] = 'Please select a year';
            for ($i = 2015; $i <= 2017; $i++) {
                $yearArrInPress[$i] = "$i";
            }

            $yearArrPublished = Array();
            $yearArrPublished[0] = 'Please select a year';
            for ($i = 1990; $i <= 2017; $i++) {
                $yearArrPublished[$i] = "$i";
            }

            $form['publication']['yearSubmitted'] = array(
              '#type' => 'select',
              '#title' => t('Year of Publication'),
              '#options' => $yearArrSubmitted,
              '#default_value' => 0,
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
              '#default_value' => 0,
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
              '#default_value' => 0,
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
        
        function secondary_authors(&$form){
            
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

            $number_options = array();
            $number_options[0] = 'Please select number of Secondary Authors';
            for ($i = 0; $i <= 30; $i++){
                $number_options[$i+1] = "$i";
            }
            $number_options[32] = '>30';

            $form['publication']['secondaryAuthors']['number'] = array(
              '#type' => 'select',
              '#title' => t('Number of Secondary Authors'),
              '#options' => $number_options,
              '#default_value' => 0,
              '#states' => array(
                'required' => array(
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

            for ($i = 2; $i <= 31; $i++){

                $author_label = $i - 1;

                $visible_states = array();
                array_push($visible_states, array(':input[name="publication[secondaryAuthors][number]"]' => array('value' => "$i")));

                for($j = $i + 1; $j <= 31; $j++){
                    array_push($visible_states, 'or');
                    array_push($visible_states, array(':input[name="publication[secondaryAuthors][number]"]' => array('value' => "$j")));
                }

                $form['publication']['secondaryAuthors'][$author_label] = array(
                  '#type' => 'textfield',
                  '#title' => t("Secondary Author $author_label:"),
                  '#autocomplete_path' => 'author/autocomplete',
                  '#states' => array(
                    'visible' => array($visible_states),
                    'required' => array($visible_states),
                  )
                );
            }

            $form['publication']['secondaryAuthors']['file'] = array(
              '#type' => 'file',
              '#title' => t('Please upload a file containing the names of all of your authors.'),
              '#states' => array(
                'visible' => array(
                  ':input[name="publication[secondaryAuthors][number]"]' => array('value' => '32')
                ),
                'required' => array(
                  ':input[name="publication[secondaryAuthors][number]"]' => array('value' => '32')
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
            0 => t('Please select a Publication Status'),
            1 => t('In Preparation'),
            2 => t('Submitted'),
            3 => t('In press'),
            4 => t('Published'),
          ),
          '#default_value' => 0,
          '#required' => true,
        );
        
        secondary_authors($form);
        
        year($form);

        $form['publication']['title'] = array(
          '#type' => 'textfield',
          '#title' => t('Title of Publication:'),
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
    
    user_info($form);
    
    publication($form);
    
    $genus_query = db_query('SELECT DISTINCT genus FROM chado.organism ORDER BY genus');
    $genus_options = array();
    $genus_options[0] = 'Please select a genus';
    foreach ($genus_query as $row){
        $genus_options[$row->genus] = $row->genus;
    }
    
    $form['genus'] = array(
      '#type' => 'select',
      '#title' => t('Genus:'),
      '#options' => $genus_options,
      '#default_value' => 0,
      '#states' => array(
        'visible' => array(
          ':input[name="species[other][check]"]' => array('checked' => false)
        ),
        'required' => array(
          ':input[name="species[other][check]"]' => array('checked' => false)
        )
      )
    );
    
    $form['species'] = array(
      '#type' => 'fieldset',
      '#tree' => true,
    );
    
    foreach($genus_options as $genus){
        
        if (substr($genus, 0, 6) == 'Please'){
            continue;
        }
        
        $species_query = db_query("SELECT species, genus FROM chado.organism WHERE genus='$genus' ORDER BY species");
        $species_options = array();
        $species_options[0] = 'Please select a species';
        foreach($species_query as $species_row){
            $species_options[$species_row->species] = $species_row->species;
        }
        
        $form['species']["$genus"] = array(
          '#type' => 'select',
          '#title' => t('Species:'),
          '#options' => $species_options,
          '#states' => array(
            'visible' => array(
              ':input[name="genus"]' => array('value' => "$genus"),
              ':input[name="species[other][check]"]' => array('checked' => false)
            ),
            'required' => array(
              ':input[name="genus"]' => array('value' => "$genus"),
              ':input[name="species[other][check]"]' => array('checked' => false)
            )
          )
        );   
    }
    
    $form['species']['other'] = array(
      '#type' => 'fieldset',
    );
    
    $form['species']['other']['check'] = array(
      '#type' => 'checkbox',
      '#title' => t('My genus or species is not in this list')
    );
    
    $form['species']['other']['textGenus'] = array(
      '#type' => 'textfield',
      '#title' => 'Please enter your custom genus:',
      '#states' => array(
        'visible' => array(
          ':input[name="species[other][check]"]' => array('checked' => true)
        ),
        'required' => array(
          ':input[name="species[other][check]"]' => array('checked' => true)
        )
      )
    );
    
    $form['species']['other']['textSpecies'] = array(
      '#type' => 'textfield',
      '#title' => 'Please enter your custom species:',
      '#states' => array(
        'visible' => array(
          ':input[name="species[other][check]"]' => array('checked' => true)
        ),
        'required' => array(
          ':input[name="species[other][check]"]' => array('checked' => true)
        )
      )
    );
    
    /*
    $form['keywords'] = array(
      '#type' => 'textfield',
      '#title' => t('Keywords'),
      '#description' => t('Please enter keywords separated by commmas'),
    );
     */

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );


    /* This is the convention for adding JS files to your form.
     * Since it is added directly to the form, I would imagine 
     * it is only accessible to this specific form.  
     */
    drupal_add_js(drupal_get_path('module', 'custom_module') . "/custom_module.js");

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
    
    $form_values = $form_state['values'];
    $publication_status = $form_values['publication']['status'];
    $secondary_authors_number = $form_values['publication']['secondaryAuthors']['number'];
    $secondary_authors_array = array_slice($form_values['publication']['secondaryAuthors'], 1, 30, true);
    $secondary_authors_file = $form_values['publication']['secondaryAuthors']['file'];
    $year_submitted = $form_values['publication']['yearSubmitted'];
    $year_in_press = $form_values['publication']['yearInPress'];
    $year_published = $form_values['publication']['yearPublished'];
    $publication_title = $form_values['publication']['title'];
    $publication_abstract = $form_values['publication']['abstract'];
    $publication_journal = $form_values['publication']['journal'];
    $genus = $form_values['genus'];
    $custom_species_check = $form_values['species']['other']['check'];
    $custom_species_genus = $form_values['species']['other']['textGenus'];
    $custom_species_species = $form_values['species']['other']['textSpecies'];
    
    if ($publication_status == 0){
        form_set_error('publication][status', 'Publication Status: field is required.');
    }
    elseif($publication_status == 2 or $publication_status == 3 or $publication_status == 4){
        if ($secondary_authors_number == 0){
            form_set_error('publication][secondaryAuthors][number', 'Number of Secondary Authors: field is required.');
        }
        elseif ($secondary_authors_number == 32){
            //need to find out how to validate file uploads. 
            //may need to user hook_form_alter() functions.
            #print_r("'" . $secondary_authors_file . "'");
        }
        else{
            for($i = 1; $i <= $number_authors_required - 1; $i++){
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
    
    if ($custom_species_check == 0){
        if ($genus == '0'){
            form_set_error('genus', 'Genus: field is required.');
        }
        else{
            $species = $form_values['species'][$genus];
            if ($species == '0'){
                form_set_error("species][$genus", 'Species: field is required.');
            }
        }
    }
    else{
        if ($custom_species_genus == ''){
            form_set_error('species][other][textGenus', 'Custom Genus: field is required.');
        }
        
        if ($custom_species_species == ''){
            form_set_error('species][other][textSpecies', 'Custom Species: field is required.');
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
