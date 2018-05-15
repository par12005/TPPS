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
        $location = "/var/www/html/Drupal/sites/default/files/$file_name";
        //dev site
        //$location = "/var/www/Drupal/sites/default/files/$file_name";
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
            //dpm($form_state['saved_values']);
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
	
/*	//This is the beginning of the process data form field which shows the columns detected from file
	//as well as the dynamic select fields
	$form_item_prefix = 'edit-tree-accession'; //id related
	$form_item_columns = array(
		'latitude' => 'Latitude',
		'longitude' => 'Longitude',
		'country' => 'Country',
		'region' => 'Region',
		'treeid' => 'Tree ID',
	);
	
	//Put columns into a csv format to add as an argument to the js function
	$form_item_columns_ids = "";
	foreach($form_item_columns as $id => $column_caption) {
		$form_item_columns_ids .= $id . ","; 
	}
	$form_item_columns_ids = substr($form_item_columns_ids, 0, count($form_item_columns_ids) - 2);
	
	
    $form['tree-accession']['columns'] = array(
      '#type' => 'textarea',
	  '#disabled' => true,
	  '#maxlength' => 1024,
	  '#rows' => 2,
	  '#field_prefix' => "<a class='populate_excel_column_button' onclick='load_excel_header_columns_and_sample_rows(\"$form_item_prefix-file-upload\", \"$form_item_prefix-columns\", \"$form_item_prefix-selectedcolumns\", \"$form_item_columns_ids\")'>Populate Column Data</a>",
	  //'#field_suffix' => $form_item_dynamic_column_html,
      //'#title' => t('Please provide the order of the columns in the file above, separated by commas.'),
	  //'#title' => t('Column data:'),
      '#default_value' => isset($values['tree-accession']['columns']) ? $values['tree-accession']['columns'] : NULL,
      '#states' => ($species_number > 1) ? (array(
        'visible' => array(
          //':input[name="tree-accession[check]"]' => array('checked' => FALSE),
        )
      )) : NULL,
    );
	
	
	foreach($form_item_columns as $id => $column_caption) {
		$form['tree-accession']['selectedcolumns'][$id] = array(
			'#type' => 'select',
			'#title' => t($column_caption),
			
			'#options' => array(
				/* 0 => t('- Select -'), 
			),
			
			'#default_value' => isset($values['tree-accession']['selected_columns'][$id]) ? $values['tree-accession']['selected_columns'][$id] : 0,
			'#states' => array(
				'visible' => array(
					//':input[name="tree-accession[check]"]' => array('checked' => FALSE),
				)
			)			  
		);
	}	*/
    
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
            

			
			//This is the beginning of the process data form field which shows the columns detected from file
			//as well as the dynamic select fields
			$form_item_prefix = 'edit-tree-accession-species-' . $i; //id related
			$form_item_columns = array(
				'treeid' => 'Tree ID',
				'location' => 'Location'
			);
			
			//Put columns into a csv format to add as an argument to the js function
			$form_item_columns_ids = "";
			foreach($form_item_columns as $id => $column_caption) {
				$form_item_columns_ids .= $id . ","; 
			}
			$form_item_columns_ids = substr($form_item_columns_ids, 0, count($form_item_columns_ids) - 2);
			
	
			
			$form['tree-accession']["species-$i"]['columns'] = array(
			  '#type' => 'textarea',
			  '#disabled' => true,
			  '#maxlength' => 1024,
			  '#rows' => 2,
			  '#field_prefix' => "<a class='populate_excel_column_button' onclick='load_excel_header_columns_and_sample_rows(\"$form_item_prefix-file-upload\", \"$form_item_prefix-columns\", \"$form_item_prefix-selectedcolumns\", \"$form_item_columns_ids\")'>Populate Column Data</a>",
			  //'#field_suffix' => $form_item_dynamic_column_html,
			  //'#title' => t('Please provide the order of the columns in the file above, separated by commas.'),
			  //'#title' => t('Column data:'),
			  '#default_value' => isset($values['tree-accession']["species-$i"]['columns']) ? $values['tree-accession']["species-$i"]['columns'] : NULL,
			);	

			foreach($form_item_columns as $id => $column_caption) {
				$form['tree-accession']["species-$i"]['selectedcolumns'][$id] = array(
					'#type' => 'select',
					'#title' => t($column_caption),
					
					'#options' => array(
						/* 0 => t('- Select -'), */
					),
					
					'#default_value' => isset($values['tree-accession']["species-$i"]['selected_columns'][$id]) ? $values['tree-accession']["species-$i"]['selected_columns'][$id] : 0,
					'#states' => array(
						'visible' => array(
							//':input[name="tree-accession[check]"]' => array('checked' => FALSE),
						)
					)			  
				);
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
                    //dpm($form['tree-accession']['file']['columns'][$req]);
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