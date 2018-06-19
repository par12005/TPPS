<?php
function page_4_create_form(&$form, &$form_state){
    if (isset($form_state['saved_values']['fourthPage'])){
        $values = $form_state['saved_values']['fourthPage'];
    }
    else{
        $values = array();
    }
    
    function phenotype(&$form, $values, $id, &$form_state){
        
        $fields = array(
          '#type' => 'fieldset',
          '#title' => t('<h2>Phenotype Information:</h2>'),
        );
        
        $phenotype_number = isset($values[$id]['phenotype']['number']) ? $values[$id]['phenotype']['number'] : 1;
        
        $fields['check'] = array(
          '#type' => 'checkbox',
          '#title' => t('I have >30 Phenotypes'),
          '#default_value' => isset($values[$id]['phenotype']['check']) ? $values[$id]['phenotype']['check'] : NULL,
          '#attributes' => array(
            'data-toggle' => array('tooltip'),
            'data-placement' => array('left'),
            'title' => array('Upload a file instead')
          )
        );
        
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
        
        $structure_arr = array();
        $dev_arr = array();
	
        $results = db_select('chado.phenotype_structure_cvterm', 'phenotype_structure_cvterm')
            ->fields('phenotype_structure_cvterm', array('name', 'definition'))
            ->execute();
		
        
        foreach ($results as $row){
            $structure_arr[$row->name] = "$row->name : $row->definition";
        }
        
        $results = db_select('chado.phenotype_defs', 'phenotype_defs')
            ->fields('phenotype_defs', array('name', 'definition'))
            ->execute();
        
		
        foreach ($results as $row){
            $dev_arr[$row->name] = "$row->name : $row->definition";
        }
        
        for ($i = 1; $i <= 30; $i++){
            
            $fields["$i"] = array(
              '#type' => 'fieldset',
            );
            
            $fields["$i"]['name'] = array(
              '#type' => 'textfield',
              '#title' => t("Phenotype $i Name:"),
              '#autocomplete_path' => 'phenotype/autocomplete',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['name']) ? $values[$id]['phenotype']["$i"]['name'] : NULL,
              '#prefix' => "<label><b>Phenotype $i:</b></label>",
              '#attributes' => array(
                'data-toggle' => array('tooltip'),
                'data-placement' => array('left'),
                'title' => array('If your phenotype is not in the autocomplete list, don\'t worry about it! We will create a new phenotype entry in the database for you.')
              )
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
                  ':input[name="' . $id . '[phenotype][' . $i . '][environment][units]"]' => array('value' => '6')
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
              '#states' => array(
                'invisible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][type]"]' => array('value' => '3')
                )
              ),
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['units']) ? $values[$id]['phenotype']["$i"]['non-environment']['units'] : NULL,
            );
            
            $fields["$i"]['non-environment']['units-other'] = array(
              '#type' => 'textfield',
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['units-other']) ? $values[$id]['phenotype']["$i"]['non-environment']['units-other'] : NULL,
              '#states' => array(
                'visible' => array(
                  ':input[name="' . $id . '[phenotype][' . $i . '][non-environment][units]"]' => array('value' => '6')
                )
              )
            );
            
            $fields["$i"]['non-environment']['structure'] = array(
              '#type' => 'select',
              '#title' => t('Plant Structure:'),
              '#options' => $structure_arr,
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['structure']) ? $values[$id]['phenotype']["$i"]['non-environment']['structure'] : NULL,
            );
            
            $fields["$i"]['non-environment']['developmental'] = array(
              '#type' => 'select',
              '#title' => t('Plant Developmental Stage:'),
              '#options' => $dev_arr,
              '#default_value' => isset($values[$id]['phenotype']["$i"]['non-environment']['developmental']) ? $values[$id]['phenotype']["$i"]['non-environment']['developmental'] : NULL,
            );
        }
        
        $fields['metadata'] = array(
          '#type' => 'managed_file',
          '#title' => t('Please upload a file containing columns with the name and description of each of your phenotypes'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('csv tsv xlsx')
          ),
          '#default_value' => isset($values[$id]['phenotype']['metadata']) ? $values[$id]['phenotype']['metadata'] : NULL,
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
            )
          ),
          '#tree' => TRUE
        );

        $fields['no-header'] = array(
          '#type' => 'checkbox',
          '#title' => t('My file has no header row'),
          '#default_value' => isset($values[$id]['phenotype']['no-header']) ? $values[$id]['phenotype']['no-header'] : NULL,
          '#ajax' => array(
            'wrapper' => "header-$id-wrapper",
            'callback' => 'metadata_header_callback',
          ),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
            )
          ),
        );

        $fields['metadata']['columns'] = array(
          '#type' => 'fieldset',
          '#title' => t('<h2>Define Data</h2>'),
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $id . '_phenotype_metadata_upload_button"]' => array('value' => 'Upload')
            )
          ),
          '#description' => 'Please define which columns hold the required data: Phenotype name',
          '#prefix' => "<div id=\"header-$id-wrapper\">",
          '#suffix' => '</div>',
        );

        $file = 0;
        if (isset($form_state['values']["$id"]['phenotype']['metadata']) and $form_state['values']["$id"]['phenotype']['metadata'] != 0){
            $file = $form_state['values']["$id"]['phenotype']['metadata'];
            $form_state['saved_values']['fourthPage']["$id"]['phenotype']['metadata'] = $form_state['values']["$id"]['phenotype']['metadata'];
        }
        elseif (isset($form_state['saved_values']['fourthPage']["$id"]['phenotype']['metadata']) and $form_state['saved_values']['fourthPage']["$id"]['phenotype']['metadata'] != 0){
            $file = $form_state['saved_values']['fourthPage']["$id"]['phenotype']['metadata'];
        }

        if ($file != 0){
            if (($file = file_load($file))){
                $file_name = explode('//', $file->uri);
                $file_name = $file_name[1];

                $location = drupal_realpath("public://$file_name");
                $content = parse_xlsx($location);
                $no_header = FALSE;

                if (isset($form_state['complete form']["$id"]['phenotype']['no-header']['#value']) and $form_state['complete form']["$id"]['phenotype']['no-header']['#value'] == 1){
                    tpps_content_no_header($content);
                    $no_header = TRUE;
                }
                elseif (!isset($form_state['complete form']["$id"]['phenotype']['no-header']['#value']) and isset($values["$id"]['phenotype']['no-header']) and $values["$id"]['phenotype']['no-header'] == 1){
                    tpps_content_no_header($content);
                    $no_header = TRUE;
                }
                
                $column_options = array(
                  'N/A',
                  'Phenotype Name/Identifier',
                );

                $first = TRUE;

                foreach ($content['headers'] as $item){
                    $fields['metadata']['columns'][$item] = array(
                      '#type' => 'select',
                      '#title' => t($item),
                      '#options' => $column_options,
                      '#default_value' => isset($values["$id"]['phenotype']['metadata-columns'][$item]) ? $values["$id"]['phenotype']['metadata-columns'][$item] : 0,
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
                        $fields['metadata']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $fields['metadata']['columns'][$item]['#prefix'];
                    }

                    if ($no_header){
                        $fields['metadata']['columns'][$item]['#title'] = '';
                        $fields['metadata']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
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

                $fields['metadata']['columns'][$item]['#suffix'] .= $display;
            }
        }
        
        return $fields;
    }
    
    function genotype(&$form, &$form_state, $values, $id){
        
        $fields = array(
          '#type' => 'fieldset',
          '#title' => t('<h2>Genotype Information:</h2>'),
        );
        
        page_4_marker_info($fields, $values, $id);
        
        page_4_ref($fields, $form_state, $values, $id);
        
        $fields['file'] = array(
          '#type' => 'managed_file',
          '#title' => t('Genotype File: please provide a spreadsheet with columns for the Tree ID of genotypes used in this study:'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('xlsx')
          ),
          '#states' => array(
            'visible' => array(
              array(
                array(':input[name="' . $id . '[genotype][marker-type][SSRs/cpSSRs]"]' => array('checked' => true)),
                'or',
                array(':input[name="' . $id . '[genotype][marker-type][Other]"]' => array('checked' => true))
              )
            )
          ),
          '#default_value' => isset($values[$id]['genotype']['file']) ? $values[$id]['genotype']['file'] : NULL,
          '#tree' => TRUE
        );
        
        $fields['no-header'] = array(
          '#type' => 'checkbox',
          '#title' => t('My file has no header row'),
          '#default_value' => isset($values[$id]['phenotype']['no-header']) ? $values[$id]['phenotype']['no-header'] : NULL,
          '#ajax' => array(
            'wrapper' => "genotype-header-$id-wrapper",
            'callback' => 'genotype_header_callback',
          ),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[phenotype][check]"]' => array('checked' => TRUE)
            )
          ),
        );

        $fields['file']['columns'] = array(
          '#type' => 'fieldset',
          '#title' => t('<h2>Define Data</h2>'),
          '#states' => array(
            'invisible' => array(
              ':input[name="' . $id . '_genotype_file_upload_button"]' => array('value' => 'Upload')
            )
          ),
          '#description' => 'Please define which columns hold the required data: Tree Identifier',
          '#prefix' => "<div id=\"genotype-header-$id-wrapper\">",
          '#suffix' => '</div>',
        );
        
        $file = 0;
        if (isset($form_state['values'][$id]['genotype']['file']) and $form_state['values'][$id]['genotype']['file'] != 0){
            $file = $form_state['values'][$id]['genotype']['file'];
            //dpm($file);
        }
        elseif (isset($form_state['saved_values']['fourthPage'][$id]['genotype']['file']) and $form_state['saved_values']['fourthPage'][$id]['genotype']['file'] != 0){
            $file = $form_state['saved_values']['fourthPage'][$id]['genotype']['file'];
        }
        
        if ($file != 0){
            if (($file = file_load($file))){
                $file_name = $file->uri;

                $location = drupal_realpath("$file_name");
                $content = parse_xlsx($location);
                $no_header = FALSE;

                if (isset($form_state['complete form'][$id]['genotype']['no-header']['#value']) and $form_state['complete form'][$id]['genotype']['no-header']['#value'] == 1){
                    tpps_content_no_header($content);
                    $no_header = TRUE;
                }
                elseif (!isset($form_state['complete form'][$id]['genotype']['no-header']['#value']) and isset($values[$id]['genotype']['no-header']) and $values[$id]['genotype']['no-header'] == 1){
                    tpps_content_no_header($content);
                    $no_header = TRUE;
                }

                $column_options = array(
                  'N/A',
                  'Tree Identifier',
                );

                $first = TRUE;

                foreach ($content['headers'] as $item){
                    $fields['file']['columns'][$item] = array(
                      '#type' => 'select',
                      '#title' => t($item),
                      '#options' => $column_options,
                      '#default_value' => isset($values[$id]['genotype']['file-columns'][$item]) ? $values[$id]['genotype']['file-columns'][$item] : 0,
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
                        $fields['file']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $fields['file']['columns'][$item]['#prefix'];
                    }

                    if ($no_header){
                        $fields['file']['columns'][$item]['#title'] = '';
                        $fields['file']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
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

                $fields['file']['columns'][$item]['#suffix'] .= $display;
            }
        }
        
        $fields['vcf'] = array(
          '#type' => 'managed_file',
          '#title' => t('Genotype VCF File:'),
          '#upload_location' => 'public://',
          '#upload_validators' => array(
            'file_validate_extensions' => array('vcf')
          ),
          '#states' => array(
            'visible' => array(
              ':input[name="' . $id . '[genotype][marker-type][SNPs]"]' => array('checked' => true),
            )
          ),
          '#default_value' => isset($values[$id]['genotype']['vcf']) ? $values[$id]['genotype']['vcf'] : NULL,
          '#tree' => TRUE
        );
        
        return $fields;
    }
    
    $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
    $data_type = $form_state['saved_values']['secondPage']['dataType'];
    //dpm($data_type);
    for ($i = 1; $i <= $organism_number; $i++){
        
        $name = $form_state['saved_values']['Hellopage']['organism']["$i"]['species'];
        
        $form["organism-$i"] = array(
          '#type' => 'fieldset',
          '#title' => t("<h2>$name:</h2>"),
          '#tree' => TRUE,
        );

        if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
            $form["organism-$i"]['phenotype'] = phenotype($form, $values, "organism-$i", $form_state);
            
            $form["organism-$i"]['phenotype']['file'] = array(
              '#type' => 'managed_file',
              '#title' => t('Phenotype file: Please upload a file containing columns for Tree Identifier, Phenotype Name, and value for all of your phenotypic data:'),
              '#upload_location' => 'public://',
              '#upload_validators' => array(
                'file_validate_extensions' => array('csv tsv xlsx')
              ),
              '#default_value' => isset($values["organism-$i"]['phenotype']['file']) ? $values["organism-$i"]['phenotype']['file'] : NULL,
              '#tree' => TRUE,
            );

            $form["organism-$i"]['phenotype']['file-no-header'] = array(
              '#type' => 'checkbox',
              '#title' => t('My file has no header row'),
              '#default_value' => isset($values["organism-$i"]['phenotype']['file-no-header']) ? $values["organism-$i"]['phenotype']['file-no-header'] : NULL,
              '#ajax' => array(
                'wrapper' => "phenotype-header-$i-wrapper",
                'callback' => 'phenotype_header_callback',
              ),
            );

            $form["organism-$i"]['phenotype']['file']['columns'] = array(
              '#type' => 'fieldset',
              '#title' => t('<h2>Define Data</h2>'),
              '#states' => array(
                'invisible' => array(
                  ':input[name="organism-' . $i . '_phenotype_file_upload_button"]' => array('value' => 'Upload')
                )
              ),
              '#description' => 'Please define which columns hold the required data: Tree Identifier, Phenotype name, and Value(s)',
              '#prefix' => "<div id=\"phenotype-header$i-wrapper\">",
              '#suffix' => '</div>',
            );

            $file = 0;
            if (isset($form_state['values']["organism-$i"]['phenotype']['file']) and $form_state['values']["organism-$i"]['phenotype']['file'] != 0){
                $file = $form_state['values']["organism-$i"]['phenotype']['file'];
                $form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file'] = $form_state['values']["organism-$i"]['phenotype']['file'];
            }
            elseif (isset($form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file']) and $form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file'] != 0){
                $file = $form_state['saved_values']['fourthPage']["organism-$i"]['phenotype']['file'];
            }
            
            if ($file != 0){
                if (($file = file_load($file))){
                    $file_name = $file->uri;

                    $location = drupal_realpath("$file_name");
                    $content = parse_xlsx($location);
                    $no_header = FALSE;

                    if (isset($form_state['complete form']["organism-$i"]['phenotype']['file-no-header']['#value']) and $form_state['complete form']["organism-$i"]['phenotype']['file-no-header']['#value'] == 1){
                        tpps_content_no_header($content);
                        $no_header = TRUE;
                    }
                    elseif (!isset($form_state['complete form']["organism-$i"]['phenotype']['file-no-header']['#value']) and isset($values["organism-$i"]['phenotype']['file-no-header']) and $values["organism-$i"]['phenotype']['file-no-header'] == 1){
                        tpps_content_no_header($content);
                        $no_header = TRUE;
                    }

                    $column_options = array(
                      'N/A',
                      'Tree Identifier',
                      'Phenotype Name/Identifier',
                      'Value(s)'
                    );

                    $first = TRUE;

                    foreach ($content['headers'] as $item){
                        $form["organism-$i"]['phenotype']['file']['columns'][$item] = array(
                          '#type' => 'select',
                          '#title' => t($item),
                          '#options' => $column_options,
                          '#default_value' => isset($values["organism-$i"]['phenotype']['file-columns'][$item]) ? $values["organism-$i"]['phenotype']['file-columns'][$item] : 0,
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
                            $form["organism-$i"]['phenotype']['file']['columns'][$item]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $form["organism-$i"]['phenotype']['file']['columns'][$item]['#prefix'];
                        }

                        if ($no_header){
                            $form["organism-$i"]['phenotype']['file']['columns'][$item]['#title'] = '';
                            $form["organism-$i"]['phenotype']['file']['columns'][$item]['#attributes']['title'] = array("Select the type of data column $item holds");
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

                    $form["organism-$i"]['phenotype']['file']['columns'][$item]['#suffix'] .= $display;
                }
            }
        }
        
        if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
            $form["organism-$i"]['genotype'] = genotype($form, $form_state, $values, "organism-$i");
        }
    }
    
    $form['Back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
    );
    
    $form['Save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Review Information and Submit')
    );

    return $form;
}

function ajax_bioproject_callback(&$form, $form_state){
    
    $ajax_id = $form_state['triggering_element']['#parents'][0];
    
    return $form[$ajax_id]['genotype']['assembly-auto'];
}

function metadata_header_callback($form, $form_state){
    return $form[$form_state['triggering_element']['#parents'][0]]['phenotype']['no-header'];
}

function genotype_header_callback($form, $form_state){
    return $form[$form_state['triggering_element']['#parents'][0]]['genotype']['no-header'];
}

function phenotype_header_callback($form, $form_state){
    return $form[$form_state['triggering_element']['#parents'][0]]['phenotype']['file-no-header'];
}

function page_4_ref(&$fields, &$form_state, $values, $id){

    $options = array(
      'key' => 'filename',
      'recurse' => FALSE
    );

    $results = file_scan_directory("/isg/treegenes/treegenes_store/FTP/Genomes", '/^([A-Z]|[a-z]){4}$/', $options);
    //dpm($results);

    $ref_genome_arr = array();
    $ref_genome_arr[0] = '- Select -';

    foreach($results as $key=>$value){
        //dpm($key);
        $query = db_select('chado.organismprop', 'organismprop')
            ->fields('organismprop', array('organism_id'))
            ->condition('value', $key)
            ->execute()
            ->fetchAssoc();
        //dpm($query['organism_id']);
        $query = db_select('chado.organism', 'organism')
            ->fields('organism', array('genus', 'species'))
            ->condition('organism_id', $query['organism_id'])
            ->execute()
            ->fetchAssoc();
        //dpm($query['genus'] . " " . $query['species']);

        $versions = file_scan_directory("/isg/treegenes/treegenes_store/FTP/Genomes/$key", '/^v([0-9]|.)+$/', $options);
        //dpm($versions);
        foreach($versions as $item){
            $opt_string = $query['genus'] . " " . $query['species'] . " " . $item->filename;
            $ref_genome_arr[$opt_string] = $opt_string;
        }
    }
    
    $ref_genome_arr["url"] = 'I can provide a URL to my Reference Genome or assembly file(s)';
    $ref_genome_arr["bio"] = 'I can provide a BioProject accession number and select assembly file(s) from a list';
    $ref_genome_arr["manual"] = 'I can upload my own assembly file';

    $fields['ref-genome'] = array(
      '#type' => 'select',
      '#title' => t('Reference Genome used:'),
      '#options' => $ref_genome_arr,
      '#default_value' => isset($values[$id]['genotype']['ref-genome']) ? $values[$id]['genotype']['ref-genome'] : 0,
    );

    $fields['ref-genome-other'] = array(
      '#type' => 'textfield',
      '#title' => t('URL to Reference Genome:'),
      '#default_value' => isset($values[$id]['genotype']['ref-genome-other']) ? $values[$id]['genotype']['ref-genome-other'] : NULL,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'url'),
        )
      ),
      '#attributes' => array(
        'data-toggle' => array('tooltip'),
        'data-placement' => array('left'),
        'title' => array('This should be a link to a reference genome on NCBI')
      )
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
        'visible' => array(
          ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'bio')
        )
      )
    );
    
    $fields['assembly-auto'] = array(
      '#type' => 'fieldset',
      '#title' => t('Waiting for BioProject accession number...'),
      '#tree' => TRUE,
      '#prefix' => "<div id='$id-assembly-auto'>",
      '#suffix' => '</div>',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'bio')
        )
      )
    );
    
    if (isset($form_state['values'][$id]['genotype']['BioProject-id']) and $form_state['values'][$id]['genotype']['BioProject-id'] != ''){
        $bio_id = $form_state['values']["$id"]['genotype']['BioProject-id'];
        $form_state['saved_values']['fourthPage'][$id]['genotype']['BioProject-id'] = $form_state['values'][$id]['genotype']['BioProject-id'];
    }
    elseif (isset($form_state['saved_values']['fourthPage'][$id]['genotype']['BioProject-id']) and $form_state['saved_values']['fourthPage'][$id]['genotype']['BioProject-id'] != ''){
        $bio_id = $form_state['saved_values']['fourthPage'][$id]['genotype']['BioProject-id'];
    }
    elseif (isset($form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value']) and $form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value'] != '') {
        $bio_id = $form_state['complete form']['organism-1']['genotype']['BioProject-id']['#value'];
    }
    
    if (isset($bio_id) and $bio_id != ''){
        
        if (strlen($bio_id) > 5){
            $bio_id = substr($bio_id, 5);
        }
        
        $options = array();
        $url = "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/elink.fcgi?dbfrom=bioproject&db=nuccore&id=" . $bio_id;
        $response_xml_data = file_get_contents($url);
        $data = simplexml_load_string($response_xml_data)->children()->children()->LinkSetDb->children();

        if(preg_match('/<LinkSetDb>/', $response_xml_data)){

            foreach ($data->Link as $link){
                array_push($options, $link->Id->__tostring());
            }

            $fields['assembly-auto']['#title'] = 'Select all that apply:';

            foreach ($options as $item){
                $fields['assembly-auto']["$item"] = array(
                  '#type' => 'checkbox',
                  '#title' => t("$item"),
                  '#default_value' => isset($form_state['saved_values']['fourthPage']["$id"]['genotype']['assembly-auto']["$item"]) ? $form_state['saved_values']['fourthPage']["$id"]['genotype']['assembly-auto']["$item"] : NULL,
                );
            }
        }
        else {
            $fields['assembly-auto']['#description'] = t('We could not find any assembly files related to that BioProject. Please ensure your accession number is of the format "PRJNA#"');
        }
    }
    
    $fields['assembly-user'] = array(
      '#type' => 'managed_file',
      '#title' => t('Assembly File: please provide an assembly file in FASTA or Multi-FASTA format'),
      '#upload_location' => 'public://',
      '#upload_validators' => array(
        'file_validate_extensions' => array('fsa_nt')
      ),
      '#default_value' => isset($values[$id]['genotype']['assembly-user']) ? $values[$id]['genotype']['assembly-user'] : NULL,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][ref-genome]"]' => array('value' => 'manual')
        )
      ),
      '#tree' => TRUE
    );
    
    $fields['assembly-user']['columns'] = array(
      '#type' => 'fieldset',
      '#title' => t('<h2>Define Data</h2>'),
      '#description' => 'Please define which column holds the scaffold/chromosome identifier:',
      '#states' => array(
        'invisible' => array(
          ':input[name="' . $id . '_genotype_assembly-user_upload_button"]' => array('value' => 'Upload')
        )
      ),
    );
    
    $file = 0;
    if (isset($form_state['values'][$id]['genotype']['assembly-user']) and $form_state['values'][$id]['genotype']['assembly-user'] != 0){
        $file = $form_state['values'][$id]['genotype']['assembly-user'];
        $form_state['saved_values']['fourthPage'][$id]['genotype']['assembly-user'] = $form_state['values'][$id]['genotype']['assembly-user'];
    }
    elseif (isset($form_state['saved_values']['fourthPage'][$id]['genotype']['assembly-user']) and $form_state['saved_values']['fourthPage'][$id]['genotype']['assembly-user'] != 0){
        $file = $form_state['saved_values']['fourthPage'][$id]['genotype']['assembly-user'];
    }
    
    if ($file != 0){
        if (($file = file_load($file))){
            $content = fopen($file->uri, 'r');

            $first = TRUE;

            $column_options = array(
              'N/A',
              'Scaffold/Chromosome',
            );
            $headers = array();

            for ($i = 0; $i < 3; $i++){
                $line = fgets($content);
                $line = explode(' ', $line);

                if ($first){
                    foreach ($line as $col){
                        $headers[$col] = $col;
                        $fields['assembly-user']['columns'][$col] = array(
                          '#type' => 'select',
                          '#title' => '',
                          '#options' => $column_options,
                          '#default_value' => isset($values[$id]['genotype']['assembly-user-columns'][$col]) ? $values[$id]['genotype']['assembly-user-columns'][$col] : 0,
                          '#prefix' => "<td>",
                          '#suffix' => "</td>",
                          '#attributes' => array(
                            'data-toggle' => array('tooltip'),
                            'data-placement' => array('left'),
                            'title' => array("Select if this column holds the scaffold/chromosome identifier")
                          )
                        );

                        if ($first){
                            $first = FALSE;
                            $fields['assembly-user']['columns'][$col]['#prefix'] = "<div style='overflow-x:auto'><table border='1'><tbody><tr>" . $fields['assembly-user']['columns'][$col]['#prefix'];
                        }
                    }
                    $display = "<tr>";
                }
                if ($line[0][0] == '>'){
                    $display .= "<tr>";
                    for ($j = 0; $j < count($headers); $j++){
                        $display .= "<th>{$line[$j]}</th>";
                    }
                    $display .= "</tr>";
                }
                elseif (!isset($line)){
                    break;
                }
                else{
                    $i--;
                }

            }
            $display .= "</tbody></table></div>";

            $fields['assembly-user']['columns'][$col]['#suffix'] .= $display;
        }
    }
    
    return $fields;
}

function page_4_marker_info(&$fields, $values, $id){
    
    $fields['marker-type'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Marker Type (select all that apply):'),
      '#options' => drupal_map_assoc(array(
        t('SNPs'),
        t('SSRs/cpSSRs'),
        t('Other'),
      ))
    );

    $fields['marker-type']['SNPs']['#default_value'] = isset($values[$id]['genotype']['marker-type']['SNPs']) ? $values[$id]['genotype']['marker-type']['SNPs'] : NULL;
    $fields['marker-type']['SSRs/cpSSRs']['#default_value'] = isset($values[$id]['genotype']['marker-type']['SSRs/cpSSRs']) ? $values[$id]['genotype']['marker-type']['SSRs/cpSSRs'] : NULL;
    $fields['marker-type']['Other']['#default_value'] = isset($values[$id]['genotype']['marker-type']['Other']) ? $values[$id]['genotype']['marker-type']['Other'] : NULL;

    $fields['SNPs'] = array(
      '#type' => 'fieldset',
      '#title' => t('<h2>SNPs Information:</h2>'),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][marker-type][SNPs]"]' => array('checked' => true)
        )
      )
    );

     $fields['SNPs']['genotyping-design'] = array(
      '#type' => 'select',
      '#title' => t('Define Genotyping Design:'),
      '#options' => array(
        0 => '- Select -',
        1 => 'GBS',
        2 => 'Targeted Capture',
        3 => 'Whole Genome Resequencing',
        4 => 'RNA-Seq',
        5 => 'Genotyping Array'
      ),
      '#default_value' => isset($values[$id]['genotype']['SNPs']['genotyping-design']) ? $values[$id]['genotype']['SNPs']['genotyping-design'] : 0,
    );

    $fields['SNPs']['GBS'] = array(
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
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1')
        )
      ),
      '#default_value' => isset($values[$id]['genotype']['SNPs']['GBS']) ? $values[$id]['genotype']['SNPs']['GBS'] : 0,
    );

    $fields['SNPs']['GBS-other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][GBS]"]' => array('value' => '5'),
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '1')
        )
      ),
      '#default_value' => isset($values[$id]['genotype']['SNPs']['GBS-other']) ? $values[$id]['genotype']['SNPs']['GBS-other'] : NULL,
    );

    $fields['SNPs']['targeted-capture'] = array(
      '#type' => 'select',
      '#title' => t('Targeted Capture Type'),
      '#options' => array(
        0 => '- Select -',
        1 => 'Exome Capture',
        2 => 'Other'
      ),
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2')
        )
      ),
      '#default_value' => isset($values[$id]['genotype']['SNPs']['targeted-capture']) ? $values[$id]['genotype']['SNPs']['targeted-capture'] : 0,
    );

    $fields['SNPs']['targeted-capture-other'] = array(
      '#type' => 'textfield',
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][SNPs][targeted-capture]"]' => array('value' => '2'),
          ':input[name="' . $id . '[genotype][SNPs][genotyping-design]"]' => array('value' => '2')
        )
      ),
      '#default_value' => isset($values[$id]['genotype']['SNPs']['targeted-capture-other']) ? $values[$id]['genotype']['SNPs']['targeted-capture-other'] : NULL,
    );
    
    $fields['SSRs/cpSSRs'] = array(
      '#type' => 'textfield',
      '#title' => t('Define SSRs/cpSSRs Type:'),
      '#default_value' => isset($values[$id]['genotype']['SSRs/cpSSRs']) ? $values[$id]['genotype']['SSRs/cpSSRs'] : NULL,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][marker-type][SSRs/cpSSRs]"]' => array('checked' => true)
        )
      )
    );

    $fields['other-marker'] = array(
      '#type' => 'textfield',
      '#title' => t('Define Other Marker Type:'),
      '#default_value' => isset($values[$id]['genotype']['other-marker']) ? $values[$id]['genotype']['other-marker'] : NULL,
      '#states' => array(
        'visible' => array(
          ':input[name="' . $id . '[genotype][marker-type][Other]"]' => array('checked' => true)
        )
      )
    );
    
    return $fields;
}

function page_4_validate_form(&$form, &$form_state){
    
    if ($form_state['submitted'] == '1'){
        
        function validate_phenotype($phenotype, $id, $form, &$form_state){
            $phenotype_number = $phenotype['number'];
            $phenotype_check = $phenotype['check'];
            $phenotype_meta = $phenotype['metadata'];
            $phenotype_file = $phenotype['file'];

            if ($phenotype_check == '1'){
                if ($phenotype_meta == ''){
                    form_set_error("$id][phenotype][metadata", "Phenotype Metadata File: field is required.");
                }
                else{
                    $required_columns = array(
                      '1' => 'Phenotype Name/Identifier',
                    );

                    $form_state['values'][$id]['phenotype']['metadata-columns'] = array();

                    foreach ($form[$id]['phenotype']['metadata']['#value']['columns'] as $req => $val){
                        if ($req[0] != '#'){
                            $form_state['values'][$id]['phenotype']['metadata-columns'][$req] = $form[$id]['phenotype']['metadata']['#value']['columns'][$req];

                            $col_val = $form_state['values'][$id]['phenotype']['metadata-columns'][$req];
                            if ($col_val != '0'){
                                $required_columns[$col_val] = NULL;
                            }
                        }
                    }

                    foreach ($required_columns as $item){
                        if ($item != NULL){
                            form_set_error("$id][phenotype][metadata][columns][$item", "Phenotype Metadata file: Please specify a column that holds $item.");
                        }
                    }
                    
                    if ($required_columns['1'] === NULL){
                        //Phenotype Name set
                        foreach ($form_state['values'][$id]['phenotype']['metadata-columns'] as $key => $val){
                            //cycle through columns
                            if ($val == '1'){
                                //find the column that was assigned phenotype name, keep track of that column, exit loop
                                $phenotype_name_col = $key;
                                break;
                            }
                        }
                    }
                }
            }
            else{
                for($i = 1; $i <= $phenotype_number; $i++){
                    $current_phenotype = $phenotype["$i"];
                    $name = $current_phenotype['name'];
                    $environment_check = $current_phenotype['environment-check'];

                    if ($name == ''){
                        form_set_error("$id][phenotype][$i][name", "Phenotype $i Name: field is required.");
                    }

                    if ($environment_check == '1'){
                        $description = $current_phenotype['environment']['description'];
                        $units = $current_phenotype['environment']['units'];

                        if ($description == ''){
                            form_set_error("$id][phenotype][$i][environment][description", "Phenotype $i Description: field is required.");
                        }

                        if ($units == ''){
                            form_set_error("$id][phenotype][$i][environment][units", "Phenotype $i Units: field is required.");
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
                        $developmental = $current_phenotype['non-environment']['developmental'];

                        if ($type == '0'){
                            form_set_error("$id][phenotype][$i][non-environment][type", "Phenotype $i Type: field is required.");
                        }
                        elseif ($type == '1'){
                            if ($binary_1 == ''){
                                form_set_error("$id][phenotype][$i][non-environment][binary][1", "Phenotype $i Binary Type 1: field is required.");
                            }
                            if ($binary_2 == ''){
                                form_set_error("$id][phenotype][$i][non-environment][binary][2", "Phenotype $i Binary Type 2: field is required.");
                            }
                        }
                        elseif($type == '2'){
                            if ($min == ''){
                                form_set_error("$id][phenotype][$i][non-environment][quantitative][min", "Phenotype $i Minimum: field is required.");
                            }
                            if ($max == ''){
                                form_set_error("$id][phenotype][$i][non-environment][quantitative][max", "Phenotype $i Maximum: field is required.");
                            }
                        }

                        if ($description == ''){
                            form_set_error("$id][phenotype][$i][non-environment][description", "Phenotype $i Description: field is required.");
                        }

                        if ($units == '' and $type == '2'){
                            form_set_error("$id][phenotype][$i][non-environment][units", "Phenotype $i Units: field is required.");
                        }

                        if ($structure == '0'){
                            form_set_error("$id][phenotype][$i][non-environment][structure", "Phenotype $i Plant Structure: field is required.");
                        }

                        if ($developmental == '0'){
                            form_set_error("$id][phenotype][$i][non-environment][developmental", "Phenotype $i Developmental Stage: field is required.");
                        }
                    }
                }
            }

            if ($phenotype_file == ''){
                form_set_error("$id][phenotype][file", "Phenotypes: field is required.");
            }
            else {
                $required_columns = array(
                  '1' => 'Tree Identifier',
                  '2' => 'Phenotype Name/Identifier',
                  '3' => 'Value(s)'
                );

                $form_state['values'][$id]['phenotype']['file-columns'] = array();

                foreach ($form[$id]['phenotype']['file']['#value']['columns'] as $req => $val){
                    if ($req[0] != '#'){
                        $form_state['values'][$id]['phenotype']['file-columns'][$req] = $form[$id]['phenotype']['file']['#value']['columns'][$req];

                        $col_val = $form_state['values'][$id]['phenotype']['file-columns'][$req];
                        if ($col_val != '0'){
                            $required_columns[$col_val] = NULL;
                        }
                    }
                }

                foreach ($required_columns as $item){
                    if ($item != NULL){
                        form_set_error("$id][phenotype][file][columns][$item", "Phenotype file: Please specify a column that holds $item.");
                    }
                }
                
                if ($required_columns['2'] === NULL){
                    //Phenotype name column was detected
                    foreach ($form_state['values'][$id]['phenotype']['file-columns'] as $key => $val){
                        //find the column that holds it
                        if ($val == '2'){
                            //save that column name
                            $phenotype_file_name_col = $key;
                            break;
                        }
                    }
                }
                
                if ($required_columns['1'] === NULL){
                    //tree id column was detected
                    foreach ($form_state['values'][$id]['phenotype']['file-columns'] as $key => $val){
                        //find the column that holds it
                        if ($val == '1'){
                            //save that column name
                            $phenotype_file_tree_col = $key;
                            break;
                        }
                    }
                }
            }
            
            if (empty(form_get_errors()) and isset($phenotype_file_name_col) and isset($phenotype_name_col)){
                $missing_phenotypes = tpps_compare_files($phenotype_file, $phenotype_meta, $phenotype_file_name_col, $phenotype_name_col);
                
                if ($missing_phenotypes !== array()){
                    $phenotype_id_str = implode(', ', $missing_phenotypes);
                    form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype Metadata file. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype Metadata file. The phenotypes we detected with missing definitions were: $phenotype_id_str");
                }
            }
            elseif (empty(form_get_errors()) and isset($phenotype_file_name_col)){
                $phenotype_file = file_load($phenotype_file);
                $phenotype_file_name = explode('//', $phenotype_file->uri);
                $phenotype_file_name = $phenotype_file_name[1];
                $location = drupal_realpath("public://$phenotype_file_name");
                $content = parse_xlsx($location);
                
                $missing_phenotypes = array();
                for ($i = 0; $i < count($content) - 1; $i++){
                    $used_phenotype = $content[$i][$phenotype_file_name_col];
                    $defined = FALSE;
                    for ($j = 1; $j <= $phenotype_number; $j++){
                        if ($phenotype[$j]['name'] == $used_phenotype){
                            $defined = TRUE;
                            break;
                        }
                    }
                    if (!$defined){
                        array_push($missing_phenotypes, $used_phenotype);
                    }
                }
                
                if ($missing_phenotypes !== array()){
                    $phenotype_id_str = implode(', ', $missing_phenotypes);
                    form_set_error("$id][phenotype][file", "Phenotype file: We detected Phenotypes that were not in your Phenotype definitions. Please either remove these phenotypes from your Phenotype file, or add them to your Phenotype definitions. The phenotypes we detected with missing definitions were: $phenotype_id_str");
                }
            }
            
            if (empty(form_get_errors()) and isset($phenotype_file_tree_col)){
                
                if ($form_state['saved_values']['Hellopage']['organism']['number'] == 1 or $form_state['saved_values']['thirdPage']['tree-accession']['check'] == '0'){
                    $tree_accession_file = $form_state['saved_values']['thirdPage']['tree-accession']['file'];
                    $column_vals = $form_state['saved_values']['thirdPage']['tree-accession']['file-columns'];
                }
                else {
                    $num = substr($id, 9);
                    $tree_accession_file = $form_state['saved_values']['thirdPage']['tree-accession']["species-$num"]['file'];
                    $column_vals = $form_state['saved_values']['thirdPage']['tree-accession']["species-$num"]['file-columns'];
                }

                foreach ($column_vals as $col => $val){
                    if ($val == '1'){
                        $id_col_accession_name = $col;
                        break;
                    }
                }

                $missing_trees = tpps_compare_files($form_state['values'][$id]['phenotype']['file'], $tree_accession_file, $phenotype_file_tree_col, $id_col_accession_name);

                if ($missing_trees !== array()){
                    $tree_id_str = implode(', ', $missing_trees);
                    form_set_error("$id][phenotype][file", "Phenotype file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Phenotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
                }
            }
        }

        function validate_genotype($genotype, $id, $form, &$form_state){
            $genotype_file = $genotype['file'];
            $marker_type = $genotype['marker-type'];
            $snps_check = $marker_type['SNPs'];
            $ssrs = $marker_type['SSRs/cpSSRs'];
            $other_marker = $marker_type['Other'];
            $marker_check = $snps_check . $ssrs . $other_marker;
            $snps = $genotype['SNPs'];
            $genotype_design = $snps['genotyping-design'];
            $gbs = $snps['GBS'];
            $targeted_capture = $snps['targeted-capture'];
            $bio_id = $genotype['BioProject-id'];
            $ref_genome = $genotype['ref-genome'];
            $vcf = $genotype['vcf'];

            if ($ref_genome === '0'){
                form_set_error("$id][genotype][ref-genome", "Reference Genome: field is required.");
            }
            elseif ($ref_genome === 'url' and $genotype['ref-genome-other'] === ''){
                form_set_error("$id][genotype][ref-genome-other", "Custom Reference Genome: field is required.");
            }
            elseif ($ref_genome === 'bio'){
                if ($bio_id == ''){
                    form_set_error("$id][genotype][Bioproject-id", 'BioProject Id: field is required.');
                }
                else {
                    $assembly_auto = $genotype['assembly-auto'];
                    $assembly_auto_check = '';
                    
                    foreach ($assembly_auto as $item){
                        $assembly_auto_check += $item;
                    }
                    
                    if (preg_match('/^0*$/', $assembly_auto_check)){
                        form_set_error("$id][genotype][assembly-auto", 'Assembly file(s): field is required.');
                    }
                }
            }
            elseif ($ref_genome === 'manual'){
                $assembly = $genotype['assembly-user'];
                
                if ($assembly == ''){
                    form_set_error("$id][genotype][assembly-user", 'Assembly file: field is required.');
                }
                else {
                    $required_columns = array(
                      '1' => 'Scaffold/Chromosome'
                    );
                    
                    $form_state['values'][$id]['genotype']['assembly-user-columns'] = array();

                    foreach ($form[$id]['genotype']['assembly-user']['#value']['columns'] as $req => $val){
                        if ($req[0] != '#'){
                            $form_state['values'][$id]['genotype']['assembly-user-columns'][$req] = $form[$id]['genotype']['assembly-user']['#value']['columns'][$req];

                            $col_val = $form_state['values'][$id]['genotype']['assembly-user-columns'][$req];
                            if ($col_val != '0'){
                                $required_columns[$col_val] = NULL;
                            }
                        }
                    }

                    foreach ($required_columns as $item){
                        if ($item != NULL){
                            form_set_error("$id][genotype][assembly-user][columns][$item", "Assembly file: Please specify a column that holds $item.");
                        }
                    }
                    
                    if ($required_columns['1'] === NULL){
                        $i = 0;
                        foreach ($form_state['values'][$id]['genotype']['assembly-user-columns'] as $key => $val){
                            if ($val == '1'){
                                $scaffold_col = $i;
                                break;
                            }
                            
                            $i++;
                        }
                    }
                }
            }
            
            if ($marker_check === '000'){
                form_set_error("$id][genotype][marker-type", "Genotype Marker Type: field is required.");
            }
            elseif($snps_check === 'SNPs'){
                if ($genotype_design == '0'){
                    form_set_error("$id][genotype][SNPs][genotyping-design", "Genotyping Design: field is required.");
                }
                elseif ($genotype_design == '1'){
                    if ($gbs == '0'){
                        form_set_error("$id][genotype][SNPs][GBS", "GBS Type: field is required.");
                    }
                    elseif ($gbs == '5' and $snps['GBS-other'] == ''){
                        form_set_error("$id][genotype][SNPs][GBS=other", "Custom GBS Type: field is required.");
                    }
                }
                elseif ($genotype_design == '2'){
                    if ($targeted_capture == '0'){
                        form_set_error("$id][genotype][SNPs][targeted-capture", "Targeted Capture: field is required.");
                    }
                    elseif ($targeted_capture == '2' and $snps['targeted-capture-other'] == ''){
                        form_set_error("$id][genotype][SNPs][targeted-capture-other", "Custom Targeted Capture: field is required.");
                    }
                }
            }
            elseif ($ssrs != '0' and $genotype['SSRs/cpSSRs'] == ''){
                form_set_error("$id][genotype][SSRs/cpSSRs", "SSRs/cpSSRs: field is required.");
            }
            elseif ($other_marker != '0' and $genotype['other-marker'] == ''){
                form_set_error("$id][genotype][other-marker", "Other Genotype marker: field is required.");
            }

            if ($snps_check === 'SNPs'){
                if ($vcf == ''){
                    form_set_error("$id][genotype][vcf", "Genotype VCF File: field is required.");
                }
                elseif ($ref_genome === 'manual' and $assembly != '' and isset($scaffold_col) and empty(form_get_errors())){
                    $vcf_content = fopen(file_load($vcf)->uri, 'r');
                    
                    while (($vcf_line = fgets($vcf_content)) !== FALSE){
                        if ($vcf_line[0] != '#'){

                            $assembly_content = fopen(file_load($assembly)->uri, 'r');
                            $vcf_values = explode("\t", $vcf_line);
                            $scaffold_id = $vcf_values[0];
                            $match = FALSE;

                            while (($assembly_line = fgets($assembly_content)) !== FALSE){
                                if ($assembly_line[0] != '>'){
                                    continue;
                                }
                                else{
                                    $assembly_values = explode(' ', $assembly_line);
                                    $assembly_scaffold = $assembly_values[$scaffold_col];
                                    if ($assembly_scaffold == $scaffold_id){
                                        $match = TRUE;
                                        break;
                                    }
                                }
                            }

                            fclose($assembly_content);

                            if (!$match){
                                //dpm($scaffold_id);
                                form_set_error('file', "VCF File: scaffold $scaffold_id not found in assembly file(s)");
                            }
                            else {
                                //dpm("matched: $scaffold_id");
                            }
                        }
                    }
                    
                }
            }
            
            if (($ssrs != '0' or $other_marker != '0') and $genotype_file == ''){
                form_set_error("$id][genotype][file", "Genotype file: field is required.");
            }
            elseif ($ssrs != '0' or $other_marker != '0') {
                $required_columns = array(
                  '1' => 'Tree Identifier',
                );

                $form_state['values'][$id]['genotype']['file-columns'] = array();

                foreach ($form[$id]['genotype']['file']['columns'] as $req => $val){
                    if ($req[0] != '#'){
                        $form_state['values'][$id]['genotype']['file-columns'][$req] = $form[$id]['genotype']['file']['columns'][$req]['#value'];

                        $col_val = $form_state['values'][$id]['genotype']['file-columns'][$req];
                        if ($col_val != '0'){
                            $required_columns[$col_val] = NULL;
                        }
                    }
                }
                
                foreach ($required_columns as $item){
                    if ($item != NULL){
                        form_set_error("$id][genotype][file][columns][$item", "Genotype file: Please specify a column that holds $item.");
                    }
                }

                //if Tree Identifier is set
                if ($required_columns['1'] === NULL and empty(form_get_errors())){
                    //cycle through the columns
                    foreach ($form_state['values'][$id]['genotype']['file-columns'] as $col => $val){
                        //find the column where Tree Identifier is selected
                        if ($val == '1'){
                            //that column name is the name of the Tree Id column, so keep track of that, and exit loop
                            $id_col_genotype_name = $col;
                            break;
                        }
                    }

                    if ($form_state['saved_values']['Hellopage']['organism']['number'] == 1 or $form_state['saved_values']['thirdPage']['tree-accession']['check'] == '0'){
                        $tree_accession_file = $form_state['saved_values']['thirdPage']['tree-accession']['file'];
                        $column_vals = $form_state['saved_values']['thirdPage']['tree-accession']['file-columns'];
                    }
                    else {
                        $num = substr($id, 9);
                        $tree_accession_file = $form_state['saved_values']['thirdPage']['tree-accession']["species-$num"]['file'];
                        $column_vals = $form_state['saved_values']['thirdPage']['tree-accession']["species-$num"]['file-columns'];
                    }

                    foreach ($column_vals as $col => $val){
                        if ($val == '1'){
                            $id_col_accession_name = $col;
                            break;
                        }
                    }

                    $missing_trees = tpps_compare_files($form_state['values'][$id]['genotype']['file'], $tree_accession_file, $id_col_genotype_name, $id_col_accession_name);

                    if ($missing_trees !== array()){
                        $tree_id_str = implode(', ', $missing_trees);
                        form_set_error("$id][genotype][file", "Genotype file: We detected Tree Identifiers that were not in your Tree Accession file. Please either remove these trees from your Genotype file, or add them to your Tree Accesison file. The Tree Identifiers we found were: $tree_id_str");
                    }
                }
            }
        }

        $form_values = $form_state['values'];
        $organism_number = $form_state['saved_values']['Hellopage']['organism']['number'];
        $data_type = $form_state['saved_values']['secondPage']['dataType'];

        for ($i = 1; $i <= $organism_number; $i++){
            $organism = $form_values["organism-$i"];

            if ($data_type == '1' or $data_type == '3' or $data_type == '4'){
                $phenotype = $organism['phenotype'];
                validate_phenotype($phenotype, "organism-$i", $form, $form_state);
            }

            if ($data_type == '1' or $data_type == '2' or $data_type == '3' or $data_type == '5'){
                $genotype = $organism['genotype'];
                validate_genotype($genotype, "organism-$i", $form, $form_state);
            }
        }
    }
    
    /*if (empty(form_get_errors())){
        form_set_error('submit', 'validation success');
    }*/
}

function page_4_submit_form(&$form, &$form_state){
    $form_state['redirect'] = 'fourthPage';
}
