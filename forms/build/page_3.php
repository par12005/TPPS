<?php

require_once 'page_3_ajax.php';

function page_3_create_form(&$form, &$form_state){
    
    if (isset($form_state['saved_values'][PAGE_3])){
        $values = $form_state['saved_values'][PAGE_3];
    }
    else{
        $values = array();
    }
    
    $form['tree-accession'] = array(
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );
    
    $file_description = "Please upload a spreadsheet file containing tree population data. When your file is uploaded, you will be shown a table with your column header names, several drop-downs, and the first few rows of your file. You will be asked to define the data type for each column, using the drop-downs provided to you. If a column data type does not fit any of the options in the drop-down menu, you may omit that drop-down menu. Your file must contain columns with information about at least the Tree Identifier and the Location of the tree (either gps coordinates or country/state).";
    $species_number = $form_state['saved_values'][PAGE_1]['organism']['number'];
    $file_upload_location = 'public://' . variable_get('tpps_accession_files_dir', 'tpps_accession');
    
    if ($form_state['saved_values'][PAGE_2]['studyType'] == '4'){
        $file_description .= ' Location columns should describe the location of the source tree for the Common Garden.';
    }
    
    $form['tree-accession']['file'] = array(
      '#type' => 'managed_file',
      '#title' => t("Tree Accession File: please provide a spreadsheet with columns for the Tree ID and location of trees used in this study: *"),
      '#upload_location' => "$file_upload_location",
      '#upload_validators' => array(
        'file_validate_extensions' => array('txt csv xlsx'),
      ),
      '#description' => $file_description,
      '#states' => ($species_number > 1) ? (array(
        'visible' => array(
          ':input[name="tree-accession[check]"]' => array('checked' => FALSE),
        )
      )) : NULL,
    );
    
    if ($species_number > 1){
        $form['tree-accession']['file']['#description'] .= " If you are uploading a single file with multiple species, your file must also specify the genus and species of each tree.";
    }
    
    $form['tree-accession']['file']['empty'] = array(
      '#default_value' => isset($values['tree-accession']['file']['empty']) ? $values['tree-accession']['file']['empty'] : 'NA',
    );
    
    $form['tree-accession']['file']['columns'] = array(
      '#description' => 'Please define which columns hold the required data: Tree Identifier and Location',
    );
    
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
    
    if ($species_number > 1){
        $column_options['6'] = 'Genus';
        $column_options['7'] = 'Species';
        $column_options['10'] = 'Genus + Species';
    }

    $form['tree-accession']['file']['columns-options'] = array(
      '#type' => 'hidden',
      '#value' => $column_options,
    );
            
    $form['tree-accession']['file']['no-header'] = array();
    
    $form['tree-accession']['coord-format'] = array(
      '#type' => 'select',
      '#title' => t('Coordinate Projection'),
      '#options' => array(
        'WGS 84',
        'NAD 83',
        'ETRS 89',
        'Other Coordinate Projection',
        'My file does not use coordinates for tree locations'
      ),
      '#states' => $form['tree-accession']['file']['#states'],
    );
    
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
        );

        for ($i = 1; $i <= $species_number; $i++){
            $name = $form_state['saved_values'][PAGE_1]['organism']["$i"];
            
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
              '#description' => $file_description,
              '#tree' => TRUE
            );
            
            $form['tree-accession']["species-$i"]['file']['empty'] = array(
              '#default_value' => isset($values['tree-accession']["species-$i"]['file']['empty']) ? $values['tree-accession']["species-$i"]['file']['empty'] : 'NA',
            );
            
            $form['tree-accession']["species-$i"]['file']['columns'] = array(
              '#description' => 'Please define which columns hold the required data: Tree Identifier and Location',
            );
            
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
            
            $form['tree-accession']["species-$i"]['file']['columns-options'] = array(
              '#type' => 'hidden',
              '#value' => $column_options,
            );
            
            $form['tree-accession']["species-$i"]['file']['no-header'] = array();
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
